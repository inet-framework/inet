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

#include "inet/applications/pingapp/PingPayload_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6

namespace inet {

using std::cout;

Define_Module(PingApp);

simsignal_t PingApp::rttSignal = registerSignal("rtt");
simsignal_t PingApp::numLostSignal = registerSignal("numLost");
simsignal_t PingApp::outOfOrderArrivalsSignal = registerSignal("outOfOrderArrivals");
simsignal_t PingApp::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t PingApp::pingRxSeqSignal = registerSignal("pingRxSeq");

enum PingSelfKinds {
    PING_FIRST_ADDR = 1001,
    PING_CHANGE_ADDR,
    PING_SEND
};

PingApp::PingApp()
{
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
        sleepDurationPar = &par("sleepDuration");
        hopLimit = par("hopLimit");
        count = par("count");
//        if (count <= 0 && count != -1)
//            throw cRuntimeError("Invalid count=%d parameter (should use -1 or a larger than zero value)", count);
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        printPing = par("printPing").boolValue();
        continuous = par("continuous").boolValue();

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
        timer = new cMessage("sendPing", PING_FIRST_ADDR);


    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // startup
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        if (isEnabled() && isNodeUp())
            startSendingPingRequests();
    }
}

void PingApp::parseDestAddressesPar()
{
    srcAddr = L3AddressResolver().resolve(par("srcAddr"));
    const char *destAddrs = par("destAddr");
    if (!strcmp(destAddrs, "*")) {
        destAddresses = getAllAddresses();
    }
    else {
        cStringTokenizer tokenizer(destAddrs);
        const char *token;

        while ((token = tokenizer.nextToken()) != nullptr) {
            L3Address addr = L3AddressResolver().resolve(token);
            destAddresses.push_back(addr);
        }
    }
}

void PingApp::handleMessage(cMessage *msg)
{
    if (!isNodeUp()) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Self message '%s' received when %s is down", msg->getName(), getComponentType()->getName());
        else {
            EV_WARN << "PingApp is down, dropping '" << msg->getName() << "' message\n";
            delete msg;
            return;
        }
    }
    if (msg->isSelfMessage()) {
        if (msg->getKind() == PING_FIRST_ADDR) {
            srcAddr = L3AddressResolver().resolve(par("srcAddr"));
            parseDestAddressesPar();
            if (destAddresses.empty()) {
                return;
            }
            destAddrIdx = 0;
            msg->setKind(PING_CHANGE_ADDR);
        }

        if (msg->getKind() == PING_CHANGE_ADDR) {
            if (destAddrIdx >= destAddresses.size())
                return;
            destAddr = destAddresses[destAddrIdx];
            EV_INFO << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "seqNo=" << sendSeqNo << endl;
            ASSERT(!destAddr.isUnspecified());
            msg->setKind(PING_SEND);
        }

        ASSERT2(msg->getKind() == PING_SEND, "Unknown kind in self message.");

        // send a ping
        sendPing();

        if (count > 0 && sendSeqNo % count == 0) {
            // choose next dest address
            destAddrIdx++;
            msg->setKind(PING_CHANGE_ADDR);
            if (destAddrIdx >= destAddresses.size()) {
                if (continuous) {
                    destAddrIdx = destAddrIdx % destAddresses.size();
                }
            }
        }

        // then schedule next one if needed
        scheduleNextPingRequest(simTime(), msg->getKind() == PING_CHANGE_ADDR);
    }
    else {
        // process ping response
        processPingResponse(check_and_cast<PingPayload *>(msg));
    }

    if (hasGUI()) {
        char buf[40];
        sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool PingApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
            startSendingPingRequests();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            stopSendingPingRequests();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stopSendingPingRequests();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void PingApp::startSendingPingRequests()
{
    ASSERT(!timer->isScheduled());
    pid = getSimulation()->getUniqueNumber();
    lastStart = simTime();
    timer->setKind(PING_FIRST_ADDR);
    sentCount = 0;
    sendSeqNo = 0;
    scheduleNextPingRequest(-1, false);
}

void PingApp::stopSendingPingRequests()
{
    pid = -1;
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    srcAddr = destAddr = L3Address();
    destAddresses.clear();
    destAddrIdx = -1;
    cancelNextPingRequest();
}

void PingApp::scheduleNextPingRequest(simtime_t previous, bool withSleep)
{
    simtime_t next;
    if (previous < SIMTIME_ZERO)
        next = simTime() <= startTime ? startTime : simTime();
    else {
        next = previous + sendIntervalPar->doubleValue();
        if (withSleep)
            next += sleepDurationPar->doubleValue();
    }
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

void PingApp::processPingResponse(PingPayload *msg)
{
    if (msg->getOriginatorId() != pid) {
        EV_WARN << "Received response was not sent by this application, dropping packet\n";
        delete msg;
        return;
    }

    // get src, hopCount etc from packet, and print them
    INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
    L3Address src = ctrl->getSourceAddress();
    //L3Address dest = ctrl->getDestinationAddress();
    int msgHopCount = ctrl->getHopLimit();

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PING_HISTORY_SIZE ?
        0 : simTime() - sendTimeHistory[msg->getSeqNo() % PING_HISTORY_SIZE];

    if (printPing) {
        cout << getFullPath() << ": reply of " << std::dec << msg->getByteLength()
             << " bytes from " << src
             << " icmp_seq=" << msg->getSeqNo() << " ttl=" << msgHopCount
             << " time=" << (rtt * 1000) << " msec"
             << " (" << msg->getName() << ")" << endl;
    }

    // update statistics
    countPingResponse(msg->getByteLength(), msg->getSeqNo(), rtt);
    delete msg;
}

void PingApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
{
    EV_INFO << "Ping reply #" << seqNo << " arrived, rtt=" << rtt << "\n";
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

std::vector<L3Address> PingApp::getAllAddresses()
{
    std::vector<L3Address> result;

#if OMNETPP_VERSION < 0x500
    int lastId = getSimulation()->getLastModuleId();
#else // if OMNETPP_VERSION < 0x500
    int lastId = getSimulation()->getLastComponentId();
#endif // if OMNETPP_VERSION < 0x500

    for (int i = 0; i <= lastId; i++)
    {
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(getSimulation()->getModule(i));
        if (ift) {
            for (int j = 0; j < ift->getNumInterfaces(); j++) {
                InterfaceEntry *ie = ift->getInterface(j);
                if (ie && !ie->isLoopback()) {
#ifdef WITH_IPv4
                    if (ie->ipv4Data()) {
                        IPv4Address address = ie->ipv4Data()->getIPAddress();
                        if (!address.isUnspecified())
                            result.push_back(L3Address(address));
                    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                    if (ie->ipv6Data()) {
                        for (int k = 0; k < ie->ipv6Data()->getNumAddresses(); k++) {
                            IPv6Address address = ie->ipv6Data()->getAddress(k);
                            if (!address.isUnspecified() && address.isGlobal())
                                result.push_back(L3Address(address));
                        }
                    }
#endif // ifdef WITH_IPv6
                }
            }
        }
    }
    return result;
}

void PingApp::finish()
{
    if (sendSeqNo == 0) {
        if (printPing)
            EV_DETAIL << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
        return;
    }

    lossCount += sendSeqNo - expectedReplySeqNo;
    // record statistics
    recordScalar("Pings sent", sendSeqNo);
    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sendSeqNo);
    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sendSeqNo);

    // print it to stdout as well
    if (printPing) {
        cout << "--------------------------------------------------------" << endl;
        cout << "\t" << getFullPath() << endl;
        cout << "--------------------------------------------------------" << endl;

        cout << "sent: " << sendSeqNo << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double)sendSeqNo) << endl;
        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
        cout << "--------------------------------------------------------" << endl;
    }
}

void PingApp::sendPing()
{
    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    PingPayload *msg = new PingPayload(name);
    ASSERT(pid != -1);
    msg->setOriginatorId(pid);
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize + 4);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PING_HISTORY_SIZE] = simTime();

    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
    sentCount++;
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

} // namespace inet

