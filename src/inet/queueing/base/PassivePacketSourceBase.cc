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

#include "inet/queueing/base/PassivePacketSourceBase.h"

namespace inet {
namespace queueing {

Packet *PassivePacketSourceBase::pullPacketStart(cGate *gate)
{
    b position;
    b extraProcessableLength;
    auto packet = pullPacketProgress(gate, position, extraProcessableLength);
    if (position != b(0))
        throw cRuntimeError("Invalid position");
    return packet;
}

Packet *PassivePacketSourceBase::pullPacketEnd(cGate *gate)
{
    b position;
    b extraProcessableLength;
    auto packet = pullPacketProgress(gate, position, extraProcessableLength);
    if (position != packet->getTotalLength())
        throw cRuntimeError("Invalid position");
    if (extraProcessableLength != b(0))
        throw cRuntimeError("Invalid extraProcessableLength");
    return packet;

}

} // namespace queueing
} // namespace inet

