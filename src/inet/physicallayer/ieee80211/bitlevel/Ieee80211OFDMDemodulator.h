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

#ifndef __INET_IEEE80211OFDMDEMODULATOR_H
#define __INET_IEEE80211OFDMDEMODULATOR_H

#include "inet/physicallayer/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMSymbol.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMInterleaving.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMDemodulator : public IDemodulator
{
  protected:
    const Ieee80211OFDMModulation *subcarrierModulation = nullptr;

  protected:
    BitVector demodulateSymbol(const Ieee80211OFDMSymbol *signalSymbol) const;
    const IReceptionBitModel *createBitModel(const BitVector *bitRepresentation, int signalFieldBitLength, bps signalFieldBitRate, int dataFieldBitLength, bps dataFieldBitRate) const;
    bool isPilotOrDcSubcarrier(int i) const;

  public:
    Ieee80211OFDMDemodulator(const Ieee80211OFDMModulation *subcarrierModulation);

    const Ieee80211OFDMModulation *getModulation() const { return subcarrierModulation; }
    virtual const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211OFDMDEMODULATOR_H

