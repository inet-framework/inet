//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {

namespace physicallayer {

const double k = 1 / sqrt(2);

const std::vector<ApskSymbol> QpskModulation::constellation = {
    k * ApskSymbol(-1, -1), k * ApskSymbol(1, -1), k * ApskSymbol(-1, 1), k * ApskSymbol(1, 1)
};

const QpskModulation QpskModulation::singleton;

QpskModulation::QpskModulation() : MqamModulationBase(k ,&constellation)
{
}

} // namespace physicallayer

} // namespace inet

