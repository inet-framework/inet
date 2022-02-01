//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"

namespace inet {

namespace physicallayer {

const double k = 1 / sqrt(42);

const std::vector<ApskSymbol> Qam64Modulation::constellation = {
    k * ApskSymbol(-7, -7), k * ApskSymbol(7, -7), k * ApskSymbol(-1, -7), k * ApskSymbol(1, -7), k * ApskSymbol(-5, -7),
    k * ApskSymbol(5, -7), k * ApskSymbol(-3, -7), k * ApskSymbol(3, -7), k * ApskSymbol(-7, 7), k * ApskSymbol(7, 7),
    k * ApskSymbol(-1, 7), k * ApskSymbol(1, 7), k * ApskSymbol(-5, 7), k * ApskSymbol(5, 7), k * ApskSymbol(-3, 7),
    k * ApskSymbol(3, 7), k * ApskSymbol(-7, -1), k * ApskSymbol(7, -1), k * ApskSymbol(-1, -1), k * ApskSymbol(1, -1),
    k * ApskSymbol(-5, -1), k * ApskSymbol(5, -1), k * ApskSymbol(-3, -1), k * ApskSymbol(3, -1), k * ApskSymbol(-7, 1),
    k * ApskSymbol(7, 1), k * ApskSymbol(-1, 1), k * ApskSymbol(1, 1), k * ApskSymbol(-5, 1), k * ApskSymbol(5, 1),
    k * ApskSymbol(-3, 1), k * ApskSymbol(3, 1), k * ApskSymbol(-7, -5), k * ApskSymbol(7, -5), k * ApskSymbol(-1, -5),
    k * ApskSymbol(1, -5), k * ApskSymbol(-5, -5), k * ApskSymbol(5, -5), k * ApskSymbol(-3, -5), k * ApskSymbol(3, -5),
    k * ApskSymbol(-7, 5), k * ApskSymbol(7, 5), k * ApskSymbol(-1, 5), k * ApskSymbol(1, 5), k * ApskSymbol(-5, 5),
    k * ApskSymbol(5, 5), k * ApskSymbol(-3, 5), k * ApskSymbol(3, 5), k * ApskSymbol(-7, -3), k * ApskSymbol(7, -3),
    k * ApskSymbol(-1, -3), k * ApskSymbol(1, -3), k * ApskSymbol(-5, -3), k * ApskSymbol(5, -3), k * ApskSymbol(-3, -3),
    k * ApskSymbol(3, -3), k * ApskSymbol(-7, 3), k * ApskSymbol(7, 3), k * ApskSymbol(-1, 3), k * ApskSymbol(1, 3),
    k * ApskSymbol(-5, 3), k * ApskSymbol(5, 3), k * ApskSymbol(-3, 3), k * ApskSymbol(3, 3)
};

const Qam64Modulation Qam64Modulation::singleton;

Qam64Modulation::Qam64Modulation() : MqamModulationBase(k, &constellation)
{
}

} // namespace physicallayer

} // namespace inet

