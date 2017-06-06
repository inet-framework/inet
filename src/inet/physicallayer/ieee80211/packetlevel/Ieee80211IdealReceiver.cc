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
#include "inet/physicallayer/idealradio/IdealNoise.h"
#include "inet/physicallayer/idealradio/IdealReception.h"
#include "inet/physicallayer/idealradio/IdealTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211IdealReceiver.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211IdealReceiver);

Ieee80211IdealReceiver::Ieee80211IdealReceiver() :
    IdealReceiver()
{
}

void Ieee80211IdealReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
    }
}

std::ostream& Ieee80211IdealReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211IdealReceiver";
    return IdealReceiver::printToStream(stream, level);
}

const ReceptionIndication *Ieee80211IdealReceiver::computeReceptionIndication(const ISNIR *snir) const
{
    auto indication = new Ieee80211ReceptionIndication();
    auto reception = check_and_cast<const IdealReception *>(snir->getReception());
    auto noise = check_and_cast_nullable<const IdealNoise *>(snir->getNoise());
    double errorRate = reception->getPower() == IdealReception::POWER_RECEIVABLE && (noise == nullptr || !noise->isInterfering()) ? 0 : 1;
    indication->setSymbolErrorRate(errorRate);
    indication->setBitErrorRate(errorRate);
    indication->setPacketErrorRate(errorRate);
    // TODO: should match and get the mode and channel from the receiver
    const Ieee80211TransmissionBase *transmission = check_and_cast<const Ieee80211TransmissionBase *>(snir->getReception()->getTransmission());
    indication->setMode(transmission->getMode());
    indication->setChannel(const_cast<Ieee80211Channel *>(transmission->getChannel()));
    return indication;
}

} // namespace physicallayer

} // namespace inet

