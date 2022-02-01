//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIGITALANALOGCONVERTER_H
#define __INET_DIGITALANALOGCONVERTER_H

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerDefs.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDigitalAnalogConverter.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API ScalarDigitalAnalogConverter : public IDigitalAnalogConverter
{
  protected:
    W power;
    // TODO why centerFrequency and bandwidth here? why not in the shaper
    Hz centerFrequency;
    Hz bandwidth;
    double sampleRate;

  public:
    ScalarDigitalAnalogConverter();

    virtual const ITransmissionAnalogModel *convertDigitalToAnalog(const ITransmissionSampleModel *sampleModel) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

