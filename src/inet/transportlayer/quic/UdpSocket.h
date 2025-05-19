//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    virtual bool match(L3Address addr, uint16_t port);
    virtual void sendto(L3Address remoteAddr, uint16_t remotePort, Packet *pkt);
    virtual Connection *popConnection();
    virtual void bind(L3Address addr, uint16_t port);
    virtual void listen(AppSocket *appSocket);
    virtual void unlisten();
    virtual void destroy();
    virtual bool isListening();
    virtual void saveToken(uint32_t token, L3Address remoteAddr);
    virtual bool doesTokenExist(uint32_t token, L3Address remoteAddr);

    L3Address getLocalAddr() {
        return localAddr;
    }
    uint16_t getLocalPort() {
        return localPort;
    }
    std::map<uint32_t, L3Address> tokenRemoteIpMap;

  private:
    inet::UdpSocket socket;
    L3Address localAddr;
    uint16_t localPort = 0;
    Quic *quicSimpleMod = nullptr;
    AppSocket *listeningAppSocket = nullptr;
    std::queue<Connection *> connectionQueue;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_UDPSOCKET_H_ */
