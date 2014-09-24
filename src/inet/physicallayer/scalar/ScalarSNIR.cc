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

#include "inet/physicallayer/scalar/ScalarSNIR.h"

namespace inet {

namespace physicallayer {

ScalarSNIR::ScalarSNIR(const ScalarReception *reception, const ScalarNoise *noise) :
    reception(reception),
    noise(noise)
{
}

void ScalarSNIR::printToStream(std::ostream& stream) const
{
    stream << "scalar SNIR";
}

double ScalarSNIR::computeMin() const
{
    return unit(reception->getPower() / noise->computeMaxPower(reception->getStartTime(), reception->getEndTime())).get();
}

} // namespace physicallayer

} // namespace inet

