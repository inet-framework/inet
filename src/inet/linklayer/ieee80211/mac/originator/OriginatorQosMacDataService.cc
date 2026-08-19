//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"

#include <algorithm>
#include <memory>
#include <unordered_set>

#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211AddbaTransactionTag_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/QoSSequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQosMacDataService);

void OriginatorQosMacDataService::initialize()
{
    aMsduAggregationPolicy = dynamic_cast<IMsduAggregationPolicy *>(getSubmodule("msduAggregationPolicy"));
    if (aMsduAggregationPolicy)
        aMsduAggregation = new MsduAggregation();
    aMpduAggregationPolicy = dynamic_cast<IMpduAggregationPolicy *>(getSubmodule("mpduAggregationPolicy"));
    if (aMpduAggregationPolicy)
        aMpduAggregation = new MpduAggregation();
    sequenceNumberAssignment = new QoSSequenceNumberAssignment();
    fragmentationPolicy = dynamic_cast<IFragmentationPolicy *>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

Packet *OriginatorQosMacDataService::aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue, Packet *candidate)
{
    auto predicate = [this](const Packet *packet) { return isFrameEligible(packet); };
    std::unique_ptr<std::vector<Packet *>> subframes(aMsduAggregationPolicy->computeAggregateFrames(pendingQueue, candidate, predicate));
    if (subframes) {
        if (subframes->size() < 2 || subframes->front() != candidate)
            throw cRuntimeError("A-MSDU policy must return at least two frames with the selected candidate first");
        std::unordered_set<Packet *> availableFrames;
        for (int i = 0; i < pendingQueue->getNumPackets(); i++)
            availableFrames.insert(pendingQueue->getPacket(i));
        std::unordered_set<Packet *> uniqueFrames;
        for (auto subframe : *subframes) {
            if (subframe == nullptr || !uniqueFrames.insert(subframe).second || availableFrames.find(subframe) == availableFrames.end() || !isFrameEligible(subframe))
                throw cRuntimeError("A-MSDU policy returned a frame that is unavailable, ineligible, or duplicated");
        }
        struct AggregateFrameState { Tid tid; MacAddress receiver; MacAddress transmitter; MacAddress address3; MacAddress address4; int type; bool toDS; bool fromDS; b dataLength; b headerLength; b trailerLength; };
        std::vector<AggregateFrameState> states;
        for (auto subframe : *subframes) {
            auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(subframe->peekAtFront<Ieee80211DataOrMgmtHeader>());
            auto dataTrailer = subframe->peekAtBack<Ieee80211MacTrailer>(B(4));
            if (dataHeader == nullptr || dataTrailer == nullptr)
                throw cRuntimeError("A-MSDU policy selected a frame without a valid data header/trailer");
            states.push_back({static_cast<Tid>(dataHeader->getTid()), dataHeader->getReceiverAddress(), dataHeader->getTransmitterAddress(), dataHeader->getAddress3(), dataHeader->getAddress4(), dataHeader->getType(), dataHeader->getToDS(), dataHeader->getFromDS(), subframe->getDataLength(), dataHeader->getChunkLength(), dataTrailer->getChunkLength()});
        }
        std::vector<std::unique_ptr<Packet>> extractedSubframes;
        extractedSubframes.reserve(subframes->size());
        for (auto subframe : *subframes) {
            auto dequeuedSubframe = pendingQueue->dequeuePacket([subframe](const Packet *packet) { return packet == subframe; });
            if (dequeuedSubframe != subframe) {
                bool alreadyExtracted = std::any_of(extractedSubframes.begin(), extractedSubframes.end(), [dequeuedSubframe](const auto& extractedSubframe) { return extractedSubframe.get() == dequeuedSubframe; });
                if (dequeuedSubframe != nullptr && !alreadyExtracted) {
                    take(dequeuedSubframe);
                    extractedSubframes.emplace_back(dequeuedSubframe);
                }
                throw cRuntimeError("A-MSDU policy-selected subframe is no longer available in scheduling order");
            }
            take(dequeuedSubframe);
            extractedSubframes.emplace_back(dequeuedSubframe);
        }
        for (size_t i = 0; i < subframes->size(); i++) {
            auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>((*subframes)[i]->peekAtFront<Ieee80211DataOrMgmtHeader>());
            auto dataTrailer = (*subframes)[i]->peekAtBack<Ieee80211MacTrailer>(B(4));
            if (dataHeader == nullptr || dataTrailer == nullptr || dataHeader->getTid() != states[i].tid || dataHeader->getReceiverAddress() != states[i].receiver || dataHeader->getTransmitterAddress() != states[i].transmitter || dataHeader->getAddress3() != states[i].address3 || dataHeader->getAddress4() != states[i].address4 || dataHeader->getType() != states[i].type || dataHeader->getToDS() != states[i].toDS || dataHeader->getFromDS() != states[i].fromDS || (*subframes)[i]->getDataLength() != states[i].dataLength || dataHeader->getChunkLength() != states[i].headerLength || dataTrailer->getChunkLength() != states[i].trailerLength)
                throw cRuntimeError("A-MSDU provider changed aggregation-critical frame fields during extraction");
        }
        for (auto& subframe : extractedSubframes)
            subframe.release();
        auto aggregatedFrame = aMsduAggregation->aggregateFrames(subframes.get());
        emit(packetAggregatedSignal, aggregatedFrame);
        return aggregatedFrame;
    }
    return nullptr;
}

Packet *OriginatorQosMacDataService::aMpduAggregateIfNeeded(std::vector<Packet *> *fragments)
{
    auto subframes = aMpduAggregationPolicy->computeAggregateFrames(fragments);
    if (subframes) {
        for (auto f : *subframes)
            fragments->erase(std::remove(fragments->begin(), fragments->end(), f), fragments->end());
        auto aggregatedFrame = aMpduAggregation->aggregateFrames(subframes);
        emit(packetAggregatedSignal, aggregatedFrame);
        delete subframes;
        return aggregatedFrame;
    }
    return nullptr;
}

void OriginatorQosMacDataService::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    sequenceNumberAssignment->assignSequenceNumber(header);
}

std::vector<Packet *> *OriginatorQosMacDataService::fragmentIfNeeded(Packet *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        emit(packetFragmentedSignal, frame);
        auto transactionTag = frame->findTag<Ieee80211AddbaTransactionTag>();
        bool hasTransactionTag = transactionTag != nullptr;
        auto transactionId = hasTransactionTag ? transactionTag->getTransactionId() : 0;
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        if (hasTransactionTag)
            for (auto fragment : *fragmentFrames)
                fragment->addTagIfAbsent<Ieee80211AddbaTransactionTag>()->setTransactionId(transactionId);
        return fragmentFrames;
    }
    return nullptr;
}

bool OriginatorQosMacDataService::isFrameEligible(const Packet *packet) const
{
    return !frameEligibilityFunction || frameEligibilityFunction(packet);
}

bool OriginatorQosMacDataService::hasEligibleFrame(queueing::IPacketQueue *pendingQueue) const
{
    return pendingQueue->findPacket([this](const Packet *packet) { return isFrameEligible(packet); }) != nullptr;
}

std::vector<Packet *> *OriginatorQosMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    Enter_Method("extractFramesToTransmit");
    auto predicate = [this](const Packet *packet) { return isFrameEligible(packet); };
    auto candidate = pendingQueue->findPacket(predicate);
    if (candidate == nullptr)
        return nullptr;
    else {
//        if (msduRateLimiting)
//            txRateLimitingIfNeeded();
        Packet *packet = nullptr;
        // Scheduling selects the anchor; the policy may select additional
        // eligible members, which are all extracted through the provider.
        if (aMsduAggregationPolicy)
            packet = aMsduAggregateIfNeeded(pendingQueue, candidate);
        if (!packet) {
            packet = pendingQueue->dequeuePacket(predicate);
            ASSERT(packet == candidate);
            take(packet);
        }
        ASSERT(packet != nullptr);
        // PS Defer Queueing
        if (sequenceNumberAssignment) {
            auto header = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(header);
            packet->insertAtFront(header);
        }
//        if (msduIntegrityAndProtection)
//            frame = protectMsduIfNeeded(frame);
        std::vector<Packet *> *fragments = nullptr;
        if (fragmentationPolicy)
            fragments = fragmentIfNeeded(packet);
        if (!fragments)
            fragments = new std::vector<Packet *>({ packet });
//        if (mpduEncryptionAndIntegrity)
//            fragments = encryptMpduIfNeeded(fragments);
//        if (mpduHeaderPlusFcs)
//            fragments = mpduFcsFooBarIfNeeded(fragments);
//        const Ptr<const Ieee80211DataOrMgmtHeader>& aMpdu = nullptr;
//        if (aMpduAggregation)
//            aMpdu = aMpduAggregateIfNeeded(fragments);
//        if (aMpdu)
//            fragments = new Fragments({aMpdu});
        return fragments;
    }
}

OriginatorQosMacDataService::~OriginatorQosMacDataService()
{
    delete aMsduAggregation;
    delete aMpduAggregation;
    delete sequenceNumberAssignment;
    delete fragmentation;
}

} /* namespace ieee80211 */
} /* namespace inet */
