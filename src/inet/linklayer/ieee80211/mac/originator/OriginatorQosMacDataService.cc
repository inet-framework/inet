//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"

#include <algorithm>

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

Packet *OriginatorQosMacDataService::aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue)
{
    auto subframes = aMsduAggregationPolicy->computeAggregateFrames(pendingQueue);
    if (subframes) {
        for (auto subframe : *subframes) {
            pendingQueue->removePacket(subframe);
            take(subframe);
        }
        auto aggregatedFrame = aMsduAggregation->aggregateFrames(subframes);
        emit(packetAggregatedSignal, aggregatedFrame);
        delete subframes;
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
                fragment->addTag<Ieee80211AddbaTransactionTag>()->setTransactionId(transactionId);
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
    for (int i = 0; i < pendingQueue->getNumPackets(); i++)
        if (isFrameEligible(pendingQueue->getPacket(i)))
            return true;
    return false;
}

std::vector<Packet *> *OriginatorQosMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    Enter_Method("extractFramesToTransmit");
    if (!hasEligibleFrame(pendingQueue))
        return nullptr;
    else {
//        if (msduRateLimiting)
//            txRateLimitingIfNeeded();
        Packet *packet = nullptr;
        for (int i = 0; i < pendingQueue->getNumPackets(); i++) {
            auto candidate = pendingQueue->getPacket(i);
            if (!isFrameEligible(candidate))
                continue;
            // The current A-MSDU policy only aggregates the queue head with
            // frames for the same receiver and TID, so the head eligibility
            // decision also applies to every selected subframe.
            if (aMsduAggregationPolicy && i == 0)
                packet = aMsduAggregateIfNeeded(pendingQueue);
            if (!packet) {
                packet = pendingQueue->dequeuePacket(candidate);
                take(packet);
            }
            break;
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
