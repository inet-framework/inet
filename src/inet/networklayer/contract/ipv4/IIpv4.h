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
    virtual void bind(int socketId, const Protocol *protocol, Ipv4Address localAddress) = 0;
    virtual void connect(int socketId, const Ipv4Address& remoteAddress) = 0;
};

} // namespace inet

#endif

