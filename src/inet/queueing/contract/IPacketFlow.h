//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETFLOW_H
#define __INET_IPACKETFLOW_H

#include "inet/queueing/contract/IPacketSink.h"
#include "inet/queueing/contract/IPacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet flows.
 */
class INET_API IPacketFlow : public virtual IPacketSink, public virtual IPacketSource
{
};

} // namespace queueing
} // namespace inet

#endif

