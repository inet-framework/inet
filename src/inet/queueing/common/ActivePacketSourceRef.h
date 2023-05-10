//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSOURCEREF_H
#define __INET_ACTIVEPACKETSOURCEREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSourceRef : public ModuleRefByGate<IActivePacketSource>
{
};

} // namespace queueing
} // namespace inet

#endif

