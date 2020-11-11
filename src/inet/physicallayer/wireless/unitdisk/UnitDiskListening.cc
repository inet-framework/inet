//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/wireless/unitdisk/UnitDiskListening.h"

namespace inet {

namespace physicallayer {

UnitDiskListening::UnitDiskListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) :
    ListeningBase(radio, startTime, endTime, startPosition, endPosition)
{
}

std::ostream& UnitDiskListening::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskListening";
    return ListeningBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

