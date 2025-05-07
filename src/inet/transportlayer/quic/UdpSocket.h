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

#ifndef INET_APPLICATIONS_QUIC_UDPSOCKET_H_
#define INET_APPLICATIONS_QUIC_UDPSOCKET_H_

#include "Quic.h"
#include "AppSocket.h"
#include "Connection.h"
#include <queue>

namespace inet {
namespace quic {

// Forward declarations:
class Quic;
class AppSocket;
class Connection;

class UdpSocket {
  public:
    UdpSocket(Quic *quicSimpleMod);
    virtual ~UdpSocket();
    virtual void processPacket(Packet *pkt);
    virtual int getSocketId();
    virtual bool match(L3Address addr, int port);
    virtual void sendto(L3Address remoteAddr, int remotePort, Packet *pkt);
    virtual Connection *popConnection();
    virtual void bind(L3Address addr, int port);
    virtual void listen(AppSocket *appSocket);
    virtual void unlisten();

    L3Address getLocalAddr() {
        return localAddr;
    }
    int getLocalPort() {
        return localPort;
    }

  private:
    inet::UdpSocket socket;
    L3Address localAddr;
    uint16_t localPort = 0;
    Quic *quicSimpleMod = nullptr;
    bool isListening = false;
    AppSocket *listeningAppSocket = nullptr;
    std::queue<Connection *> connectionQueue;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_UDPSOCKET_H_ */
