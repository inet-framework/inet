//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETDROPPERFUNCTION_H
#define __INET_IPACKETDROPPERFUNCTION_H

#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet dropper functions.
 */
class INET_API IPacketDropperFunction
{
  public:
    virtual ~IPacketDropperFunction() {}

    /**
     * Returns a packet to be dropped from the collection.
     */
    virtual Packet *selectPacket(IPacketCollection *collection) const = 0;
};

} // namespace queueing
} // namespace inet

#endif

