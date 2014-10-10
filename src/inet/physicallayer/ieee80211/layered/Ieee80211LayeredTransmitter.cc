//
// Copyright (C) 2014 OpenSim Ltd.
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
#include "inet/physicallayer/ieee80211/layered/Ieee80211LayeredTransmitter.h"
#include "inet/physicallayer/layered/LayeredTransmission.h"
#include "inet/physicallayer/layered/SignalPacketModel.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211PHYFrame_m.h"

namespace inet {

namespace physicallayer {

Ieee80211LayeredTransmitter::Ieee80211LayeredTransmitter() :
    encoder(NULL),
    modulator(NULL),
    pulseShaper(NULL),
    digitalAnalogConverter(NULL)
{
}

void Ieee80211LayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        encoder = check_and_cast<IEncoder *>(getSubmodule("encoder"));
        modulator = check_and_cast<IModulator *>(getSubmodule("modulator"));
        pulseShaper = check_and_cast<IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = check_and_cast<IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        const char *codeRate = par("codeRate").stringValue();
        setCodeRateParameters(codeRate);
    }
}

const ITransmissionPacketModel* Ieee80211LayeredTransmitter::createPacketModel(const cPacket* macFrame) const
{
    // const RadioTransmissionRequest *controlInfo = dynamic_cast<const RadioTransmissionRequest *>(macFrame->getControlInfo());
    // bps bitRate = controlInfo->getBitrate(); // TODO: Calculate the convolutional code rate from bitRate
    // The PCLP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    int plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16;
    Ieee80211PHYFrame * phyFrame = new Ieee80211PHYFrame();
    phyFrame->setRate1(codeRateK);
    phyFrame->setRate2(codeRateN);
    phyFrame->setLength(macFrame->getByteLength());
    phyFrame->encapsulate(macFrame->dup()); // TODO: fix this memory leak
    phyFrame->setBitLength(phyFrame->getLength() + plcpHeaderLength);
    const ITransmissionPacketModel *packetModel = new TransmissionPacketModel(phyFrame);
    return packetModel;
}

void Ieee80211LayeredTransmitter::setCodeRateParameters(const char *codeRate)
{
    cStringTokenizer tokenizer(codeRate,"/");
    if (tokenizer.hasMoreTokens())
        codeRateK = atoi(tokenizer.nextToken());
    if (tokenizer.hasMoreTokens())
        codeRateN = atoi(tokenizer.nextToken());
    else
        throw cRuntimeError("Code rate parameter parsing failed");
}

const ITransmission *Ieee80211LayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
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
