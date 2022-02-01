//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/16PpmModulation.h"

namespace inet {

namespace physicallayer {

const _16PpmModulation _16PpmModulation::singleton;

_16PpmModulation::_16PpmModulation() :
    PpmModulationBase(4)
{
}

} // namespace physicallayer

} // namespace inet

