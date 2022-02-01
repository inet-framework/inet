//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETCLASSIFIERFUNCTION_H
#define __INET_IPACKETCLASSIFIERFUNCTION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet classifier functions.
 */
class INET_API IPacketClassifierFunction
{
  public:
    virtual ~IPacketClassifierFunction() {}

    /**
     * Returns the class index of the given packet.
     */
    virtual int classifyPacket(Packet *packet) const = 0;
};

} // namespace queueing
} // namespace inet

#endif

