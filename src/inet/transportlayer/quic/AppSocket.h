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
