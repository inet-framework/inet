//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Edca.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/QoSSequenceNumberAssignment.h"
#include "inet/queueing/function/PacketFilterFunction.h"

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

Packet *OriginatorQosMacDataService::aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue, Packet *selected)
{
    auto subframes = aMsduAggregationPolicy->computeAggregateFrames(pendingQueue);
    if (subframes) {
        bool valid = !subframes->empty() && subframes->front() == selected;
        std::set<Packet *> unique;
        for (auto frame : *subframes) {
            bool present = false;
            for (int i = 0; i < pendingQueue->getNumPackets(); i++) {
                if (pendingQueue->getPacket(i) == frame) {
                    present = true;
                    break;
                }
            }
            valid &= present && unique.insert(frame).second &&
                (!extractionEligible || extractionEligible(frame)) &&
                (!aMsduPermitted || aMsduPermitted(frame));
        }
        if (!valid) {
            delete subframes;
            return nullptr;
        }
    }
    if (subframes) {
        std::vector<int64_t> sourceIds;
        AccessCategory ac = AC_BE;
        if (lifetimeHandler) {
            auto header = subframes->front()->peekAtFront<Ieee80211DataHeader>();
            ac = check_and_cast<Edca *>(getParentModule()->getSubmodule("edca"))->classifyFrame(header);
        }
        for (auto subframe : *subframes) {
            pendingQueue->removePacket(subframe);
            take(subframe);
            // qOsCheck can permit non-QoS sources that have no lifetime entry.
            if (lifetimeHandler && lifetimeHandler->getDeadline(subframe) != SIMTIME_MAX)
                sourceIds.push_back(subframe->getId());
        }
        if (lifetimeHandler)
            lifetimeHandler->prepareFrameReplacement(sourceIds);
        auto aggregatedFrame = aMsduAggregation->aggregateFrames(subframes);
        if (lifetimeHandler) {
            // IEEE Std 802.11-2024, 10.23.2.12.1: each addition restarts the A-MSDU timer.
            // This builder adds all constituents at the current simulation time.
            lifetimeHandler->registerFrame(aggregatedFrame, ac);
        }
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
        auto deadline = lifetimeHandler ? lifetimeHandler->getDeadline(frame) : SIMTIME_MAX;
        if (deadline != SIMTIME_MAX)
            lifetimeHandler->prepareFrameReplacement({frame->getId()});
        emit(packetFragmentedSignal, frame);
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        if (deadline != SIMTIME_MAX)
            lifetimeHandler->registerFragments(*fragmentFrames, deadline);
        return fragmentFrames;
    }
    return nullptr;
}

std::vector<Packet *> *OriginatorQosMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    Enter_Method("extractFramesToTransmit");
    if (expireFrames)
        expireFrames();
    auto selected = peekEligibleFrame(pendingQueue);
    if (selected == nullptr)
        return nullptr;
    else {
//        if (msduRateLimiting)
//            txRateLimitingIfNeeded();
        Packet *packet = nullptr;
        auto module = dynamic_cast<cModule *>(pendingQueue);
        auto next = pendingQueue->canPullPacket(module ? module->gate("out") : nullptr);
        if (selected == next && aMsduAggregationPolicy && (!aMsduPermitted || aMsduPermitted(selected)))
            packet = aMsduAggregateIfNeeded(pendingQueue, selected);
        if (!packet) {
            queueing::CPacketFilterFunction filter([selected](const Packet *candidate) { return candidate == selected; });
            packet = selected == next ? pendingQueue->dequeuePacket() :
                pendingQueue->pullPacket(module ? module->gate("out") : nullptr, &filter);
            take(packet);
        }
        // PS Defer Queueing
        if (sequenceNumberAssignment) {
            auto header = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(header);
            packet->insertAtFront(header);
        }
        if (dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            packet->addTagIfAbsent<Ieee80211DataSequenceTag>()->setAllocationIndex(nextDataAllocationIndex++);
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

Packet *OriginatorQosMacDataService::peekEligibleFrame(queueing::IPacketQueue *queue) const
{
    if (queue->isEmpty())
        return nullptr;
    queueing::CPacketFilterFunction filter([this](const Packet *packet) { return !extractionEligible || extractionEligible(packet); });
    auto module = dynamic_cast<cModule *>(queue);
    auto packet = queue->canPullPacket(module ? module->gate("out") : nullptr);
    if (packet && filter.matchesPacket(packet))
        return packet;
    return queue->canPullPacket(module ? module->gate("out") : nullptr, &filter);
}

SequenceNumberCyclic OriginatorQosMacDataService::getNextSequenceNumber(MacAddress peer, Tid tid) const
{
    return check_and_cast<QoSSequenceNumberAssignment *>(sequenceNumberAssignment)->getNextSequenceNumber(peer, tid);
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

