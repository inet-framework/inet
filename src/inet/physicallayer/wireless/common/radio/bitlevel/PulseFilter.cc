//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/PulseFilter.h"

namespace inet {
namespace physicallayer {

PulseFilter::PulseFilter() :
    samplePerSymbol(-1)
{}

const IReceptionSymbolModel *PulseFilter::filter(const IReceptionSampleModel *sampleModel) const
{
    int headerSymbolLength = sampleModel->getHeaderSampleLength() / samplePerSymbol;
    double headerSymbolRate = sampleModel->getHeaderSampleRate() / samplePerSymbol;
    int dataSymbolLength = sampleModel->getDataSampleLength() / samplePerSymbol;
    double dataSymbolRate = sampleModel->getDataSampleRate() / samplePerSymbol;
    return new ReceptionSymbolModel(headerSymbolLength, headerSymbolRate, dataSymbolLength, dataSymbolRate, nullptr, NaN);
}

} // namespace physicallayer
} // namespace inet

