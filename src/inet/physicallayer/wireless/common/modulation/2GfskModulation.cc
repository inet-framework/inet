//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/2GfskModulation.h"

namespace inet {

namespace physicallayer {

const _2GfskModulation _2GfskModulation::singleton;

_2GfskModulation::_2GfskModulation() :
    GfskModulationBase(2)
{
}

} // namespace physicallayer

} // namespace inet

