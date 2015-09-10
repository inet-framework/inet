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

#include "inet/physicallayer/antenna/ConstantGainAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantGainAntenna);

ConstantGainAntenna::ConstantGainAntenna() :
    AntennaBase(),
    gain(NaN)
{
}

void ConstantGainAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        gain = math::dB2fraction(par("gain"));
}

std::ostream& ConstantGainAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "ConstantGainAntenna";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", gain = " << gain;
    return AntennaBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

