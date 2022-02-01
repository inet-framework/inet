//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/PulseShaper.h"

namespace inet {
namespace physicallayer {

PulseShaper::PulseShaper() :
    samplePerSymbol(-1)
{}

const ITransmissionSampleModel *PulseShaper::shape(const ITransmissionSymbolModel *symbolModel) const
{
    const int sampleLength = symbolModel->getPayloadSymbolLength() * samplePerSymbol;
    const double sampleRate = symbolModel->getPayloadSymbolRate() * samplePerSymbol;
    return new TransmissionSampleModel(sampleLength, sampleRate, nullptr);
}

} // namespace physicallayer
} // namespace inet

