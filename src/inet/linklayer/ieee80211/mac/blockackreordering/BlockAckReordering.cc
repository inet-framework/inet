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

#include "BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Ieee80211DataFrame* dataFrame)
{
    ReceiveBuffer *receiveBuffer = createReceiveBufferIfNecessary(agreement);
    // The reception of QoS data frames using Normal Ack policy shall not be used by the
    // recipient to reset the timer to detect Block Ack timeout (see 10.5.4).
    // This allows the recipient to delete the Block Ack if the originator does not switch
    // back to using Block Ack.
    if (receiveBuffer->insertFrame(dataFrame)) {
        if (dataFrame->getAckPolicy() == BLOCK_ACK)
            agreement->blockAckPolicyFrameReceived(dataFrame);
        auto earliestCompleteMsduOrAMsdu = getEarliestCompleteMsduOrAMsduIfExists(receiveBuffer);
        if (earliestCompleteMsduOrAMsdu.size() > 0) {
            int earliestSequenceNumber = earliestCompleteMsduOrAMsdu.at(0)->getSequenceNumber();
            // If, after an MPDU is received, the receive buffer is full, the complete MSDU or A-MSDU with the earliest
            // sequence number shall be passed up to the next MAC process.
            if (receiveBuffer->isFull()) {
                passedUp(receiveBuffer, earliestSequenceNumber);
                return ReorderBuffer({std::make_pair(earliestSequenceNumber, Fragments(earliestCompleteMsduOrAMsdu))});
            }
            // If, after an MPDU is received, the receive buffer is not full, but the sequence number of the complete MSDU or
            // A-MSDU in the buffer with the lowest sequence number is equal to the NextExpectedSequenceNumber for
            // that Block Ack agreement, then the MPDU shall be passed up to the next MAC process.
            else if (earliestSequenceNumber == receiveBuffer->getNextExpectedSequenceNumber()) {
                passedUp(receiveBuffer, earliestSequenceNumber);
                return ReorderBuffer({std::make_pair(earliestSequenceNumber, Fragments(earliestCompleteMsduOrAMsdu))});
            }
        }
    }
    else
        delete dataFrame;
    return ReorderBuffer({});
}

//
// The recipient flushes received MSDUs from its receive buffer as described in this subclause. [...]
//
BlockAckReordering::ReorderBuffer BlockAckReordering::processReceivedBlockAckReq(Ieee80211BlockAckReq* frame)
{
    // The originator shall use the Block Ack starting sequence control to signal the first MPDU in the block for
    // which an acknowledgment is expected.
    int startingSequenceNumber = -1;
    Tid tid = -1;
    if (auto basicReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(frame)) {
        tid = basicReq->getTidInfo();
        startingSequenceNumber = basicReq->getStartingSequenceNumber();
    }
    else if (auto compressedReq = dynamic_cast<Ieee80211CompressedBlockAck*>(frame)) {
        tid = compressedReq->getTidInfo();
        startingSequenceNumber = compressedReq->getStartingSequenceNumber();
    }
    else {
        throw cRuntimeError("Multi-Tid BlockAckReq is currently an unimplemented feature");
    }
    auto id = std::make_pair(tid, frame->getTransmitterAddress());
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
        if (numOfMsdusToPassUp == 0  && isSequenceNumberLess(receiveBuffer->getNextExpectedSequenceNumber(), startingSequenceNumber, receiveBuffer->getNextExpectedSequenceNumber(), receiveBuffer->getBufferSize()))
            receiveBuffer->setNextExpectedSequenceNumber(startingSequenceNumber);
        // The recipient shall then release any buffers held by preceding MPDUs.
        releaseReceiveBuffer(receiveBuffer, completePrecedingMpdus);
        releaseReceiveBuffer(receiveBuffer, consecutiveCompleteFollowingMpdus);
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
BlockAckReordering::ReorderBuffer BlockAckReordering::collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, int startingSequenceNumber)
{
    ASSERT(startingSequenceNumber != -1);
    ReorderBuffer completePrecedingMpdus;
    const auto& buffer = receiveBuffer->getBuffer();
    for (auto it : buffer) { // collects complete preceding MPDUs
        int sequenceNumber = it.first;
        auto fragments = it.second;
        if (isSequenceNumberLess(sequenceNumber, startingSequenceNumber, receiveBuffer->getNextExpectedSequenceNumber(), receiveBuffer->getBufferSize()))
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
BlockAckReordering::ReorderBuffer BlockAckReordering::collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, int startingSequenceNumber)
{
    ASSERT(startingSequenceNumber != -1);
    ReorderBuffer framesToPassUp;
    if (startingSequenceNumber + receiveBuffer->getBufferSize() > 4095) {
        for (int i = startingSequenceNumber; i <= 4095; i++) {
            if (!addMsduIfComplete(receiveBuffer, framesToPassUp, i))
                return framesToPassUp;
        }
        for (int i = 0; i < (startingSequenceNumber + receiveBuffer->getBufferSize()) % 4096; i++) {
            if (!addMsduIfComplete(receiveBuffer, framesToPassUp, i))
                return framesToPassUp;
        }
    }
    else {
        for (int i = startingSequenceNumber; i < startingSequenceNumber + receiveBuffer->getBufferSize(); i++) {
            if (!addMsduIfComplete(receiveBuffer, framesToPassUp, i))
                return framesToPassUp;
        }
    }
    return framesToPassUp;
}

bool BlockAckReordering::addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReorderBuffer& reorderBuffer, SequenceNumber seqNum)
{
    const auto& buffer = receiveBuffer->getBuffer();
    auto it = buffer.find(seqNum);
    if (it != buffer.end()) {
        auto fragments = it->second;
        if (isComplete(fragments)) {
            reorderBuffer[seqNum] = fragments;
            return true;
        }
    }
    return false;
}

void BlockAckReordering::releaseReceiveBuffer(ReceiveBuffer *receiveBuffer, const ReorderBuffer& reorderBuffer)
{
    for (auto it : reorderBuffer) {
        int sequenceNumber = it.first;
        passedUp(receiveBuffer, sequenceNumber);
    }
}


bool BlockAckReordering::isComplete(const std::vector<Ieee80211DataFrame*>& fragments)
{
    int largestFragmentNumber = -1;
    std::set<FragmentNumber> fragNums; // possible duplicate frames
    for (auto fragment : fragments) {
        if (!fragment->getMoreFragments())
            largestFragmentNumber = fragment->getFragmentNumber();
        fragNums.insert(fragment->getFragmentNumber());
    }
    return largestFragmentNumber != -1 && largestFragmentNumber + 1 == (int)fragNums.size();
}

ReceiveBuffer* BlockAckReordering::createReceiveBufferIfNecessary(RecipientBlockAckAgreement *agreement)
{
    SequenceNumber startingSequenceNumber = agreement->getStartingSequenceNumber();
    int bufferSize = agreement->getBufferSize();
    Tid tid = agreement->getBlockAckRecord()->getTid();
    MACAddress originatorAddr = agreement->getBlockAckRecord()->getOriginatorAddress();
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

void BlockAckReordering::processReceivedDelba(Ieee80211Delba* delba)
{
    Tid tid = delba->getTid();
    MACAddress originatorAddr = delba->getTransmitterAddress();
    auto id = std::make_pair(tid, originatorAddr);
    auto it = receiveBuffers.find(id);
    if (it != receiveBuffers.end()) {
        delete it->second;
        receiveBuffers.erase(it);
    }
    else
        EV_DETAIL << "Receive buffer is not found" << endl;
}

void BlockAckReordering::passedUp(ReceiveBuffer *receiveBuffer, int sequenceNumber)
{
    // Each time that the recipient passes an MSDU or A-MSDU for a Block Ack agreement up to the next MAC
    // process, the NextExpectedSequenceNumber for that Block Ack agreement is set to the sequence number of the
    // MSDU or A-MSDU that was passed up to the next MAC process plus one.
    receiveBuffer->setNextExpectedSequenceNumber((sequenceNumber + 1) % 4096);
    receiveBuffer->remove(sequenceNumber);
}

std::vector<Ieee80211DataFrame*> BlockAckReordering::getEarliestCompleteMsduOrAMsduIfExists(ReceiveBuffer *receiveBuffer)
{
    Fragments earliestFragments = Fragments();
    SequenceNumber earliestSeqNum = 0;
    const auto& buffer = receiveBuffer->getBuffer();
    for (auto it : buffer) {
        if (isComplete(it.second)) {
            earliestFragments = it.second;
            earliestSeqNum = earliestFragments.at(0)->getSequenceNumber();
            break;
        }
    }
    if (earliestFragments.size() > 0) {
        for (auto it : buffer) {
            SequenceNumber currentSeqNum = it.second.at(0)->getSequenceNumber();
            if (isSequenceNumberLess(currentSeqNum, earliestSeqNum, receiveBuffer->getNextExpectedSequenceNumber(), receiveBuffer->getBufferSize())) {
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
