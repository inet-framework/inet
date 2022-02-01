//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"

namespace inet {

namespace physicallayer {

ReceptionBase::ReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    receiver(receiver),
    transmission(transmission),
    startTime(startTime),
    endTime(endTime),
    preambleDuration(transmission->getPreambleDuration()),
    headerDuration(transmission->getHeaderDuration()),
    dataDuration(transmission->getDataDuration()),
    startPosition(startPosition),
    endPosition(endPosition),
    startOrientation(startOrientation),
    endOrientation(endOrientation)
{
}

std::ostream& ReceptionBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(transmissionId, transmission->getId());
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(receiverId, receiver->getId())
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

const simtime_t ReceptionBase::getStartTime(IRadioSignal::SignalPart part) const
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

const simtime_t ReceptionBase::getEndTime(IRadioSignal::SignalPart part) const
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

const simtime_t ReceptionBase::getDuration(IRadioSignal::SignalPart part) const
{
    switch (part) {
        case IRadioSignal::SIGNAL_PART_WHOLE:
            return getDuration();
        case IRadioSignal::SIGNAL_PART_PREAMBLE:
            return getPreambleDuration();
        case IRadioSignal::SIGNAL_PART_HEADER:
            return getHeaderDuration();
        case IRadioSignal::SIGNAL_PART_DATA:
            return getDataDuration();
        default:
            throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    }
}

} // namespace physicallayer

} // namespace inet

