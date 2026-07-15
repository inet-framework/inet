//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IL3PROTOCOL_H
#define __INET_IL3PROTOCOL_H

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class INET_API IL3Protocol
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
        virtual void handleClosed() = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void bind(int socketId, const Protocol *protocol, const L3Address& localAddress) = 0;
    virtual void connect(int socketId, const L3Address& remoteAddress) = 0;
    virtual void close(int socketId) = 0;
    virtual void destroy(int socketId) = 0;
};

} // namespace inet

#endif

