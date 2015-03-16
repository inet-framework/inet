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

#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMSymbol.h"

namespace inet {

namespace physicallayer {

void Ieee80211OFDMSymbol::pushAPSKSymbol(const APSKSymbol *apskSymbol, int subcarrierIndex)
{
    if (subcarrierIndex >= 53)
        throw cRuntimeError("Out of range with subcarrierIndex = %d", subcarrierIndex);
    subcarrierSymbols[subcarrierIndex] = apskSymbol;
}

std::ostream& operator<<(std::ostream& out, const Ieee80211OFDMSymbol& symbol)
{
    if (symbol.subcarrierSymbols[0])
        out << *symbol.subcarrierSymbols[0];
    else
        out << "UNDEFINED SYMBOL";
    for (unsigned int i = 1; i < symbol.subcarrierSymbols.size(); i++)
        if (symbol.subcarrierSymbols[i])
            out << " " << *symbol.subcarrierSymbols[i];
        else
            out << " UNDEFINED SYMBOL";
    return out;
}

Ieee80211OFDMSymbol::Ieee80211OFDMSymbol(const Ieee80211OFDMSymbol& otherSymbol)
{
    subcarrierSymbols.resize(53, nullptr);
    for (unsigned int i = 0; i < subcarrierSymbols.size(); i++)
        subcarrierSymbols[i] = otherSymbol.subcarrierSymbols[i];
    // Default copy constructor doesn't work here since the data below is not static data.
    subcarrierSymbols[5] = new APSKSymbol(*otherSymbol.subcarrierSymbols[5]);
    subcarrierSymbols[19] = new APSKSymbol(*otherSymbol.subcarrierSymbols[19]);
    subcarrierSymbols[33] = new APSKSymbol(*otherSymbol.subcarrierSymbols[33]);
    subcarrierSymbols[47] = new APSKSymbol(*otherSymbol.subcarrierSymbols[47]);
}

Ieee80211OFDMSymbol::~Ieee80211OFDMSymbol()
{
    // Dynamically created pilot symbols need to be deleted
    delete subcarrierSymbols[5];
    delete subcarrierSymbols[19];
    delete subcarrierSymbols[33];
    delete subcarrierSymbols[47];
}
} /* namespace physicallayer */
} /* namespace inet */

