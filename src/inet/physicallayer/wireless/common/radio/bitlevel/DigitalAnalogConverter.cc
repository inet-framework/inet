//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/DigitalAnalogConverter.h"

namespace inet {
namespace physicallayer {

ScalarDigitalAnalogConverter::ScalarDigitalAnalogConverter() :
    power(W(NaN)),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    sampleRate(NaN)
{}

const ITransmissionAnalogModel *ScalarDigitalAnalogConverter::convertDigitalToAnalog(const ITransmissionSampleModel *sampleModel) const
{
    const simtime_t duration = sampleModel->getSampleLength() / sampleModel->getSampleRate();
    return new ScalarTransmissionSignalAnalogModel(duration, centerFrequency, bandwidth, power);
}

} // namespace physicallayer
} // namespace inet

