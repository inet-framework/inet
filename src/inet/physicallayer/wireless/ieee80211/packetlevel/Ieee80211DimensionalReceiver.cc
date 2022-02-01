//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211DimensionalReceiver.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211DimensionalReceiver);

Ieee80211DimensionalReceiver::Ieee80211DimensionalReceiver() :
    Ieee80211ReceiverBase()
{
}

std::ostream& Ieee80211DimensionalReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211DimensionalReceiver";
    return Ieee80211ReceiverBase::printToStream(stream, level);
}

bool Ieee80211DimensionalReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    const Ieee80211DimensionalTransmission *ieee80211Transmission = dynamic_cast<const Ieee80211DimensionalTransmission *>(transmission);
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && Ieee80211ReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211DimensionalReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211DimensionalTransmission *>(reception->getTransmission());
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && Ieee80211ReceiverBase::computeIsReceptionPossible(listening, reception, part);
}

} // namespace physicallayer

} // namespace inet

