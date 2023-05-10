//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCEREF_H
#define __INET_PASSIVEPACKETSOURCEREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSourceRef : public ModuleRefByGate<IPassivePacketSource>
{
};

} // namespace queueing
} // namespace inet

#endif

