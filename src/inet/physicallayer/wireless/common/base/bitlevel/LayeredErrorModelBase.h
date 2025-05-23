//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDERRORMODELBASE_H
#define __INET_LAYEREDERRORMODELBASE_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ILayeredErrorModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredErrorModelBase : public Module, public ILayeredErrorModel
{
  protected:
    const char *symbolCorruptionMode = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual const IReceptionPacketModel *computePacketModel(const ITransmission *transmission, double packetErrorRate) const;
    virtual const IReceptionBitModel *computeBitModel(const ITransmission *transmission, double bitErrorRate) const;
    virtual const IReceptionSymbolModel *computeSymbolModel(const ITransmission *transmission, double symbolErrorRate) const;

    virtual const ApskSymbol *computeCorruptSymbol(const ApskModulationBase *modulation, const ApskSymbol *transmittedSymbol) const;
};

} // namespace physicallayer

} // namespace inet

#endif

