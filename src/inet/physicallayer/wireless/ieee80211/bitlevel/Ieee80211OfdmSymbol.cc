//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"

namespace inet {

namespace physicallayer {

void Ieee80211OfdmSymbol::pushApskSymbol(const ApskSymbol *apskSymbol, int subcarrierIndex)
{
    if (subcarrierIndex >= 53)
        throw cRuntimeError("Out of range with subcarrierIndex = %d", subcarrierIndex);
    subcarrierSymbols[subcarrierIndex] = apskSymbol;
}

std::ostream& operator<<(std::ostream& out, const Ieee80211OfdmSymbol& symbol)
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

} /* namespace physicallayer */
} /* namespace inet */

