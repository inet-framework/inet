//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETMETER_H
#define __INET_IPACKETMETER_H

#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet meters.
 */
class INET_API IPacketMeter : public virtual IPacketFlow
{
  public:
    /**
     * Meters the packet and attaches the result.
     */
    virtual void meterPacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif

