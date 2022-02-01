//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/CosineAntenna.h"

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

std::ostream& CosineAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "CosineAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(maxGain, gain->getMaxGain())
               << EV_FIELD(beamWidth, gain->getBeamWidth());
    return AntennaBase::printToStream(stream, level);
}

CosineAntenna::AntennaGain::AntennaGain(double maxGain, deg beamWidth) :
    maxGain(maxGain), beamWidth(beamWidth)
{
}

double CosineAntenna::AntennaGain::computeGain(const Quaternion& direction) const
{
    double exponent = -3.0 / (20 * std::log10(std::cos(math::deg2rad(beamWidth.get()) / 4.0)));
    double product = math::minnan(1.0, math::maxnan(-1.0, direction.rotate(Coord::X_AXIS) * Coord::X_AXIS));
    double angle = std::acos(product);
    return maxGain * std::pow(std::abs(std::cos(angle / 2.0)), exponent);
}

} // namespace physicallayer

} // namespace inet

