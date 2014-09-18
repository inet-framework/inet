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

#include "APSKModulationBase.h"

namespace inet {
namespace physicallayer {

void APSKModulationBase::printToStream(std::ostream &stream) const
{
    stream << "APSK modulation with";
    stream << " constellation size = " << constellationSize << " codeword length = " << codeWordLength;
    stream << " normalization factor = " << normalizationFactor << endl;
}

const APSKSymbol *APSKModulationBase::mapToConstellationDiagram(const ShortBitVector& symbol) const
{
    int decimalSymbol = symbol.toDecimal();
    if (decimalSymbol >= constellationSize)
        throw cRuntimeError("Unknown input: %d", decimalSymbol);
    return &encodingTable[decimalSymbol];
}

} /* namespace physicallayer */
} /* namespace inet */
