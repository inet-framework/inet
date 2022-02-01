//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/ParabolicAntenna.h"

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

std::ostream& ParabolicAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ParabolicAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(maxGain, gain->getMaxGain())
               << EV_FIELD(minGain, gain->getMinGain())
               << EV_FIELD(beamWidth, gain->getBeamWidth());
    return AntennaBase::printToStream(stream, level);
}

ParabolicAntenna::AntennaGain::AntennaGain(double maxGain, double minGain, deg beamWidth) :
    maxGain(maxGain), minGain(minGain), beamWidth(beamWidth)
{
}

double ParabolicAntenna::AntennaGain::computeGain(const Quaternion& direction) const
{
    double product = math::minnan(1.0, math::maxnan(-1.0, direction.rotate(Coord::X_AXIS) * Coord::X_AXIS));

    deg alpha = rad(std::acos(product));
    ASSERT(deg(0) <= alpha && alpha <= deg(360));
    if (alpha > deg(180))
        alpha = deg(360) - alpha;
    return math::maxnan(minGain, maxGain * math::dB2fraction(-12 * pow(unit(alpha / beamWidth).get(), 2)));
}

} // namespace physicallayer

} // namespace inet

