//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETSOURCE_H
#define __INET_IPACKETSOURCE_H

#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet sources which can be both active
 * and passive.
 */
class INET_API IPacketSource : public virtual IPassivePacketSource, public virtual IActivePacketSource
{
};

} // namespace queueing
} // namespace inet

#endif

