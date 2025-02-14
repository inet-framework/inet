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

UdpSocket::~UdpSocket()
{

}

void UdpSocket::processAppCommand(AppSocket *appSocket, cMessage *msg) {
    switch (msg->getKind()) {
        case QUIC_C_CREATE_PCB: { // bind
            QuicBindCommand *quicBind = check_and_cast<QuicBindCommand *>(msg->getControlInfo());
            localAddr = quicBind->getLocalAddr();
            localPort = quicBind->getLocalPort();
            socket.bind(localAddr, localPort);
            break;
        }
        case QUIC_C_OPEN_PASSIVE: { // listen
            isListening = true;
            // currently a server accepts only one connection upon the first incoming packet
            listeningAppSocket = appSocket;
            break;
        }
        case QUIC_C_OPEN_ACTIVE: { // connect
            QuicOpenCommand *quicOpen = check_and_cast<QuicOpenCommand *>(msg->getControlInfo());
            L3Address remoteAddr = quicOpen->getRemoteAddr();
            int remotePort = quicOpen->getRemotePort();
            Connection *connection = createConnection(appSocket, remoteAddr, remotePort);
            connection->connect();
            break;
        }
        case QUIC_C_ACCEPT: { // accept
            // currently a server accepts only one connection upon the first incoming packet
            break;
        }
        default: {
            throw cRuntimeError("Unexpected/unknown App Command");
        }

    }
}

Connection *UdpSocket::createConnection(AppSocket *appSocket, L3Address remoteAddr, int remotePort)
{
    Connection *connection = new Connection(quicSimpleMod, this, appSocket, remoteAddr, remotePort);
    appSocket->setConnection(connection);
    quicSimpleMod->addConnection(connection);
    return connection;
}

void UdpSocket::processPacket(Packet *pkt)
{
    if (!isListening) {
        throw cRuntimeError("Unexpected packet");
    }
    // currently a server accepts only one connection upon the first incoming packet
    auto& tags = pkt->getTags();
    L3Address remoteAddr = tags.getTag<L3AddressInd>()->getSrcAddress();
    int remotePort = tags.getTag<L4PortInd>()->getSrcPort();
    Connection *connection = createConnection(listeningAppSocket, remoteAddr, remotePort);
    connection->processPackets(pkt);

    connection->accept();
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
    static int outgoingPacketCounter = 0;
    double sendLossRate = quicSimpleMod->par("sendLossRate");
    if (sendLossRate != 0.0) {
        if (quicSimpleMod->par("periodicSendLoss")) {
            outgoingPacketCounter++;
            if (outgoingPacketCounter > 1/sendLossRate) {
                // drop packet
                delete pkt;
                outgoingPacketCounter = 0;
                return;
            }
        } else {
            // random send loss
            if (quicSimpleMod->uniform(0.0, 1.0) < sendLossRate) {
                // drop packet
                delete pkt;
                return;
            }
        }
    }

    // send all QUIC packets with Don't Fragment bit set in IP header
    pkt->addTag<FragmentationReq>()->setDontFragment(true);
    socket.sendTo(pkt, remoteAddr, remotePort);
}

} /* namespace quic */
} /* namespace inet */
