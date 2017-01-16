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
#include "inet/physicallayer/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKPhyHeader_m.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKScalarTransmitter.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKScalarTransmitter);

APSKScalarTransmitter::APSKScalarTransmitter() :
    FlatTransmitterBase()
{
}

std::ostream& APSKScalarTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKScalarTransmitter";
    return FlatTransmitterBase::printToStream(stream, level);
}

const ITransmission *APSKScalarTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekHeader<APSKPhyHeader>();
    auto dataBitLength = packet->getBitLength() - phyHeader->getChunkLength() * 8;
    W transmissionPower = computeTransmissionPower(packet);
    Hz transmissionCarrierFrequency = computeCarrierFrequency(packet);
    Hz transmissionBandwidth = computeBandwidth(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = headerBitLength / transmissionBitrate.get();
    const simtime_t dataDuration = dataBitLength / transmissionBitrate.get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new ScalarTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, packet->getBitLength(), transmissionCarrierFrequency, transmissionBandwidth, transmissionBitrate, transmissionPower);
}

} // namespace physicallayer

} // namespace inet

