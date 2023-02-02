//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmissionAnalogModel.h"

namespace inet {
namespace physicallayer {

std::ostream& DimensionalTransmissionAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerFunction);
    return stream;
}

} // namespace physicallayer
} // namespace inet

