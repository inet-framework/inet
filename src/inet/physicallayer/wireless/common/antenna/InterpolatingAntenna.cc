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

#include "inet/physicallayer/wireless/common/antenna/InterpolatingAntenna.h"

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
        auto elevationGains = check_and_cast<cValueMap*>(par("elevationGains").objectValue());
        auto headingGains = check_and_cast<cValueMap*>(par("headingGains").objectValue());
        auto bankGains = check_and_cast<cValueMap*>(par("bankGains").objectValue());
        gain = makeShared<AntennaGain>(this, elevationGains, headingGains, bankGains);
    }
}

std::ostream& InterpolatingAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "InterpolatingAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(maxGain, gain->getMaxGain());
    return AntennaBase::printToStream(stream, level);
}

InterpolatingAntenna::AntennaGain::AntennaGain(cModule *module, const cValueMap *elevation, const cValueMap *heading, const cValueMap *bank) :
    module(module), minGain(NaN), maxGain(NaN)
{
    elevationGainMap = parseMap(elevation);
    headingGainMap = parseMap(heading);
    bankGainMap = parseMap(bank);
}

std::map<rad, double> InterpolatingAntenna::AntennaGain::parseMap(const cValueMap *gains)
{
    std::map<rad, double> gainMap;

    for (const auto& elem: gains->getFields()) {
        cDynamicExpression angleExpression;
        angleExpression.parse(elem.first.c_str());
        rad angle = rad(angleExpression.doubleValue(module, "rad"));
        double gain = math::dB2fraction(elem.second.doubleValueInUnit("dB"));
        if (std::isnan(minGain) || gain < minGain)
            minGain = gain;
        if (std::isnan(maxGain) || gain > maxGain)
            maxGain = gain;
        gainMap.insert(std::pair<rad, double>(angle, gain));
    }

    auto it = gainMap.find(rad(0.0));
    if (it == gainMap.end())
        throw cRuntimeError("The angle 0 must be specified");
    else
        gainMap.insert(std::pair<rad, double>(rad(2 * M_PI), it->second));

    return gainMap;
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

double InterpolatingAntenna::AntennaGain::computeGain(const Quaternion& direction) const
{
    auto eulerAngles = direction.toEulerAngles();
    return computeGain(headingGainMap, eulerAngles.alpha) *
           computeGain(elevationGainMap, eulerAngles.beta) *
           computeGain(bankGainMap, eulerAngles.gamma);
}

} // namespace physicallayer

} // namespace inet

