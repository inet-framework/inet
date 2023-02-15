//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/Reception.h"

namespace inet {
namespace physicallayer {

Reception::Reception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IReceptionAnalogModel *analogModel) :
    ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, analogModel)
{
}

std::ostream& Reception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Reception";
    return ReceptionBase::printToStream(stream, level);

}

} // namespace physicallayer
} // namespace inet

