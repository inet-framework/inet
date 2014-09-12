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

#include "QAM16Modulation.h"

namespace inet {
namespace physicallayer {

const double QAM16Modulation::kMOD = 1/sqrt(10);
const Complex QAM16Modulation::encodingTable[] = {kMOD * Complex(-1, -3), kMOD * Complex(1, -3), kMOD * Complex(3, -3),
                                                  kMOD * Complex(-3, -1), kMOD * Complex(-1, -1), kMOD * Complex(1, -1),
                                                  kMOD * Complex(3, -1), kMOD * Complex(-3, 1), kMOD * Complex(-1, 1),
                                                  kMOD * Complex(1, 1), kMOD * Complex(3, 1), kMOD * Complex(-3, 3),
                                                  kMOD * Complex(-1, 3), kMOD * Complex(1, 3), kMOD * Complex(3, 3)};

QAM16Modulation::QAM16Modulation() : Modulation(encodingTable, 4,16, kMOD)
{

}

} /* namespace physicallayer */
} /* namespace inet */
