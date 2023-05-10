//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSINKREF_H
#define __INET_ACTIVEPACKETSINKREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSinkRef : public ModuleRefByGate<IActivePacketSink>
{
};

} // namespace queueing
} // namespace inet

#endif

