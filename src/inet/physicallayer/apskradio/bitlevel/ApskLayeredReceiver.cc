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

#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarAnalogModel.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskDecoder.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskDemodulator.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskLayeredReceiver.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/bitlevel/LayeredReception.h"
#include "inet/physicallayer/common/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace physicallayer {

Define_Module(ApskLayeredReceiver);

ApskLayeredReceiver::ApskLayeredReceiver() :
    levelOfDetail(static_cast<LevelOfDetail>(-1)),
    errorModel(nullptr),
    decoder(nullptr),
    demodulator(nullptr),
    pulseFilter(nullptr),
    analogDigitalConverter(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN)),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN)),
    snirThreshold(NaN)
{
}

void ApskLayeredReceiver::initialize(int stage)
{
    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        decoder = dynamic_cast<IDecoder *>(getSubmodule("decoder"));
        demodulator = dynamic_cast<IDemodulator *>(getSubmodule("demodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));
        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
        // TODO: temporary parameters
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
        const char *levelOfDetailStr = par("levelOfDetail");
        if (strcmp("packet", levelOfDetailStr) == 0)
            levelOfDetail = PACKET_DOMAIN;
        else if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
        if (levelOfDetail >= BIT_DOMAIN && !decoder)
            throw cRuntimeError("Decoder not configured");
        if (levelOfDetail >= SYMBOL_DOMAIN && !demodulator)
            throw cRuntimeError("Demodulator not configured");
        if (levelOfDetail >= SAMPLE_DOMAIN && !pulseFilter)
            throw cRuntimeError("Pulse filter not configured");
    }
}

const IReceptionAnalogModel *ApskLayeredReceiver::createAnalogModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    // TODO: interference + receptionAnalogModel;
    return nullptr;
}

std::ostream& ApskLayeredReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "ApskLayeredReceiver";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", levelOfDetail = " << levelOfDetail
               << ", centerFrequency = " << centerFrequency;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", errorModel = " << printObjectToString(errorModel, level + 1)
               << ", decoder = " << printObjectToString(decoder, level + 1)
               << ", demodulator = " << printObjectToString(demodulator, level + 1)
               << ", pulseFilter = " << printObjectToString(pulseFilter, level + 1)
               << ", analogDigitalConverter = " << printObjectToString(analogDigitalConverter, level + 1)
               << ", energyDetection = " << energyDetection
               << ", sensitivity = " << sensitivity
               << ", bandwidth = " << bandwidth
               << ", snirThreshold = " << snirThreshold;
    return stream;
}

const IReceptionSampleModel *ApskLayeredReceiver::createSampleModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionAnalogModel *analogModel) const
{
    if (levelOfDetail == SAMPLE_DOMAIN)
        return errorModel->computeSampleModel(transmission, snir);
    else if (analogDigitalConverter)
        return analogDigitalConverter->convertAnalogToDigital(analogModel);
    else
        return nullptr;
}

const IReceptionSymbolModel *ApskLayeredReceiver::createSymbolModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionSampleModel *sampleModel) const
{
    if (levelOfDetail == SYMBOL_DOMAIN)
        return errorModel->computeSymbolModel(transmission, snir);
    else if (levelOfDetail >= SAMPLE_DOMAIN)
        return pulseFilter->filter(sampleModel);
    else
        return nullptr;
}

const IReceptionBitModel *ApskLayeredReceiver::createBitModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionSymbolModel *symbolModel) const
{
    if (levelOfDetail == BIT_DOMAIN)
        return errorModel->computeBitModel(transmission, snir);
    else if (levelOfDetail >= SYMBOL_DOMAIN)
        return demodulator->demodulate(symbolModel);
    else
        return nullptr;
}

const IReceptionPacketModel *ApskLayeredReceiver::createPacketModel(const LayeredTransmission *transmission, const ISnir *snir, const IReceptionBitModel *bitModel) const
{
    if (levelOfDetail == PACKET_DOMAIN)
        return errorModel->computePacketModel(transmission, snir);
    else if (levelOfDetail >= BIT_DOMAIN)
        return decoder->decode(bitModel);
    else
        return nullptr;
}

const IReceptionResult *ApskLayeredReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission *>(reception->getTransmission());
    const IReceptionAnalogModel *analogModel = createAnalogModel(transmission, snir);
    const IReceptionSampleModel *sampleModel = createSampleModel(transmission, snir, analogModel);
    const IReceptionSymbolModel *symbolModel = createSymbolModel(transmission, snir, sampleModel);
    const IReceptionBitModel *bitModel = createBitModel(transmission, snir, symbolModel);
    const IReceptionPacketModel *packetModel = createPacketModel(transmission, snir, bitModel);
    auto packet = const_cast<Packet *>(packetModel->getPacket());
    packet->addTagIfAbsent<ErrorRateInd>(); // TODO: setPacketErrorRate(per);
    auto snirInd = packet->addTagIfAbsent<SnirInd>();
    snirInd->setMinimumSnir(snir->getMin());
    snirInd->setMaximumSnir(snir->getMax());
    snirInd->setAverageSnir(snir->getMean());
    return new LayeredReceptionResult(reception, decisions, packetModel, bitModel, symbolModel, sampleModel, analogModel);
}

const IListening *ApskLayeredReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);
}

// TODO: copy
const IListeningDecision *ApskLayeredReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *flatNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing listening possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

// TODO: this is not purely functional, see interface comment
// TODO: copy
bool ApskLayeredReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const LayeredReception *scalarReception = check_and_cast<const LayeredReception *>(reception);
    // TODO: scalar
    const ScalarReceptionSignalAnalogModel *analogModel = check_and_cast<const ScalarReceptionSignalAnalogModel *>(scalarReception->getAnalogModel());
    if (bandListening->getCenterFrequency() != analogModel->getCenterFrequency() || bandListening->getBandwidth() != analogModel->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(scalarReception->getAnalogModel());
        W minReceptionPower = narrowbandSignalAnalogModel->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

} // namespace physicallayer
} // namespace inet

