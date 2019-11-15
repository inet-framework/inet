//
// Copyright (C) 2014 Florian Meier
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
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee802154/packetlevel/Ieee802154NarrowbandDimensionalTransmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandDimensionalTransmitter);

Ieee802154NarrowbandDimensionalTransmitter::Ieee802154NarrowbandDimensionalTransmitter() :
    FlatTransmitterBase(),
    DimensionalTransmitterBase()
{
}

void Ieee802154NarrowbandDimensionalTransmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    DimensionalTransmitterBase::initialize(stage);
}

std::ostream& Ieee802154NarrowbandDimensionalTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee802154NarrowbandDimensionalTransmitter";
    DimensionalTransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee802154NarrowbandDimensionalTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    W transmissionPower = computeTransmissionPower(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = b(packet->getTotalLength()).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = createPowerFunction(startTime, endTime, centerFrequency, bandwidth, transmissionPower);
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();
    return new DimensionalTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerLength, packet->getTotalLength(), centerFrequency, bandwidth, transmissionBitrate, powerFunction);
}

} // namespace physicallayer

} // namespace inet

