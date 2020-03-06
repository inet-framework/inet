//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/bitlevel/ApskLayeredReceiver.h"

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskDecoder.h"
#include "inet/physicallayer/wireless/apsk/bitlevel/ApskDemodulator.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"

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
        // TODO temporary parameters
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
    // TODO interference + receptionAnalogModel;
    return nullptr;
}

std::ostream& ApskLayeredReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskLayeredReceiver";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(levelOfDetail)
               << EV_FIELD(centerFrequency);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(errorModel, printFieldToString(errorModel, level + 1, evFlags))
               << EV_FIELD(decoder, printFieldToString(decoder, level + 1, evFlags))
               << EV_FIELD(demodulator, printFieldToString(demodulator, level + 1, evFlags))
               << EV_FIELD(pulseFilter, printFieldToString(pulseFilter, level + 1, evFlags))
               << EV_FIELD(analogDigitalConverter, printFieldToString(analogDigitalConverter, level + 1, evFlags))
               << EV_FIELD(energyDetection)
               << EV_FIELD(sensitivity)
               << EV_FIELD(bandwidth)
               << EV_FIELD(snirThreshold);
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
    auto errorRateInd = packet->addTagIfAbsent<ErrorRateInd>();
    if (symbolModel != nullptr)
        errorRateInd->setSymbolErrorRate(symbolModel->getSymbolErrorRate());
    if (bitModel != nullptr)
        errorRateInd->setBitErrorRate(bitModel->getBitErrorRate());
    if (packetModel != nullptr)
        errorRateInd->setPacketErrorRate(packetModel->getPacketErrorRate());
    auto snirInd = packet->addTagIfAbsent<SnirInd>();
    snirInd->setMinimumSnir(snir->getMin());
    snirInd->setMaximumSnir(snir->getMax());
    snirInd->setAverageSnir(snir->getMean());
    return new LayeredReceptionResult(reception, decisions, packetModel, bitModel, symbolModel, sampleModel, analogModel);
}

const IListening *ApskLayeredReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, bandwidth);
}

// TODO copy
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

bool ApskLayeredReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto layeredTransmission = dynamic_cast<const LayeredTransmission *>(transmission);
    return layeredTransmission && SnirReceiverBase::computeIsReceptionPossible(listening, transmission);
}

// TODO this is not purely functional, see interface comment
// TODO copy
bool ApskLayeredReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const LayeredReception *scalarReception = check_and_cast<const LayeredReception *>(reception);
    // TODO scalar
    const ScalarReceptionSignalAnalogModel *analogModel = check_and_cast<const ScalarReceptionSignalAnalogModel *>(scalarReception->getAnalogModel());
    if (bandListening->getCenterFrequency() != analogModel->getCenterFrequency() || bandListening->getBandwidth() != analogModel->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(scalarReception->getAnalogModel());
        W minReceptionPower = narrowbandSignalAnalogModel->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible" << EV_FIELD(minReceptionPower) << EV_FIELD(sensitivity) << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

} // namespace physicallayer
} // namespace inet

