//
// Copyright (C) 2004 Andras Varga
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

#include "inet/transportlayer/tcp/Tcp.h"

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif // ifdef WITH_IPv6


namespace inet {

namespace tcp {

Define_Module(Tcp);

simsignal_t Tcp::tcpConnectionAddedSignal = registerSignal("tcpConnectionAdded");
simsignal_t Tcp::tcpConnectionRemovedSignal = registerSignal("tcpConnectionRemoved");

#define EPHEMERAL_PORTRANGE_START    1024
#define EPHEMERAL_PORTRANGE_END      5000

static std::ostream& operator<<(std::ostream& os, const Tcp::SockPair& sp)
{
    os << "loc=" << sp.localAddr << ":" << sp.localPort << " "
       << "rem=" << sp.remoteAddr << ":" << sp.remotePort;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const Tcp::AppConnKey& app)
{
    os << "socketId=" << app.socketId;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const TcpConnection& conn)
{
    os << "socketId=" << conn.socketId << " " << TcpConnection::stateName(conn.getFsmState())
       << " state={" << conn.getState()->str() << "}";
    return os;
}

void Tcp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
        WATCH(lastEphemeralPort);

        WATCH_PTRMAP(tcpConnMap);
        WATCH_PTRMAP(tcpAppConnMap);

        recordStatistics = par("recordStats");
        useDataNotification = par("useDataNotification");

        const char *crcModeString = par("crcMode");
        if (!strcmp(crcModeString, "declared"))
            crcMode = CRC_DECLARED_CORRECT;
        else if (!strcmp(crcModeString, "computed"))
            crcMode = CRC_COMPUTED;
        else
            throw cRuntimeError("Unknown crc mode: '%s'", crcModeString);
        crcInsertion.setCrcMode(crcMode);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        cModule *host = findContainingNode(this);
        NodeStatus *nodeStatus = check_and_cast_nullable<NodeStatus *>(host ? host->getSubmodule("status") : nullptr);
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        registerService(Protocol::tcp, gate("appIn"), gate("ipIn"));
        registerProtocol(Protocol::tcp, gate("ipOut"), gate("appOut"));
        if (crcMode == CRC_COMPUTED) {
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, &crcInsertion);
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
    }
}

Tcp::~Tcp()
{
    while (!tcpAppConnMap.empty()) {
        auto i = tcpAppConnMap.begin();
        delete i->second;
        tcpAppConnMap.erase(i);
    }
}

// packet contains the tcpHeader
bool Tcp::checkCrc(const Ptr<const TcpHeader>& tcpHeader, Packet *packet)
{
    switch (tcpHeader->getCrcMode()) {
        case CRC_COMPUTED: {
            //check CRC:
            auto networkProtocol = packet->getTag<NetworkProtocolInd>()->getProtocol();
            const std::vector<uint8_t> tcpBytes = packet->peekDataAsBytes()->getBytes();
            auto pseudoHeader = makeShared<TransportPseudoHeader>();
            L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
            L3Address destAddr = packet->getTag<L3AddressInd>()->getDestAddress();
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
            Chunk::serialize(stream, packet->peekData());
            uint16_t crc = TcpIpChecksum::checksum(stream.getData());
            return (crc == 0);
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

void Tcp::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
        EV_ERROR << "Tcp is turned off, dropping '" << msg->getName() << "' message\n";
        delete msg;
    }
    else if (msg->isSelfMessage()) {
        TcpConnection *conn = (TcpConnection *)msg->getContextPointer();
        bool ret = conn->processTimer(msg);
        if (!ret)
            removeConnection(conn);
    }
    else if (msg->arrivedOn("ipIn")) {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::icmpv4 || protocol == &Protocol::icmpv6)  {
            EV_DETAIL << "ICMP error received -- discarding\n";    // FIXME can ICMP packets really make it up to Tcp???
            delete msg;
        }
        else if (protocol == &Protocol::tcp) {
            // must be a TcpHeader
            auto tcpHeader = packet->peekAtFront<TcpHeader>();

            // get src/dest addresses
            L3Address srcAddr, destAddr;

            srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
            destAddr = packet->getTag<L3AddressInd>()->getDestAddress();
            //interfaceId = controlInfo->getInterfaceId();

            if (!checkCrc(tcpHeader, packet)) {
                EV_WARN << "Tcp segment has wrong CRC, dropped\n";
                delete packet;
                return;
            }

            // process segment
            TcpConnection *conn = findConnForSegment(tcpHeader, srcAddr, destAddr);
            if (conn) {
                bool ret = conn->processTCPSegment(packet, tcpHeader, srcAddr, destAddr);
                if (!ret)
                    removeConnection(conn);
            }
            else {
                segmentArrivalWhileClosed(packet, tcpHeader, srcAddr, destAddr);
            }
        }
        else
            throw cRuntimeError("Unknown protocol: '%s'", (protocol != nullptr ? protocol->getName() : "<nullptr>"));
    }
    else {    // must be from app
        auto& tags = getTags(msg);
        int socketId = tags.getTag<SocketReq>()->getSocketId();

        TcpConnection *conn = findConnForApp(socketId);

        if (!conn) {
            conn = createConnection(socketId);

            // add into appConnMap here; it'll be added to connMap during processing
            // the OPEN command in TcpConnection's processAppCommand().
            AppConnKey key;
            key.socketId = socketId;
            tcpAppConnMap[key] = conn;

            EV_INFO << "Tcp connection created for " << msg << "\n";
        }
        bool ret = conn->processAppCommand(msg);
        if (!ret)
            removeConnection(conn);
    }
}

TcpConnection *Tcp::createConnection(int socketId)
{
    return new TcpConnection(this, socketId);
}

void Tcp::segmentArrivalWhileClosed(Packet *packet, const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr)
{
    TcpConnection *tmp = new TcpConnection(this);
    tmp->segmentArrivalWhileClosed(packet, tcpseg, srcAddr, destAddr);
    delete tmp;
    delete packet;
}

void Tcp::refreshDisplay() const
{
    if (getEnvir()->isExpressMode()) {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    //char buf[40];
    //sprintf(buf,"%d conns", tcpAppConnMap.size());
    //getDisplayString().setTagArg("t",0,buf);
    if (isOperational)
        getDisplayString().removeTag("i2");
    else
        getDisplayString().setTagArg("i2", 0, "status/cross");

    int numINIT = 0, numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
        numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
        numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (auto & elem : tcpAppConnMap) {
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

TcpConnection *Tcp::findConnForSegment(const Ptr<const TcpHeader>& tcpseg, L3Address srcAddr, L3Address destAddr)
{
    SockPair key;
    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = tcpseg->getDestPort();
    key.remotePort = tcpseg->getSrcPort();
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

TcpConnection *Tcp::findConnForApp(int socketId)
{
    AppConnKey key;
    key.socketId = socketId;

    auto i = tcpAppConnMap.find(key);
    return i == tcpAppConnMap.end() ? nullptr : i->second;
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

void Tcp::addForkedConnection(TcpConnection *conn, TcpConnection *newConn, L3Address localAddr, L3Address remoteAddr, int localPort, int remotePort)
{
    // update conn's socket pair, and register newConn (which'll keep LISTENing)
    updateSockPair(conn, localAddr, remoteAddr, localPort, remotePort);
    addSockPair(newConn, newConn->localAddr, newConn->remoteAddr, newConn->localPort, newConn->remotePort);

    // conn will get a new socketId...
    AppConnKey key;
    key.socketId = conn->socketId;
    tcpAppConnMap.erase(key);
    conn->listeningSocketId = conn->socketId;
    key.socketId = conn->socketId = getEnvir()->getUniqueNumber();
    tcpAppConnMap[key] = conn;

    // ...and newConn will live on with the old socketId
    key.socketId = newConn->socketId;
    tcpAppConnMap[key] = newConn;
}

void Tcp::removeConnection(TcpConnection *conn)
{
    EV_INFO << "Deleting Tcp connection\n";

    AppConnKey key;
    key.socketId = conn->socketId;
    tcpAppConnMap.erase(key);

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
    delete conn;
}

void Tcp::finish()
{
    EV_INFO << getFullPath() << ": finishing with " << tcpConnMap.size() << " connections open.\n";
}

TcpSendQueue *Tcp::createSendQueue()
{
    return new TcpSendQueue();
}

TcpReceiveQueue *Tcp::createReceiveQueue()
{
    return new TcpReceiveQueue();
}

bool Tcp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_TRANSPORT_LAYER) {
            //FIXME implementation
            isOperational = true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_TRANSPORT_LAYER) {
            //FIXME close connections???
            reset();
            isOperational = false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH) {
            //FIXME implementation
            reset();
            isOperational = false;
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

void Tcp::reset()
{
    for (auto & elem : tcpAppConnMap)
        delete elem.second;
    tcpAppConnMap.clear();
    tcpConnMap.clear();
    usedEphemeralPorts.clear();
    lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
}

} // namespace tcp

} // namespace inet
