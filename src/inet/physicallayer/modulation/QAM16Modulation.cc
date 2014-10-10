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

#include "inet/physicallayer/modulation/QAM16Modulation.h"

namespace inet {
namespace physicallayer {

const double QAM16Modulation::kMOD = 1/sqrt(10);
const APSKSymbol QAM16Modulation::encodingTable[] = {kMOD * APSKSymbol(-3, -3), kMOD * APSKSymbol(3, -3), kMOD * APSKSymbol(-1, -3),
                                                     kMOD * APSKSymbol(1, -3), kMOD * APSKSymbol(-3, 3), kMOD * APSKSymbol(3, 3),
                                                     kMOD * APSKSymbol(-1, 3), kMOD * APSKSymbol(1, 3), kMOD * APSKSymbol(-3, -1),
                                                     kMOD * APSKSymbol(3, -1), kMOD * APSKSymbol(-1, -1), kMOD * APSKSymbol(1, -1),
                                                     kMOD * APSKSymbol(-3, 1), kMOD * APSKSymbol(3, 1), kMOD * APSKSymbol(-1, 1),
                                                     kMOD * APSKSymbol(1, 1)};

const QAM16Modulation QAM16Modulation::singleton;
QAM16Modulation::QAM16Modulation() : APSKModulationBase(encodingTable, 4,16, kMOD)
{

}

} /* namespace physicallayer */
} /* namespace inet */
