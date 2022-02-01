//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PassivePacketSinkBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PassivePacketSinkBase::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(inputGate);
}

void PassivePacketSinkBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

} // namespace queueing
} // namespace inet

