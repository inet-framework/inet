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
    const int symbolLength = sampleModel->getSampleLength() / samplePerSymbol;
    const double symbolRate = sampleModel->getSampleRate() / samplePerSymbol;
    return new ReceptionSymbolModel(symbolLength, symbolRate, -1, NaN, nullptr, NaN);
}

} // namespace physicallayer
} // namespace inet

