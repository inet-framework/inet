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
    AntennaBase()
{
}

void CosineAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        double maxGain = math::dB2fraction(par("maxGain"));
        deg beamWidth = deg(par("beamWidth"));
        gain = makeShared<AntennaGain>(maxGain, beamWidth);
    }
}

std::ostream& CosineAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "CosineAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", maxGain = " << gain->getMaxGain()
               << ", beamWidth = " << gain->getBeamWidth();
    return AntennaBase::printToStream(stream, level);
}

CosineAntenna::AntennaGain::AntennaGain(double maxGain, deg beamWidth) :
    maxGain(maxGain), beamWidth(beamWidth)
{
}

double CosineAntenna::AntennaGain::computeGain(const Quaternion direction) const
{
    double exponent = -3.0 / (20 * std::log10(std::cos(math::deg2rad(beamWidth.get()) / 4.0)));
    double angle = std::acos(direction.rotate(Coord::X_AXIS) * Coord::X_AXIS);
    return maxGain * std::pow(std::abs(std::cos(angle / 2.0)), exponent);
}

} // namespace physicallayer

} // namespace inet

