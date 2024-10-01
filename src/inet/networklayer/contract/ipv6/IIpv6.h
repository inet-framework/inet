//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IIPV6_H
#define __INET_IIPV6_H

#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class INET_API IIpv6
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
  };

  public:
    virtual void bind(int socketId, const Protocol *protocol, Ipv6Address localAddress) = 0;
    virtual void connect(int socketId, const Ipv6Address& remoteAddress) = 0;
    virtual void close(int socketId) = 0;
    virtual void destroy(int socketId) = 0;
};

} // namespace inet

#endif

