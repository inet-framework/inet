//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "AppSocket.h"
#include "QueueAppSocket.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "inet/common/socket/SocketTag_m.h"

namespace inet {
namespace quic {

AppSocket::AppSocket(Quic *quicSimpleMod, int socketId) : socketId(socketId), quicSimpleMod(quicSimpleMod) { }

AppSocket::~AppSocket() { }

void AppSocket::sendEstablished()
{
    EV_INFO << "Notifying app: quic established" << endl;
    auto indication = new Indication("quic established", QUIC_I_ESTABLISHED);
    QuicConnectionInfo *quicConnectionInfo = new QuicConnectionInfo();
    quicConnectionInfo->setLocalAddr(connection->getPath()->getLocalAddr());
    quicConnectionInfo->setRemoteAddr(connection->getPath()->getRemoteAddr());
    quicConnectionInfo->setLocalPort(connection->getPath()->getLocalPort());
    quicConnectionInfo->setRemotePort(connection->getPath()->getRemotePort());
    indication->setControlInfo(quicConnectionInfo);
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

void AppSocket::sendConnectionAvailable()
{
    auto indication = new Indication("quic connection available", QUIC_I_CONNECTION_AVAILABLE);
    sendIndication(indication);
}

void AppSocket::sendClosed()
{
    auto indication = new Indication("quic connection closed", QUIC_I_CLOSED);
    sendIndication(indication);
}

void AppSocket::sendDestroyed()
{
    auto indication = new Indication("socket destroyed", QUIC_I_DESTROYED);
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
    auto indication = new Indication("stream: data is ready for app", QUIC_I_DATA_AVAILABLE);

    QuicDataInfo *ctrInfo = new QuicDataInfo();
    ctrInfo->setStreamID(streamId);
    ctrInfo->setAvaliableDataSize(dataSize);
    indication->setControlInfo(ctrInfo);
    sendIndication(indication);
}

void AppSocket::sendToken(std::string clientToken)
{
    auto indication = new Indication("client token", QUIC_I_NEW_TOKEN);
    QuicNewToken *quicNewToken = new QuicNewToken();
    quicNewToken->setToken(clientToken.c_str());
    indication->setControlInfo(quicNewToken);
    sendIndication(indication);
}

void AppSocket::sendIndication(Indication *indication)
{
    indication->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
    this->quicSimpleMod->send(indication, "appOut");
}

void AppSocket::sendIndications(std::list<Indication *> indications)
{
    for (Indication *indication : indications) {
        sendIndication(indication);
    }
}

void AppSocket::sendPacket(Packet *pkt)
{
    pkt->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
    this->quicSimpleMod->send(pkt, "appOut");
}

void AppSocket::processAppCommand(cMessage *msg)
{
    if (!udpSocket) {
        // no udpSocket for AppSocket, find udpSocket by src addr/port for a bind
        if (msg->getKind() == QUIC_C_CREATE_PCB) { // bind
            QuicBindCommand *quicBind = check_and_cast<QuicBindCommand *>(msg->getControlInfo());
            L3Address localAddr = quicBind->getLocalAddr();
            uint16_t localPort = quicBind->getLocalPort();
            udpSocket = quicSimpleMod->findUdpSocket(localAddr, localPort);
        }
        // if there is no existing udpSocket, create one
        if (!udpSocket) {
            udpSocket = quicSimpleMod->createUdpSocket();
        }
    }
    switch (msg->getKind()) {
        case QUIC_C_CREATE_PCB: { // bind
            QuicBindCommand *quicBind = check_and_cast<QuicBindCommand *>(msg->getControlInfo());
            udpSocket->bind(quicBind->getLocalAddr(), quicBind->getLocalPort());
            break;
        }
        case QUIC_C_OPEN_PASSIVE: { // listen
            udpSocket->listen(this);
            break;
        }
        case QUIC_C_OPEN_ACTIVE: { // connect
            QuicOpenCommand *quicOpen = check_and_cast<QuicOpenCommand *>(msg->getControlInfo());
            L3Address remoteAddr = quicOpen->getRemoteAddr();
            uint16_t remotePort = quicOpen->getRemotePort();

            quicSimpleMod->createConnection(udpSocket, this, remoteAddr, remotePort);
            connection->processAppCommand(msg);
            break;
        }
        case QUIC_C_CONNECT_AND_SEND: { // 0-RTT connection setup
            Packet *pkt = check_and_cast<Packet *>(msg);
            Ptr<const QuicOpenCommand> quicOpen = pkt->getTag<QuicOpenCommand>();
            L3Address remoteAddr = quicOpen->getRemoteAddr();
            uint16_t remotePort = quicOpen->getRemotePort();

            quicSimpleMod->createConnection(udpSocket, this, remoteAddr, remotePort);
            connection->processAppCommand(msg);
            break;
        }
        case QUIC_C_ACCEPT: { // accept
            // get available connection from listening UDP socket
            Connection *acceptConnection = udpSocket->popConnection();
            if (acceptConnection == nullptr) {
                throw cRuntimeError("AppSocket::processAppCommand: No connection to accept available");
            }

            // get AppSocket from new socket ID within ACCEPT request and set connection
            QuicAcceptCommand *quicAccept = check_and_cast<QuicAcceptCommand *>(msg->getControlInfo());
            int newSocketId = quicAccept->getNewSocketId();
            EV_DEBUG << "AppSocket::processAppCommand: Got accept with new socket id " << newSocketId << endl;
            AppSocket *newAppSocket = quicSimpleMod->findOrCreateAppSocket(newSocketId);
            newAppSocket->setConnection(acceptConnection);

            // replace QueueAppSocket with the real AppSocket and send all queued indications over it
            QueueAppSocket *queueAppSocket = check_and_cast<QueueAppSocket *>(acceptConnection->getAppSocket());
            acceptConnection->setAppSocket(newAppSocket);
            newAppSocket->sendIndications(queueAppSocket->getIndications());
            delete queueAppSocket;

            break;
        }
        case QUIC_C_CLOSE: { // close (listening socket)
            udpSocket->unlisten();
            sendClosed();
            break;
        }
        default: {
            throw cRuntimeError("Unexpected/unknown App Command");
        }

    }
}

} /* namespace quic */
} /* namespace inet */
