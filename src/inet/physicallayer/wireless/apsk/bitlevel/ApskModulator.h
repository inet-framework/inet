//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKMODULATOR_H
#define __INET_APSKMODULATOR_H

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IModulator.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API ApskModulator : public IModulator, public cSimpleModule
{
  protected:
    const ApskModulationBase *modulation;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    ApskModulator();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const IModulation *getModulation() const override { return modulation; }
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

