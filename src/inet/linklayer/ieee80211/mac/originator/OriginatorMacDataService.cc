//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorMacDataService.h"

#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"
#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/NonQoSSequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorMacDataService);

void OriginatorMacDataService::initialize()
{
    sequenceNumberAssignment = new NonQoSSequenceNumberAssignment();
    fragmentationPolicy = check_and_cast<IFragmentationPolicy *>(getSubmodule("fragmentationPolicy"));
    fragmentation = new Fragmentation();
}

void OriginatorMacDataService::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    sequenceNumberAssignment->assignSequenceNumber(header);
}

std::vector<Packet *> *OriginatorMacDataService::fragmentIfNeeded(Packet *frame)
{
    auto fragmentSizes = fragmentationPolicy->computeFragmentSizes(frame);
    if (fragmentSizes.size() != 0) {
        emit(packetFragmentedSignal, frame);
        auto fragmentFrames = fragmentation->fragmentFrame(frame, fragmentSizes);
        return fragmentFrames;
    }
    return nullptr;
}

std::vector<Packet *> *OriginatorMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    Enter_Method("extractFramesToTransmit");
    if (pendingQueue->isEmpty())
        return nullptr;
    else {
//        if (msduRateLimiting)
//            txRateLimitingIfNeeded();
        Packet *packet = pendingQueue->dequeuePacket();
        take(packet);
        if (sequenceNumberAssignment) {
            auto frame = packet->removeAtFront<Ieee80211DataOrMgmtHeader>();
            assignSequenceNumber(frame);
            packet->insertAtFront(frame);
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
//        if (mpduHeaderPlusCrc)
//            fragments = mpduCrcFooBarIfNeeded(fragments);
        return fragments;
    }
}

OriginatorMacDataService::~OriginatorMacDataService()
{
    delete sequenceNumberAssignment;
    delete fragmentation;
}

} /* namespace ieee80211 */
} /* namespace inet */

