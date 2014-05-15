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

#include "LayeredImplementation.h"

void RadioSignalPacketModel::printToStream(std::ostream &stream) const
{
    stream << packet;
}

void RadioSignalBitModel::printToStream(std::ostream &stream) const
{
    stream << forwardErrorCorrection << ", bit length = " << bitLength;
}

void RadioSignalSymbolModel::printToStream(std::ostream &stream) const
{
    stream << modulation;
}

void RadioSignalSampleModel::printToStream(std::ostream &stream) const
{
}

void RadioSignalAnalogModel::printToStream(std::ostream &stream) const
{
    stream << "duration = " << duration;
}

void ForwardErrorCorrection::printToStream(std::ostream &stream) const
{
    stream << "rate = " << inputBitLength << " / " << outputBitLength;
}

void RadioSignalModulation::printToStream(std::ostream &stream) const
{
    stream << "type = " << type << ", constellation size = " << constellationSize;
}

void ScalarRadioSignalAnalogModel::printToStream(std::ostream &stream) const
{
    stream << "power = " << power << ", carrier frequency = " << carrierFrequency << ", bandwidth = " << bandwidth << ", ";
    RadioSignalAnalogModel::printToStream(stream);
}

const IRadioSignalTransmission *LayeredRadioSignalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const IRadioSignalTransmissionPacketModel *packetModel = new RadioSignalTransmissionPacketModel(macFrame);
    const IRadioSignalTransmissionBitModel *bitModel = encoder->encode(packetModel);
    const IRadioSignalTransmissionSymbolModel *symbolModel = modulator->modulate(bitModel);
    const IRadioSignalTransmissionSampleModel *sampleModel = pulseShaper->shape(symbolModel);
    const IRadioSignalTransmissionAnalogModel *analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    const simtime_t endTime = startTime + analogModel->getDuration();
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getPosition(startTime);
    const Coord endPosition = mobility->getPosition(endTime);
    return new LayeredRadioSignalTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition);
}

const IRadioSignalListening *LayeredRadioSignalReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalListeningDecision *LayeredRadioSignalReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    throw cRuntimeError("Not yet implemented");
}

const IRadioSignalReceptionDecision *LayeredRadioSignalReceiver::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    throw cRuntimeError("Not yet implemented");
//    const IRadioSignalTransmission *transmission = frame->getTransmission();
//    const IRadioSignalListening *listening = createListening(transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
//
//    const IRadioSignalXXX *receptionAndNoise = channel->receiveFromChannel(this, listening, transmission);
//    const IRadioSignalNoise *noise = receptionAndNoise->getNoise();
//    const IRadioSignalReception *reception = receptionAndNoise->getReception();
//    double snirMin = computeSNIR(reception, noise);
//    bool isReceptionPossible = computeIsReceptionPossible(reception, interferingReceptions);
//    bool isReceptionSuccessful = isReceptionPossible && snirMin > snirThreshold;
//    return new RadioSignalReceptionDecision(reception, isReceptionPossible, isReceptionSuccessful);
//    const IRadioSignalReception *receptionXXX = NULL;
//    const IRadioSignalReceptionAnalogModel *analogModel = NULL; // receptionXXX->getAnalogModel();
//    const IRadioSignalReceptionSampleModel *sampleModel = analogDigitalConverter->convertAnalogToDigital(analogModel);
//    const IRadioSignalReceptionSymbolModel *symbolModel = pulseFilter->filter(sampleModel);
//    const IRadioSignalReceptionBitModel *bitModel = demodulator->demodulate(symbolModel);
//    const IRadioSignalReceptionPacketModel *packetModel = decoder->decode(bitModel);
//    simtime_t startTime = 0; // TODO:
//    cPacket *packet = check_and_cast<cPacket *>(frame)->decapsulate();
//    simtime_t endTime = startTime + analogModel->getDuration();
//    IMobility *mobility = receiverAntenna->getMobility();
//    Coord startPosition = mobility->getPosition(startTime);
//    Coord endPosition = mobility->getPosition(endTime);
//    const IRadioSignalReception *reception = createReception(transmission, packetModel, bitModel, symbolModel, sampleModel, analogModel, startTime, endTime, startPosition, endPosition);
//    return new LayeredRadioSignalReception(packetModel, bitModel, symbolModel, sampleModel, analogModel, radio, transmission, startTime, endTime, startPosition, endPosition);
}
