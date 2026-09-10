//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.h"

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/QosDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientQosMacDataService);

// TODO refactor to avoid code duplication
void RecipientQosMacDataService::initialize()
{
    maxReceiveLifetime = par("maxReceiveLifetime");
    if (maxReceiveLifetime < SIMTIME_ZERO)
        throw cRuntimeError("maxReceiveLifetime must not be negative");
    duplicateRemoval = new QoSDuplicateRemoval();
    basicReassembly = new BasicReassembly(maxReceiveLifetime);
    aMsduDeaggregation = new MsduDeaggregation();
    aMpduDeaggregation = new MpduDeaggregation();
    blockAckReordering = new BlockAckReordering(maxReceiveLifetime);
    receiveLifetimeTimer = new cMessage("receiveLifetimeTimer");
}

void RecipientQosMacDataService::handleMessage(cMessage *message)
{
    if (message != receiveLifetimeTimer)
        throw cRuntimeError("Unknown message");
    expireReceiveLifetime();
    scheduleReceiveLifetimeTimer();
}

void RecipientQosMacDataService::expireReceiveLifetime()
{
    for (auto packet : basicReassembly->removeExpiredFragments(simTime())) {
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
    for (auto packet : blockAckReordering->removeExpiredFragments(simTime())) {
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void RecipientQosMacDataService::scheduleReceiveLifetimeTimer()
{
    if (receiveLifetimeTimer->isScheduled())
        cancelEvent(receiveLifetimeTimer);
    auto nextExpirationTime = std::min(basicReassembly->getNextExpirationTime(), blockAckReordering->getNextExpirationTime());
    if (nextExpirationTime != SIMTIME_MAX)
        scheduleAt(nextExpirationTime, receiveLifetimeTimer);
}

void RecipientQosMacDataService::resetBlockAckReordering(Tid tid, MacAddress originatorAddr)
{
    Enter_Method("resetBlockAckReordering");
    if (blockAckReordering) {
        auto droppedFrames = blockAckReordering->resetReceiveBuffer(tid, originatorAddr);
        for (auto packet : droppedFrames) {
            take(packet);
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    if (basicReassembly) {
        auto droppedFragments = basicReassembly->purge(originatorAddr, tid, 0, 4095);
        for (auto packet : droppedFragments) {
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    scheduleReceiveLifetimeTimer();
}

Packet *RecipientQosMacDataService::defragment(std::vector<Packet *> completeFragments)
{
    expireReceiveLifetime();
    Packet *defragmentedPacket = nullptr;
    for (auto fragment : completeFragments) {
        auto packet = basicReassembly->addFragment(fragment);
        if (packet != nullptr) {
            defragmentedPacket = packet;
            break;
        }
    }
    scheduleReceiveLifetimeTimer();
    if (defragmentedPacket != nullptr)
        emit(packetDefragmentedSignal, defragmentedPacket);
    return defragmentedPacket;
}

Packet *RecipientQosMacDataService::defragment(Packet *mgmtFragment)
{
    expireReceiveLifetime();
    auto packet = basicReassembly->addFragment(mgmtFragment);
    scheduleReceiveLifetimeTimer();
    if (packet && packet->hasAtFront<Ieee80211DataOrMgmtHeader>()) {
        emit(packetDefragmentedSignal, packet);
        return packet;
    }
    else
        return nullptr;
}

std::vector<Packet *> RecipientQosMacDataService::dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    Enter_Method("dataFrameReceived");
    take(dataPacket);
    expireReceiveLifetime();
    // TODO A-MPDU Deaggregation, MPDU Header+FCS Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    RecipientBlockAckAgreement *agreement = nullptr;
    if (dataHeader->getAckPolicy() == AckPolicy::BLOCK_ACK) {
        if (blockAckAgreementHandler != nullptr)
            agreement = blockAckAgreementHandler->getActiveAgreement(dataHeader->getTid(), dataHeader->getTransmitterAddress());
        if (agreement == nullptr) {
            EV_INFO << "Dropping Block Ack policy data without an active Block Ack agreement.\n";
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, dataPacket, &details);
            delete dataPacket;
            scheduleReceiveLifetimeTimer();
            return std::vector<Packet *>();
        }
    }
    if (duplicateRemoval && duplicateRemoval->isDuplicate(dataHeader)) {
        EV_WARN << "Dropping duplicate packet " << *dataPacket << ".\n";
        PacketDropDetails details;
        details.setReason(DUPLICATE_DETECTED);
        emit(packetDroppedSignal, dataPacket, &details);
        delete dataPacket;
        scheduleReceiveLifetimeTimer();
        return std::vector<Packet *>();
    }
    BlockAckReordering::ReorderBuffer frames;
    frames[dataHeader->getSequenceNumber().get()].push_back(dataPacket);
    if (blockAckReordering && blockAckAgreementHandler) {
        Tid tid = dataHeader->getTid();
        MacAddress originatorAddr = dataHeader->getTransmitterAddress();
        if (agreement == nullptr)
            agreement = blockAckAgreementHandler->getActiveAgreement(tid, originatorAddr);
        if (agreement) {
            auto processingResult = blockAckReordering->processReceivedQoSFrameWithResult(agreement, dataPacket, dataHeader);
            frames = processingResult.frames;
            for (auto packet : processingResult.tombstonedFragments) {
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
            }
        }
    }
    std::vector<Packet *> defragmentedFrames;
    if (basicReassembly) { // FIXME defragmentation
        for (auto it : frames) {
            auto fragments = it.second;
            Packet *frame = defragment(fragments);
            // TODO revise
            if (frame)
                defragmentedFrames.push_back(frame);
        }
    }
    else {
        for (auto it : frames) {
            auto fragments = it.second;
            if (fragments.size() == 1)
                defragmentedFrames.push_back(fragments.at(0));
            else ; // TODO drop?
        }
    }
    scheduleReceiveLifetimeTimer();
    std::vector<Packet *> deaggregatedFrames;
    if (aMsduDeaggregation) {
        for (auto defragmentedFrame : defragmentedFrames) {
            auto defragmentedHeader = defragmentedFrame->peekAtFront<Ieee80211DataHeader>();
            if (defragmentedHeader->getAMsduPresent()) {
                emit(packetDeaggregatedSignal, defragmentedFrame);
                auto subframes = aMsduDeaggregation->deaggregateFrame(defragmentedFrame);
                for (auto subframe : *subframes)
                    deaggregatedFrames.push_back(subframe);
                delete subframes;
            }
            else
                deaggregatedFrames.push_back(defragmentedFrame);
        }
    }
    // TODO MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return deaggregatedFrames;
}

IRecipientQosMacDataService::ManagementFrameReceptionResult RecipientQosMacDataService::managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    Enter_Method("managementFrameReceived");
    take(mgmtPacket);
    expireReceiveLifetime();
    // TODO MPDU Header+FCS Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(mgmtHeader)) {
        delete mgmtPacket;
        // A duplicate fragment is acknowledged by HCF but is not a complete
        // MMPDU. Preserve the existing subtype-specific handling only for an
        // unfragmented duplicate management frame.
        if (mgmtHeader->getFragmentNumber() == 0 && !mgmtHeader->getMoreFragments()) {
            scheduleReceiveLifetimeTimer();
            return { {}, mgmtHeader, true };
        }
        else {
            scheduleReceiveLifetimeTimer();
            return { {}, nullptr, true };
        }
    }
    if (basicReassembly) { // FIXME defragmentation
        mgmtPacket = defragment(mgmtPacket);
    }
    if (mgmtPacket == nullptr)
        return {};
    const auto& completeHeader = mgmtPacket->peekAtFront<Ieee80211MgmtHeader>();
    // TODO Defrag, MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    if (dynamicPtrCast<const Ieee80211ActionFrame>(completeHeader)) {
        delete mgmtPacket;
        scheduleReceiveLifetimeTimer();
        return { {}, completeHeader, false };
    }
    else {
        scheduleReceiveLifetimeTimer();
        return { { mgmtPacket }, completeHeader, false };
    }
}

std::vector<Packet *> RecipientQosMacDataService::controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    Enter_Method("controlFrameReceived");
    expireReceiveLifetime();
    if (auto blockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(controlHeader)) {
        BlockAckReordering::ReorderBuffer frames;
        if (blockAckReordering) {
            Tid tid = blockAckReq->getTidInfo();
            MacAddress originatorAddr = blockAckReq->getTransmitterAddress();
            RecipientBlockAckAgreement *agreement = blockAckAgreementHandler == nullptr ? nullptr : blockAckAgreementHandler->getActiveAgreement(tid, originatorAddr);
            if (agreement)
                frames = blockAckReordering->processReceivedBlockAckReq(agreement, blockAckReq);
            else {
                scheduleReceiveLifetimeTimer();
                return std::vector<Packet *>();
            }
        }
        std::vector<Packet *> defragmentedFrames;
        if (basicReassembly) { // FIXME defragmentation
            for (auto it : frames) {
                auto fragments = it.second;
                auto frame = defragment(fragments);
                if (frame != nullptr)
                    defragmentedFrames.push_back(frame);
            }
        }
        else {
            for (auto it : frames) {
                auto fragments = it.second;
                if (fragments.size() == 1) {
                    defragmentedFrames.push_back(fragments.at(0));
                }
                else {
                    // TODO drop?
                }
            }
        }
        std::vector<Packet *> deaggregatedFrames;
        if (aMsduDeaggregation) {
            for (auto frame : defragmentedFrames) {
                if (frame->peekAtFront<Ieee80211DataHeader>()->getAMsduPresent()) {
                    emit(packetDeaggregatedSignal, frame);
                    auto subframes = aMsduDeaggregation->deaggregateFrame(frame);
                    for (auto subframe : *subframes)
                        deaggregatedFrames.push_back(subframe);
                    delete subframes;
                }
                else
                    deaggregatedFrames.push_back(frame);
            }
        }
        // TODO MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
        scheduleReceiveLifetimeTimer();
        return deaggregatedFrames;
    }
    scheduleReceiveLifetimeTimer();
    return std::vector<Packet *>();
}

RecipientQosMacDataService::~RecipientQosMacDataService()
{
    cancelAndDelete(receiveLifetimeTimer);
    delete duplicateRemoval;
    delete basicReassembly;
    delete aMsduDeaggregation;
    delete aMpduDeaggregation;
    delete blockAckReordering;
}

} /* namespace ieee80211 */
} /* namespace inet */
