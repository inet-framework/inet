//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

