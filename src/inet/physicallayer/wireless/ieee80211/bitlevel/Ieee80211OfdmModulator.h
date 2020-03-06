//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMMODULATOR_H
#define __INET_IEEE80211OFDMMODULATOR_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmModulator : public IModulator
{
  public:
    static const ApskSymbol negativePilotSubcarrier;
    static const ApskSymbol positivePilotSubcarrier;

  protected:
    const Ieee80211OfdmModulation *modulation;
    static const double pilotSubcarrierPolarityVector[127];
    unsigned int pilotSubcarrierPolarityVectorOffset;

  protected:
    int getSubcarrierIndex(int ofdmSymbolIndex) const;
    void insertPilotSubcarriers(Ieee80211OfdmSymbol *ofdmSymbol, int symbolID) const;

  public:
    Ieee80211OfdmModulator(const Ieee80211OfdmModulation *subcarrierModulation, unsigned int polarityVectorOffset);

    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const override;
    const Ieee80211OfdmModulation *getModulation() const override { return modulation; }
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

