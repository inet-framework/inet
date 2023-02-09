//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarReceiver.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211ScalarReceiver);

Ieee80211ScalarReceiver::Ieee80211ScalarReceiver() :
    Ieee80211ReceiverBase()
{
}

std::ostream& Ieee80211ScalarReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211ScalarReceiver";
    return Ieee80211ReceiverBase::printToStream(stream, level);
}

bool Ieee80211ScalarReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211ScalarTransmission *>(transmission);
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211ScalarReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211ScalarTransmission *>(reception->getTransmission());
    return ieee80211Transmission && modeSet->containsMode(ieee80211Transmission->getMode()) && FlatReceiverBase::computeIsReceptionPossible(listening, reception, part);
}

} // namespace physicallayer

} // namespace inet

