//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/4GfskModulation.h"

namespace inet {

namespace physicallayer {

const _4GfskModulation _4GfskModulation::singleton;

_4GfskModulation::_4GfskModulation() :
    GfskModulationBase(4)
{
}

} // namespace physicallayer

} // namespace inet

