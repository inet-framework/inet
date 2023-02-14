//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

SignalAnalogModel::SignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration) :
    preambleDuration(preambleDuration),
    headerDuration(headerDuration),
    dataDuration(dataDuration)
{
}

std::ostream& SignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE) {
        stream << ", preambleDuration = " << preambleDuration
               << ", headerDuration = " << headerDuration;
    }
    if (level <= PRINT_LEVEL_DEBUG)
        stream << ", preambleDuration = " << preambleDuration;
    return stream;
}

NarrowbandSignalAnalogModel::NarrowbandSignalAnalogModel(const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, Hz centerFrequency, Hz bandwidth) :
    SignalAnalogModel(preambleDuration, headerDuration, dataDuration),
    centerFrequency(centerFrequency),
    bandwidth(bandwidth)
{
}

std::ostream& NarrowbandSignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(centerFrequency);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(bandwidth);
    return stream;
}

} // namespace physicallayer

} // namespace inet

