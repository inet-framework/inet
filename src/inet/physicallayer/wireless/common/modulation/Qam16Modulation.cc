//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"

namespace inet {

namespace physicallayer {

const double k = 1 / sqrt(10);

const std::vector<ApskSymbol> Qam16Modulation::constellation = {
    k * ApskSymbol(-3, -3), k * ApskSymbol(3, -3), k * ApskSymbol(-1, -3),
    k * ApskSymbol(1, -3), k * ApskSymbol(-3, 3), k * ApskSymbol(3, 3),
    k * ApskSymbol(-1, 3), k * ApskSymbol(1, 3), k * ApskSymbol(-3, -1),
    k * ApskSymbol(3, -1), k * ApskSymbol(-1, -1), k * ApskSymbol(1, -1),
    k * ApskSymbol(-3, 1), k * ApskSymbol(3, 1), k * ApskSymbol(-1, 1),
    k * ApskSymbol(1, 1)
};

const Qam16Modulation Qam16Modulation::singleton;

Qam16Modulation::Qam16Modulation() : MqamModulationBase(k, &constellation)
{
}

} // namespace physicallayer

} // namespace inet

