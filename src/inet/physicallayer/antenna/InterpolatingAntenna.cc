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
    AntennaBase(),
    minGain(NaN),
    maxGain(NaN)
{
}

void InterpolatingAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        parseMap(elevationGainMap, par("elevationGains"));
        parseMap(headingGainMap, par("headingGains"));
        parseMap(bankGainMap, par("bankGains"));
    }
}

void InterpolatingAntenna::printToStream(std::ostream& stream) const
{
    stream << "InterpolatingAntenna, maxGain = " << maxGain;
}

void InterpolatingAntenna::parseMap(std::map<double, double>& gainMap, const char *text)
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
    gainMap.insert(std::pair<double, double>(0, math::dB2fraction(atof(firstGain))));
    gainMap.insert(std::pair<double, double>(2 * M_PI, math::dB2fraction(atof(firstGain))));
    while (tokenizer.hasMoreTokens()) {
        const char *angleString = tokenizer.nextToken();
        const char *gainString = tokenizer.nextToken();
        if (!angleString || !gainString)
            throw cRuntimeError("Insufficient number of values");
        double angle = atof(angleString) * M_PI / 180;
        double gain = math::dB2fraction(atof(gainString));
        if (isNaN(minGain) || gain < minGain)
            minGain = gain;
        if (isNaN(maxGain) || gain > maxGain)
            maxGain = gain;
        gainMap.insert(std::pair<double, double>(angle, gain));
    }
}

double InterpolatingAntenna::computeGain(const std::map<double, double>& gainMap, double angle) const
{
    ASSERT(0 <= angle && angle <= 2 * M_PI);
    // NOTE: 0 and 2 * M_PI are always in the map
    std::map<double, double>::const_iterator lowerBound = gainMap.lower_bound(angle);
    std::map<double, double>::const_iterator upperBound = gainMap.upper_bound(angle);
    if (lowerBound->first != angle)
        lowerBound--;
    if (upperBound == gainMap.end())
        upperBound--;
    if (upperBound == lowerBound)
        return lowerBound->second;
    else {
        double lowerAngle = lowerBound->first;
        double upperAngle = upperBound->first;
        double lowerGain = lowerBound->second;
        double upperGain = upperBound->second;
        double alpha = (angle - lowerAngle) / (upperAngle - lowerAngle);
        return (1 - alpha) * lowerGain + alpha * upperGain;
    }
}

double InterpolatingAntenna::computeGain(EulerAngles direction) const
{
    return computeGain(headingGainMap, direction.alpha) *
           computeGain(elevationGainMap, direction.beta) *
           computeGain(bankGainMap, direction.gamma);
}

} // namespace physicallayer

} // namespace inet

