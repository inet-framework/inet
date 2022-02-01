//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskReception.h"

namespace inet {

namespace physicallayer {

Register_Enum(inet::physicallayer::UnitDiskReception::Power,
        (UnitDiskReception::POWER_UNDETECTABLE,
         UnitDiskReception::POWER_DETECTABLE,
         UnitDiskReception::POWER_INTERFERING,
         UnitDiskReception::POWER_RECEIVABLE));

UnitDiskReception::UnitDiskReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const Power power) :
    ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
    power(power)
{
}

std::ostream& UnitDiskReception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskReception";
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(power, cEnum::get(opp_typename(typeid(UnitDiskReception::Power)))->getStringFor(power) + 6);
    return ReceptionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

