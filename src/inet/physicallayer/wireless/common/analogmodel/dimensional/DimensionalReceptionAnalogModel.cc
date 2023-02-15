//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {

DimensionalReceptionAnalogModel::DimensionalReceptionAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& power) :
    DimensionalSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

