//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "UdpSocket.h"
#include "AppSocket.h"
#include "QueueAppSocket.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"

namespace inet {
namespace quic {

UdpSocket::UdpSocket(Quic *quicSimpleMod): quicSimpleMod(quicSimpleMod)
{
    socket.setOutputGate(quicSimpleMod->gate("udpOut"));
}

UdpSocket::~UdpSocket() { }

void UdpSocket::processPacket(Packet *pkt)
{
    if (!isListening()) {
        throw cRuntimeError("UdpSocket::processPacket: Unexpected packet");
    }
    if (dynamicPtrCast<const InitialPacketHeader>(pkt->peekAtFront()) == nullptr) {
        EV_WARN << "UdpSocket::processPacket: received a packet on a listening socket that is not a QUIC Initial packet. Ignore.";
        return;
    }
    auto& tags = pkt->getTags();
    L3Address remoteAddr = tags.getTag<L3AddressInd>()->getSrcAddress();
    uint16_t remotePort = (uint16_t)tags.getTag<L4PortInd>()->getSrcPort();

    // create AppSocket that queues indications for the new connection
    AppSocket *queueAppSocket = new QueueAppSocket(quicSimpleMod);
    queueAppSocket->setUdpSocket(this);

    // create new connection and handle the (INIT) packet
    Connection *connection = quicSimpleMod->createConnection(this, queueAppSocket, remoteAddr, remotePort);
    connection->processPackets(pkt);

    connectionQueue.push(connection);

    // notify the app about the new connection
    listeningAppSocket->sendConnectionAvailable();
}

int UdpSocket::getSocketId()
{
    return socket.getSocketId();
}

bool UdpSocket::match(L3Address addr, uint16_t port)
{
    return (this->localAddr == addr && this->localPort == port);
}

void UdpSocket::sendto(L3Address remoteAddr, uint16_t remotePort, Packet *pkt)
{
    // send all QUIC packets with Don't Fragment bit set in IP header
    pkt->addTag<FragmentationReq>()->setDontFragment(true);
    socket.sendTo(pkt, remoteAddr, (int)remotePort);
}

Connection *UdpSocket::popConnection()
{
    if (connectionQueue.empty()) {
        return nullptr;
    }
    Connection *connection = connectionQueue.front();
    connectionQueue.pop();
    return connection;
}

void UdpSocket::bind(L3Address addr, uint16_t port)
{
    if (port == 0) {
        throw cRuntimeError("UdpSocket::bind: cannot bind to port 0");
    }
    if (localPort != 0) {
        // already bound
        return;
    }
    localAddr = addr;
    localPort = port;
    socket.bind(addr, (int)port);
}

void UdpSocket::listen(AppSocket *appSocket)
{
    if (appSocket == nullptr) {
        throw cRuntimeError("UdpSocket::listen: called with appSocket=nullptr, not valid");
    }
    if (listeningAppSocket == appSocket) {
        // already listening for this app socket
        return;
    }
    if (listeningAppSocket != nullptr) {
        throw cRuntimeError("UdpSocket::listen: Already listening for another app socket");
    }
    listeningAppSocket = appSocket;
}

void UdpSocket::unlisten()
{
    listeningAppSocket = nullptr;
}

bool UdpSocket::isListening()
{
    return (listeningAppSocket != nullptr);
}

void UdpSocket::destroy()
{
    socket.destroy();
}

void UdpSocket::saveToken(uint32_t token, L3Address remoteAddr)
{
    EV_DEBUG << "UdpSocket::saveToken(" << token << ", " << remoteAddr << ")" << endl;
    auto result = tokenRemoteIpMap.insert({ token, remoteAddr });
    if (result.second == false) {
        EV_INFO << "UdpSocket::saveToken: The given token is already in the map." << endl;
        if (result.first->second != remoteAddr) {
            throw cRuntimeError("UdpSocket::saveToken: Got the same token for another remoteAddr.");
        }
    }
}

bool UdpSocket::doesTokenExist(uint32_t token, L3Address remoteAddr)
{
    auto it = tokenRemoteIpMap.find(token);
    if (it != tokenRemoteIpMap.end() && it->second == remoteAddr) {
        tokenRemoteIpMap.erase(token);
        return true;
    }
    EV_DEBUG << "UdpSocket::doesTokenExist: token " << token << " not found or address " << remoteAddr << " does not match" << endl;
    return false;
}

} /* namespace quic */
} /* namespace inet */
