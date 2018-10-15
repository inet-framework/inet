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

#include "inet/physicallayer/antenna/ParabolicAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(ParabolicAntenna);

ParabolicAntenna::ParabolicAntenna() :
    AntennaBase()
{
}

void ParabolicAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        double maxGain = math::dB2fraction(par("maxGain"));
        double minGain = math::dB2fraction(par("minGain"));
        deg beamWidth = deg(par("beamWidth"));
        gain = makeShared<AntennaGain>(maxGain, minGain, beamWidth);
    }
}

std::ostream& ParabolicAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "ParabolicAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", maxGain = " << gain->getMaxGain()
               << ", minGain = " << gain->getMinGain()
               << ", beamWidth = " << gain->getBeamWidth();
    return AntennaBase::printToStream(stream, level);
}

ParabolicAntenna::AntennaGain::AntennaGain(double maxGain, double minGain, deg beamWidth) :
    maxGain(maxGain), minGain(minGain), beamWidth(beamWidth)
{
}

double ParabolicAntenna::AntennaGain::computeGain(const Quaternion direction) const
{
    deg alpha = rad(std::acos(direction.rotate(Coord::X_AXIS) * Coord::X_AXIS));
    ASSERT(deg(0) <= alpha && alpha <= deg(360));
    if (alpha > deg(180))
        alpha = deg(360) - alpha;
    return std::max(minGain, maxGain * math::dB2fraction(-12 * pow(unit(alpha / beamWidth).get(), 2)));
}

} // namespace physicallayer

} // namespace inet

