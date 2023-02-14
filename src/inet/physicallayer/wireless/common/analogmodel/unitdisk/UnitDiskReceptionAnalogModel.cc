//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {


Register_Enum(inet::physicallayer::UnitDiskReceptionAnalogModel::Power,
        (UnitDiskReceptionAnalogModel::POWER_UNDETECTABLE,
         UnitDiskReceptionAnalogModel::POWER_DETECTABLE,
         UnitDiskReceptionAnalogModel::POWER_INTERFERING,
         UnitDiskReceptionAnalogModel::POWER_RECEIVABLE));

std::ostream& UnitDiskReceptionAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskReceptionAnalogModel";
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(power, cEnum::get(opp_typename(typeid(UnitDiskReceptionAnalogModel::Power)))->getStringFor(power) + 6);
    return stream;
}

} // namespace physicallayer
} // namespace inet

