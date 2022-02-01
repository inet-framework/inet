//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/AxiallySymmetricAntenna.h"

#include "inet/common/stlutils.h"

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

std::ostream& AxiallySymmetricAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
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
    if (!containsKey(gainMap, deg(0)))
        throw cRuntimeError("The first angle must be 0");
    if (!containsKey(gainMap, deg(180)))
        throw cRuntimeError("The last angle must be 180");
}

double AxiallySymmetricAntenna::AntennaGain::computeGain(const Quaternion& direction) const
{
    double product = math::minnan(1.0, math::maxnan(-1.0, direction.rotate(Coord::X_AXIS) * Coord::X_AXIS));
    rad angle = rad(std::acos(product));
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
        double gain = (1 - alpha) * lowerGain + alpha * upperGain;
        ASSERT(!std::isnan(gain));
        return gain;
    }
}

} // namespace physicallayer

} // namespace inet

