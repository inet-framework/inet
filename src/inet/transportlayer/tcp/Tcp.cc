//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif // ifdef INET_WITH_IPv6

#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

Define_Module(Tcp);

simsignal_t Tcp::tcpConnectionAddedSignal = registerSignal("tcpConnectionAdded");
simsignal_t Tcp::tcpConnectionRemovedSignal = registerSignal("tcpConnectionRemoved");

Tcp::~Tcp()
{
}

void Tcp::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;

        msl = par("msl");
        useDataNotification = par("useDataNotification");

        WATCH(lastEphemeralPort);
        WATCH_PTRMAP(tcpConnMap);
        WATCH_PTRMAP(tcpAppConnMap);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        registerService(Protocol::tcp, gate("appIn"), gate("appOut"));
        registerProtocol(Protocol::tcp, gate("ipOut"), gate("ipIn"));
        if (crcMode == CRC_COMPUTED) {
            cModuleType *moduleType = cModuleType::get("inet.transportlayer.tcp_common.TcpCrcInsertionHook");
            auto crcInsertion = check_and_cast<TcpCrcInsertionHook *>(moduleType->create("crcInsertion", this));
            crcInsertion->finalizeParameters();
            crcInsertion->callInitialize();

#ifdef INET_WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(findModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, crcInsertion);
#endif
#ifdef INET_WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(findModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, crcInsertion);
#endif
        }
    }
}

void Tcp::finish()
{
    EV_INFO << getFullPath() << ": finishing with " << tcpConnMap.size() << " connections open.\n";
}

void Tcp::handleSelfMessage(cMessage *msg)
{
    throw cRuntimeError("model error: should schedule timers on connection");
}

void Tcp::handleUpperCommand(cMessage *msg)
{
    int socketId = check_and_cast<ITaggedObject *>(msg)->getTags().getTag<SocketReq>()->getSocketId();
    TcpConnection *conn = findConnForApp(socketId);

    if (!conn) {
        conn = createConnection(socketId);

        // add into appConnMap here; it'll be added to connMap during processing
        // the OPEN command in TcpConnection's processAppCommand().
        tcpAppConnMap[socketId] = conn;

        EV_INFO << "Tcp connection created for " << msg << "\n";
    }

    if (!conn->processAppCommand(msg))
        removeConnection(conn);
}

void Tcp::sendFromConn(cMessage *msg, const char *gatename, int gateindex)
{
    Enter_Method("sendFromConn");
    take(msg);
    send(msg, gatename, gateindex);
}

void Tcp::handleUpperPacket(Packet *packet)
{
    handleUpperCommand(packet);
}

TcpConnection *Tcp::findConnForApp(int socketId)
{
    auto i = tcpAppConnMap.find(socketId);
    return i == tcpAppConnMap.end() ? nullptr : i->second;
}

void Tcp::handleLowerPacket(Packet *packet)
{
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::tcp) {
        if (!checkCrc(packet)) {
            EV_WARN << "Tcp segment has wrong CRC, dropped\n";
            PacketDropDetails details;
            details.setReason(INCORRECTLY_RECEIVED);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
            return;
        }

        // must be a TcpHeader
        auto tcpHeader = packet->peekAtFront<TcpHeader>();

        // get src/dest addresses
        L3Address srcAddr, destAddr;
        srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
        destAddr = packet->getTag<L3AddressInd>()->getDestAddress();
        int ecn = 0;
        if (auto ecnTag = packet->findTag<EcnInd>())
            ecn = ecnTag->getExplicitCongestionNotification();
        ASSERT(ecn != -1);

        // process segment
        TcpConnection *conn = findConnForSegment(tcpHeader, srcAddr, destAddr);
        if (conn) {
            TcpStateVariables *state = conn->getState();
            if (state && state->ect) {
                // This may be true only in receiver side. According to RFC 3168, page 20:
                // pure acknowledgement packets (e.g., packets that do not contain
                // any accompanying data) MUST be sent with the not-ECT codepoint.
                state->gotCeIndication = (ecn == 3);
            }

            bool ret = conn->processTCPSegment(packet, tcpHeader, srcAddr, destAddr);
            if (!ret)
                removeConnection(conn);
        }
        else {
            segmentArrivalWhileClosed(packet, tcpHeader, srcAddr, destAddr);
        }
    }
    else if (protocol == &Protocol::icmpv4 || protocol == &Protocol::icmpv6) {
        EV_DETAIL << "ICMP error received -- discarding\n"; // FIXME can ICMP packets really make it up to Tcp???
        delete packet;
    }
    else
        throw cRuntimeError("Unknown protocol: '%s'", (protocol != nullptr ? protocol->getName() : "<nullptr>"));
}

TcpConnection *Tcp::createConnection(int socketId)
{
    auto moduleType = cModuleType::get("inet.transportlayer.tcp.TcpConnection");
    char submoduleName[24];
    sprintf(submoduleName, "conn-%d", socketId);
    auto module = check_and_cast<TcpConnection *>(moduleType->createScheduleInit(submoduleName, this));
    module->initConnection(this, socketId);
    return module;
}

void Tcp::removeConnection(TcpConnection *conn)
{
    EV_INFO << "Deleting Tcp connection\n";

    tcpAppConnMap.erase(conn->socketId);

    SockPair key2;
    key2.localAddr = conn->localAddr;
    key2.remoteAddr = conn->remoteAddr;
    key2.localPort = conn->localPort;
    key2.remotePort = conn->remotePort;
    tcpConnMap.erase(key2);

    // IMPORTANT: usedEphemeralPorts.erase(conn->localPort) is NOT GOOD because it
    // deletes ALL occurrences of the port from the multiset.
    auto it = usedEphemeralPorts.find(conn->localPort);

    if (it != usedEphemeralPorts.end())
        usedEphemeralPorts.erase(it);

    emit(tcpConnectionRemovedSignal, conn);
    conn->deleteModule();
}

TcpConnection *Tcp::findConnForSegment(const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    SockPair key;
    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = tcpHeader->getDestPort();
    key.remotePort = tcpHeader->getSrcPort();
    SockPair save = key;

    // try with fully qualified SockPair
    auto i = tcpConnMap.find(key);
    if (i != tcpConnMap.end())
        return i->second;

    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr = L3Address();
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // try fully qualified local socket + blank remote socket (for incoming SYN)
    key = save;
    key.remoteAddr = L3Address();
    key.remotePort = -1;
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // try with blank remote socket, and localAddr missing (for incoming SYN)
    key.localAddr = L3Address();
    i = tcpConnMap.find(key);

    if (i != tcpConnMap.end())
        return i->second;

    // given up
    return nullptr;
}

void Tcp::segmentArrivalWhileClosed(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    auto moduleType = cModuleType::get("inet.transportlayer.tcp.TcpConnection");
    const char *submoduleName = "conn-temp";
    auto module = check_and_cast<TcpConnection *>(moduleType->createScheduleInit(submoduleName, this));
    module->initConnection(this, -1);
    module->segmentArrivalWhileClosed(tcpSegment, tcpHeader, srcAddr, destAddr);
    module->deleteModule();
    delete tcpSegment;
}

ushort Tcp::getEphemeralPort()
{
    // start at the last allocated port number + 1, and search for an unused one
    ushort searchUntil = lastEphemeralPort++;
    if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;

    while (usedEphemeralPorts.find(lastEphemeralPort) != usedEphemeralPorts.end()) {
        if (lastEphemeralPort == searchUntil) // got back to starting point?
            throw cRuntimeError("Ephemeral port range %d..%d exhausted, all ports occupied", EPHEMERAL_PORTRANGE_START, EPHEMERAL_PORTRANGE_END);

        lastEphemeralPort++;

        if (lastEphemeralPort == EPHEMERAL_PORTRANGE_END) // wrap
            lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    }

    // found a free one, return it
    return lastEphemeralPort;
}

void Tcp::addForkedConnection(TcpConnection *conn, TcpConnection *newConn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // update conn's socket pair, and register newConn
    addSockPair(newConn, localAddr, remoteAddr, localPort, remotePort);

    // newConn will live on with the new socketId
    tcpAppConnMap[newConn->socketId] = newConn;
}

void Tcp::addSockPair(TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // update addresses/ports in TcpConnection
    SockPair key;
    key.localAddr = conn->localAddr = localAddr;
    key.remoteAddr = conn->remoteAddr = remoteAddr;
    key.localPort = conn->localPort = localPort;
    key.remotePort = conn->remotePort = remotePort;

    // make sure connection is unique
    auto it = tcpConnMap.find(key);
    if (it != tcpConnMap.end()) {
        // throw "address already in use" error
        if (remoteAddr.isUnspecified() && remotePort == -1)
            throw cRuntimeError("Address already in use: there is already a connection listening on %s:%d",
                    localAddr.str().c_str(), localPort);
        else
            throw cRuntimeError("Address already in use: there is already a connection %s:%d to %s:%d",
                    localAddr.str().c_str(), localPort, remoteAddr.str().c_str(), remotePort);
    }

    // then insert it into tcpConnMap
    tcpConnMap[key] = conn;

    // mark port as used
    if (localPort >= EPHEMERAL_PORTRANGE_START && localPort < EPHEMERAL_PORTRANGE_END)
        usedEphemeralPorts.insert(localPort);
}

void Tcp::updateSockPair(TcpConnection *conn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // find with existing address/port pair...
    SockPair key;
    key.localAddr = conn->localAddr;
    key.remoteAddr = conn->remoteAddr;
    key.localPort = conn->localPort;
    key.remotePort = conn->remotePort;
    auto it = tcpConnMap.find(key);

    ASSERT(it != tcpConnMap.end() && it->second == conn);

    // ...and remove from the old place in tcpConnMap
    tcpConnMap.erase(it);

    // then update addresses/ports, and re-insert it with new key into tcpConnMap
    key.localAddr = conn->localAddr = localAddr;
    key.remoteAddr = conn->remoteAddr = remoteAddr;
    ASSERT(conn->localPort == localPort);
    key.remotePort = conn->remotePort = remotePort;
    tcpConnMap[key] = conn;

    // localPort doesn't change (see ASSERT above), so there's no need to update usedEphemeralPorts[].
}

TcpSendQueue *Tcp::createSendQueue()
{
    return new TcpSendQueue();
}

TcpReceiveQueue *Tcp::createReceiveQueue()
{
    return new TcpReceiveQueue();
}

void Tcp::handleStartOperation(LifecycleOperation *operation)
{
    // FIXME implementation
}

void Tcp::handleStopOperation(LifecycleOperation *operation)
{
    // FIXME close connections??? yes, because the applications may not close them!!!
    reset();
    delayActiveOperationFinish(par("stopOperationTimeout"));
    startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void Tcp::handleCrashOperation(LifecycleOperation *operation)
{
    reset();
}

void Tcp::reset()
{
    for (auto& elem : tcpAppConnMap)
        elem.second->deleteModule();
    tcpAppConnMap.clear();
    tcpConnMap.clear();
    usedEphemeralPorts.clear();
    lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
}

// packet contains the tcpHeader
bool Tcp::checkCrc(Packet *tcpSegment)
{
    auto tcpHeader = tcpSegment->peekAtFront<TcpHeader>();

    switch (tcpHeader->getCrcMode()) {
        case CRC_COMPUTED: {
            // check CRC:
            auto networkProtocol = tcpSegment->getTag<NetworkProtocolInd>()->getProtocol();
            const std::vector<uint8_t> tcpBytes = tcpSegment->peekDataAsBytes()->getBytes();
            auto pseudoHeader = makeShared<TransportPseudoHeader>();
            L3Address srcAddr = tcpSegment->getTag<L3AddressInd>()->getSrcAddress();
            L3Address destAddr = tcpSegment->getTag<L3AddressInd>()->getDestAddress();
            pseudoHeader->setSrcAddress(srcAddr);
            pseudoHeader->setDestAddress(destAddr);
            ASSERT(networkProtocol);
            pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
            pseudoHeader->setProtocolId(IP_PROT_TCP);
            pseudoHeader->setPacketLength(B(tcpBytes.size()));
            // pseudoHeader length: ipv4: 12 bytes, ipv6: 40 bytes, generic: ???
            if (networkProtocol == &Protocol::ipv4)
                pseudoHeader->setChunkLength(B(12));
            else if (networkProtocol == &Protocol::ipv6)
                pseudoHeader->setChunkLength(B(40));
            else
                throw cRuntimeError("Unknown network protocol: %s", networkProtocol->getName());
            MemoryOutputStream stream;
            Chunk::serialize(stream, pseudoHeader);
            Chunk::serialize(stream, tcpSegment->peekData());
            uint16_t crc = TcpIpChecksum::checksum(stream.getData());
            return crc == 0;
        }
        case CRC_DECLARED_CORRECT:
            return true;
        case CRC_DECLARED_INCORRECT:
            return false;
        default:
            break;
    }
    throw cRuntimeError("unknown CRC mode: %d", tcpHeader->getCrcMode());
}

void Tcp::refreshDisplay() const
{
    OperationalBase::refreshDisplay();

    if (getEnvir()->isExpressMode()) {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    int numINIT = 0, numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
        numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
        numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (auto& elem : tcpAppConnMap) {
        int state = (elem).second->getFsmState();

        switch (state) {
            case TCP_S_INIT:
                numINIT++;
                break;

            case TCP_S_CLOSED:
                numCLOSED++;
                break;

            case TCP_S_LISTEN:
                numLISTEN++;
                break;

            case TCP_S_SYN_SENT:
                numSYN_SENT++;
                break;

            case TCP_S_SYN_RCVD:
                numSYN_RCVD++;
                break;

            case TCP_S_ESTABLISHED:
                numESTABLISHED++;
                break;

            case TCP_S_CLOSE_WAIT:
                numCLOSE_WAIT++;
                break;

            case TCP_S_LAST_ACK:
                numLAST_ACK++;
                break;

            case TCP_S_FIN_WAIT_1:
                numFIN_WAIT_1++;
                break;

            case TCP_S_FIN_WAIT_2:
                numFIN_WAIT_2++;
                break;

            case TCP_S_CLOSING:
                numCLOSING++;
                break;

            case TCP_S_TIME_WAIT:
                numTIME_WAIT++;
                break;
        }
    }

    char buf2[200];
    buf2[0] = '\0';

    if (numINIT > 0)
        sprintf(buf2 + strlen(buf2), "init:%d ", numINIT);
    if (numCLOSED > 0)
        sprintf(buf2 + strlen(buf2), "closed:%d ", numCLOSED);
    if (numLISTEN > 0)
        sprintf(buf2 + strlen(buf2), "listen:%d ", numLISTEN);
    if (numSYN_SENT > 0)
        sprintf(buf2 + strlen(buf2), "syn_sent:%d ", numSYN_SENT);
    if (numSYN_RCVD > 0)
        sprintf(buf2 + strlen(buf2), "syn_rcvd:%d ", numSYN_RCVD);
    if (numESTABLISHED > 0)
        sprintf(buf2 + strlen(buf2), "estab:%d ", numESTABLISHED);
    if (numCLOSE_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "close_wait:%d ", numCLOSE_WAIT);
    if (numLAST_ACK > 0)
        sprintf(buf2 + strlen(buf2), "last_ack:%d ", numLAST_ACK);
    if (numFIN_WAIT_1 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_1:%d ", numFIN_WAIT_1);
    if (numFIN_WAIT_2 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_2:%d ", numFIN_WAIT_2);
    if (numCLOSING > 0)
        sprintf(buf2 + strlen(buf2), "closing:%d ", numCLOSING);
    if (numTIME_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "time_wait:%d ", numTIME_WAIT);

    getDisplayString().setTagArg("t", 0, buf2);
}

std::ostream& operator<<(std::ostream& os, const Tcp::SockPair& sp)
{
    os << "locSocket=" << sp.localAddr << ":" << sp.localPort << " "
       << "remSocket=" << sp.remoteAddr << ":" << sp.remotePort;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TcpConnection& conn)
{
    os << "socketId=" << conn.socketId << " ";
    os << "fsmState=" << TcpConnection::stateName(conn.getFsmState()) << " ";
    os << "connection=" << (conn.getState() == nullptr ? "<empty>" : conn.getState()->str()) << " ";
    os << "ttl=" << (conn.ttl == -1 ? "<default>" : std::to_string(conn.ttl)) << " ";
    return os;
}

} // namespace tcp
} // namespace inet

