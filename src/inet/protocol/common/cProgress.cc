//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/protocol/common/cProgress.h"

namespace inet {

Register_Class(cProgress);

void cProgress::copy(const cProgress& orig)
{
    if (orig.packet) {
        packet = orig.packet->dup();
        take(packet);
    }
    else
        packet = nullptr;
    bitPosition = orig.bitPosition;
    timePosition = orig.timePosition;
    extraProcessableBitLength = orig.extraProcessableBitLength;
    extraProcessableDuration = orig.extraProcessableDuration;
}

void cProgress::parsimPack(cCommBuffer *buffer) const
{
#ifndef WITH_PARSIM
    throw cRuntimeError(this, E_NOPARSIM);
#else
    cMessage::parsimPack(buffer);
    buffer->packObject(packet);
    buffer->pack(datarate);
    buffer->pack(bitPosition);
    buffer->pack(timePosition);
    buffer->pack(extraProcessableBitLength);
    buffer->pack(extraProcessableDuration);
#endif
}

void cProgress::parsimUnpack(cCommBuffer *buffer)
{
#ifndef WITH_PARSIM
    throw cRuntimeError(this, E_NOPARSIM);
#else
    cMessage::parsimUnpack(buffer);
    packet = check_and_cast<cPacket *>(buffer->unpackObject());
    take(packet);
    buffer->unpack(datarate);
    buffer->unpack(bitPosition);
    buffer->unpack(timePosition);
    buffer->unpack(extraProcessableBitLength);
    buffer->unpack(extraProcessableDuration);
#endif
}

cPacket *cProgress::getPacket() const
{
    return packet;
}

cPacket *cProgress::removePacket()
{
    auto p = packet;
    packet = nullptr;
    drop(p);
    return p;
}

void cProgress::setPacket(cPacket *newPacket)
{
    dropAndDelete(packet);
    packet = newPacket;
    if (packet != nullptr)
        take(packet);
}

} // namespace inet

