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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211DimensionalTransmitter.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211DimensionalTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211DimensionalTransmitter);

Ieee80211DimensionalTransmitter::Ieee80211DimensionalTransmitter() :
    DimensionalTransmitterBase(),
    modeSet(nullptr),
    mode(nullptr)
{
}

void Ieee80211DimensionalTransmitter::initialize(int stage)
{
    DimensionalTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        carrierFrequency = Hz(CENTER_FREQUENCIES[par("channelNumber")]);
        modeSet = Ieee80211ModeSet::getModeSet(*par("opMode").stringValue());
        mode = modeSet->getMode(bitrate);
    }
}

const ITransmission *Ieee80211DimensionalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    TransmissionRequest *transmissionRequest = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    Ieee80211TransmissionRequest *ieee80211TransmissionRequest = dynamic_cast<Ieee80211TransmissionRequest *>(transmissionRequest);
    W transmissionPower = transmissionRequest && !isNaN(transmissionRequest->getPower().get()) ? transmissionRequest->getPower() : power;
    bps transmissionBitrate = transmissionRequest && !isNaN(transmissionRequest->getBitrate().get()) ? transmissionRequest->getBitrate() : bitrate;
    const IIeee80211Mode *transmissionMode = ieee80211TransmissionRequest != nullptr ? ieee80211TransmissionRequest->getMode() : transmissionBitrate != bitrate ? modeSet->getMode(transmissionBitrate) : mode;
    const simtime_t duration = transmissionMode->getDuration(macFrame->getBitLength());
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, transmissionPower);
    int headerBitLength = mode->getHeaderMode()->getBitLength();
    int64_t payloadBitLength = macFrame->getBitLength();
    return new Ieee80211DimensionalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, payloadBitLength, carrierFrequency, bandwidth, transmissionBitrate, powerMapping, transmissionMode);
}

} // namespace physicallayer

} // namespace inet

