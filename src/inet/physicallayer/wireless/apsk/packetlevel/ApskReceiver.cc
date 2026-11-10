//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskReceiver.h"

#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskReceiver);

std::ostream& ApskReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

bool ApskReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto apskTransmission = dynamic_cast<const ApskTransmission *>(transmission);
    return apskTransmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool ApskReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto apksTransmission = dynamic_cast<const ApskTransmission *>(reception->getTransmission());
    return apksTransmission && getAnalogModel()->computeIsReceptionPossible(listening, reception, sensitivity);
}

std::optional<FlatReceiverBase::PartRange> ApskReceiver::computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    auto apskTransmission = check_and_cast<const ApskTransmission *>(transmission);
    PartRange range;
    range.isInPacket = true;
    if (part == IRadioSignal::SIGNAL_PART_HEADER) {
        range.offset = b(0);
        range.length = apskTransmission->getHeaderLength();
    }
    else if (part == IRadioSignal::SIGNAL_PART_DATA) {
        // the data part also holds the padding of the modulation
        range.offset = apskTransmission->getHeaderLength();
        range.length = apskTransmission->getDataLength();
    }
    else
        throw cRuntimeError("Only the header part and the data part have regions, not the %s part", IRadioSignal::getSignalPartName(part));
    if (range.offset + range.length > transmission->getPacket()->getTotalLength())
        return std::nullopt;
    return range;
}

} // namespace physicallayer

} // namespace inet

