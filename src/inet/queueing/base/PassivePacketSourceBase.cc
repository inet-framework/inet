//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PassivePacketSourceBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PassivePacketSourceBase::initialize(int stage)
{
    PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = findConnectedModule<IActivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

} // namespace queueing
} // namespace inet

