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
    simtime_t preambleDuration = 0; // TODO:
    simtime_t headerDuration = sampleModel->getHeaderSampleLength() / sampleModel->getHeaderSampleRate();
    simtime_t dataDuration = sampleModel->getDataSampleLength() / sampleModel->getDataSampleRate();
    return new ScalarTransmissionSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, bandwidth, power);
}

} // namespace physicallayer
} // namespace inet

