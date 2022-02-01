//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/4PpmModulation.h"

namespace inet {

namespace physicallayer {

const _4PpmModulation _4PpmModulation::singleton;

_4PpmModulation::_4PpmModulation() :
    PpmModulationBase(2)
{
}

} // namespace physicallayer

} // namespace inet

