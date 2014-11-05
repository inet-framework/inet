//
// Copyright (C) 2001, 2003, 2004 Johnny Lai, Monash University, Melbourne, Australia
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <iostream>

#include "inet/applications/pingapp/PingApp.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

namespace inet {

using std::cout;

Define_Module(PingApp);

simsignal_t PingApp::rttSignal = registerSignal("rtt");
simsignal_t PingApp::numLostSignal = registerSignal("numLost");
simsignal_t PingApp::outOfOrderArrivalsSignal = registerSignal("outOfOrderArrivals");
simsignal_t PingApp::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t PingApp::pingRxSeqSignal = registerSignal("pingRxSeq");

PingApp::PingApp()
{
    timer = NULL;
    nodeStatus = NULL;
    sendIntervalPar = NULL;
}

PingApp::~PingApp()
{
    cancelAndDelete(timer);
}

void PingApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        // (defer reading srcAddr/destAddr to when ping starts, maybe
        // addresses will be assigned later by some protocol)
        packetSize = par("packetSize");
        sendIntervalPar = &par("sendInterval");
        hopLimit = par("hopLimit");
        count = par("count");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        printPing = par("printPing").boolValue();

        // state
        pid = -1;
        lastStart = -1;
        sendSeqNo = expectedReplySeqNo = 0;
        WATCH(sendSeqNo);
        WATCH(expectedReplySeqNo);

        // statistics
        rttStat.setName("pingRTT");
        sentCount = lossCount = outOfOrderArrivalCount = numPongs = 0;
        WATCH(lossCount);
        WATCH(outOfOrderArrivalCount);
        WATCH(numPongs);

        // references
        timer = new cMessage("sendPing");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // startup
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        if (isEnabled() && isNodeUp())
            startSendingPingRequests();
    }
}

void PingApp::handleMessage(cMessage *msg)
{
    if (!isNodeUp()) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Application is not running");
        delete msg;
        return;
    }
    if (msg == timer) {
        sendPingRequest();
        if (isEnabled())
            scheduleNextPingRequest(simTime());
    }
    else
        processPingResponse(check_and_cast<PingPayload *>(msg));

    if (ev.isGUI()) {
        char buf[40];
        sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool PingApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
            startSendingPingRequests();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            stopSendingPingRequests();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            stopSendingPingRequests();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void PingApp::startSendingPingRequests()
{
    ASSERT(!timer->isScheduled());
    pid = simulation.getUniqueNumber();
    lastStart = simTime();
    scheduleNextPingRequest(-1);
}

void PingApp::stopSendingPingRequests()
{
    pid = -1;
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    srcAddr = destAddr = L3Address();
    cancelNextPingRequest();
}

void PingApp::scheduleNextPingRequest(simtime_t previous)
{
    simtime_t next;
    if (previous == -1)
        next = simTime() <= startTime ? startTime : simTime();
    else
        next = previous + sendIntervalPar->doubleValue();
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void PingApp::cancelNextPingRequest()
{
    cancelEvent(timer);
}

bool PingApp::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool PingApp::isEnabled()
{
    return par("destAddr").stringValue()[0] && (count == -1 || sentCount < count);
}

void PingApp::sendPingRequest()
{
    if (destAddr.isUnspecified()) {
        srcAddr = L3AddressResolver().resolve(par("srcAddr"));
        destAddr = L3AddressResolver().resolve(par("destAddr"));
        EV_INFO << "Starting up with destination = " << destAddr << ", source = " << srcAddr << ".\n";
    }

    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    PingPayload *msg = new PingPayload(name);
    msg->setOriginatorId(pid);
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);
    std::string time = SIMTIME_STR(simTime());
    int timeLength = time.length();
    msg->setDataArraySize(timeLength);
    for (int i = 0; i < timeLength; i++)
        msg->setData(i, time[i]);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PING_HISTORY_SIZE] = simTime();

    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
    sentCount++;
    sendToICMP(msg, destAddr, srcAddr, hopLimit);
}

void PingApp::sendToICMP(PingPayload *msg, const L3Address& destAddr, const L3Address& srcAddr, int hopLimit)
{
    IL3AddressType *addressType = destAddr.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setSourceAddress(srcAddr);
    controlInfo->setDestinationAddress(destAddr);
    controlInfo->setHopLimit(hopLimit);
    // TODO: remove
    controlInfo->setTransportProtocol(1);    // IP_PROT_ICMP);
    msg->setControlInfo(dynamic_cast<cObject *>(controlInfo));
    EV_INFO << "Sending ping request #" << msg->getSeqNo() << " to lower layer.\n";
    send(msg, "pingOut");
}

void PingApp::processPingResponse(PingPayload *msg)
{
    if (msg->getOriginatorId() != pid) {
        EV_WARN << "Received response was not sent by this application, dropping packet\n";
        delete msg;
        return;
    }

    int timeLength = msg->getDataArraySize();
    char time[timeLength + 1];    //FIXME visualC ???
    for (int i = 0; i < timeLength; i++)
        time[i] = msg->getData(i);
    time[timeLength] = '\0';
    simtime_t sendTime = STR_SIMTIME(time);    // Why converting to/from string?

    if (sendTime < lastStart) {
        EV_WARN << "Received response was not sent since last application start, dropping packet\n";
        delete msg;
        return;
    }

    // get src, hopCount etc from packet, and print them
    INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
    L3Address src = ctrl->getSourceAddress();
    //Address dest = ctrl->getDestinationAddress();
    int msgHopLimit = ctrl->getHopLimit();

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PING_HISTORY_SIZE ?
        0 : simTime() - sendTimeHistory[msg->getSeqNo() % PING_HISTORY_SIZE];

    if (printPing) {
        // TODO: why not EV? is it because of express mode?
        cout << getFullPath() << ": reply of " << std::dec << msg->getByteLength()
             << " bytes from " << src
             << " icmp_seq=" << msg->getSeqNo() << " ttl=" << msgHopLimit
             << " time=" << (rtt * 1000) << " msec"
             << " (" << msg->getName() << ")" << endl;
    }

    // update statistics
    countPingResponse(msg->getByteLength(), msg->getSeqNo(), rtt);
    delete msg;
}

void PingApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
{
    EV_INFO << "Received ping reply #" << seqNo << " with RTT = " << rtt << "\n";
    emit(pingRxSeqSignal, seqNo);

    numPongs++;

    // count only non 0 RTT values as 0s are invalid
    if (rtt > 0) {
        rttStat.collect(rtt);
        emit(rttSignal, rtt);
    }

    if (seqNo == expectedReplySeqNo) {
        // expected ping reply arrived; expect next sequence number
        expectedReplySeqNo++;
    }
    else if (seqNo > expectedReplySeqNo) {
        EV_DETAIL << "Jump in seq numbers, assuming pings since #" << expectedReplySeqNo << " got lost\n";

        // jump in the sequence: count pings in gap as lost for now
        // (if they arrive later, we'll decrement back the loss counter)
        long jump = seqNo - expectedReplySeqNo;
        lossCount += jump;
        emit(numLostSignal, lossCount);

        // expect sequence numbers to continue from here
        expectedReplySeqNo = seqNo + 1;
    }
    else {    // seqNo < expectedReplySeqNo
              // ping reply arrived too late: count as out-of-order arrival (not loss after all)
        EV_DETAIL << "Arrived out of order (too late)\n";
        outOfOrderArrivalCount++;
        lossCount--;
        emit(outOfOrderArrivalsSignal, outOfOrderArrivalCount);
        emit(numLostSignal, lossCount);
    }
}

void PingApp::finish()
{
    if (sentCount == 0) {
        if (printPing)
            EV_WARN << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
        return;
    }

    lossCount += sendSeqNo - expectedReplySeqNo;
    // record statistics
    recordScalar("Pings sent", sentCount);
    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sentCount);
    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sentCount);

    // print it to stdout as well
    if (printPing) {
        // TODO: why not EV? is it because of express mode?
        cout << "--------------------------------------------------------" << endl;
        cout << "\t" << getFullPath() << endl;
        cout << "--------------------------------------------------------" << endl;
        cout << "ping " << par("destAddr").stringValue() << " (" << destAddr << "):" << endl;
        cout << "sent: " << sentCount << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double)sentCount) << endl;
        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
        cout << "--------------------------------------------------------" << endl;
    }
}

} // namespace inet

