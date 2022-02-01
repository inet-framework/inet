//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETFILTER_H
#define __INET_IPACKETFILTER_H

#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet filters.
 */
class INET_API IPacketFilter : public virtual IPacketFlow
{
  public:
    /**
     * Returns true if the filter matches the given packet.
     */
    virtual bool matchesPacket(const Packet *packet) const = 0;
};

} // namespace queueing
} // namespace inet

#endif

