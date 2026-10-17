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
#include "inet/queueing/contract/ISelectivePacketCollection.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQosMacDataService);

PreparedQosData::~PreparedQosData()
{
    if (packet)
        drop(packet.get());
}

void PreparedQosData::setPacket(Packet *value)
{
    if (packet)
        drop(packet.get());
    packet.reset(value);
    if (packet)
        take(packet.get());
}

Packet *PreparedQosData::releasePacket()
{
    drop(packet.get());
    return packet.release();
}

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

std::unique_ptr<PreparedQosData> OriginatorQosMacDataService::prepareFrame(queueing::IPacketQueue *queue, bool allowAggregation)
{
    Enter_Method("prepareFrame");
    if (expireFrames)
        expireFrames();
    auto selected = peekEligibleFrame(queue);
    if (!selected)
        return nullptr;
    auto selectedHeader = dynamicPtrCast<const Ieee80211DataHeader>(selected->peekAtFront<Ieee80211MacHeader>());
    if (!selectedHeader || selectedHeader->getType() != ST_DATA_WITH_QOS)
        return nullptr;
    auto result = std::make_unique<PreparedQosData>();
    result->queue = queue;
    std::vector<Packet *> sources{selected};
    auto module = dynamic_cast<cModule *>(queue);
    auto head = queue->canPullPacket(module ? module->gate("out") : nullptr);
    if (allowAggregation && selected == head && aMsduAggregationPolicy && (!aMsduPermitted || aMsduPermitted(selected))) {
        std::unique_ptr<std::vector<Packet *>> proposed(aMsduAggregationPolicy->computeAggregateFrames(queue));
        bool valid = proposed && !proposed->empty() && proposed->front() == selected;
        std::set<Packet *> unique;
        if (proposed) {
            for (auto frame : *proposed) {
                bool present = false;
                for (int i = 0; i < queue->getNumPackets(); i++)
                    present |= queue->getPacket(i) == frame;
                valid &= present && unique.insert(frame).second &&
                    (!extractionEligible || extractionEligible(frame)) && (!aMsduPermitted || aMsduPermitted(frame));
            }
        }
        if (valid) {
            sources = *proposed;
            result->aggregate = true;
        }
    }
    for (auto source : sources)
        result->sourceIds.push_back(source->getId());
    if (result->aggregate) {
        std::vector<Packet *> duplicates;
        for (auto source : sources)
            duplicates.push_back(source->dup());
        result->setPacket(aMsduAggregation->aggregateFrames(&duplicates));
    }
    else
        result->setPacket(selected->dup());
    // The preview carries the next value without consuming the sequence counter.
    auto header = result->packet->removeAtFront<Ieee80211DataHeader>();
    header->setSequenceNumber(getNextSequenceNumber(header->getReceiverAddress(), header->getTid()));
    result->packet->insertAtFront(header);
    return result;
}

bool OriginatorQosMacDataService::isPreparationValid(const PreparedQosData& preparation, simtime_t start) const
{
    if (!preparation.packet || !preparation.queue || preparation.sourceIds.empty())
        return false;
    auto header = preparation.packet->peekAtFront<Ieee80211DataHeader>();
    if (header->getSequenceNumber() != getNextSequenceNumber(header->getReceiverAddress(), header->getTid()))
        return false;
    for (auto id : preparation.sourceIds) {
        Packet *source = nullptr;
        for (int i = 0; i < preparation.queue->getNumPackets(); i++) {
            auto candidate = preparation.queue->getPacket(i);
            if (candidate->getId() == id) {
                source = candidate;
                break;
            }
        }
        if (!source || (lifetimeHandler && lifetimeHandler->isExpired(source, start)) ||
            (extractionEligible && !extractionEligible(source)) ||
            (preparation.aggregate && aMsduPermitted && !aMsduPermitted(source)))
            return false;
    }
    return true;
}

Packet *OriginatorQosMacDataService::commitPreparedFrame(PreparedQosData& preparation)
{
    Enter_Method("commitPreparedFrame");
    if (!isPreparationValid(preparation, simTime()))
        return nullptr;
    std::vector<Packet *> sources;
    std::vector<int64_t> trackedIds;
    simtime_t deadline = SIMTIME_MAX;
    for (auto id : preparation.sourceIds) {
        for (int i = 0; i < preparation.queue->getNumPackets(); i++) {
            auto source = preparation.queue->getPacket(i);
            if (source->getId() == id) {
                sources.push_back(source);
                if (lifetimeHandler && lifetimeHandler->getDeadline(source) != SIMTIME_MAX) {
                    trackedIds.push_back(id);
                    deadline = lifetimeHandler->getDeadline(source);
                }
                break;
            }
        }
    }
    for (auto source : sources) {
        if (preparation.aggregate)
            preparation.queue->removePacket(source);
        else {
            auto module = dynamic_cast<cModule *>(preparation.queue);
            auto next = preparation.queue->canPullPacket(module ? module->gate("out") : nullptr);
            auto extracted = source == next ? preparation.queue->dequeuePacket() :
                check_and_cast<queueing::ISelectivePacketCollection *>(preparation.queue)->extractPacket(source);
            ASSERT(extracted == source);
            // The preview predates extraction. Preserve the queue's completed service metadata.
            preparation.packet->copyRegionTags(*source, source->getFrontOffset(),
                    preparation.packet->getFrontOffset(), source->getDataLength());
        }
        take(source);
    }
    if (lifetimeHandler)
        lifetimeHandler->prepareFrameReplacement(trackedIds);
    for (auto source : sources)
        delete source;
    auto packet = preparation.releasePacket();
    take(packet);
    if (lifetimeHandler) {
        if (preparation.aggregate) {
            auto edca = check_and_cast<Edca *>(getParentModule()->getSubmodule("edca"));
            lifetimeHandler->registerFrame(packet, edca->classifyFrame(packet->peekAtFront<Ieee80211DataHeader>()));
        }
        else if (deadline != SIMTIME_MAX)
            lifetimeHandler->registerFragments({packet}, deadline);
    }
    auto header = packet->removeAtFront<Ieee80211DataHeader>();
    assignSequenceNumber(header);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<Ieee80211DataSequenceTag>()->setAllocationIndex(nextDataAllocationIndex++);
    if (preparation.aggregate)
        emit(packetAggregatedSignal, packet);
    return packet;
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
            packet = selected == next ? pendingQueue->dequeuePacket() :
                check_and_cast<queueing::ISelectivePacketCollection *>(pendingQueue)->extractPacket(selected);
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
        if (fragmentationPolicy && (!fragmentationPermitted || fragmentationPermitted(packet)))
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
    auto predicate = [this](Packet *packet) { return !extractionEligible || extractionEligible(packet); };
    auto module = dynamic_cast<cModule *>(queue);
    auto packet = queue->canPullPacket(module ? module->gate("out") : nullptr);
    if (packet && predicate(packet))
        return packet;
    if (auto selected = dynamic_cast<queueing::ISelectivePacketCollection *>(queue)) {
        try {
            return selected->peekEligiblePacket(predicate);
        }
        catch (const queueing::UnsupportedPacketSelection&) {
            // Ordinary service remains valid when this composition cannot bypass a deferred head.
        }
    }
    for (int i = 0; i < queue->getNumPackets(); i++) {
        if (predicate(queue->getPacket(i)))
            throw cRuntimeError("Deferred DATA hides an eligible frame: queue requires ISelectivePacketCollection");
    }
    return nullptr;
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

