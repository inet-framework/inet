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

#include "inet/physicallayer/base/packetlevel/SnirBase.h"

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

