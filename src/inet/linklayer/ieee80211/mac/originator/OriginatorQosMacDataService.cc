//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/QoSSequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQosMacDataService);

void OriginatorQosMacDataService::initialize()
{
    aMsduAggregationPolicy = dynamic_cast<IMsduAggregationPolicy*>(getSubmodule("msduAggregationPolicy"));
    if (aMsduAggregationPolicy)
        aMsduAggregation = new MsduAggregation();
    aMpduAggregationPolicy = dynamic_cast<IMpduAggregationPolicy*>(getSubmodule("mpduAggregationPolicy"));
    if (aMpduAggregationPolicy)
        aMpduAggregation = new MpduAggregation();
    sequenceNumberAssigment = new QoSSequenceNumberAssignment();
    fragmentationPolicy = dynamic_cast<IFragmentationPolicy*>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

Packet *OriginatorQosMacDataService::aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue)
{
    auto subframes = aMsduAggregationPolicy->computeAggregateFrames(pendingQueue);
    if (subframes) {
        for (auto f : *subframes)
            pendingQueue->removePacket(f);
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
    sequenceNumberAssigment->assignSequenceNumber(header);
}

std::vector<Packet *> *OriginatorQosMacDataService::fragmentIfNeeded(Packet *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        emit(packetFragmentedSignal, frame);
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        return fragmentFrames;
    }
    return nullptr;
}

std::vector<Packet *> *OriginatorQosMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    if (pendingQueue->isEmpty())
        return nullptr;
    else {
        // if (msduRateLimiting)
        //    txRateLimitingIfNeeded();
        Packet *packet = nullptr;
        if (aMsduAggregationPolicy)
            packet = aMsduAggregateIfNeeded(pendingQueue);
        if (!packet) {
            packet = pendingQueue->dequeuePacket();
            take(packet);
        }
        // PS Defer Queueing
        if (sequenceNumberAssigment) {
            auto header = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(header);
            packet->insertAtFront(header);
        }
        // if (msduIntegrityAndProtection)
        //    frame = protectMsduIfNeeded(frame);
        std::vector<Packet *> *fragments = nullptr;
        if (fragmentationPolicy)
            fragments = fragmentIfNeeded(packet);
        if (!fragments)
            fragments = new std::vector<Packet *>({packet});
        // if (mpduEncryptionAndIntegrity)
        //    fragments = encryptMpduIfNeeded(fragments);
        // if (mpduHeaderPlusCrc)
        //    fragments = mpduCrcFooBarIfNeeded(fragments);
        // const Ptr<const Ieee80211DataOrMgmtHeader>& aMpdu = nullptr;
        // if (aMpduAggregation)
        //    aMpdu = aMpduAggregateIfNeeded(fragments);
        // if (aMpdu)
        //    fragments = new Fragments({aMpdu});
        return fragments;
    }
}

OriginatorQosMacDataService::~OriginatorQosMacDataService()
{
    delete aMsduAggregation;
    delete aMpduAggregation;
    delete sequenceNumberAssigment;
    delete fragmentation;
}

} /* namespace ieee80211 */
} /* namespace inet */
