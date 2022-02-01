//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETSINK_H
#define __INET_IPACKETSINK_H

#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet sinks which can be both passive
 * and active.
 */
class INET_API IPacketSink : public virtual IPassivePacketSink, public virtual IActivePacketSink
{
};

} // namespace queueing
} // namespace inet

#endif

