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

#include "UdpSocket.h"
#include "AppSocket.h"
#include "QueueAppSocket.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"

namespace inet {
namespace quic {

UdpSocket::UdpSocket(Quic *quicSimpleMod)
{
    this->quicSimpleMod = quicSimpleMod;
    socket.setOutputGate(quicSimpleMod->gate("udpOut"));
    isListening = false;
}

UdpSocket::~UdpSocket() { }

void UdpSocket::processPacket(Packet *pkt)
{
    if (!isListening) {
        throw cRuntimeError("UdpSocket::processPacket: Unexpected packet");
    }
    auto& tags = pkt->getTags();
    L3Address remoteAddr = tags.getTag<L3AddressInd>()->getSrcAddress();
    int remotePort = tags.getTag<L4PortInd>()->getSrcPort();

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

bool UdpSocket::match(L3Address addr, int port)
{
    return (this->localAddr == addr && this->localPort == port);
}

void UdpSocket::sendto(L3Address remoteAddr, int remotePort, Packet *pkt)
{
    // send all QUIC packets with Don't Fragment bit set in IP header
    pkt->addTag<FragmentationReq>()->setDontFragment(true);
    socket.sendTo(pkt, remoteAddr, remotePort);
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

void UdpSocket::bind(L3Address addr, int port)
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
    socket.bind(addr, port);
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
    isListening = true;
}

void UdpSocket::unlisten()
{
    isListening = false;
    listeningAppSocket = nullptr;
}

} /* namespace quic */
} /* namespace inet */
