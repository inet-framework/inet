//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMDEMODULATOR_H
#define __INET_IEEE80211OFDMDEMODULATOR_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaving.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmDemodulator : public IDemodulator
{
  protected:
    const Ieee80211OfdmModulation *subcarrierModulation = nullptr;

  protected:
    BitVector demodulateSymbol(const Ieee80211OfdmSymbol *signalSymbol) const;
    const IReceptionBitModel *createBitModel(const BitVector *bitRepresentation, int signalFieldBitLength, bps signalFieldBitrate, int dataFieldBitLength, bps dataFieldBitrate) const;
    bool isPilotOrDcSubcarrier(int i) const;

  public:
    Ieee80211OfdmDemodulator(const Ieee80211OfdmModulation *subcarrierModulation);

    const Ieee80211OfdmModulation *getModulation() const { return subcarrierModulation; }
    virtual const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

