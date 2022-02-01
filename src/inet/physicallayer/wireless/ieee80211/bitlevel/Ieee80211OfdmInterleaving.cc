//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaving.h"

namespace inet {

namespace physicallayer {

Ieee80211OfdmInterleaving::Ieee80211OfdmInterleaving(int numberOfCodedBitsPerSymbol, int numberOfCodedBitsPerSubcarrier) :
    numberOfCodedBitsPerSymbol(numberOfCodedBitsPerSymbol),
    numberOfCodedBitsPerSubcarrier(numberOfCodedBitsPerSubcarrier)
{
}

std::ostream& Ieee80211OfdmInterleaving::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmInterleaving";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(numberOfCodedBitsPerSymbol)
               << EV_FIELD(numberOfCodedBitsPerSubcarrier);
    return stream;
}

} /* namespace physicallayer */

} /* namespace inet */

