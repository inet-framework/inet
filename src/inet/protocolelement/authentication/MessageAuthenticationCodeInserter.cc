//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/authentication/MessageAuthenticationCodeInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(MessageAuthenticationCodeInserter);

void MessageAuthenticationCodeInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerLength = b(par("headerLength"));
    }
}

cGate *MessageAuthenticationCodeInserter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void MessageAuthenticationCodeInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<ByteCountChunk>(headerLength);
    packet->insertAtFront(header);
}

} // namespace inet

