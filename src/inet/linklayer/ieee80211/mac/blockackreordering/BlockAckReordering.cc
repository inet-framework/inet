//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"

#include "inet/linklayer/ieee80211/mac/blockack/OneTidBlockAckReqVariant.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    ReceiveBuffer *receiveBuffer = createReceiveBufferIfNecessary(agreement);
    ReorderBuffer framesToPassUp;
    auto sequenceNumber = dataHeader->getSequenceNumber();
    auto startingSequenceNumber = receiveBuffer->getNextExpectedSequenceNumber();
    bool advancesWindow = startingSequenceNumber + receiveBuffer->getBufferSize() <= sequenceNumber && sequenceNumber < startingSequenceNumber + 2048;
    // IEEE Std 802.11-2024, 10.25.6.3 and 10.25.6.4: update the
    // scoreboard for every related Data frame, independently of reorder storage.
    agreement->dataFrameReceived(dataHeader);
    if (advancesWindow) {
        // IEEE Std 802.11-2024, 10.25.6.6.2.1(b): store the future MPDU
        // before moving WinStartB and releasing complete displaced MSDUs.
        auto newStartingSequenceNumber = sequenceNumber - receiveBuffer->getBufferSize() + 1;
        if (!receiveBuffer->insertFrame(dataPacket, dataHeader, newStartingSequenceNumber)) {
            delete dataPacket;
            return framesToPassUp;
        }
        framesToPassUp = collectCompletePrecedingMpdus(receiveBuffer, newStartingSequenceNumber);
        for (const auto& entry : framesToPassUp)
            receiveBuffer->removeFrame(SequenceNumberCyclic(entry.first));
        // Any remaining displaced entries are incomplete and cannot be delivered.
        receiveBuffer->dropFramesUntil(newStartingSequenceNumber);
        receiveBuffer->setNextExpectedSequenceNumber(newStartingSequenceNumber);
    }
    // The reception of QoS data frames using Normal Ack policy shall not be used by the
    // recipient to reset the timer to detect Block Ack timeout (see 10.5.4).
    // This allows the recipient to delete the Block Ack if the originator does not switch
    // back to using Block Ack.
    if (!advancesWindow && !receiveBuffer->insertFrame(dataPacket, dataHeader)) {
        delete dataPacket;
        return framesToPassUp;
    }
    if (advancesWindow) {
        auto consecutiveCompleteMpdus = collectConsecutiveCompleteFollowingMpdus(receiveBuffer, receiveBuffer->getNextExpectedSequenceNumber());
        releaseReceiveBuffer(receiveBuffer, consecutiveCompleteMpdus);
        framesToPassUp.insert(framesToPassUp.end(), consecutiveCompleteMpdus.begin(), consecutiveCompleteMpdus.end());
        return framesToPassUp;
    }
    auto earliestCompleteMsduOrAMsdu = getEarliestCompleteMsduOrAMsduIfExists(receiveBuffer);
    if (earliestCompleteMsduOrAMsdu.size() > 0) {
        auto earliestSequenceNumber = earliestCompleteMsduOrAMsdu.at(0)->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
        // If, after an MPDU is received, the receive buffer is full, the complete MSDU or A-MSDU with the earliest
        // sequence number shall be passed up to the next MAC process.
        if (receiveBuffer->isFull()) {
            passedUp(receiveBuffer, earliestSequenceNumber);
            return ReorderBuffer({ std::make_pair(earliestSequenceNumber.get(), Fragments(earliestCompleteMsduOrAMsdu)) });
        }
        // If, after an MPDU is received, the receive buffer is not full, but the sequence number of the complete MSDU or
        // A-MSDU in the buffer with the lowest sequence number is equal to the NextExpectedSequenceNumber for
        // that Block Ack agreement, then the MPDU shall be passed up to the next MAC process.
        else if (earliestSequenceNumber == receiveBuffer->getNextExpectedSequenceNumber()) {
            passedUp(receiveBuffer, earliestSequenceNumber);
            return ReorderBuffer({ std::make_pair(earliestSequenceNumber.get(), Fragments(earliestCompleteMsduOrAMsdu)) });
        }
    }
    return framesToPassUp;
}

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedBlockAckReq(RecipientBlockAckAgreement *agreement, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    // The originator shall use the Block Ack starting sequence control to signal the first MPDU in the block for
    // which an acknowledgment is expected.
    auto blockAckReqDetails = getOneTidBlockAckReqDetails(blockAckReq);
    if (!blockAckReqDetails)
        throw cRuntimeError("Multi-Tid BlockAckReq is currently an unimplemented feature");
    auto startingSequenceNumber = blockAckReqDetails->startingSequenceNumber;
    // IEEE Std 802.11-2024, 10.25.6.3-10.25.6.5: adjust WinStartR
    // from the BAR before generating the response, even without a receive buffer.
    agreement->getBlockAckRecord()->advanceStartingSequenceNumber(startingSequenceNumber);
    auto id = std::make_pair(blockAckReqDetails->tid, blockAckReq->getTransmitterAddress());
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
        // The recipient shall pass MSDUs and A-MSDUs up to the next MAC process in order of increasing sequence
        // number.
        completePrecedingMpdus.insert(completePrecedingMpdus.end(), consecutiveCompleteFollowingMpdus.begin(), consecutiveCompleteFollowingMpdus.end());
        // Detach all packets being returned before releasing stale buffered entries.
        releaseReceiveBuffer(receiveBuffer, completePrecedingMpdus);
        // Release any remaining buffers held by incomplete preceding MPDUs, then
        // advance NextExpectedSequenceNumber to at least the BAR SSN without
        // regressing it past consecutively released MSDUs.
        receiveBuffer->dropFramesUntil(startingSequenceNumber);
        if (receiveBuffer->getNextExpectedSequenceNumber() < startingSequenceNumber)
            receiveBuffer->setNextExpectedSequenceNumber(startingSequenceNumber);
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
    auto currentStartingSequenceNumber = receiveBuffer->getNextExpectedSequenceNumber();
    for (int i = 0; i < receiveBuffer->getBufferSize(); i++) {
        auto sequenceNumber = currentStartingSequenceNumber + i;
        if (!(sequenceNumber < startingSequenceNumber))
            break;
        auto it = buffer.find(sequenceNumber.get());
        if (it != buffer.end() && isComplete(it->second))
            completePrecedingMpdus.push_back(std::make_pair(sequenceNumber.get(), it->second));
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
            reorderBuffer.push_back(std::make_pair(seqNum.get(), fragments));
            return true;
        }
    }
    return false;
}

void BlockAckReordering::releaseReceiveBuffer(ReceiveBuffer *receiveBuffer, const ReorderBuffer& reorderBuffer)
{
    // Detach all packets whose ownership is returned before stale-buffer cleanup.
    for (const auto& entry : reorderBuffer)
        receiveBuffer->removeFrame(SequenceNumberCyclic(entry.first));
    for (const auto& entry : reorderBuffer) {
        auto sequenceNumber = entry.first;
        receiveBuffer->setNextExpectedSequenceNumber(SequenceNumberCyclic(sequenceNumber) + 1);
        receiveBuffer->dropFramesUntil(SequenceNumberCyclic(sequenceNumber));
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

std::vector<Packet *> BlockAckReordering::resetReceiveBuffer(Tid tid, MacAddress originatorAddr)
{
    std::vector<Packet *> frames;
    auto id = std::make_pair(tid, originatorAddr);
    auto it = receiveBuffers.find(id);
    if (it != receiveBuffers.end()) {
        frames = it->second->extractFrames();
        delete it->second;
        receiveBuffers.erase(it);
    }
    return frames;
}

void BlockAckReordering::passedUp(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic sequenceNumber)
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
