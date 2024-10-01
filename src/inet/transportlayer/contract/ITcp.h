//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITCP_H
#define __INET_ITCP_H

#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

namespace inet {
namespace tcp {

class INET_API ITcp
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
        virtual void handleEstablished(Indication *indication) = 0;
        virtual void handleAvailable(TcpAvailableInfo *availableInfo) = 0;
        virtual void handleClosed() = 0;
        virtual void handlePeerClosed() = 0;
        virtual void handleFailure(int code) = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void listen(int socketId, const L3Address& localAddr, int localPrt, bool fork, bool autoRead, std::string tcpAlgorithmClass) = 0;
    virtual void connect(int socketId, const L3Address& localAddr, int localPort, const L3Address& remoteAddr, int remotePort, bool autoRead, std::string tcpAlgorithmClass) = 0;
    virtual void accept(int socketId) = 0;
    virtual void close(int socketId) = 0;
    virtual void abort(int socketId) = 0;
    virtual void setTimeToLive(int socketId, int ttl) = 0;
    virtual void setQueueLimits(int socketId, int packetCapacity, B dataCapacity) = 0;
};

} // namespace tcp
} // namespace inet

#endif

