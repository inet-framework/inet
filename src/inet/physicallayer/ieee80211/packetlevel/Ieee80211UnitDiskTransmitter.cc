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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskTransmitter.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211UnitDiskTransmitter);

Ieee80211UnitDiskTransmitter::Ieee80211UnitDiskTransmitter() :
    Ieee80211TransmitterBase()
{
}

void Ieee80211UnitDiskTransmitter::initialize(int stage)
{
    Ieee80211TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        communicationRange = m(par("communicationRange"));
        interferenceRange = m(par("interferenceRange"));
        detectionRange = m(par("detectionRange"));
    }
}

std::ostream& Ieee80211UnitDiskTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211UnitDiskTransmitter";
    if (level <= PRINT_LEVEL_INFO)
        stream << ", communicationRange = " << communicationRange;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", interferenceRange = " << interferenceRange
               << ", detectionRange = " << detectionRange;
    return Ieee80211TransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211UnitDiskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, simtime_t startTime) const
{
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(packet);
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    const simtime_t duration = transmissionMode->getDuration(packet->getDataLength());
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();
    const simtime_t preambleDuration = transmissionMode->getPreambleMode()->getDuration();
    const simtime_t headerDuration = transmissionMode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = duration - headerDuration - preambleDuration;
    return new Ieee80211UnitDiskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, communicationRange, interferenceRange, detectionRange, transmissionMode, channel);
}

} // namespace physicallayer
} // namespace inet

