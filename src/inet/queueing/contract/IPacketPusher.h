//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETPUSHER_H
#define __INET_IPACKETPUSHER_H

#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet pushers.
 */
class INET_API IPacketPusher : public virtual IPassivePacketSink, public virtual IActivePacketSource
{
};

} // namespace queueing
} // namespace inet

#endif

