//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/signal/Arrival.h"

namespace inet {
namespace physicallayer {

Arrival::Arrival(const simtime_t startPropagationTime, const simtime_t endPropagationTime, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    startPropagationTime(startPropagationTime),
    endPropagationTime(endPropagationTime),
    startTime(startTime),
    endTime(endTime),
    preambleDuration(preambleDuration),
    headerDuration(headerDuration),
    dataDuration(dataDuration),
    startPosition(startPosition),
    endPosition(endPosition),
    startOrientation(startOrientation),
    endOrientation(endOrientation)
{
}

std::ostream& Arrival::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Arrival";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(startPropagationTime)
               << EV_FIELD(endPropagationTime)
               << EV_FIELD(startTime)
               << EV_FIELD(endTime)
               << EV_FIELD(preambleDuration)
               << EV_FIELD(headerDuration)
               << EV_FIELD(dataDuration)
               << EV_FIELD(startPosition)
               << EV_FIELD(endPosition)
               << EV_FIELD(startOrientation)
               << EV_FIELD(endOrientation);
    return stream;
}

const simtime_t Arrival::getStartTime(IRadioSignal::SignalPart part) const
{
    switch (part) {
        case IRadioSignal::SIGNAL_PART_WHOLE:
            return getStartTime();
        case IRadioSignal::SIGNAL_PART_PREAMBLE:
            return getPreambleStartTime();
        case IRadioSignal::SIGNAL_PART_HEADER:
            return getHeaderStartTime();
        case IRadioSignal::SIGNAL_PART_DATA:
            return getDataStartTime();
        default:
            throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    }
}

const simtime_t Arrival::getEndTime(IRadioSignal::SignalPart part) const
{
    switch (part) {
        case IRadioSignal::SIGNAL_PART_WHOLE:
            return getEndTime();
        case IRadioSignal::SIGNAL_PART_PREAMBLE:
            return getPreambleEndTime();
        case IRadioSignal::SIGNAL_PART_HEADER:
            return getHeaderEndTime();
        case IRadioSignal::SIGNAL_PART_DATA:
            return getDataEndTime();
        default:
            throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    }
}

} // namespace physicallayer
} // namespace inet

