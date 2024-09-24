//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IETHERNET_H
#define __INET_IETHERNET_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class INET_API IEthernet
{
  public:
    virtual void bind(int socketId, int interfaceId, const MacAddress& localAddress, const MacAddress& remoteAddress, const Protocol *protocol, bool steal) = 0;
};

} // namespace inet

#endif

