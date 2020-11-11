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

#include "inet/physicallayer/wireless/common/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

SnirBase::SnirBase(const IReception *reception, const INoise *noise) :
    reception(reception),
    noise(noise)
{
}

std::ostream& SnirBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(reception, printFieldToString(reception, level + 1, evFlags))
               << EV_FIELD(noise, printFieldToString(noise, level + 1, evFlags));
    return stream;
}

} // namespace physicallayer

} // namespace inet

