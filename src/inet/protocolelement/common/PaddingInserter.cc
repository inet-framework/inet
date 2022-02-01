//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/common/PaddingInserter.h"

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Define_Module(PaddingInserter);

void PaddingInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minLength = b(par("minLength"));
        roundingLength = b(par("roundingLength"));
        insertionPosition = parseHeaderPosition(par("insertionPosition"));
    }
}

void PaddingInserter::processPacket(Packet *packet)
{
    auto length = packet->getTotalLength();
    b paddingLength = roundingLength * std::ceil(b(length).get() / b(roundingLength).get()) - length;
    if (length + paddingLength < minLength)
        paddingLength = minLength - length;
    if (paddingLength > b(0)) {
        if (b(paddingLength).get() % 8 == 0) {
            const auto& padding = makeShared<ByteCountChunk>(paddingLength, 0);
            insertHeader<ByteCountChunk>(packet, padding, insertionPosition);
        }
        else {
            const auto& padding = makeShared<BitCountChunk>(paddingLength);
            insertHeader<BitCountChunk>(packet, padding, insertionPosition);
        }
    }
}

} // namespace inet

