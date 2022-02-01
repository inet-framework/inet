//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETMARKER_H
#define __INET_IPACKETMARKER_H

#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet markers.
 */
class INET_API IPacketMarker : public virtual IPacketFlow
{
  public:
    /**
     * Marks the packet by attaching information to it.
     */
    virtual void markPacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif

