//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"

namespace inet {

bool ProtocolHeaderInserterBase::canPushPacket(Packet *packet, cGate *gate) const
{
    auto copy = packet->dup();
    // TODO: KLUDGE: const_cast
    PacketFlowBase::processPacket(copy);
    return PacketFlowBase::canPushPacket(copy, gate);
}

} // namespace inet

