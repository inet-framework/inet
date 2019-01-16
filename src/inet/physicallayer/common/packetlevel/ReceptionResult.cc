//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/common/packetlevel/ReceptionResult.h"

namespace inet {
namespace physicallayer {

ReceptionResult::ReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const Packet *packet) :
    reception(reception),
    decisions(decisions),
    packet(packet)
{
}

ReceptionResult::~ReceptionResult()
{
    delete packet;
    delete decisions;
}

std::ostream& ReceptionResult::printToStream(std::ostream& stream, int level) const
{
    stream << "ReceptionResult";
    return stream;
}

const Packet* ReceptionResult::getPacket() const
{
    return packet;
}

} // namespace physicallayer
} // namespace inet

