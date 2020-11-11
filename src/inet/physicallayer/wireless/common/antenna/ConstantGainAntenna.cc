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

#include "inet/physicallayer/wireless/common/antenna/ConstantGainAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantGainAntenna);

ConstantGainAntenna::ConstantGainAntenna() :
    AntennaBase()
{
}

void ConstantGainAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        gain = makeShared<AntennaGain>(math::dB2fraction(par("gain")));
}

std::ostream& ConstantGainAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ConstantGainAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(gain, gain->getMaxGain());
    return AntennaBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

