//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ANALOGDIGITALCONVERTER_H
#define __INET_ANALOGDIGITALCONVERTER_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/SignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IAnalogDigitalConverter.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API ScalarAnalogDigitalConverter : public IAnalogDigitalConverter
{
  protected:
    W power;
    // TODO why centerFrequency and bandwidth here? why not in the shaper
    Hz centerFrequency;
    Hz bandwidth;
    double sampleRate;

  public:
    ScalarAnalogDigitalConverter();

    virtual const IReceptionSampleModel *convertAnalogToDigital(const IReceptionAnalogModel *analogModel) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

