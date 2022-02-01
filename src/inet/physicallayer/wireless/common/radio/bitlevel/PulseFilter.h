//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PULSEFILTER_H
#define __INET_PULSEFILTER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IPulseFilter.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API PulseFilter : public IPulseFilter
{
  protected:
    const int samplePerSymbol;

  public:
    PulseFilter();

    virtual const IReceptionSymbolModel *filter(const IReceptionSampleModel *sampleModel) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

