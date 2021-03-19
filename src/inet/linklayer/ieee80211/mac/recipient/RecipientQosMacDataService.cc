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

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MsduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/QosDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientQosMacDataService);

// TODO: refactor to avoid code duplication
void RecipientQosMacDataService::initialize()
{
    duplicateRemoval = new QoSDuplicateRemoval();
    basicReassembly = new BasicReassembly();
    aMsduDeaggregation = new MsduDeaggregation();
    aMpduDeaggregation = new MpduDeaggregation();
    blockAckReordering = new BlockAckReordering();
}

Packet *RecipientQosMacDataService::defragment(std::vector<Packet *> completeFragments)
{
    for (auto fragment : completeFragments) {
        auto packet = basicReassembly->addFragment(fragment);
        if (packet != nullptr) {
            emit(packetDefragmentedSignal, packet);
            return packet;
        }
    }
    return nullptr;
}

Packet *RecipientQosMacDataService::defragment(Packet *mgmtFragment)
{
    auto packet = basicReassembly->addFragment(mgmtFragment);
    if (packet && packet->hasAtFront<Ieee80211DataOrMgmtHeader>()) {
        emit(packetDefragmentedSignal, packet);
        return packet;
    }
    else
        return nullptr;
}

std::vector<Packet *> RecipientQosMacDataService::dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    // TODO: A-MPDU Deaggregation, MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(dataHeader)) {
        EV_WARN << "Dropping duplicate packet " << *dataPacket << ".\n";
        PacketDropDetails details;
        details.setReason(DUPLICATE_DETECTED);
        emit(packetDroppedSignal, dataPacket, &details);
        delete dataPacket;
        return std::vector<Packet *>();
    }
    BlockAckReordering::ReorderBuffer frames;
    frames[dataHeader->getSequenceNumber().get()].push_back(dataPacket);
    if (blockAckReordering && blockAckAgreementHandler) {
        Tid tid = dataHeader->getTid();
        MacAddress originatorAddr = dataHeader->getTransmitterAddress();
        RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
        if (agreement)
            frames = blockAckReordering->processReceivedQoSFrame(agreement, dataPacket, dataHeader);
    }
    std::vector<Packet *> defragmentedFrames;
    if (basicReassembly) { // FIXME: defragmentation
        for (auto it : frames) {
            auto fragments = it.second;
            Packet *frame = defragment(fragments);
            // TODO: revise
            if (frame)
                defragmentedFrames.push_back(frame);
        }
    }
    else {
        for (auto it : frames) {
            auto fragments = it.second;
            if (fragments.size() == 1)
                defragmentedFrames.push_back(fragments.at(0));
            else ; // TODO: drop?
        }
    }
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
    // TODO: MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return deaggregatedFrames;
}

std::vector<Packet *> RecipientQosMacDataService::managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    // TODO: MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(mgmtHeader))
        return std::vector<Packet *>();
    if (basicReassembly) { // FIXME: defragmentation
        mgmtPacket = defragment(mgmtPacket);
    }
    if (auto delba = dynamicPtrCast<const Ieee80211Delba>(mgmtHeader))
        blockAckReordering->processReceivedDelba(delba);
    // TODO: Defrag, MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return std::vector<Packet *>({mgmtPacket});
}

std::vector<Packet *> RecipientQosMacDataService::controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    if (auto blockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(controlHeader)) {
        BlockAckReordering::ReorderBuffer frames;
        if (blockAckReordering) {
            Tid tid = blockAckReq->getTidInfo();
            MacAddress originatorAddr = blockAckReq->getTransmitterAddress();
            RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
            if (agreement)
                frames = blockAckReordering->processReceivedBlockAckReq(agreement, blockAckReq);
            else
                return std::vector<Packet *>();
        }
        std::vector<Packet *> defragmentedFrames;
        if (basicReassembly) { // FIXME: defragmentation
            for (auto it : frames) {
                auto fragments = it.second;
                defragmentedFrames.push_back(defragment(fragments));
            }
        }
        else {
            for (auto it : frames) {
                auto fragments = it.second;
                if (fragments.size() == 1) {
                    defragmentedFrames.push_back(fragments.at(0));
                }
                else {
                    // TODO: drop?
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
        // TODO: MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
        return deaggregatedFrames;
    }
    return std::vector<Packet *>();
}

RecipientQosMacDataService::~RecipientQosMacDataService()
{
    delete duplicateRemoval;
    delete basicReassembly;
    delete aMsduDeaggregation;
    delete aMpduDeaggregation;
    delete blockAckReordering;
}

} /* namespace ieee80211 */
} /* namespace inet */
