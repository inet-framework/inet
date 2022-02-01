//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"

namespace inet {
namespace physicallayer {

LayeredReception::LayeredReception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
    analogModel(analogModel)
{
}

LayeredReception::~LayeredReception()
{
    delete analogModel;
}

std::ostream& LayeredReception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredReception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(analogModel, printFieldToString(analogModel, level + 1, evFlags));
    return ReceptionBase::printToStream(stream, level);

}

} // namespace physicallayer
} // namespace inet

