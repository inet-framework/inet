//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

ScalarSignalAnalogModel::ScalarSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    NarrowbandSignalAnalogModel(duration, centerFrequency, bandwidth),
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

ScalarTransmissionSignalAnalogModel::ScalarTransmissionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    ScalarSignalAnalogModel(duration, centerFrequency, bandwidth, power)
{
}

ScalarReceptionSignalAnalogModel::ScalarReceptionSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) :
    ScalarSignalAnalogModel(duration, centerFrequency, bandwidth, power)
{
}

} // namespace physicallayer

} // namespace inet

