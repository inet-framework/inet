//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKDEMODULATOR_H
#define __INET_APSKDEMODULATOR_H

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API ApskDemodulator : public IDemodulator, public cSimpleModule
{
  protected:
    const ApskModulationBase *modulation;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    ApskDemodulator();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ApskModulationBase *getModulation() const { return modulation; }
    virtual const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

