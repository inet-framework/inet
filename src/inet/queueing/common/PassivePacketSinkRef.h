//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSINKREF_H
#define __INET_PASSIVEPACKETSINKREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSinkRef : public ModuleRefByGate<IPassivePacketSink>
{
};

} // namespace queueing
} // namespace inet

#endif

