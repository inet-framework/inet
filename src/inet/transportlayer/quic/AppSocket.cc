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

#include "AppSocket.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "inet/common/socket/SocketTag_m.h"

namespace inet {
namespace quic {

AppSocket::AppSocket(Quic *quicSimpleMod, int socketId) {
    this->quicSimpleMod = quicSimpleMod;
    this->socketId = socketId;
    connection = nullptr;
    udpSocket = nullptr;
}

AppSocket::~AppSocket() {
    // TODO Auto-generated destructor stub
}

void AppSocket::sendEstablished()
{
    EV_INFO << "Notifying app: quic established" << endl;
    auto indication = new Indication("quic established", QUIC_I_ESTABLISHED);
    QuicConnectInfo *quicConnectInfo = new QuicConnectInfo();
    quicConnectInfo->setLocalAddr(connection->getPath()->getLocalAddr());
    quicConnectInfo->setRemoteAddr(connection->getPath()->getRemoteAddr());
    quicConnectInfo->setLocalPort(connection->getPath()->getLocalPort());
    quicConnectInfo->setRemotePort(connection->getPath()->getRemotePort());
    indication->setControlInfo(quicConnectInfo);
    sendIndication(indication);
}

void AppSocket::sendSendQueueFull()
{
    auto indication = new Indication("quic send queue full", QUIC_I_SENDQUEUE_FULL);
    sendIndication(indication);
}

void AppSocket::sendSendQueueDrain()
{
    auto indication = new Indication("quic send queue drain", QUIC_I_SENDQUEUE_DRAIN);
    sendIndication(indication);
}

void AppSocket::sendMsgRejected()
{
    auto indication = new Indication("quic msg rejected", QUIC_I_MSG_REJECTED);
    sendIndication(indication);
}

void AppSocket::sendData(Ptr<const Chunk> data)
{
    Packet *pkt = new Packet("data");
    pkt->setKind(QUIC_I_DATA);
    pkt->insertAtBack(data);
    sendPacket(pkt);
}

void AppSocket::sendDataNotification(uint64_t streamId, uint64_t dataSize)
{
    EV_INFO << "Notifying app: quic has data ready to read" << endl;
    auto indication = new Indication("stream: data is ready for app", QUIC_I_DATA_NOTIFICATION);

    QuicDataAvailableInfo *ctrInfo = new QuicDataAvailableInfo();
    ctrInfo->setStreamID(streamId);
    ctrInfo->setAvaliableDataSize(dataSize);
    indication->setControlInfo(ctrInfo);
    sendIndication(indication);
}

void AppSocket::sendIndication(Indication *indication)
{
    indication->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
    this->quicSimpleMod->send(indication, "appOut");
}

void AppSocket::sendPacket(Packet *pkt)
{
    pkt->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
    this->quicSimpleMod->send(pkt, "appOut");
}

} /* namespace quic */
} /* namespace inet */
