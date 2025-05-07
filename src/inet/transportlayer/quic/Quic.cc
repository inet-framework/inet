//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "Quic.h"
#include "inet/common/Protocol.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "UdpSocket.h"
#include "AppSocket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/IcmpErrorTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "exception/ConnectionDiedException.h"
#include "packet/ConnectionId.h"

namespace inet {
namespace quic {

Define_Module(Quic);

void random_bytes(void *buf, size_t len) {
    cModule *contextModule = cSimulation::getActiveSimulation()->getContextModule();
    uint8_t *bytebuf = (uint8_t *)buf;
    for (int i = 0; i < len; i++)
        bytebuf[i] = contextModule->intrand(256);
}

static uint64_t get_simtime(ptls_get_time_t *self)
{
    return cSimulation::getActiveSimulation()->getSimTime().inUnit(SIMTIME_MS);
}

ptls_get_time_t opp_get_time = {get_simtime};


Quic::Quic() : OperationalBase()
{
    memset(&ctx, 0, sizeof(ctx));
    ctx.random_bytes = random_bytes;
    ctx.key_exchanges = ptls_openssl_key_exchanges;
    ctx.cipher_suites = ptls_openssl_cipher_suites;
    ctx.get_time = &opp_get_time;
}

Quic::~Quic() {
    std::set<Connection *> connections;
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        connections.insert(it->second);
    }
    for (Connection *connection : connections) {
        delete connection;
    }
    for (std::map<int, UdpSocket *>::iterator it = udpSocketIdUdpSocketMap.begin(); it != udpSocketIdUdpSocketMap.end(); ++it) {
        delete it->second;
    }
    for (std::map<int, AppSocket *>::iterator it = socketIdAppSocketMap.begin(); it != socketIdAppSocketMap.end(); ++it) {
        delete it->second;
    }
}

void Quic::handleStartOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "start operation" << endl;
    registerService(Protocol::quic, gate("appIn"), gate("udpIn"));
    registerProtocol(Protocol::quic, gate("udpOut"), gate("appOut"));
}

void Quic::handleStopOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "stop operation" << endl;
    //removeAllConnections();
}

void Quic::handleCrashOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "crash operation" << endl;
    //removeAllConnections();
}

void Quic::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message when up of kind " << msg->getKind() << endl;

    if (msg->isSelfMessage()) {
        handleTimeout(msg);
    } else if (msg->arrivedOn("appIn")) {
        handleMessageFromApp(msg);
        delete msg;
    } else if (msg->arrivedOn("udpIn")) {
        handleMessageFromUdp(msg);
        delete msg;
    } else {
        throw cRuntimeError("Message arrived from unknown gate.");
    }
}

void Quic::handleTimeout(cMessage *msg)
{
    Connection *connection = (Connection *)msg->getContextPointer();
    try {
        connection->processTimeout(msg);
    } catch (ConnectionDiedException& e) {
        EV_WARN << e.what() << endl;

        AppSocket *appSocket = connection->getAppSocket();

        // delete connection
        connectionIdConnectionMap.erase(connection->getSrcConnectionIds()[0]->getId());
        delete connection;

        destroySockets(appSocket);
    }
}

void Quic::handleMessageFromApp(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int socketId = tags.getTag<SocketReq>()->getSocketId();

    AppSocket *appSocket = findOrCreateAppSocket(socketId);

    Connection *connection = appSocket->getConnection();
    if (connection) {
        connection->processAppCommand(msg);
    } else {
        // no connection found, process message in app socket
        appSocket->processAppCommand(msg);
        if (msg->getKind() == QUIC_C_CLOSE) {
            destroySockets(appSocket);
        }
    }
}

void Quic::handleMessageFromUdp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_SOCKET_CLOSED) {
        EV_WARN << "Socket closed message received from UDP" << endl;
    } else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Error message received from UDP" << endl;
        Indication *ind = check_and_cast<Indication *>(msg);
        auto quotedPacket = ind->getTag<IcmpErrorInd>()->getQuotedPacket()->dup();
        auto icmpHeader = quotedPacket->popAtFront<IcmpHeader>();
        if (icmpHeader != nullptr) {
            if (icmpHeader->getType() == ICMP_DESTINATION_UNREACHABLE && icmpHeader->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
                // Packet Too Big (PTB) message received
                Ptr<const IcmpPtb> icmpPtb = dynamicPtrCast<const IcmpPtb>(icmpHeader);

                quotedPacket->popAtFront<Ipv4Header>(); // skip IP header of reflected packet
                quotedPacket->popAtFront<UdpHeader>(); // skip UDP header of reflected packet
                if (quotedPacket->getByteLength() > 0) {
                    bool readSrcConnectionId = true;
                    uint64_t connectionId = extractConnectionId(quotedPacket, &readSrcConnectionId);
                    Connection *connection = nullptr;
                    if (readSrcConnectionId) {
                        // could read source connection ID, finding connection should be easy
                        connection = findConnection(connectionId);
                    } else {
                        // could only read destination connection ID, try to find connection
                        connection = findConnectionByDstConnectionId(connectionId, quotedPacket);
                    }

                    if (connection) {
                        connection->processIcmpPtb(quotedPacket, icmpPtb->getMtu());
                    } else {
                        EV_WARN << "Could not find connection for a ICMP PTB" << endl;
                    }
                } else {
                    EV_WARN << "ICMP PTB message reflects not enough data from the original packet." << endl;
                }
            } else {
                EV_WARN << "ICMP types other than PTB not handled" << endl;
            }
        } else {
            EV_WARN << "ICMPv6 not handled" << endl;
        }
        delete quotedPacket;
    } else if (msg->getKind() == UDP_I_DATA) {
        Packet *pkt = check_and_cast<Packet *>(msg);

        uint64_t connectionId = extractConnectionId(pkt);
        Connection *connection = findConnection(connectionId);
        if (connection) {
            connection->processPackets(pkt);
        } else {
            EV_DEBUG << "no connection found for packet, use udpSocket to process it" << endl;
            auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
            int udpSocketId = tags.getTag<SocketInd>()->getSocketId();
            UdpSocket *udpSocket = findUdpSocket(udpSocketId);

            udpSocket->processPacket(pkt);
        }
    } else {
        EV_WARN << "Message with unknown kind received from UDP" << endl;
    }
}

AppSocket *Quic::findOrCreateAppSocket(int socketId)
{
    auto it = socketIdAppSocketMap.find(socketId);
    // return appSocket, if found
    if (it != socketIdAppSocketMap.end()) {
        return it->second;
    }
    // create new appSocket
    AppSocket *appSocket = new AppSocket(this, socketId);
    socketIdAppSocketMap.insert({ socketId, appSocket });
    return appSocket;
}

Connection *Quic::findConnection(uint64_t srcConnectionId)
{
    EV_DEBUG << "find connection for ID " << srcConnectionId << endl;
    auto it = connectionIdConnectionMap.find(srcConnectionId);
    if (it == connectionIdConnectionMap.end()) {
        return nullptr;
    }
    return it->second;
}

Connection *Quic::findConnectionByDstConnectionId(uint64_t dstConnectionId, Packet *pkt)
{
    EV_DEBUG << "find connection for destination connection ID " << dstConnectionId << " and packet " << pkt << endl;
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        Connection *connection = it->second;
        if (connection->belongsPacketTo(pkt, dstConnectionId)) {
            return connection;
        }
    }
    return nullptr;
}

UdpSocket *Quic::findUdpSocket(int socketId)
{
    auto it = udpSocketIdUdpSocketMap.find(socketId);
    if (it == udpSocketIdUdpSocketMap.end()) {
        return nullptr;
    }
    return it->second;
}

UdpSocket *Quic::findUdpSocket(L3Address addr, uint16_t port)
{
    for (std::map<int, UdpSocket *>::iterator it = udpSocketIdUdpSocketMap.begin(); it != udpSocketIdUdpSocketMap.end(); ++it) {
        if (it->second->match(addr, port)) {
            return it->second;
        }
    }
    return nullptr;
}

void Quic::addConnection(uint64_t connectionId, Connection *connection)
{
    EV_DEBUG << "Quic::addConnection: add connection for ID " << connectionId << endl;
    auto result = connectionIdConnectionMap.insert({ connectionId, connection });
    if (!result.second) {
        throw cRuntimeError("Cannot insert connection. A connection for the connection id already exists.");
    }
}

void Quic::addConnection(Connection *connection)
{
    for (ConnectionId *srcConnectionId : connection->getSrcConnectionIds()) {
        addConnection(srcConnectionId->getId(), connection);
    }
}

void Quic::removeConnectionId(uint64_t connectionId)
{
    EV_DEBUG << "Quic::removeConnectionId: remove connection ID " << connectionId << endl;
    connectionIdConnectionMap.erase(connectionId);
}

Connection *Quic::createConnection(UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort)
{
    uint64_t connectionId = nextConnectionId;
    nextConnectionId++;
    Connection *connection = new Connection(this, udpSocket, appSocket, remoteAddr, remotePort, connectionId);
    appSocket->setConnection(connection);
    addConnection(connection);
    return connection;
}

void Quic::destroySockets(AppSocket *appSocket)
{
    EV_TRACE << "enter Quic::destroySockets" << endl;

    UdpSocket *udpSocket = appSocket->getUdpSocket();

    // send destroy notification and delete AppSocket
    appSocket->sendDestroyed();
    socketIdAppSocketMap.erase(appSocket->getSocketId());
    delete appSocket;

    // destroy and delete UdpSocket if not used by another connection of app socket
    if (udpSocket != nullptr && !isUdpSocketInUse(udpSocket)) {
        udpSocket->destroy();
        udpSocketIdUdpSocketMap.erase(udpSocket->getSocketId());
        delete udpSocket;
    }

    EV_TRACE << "leave Quic::destroySockets" << endl;
}

uint64_t Quic::extractConnectionId(Packet *pkt, bool *readSourceConnectionId)
{
    auto packetHeader = pkt->peekAtFront<PacketHeader>();
    EV_DEBUG << "extract connection ID from: " << packetHeader << endl;

    switch (packetHeader->getHeaderForm()) {
        case PACKET_HEADER_FORM_LONG: {
            auto longPacketHeader = staticPtrCast<const LongPacketHeader>(packetHeader);
            if (readSourceConnectionId != nullptr && *readSourceConnectionId) {
                return longPacketHeader->getSrcConnectionId();
            }
            return longPacketHeader->getDstConnectionId();
        }
        case PACKET_HEADER_FORM_SHORT: {
            auto shortPacketHeader = staticPtrCast<const ShortPacketHeader>(packetHeader);
            if (readSourceConnectionId != nullptr) {
                *readSourceConnectionId = false;
            }
            return shortPacketHeader->getDstConnectionId();
        }
        default: {
            throw cRuntimeError("Quic::extractConnectionId: Unknown header form.");
        }
    }
}

void Quic::addUdpSocket(UdpSocket *udpSocket)
{
    auto result = udpSocketIdUdpSocketMap.insert({ udpSocket->getSocketId(), udpSocket });
    if (!result.second) {
        throw cRuntimeError("Cannot insert udp socket. A udp socket for the socket id already exists.");
    }
}

UdpSocket *Quic::createUdpSocket()
{
    UdpSocket *udpSocket = new UdpSocket(this);
    addUdpSocket(udpSocket);
    return udpSocket;
}

bool Quic::isUdpSocketInUse(UdpSocket *udpSocket)
{
    // does a connection exists that uses this udp socket?
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        Connection *connection = it->second;
        if (udpSocket == connection->getUdpSocket()) {
            return true;
        }
    }

    // does a app socket exists that uses this udp socket?
    // we might have an app socket with that udp socket that has not (yet) created a connection
    for (std::map<int, AppSocket *>::iterator it = socketIdAppSocketMap.begin(); it != socketIdAppSocketMap.end(); ++it) {
        AppSocket *appSocket = it->second;
        if (udpSocket == appSocket->getUdpSocket()) {
            return true;
        }
    }
    return false;
}

IRoutingTable *Quic::getRoutingTable() {
    return getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
}

} //namespace quic
} //namespace inet
