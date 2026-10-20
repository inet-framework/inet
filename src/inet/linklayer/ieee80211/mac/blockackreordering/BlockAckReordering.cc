//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

BlockAckReordering::ReleasedFrames BlockAckReordering::detachAgreement(MacAddress peer, Tid tid,
        bool deliverComplete, std::vector<Packet *>& discarded)
{
    auto it = receiveBuffers.find({tid, peer});
    if (it == receiveBuffers.end())
        return {};
    auto buffer = it->second;
    receiveBuffers.erase(it);
    ReleasedFrames released;
    auto start = buffer->getNextExpectedSequenceNumber();
    for (int i = 0; i < buffer->getBufferSize(); i++) {
        auto sequence = start + i;
        auto position = buffer->getBuffer().find(sequence.get());
        if (position == buffer->getBuffer().end())
            continue;
        auto fragments = position->second;
        if (deliverComplete && isComplete(fragments))
            released.push_back({sequence.get(), fragments});
        else
            discarded.insert(discarded.end(), fragments.begin(), fragments.end());
        buffer->removeFrame(sequence);
    }
    delete buffer;
    return released;
}

void BlockAckReordering::clearPeer(MacAddress peer)
{
    for (auto it = receiveBuffers.begin(); it != receiveBuffers.end();) {
        if (it->first.second == peer) {
            delete it->second;
            it = receiveBuffers.erase(it);
        }
        else
            ++it;
    }
}

BlockAckReordering::ReleasedFrames BlockAckReordering::processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    ReceiveBuffer *receiveBuffer = createReceiveBufferIfNecessary(agreement);
    auto sequenceNumber = dataHeader->getSequenceNumber();
    auto windowStart = receiveBuffer->getNextExpectedSequenceNumber();
    ReleasedFrames releasedFrames;
    // IEEE Std 802.11-2024, 10.25.6.6.2.1(b,c): advance for newer DATA beyond the window; discard older DATA.
    // The shared reorder service also retains Basic fragment reassembly as an INET compatibility choice.
    if (!(windowStart <= sequenceNumber)) {
        delete dataPacket;
        return releasedFrames;
    }
    if (!(sequenceNumber < windowStart + receiveBuffer->getBufferSize()))
        releasedFrames = advanceReceiveBuffer(agreement, receiveBuffer, sequenceNumber - receiveBuffer->getBufferSize() + 1);
    // The reception of QoS data frames using Normal Ack policy shall not be used by the
    // recipient to reset the timer to detect Block Ack timeout (see 10.5.4).
    // This allows the recipient to delete the Block Ack if the originator does not switch
    // back to using Block Ack.
    if (receiveBuffer->insertFrame(dataPacket, dataHeader)) {
        if (dataHeader->getAckPolicy() == BLOCK_ACK)
            agreement->blockAckPolicyFrameReceived(dataHeader);
        // IEEE Std 802.11-2024, 10.25.6.6.2.1(a)(2): release every consecutive complete unit at the window start.
        auto consecutiveFrames = collectConsecutiveCompleteFollowingMpdus(receiveBuffer, receiveBuffer->getNextExpectedSequenceNumber());
        releaseReceiveBuffer(agreement, receiveBuffer, consecutiveFrames);
        releasedFrames.insert(releasedFrames.end(), consecutiveFrames.begin(), consecutiveFrames.end());
    }
    else
        delete dataPacket;
    return releasedFrames;
}

BlockAckReordering::ReleasedFrames BlockAckReordering::processReceivedBlockAckReq(RecipientBlockAckAgreement *agreement, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    SequenceNumberCyclic startingSequenceNumber;
    if (auto basicReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        // IEEE Std 802.11e-2005, 7.2.1.7: Basic BAR uses a zero Fragment Number.
        // Reject inconsistent model fields before record creation.
        if (basicReq->getFragmentNumber() != 0 || basicReq->getMultiTid() || basicReq->getCompressedBitmap() ||
            basicReq->getTidInfo() < 0 || basicReq->getTidInfo() > 15 || !basicReq->getStartingSequenceNumber().isSpecified())
            return ReleasedFrames();
        startingSequenceNumber = basicReq->getStartingSequenceNumber();
    }
    else {
        throw cRuntimeError("Multi-Tid BlockAckReq is currently an unimplemented feature");
    }
    ReceiveBuffer *receiveBuffer = createReceiveBufferIfNecessary(agreement);
    // IEEE Std 802.11-2024, 10.25.6.6.3.1: only a newer BAR advances the window, including an empty window.
    if (!(receiveBuffer->getNextExpectedSequenceNumber() < startingSequenceNumber))
        return ReleasedFrames();
    auto releasedFrames = advanceReceiveBuffer(agreement, receiveBuffer, startingSequenceNumber);
    auto consecutiveFrames = collectConsecutiveCompleteFollowingMpdus(receiveBuffer, startingSequenceNumber);
    releaseReceiveBuffer(agreement, receiveBuffer, consecutiveFrames);
    releasedFrames.insert(releasedFrames.end(), consecutiveFrames.begin(), consecutiveFrames.end());
    return releasedFrames;
}

BlockAckReordering::ReleasedFrames BlockAckReordering::advanceReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber)
{
    auto releasedFrames = collectCompletePrecedingMpdus(receiveBuffer, startingSequenceNumber);
    releaseReceiveBuffer(agreement, receiveBuffer, releasedFrames);
    receiveBuffer->dropFramesUntil(startingSequenceNumber);
    receiveBuffer->setNextExpectedSequenceNumber(startingSequenceNumber);
    return releasedFrames;
}

BlockAckReordering::ReleasedFrames BlockAckReordering::collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber)
{
    ReleasedFrames completePrecedingMpdus;
    auto windowStart = receiveBuffer->getNextExpectedSequenceNumber();
    // Numeric map order cannot preserve delivery order across 4095 -> 0.
    for (int i = 0; i < receiveBuffer->getBufferSize() && windowStart + i < startingSequenceNumber; i++)
        addMsduIfComplete(receiveBuffer, completePrecedingMpdus, windowStart + i);
    return completePrecedingMpdus;
}

BlockAckReordering::ReleasedFrames BlockAckReordering::collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber)
{
    ReleasedFrames framesToPassUp;
    for (int i = 0; i < receiveBuffer->getBufferSize(); i++) {
        if (!addMsduIfComplete(receiveBuffer, framesToPassUp, startingSequenceNumber + i))
            return framesToPassUp;
    }
    return framesToPassUp;
}

bool BlockAckReordering::addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReleasedFrames& releasedFrames, SequenceNumberCyclic seqNum)
{
    const auto& buffer = receiveBuffer->getBuffer();
    auto it = buffer.find(seqNum.get());
    if (it != buffer.end()) {
        auto fragments = it->second;
        if (isComplete(fragments)) {
            releasedFrames.push_back({seqNum.get(), fragments});
            return true;
        }
    }
    return false;
}

void BlockAckReordering::releaseReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, const ReleasedFrames& releasedFrames)
{
    for (const auto& it : releasedFrames) {
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
    Tid tid = delba->getTid();
    MacAddress originatorAddr = delba->getTransmitterAddress();
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
    agreement->getBlockAckRecord()->removeAckStates(sequenceNumber);
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

