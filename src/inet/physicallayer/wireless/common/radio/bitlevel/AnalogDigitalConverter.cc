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
    const simtime_t duration = analogModel->getDuration();
    const int sampleLength = std::ceil(duration.dbl() / sampleRate);
    return new ReceptionSampleModel(sampleLength, sampleRate, nullptr, W(0));
}

} // namespace physicallayer
} // namespace inet

