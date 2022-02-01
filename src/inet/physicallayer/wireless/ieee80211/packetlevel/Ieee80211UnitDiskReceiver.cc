//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211UnitDiskReceiver.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211UnitDiskTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211UnitDiskReceiver);

Ieee80211UnitDiskReceiver::Ieee80211UnitDiskReceiver() :
    UnitDiskReceiver()
{
}

void Ieee80211UnitDiskReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
    }
}

std::ostream& Ieee80211UnitDiskReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211UnitDiskReceiver";
    return UnitDiskReceiver::printToStream(stream, level);
}

bool Ieee80211UnitDiskReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211UnitDiskTransmission *>(transmission);
    return ieee80211Transmission && /*modeSet->containsMode(ieee80211Transmission->getMode()) &&*/ ReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee80211UnitDiskReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211UnitDiskTransmission *>(reception->getTransmission());
    return ieee80211Transmission && /*modeSet->containsMode(ieee80211Transmission->getMode()) &&*/ UnitDiskReceiver::computeIsReceptionPossible(listening, reception, part);
}

const IReceptionResult *Ieee80211UnitDiskReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto transmission = check_and_cast<const Ieee80211TransmissionBase *>(reception->getTransmission());
    auto receptionResult = UnitDiskReceiver::computeReceptionResult(listening, reception, interference, snir, decisions);
    auto modeInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<Ieee80211ModeInd>();
    modeInd->setMode(transmission->getMode());
    auto channelInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<Ieee80211ChannelInd>();
    channelInd->setChannel(transmission->getChannel());
    return receptionResult;
}

} // namespace physicallayer
} // namespace inet

