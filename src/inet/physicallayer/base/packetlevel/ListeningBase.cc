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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/physicallayer/base/packetlevel/ListeningBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

ListeningBase::ListeningBase(const IRadio *receiver, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
    receiver(receiver),
    startTime(startTime),
    endTime(endTime),
    startPosition(startPosition),
    endPosition(endPosition)
{
}

std::ostream& ListeningBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(receiverId, receiver->getId())
               << EV_FIELD(startTime)
               << EV_FIELD(endTime)
               << EV_FIELD(startPosition)
               << EV_FIELD(endPosition);
    return stream;
}

} // namespace physicallayer

} // namespace inet

