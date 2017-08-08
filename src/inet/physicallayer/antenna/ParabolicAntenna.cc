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
        parameters.maxGain = math::dB2fraction(par("maxGain"));
        parameters.minGain = math::dB2fraction(par("minGain"));
        parameters.beamWidth = degree(par("beamWidth"));
    }
}

std::ostream& ParabolicAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "ParabolicAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", maxGain = " << parameters.maxGain
               << ", minGain = " << parameters.minGain
               << ", beamWidth = " << parameters.beamWidth;
    return AntennaBase::printToStream(stream, level);
}

std::shared_ptr<IAntennaSnapshot> ParabolicAntenna::createSnapshot()
{
    return std::make_shared<Snapshot>(parameters);
}

double ParabolicAntenna::Snapshot::computeGain(const EulerAngles direction) const
{
    return std::max(parameters.minGain, parameters.maxGain * math::dB2fraction(-12 * pow(direction.alpha / math::deg2rad(parameters.beamWidth.get()), 2)));
}

} // namespace physicallayer

} // namespace inet

