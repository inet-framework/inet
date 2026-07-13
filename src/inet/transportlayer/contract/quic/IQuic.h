//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IQUIC_H
#define __INET_IQUIC_H

#include "inet/networklayer/common/L3Address.h"

namespace inet {
namespace quic {

class INET_API IQuic
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}
        virtual void handleMessage(cMessage *msg) = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void bind(int socketId, const L3Address& localAddr, uint16_t localPort) = 0;
    virtual void listen(int socketId) = 0;
    virtual void connect(int socketId, const L3Address& remoteAddr, uint16_t remotePort) = 0;
    virtual void accept(int socketId, int newSocketId) = 0;
    virtual void recv(int socketId, uint64_t streamId, int64_t expectedDataSize) = 0;
    virtual void close(int socketId) = 0;
};

} // namespace quic
} // namespace inet

#endif
