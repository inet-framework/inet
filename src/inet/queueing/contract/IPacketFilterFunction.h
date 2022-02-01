//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETFILTERFUNCTION_H
#define __INET_IPACKETFILTERFUNCTION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet filter functions.
 */
class INET_API IPacketFilterFunction
{
  public:
    virtual ~IPacketFilterFunction() {}

    /**
     * Returns true if the filter matches the given packet.
     */
    virtual bool matchesPacket(const Packet *packet) const = 0;
};

} // namespace queueing
} // namespace inet

#endif

