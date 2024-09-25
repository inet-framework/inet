//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IIPV4_H
#define __INET_IIPV4_H

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class INET_API IIpv4
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
        virtual void handleClosed() = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void bind(int socketId, const Protocol *protocol, Ipv4Address localAddress) = 0;
    virtual void connect(int socketId, const Ipv4Address& remoteAddress) = 0;
    virtual void close(int socketId) = 0;
    virtual void destroy(int socketId) = 0;
};

} // namespace inet

#endif

