//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"

namespace inet {

namespace physicallayer {

Ieee80211OfdmTransmissionSymbolModel::Ieee80211OfdmTransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation) :
    TransmissionSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols, headerModulation, payloadModulation)
{
}

Ieee80211OfdmReceptionSymbolModel::Ieee80211OfdmReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols) :
    ReceptionSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols, NaN)
{
}

} /* namespace physicallayer */
} /* namespace inet */

