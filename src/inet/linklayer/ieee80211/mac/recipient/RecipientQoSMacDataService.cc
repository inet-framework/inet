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

// TODO: refactor to avoid code duplication
void RecipientQoSMacDataService::initialize()
{
    duplicateRemoval = new QoSDuplicateRemoval();
    basicReassembly = new BasicReassembly();
    aMsduDeaggregation = new MsduDeaggregation();
    blockAckReordering = new BlockAckReordering();
}

Ieee80211DataFrame* RecipientQoSMacDataService::defragment(std::vector<Ieee80211DataFrame *> completeFragments)
{
    for (auto fragment : completeFragments)
        if (auto completeFrame = dynamic_cast<Ieee80211DataFrame*>(basicReassembly->addFragment(fragment)))
            return completeFrame;
    return nullptr;
}

Ieee80211ManagementFrame* RecipientQoSMacDataService::defragment(Ieee80211ManagementFrame *mgmtFragment)
{
    if (auto completeFrame = dynamic_cast<Ieee80211ManagementFrame *>(basicReassembly->addFragment(mgmtFragment)))
        return completeFrame;
    else
        return nullptr;
}

std::vector<Ieee80211Frame*> RecipientQoSMacDataService::dataFrameReceived(Ieee80211DataFrame* dataFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    // TODO: A-MPDU Deaggregation, MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(dataFrame)) {
        delete dataFrame;
        return std::vector<Ieee80211Frame*>();
    }
    BlockAckReordering::ReorderBuffer frames;
    frames[dataFrame->getSequenceNumber()].push_back(dataFrame);
    if (blockAckReordering && blockAckAgreementHandler) {
        Tid tid = dataFrame->getTid();
        MACAddress originatorAddr = dataFrame->getTransmitterAddress();
        RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
        if (agreement)
            frames = blockAckReordering->processReceivedQoSFrame(agreement, dataFrame);
    }
    std::vector<Ieee80211Frame *> defragmentedFrames;
    if (basicReassembly) { // FIXME: defragmentation
        for (auto it : frames) {
            auto fragments = it.second;
            Ieee80211DataFrame *frame = defragment(fragments);
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
    std::vector<Ieee80211Frame *> deaggregatedFrames;
    if (aMsduDeaggregation) {
        for (auto frame : defragmentedFrames) {
            auto dataFrame = check_and_cast<Ieee80211DataFrame *>(frame);
            if (dataFrame->getAMsduPresent()) {
                auto subframes = aMsduDeaggregation->deaggregateFrame(dataFrame);
                for (auto subframe : *subframes)
                    deaggregatedFrames.push_back(subframe);
                delete subframes;
            }
            else
                deaggregatedFrames.push_back(dataFrame);
        }
    }
    // TODO: MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return deaggregatedFrames;
}

std::vector<Ieee80211Frame*> RecipientQoSMacDataService::managementFrameReceived(Ieee80211ManagementFrame* mgmtFrame)
{
    // TODO: MPDU Header+CRC Validation, Address1 Filtering, Duplicate Removal, MPDU Decryption
    if (duplicateRemoval && duplicateRemoval->isDuplicate(mgmtFrame))
        return std::vector<Ieee80211Frame*>();
    if (basicReassembly) { // FIXME: defragmentation
        mgmtFrame = defragment(mgmtFrame);
    }
    if (auto delba = dynamic_cast<Ieee80211Delba *>(mgmtFrame))
        blockAckReordering->processReceivedDelba(delba);
    // TODO: Defrag, MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
    return std::vector<Ieee80211Frame*>({mgmtFrame});
}

std::vector<Ieee80211Frame*> RecipientQoSMacDataService::controlFrameReceived(Ieee80211Frame* controlFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler)
{
    if (auto blockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(controlFrame)) {
        BlockAckReordering::ReorderBuffer frames;
        if (blockAckReordering) {
            Tid tid = blockAckReq->getTidInfo();
            MACAddress originatorAddr = blockAckReq->getTransmitterAddress();
            RecipientBlockAckAgreement *agreement = blockAckAgreementHandler->getAgreement(tid, originatorAddr);
            if (agreement)
                frames = blockAckReordering->processReceivedBlockAckReq(blockAckReq);
            else
                return std::vector<Ieee80211Frame*>();
        }
        std::vector<Ieee80211Frame *> defragmentedFrames;
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
        std::vector<Ieee80211Frame *> deaggregatedFrames;
        if (aMsduDeaggregation) {
            for (auto frame : defragmentedFrames) {
                auto dataFrame = check_and_cast<Ieee80211DataFrame *>(frame);
                if (dataFrame->getAMsduPresent()) {
                    auto subframes = aMsduDeaggregation->deaggregateFrame(dataFrame);
                    for (auto subframe : *subframes)
                        deaggregatedFrames.push_back(subframe);
                    delete subframes;
                }
                else
                    deaggregatedFrames.push_back(dataFrame);
            }
        }
        // TODO: MSDU Integrity, Replay Detection, RX MSDU Rate Limiting
        return deaggregatedFrames;
    }
    return std::vector<Ieee80211Frame*>();
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
