//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

#include <algorithm>

namespace inet {
namespace ieee80211 {

std::vector<std::pair<SequenceNumber, BlockAckReordering::Fragments>> BlockAckReordering::getFramesInOrder(const ReorderBuffer& frames)
{
    std::vector<std::pair<SequenceNumber, Fragments>> ordered(frames.begin(), frames.end());
    if (ordered.empty())
        return ordered;
    auto first = SequenceNumberCyclic(ordered.front().first);
    for (const auto& entry : ordered)
        if (SequenceNumberCyclic(entry.first) < first)
            first = SequenceNumberCyclic(entry.first);
    // Use offsets from one anchor, rather than a cyclic map comparator.
    std::sort(ordered.begin(), ordered.end(), [first](const auto& a, const auto& b) {
        return (a.first - first.get() + 4096) % 4096 < (b.first - first.get() + 4096) % 4096;
    });
    return ordered;
}

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    ReceiveBuffer *receiveBuffer = createReceiveBufferIfNecessary(agreement);
    ReorderBuffer frames;
    auto sequenceNumber = dataHeader->getSequenceNumber();
    auto windowStart = receiveBuffer->getNextExpectedSequenceNumber();
    auto windowSize = receiveBuffer->getBufferSize();
    // IEEE Std 802.11-2024, 10.25.6.6.2.1 b): retain forward MPDUs and flush preceding complete MSDUs.
    if (windowStart < sequenceNumber && !(sequenceNumber < windowStart + windowSize)) {
        auto newStart = sequenceNumber - windowSize + 1;
        frames = collectCompletePrecedingMpdus(receiveBuffer, newStart);
        releaseReceiveBuffer(agreement, receiveBuffer, frames);
        receiveBuffer->dropFramesUntil(newStart);
        receiveBuffer->setNextExpectedSequenceNumber(newStart);
    }
    // Receipt status covers every related Data frame. The agreement handler owns policy-specific inactivity refresh.
    agreement->getBlockAckRecord()->dataFrameReceived(dataHeader);
    if (receiveBuffer->insertFrame(dataPacket, dataHeader)) {
        // IEEE Std 802.11-2024, 10.25.6.6.2.1 a): delivery continues until the next missing MSDU.
        auto consecutive = collectConsecutiveCompleteFollowingMpdus(receiveBuffer, receiveBuffer->getNextExpectedSequenceNumber());
        releaseReceiveBuffer(agreement, receiveBuffer, consecutive);
        frames.insert(consecutive.begin(), consecutive.end());
        auto earliestCompleteMsduOrAMsdu = getEarliestCompleteMsduOrAMsduIfExists(receiveBuffer);
        if (earliestCompleteMsduOrAMsdu.size() > 0) {
            auto earliestSequenceNumber = earliestCompleteMsduOrAMsdu.at(0)->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
            // If, after an MPDU is received, the receive buffer is full, the complete MSDU or A-MSDU with the earliest
            // sequence number shall be passed up to the next MAC process.
            if (receiveBuffer->isFull()) {
                passedUp(agreement, receiveBuffer, earliestSequenceNumber);
                frames[earliestSequenceNumber.get()] = earliestCompleteMsduOrAMsdu;
            }
            // If, after an MPDU is received, the receive buffer is not full, but the sequence number of the complete MSDU or
            // A-MSDU in the buffer with the lowest sequence number is equal to the NextExpectedSequenceNumber for
            // that Block Ack agreement, then the MPDU shall be passed up to the next MAC process.
            else if (earliestSequenceNumber == receiveBuffer->getNextExpectedSequenceNumber()) {
                passedUp(agreement, receiveBuffer, earliestSequenceNumber);
                frames[earliestSequenceNumber.get()] = earliestCompleteMsduOrAMsdu;
            }
        }
    }
    else
        delete dataPacket;
    return frames;
}

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedBlockAckReq(RecipientBlockAckAgreement *agreement, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    // The originator shall use the Block Ack starting sequence control to signal the first MPDU in the block for
    // which an acknowledgment is expected.
    SequenceNumberCyclic startingSequenceNumber;
    Tid tid = -1;
    if (auto basicReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        tid = basicReq->getTidInfo();
        startingSequenceNumber = basicReq->getStartingSequenceNumber();
    }
    else if (auto compressedReq = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(blockAckReq)) {
        tid = compressedReq->getTidInfo();
        startingSequenceNumber = compressedReq->getStartingSequenceNumber();
    }
    else {
        throw cRuntimeError("Multi-Tid BlockAckReq is currently an unimplemented feature");
    }
    agreement->getBlockAckRecord()->blockAckReqReceived(startingSequenceNumber);
    auto id = std::make_pair(tid, blockAckReq->getTransmitterAddress());
    auto it = receiveBuffers.find(id);
    if (it != receiveBuffers.end()) {
        ReceiveBuffer *receiveBuffer = it->second;
        // MPDUs in the recipient’s buffer with a sequence control value that
        // precedes the starting sequence control value are called preceding MPDUs.
        // The recipient shall reassemble any complete MSDUs from buffered preceding
        // MPDUs and indicate these to its higher layer.
        auto completePrecedingMpdus = collectCompletePrecedingMpdus(receiveBuffer, startingSequenceNumber);
        // Upon arrival of a BlockAckReq frame, the recipient shall pass up the MSDUs and A-MSDUs starting with
        // the starting sequence number sequentially until there is an incomplete or missing MSDU
        // or A-MSDU in the buffer.
        auto consecutiveCompleteFollowingMpdus = collectConsecutiveCompleteFollowingMpdus(receiveBuffer, startingSequenceNumber);
        // If no MSDUs or A-MSDUs are passed up to the next MAC process after the receipt
        // of the BlockAckReq frame and the starting sequence number of the BlockAckReq frame is newer than the
        // NextExpectedSequenceNumber for that Block Ack agreement, then the NextExpectedSequenceNumber for
        // that Block Ack agreement is set to the sequence number of the BlockAckReq frame.
        int numOfMsdusToPassUp = completePrecedingMpdus.size() + consecutiveCompleteFollowingMpdus.size();
        if (numOfMsdusToPassUp == 0 && receiveBuffer->getNextExpectedSequenceNumber() < startingSequenceNumber)
            receiveBuffer->setNextExpectedSequenceNumber(startingSequenceNumber);
        // The recipient shall then release any buffers held by preceding MPDUs.
        releaseReceiveBuffer(agreement, receiveBuffer, completePrecedingMpdus);
        releaseReceiveBuffer(agreement, receiveBuffer, consecutiveCompleteFollowingMpdus);
        // The recipient shall pass MSDUs and A-MSDUs up to the next MAC process in order of increasing sequence
        // number.
        completePrecedingMpdus.insert(consecutiveCompleteFollowingMpdus.begin(), consecutiveCompleteFollowingMpdus.end());
        return completePrecedingMpdus;
    }
    return ReorderBuffer();
}

//
// If a BlockAckReq frame is received, all complete MSDUs and A-MSDUs with lower sequence numbers than
// the starting sequence number contained in the BlockAckReq frame shall be passed up to the next MAC process
// as shown in Figure 5-1.
//
BlockAckReordering::ReorderBuffer BlockAckReordering::collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber)
{
    ReorderBuffer completePrecedingMpdus;
    const auto& buffer = receiveBuffer->getBuffer();
    for (auto it : buffer) { // collects complete preceding MPDUs
        auto sequenceNumber = it.first;
        auto fragments = it.second;
        if (SequenceNumberCyclic(sequenceNumber) < startingSequenceNumber)
            if (isComplete(fragments))
                completePrecedingMpdus[sequenceNumber] = fragments;
    }
    return completePrecedingMpdus;
}

//
// Upon arrival of a BlockAckReq frame, the recipient shall pass up the MSDUs and A-MSDUs starting with
// the starting sequence number sequentially until there is an incomplete or missing MSDU
// or A-MSDU in the buffer.
//
BlockAckReordering::ReorderBuffer BlockAckReordering::collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber)
{
    ReorderBuffer framesToPassUp;
    for (int i = 0; i < receiveBuffer->getBufferSize(); i++) {
        if (!addMsduIfComplete(receiveBuffer, framesToPassUp, startingSequenceNumber + i))
            return framesToPassUp;
    }
    return framesToPassUp;
}

bool BlockAckReordering::addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReorderBuffer& reorderBuffer, SequenceNumberCyclic seqNum)
{
    const auto& buffer = receiveBuffer->getBuffer();
    auto it = buffer.find(seqNum.get());
    if (it != buffer.end()) {
        auto fragments = it->second;
        if (isComplete(fragments)) {
            reorderBuffer[seqNum.get()] = fragments;
            return true;
        }
    }
    return false;
}

void BlockAckReordering::releaseReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, const ReorderBuffer& reorderBuffer)
{
    for (const auto& it : getFramesInOrder(reorderBuffer)) {
        auto sequenceNumber = it.first;
        passedUp(agreement, receiveBuffer, SequenceNumberCyclic(sequenceNumber));
    }
}

bool BlockAckReordering::isComplete(const std::vector<Packet *>& fragments)
{
    int largestFragmentNumber = -1;
    std::set<FragmentNumber> fragNums; // possible duplicate frames
    for (auto fragment : fragments) {
        const auto& header = fragment->peekAtFront<Ieee80211DataHeader>();
        if (!header->getMoreFragments())
            largestFragmentNumber = header->getFragmentNumber();
        fragNums.insert(header->getFragmentNumber());
    }
    return largestFragmentNumber != -1 && largestFragmentNumber + 1 == (int)fragNums.size();
}

ReceiveBuffer *BlockAckReordering::createReceiveBufferIfNecessary(RecipientBlockAckAgreement *agreement)
{
    SequenceNumberCyclic startingSequenceNumber = agreement->getStartingSequenceNumber();
    int bufferSize = agreement->getBufferSize();
    Tid tid = agreement->getBlockAckRecord()->getTid();
    MacAddress originatorAddr = agreement->getBlockAckRecord()->getOriginatorAddress();
    auto id = std::make_pair(tid, originatorAddr);
    auto it = receiveBuffers.find(id);
    if (it == receiveBuffers.end()) {
        ReceiveBuffer *buffer = new ReceiveBuffer(bufferSize, startingSequenceNumber);
        receiveBuffers[id] = buffer;
        return buffer;
    }
    else
        return it->second;
}

void BlockAckReordering::processReceivedDelba(const Ptr<const Ieee80211Delba>& delba)
{
    if (delba->getInitiator())
        clearReceiveBuffer(delba->getTransmitterAddress(), delba->getTid());
}

void BlockAckReordering::clearReceiveBuffer(MacAddress originatorAddr, Tid tid)
{
    auto id = std::make_pair(tid, originatorAddr);
    auto it = receiveBuffers.find(id);
    if (it != receiveBuffers.end()) {
        delete it->second;
        receiveBuffers.erase(it);
    }
    else
        EV_DETAIL << "Receive buffer is not found" << endl;
}

void BlockAckReordering::passedUp(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, SequenceNumberCyclic sequenceNumber)
{
    // Each time that the recipient passes an MSDU or A-MSDU for a Block Ack agreement up to the next MAC
    // process, the NextExpectedSequenceNumber for that Block Ack agreement is set to the sequence number of the
    // MSDU or A-MSDU that was passed up to the next MAC process plus one.
    receiveBuffer->setNextExpectedSequenceNumber(sequenceNumber + 1);
    receiveBuffer->dropFramesUntil(sequenceNumber);
    receiveBuffer->removeFrame(sequenceNumber);
}

std::vector<Packet *> BlockAckReordering::getEarliestCompleteMsduOrAMsduIfExists(ReceiveBuffer *receiveBuffer)
{
    Fragments earliestFragments = Fragments();
    SequenceNumberCyclic earliestSeqNum = SequenceNumberCyclic(0);
    const auto& buffer = receiveBuffer->getBuffer();
    for (auto it : buffer) {
        if (isComplete(it.second)) {
            earliestFragments = it.second;
            earliestSeqNum = earliestFragments.at(0)->peekAtFront<Ieee80211DataOrMgmtHeader>()->getSequenceNumber();
            break;
        }
    }
    if (earliestFragments.size() > 0) {
        for (auto it : buffer) {
            SequenceNumberCyclic currentSeqNum = it.second.at(0)->peekAtFront<Ieee80211DataOrMgmtHeader>()->getSequenceNumber();
            if (currentSeqNum < earliestSeqNum) {
                if (isComplete(it.second)) {
                    earliestFragments = it.second;
                    earliestSeqNum = currentSeqNum;
                }
            }
        }
    }
    return earliestFragments;
}

BlockAckReordering::~BlockAckReordering()
{
    for (auto receiveBuffer : receiveBuffers)
        delete receiveBuffer.second;
}

} /* namespace ieee80211 */
} /* namespace inet */
