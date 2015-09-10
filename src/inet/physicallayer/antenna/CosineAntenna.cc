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

#include "inet/physicallayer/antenna/CosineAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(CosineAntenna);

CosineAntenna::CosineAntenna() :
    AntennaBase(),
    maxGain(NaN),
    beamWidth(NaN)
{
}

void CosineAntenna::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        maxGain = math::dB2fraction(par("maxGain"));
        beamWidth = degree(par("beamWidth"));
    }
}

std::ostream& CosineAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "CosineAntenna";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", maxGain = " << maxGain
               << ", beamWidth = " << beamWidth;
    return AntennaBase::printToStream(stream, level);
}

double CosineAntenna::computeGain(const EulerAngles direction) const
{
    double exponent = -3.0 / (20 * std::log10(std::cos(math::deg2rad(beamWidth.get()) / 4.0)));
    return maxGain * std::pow(std::cos(direction.alpha / 2.0), exponent);
}

} // namespace physicallayer

} // namespace inet

