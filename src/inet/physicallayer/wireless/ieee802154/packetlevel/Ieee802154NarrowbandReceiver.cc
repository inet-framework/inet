//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandReceiver.h"
#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandReceiver);

Ieee802154NarrowbandReceiver::Ieee802154NarrowbandReceiver() :
    FlatReceiverBase()
{
}

void Ieee802154NarrowbandReceiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minInterferencePower = mW(math::dBmW2mW(par("minInterferencePower")));
    }
}

bool Ieee802154NarrowbandReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee802154Transmission = dynamic_cast<const Ieee802154Transmission *>(transmission);
    return ieee802154Transmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee802154NarrowbandReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee802154Transmission = dynamic_cast<const Ieee802154Transmission *>(reception->getTransmission());
    return ieee802154Transmission && getAnalogModel()->computeIsReceptionPossible(listening, reception, sensitivity);
}

std::ostream& Ieee802154NarrowbandReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

std::optional<FlatReceiverBase::PartRange> Ieee802154NarrowbandReceiver::computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    auto ieee802154Transmission = check_and_cast<const Ieee802154Transmission *>(transmission);
    PartRange range;
    if (part == IRadioSignal::SIGNAL_PART_HEADER) {
        // the radio sends the header bits, but the packet holds no header chunk
        range.isInPacket = false;
        range.offset = b(0);
        range.length = ieee802154Transmission->getHeaderLength();
    }
    else if (part == IRadioSignal::SIGNAL_PART_DATA) {
        range.isInPacket = true;
        range.offset = b(0);
        range.length = ieee802154Transmission->getDataLength();
        if (range.length > transmission->getPacket()->getTotalLength())
            return std::nullopt;
    }
    else
        throw cRuntimeError("Only the header part and the data part have regions, not the %s part", IRadioSignal::getSignalPartName(part));
    return range;
}

} // namespace physicallayer

} // namespace inet

