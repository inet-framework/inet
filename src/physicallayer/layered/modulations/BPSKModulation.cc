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

#include "BPSKModulation.h"


namespace inet {
namespace physicallayer {

const Complex BPSKModulation::encodingTable[] = {Complex(-1,0), Complex(1,0)};

BPSKModulation::BPSKModulation() : Modulation(1, 2, 1)
{
}

const Complex& BPSKModulation::map(const ShortBitVector& input) const
{
    if (input.toDecimal() >= constellationSize)
        throw cRuntimeError("Undefined ...");
    return encodingTable[input.toDecimal()];
}


} /* namespace physicallayer */
} /* namespace inet */
