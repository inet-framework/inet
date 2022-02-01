//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/DqpskModulation.h"

namespace inet {

namespace physicallayer {

const DqpskModulation DqpskModulation::singleton;

DqpskModulation::DqpskModulation() :
    DpskModulationBase(4)
{
}

} // namespace physicallayer

} // namespace inet

