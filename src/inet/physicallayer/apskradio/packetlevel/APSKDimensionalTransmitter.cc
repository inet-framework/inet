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
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKDimensionalTransmitter.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKDimensionalTransmitter);

APSKDimensionalTransmitter::APSKDimensionalTransmitter() :
    DimensionalTransmitterBase(),
    FlatTransmitterBase()
{
}

void APSKDimensionalTransmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    DimensionalTransmitterBase::initialize(stage);
}

std::ostream& APSKDimensionalTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKDimensionalTransmitter";
    FlatTransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *APSKDimensionalTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekHeader<APSKPhyHeader>();
    auto dataLength = packet->getPacketLength() - phyHeader->getChunkLength();
    W transmissionPower = computeTransmissionPower(packet);
    Hz transmissionCarrierFrequency = computeCarrierFrequency(packet);
    Hz transmissionBandwidth = computeBandwidth(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = bit(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = bit(dataLength).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, transmissionPower);
    return new DimensionalTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, headerLength, dataLength, transmissionBitrate, modulation, transmissionCarrierFrequency, transmissionBandwidth, powerMapping);
}

} // namespace physicallayer

} // namespace inet

