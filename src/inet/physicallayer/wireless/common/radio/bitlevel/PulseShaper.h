//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PULSESHAPER_H
#define __INET_PULSESHAPER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseShaper.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API PulseShaper : public IPulseShaper
{
  protected:
    const int samplePerSymbol;

  public:
    PulseShaper();

    virtual const ITransmissionSampleModel *shape(const ITransmissionSymbolModel *symbolModel) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

