//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/DbpskModulation.h"

namespace inet {

namespace physicallayer {

const DbpskModulation DbpskModulation::singleton;

DbpskModulation::DbpskModulation() :
    DpskModulationBase(2)
{
}

} // namespace physicallayer

} // namespace inet

