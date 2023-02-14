//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/Reception.h"

namespace inet {
namespace physicallayer {

Reception::Reception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation)
{
    // TODO
    this->analogModel = analogModel;
}

Reception::~Reception()
{
    delete analogModel;
}

std::ostream& Reception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Reception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(analogModel, printFieldToString(analogModel, level + 1, evFlags));
    return ReceptionBase::printToStream(stream, level);

}

} // namespace physicallayer
} // namespace inet

