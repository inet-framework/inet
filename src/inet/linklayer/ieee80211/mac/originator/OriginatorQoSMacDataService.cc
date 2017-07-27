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

#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/QoSSequenceNumberAssignment.h"
#include "OriginatorQoSMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQoSMacDataService);

void OriginatorQoSMacDataService::initialize()
{
    aMsduAggregationPolicy = dynamic_cast<IMsduAggregationPolicy*>(getSubmodule("msduAggregationPolicy"));
    if (aMsduAggregationPolicy)
        aMsduAggregation = new MsduAggregation();
    sequenceNumberAssigment = new QoSSequenceNumberAssignment();
    fragmentationPolicy = dynamic_cast<IFragmentationPolicy*>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

Packet *OriginatorQoSMacDataService::aMsduAggregateIfNeeded(PendingQueue *pendingQueue)
{
    auto subframes = aMsduAggregationPolicy->computeAggregateFrames(pendingQueue);
    if (subframes) {
        for (auto f : *subframes)
            pendingQueue->remove(f);
        auto aggregatedFrame = aMsduAggregation->aggregateFrames(subframes);
        delete subframes;
        return aggregatedFrame;
    }
    return nullptr;
}

void OriginatorQoSMacDataService::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    sequenceNumberAssigment->assignSequenceNumber(header);
}

std::vector<Packet *> *OriginatorQoSMacDataService::fragmentIfNeeded(Packet *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        return fragmentFrames;
    }
    return nullptr;
}

std::vector<Packet *> *OriginatorQoSMacDataService::extractFramesToTransmit(PendingQueue *pendingQueue)
{
    if (pendingQueue->isEmpty())
        return nullptr;
    else {
        // if (msduRateLimiting)
        //    txRateLimitingIfNeeded();
        Packet *packet = nullptr;
        if (aMsduAggregationPolicy)
            packet = aMsduAggregateIfNeeded(pendingQueue);
        if (!packet)
            packet = pendingQueue->pop();
        // PS Defer Queueing
        if (sequenceNumberAssigment) {
            auto header = packet->removeHeader<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(header);
            packet->insertHeader(header);
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

OriginatorQoSMacDataService::~OriginatorQoSMacDataService()
{
    delete aMsduAggregation;
    delete sequenceNumberAssigment;
    delete fragmentation;
}

} /* namespace ieee80211 */
} /* namespace inet */
