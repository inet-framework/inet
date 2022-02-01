//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/DipoleAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(DipoleAntenna);

DipoleAntenna::DipoleAntenna() :
    AntennaBase()
{
}

void DipoleAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        gain = makeShared<AntennaGain>(par("wireAxis"), m(par("length")));
}

std::ostream& DipoleAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DipoleAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(length, gain->getLength());
    return AntennaBase::printToStream(stream, level);
}

DipoleAntenna::AntennaGain::AntennaGain(const char *wireAxis, m length) :
    length(length)
{
    wireAxisDirection = Coord::parse(wireAxis);
}

double DipoleAntenna::AntennaGain::computeGain(const Quaternion& direction) const
{
    double product = math::minnan(1.0, math::maxnan(-1.0, direction.rotate(Coord::X_AXIS) * wireAxisDirection));
    double angle = std::acos(product);
    double q = sin(angle);
    return 1.5 * q * q;
}

} // namespace physicallayer

} // namespace inet

