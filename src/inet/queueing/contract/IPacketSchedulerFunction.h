//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETSCHEDULERFUNCTION_H
#define __INET_IPACKETSCHEDULERFUNCTION_H

#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet scheduler functions.
 */
class INET_API IPacketSchedulerFunction
{
  public:
    virtual ~IPacketSchedulerFunction() {}

    /**
     * Returns the index of the scheduled provider.
     */
    virtual int schedulePacket(const std::vector<IPassivePacketSource *>& sources) const = 0;
};

} // namespace queueing
} // namespace inet

#endif

