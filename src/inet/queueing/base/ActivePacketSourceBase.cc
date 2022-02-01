//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/ActivePacketSourceBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void ActivePacketSourceBase::initialize(int stage)
{
    PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

} // namespace queueing
} // namespace inet

