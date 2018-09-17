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

#include "inet/physicallayer/antenna/InterpolatingAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(InterpolatingAntenna);

InterpolatingAntenna::InterpolatingAntenna() :
    AntennaBase()
{
}

void InterpolatingAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        gain = makeShared<AntennaGain>(par("elevationGains"), par("headingGains"), par("bankGains"));
    }
}

std::ostream& InterpolatingAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "InterpolatingAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", maxGain = " << gain->getMaxGain();
    return AntennaBase::printToStream(stream, level);
}

InterpolatingAntenna::AntennaGain::AntennaGain(const char *elevation, const char *heading, const char *bank) :
    minGain(NaN), maxGain(NaN)
{
    parseMap(elevationGainMap, elevation);
    parseMap(headingGainMap, heading);
    parseMap(bankGainMap, bank);
}

void InterpolatingAntenna::AntennaGain::parseMap(std::map<rad, double>& gainMap, const char *text)
{
    cStringTokenizer tokenizer(text);
    const char *firstAngle = tokenizer.nextToken();
    if (!firstAngle)
        throw cRuntimeError("Insufficient number of values");
    if (strcmp(firstAngle, "0"))
        throw cRuntimeError("The first angle must be 0");
    const char *firstGain = tokenizer.nextToken();
    if (!firstGain)
        throw cRuntimeError("Insufficient number of values");
    gainMap.insert(std::pair<rad, double>(rad(0), math::dB2fraction(atof(firstGain))));
    gainMap.insert(std::pair<rad, double>(rad(2 * M_PI), math::dB2fraction(atof(firstGain))));
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

double InterpolatingAntenna::AntennaGain::computeGain(const std::map<rad, double>& gainMap, rad angle) const
{
    angle = rad(fmod(rad(angle).get(), 2 * M_PI));
    if (angle < rad(0.0)) angle += rad(2 * M_PI);
    // NOTE: 0 and 2 * M_PI are always in the map
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

double InterpolatingAntenna::AntennaGain::computeGain(Quaternion direction) const
{
    auto eulerAngles = direction.toEulerAngles();
    return computeGain(headingGainMap, eulerAngles.alpha) *
           computeGain(elevationGainMap, eulerAngles.beta) *
           computeGain(bankGainMap, eulerAngles.gamma);
}

} // namespace physicallayer

} // namespace inet

