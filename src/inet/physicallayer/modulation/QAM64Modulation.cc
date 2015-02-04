//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {

namespace physicallayer {

const double k = 1 / sqrt(42);

const std::vector<APSKSymbol> QAM64Modulation::constellation = {
    k * APSKSymbol(-7, -7), k * APSKSymbol(7, -7), k * APSKSymbol(-1, -7), k * APSKSymbol(1, -7), k * APSKSymbol(-5, -7),
    k * APSKSymbol(5, -7), k * APSKSymbol(-3, -7), k * APSKSymbol(3, -7), k * APSKSymbol(-7, 7), k * APSKSymbol(7, 7),
    k * APSKSymbol(-1, 7), k * APSKSymbol(1, 7), k * APSKSymbol(-5, 7), k * APSKSymbol(5, 7), k * APSKSymbol(-3, 7),
    k * APSKSymbol(3, 7), k * APSKSymbol(-7, -1), k * APSKSymbol(7, -1), k * APSKSymbol(-1, -1), k * APSKSymbol(1, -1),
    k * APSKSymbol(-5, -1), k * APSKSymbol(5, -1), k * APSKSymbol(-3, -1), k * APSKSymbol(3, -1), k * APSKSymbol(-7, 1),
    k * APSKSymbol(7, 1), k * APSKSymbol(-1, 1), k * APSKSymbol(1, 1), k * APSKSymbol(-5, 1), k * APSKSymbol(5, 1),
    k * APSKSymbol(-3, 1), k * APSKSymbol(3, 1), k * APSKSymbol(-7, -5), k * APSKSymbol(7, -5), k * APSKSymbol(-1, -5),
    k * APSKSymbol(1, -5), k * APSKSymbol(-5, -5), k * APSKSymbol(5, -5), k * APSKSymbol(-3, -5), k * APSKSymbol(3, -5),
    k * APSKSymbol(-7, 5), k * APSKSymbol(7, 5), k * APSKSymbol(-1, 5), k * APSKSymbol(1, 5), k * APSKSymbol(-5, 5),
    k * APSKSymbol(5, 5), k * APSKSymbol(-3, 5), k * APSKSymbol(3, 5), k * APSKSymbol(-7, -3), k * APSKSymbol(7, -3),
    k * APSKSymbol(-1, -3), k * APSKSymbol(1, -3), k * APSKSymbol(-5, -3), k * APSKSymbol(5, -3), k * APSKSymbol(-3, -3),
    k * APSKSymbol(3, -3), k * APSKSymbol(-7, 3), k * APSKSymbol(7, 3), k * APSKSymbol(-1, 3), k * APSKSymbol(1, 3),
    k * APSKSymbol(-5, 3), k * APSKSymbol(5, 3), k * APSKSymbol(-3, 3), k * APSKSymbol(3, 3)
};

const QAM64Modulation QAM64Modulation::singleton;

QAM64Modulation::QAM64Modulation() : MQAMModulationBase(&constellation)
{
}

} // namespace physicallayer

} // namespace inet

