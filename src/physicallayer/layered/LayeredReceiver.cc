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

#include "LayeredReceiver.h"

namespace inet {

namespace physicallayer {

LayeredReceiver::LayeredReceiver() :
    decoder(NULL),
    demodulator(NULL),
    pulseFilter(NULL),
    analogDigitalConverter(NULL)
{
}

void LayeredReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        decoder = check_and_cast<IDecoder *>(getSubmodule("decoder"));
        demodulator = check_and_cast<IDemodulator *>(getSubmodule("demodulator"));
        pulseFilter = check_and_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = check_and_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}


//const IListening *LayeredReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
//{
//    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
//}
//
//const IListeningDecision *LayeredReceiver::computeListeningDecision(const IListening *listening, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
//{
//    const INoise *noise = computeNoise(listening, interferingReceptions, backgroundNoise);
//    const FlatNoiseBase *flatNoise = check_and_cast<const FlatNoiseBase *>(noise);
//    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
//    delete noise;
//    return new ListeningDecision(listening, maxPower >= energyDetection);
//}

const IReceptionDecision *LayeredReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    throw cRuntimeError("Not yet implemented");
//    const ITransmission *transmission = frame->getTransmission();
//    const IListening *listening = createListening(transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
//
//    const IXXX *receptionAndNoise = channel->receiveFromChannel(this, listening, transmission);
//    const INoise *noise = receptionAndNoise->getNoise();
//    const IReception *reception = receptionAndNoise->getReception();
//    double snirMin = computeSNIR(reception, noise);
//    bool isReceptionPossible = computeIsReceptionPossible(reception, interferingReceptions);
//    bool isReceptionSuccessful = isReceptionPossible && snirMin > snirThreshold;
//    return new RadioReceptionDecision(reception, isReceptionPossible, isReceptionSuccessful);
//    const IReception *receptionXXX = NULL;
//    const IReceptionAnalogModel *analogModel = NULL; // receptionXXX->getAnalogModel();
//    const IReceptionSampleModel *sampleModel = analogDigitalConverter->convertAnalogToDigital(analogModel);
//    const IReceptionSymbolModel *symbolModel = pulseFilter->filter(sampleModel);
//    const IReceptionBitModel *bitModel = demodulator->demodulate(symbolModel);
//    const IReceptionPacketModel *packetModel = decoder->decode(bitModel);
//    simtime_t startTime = 0; // TODO:
//    cPacket *packet = check_and_cast<cPacket *>(frame)->decapsulate();
//    simtime_t endTime = startTime + analogModel->getDuration();
//    IMobility *mobility = receiverAntenna->getMobility();
//    Coord startPosition = mobility->getPosition(startTime);
//    Coord endPosition = mobility->getPosition(endTime);
//    const IReception *reception = createReception(transmission, packetModel, bitModel, symbolModel, sampleModel, analogModel, startTime, endTime, startPosition, endPosition);
//    return new LayeredRadioReception(packetModel, bitModel, symbolModel, sampleModel, analogModel, radio, transmission, startTime, endTime, startPosition, endPosition);
}

} // namespace physicallayer

} // namespace inet
