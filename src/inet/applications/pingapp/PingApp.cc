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
#include "inet/applications/pingapp/PingApp_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/L3Socket.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/networklayer/contract/ipv6/Ipv6Socket.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef WITH_IPv6


namespace inet {

using std::cout;

Define_Module(PingApp);

simsignal_t PingApp::rttSignal = registerSignal("rtt");
simsignal_t PingApp::numLostSignal = registerSignal("numLost");
simsignal_t PingApp::numOutOfOrderArrivalsSignal = registerSignal("numOutOfOrderArrivals");
simsignal_t PingApp::pingTxSeqSignal = registerSignal("pingTxSeq");
simsignal_t PingApp::pingRxSeqSignal = registerSignal("pingRxSeq");

const std::map<const Protocol *, const Protocol *> PingApp::l3Echo( {
    { &Protocol::ipv4, &Protocol::icmpv4 },
    { &Protocol::ipv6, &Protocol::icmpv6 },
    { &Protocol::flooding, &Protocol::echo },
    { &Protocol::nextHopForwarding, &Protocol::echo },
    { &Protocol::probabilistic, &Protocol::echo },
    { &Protocol::wiseRoute, &Protocol::echo },
});

PingApp::PingApp()
{
}

PingApp::~PingApp()
{
    cancelAndDelete(timer);
    socketMap.deleteSockets();
}

void PingApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

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
        printPing = par("printPing");
        continuous = par("continuous");

        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        // state
        pid = -1;
        lastStart = -1;
        sendSeqNo = expectedReplySeqNo = 0;
        for (int i = 0; i < PING_HISTORY_SIZE; i++) {
            sendTimeHistory[i] = SIMTIME_MAX;
            pongReceived[i] = false;
        }
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

void PingApp::handleSelfMessage(cMessage *msg)
{
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
        if (destAddrIdx >= (int)destAddresses.size())
            return;
        destAddr = destAddresses[destAddrIdx];
        EV_INFO << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "seqNo=" << sendSeqNo << endl;
        ASSERT(!destAddr.isUnspecified());
        const Protocol *networkProtocol = nullptr;
        const char *networkProtocolAsString = par("networkProtocol");
        if (*networkProtocolAsString)
            networkProtocol = Protocol::getProtocol(networkProtocolAsString);
        else {
            switch (destAddr.getType()) {
                case L3Address::IPv4: networkProtocol = &Protocol::ipv4; break;
                case L3Address::IPv6: networkProtocol = &Protocol::ipv6; break;
                case L3Address::MODULEID:
                case L3Address::MODULEPATH: networkProtocol = &Protocol::nextHopForwarding; break;
                default: throw cRuntimeError("unknown address type: %d(%s)", (int)destAddr.getType(), L3Address::getTypeName(destAddr.getType()));
            }
        }
        const Protocol *icmp = l3Echo.at(networkProtocol);

        for (auto socket: socketMap.getMap()) {
            socket.second->close();
        }
        currentSocket = nullptr;
        if (networkProtocol == &Protocol::ipv4)
            currentSocket = new Ipv4Socket(gate("socketOut"));
        else if (networkProtocol == &Protocol::ipv6)
            currentSocket = new Ipv6Socket(gate("socketOut"));
        else
            currentSocket = new L3Socket(networkProtocol, gate("socketOut"));
        socketMap.addSocket(currentSocket);
        currentSocket->bind(icmp, L3Address());
        currentSocket->setCallback(this);
        msg->setKind(PING_SEND);
    }

    ASSERT2(msg->getKind() == PING_SEND, "Unknown kind in self message.");

    // send a ping
    sendPingRequest();

    if (count > 0 && sendSeqNo % count == 0) {
        // choose next dest address
        destAddrIdx++;
        msg->setKind(PING_CHANGE_ADDR);
        if (destAddrIdx >= (int)destAddresses.size()) {
            if (continuous) {
                destAddrIdx = destAddrIdx % destAddresses.size();
            }
        }
    }

    // then schedule next one if needed
    scheduleNextPingRequest(simTime(), msg->getKind() == PING_CHANGE_ADDR);
}

void PingApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else {
        auto socket = check_and_cast_nullable<INetworkSocket *>(socketMap.findSocketFor(msg));
        if (socket)
            socket->processMessage(msg);
        else
            throw cRuntimeError("Unaccepted message: %s(%s)", msg->getName(), msg->getClassName());
    }
    if (operationalState == State::STOPPING_OPERATION && socketMap.size() == 0)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void PingApp::socketDataArrived(INetworkSocket *socket, Packet *packet)
{
#ifdef WITH_IPv4
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::icmpv4) {
        const auto& icmpHeader = packet->popAtFront<IcmpHeader>();
        if (icmpHeader->getType() == ICMP_ECHO_REPLY) {
            const auto& echoReply = CHK(dynamicPtrCast<const IcmpEchoReply>(icmpHeader));
            processPingResponse(echoReply->getIdentifier(), echoReply->getSeqNumber(), packet);
        }
        else {
            // process other icmp messages, process icmp errors
        }
        delete packet;
    }
    else
#endif
#ifdef WITH_IPv6
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::icmpv6) {
        const auto& icmpHeader = packet->popAtFront<Icmpv6Header>();
        if (icmpHeader->getType() == ICMPv6_ECHO_REPLY) {
            const auto& echoReply = CHK(dynamicPtrCast<const Icmpv6EchoReplyMsg>(icmpHeader));
            processPingResponse(echoReply->getIdentifier(), echoReply->getSeqNumber(), packet);
        }
        else {
            // process other icmpv6 messages, process icmpv6 errors
        }
        delete packet;
    }
    else
#endif
#ifdef WITH_NEXTHOP
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::echo) {
        const auto& icmpHeader = packet->popAtFront<EchoPacket>();
        if (icmpHeader->getType() == ECHO_PROTOCOL_REPLY) {
            processPingResponse(icmpHeader->getIdentifier(), icmpHeader->getSeqNumber(), packet);
        }
        else {
            // process other icmp messages, process icmp errors
        }
        delete packet;
    }
    else
#endif
    {
        throw cRuntimeError("Unaccepted packet: %s(%s)", packet->getName(), packet->getClassName());
    }
}

void PingApp::socketClosed(INetworkSocket *socket)
{
    if (socket == currentSocket)
        currentSocket = nullptr;
    delete socketMap.removeSocket(socket);
}

void PingApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[40];
    sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
    getDisplayString().setTagArg("t", 0, buf);
}

void PingApp::handleStartOperation(LifecycleOperation *operation)
{
    if (isEnabled())
        startSendingPingRequests();
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

void PingApp::handleStopOperation(LifecycleOperation *operation)
{
    pid = -1;
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    srcAddr = destAddr = L3Address();
    destAddresses.clear();
    destAddrIdx = -1;
    cancelNextPingRequest();
    currentSocket = nullptr;
    // TODO: close sockets
    // TODO: remove getMap()
    if (socketMap.size() > 0) {
        for (auto socket: socketMap.getMap())
            socket.second->close();
    }
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void PingApp::handleCrashOperation(LifecycleOperation *operation)
{
    pid = -1;
    lastStart = -1;
    sendSeqNo = expectedReplySeqNo = 0;
    srcAddr = destAddr = L3Address();
    destAddresses.clear();
    destAddrIdx = -1;
    cancelNextPingRequest();
    currentSocket = nullptr;
    // TODO: remove check?
    if (operation->getRootModule() != getContainingNode(this)) {
        // TODO: destroy sockets
        for (auto socket: socketMap.getMap())
            socket.second->destroy();
        socketMap.deleteSockets();
    }
}

void PingApp::scheduleNextPingRequest(simtime_t previous, bool withSleep)
{
    simtime_t next;
    if (previous < SIMTIME_ZERO)
        next = simTime() <= startTime ? startTime : simTime();
    else {
        next = previous + *sendIntervalPar;
        if (withSleep)
            next += *sleepDurationPar;
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void PingApp::cancelNextPingRequest()
{
    cancelEvent(timer);
}

bool PingApp::isEnabled()
{
    return par("destAddr").stringValue()[0] && (count == -1 || sentCount < count);
}

void PingApp::sendPingRequest()
{
    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    ASSERT(pid != -1);

    Packet *outPacket = new Packet(name);
    auto payload = makeShared<ByteCountChunk>(B(packetSize));

    switch (destAddr.getType()) {
        case L3Address::IPv4: {
#ifdef WITH_IPv4
            const auto& request = makeShared<IcmpEchoRequest>();
            request->setIdentifier(pid);
            request->setSeqNumber(sendSeqNo);
            outPacket->insertAtBack(payload);
            Icmp::insertCrc(crcMode, request, outPacket);
            outPacket->insertAtFront(request);
            outPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);
            break;
#else
            throw cRuntimeError("INET compiled without Ipv4");
#endif
        }
        case L3Address::IPv6: {
#ifdef WITH_IPv6
            const auto& request = makeShared<Icmpv6EchoRequestMsg>();
            request->setIdentifier(pid);
            request->setSeqNumber(sendSeqNo);
            outPacket->insertAtBack(payload);
            Icmpv6::insertCrc(crcMode, request, outPacket);
            outPacket->insertAtFront(request);
            outPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
            break;
#else
            throw cRuntimeError("INET compiled without Ipv6");
#endif
        }
        case L3Address::MODULEID:
        case L3Address::MODULEPATH: {
#ifdef WITH_NEXTHOP
            const auto& request = makeShared<EchoPacket>();
            request->setChunkLength(B(8));
            request->setType(ECHO_PROTOCOL_REQUEST);
            request->setIdentifier(pid);
            request->setSeqNumber(sendSeqNo);
            outPacket->insertAtBack(payload);
            // insertCrc(crcMode, request, outPacket);
            outPacket->insertAtFront(request);
            outPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::echo);
            break;
#else
            throw cRuntimeError("INET compiled without Next Hop Forwarding");
#endif
        }
        default:
            throw cRuntimeError("Unaccepted destination address type: %d (address: %s)", (int)destAddr.getType(), destAddr.str().c_str());
    }

    auto addressReq = outPacket->addTag<L3AddressReq>();
    addressReq->setSrcAddress(srcAddr);
    addressReq->setDestAddress(destAddr);
    if (hopLimit != -1)
        outPacket->addTag<HopLimitReq>()->setHopLimit(hopLimit);
    EV_INFO << "Sending ping request #" << sendSeqNo << " to lower layer.\n";
    currentSocket->send(outPacket);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PING_HISTORY_SIZE] = simTime();
    pongReceived[sendSeqNo % PING_HISTORY_SIZE] = false;
    emit(pingTxSeqSignal, sendSeqNo);

    sendSeqNo++;
    sentCount++;
}

void PingApp::processPingResponse(int originatorId, int seqNo, Packet *packet)
{
    const auto& pingPayload = packet->peekDataAt(B(0), packet->getDataLength());
    if (originatorId != pid) {
        EV_WARN << "Received response was not sent by this application, dropping packet\n";
        return;
    }

    // get src, hopCount etc from packet, and print them
    L3Address src = packet->getTag<L3AddressInd>()->getSrcAddress();
    //L3Address dest = msg->getTag<L3AddressInd>()->getDestination();
    auto msgHopCountTag = packet->findTag<HopLimitInd>();
    int msgHopCount = msgHopCountTag ? msgHopCountTag->getHopLimit() : -1;

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = SIMTIME_ZERO;
    bool isDup = false;
    if (sendSeqNo - seqNo < PING_HISTORY_SIZE) {
        int idx = seqNo % PING_HISTORY_SIZE;
        rtt = simTime() - sendTimeHistory[idx];
        isDup = pongReceived[idx];
        pongReceived[idx] = true;
    }

    if (printPing) {
        cout << getFullPath() << ": reply of " << packet->getByteLength()
             << " bytes from " << src
             << " icmp_seq=" << seqNo << " ttl=" << msgHopCount
             << " time=" << (rtt * 1000) << " msec"
             << " (" << packet->getName() << ")" << endl;
    }

    // update statistics
    countPingResponse(B(pingPayload->getChunkLength()).get(), seqNo, rtt, isDup);
}

void PingApp::countPingResponse(int bytes, long seqNo, simtime_t rtt, bool isDup)
{
    EV_INFO << "Ping reply #" << seqNo << " arrived, rtt=" << (rtt == SIMTIME_ZERO ? "unknown" : rtt.str().c_str()) << (isDup ? ", duplicated" : "") << "\n";
    emit(pingRxSeqSignal, seqNo);

    if (isDup)
        numDuplicatedPongs++;
    else
        numPongs++;

    // count only non 0 RTT values as 0s are invalid
    if (rtt > SIMTIME_ZERO && !isDup) {
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
        if (!isDup && rtt > SIMTIME_ZERO)
            lossCount--;
        emit(numOutOfOrderArrivalsSignal, outOfOrderArrivalCount);
        emit(numLostSignal, lossCount);
    }
}

std::vector<L3Address> PingApp::getAllAddresses()
{
    std::vector<L3Address> result;

    int lastId = getSimulation()->getLastComponentId();

    for (int i = 0; i <= lastId; i++)
    {
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(getSimulation()->getModule(i));
        if (ift) {
            for (int j = 0; j < ift->getNumInterfaces(); j++) {
                InterfaceEntry *ie = ift->getInterface(j);
                if (ie && !ie->isLoopback()) {
#ifdef WITH_IPv4
                    auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
                    if (ipv4Data != nullptr) {
                        Ipv4Address address = ipv4Data->getIPAddress();
                        if (!address.isUnspecified())
                            result.push_back(L3Address(address));
                    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                    auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
                    if (ipv6Data != nullptr) {
                        for (int k = 0; k < ipv6Data->getNumAddresses(); k++) {
                            Ipv6Address address = ipv6Data->getAddress(k);
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

} // namespace inet

