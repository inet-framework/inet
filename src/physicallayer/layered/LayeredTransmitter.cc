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

#include "IMobility.h"
#include "LayeredTransmitter.h"
#include "LayeredTransmission.h"
#include "SignalPacketModel.h"

namespace inet {

namespace physicallayer {

LayeredTransmitter::LayeredTransmitter() :
    encoder(NULL),
    modulator(NULL),
    pulseShaper(NULL),
    digitalAnalogConverter(NULL)
{
}

void LayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        encoder = check_and_cast<IEncoder *>(getSubmodule("encoder"));
        modulator = check_and_cast<IModulator *>(getSubmodule("modulator"));
        pulseShaper = check_and_cast<IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = check_and_cast<IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
    }
}

const ITransmissionPacketModel* LayeredTransmitter::createPacketModel(const cPacket *macFrame) const
{
    const ITransmissionPacketModel *packetModel = new TransmissionPacketModel(macFrame);
    return packetModel;
}

const ITransmission *LayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    const ITransmissionBitModel *bitModel = encoder->encode(packetModel);
    const ITransmissionSymbolModel *symbolModel = modulator->modulate(bitModel);
    const ITransmissionSampleModel *sampleModel = pulseShaper->shape(symbolModel);
    const ITransmissionAnalogModel *analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation);
}

} // namespace physicallayer

} // namespace inet
