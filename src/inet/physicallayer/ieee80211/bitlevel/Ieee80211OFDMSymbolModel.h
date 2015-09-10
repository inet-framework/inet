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

#ifndef __INET_IEEE80211OFDMSYMBOLMODEL_H
#define __INET_IEEE80211OFDMSYMBOLMODEL_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMTransmissionSymbolModel : public TransmissionSymbolModel
{
  public:
    Ieee80211OFDMTransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation);
    virtual ~Ieee80211OFDMTransmissionSymbolModel();
};

class INET_API Ieee80211OFDMReceptionSymbolModel : public ReceptionSymbolModel
{
  public:
    Ieee80211OFDMReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols);
    virtual ~Ieee80211OFDMReceptionSymbolModel();
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMSYMBOLMODEL_H

