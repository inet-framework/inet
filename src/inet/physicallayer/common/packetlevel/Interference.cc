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

#include "inet/physicallayer/common/packetlevel/Interference.h"

namespace inet {
namespace physicallayer {

Interference::Interference(const INoise *backgroundNoise, const std::vector<const IReception *> *interferingReceptions) :
    backgroundNoise(backgroundNoise),
    interferingReceptions(interferingReceptions)
{
}

Interference::~Interference()
{
    delete backgroundNoise;
    delete interferingReceptions;
}

std::ostream& Interference::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Interference";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(backgroundNoise, printFieldToString(backgroundNoise, level + 1, evFlags))
               << EV_FIELD(interferingReceptions, interferingReceptions );
    return stream;
}

} // namespace physicallayer
} // namespace inet

