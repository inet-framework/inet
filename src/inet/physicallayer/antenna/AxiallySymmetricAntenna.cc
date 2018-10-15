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
        gain = makeShared<AntennaGain>(par("axisOfSymmetry"), par("gains"));
    }
}


std::ostream& AxiallySymmetricAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "AxiallySymmetricAntenna";
    return AntennaBase::printToStream(stream, level);
}

AxiallySymmetricAntenna::AntennaGain::AntennaGain(const char *axis, const char *gains) :
    minGain(NaN),
    maxGain(NaN)
{
    axisOfSymmetryDirection = Coord::parse(axis);
    cStringTokenizer tokenizer(gains);
    const char *firstAngle = tokenizer.nextToken();
    if (!firstAngle)
        throw cRuntimeError("Insufficient number of values");
    if (strcmp(firstAngle, "0"))
        throw cRuntimeError("The first angle must be 0");
    const char *firstGain = tokenizer.nextToken();
    if (!firstGain)
        throw cRuntimeError("Insufficient number of values");
    gainMap.insert(std::pair<rad, double>(rad(0), math::dB2fraction(atof(firstGain))));
    gainMap.insert(std::pair<rad, double>(rad(M_PI), math::dB2fraction(atof(firstGain))));
    while (tokenizer.hasMoreTokens()) {
        const char *angleString = tokenizer.nextToken();
        const char *gainString = tokenizer.nextToken();
        if (!angleString || !gainString)
            throw cRuntimeError("Insufficient number of values");
        auto angle = deg(atof(angleString));
        double gain = math::dB2fraction(atof(gainString));
        if (std::isnan(minGain) || gain < minGain)
            minGain = gain;
        if (std::isnan(maxGain) || gain > maxGain)
            maxGain = gain;
        gainMap.insert(std::pair<rad, double>(angle, gain));
    }
}

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

