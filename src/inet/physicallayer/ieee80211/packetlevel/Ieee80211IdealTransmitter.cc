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
#include "inet/physicallayer/idealradio/IdealTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211IdealTransmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211IdealTransmitter);

Ieee80211IdealTransmitter::Ieee80211IdealTransmitter() :
    Ieee80211TransmitterBase()
{
}

void Ieee80211IdealTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        maxCommunicationRange = m(par("maxCommunicationRange"));
        maxInterferenceRange = m(par("maxInterferenceRange"));
        maxDetectionRange = m(par("maxDetectionRange"));
    }
}

std::ostream& Ieee80211IdealTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211IdealTransmitter";
    if (level >= PRINT_LEVEL_INFO)
        stream << ", maxCommunicationRange = " << maxCommunicationRange;
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", maxInterferenceRange = " << maxInterferenceRange
               << ", maxDetectionRange = " << maxDetectionRange;
    return Ieee80211TransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee80211IdealTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    const TransmissionRequest *transmissionRequest = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    const IIeee80211Mode *transmissionMode = computeTransmissionMode(transmissionRequest);
    if (transmissionMode->getDataMode()->getNumberOfSpatialStreams() > transmitter->getAntenna()->getNumAntennas())
        throw cRuntimeError("Number of spatial streams is higher than the number of antennas");
    const simtime_t duration = transmissionMode->getDuration(macFrame->getBitLength());
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const simtime_t preambleDuration = transmissionMode->getPreambleMode()->getDuration();
    const simtime_t headerDuration = transmissionMode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = duration - headerDuration - preambleDuration;
    return new IdealTransmission(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, maxCommunicationRange, maxInterferenceRange, maxDetectionRange);
}

} // namespace physicallayer

} // namespace inet

