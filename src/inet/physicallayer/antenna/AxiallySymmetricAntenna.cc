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

#include "inet/physicallayer/antenna/AxiallySymmetricAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(AxiallySymmetricAntenna);

AxiallySymmetricAntenna::AxiallySymmetricAntenna() :
    AntennaBase()
{
}

void AxiallySymmetricAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        double baseGain = math::dB2fraction(par("baseGain"));
        gain = makeShared<AntennaGain>(par("axisOfSymmetry"), baseGain, par("gains"));
    }
}


std::ostream& AxiallySymmetricAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "AxiallySymmetricAntenna";
    return AntennaBase::printToStream(stream, level);
}

AxiallySymmetricAntenna::AntennaGain::AntennaGain(const char *axis, double baseGain, const char *gains) :
    minGain(NaN),
    maxGain(NaN)
{
    axisOfSymmetryDirection = Coord::parse(axis);
    cStringTokenizer tokenizer(gains);
    while (tokenizer.hasMoreTokens()) {
        const char *angleString = tokenizer.nextToken();
        const char *gainString = tokenizer.nextToken();
        if (!angleString || !gainString)
            throw cRuntimeError("Insufficient number of values");
        auto angle = deg(atof(angleString));
        double gain = baseGain * math::dB2fraction(atof(gainString));
        if (std::isnan(minGain) || gain < minGain)
            minGain = gain;
        if (std::isnan(maxGain) || gain > maxGain)
            maxGain = gain;
        gainMap.insert(std::pair<rad, double>(angle, gain));
    }
    if (gainMap.find(deg(0)) == gainMap.end())
        throw cRuntimeError("The first angle must be 0");
    if (gainMap.find(deg(180)) == gainMap.end())
        throw cRuntimeError("The last angle must be 180");}

double AxiallySymmetricAntenna::AntennaGain::computeGain(const Quaternion direction) const
{
    rad angle = rad(std::acos(direction.rotate(Coord::X_AXIS) * Coord::X_AXIS));
    // NOTE: 0 and M_PI are always in the map
    std::map<rad, double>::const_iterator lowerBound = gainMap.lower_bound(angle);
    std::map<rad, double>::const_iterator upperBound = gainMap.upper_bound(angle);
    if (lowerBound->first != angle)
        lowerBound--;
    if (upperBound == gainMap.end())
        upperBound--;
    if (upperBound == lowerBound)
        return lowerBound->second;
    else {
        auto lowerAngle = lowerBound->first;
        auto upperAngle = upperBound->first;
        double lowerGain = lowerBound->second;
        double upperGain = upperBound->second;
        double alpha = unit((angle - lowerAngle) / (upperAngle - lowerAngle)).get();
        return (1 - alpha) * lowerGain + alpha * upperGain;
    }
}

} // namespace physicallayer

} // namespace inet

