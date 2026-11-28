//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"

namespace inet {
namespace physicallayer {

FlatReceiverBase::FlatReceiverBase() :
    NarrowbandReceiverBase(),
    errorModel(nullptr),
    energyDetection(W(NaN)),
    sensitivity(W(NaN))
{
}

void FlatReceiverBase::initialize(int stage)
{
    NarrowbandReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));
    }
}

std::ostream& FlatReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(errorModel, printFieldToString(errorModel, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(energyDetection)
               << EV_FIELD(sensitivity);
    return NarrowbandReceiverBase::printToStream(stream, level);
}

const IListeningDecision *FlatReceiverBase::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiverRadio();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IMediumAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);

    W maxPower = noise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing whether listening is possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

const IReceptionDecision *FlatReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    if (errorModel == nullptr)
        return NarrowbandReceiverBase::computeReceptionDecision(listening, reception, part, interference, snir);
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception, part);
    bool isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, part, interference);
    bool isReceptionSuccessful = false;
    std::vector<ErrorRegion> errorRegions;
    // a decision draws only for a reception that the receiver attempts
    if (isReceptionAttempted) {
        auto radio = reception->getReceiverRadio();
        auto medium = radio->getMedium();
        auto transmission = reception->getTransmission();
        // the preamble has no regions and no error rate: only its SNIR decides
        auto isPreambleSuccessful = [&] {
            simtime_t startTime = reception->getStartTime(IRadioSignal::SIGNAL_PART_PREAMBLE);
            simtime_t endTime = reception->getEndTime(IRadioSignal::SIGNAL_PART_PREAMBLE);
            return startTime == endTime || computeIsReceptionSuccessful(listening, reception, IRadioSignal::SIGNAL_PART_PREAMBLE, interference, snir, startTime, endTime);
        };
        // each part needs the part before it; the radio medium keeps the decision of each part
        switch (part) {
            case IRadioSignal::SIGNAL_PART_PREAMBLE:
                isReceptionSuccessful = isPreambleSuccessful();
                break;
            case IRadioSignal::SIGNAL_PART_HEADER:
                isReceptionSuccessful = isPreambleSuccessful() && computeIsPartSuccessful(listening, reception, part, interference, snir, errorRegions);
                break;
            case IRadioSignal::SIGNAL_PART_DATA:
                isReceptionSuccessful = medium->getReceptionDecision(radio, listening, transmission, IRadioSignal::SIGNAL_PART_HEADER)->isReceptionSuccessful() &&
                                        computeIsPartSuccessful(listening, reception, part, interference, snir, errorRegions);
                break;
            case IRadioSignal::SIGNAL_PART_WHOLE: {
                // the whole signal keeps the failed regions of the data, which the received packet gets
                auto dataDecision = medium->getReceptionDecision(radio, listening, transmission, IRadioSignal::SIGNAL_PART_DATA);
                isReceptionSuccessful = dataDecision->isReceptionSuccessful();
                errorRegions = dataDecision->getErrorRegions();
                break;
            }
            default:
                throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
        }
    }
    return new ReceptionDecision(reception, part, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful, errorRegions);
}

bool FlatReceiverBase::computeIsPartSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir, std::vector<ErrorRegion>& errorRegions) const
{
    auto transmission = reception->getTransmission();
    auto range = computePartRange(transmission, part);
    // without the complete part in the packet, the receiver cannot decode the part
    if (!range)
        return false;
    // a part without bits has no region
    if (range->length == b(0))
        return true;
    // a part that the packet does not hold is one region, the packet mode has one
    // region for each part, and the other modes have one region for each chunk
    std::vector<std::pair<b, b>> regions;
    if (!range->isInPacket || errorModel->getCorruptionMode() == IErrorModel::CorruptionMode::CM_PACKET)
        regions.push_back({range->offset, range->length});
    else {
        const auto& content = transmission->getPacket()->peekAt(range->offset, range->length, Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
        b offset = range->offset;
        auto addRegion = [&] (const Ptr<const Chunk>& chunk) {
            regions.push_back({offset, chunk->getChunkLength()});
            offset += chunk->getChunkLength();
        };
        if (content->getChunkType() == Chunk::CT_SEQUENCE) {
            for (const auto& chunk : staticPtrCast<const SequenceChunk>(content)->getChunks())
                addRegion(chunk);
        }
        else
            addRegion(content);
    }
    bool isSuccessful = true;
    for (const auto& region : regions) {
        b offset = region.first;
        b length = region.second;
        auto [startTime, endTime] = computeRegionInterval(reception, part, offset - range->offset, length, range->length);
        // the SNIR threshold is a condition, and the error rate is a probability
        bool isFailed = !computeIsReceptionSuccessful(listening, reception, part, interference, snir, startTime, endTime);
        if (!isFailed) {
            double errorRate = errorModel->computePacketErrorRate(snir, part, length, startTime, endTime);
            if (!(0 <= errorRate && errorRate <= 1))
                throw cRuntimeError("Invalid error rate of a region: %g", errorRate);
            isFailed = errorRate != 0 && (errorRate == 1 || !(dblrand() > errorRate));
        }
        if (isFailed) {
            isSuccessful = false;
            if (range->isInPacket)
                errorRegions.push_back({offset, length, errorModel->computeBitErrorRate(snir, part, length, startTime, endTime)});
        }
    }
    return isSuccessful;
}

const IReceptionResult *FlatReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    auto receptionResult = NarrowbandReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions);
    if (errorModel != nullptr) {
        auto errorRateInd = const_cast<Packet *>(receptionResult->getPacket())->addTagIfAbsent<ErrorRateInd>();
        errorRateInd->setPacketErrorRate(errorModel->computePacketErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE));
        errorRateInd->setBitErrorRate(errorModel->computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE));
        errorRateInd->setSymbolErrorRate(errorModel->computeSymbolErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE));
    }
    return receptionResult;
}

Packet *FlatReceiverBase::computeReceivedPacket(const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    // the decision of the whole signal keeps the failed regions that the received packet gets
    for (auto decision : *decisions) {
        if (decision->getSignalPart() == IRadioSignal::SIGNAL_PART_WHOLE && !decision->getErrorRegions().empty()) {
            auto transmittedPacket = snir->getReception()->getTransmission()->getPacket();
            auto receivedPacket = errorModel->computeCorruptedPacket(transmittedPacket, decision->getErrorRegions());
            receivedPacket->clearTags();
            receivedPacket->addTag<PacketProtocolTag>()->setProtocol(transmittedPacket->getTag<PacketProtocolTag>()->getProtocol());
            return receivedPacket;
        }
    }
    return NarrowbandReceiverBase::computeReceivedPacket(snir, decisions);
}

std::optional<FlatReceiverBase::PartRange> FlatReceiverBase::computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const
{
    throw cRuntimeError("%s does not name the bits of the signal parts, which the error decisions of the signal regions need", getClassName());
}

std::pair<simtime_t, simtime_t> FlatReceiverBase::computeRegionInterval(const IReception *reception, IRadioSignal::SignalPart part, b offset, b length, b partLength) const
{
    simtime_t partStartTime = reception->getStartTime(part);
    simtime_t partDuration = reception->getEndTime(part) - partStartTime;
    simtime_t startTime = partStartTime + partDuration * (offset.get<b>() / (double)partLength.get<b>());
    simtime_t endTime = partStartTime + partDuration * ((offset + length).get<b>() / (double)partLength.get<b>());
    return {startTime, endTime};
}

} // namespace physicallayer
} // namespace inet

