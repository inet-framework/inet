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
    int headerSampleLength = symbolModel->getHeaderSymbolLength() * samplePerSymbol;
    double headerSampleRate = symbolModel->getHeaderSymbolRate() * samplePerSymbol;
    int dataSampleLength = symbolModel->getDataSymbolLength() * samplePerSymbol;
    double dataSampleRate = symbolModel->getDataSymbolRate() * samplePerSymbol;
    return new TransmissionSampleModel(headerSampleLength, headerSampleRate, dataSampleLength, dataSampleRate, nullptr);
}

} // namespace physicallayer
} // namespace inet

