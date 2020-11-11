//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

