//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "NoResponseException.h"

namespace inet {
namespace quic {

Define_Module(Quic);

Quic::~Quic() {
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        delete it->second;
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
    } catch (NoResponseException& e) {
        EV_WARN << e.what() << endl;
        connectionIdConnectionMap.erase(connection->getConnectionIds()[0]);
        delete connection;
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
        // no connection for AppSocket, find udpSocket
        UdpSocket *udpSocket = appSocket->getUdpSocket();
        if (!udpSocket) {
            // no udpSocket for AppSocket, find udpSocket by src addr/port for a bind
            if (msg->getKind() == QUIC_C_CREATE_PCB) { // bind
                QuicBindCommand *quicBind = check_and_cast<QuicBindCommand *>(msg->getControlInfo());
                L3Address localAddr = quicBind->getLocalAddr();
                int localPort = quicBind->getLocalPort();
                udpSocket = findUdpSocket(localAddr, localPort);
            }
            // if there is no existing udpSocket, create one
            if (!udpSocket) {
                udpSocket = new UdpSocket(this);
            }
            addUdpSocket(udpSocket);
            appSocket->setUdpSocket(udpSocket);
        }

        udpSocket->processAppCommand(appSocket, msg);
    }
}

void Quic::handleMessageFromUdp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_SOCKET_CLOSED) {
        EV_WARN << "Socket closed message received from UDP" << endl;
    } else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Error message received from UDP" << endl;
        Packet *pkt = check_and_cast<Packet *>(msg->getControlInfo());
        auto header = pkt->popAtFront();
        Ptr<const IcmpHeader> icmpHeader = dynamicPtrCast<const IcmpHeader>(header);
        if (icmpHeader != nullptr) {
            if (icmpHeader->getType() == ICMP_DESTINATION_UNREACHABLE && icmpHeader->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
                // Packet Too Big (PTB) message received
                Ptr<const IcmpPtb> icmpPtb = dynamicPtrCast<const IcmpPtb>(icmpHeader);

                pkt->popAtFront(); // skip IP header of reflected packet
                pkt->popAtFront(); // skip UDP header of reflected packet
                if (pkt->getByteLength() > 0) {
                    uint64_t dstConnectionId = extractConnectionId(pkt);
                    Connection *connection = findConnectionByDstConnectionId(dstConnectionId);
                    if (connection) {
                        connection->processIcmpPtb(pkt, icmpPtb->getMtu());
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
    } else if (msg->getKind() == UDP_I_DATA) {
        Packet *pkt = check_and_cast<Packet *>(msg);

        uint64_t connectionId = extractConnectionId(msg);
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

Connection *Quic::findConnection(uint64_t connectionId)
{
    EV_DEBUG << "find connection for ID " << connectionId << endl;
    auto it = connectionIdConnectionMap.find(connectionId);
    if (it == connectionIdConnectionMap.end()) {
        return nullptr;
    }
    return it->second;
}

Connection *Quic::findConnectionByDstConnectionId(uint64_t connectionId)
{
    EV_DEBUG << "find connection for DST ID " << connectionId << endl;
    return findConnection(connectionId); // TODO: Implement way to find a connection by dst conn id
}

UdpSocket *Quic::findUdpSocket(int socketId)
{
    auto it = udpSocketIdUdpSocketMap.find(socketId);
    if (it == udpSocketIdUdpSocketMap.end()) {
        return nullptr;
    }
    return it->second;
}

UdpSocket *Quic::findUdpSocket(L3Address addr, int port)
{
    for (std::map<int, UdpSocket *>::iterator it = udpSocketIdUdpSocketMap.begin(); it != udpSocketIdUdpSocketMap.end(); ++it) {
        if (it->second->match(addr, port)) {
            return it->second;
        }
    }
    return nullptr;
}

void Quic::addConnection(Connection *connection)
{
    for (uint64_t connectionId : connection->getConnectionIds()) {
        EV_DEBUG << "add connection for ID " << connectionId << endl;
        auto result = connectionIdConnectionMap.insert({ connectionId, connection });
        if (!result.second) {
            throw cRuntimeError("Cannot insert connection. A connection for the connection id already exists.");
        }
    }

}

uint64_t Quic::extractConnectionId(cMessage *msg)
{
    return 0;
}

void Quic::addUdpSocket(UdpSocket *udpSocket)
{
    auto result = udpSocketIdUdpSocketMap.insert({ udpSocket->getSocketId(), udpSocket });
    if (!result.second) {
        throw cRuntimeError("Cannot insert udp socket. A udp socket for the socket id already exists.");
    }
}

IRoutingTable *Quic::getRoutingTable() {
    return getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
}

} //namespace quic
} //namespace inet
