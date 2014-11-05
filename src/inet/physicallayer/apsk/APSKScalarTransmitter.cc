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

#include "inet/physicallayer/apsk/APSKScalarTransmitter.h"
#include "inet/physicallayer/analogmodel/ScalarTransmission.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKScalarTransmitter);

APSKScalarTransmitter::APSKScalarTransmitter() :
    NarrowbandTransmitterBase()
{
}

void APSKScalarTransmitter::printToStream(std::ostream& stream) const
{
    stream << "APSKScalarTransmitter, ";
    NarrowbandTransmitterBase::printToStream(stream);
}

const ITransmission *APSKScalarTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    RadioTransmissionRequest *controlInfo = dynamic_cast<RadioTransmissionRequest *>(macFrame->getControlInfo());
    W transmissionPower = controlInfo && !isNaN(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    bps transmissionBitrate = controlInfo && !isNaN(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    const simtime_t duration = (macFrame->getBitLength() + headerBitLength) / transmissionBitrate.get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new ScalarTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, transmissionBitrate, transmissionPower);
}

} // namespace physicallayer

} // namespace inet

