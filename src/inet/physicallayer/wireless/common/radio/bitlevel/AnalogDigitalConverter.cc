//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/AnalogDigitalConverter.h"

namespace inet {
namespace physicallayer {

ScalarAnalogDigitalConverter::ScalarAnalogDigitalConverter() :
    power(W(NaN)),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    sampleRate(NaN)
{}

const IReceptionSampleModel *ScalarAnalogDigitalConverter::convertAnalogToDigital(const IReceptionAnalogModel *analogModel) const
{
    int headerSampleLength = std::ceil(analogModel->getHeaderDuration().dbl() / sampleRate);
    int dataSampleLength = std::ceil(analogModel->getDataDuration().dbl() / sampleRate);
    return new ReceptionSampleModel(headerSampleLength, sampleRate, dataSampleLength, sampleRate, nullptr);
}

} // namespace physicallayer
} // namespace inet

