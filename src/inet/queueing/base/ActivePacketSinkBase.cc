//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/ActivePacketSinkBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void ActivePacketSinkBase::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(inputGate);
}

} // namespace queueing
} // namespace inet

