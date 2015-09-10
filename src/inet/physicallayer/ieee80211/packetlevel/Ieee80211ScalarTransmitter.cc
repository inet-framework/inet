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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmitter.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211ScalarTransmitter);

Ieee80211ScalarTransmitter::Ieee80211ScalarTransmitter() :
    Ieee80211TransmitterBase()
{
}

std::ostream& Ieee80211ScalarTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211ScalarTransmitter";
    return Ieee80211TransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211ScalarTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    const TransmissionRequest *transmissionRequest = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(transmissionRequest);
    const Ieee80211Channel *transmissionChannel = computeTransmissionChannel(transmissionRequest);
    W transmissionPower = computeTransmissionPower(transmissionRequest);
    bps transmissionBitrate = transmissionMode->getDataMode()->getNetBitrate();
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    const simtime_t duration = transmissionMode->getDuration(macFrame->getBitLength());
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    int headerBitLength = transmissionMode->getHeaderMode()->getBitLength();
    int64_t payloadBitLength = macFrame->getBitLength();
    return new Ieee80211ScalarTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, payloadBitLength, carrierFrequency, bandwidth, transmissionBitrate, transmissionPower, transmissionMode, transmissionChannel);
}

} // namespace physicallayer

} // namespace inet

