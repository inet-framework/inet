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
#include "inet/physicallayer/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee802154/packetlevel/Ieee802154DimensionalTransmission.h"
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
    FlatTransmitterBase::printToStream(stream, level);
    return DimensionalTransmitterBase::printToStream(stream, level);
}

const ITransmission *Ieee802154NarrowbandDimensionalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    TransmissionRequest *controlInfo = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    W transmissionPower = controlInfo && !std::isnan(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    bps transmissionBitrate = controlInfo && !std::isnan(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    const simtime_t headerDuration = headerBitLength / transmissionBitrate.get();
    const simtime_t dataDuration = macFrame->getBitLength() / transmissionBitrate.get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, transmissionPower);
    return new Ieee802154DimensionalTransmission(transmitter, macFrame, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, transmissionBitrate, powerMapping);
}

} // namespace physicallayer

} // namespace inet
