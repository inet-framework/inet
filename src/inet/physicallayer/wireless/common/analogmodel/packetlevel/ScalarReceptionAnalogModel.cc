//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {

std::ostream& ScalarReceptionAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ScalarReception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(centerFrequency);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(bandwidth);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(power);
    return stream;
}

} // namespace physicallayer
} // namespace inet

