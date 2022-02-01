//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETPULLER_H
#define __INET_IPACKETPULLER_H

#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet pullers.
 */
class INET_API IPacketPuller : public virtual IActivePacketSink, public virtual IPassivePacketSource
{
};

} // namespace queueing
} // namespace inet

#endif

