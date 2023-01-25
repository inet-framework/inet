//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMERRORMODEL_H
#define __INET_IEEE80211OFDMERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmErrorModel : public Ieee80211NistErrorModel, public ILayeredErrorModel
{
  protected:
    const char *symbolCorruptionMode = nullptr;
    double signalSymbolErrorRate;
    double dataSymbolErrorRate;
    double signalBitErrorRate;
    double dataBitErrorRate;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void corruptBits(BitVector *bits, double ber, int begin, int end) const;

  public:
    virtual const IReceptionPacketModel *computePacketModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionBitModel *computeBitModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSymbolModel *computeSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual const IReceptionSampleModel *computeSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    // TODO: don't pass in symbolErrorRate - it's not needed for vectorial corruption mode
    virtual const ApskSymbol *computeReceivedSymbol(const ApskModulationBase *modulation, const ApskSymbol *transmittedSymbol, double snir, double symbolErrorRate) const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

