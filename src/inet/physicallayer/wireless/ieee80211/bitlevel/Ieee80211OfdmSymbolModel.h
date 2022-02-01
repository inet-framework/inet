//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMSYMBOLMODEL_H
#define __INET_IEEE80211OFDMSYMBOLMODEL_H

#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmTransmissionSymbolModel : public TransmissionSymbolModel
{
  public:
    Ieee80211OfdmTransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation);
    virtual ~Ieee80211OfdmTransmissionSymbolModel();
};

class INET_API Ieee80211OfdmReceptionSymbolModel : public ReceptionSymbolModel
{
  public:
    Ieee80211OfdmReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols);
    virtual ~Ieee80211OfdmReceptionSymbolModel();
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

