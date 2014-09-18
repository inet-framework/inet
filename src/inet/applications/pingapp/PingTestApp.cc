//
// Copyright (C) 2012 OpenSim Ltd.
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

#include "inet/applications/pingapp/PingTestApp.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/pingapp/PingPayload_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

using std::cout;

Define_Module(PingTestApp);

simsignal_t PingTestApp::rttSignal = registerSignal("rtt");
simsignal_t PingTestApp::numLostSignal = registerSignal("numLost");
simsignal_t PingTestApp::outOfOrderArrivalsSignal = registerSignal("outOfOrderArrivals");
simsignal_t PingTestApp::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t PingTestApp::pingRxSeqSignal = registerSignal("pingRxSeq");

void PingTestApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params
        // (defer reading srcAddr/destAddr to when ping starts, maybe
        // addresses will be assigned later by some protocol)
        packetSize = par("packetSize");
        sendIntervalp = &par("sendInterval");
        hopLimit = par("hopLimit");
        count = par("count");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        printPing = par("printPing").boolValue();
        continuous = par("continuous").boolValue();

        // state
        sendSeqNo = expectedReplySeqNo = 0;
        WATCH(sendSeqNo);
        WATCH(expectedReplySeqNo);

        // statistics
        rttStat.setName("pingRTT");

        lossCount = outOfOrderArrivalCount = numPongs = 0;
        WATCH(lossCount);
        WATCH(outOfOrderArrivalCount);
        WATCH(numPongs);

        if (strcmp(par("destAddresses").stringValue(), "")) {
            // schedule first ping (use empty destAddr to disable)
            cMessage *msg = new cMessage("sendPing");
            scheduleAt(startTime, msg);
        }
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void PingTestApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (sendSeqNo == 0) {
            srcAddr = L3AddressResolver().resolve(par("srcAddr"));
            const char *destAddrs = par("destAddresses");
            if (!strcmp(destAddrs, "*")) {
                destAddresses = getAllAddresses();
            }
            else {
                cStringTokenizer tokenizer(destAddrs);
                const char *token;

                while ((token = tokenizer.nextToken()) != NULL) {
                    L3Address addr = L3AddressResolver().resolve(token);
                    destAddresses.push_back(addr);
                }
            }
        }

        if (sendSeqNo % count == 0) {
            // choose next dest address
            unsigned long i = sendSeqNo / count;
            if (i >= destAddresses.size() && !continuous) {
                delete msg;
                return;
            }

            i = i % destAddresses.size();
            destAddr = destAddresses[i];
            EV_INFO << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "seqNo=" << sendSeqNo << endl;
            ASSERT(!destAddr.isUnspecified());
        }

        // send a ping
        sendPing();

        // then schedule next one if needed
        scheduleNextPing(msg);
    }
    else {
        // process ping response
        processPingResponse(check_and_cast<PingPayload *>(msg));
    }
}

std::vector<L3Address> PingTestApp::getAllAddresses()
{
    std::vector<L3Address> result;

#if OMNETPP_VERSION < 0x500
    for (int i = 0; i <= simulation.getLastModuleId(); i++)
#else // if OMNETPP_VERSION < 0x500
    for (int i = 0; i <= simulation.getLastComponentId(); i++)
#endif // if OMNETPP_VERSION < 0x500
    {
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(simulation.getModule(i));
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

void PingTestApp::sendPing()
{
    EV_INFO << "Sending ping #" << sendSeqNo << "\n";

    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    PingPayload *msg = new PingPayload(name);
    msg->setOriginatorId(getId());
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PINGTEST_HISTORY_SIZE] = simTime();

    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
    sendToICMP(msg, destAddr, srcAddr, hopLimit);
}

void PingTestApp::scheduleNextPing(cMessage *timer)
{
    simtime_t nextPing = simTime() + sendIntervalp->doubleValue();
    if (sendSeqNo % count == 0)
        nextPing += par("sleepDuration").doubleValue();

    if (stopTime < SIMTIME_ZERO || nextPing < stopTime)
        scheduleAt(nextPing, timer);
    else
        delete timer;
}

void PingTestApp::sendToICMP(cMessage *msg, const L3Address& destAddr, const L3Address& srcAddr, int hopLimit)
{
    IL3AddressType *addressType = destAddr.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setSourceAddress(srcAddr);
    controlInfo->setDestinationAddress(destAddr);
    controlInfo->setHopLimit(hopLimit);
    // TODO: remove
    controlInfo->setTransportProtocol(1);    // IP_PROT_ICMP);
    msg->setControlInfo(dynamic_cast<cObject *>(controlInfo));
    send(msg, "pingOut");
}

void PingTestApp::processPingResponse(PingPayload *msg)
{
    // get src, hopCount etc from packet, and print them
    L3Address src, dest;
    int msgHopCount = -1;

    ASSERT(msg->getOriginatorId() == getId());    // ICMP module error

    // get src, hopCount etc from packet, and print them
    INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
    src = ctrl->getSourceAddress();
    dest = ctrl->getDestinationAddress();
    msgHopCount = ctrl->getHopLimit();

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PINGTEST_HISTORY_SIZE ?
        0 : simTime() - sendTimeHistory[msg->getSeqNo() % PINGTEST_HISTORY_SIZE];

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

void PingTestApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
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
        EV_INFO << "Jump in seq numbers, assuming pings since #" << expectedReplySeqNo << " got lost\n";

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
        EV_INFO << "Arrived out of order (too late)\n";
        outOfOrderArrivalCount++;
        lossCount--;
        emit(outOfOrderArrivalsSignal, outOfOrderArrivalCount);
        emit(numLostSignal, lossCount);
    }
}

void PingTestApp::finish()
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

} // namespace inet

