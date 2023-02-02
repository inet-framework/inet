//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {

std::ostream& DimensionalReceptionAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalReception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerFunction);
    return stream;
}

} // namespace physicallayer
} // namespace inet

