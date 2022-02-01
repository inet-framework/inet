//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/ConstantGainAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(ConstantGainAntenna);

ConstantGainAntenna::ConstantGainAntenna() :
    AntennaBase()
{
}

void ConstantGainAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        gain = makeShared<AntennaGain>(math::dB2fraction(par("gain")));
}

std::ostream& ConstantGainAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ConstantGainAntenna";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(gain, gain->getMaxGain());
    return AntennaBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

