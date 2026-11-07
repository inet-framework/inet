//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/DpskModulationBase.h"

namespace inet {

namespace physicallayer {

DpskModulationBase::DpskModulationBase(unsigned int constellationSize) :
    constellationSize(constellationSize)
{
}

unsigned int DpskModulationBase::getBitsPerSymbol() const
{
    unsigned int bits = 0;
    for (unsigned int size = constellationSize; size > 1; size >>= 1)
        bits++;
    return bits;
}

} // namespace physicallayer

} // namespace inet

