//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_APPSOCKET_H_
#define INET_APPLICATIONS_QUIC_APPSOCKET_H_

#include "Connection.h"
#include "UdpSocket.h"
#include "Quic.h"

namespace inet {
namespace quic {

// Forward declarations:
class Connection;
class UdpSocket;
class Quic;

class AppSocket {
public:
    AppSocket(Quic *quicSimpleMod, int socketId);
    virtual ~AppSocket();

    virtual void processAppCommand(cMessage *msg);
    virtual void sendEstablished();
    virtual void sendData(Ptr<const Chunk> data);
    virtual void sendIndication(Indication *indication);
    virtual void sendIndications(std::list<Indication *> indications);
    virtual void sendPacket(Packet *pkt);
    virtual void sendSendQueueFull();
    virtual void sendSendQueueDrain();
    virtual void sendMsgRejected();
    virtual void sendConnectionAvailable();
    virtual void sendClosed();
    virtual void sendDestroyed();
    virtual void sendDataNotification(uint64_t streamId, uint64_t dataSize);
    virtual void sendToken(std::string clientToken);

    Connection *getConnection() {
        return this->connection;
    }
    UdpSocket *getUdpSocket() {
        return this->udpSocket;
    }
    void setConnection(Connection *connection) {
        this->connection = connection;
    }
    void setUdpSocket(UdpSocket *udpSocket) {
        this->udpSocket = udpSocket;
    }
    int getSocketId() {
        return socketId;
    }

private:
    int socketId;
    Quic *quicSimpleMod;
    Connection *connection = nullptr;
    UdpSocket *udpSocket = nullptr;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_APPSOCKET_H_ */
