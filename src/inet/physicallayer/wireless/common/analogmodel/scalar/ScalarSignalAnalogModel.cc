//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

ScalarSignalAnalogModel::ScalarSignalAnalogModel(const simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power) :
    NarrowbandSignalAnalogModel(preambleDuration, dataDuration, headerDuration, centerFrequency, bandwidth),
    power(power)
{
}

std::ostream& ScalarSignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarSignalAnalogModel";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(power);
    return NarrowbandSignalAnalogModel::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

