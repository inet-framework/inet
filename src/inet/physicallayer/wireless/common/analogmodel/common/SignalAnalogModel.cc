//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/common/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

SignalAnalogModel::SignalAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration) :
    preambleDuration(preambleDuration),
    headerDuration(headerDuration),
    dataDuration(dataDuration)
{
}

std::ostream& SignalAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE) {
        stream << EV_FIELD(preambleDuration)
               << EV_FIELD(headerDuration);
    }
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(dataDuration);
    return stream;
}

} // namespace physicallayer

} // namespace inet

