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

#include "inet/linklayer/ieee80211/mac/aggregation/MsduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/QosDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Defragmentation.h"
#include "RecipientQoSMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientQoSMacDataService);

void RecipientQoSMacDataService::initialize()
{
    duplicateRemoval = new QoSDuplicateRemoval();
    basicReassembly = new BasicReassembly();
    aMsduDeaggregation = new MsduDeaggregation();
    blockAckReordering = new BlockAckReordering();
}

Packet *RecipientQoSMacDataService::defragment(std::vector<Packet *> completeFragments)
{
    for (auto fragment : completeFragments) {
        auto packet = basicReassembly->addFragment(fragment);
        if (packet != nullptr)
            return packet;
    }
    return nullptr;
}

Packet *RecipientQoSMacDataService::defragment(Packet *mgmtFragment)
{
    auto packet = basicReassembly->addFragment(mgmtFragment);
    if (packet && packet->hasHeader<Ieee80211DataOrMgmtHeader>())
        return packet;
    else
        return nullptr;
}

std::vector<Packet *> RecipientQoSMacDataService::dataFrameReceived(Packet *dataPacket, const Ptr<Ieee80211DataHeader>& dataFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    // TODO: A-MPDU Deaggregation, MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(dataFrame))
        return std::vector<Packet *>();
    BlockAckReordering::ReorderBuffer frames;
    frames[dataFrame->getSequenceNumber()].push_back(dataPacket);
    if (blockAckReordering && blockAckAgreementHandler) {
        Tid tid = dataFrame->getTid();
        MACAddress originatorAddr = dataFrame->getTransmitterAddress();
        RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
        if (agreement)
            frames = blockAckReordering->processReceivedQoSFrame(agreement, dataPacket, dataFrame);
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
        for (auto frame : defragmentedFrames) {
            auto dataFrame = frame->peekHeader<Ieee80211DataHeader>();
            if (dataFrame->getAMsduPresent()) {
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

std::vector<Packet *> RecipientQoSMacDataService::managementFrameReceived(Packet *mgmtPacket, const Ptr<Ieee80211MgmtHeader>& mgmtFrame)
{
    // TODO: MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(mgmtFrame))
        return std::vector<Packet *>();
    if (basicReassembly) { // FIXME: defragmentation
        mgmtPacket = defragment(mgmtPacket);
    }
    if (auto delba = std::dynamic_pointer_cast<Ieee80211Delba>(mgmtFrame))
        blockAckReordering->processReceivedDelba(delba);
    // TODO: Defrag, MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return std::vector<Packet *>({mgmtPacket});
}

std::vector<Packet *> RecipientQoSMacDataService::controlFrameReceived(Packet *controlPacket, const Ptr<Ieee80211MacHeader>& controlFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    if (auto blockAckReq = std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(controlFrame)) {
        BlockAckReordering::ReorderBuffer frames;
        if (blockAckReordering) {
            Tid tid = blockAckReq->getTidInfo();
            MACAddress originatorAddr = blockAckReq->getTransmitterAddress();
            RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
            if (agreement)
                frames = blockAckReordering->processReceivedBlockAckReq(blockAckReq);
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
        // TODO: Defrag, MSDU Integrity, Replay Detection, A-MSDU Degagg., RX MSDU Rate Limiting
        return defragmentedFrames;
    }
    return std::vector<Packet *>();
}

RecipientQoSMacDataService::~RecipientQoSMacDataService()
{
    delete duplicateRemoval;
    delete basicReassembly;
    delete aMsduDeaggregation;
    delete blockAckReordering;
}

} /* namespace ieee80211 */
} /* namespace inet */
