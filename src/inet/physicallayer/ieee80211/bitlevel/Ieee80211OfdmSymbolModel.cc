//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"

namespace inet {

namespace physicallayer {

Ieee80211OfdmTransmissionSymbolModel::~Ieee80211OfdmTransmissionSymbolModel()
{
    if (symbols) {
        for (auto it : *symbols)
            delete it;
    }
}

Ieee80211OfdmReceptionSymbolModel::~Ieee80211OfdmReceptionSymbolModel()
{
    if (symbols) {
        for (auto it : *symbols)
            delete it;
    }
}

Ieee80211OfdmTransmissionSymbolModel::Ieee80211OfdmTransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation) :
    TransmissionSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols, headerModulation, payloadModulation)
{
}

Ieee80211OfdmReceptionSymbolModel::Ieee80211OfdmReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols) :
    ReceptionSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols)
{
}
} /* namespace physicallayer */
} /* namespace inet */

