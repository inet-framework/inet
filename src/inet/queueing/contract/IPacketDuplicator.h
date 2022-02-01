//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETDUPLICATOR_H
#define __INET_IPACKETDUPLICATOR_H

#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet duplicators.
 */
class INET_API IPacketDuplicator : public virtual IPassivePacketSink, public virtual IActivePacketSource
{
  public:
    /**
     * Returns the number of duplicates to be generated.
     */
    virtual int getNumPacketDuplicates(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif

