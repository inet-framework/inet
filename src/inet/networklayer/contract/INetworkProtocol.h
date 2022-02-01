//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETWORKPROTOCOL_H
#define __INET_INETWORKPROTOCOL_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This purely virtual interface provides an abstraction for different network protocols.
 */
class INET_API INetworkProtocol
{
  public:
    virtual ~INetworkProtocol() {}
};

} // namespace inet

#endif

