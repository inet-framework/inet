//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorMacDataService.h"

#include "inet/linklayer/ieee80211/mac/blockack/Ieee80211AddbaTransactionTag_m.h"
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

bool OriginatorMacDataService::isFrameEligible(const Packet *packet) const
{
    return !frameEligibilityFunction || frameEligibilityFunction(packet);
}

bool OriginatorMacDataService::hasEligibleFrame(queueing::IPacketQueue *pendingQueue) const
{
    return pendingQueue->findPacket([this](const Packet *packet) { return isFrameEligible(packet); }) != nullptr;
}

std::vector<Packet *> *OriginatorMacDataService::extractFramesToTransmit(queueing::IPacketQueue *pendingQueue)
{
    Enter_Method("extractFramesToTransmit");
    auto packet = pendingQueue->dequeuePacket([this](const Packet *packet) { return isFrameEligible(packet); });
    if (packet == nullptr)
        return nullptr;
    else {
//        if (msduRateLimiting)
//            txRateLimitingIfNeeded();
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
//        if (mpduHeaderPlusFcs)
//            fragments = mpduFcsFooBarIfNeeded(fragments);
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
