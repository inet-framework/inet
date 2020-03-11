//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskReceiver.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskTransmission.h"

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

std::ostream& Ieee80211UnitDiskReceiver::printToStream(std::ostream& stream, int level) const
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

