//
// Copyright (C) 2007-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/sctp/SCTPAlgorithm.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

#include <assert.h>
#include <algorithm>

namespace inet {

namespace sctp {

void SCTPAssociation::increaseOutstandingBytes(SCTPDataVariables *chunk,
        SCTPPathVariables *path)
{
    path->outstandingBytes += chunk->booksize;
    path->statisticsPathOutstandingBytes->record(path->outstandingBytes);
    state->outstandingBytes += chunk->booksize;
    statisticsOutstandingBytes->record(state->outstandingBytes);

    CounterMap::iterator iterator = qCounter.roomRetransQ.find(path->remoteAddress);
    state->outstandingMessages++;
    if (state->osbWithHeader)
        iterator->second += ADD_PADDING(chunk->booksize);
    else
        iterator->second += ADD_PADDING(chunk->booksize + SCTP_DATA_CHUNK_LENGTH);
}

void SCTPAssociation::storePacket(SCTPPathVariables *pathVar,
        SCTPMessage *sctpMsg,
        const uint16 chunksAdded,
        const uint16 dataChunksAdded,
        const bool authAdded)
{
    uint32 packetBytes = 0;
    for (uint16 i = 0; i < sctpMsg->getChunksArraySize(); i++) {
        SCTPDataVariables *chunk = retransmissionQ->payloadQueue.find(((SCTPDataChunk *)sctpMsg->getChunks(i))->getTsn())->second;
        decreaseOutstandingBytes(chunk);
        chunk->queuedOnPath->queuedBytes -= chunk->booksize;
        chunk->queuedOnPath = NULL;
        packetBytes += chunk->booksize;
    }
    state->sctpMsg = sctpMsg;
    state->chunksAdded = chunksAdded;
    state->dataChunksAdded = dataChunksAdded;
    state->packetBytes = packetBytes;
    state->authAdded = authAdded;
    EV_INFO << "storePacket: path=" << pathVar->remoteAddress
            << " state->packetBytes=" << state->packetBytes
            << " osb=" << pathVar->outstandingBytes << " -> "
            << pathVar->outstandingBytes - state->packetBytes << endl;
    if (state->osbWithHeader)
        qCounter.roomSumSendStreams += state->packetBytes;
    else
        qCounter.roomSumSendStreams += state->packetBytes + (dataChunksAdded * SCTP_DATA_CHUNK_LENGTH);
    qCounter.bookedSumSendStreams += state->packetBytes;
}

void SCTPAssociation::loadPacket(SCTPPathVariables *pathVar,
        SCTPMessage **sctpMsg,
        uint16 *chunksAdded,
        uint16 *dataChunksAdded,
        bool *authAdded)
{
    *sctpMsg = state->sctpMsg;
    state->sctpMsg = NULL;
    *chunksAdded = state->chunksAdded;
    *dataChunksAdded = state->dataChunksAdded;
    *authAdded = state->authAdded;
    EV_INFO << "loadPacket: path=" << pathVar->remoteAddress << " osb=" << pathVar->outstandingBytes << " -> " << pathVar->outstandingBytes + state->packetBytes << endl;
    if (state->osbWithHeader) {
        qCounter.roomSumSendStreams -= state->packetBytes;
    }
    else {
        qCounter.roomSumSendStreams -= state->packetBytes + ((*dataChunksAdded) * SCTP_DATA_CHUNK_LENGTH);
    }
    qCounter.bookedSumSendStreams -= state->packetBytes;

    for (uint16 i = 0; i < (*sctpMsg)->getChunksArraySize(); i++) {
        SCTPDataVariables *chunk = retransmissionQ->payloadQueue.find(((SCTPDataChunk *)(*sctpMsg)->getChunks(i))->getTsn())->second;
        chunk->queuedOnPath = pathVar;
        chunk->queuedOnPath->queuedBytes += chunk->booksize;
        chunk->setLastDestination(pathVar);
        increaseOutstandingBytes(chunk, pathVar);
        chunk->countsAsOutstanding = true;
    }
}

std::vector<SCTPPathVariables *> SCTPAssociation::getSortedPathMap()
{
    std::vector<SCTPPathVariables *> sortedPaths;
    for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        SCTPPathVariables *path = iterator->second;
        sortedPaths.insert(sortedPaths.end(), path);
    }
    if (state->cmtSendAllComparisonFunction != NULL) {
        std::sort(sortedPaths.begin(), sortedPaths.end(), state->cmtSendAllComparisonFunction);
    }

    EV << "SORTED PATH MAP:" << endl;
    for (std::vector<SCTPPathVariables *>::iterator iterator = sortedPaths.begin(); iterator != sortedPaths.end(); ++iterator) {
        SCTPPathVariables *path = *iterator;
        EV << " - " << path->remoteAddress
           << "  cwnd=" << path->cwnd
           << "  ssthresh=" << path->ssthresh
           << "  outstanding=" << path->outstandingBytes
           << "  bytesToRetransmit=" << state->bytesToRetransmit << endl;
    }
    return sortedPaths;
}

bool SCTPAssociation::pathMapRandomized(const SCTPPathVariables *left, const SCTPPathVariables *right)
{
    return left->sendAllRandomizer < right->sendAllRandomizer;
}

bool SCTPAssociation::pathMapSmallestLastTransmission(const SCTPPathVariables *left, const SCTPPathVariables *right)
{
    return left->lastTransmission < right->lastTransmission;
}

bool SCTPAssociation::pathMapLargestSSThreshold(const SCTPPathVariables *left, const SCTPPathVariables *right)
{
    return left->ssthresh > right->ssthresh;
}

bool SCTPAssociation::pathMapLargestSpace(const SCTPPathVariables *left, const SCTPPathVariables *right)
{
    const int l = (left->cwnd - left->outstandingBytes);
    const int r = (right->cwnd - right->outstandingBytes);
    return l > r;
}

bool SCTPAssociation::pathMapLargestSpaceAndSSThreshold(const SCTPPathVariables *left, const SCTPPathVariables *right)
{
    if (left->ssthresh != right->ssthresh) {
        return left->ssthresh > right->ssthresh;
    }

    const int l = (left->cwnd - left->outstandingBytes);
    const int r = (right->cwnd - right->outstandingBytes);
    return l > r;
}

SCTPDataVariables *SCTPAssociation::makeDataVarFromDataMsg(SCTPDataMsg *datMsg,
        SCTPPathVariables *path)
{
    SCTPDataVariables *datVar = new SCTPDataVariables();

    datMsg->setInitialDestination(path->remoteAddress);
    datVar->setInitialDestination(path);

    datVar->bbit = datMsg->getBBit();
    datVar->ebit = datMsg->getEBit();
    datVar->ibit = datMsg->getSackNow();
    datVar->enqueuingTime = datMsg->getEnqueuingTime();
    datVar->expiryTime = datMsg->getExpiryTime();
    datVar->ppid = datMsg->getPpid();
    datVar->len = datMsg->getBitLength();
    datVar->sid = datMsg->getSid();
    datVar->allowedNoRetransmissions = datMsg->getRtx();
    datVar->booksize = datMsg->getBooksize();
    datVar->prMethod = datMsg->getPrMethod();
    datVar->priority = datMsg->getPriority();
    datVar->strReset = datMsg->getStrReset();

    // ------ Stream handling ---------------------------------------
    SCTPSendStreamMap::iterator iterator = sendStreams.find(datMsg->getSid());
    SCTPSendStream *stream = iterator->second;
    uint32 nextSSN = stream->getNextStreamSeqNum();
    datVar->userData = datMsg->decapsulate();
    if (datMsg->getOrdered()) {
        // ------ Ordered mode: assign SSN ---------
        if (datMsg->getEBit()) {
            datVar->ssn = nextSSN++;
        }
        else {
            datVar->ssn = nextSSN;
        }

        datVar->ordered = true;
        if (nextSSN > 65535) {
            stream->setNextStreamSeqNum(0);
        }
        else {
            stream->setNextStreamSeqNum(nextSSN);
        }
    }
    else {
        // ------ Ordered mode: no SSN needed ------
        datVar->ssn = 0;
        datVar->ordered = false;
    }

    state->ssLastDataChunkSizeSet = true;

    return datVar;
}

SCTPPathVariables *SCTPAssociation::choosePathForRetransmission()
{
    uint32 max = 0;
    SCTPPathVariables *temp = NULL;

    for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        SCTPPathVariables *path = iterator->second;
        CounterMap::const_iterator tq = qCounter.roomTransQ.find(path->remoteAddress);
        if ((tq != qCounter.roomTransQ.end()) && (tq->second > max)) {
            max = tq->second;
            temp = path;
        }
    }
    return temp;
}

void SCTPAssociation::timeForSack(bool& sackOnly, bool& sackWithData)
{
    sackOnly = sackWithData = false;
    // CMT DAC implementation at the receiver side
    // If CMT DAC is used, a SACK is *not* immediately transferred upon reordering
    if (((state->gapList.getNumGaps(SCTPGapList::GT_Any) > 0) || (state->dupList.size() > 0)) &&
        (state->sackAllowed) &&
        (!((state->allowCMT == true) && (state->cmtUseDAC == true))))
    {
        // Schedule sending of SACKs at once, when we have fragments to report
        state->ackState = sackFrequency;
        sackOnly = sackWithData = true;    // SACK necessary, regardless of data available
    }
    if (state->ackState >= sackFrequency) {
        sackOnly = sackWithData = true;    // SACK necessary, regardless of data available
    }
    else if (SackTimer->isScheduled()) {
        sackOnly = false;
        sackWithData = true;    // Only send SACK when data is present.
    }
}

void SCTPAssociation::sendOnAllPaths(SCTPPathVariables *firstPath)
{
    if (state->allowCMT) {
        // ------ Send on provided path first ... -----------------------------
        if (firstPath != NULL) {
            sendOnPath(firstPath);
        }

        // ------ ... then, try sending on all other paths --------------------
        std::vector<SCTPPathVariables *> sortedPaths = getSortedPathMap();
        for (std::vector<SCTPPathVariables *>::iterator iterator = sortedPaths.begin(); iterator != sortedPaths.end(); ++iterator) {
            SCTPPathVariables *path = *iterator;
            EV << path->remoteAddress << " [" << path->lastTransmission << "]\t";
        }
        EV << endl;

        for (std::vector<SCTPPathVariables *>::iterator iterator = sortedPaths.begin();
             iterator != sortedPaths.end(); ++iterator)
        {
            SCTPPathVariables *path = *iterator;
            if (path != firstPath) {
                sendOnPath(path);
                path->sendAllRandomizer = uniform(0, (1 << 31));
            }
        }
        if ((state->strictCwndBooking) &&
            (sctpPathMap.size() > 1))    // T.D. 08.02.2010: strict behaviour only for more than 1 paths!
        {    // T.D. 14.01.2010: Second pass for "Strict Cwnd Booking" option.
            for (std::vector<SCTPPathVariables *>::iterator iterator = sortedPaths.begin();
                 iterator != sortedPaths.end(); ++iterator)
            {
                SCTPPathVariables *path = *iterator;
                sendOnPath(path, false);
            }
        }
    }
    else {
        // ------ Send on provided path first ... -----------------------------
        if (firstPath != NULL) {
            sendOnPath(firstPath);
        }

        // ------ ... then, try sending on all other paths --------------------
        for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
            SCTPPathVariables *path = iterator->second;
            if (path != firstPath) {
                sendOnPath(path);
            }
        }
        if (state->strictCwndBooking) {
            // T.D. 14.01.2010: Second pass for "Strict Cwnd Booking" option.
            sendOnPath(firstPath, false);

            // ------ Then, try sending on all other paths ---------------------------
            for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
                SCTPPathVariables *path = iterator->second;
                if (path != firstPath) {
                    sendOnPath(path, false);
                }
            }
        }
    }
}

void SCTPAssociation::chunkReschedulingControl(SCTPPathVariables *path)
{
    EV << simTime() << ": chunkReschedulingControl:"
       << "\tqueueFillLevel=" << (100.0 * (double)state->queuedSentBytes) / (double)state->sendQueueLimit << "%"
       << "\tpeerRwnd=" << state->peerRwnd
       << endl;

    double totalBandwidth = 0.0;
    unsigned int totalOutstandingBytes = 0;
    unsigned int totalQueuedBytes = 0;
    for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        const SCTPPathVariables *myPath = iterator->second;
        totalQueuedBytes += myPath->queuedBytes;
        totalOutstandingBytes += myPath->outstandingBytes;
        totalBandwidth += (double)myPath->cwnd / myPath->srtt.dbl();
    }
    assert(totalOutstandingBytes == state->outstandingBytes);

    const unsigned int queuedBytes = path->queuedBytes;
    const unsigned int outstandingBytes = path->outstandingBytes;

    /* senderLimit is just the size of the send queue! */
    uint32 senderLimit = ((state->sendQueueLimit != 0) ? state->sendQueueLimit : 0xffffffff);
    if ((state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_BothSides) ||
        (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_SenderOnly) ||
        (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_ReceiverOnly))
    {
        senderLimit = senderLimit / sctpPathMap.size();
    }

    /* receiverLimit is the peerRwnd + all bytes queued
       (i.e. waiting or being outstanding)! */
    uint32 receiverLimit = state->peerRwnd + totalQueuedBytes;
    if ((state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_BothSides) ||
        (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_SenderOnly) ||
        (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_ReceiverOnly))
    {
        receiverLimit = receiverLimit / sctpPathMap.size();
    }

    const double senderBlockingFraction = (queuedBytes - outstandingBytes) / (double)senderLimit;
    const double receiverBlockingFraction = (queuedBytes - outstandingBytes) / (double)receiverLimit;

    EV << " - " << path->remoteAddress
       << "\tt3=" << (path->T3_RtxTimer->isScheduled() ? path->T3_RtxTimer->getArrivalTime().dbl() : -1.0)
       << "\tssthresh=" << path->ssthresh
       << "\tcwnd=" << path->cwnd
       << "\tsrtt=" << path->srtt
       << "\tbw=" << (8.0 * (double)path->cwnd) / (1000000.0 * path->srtt) << " Mbit/s"
       << "\tosb=" << path->outstandingBytes
       << "\tqueued=" << path->queuedBytes
       << "\tslimit=" << senderLimit
       << "\trlimit=" << receiverLimit
       << "\tsblocking=" << 100.0 * senderBlockingFraction << " %"
       << "\trblocking=" << 100.0 * receiverBlockingFraction << " %"
       << endl;

    path->statisticsPathSenderBlockingFraction->record(senderBlockingFraction);
    path->statisticsPathReceiverBlockingFraction->record(receiverBlockingFraction);

    // ====== Chunk Rescheduling =============================================
    if ((state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_SenderOnly) ||
        (state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_ReceiverOnly) ||
        (state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_BothSides))
    {
        if ((((state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_SenderOnly) ||
              (state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_BothSides)) &&
             (senderBlockingFraction >= state->cmtChunkReschedulingThreshold)) ||
            (((state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_ReceiverOnly) ||
              (state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_BothSides)) &&
             (receiverBlockingFraction >= state->cmtChunkReschedulingThreshold)))
        {
            if ((!path->fastRecoveryActive) &&    // Only apply when path is not yet in Fast Recovery!
                ((path->blockingTimeout < 0.0) ||    // Do not move chunks to an already-blocked path!
                 (simTime() >= path->blockingTimeout)))
            {
                // ====== Rescheduling of chunk from other path to current path ====
                SCTPQueue::PayloadQueue::iterator iterator = retransmissionQ->payloadQueue.begin();
                if (iterator != retransmissionQ->payloadQueue.end()) {
                    SCTPDataVariables *chunk = iterator->second;
                    SCTPPathVariables *lastPath = chunk->getLastDestinationPath();

                    if ((chunk->countsAsOutstanding == true) &&    // T.D. 04.08.2011: Chunk must be outstanding, i.e. in the network!
                        (chunk->hasBeenMoved == false) &&    // T.D. 08.07.2011: Check, whether this chunk has already been moved!
                        ((lastPath != path) ||
                         (chunk->sendTime + (2 * path->srtt) < simTime())))
                    {
                        assert(chunk->numberOfTransmissions > 0);    // It has been transmitted as least once
                        assert(chunk->hasBeenAcked == false);    // It has not been acked (since it is outstanding)

                        EV << simTime() << ": RESCHEDULING TSN " << chunk->tsn << " on "
                           << lastPath->remoteAddress << " to "
                           << path->remoteAddress << endl;

                        // ====== Move chunk ======================================
                        lastPath->vectorPathBlockingTSNsMoved->record(chunk->tsn);
                        moveChunkToOtherPath(chunk, path);

                        // This chunk is important, since it is blocking others ...
                        // If SackNow is supported, ensure that it is SACK'ed quickly!
                        chunk->ibit = sctpMain->sackNow;

                        state->blockingTSNsMoved++;
                        chunk->hasBeenMoved = (lastPath != path);

                        // Restart T3 timer on its old path, if it is scheduled
                        if (lastPath->T3_RtxTimer->isScheduled()) {
                            // Stop timer, if path is empty now.
                            // Else, keep it running, without reset!
                            if (lastPath->queuedBytes == 0) {
                                stopTimer(lastPath->T3_RtxTimer);
                            }
                        }

                        // ====== Handle Congestion Control =======================
                        if (state->cmtMovedChunksReduceCwnd == true) {
                            if (lastPath != path) {
                                if ((lastPath->blockingTimeout >= 0.0) &&
                                    (simTime() < lastPath->blockingTimeout))
                                {
                                    // Path is already blocked
                                    EV << simTime() << "\tCR-3 on path "
                                       << lastPath->remoteAddress << " (blocked until "
                                       << lastPath->blockingTimeout << ")" << endl;
                                }
                                else if (lastPath->fastRecoveryActive) {
                                    // A further problem during Fast Recovery -> block path
                                    const simtime_t pause = lastPath->srtt;
                                    lastPath->blockingTimeout = simTime() + pause;
                                    assert(!lastPath->BlockingTimer->isScheduled());
                                    startTimer(lastPath->BlockingTimer, pause);
                                    EV << simTime() << "\tCR-2 on path "
                                       << lastPath->remoteAddress << " (pause="
                                       << pause << "; blocked until "
                                       << lastPath->blockingTimeout << ")" << endl;
                                }
                                else {
                                    // Go into Fast Recovery mode ...
                                    lastPath->requiresRtx = true;
                                    (this->*ccFunctions.ccUpdateBeforeSack)();
                                    (this->*ccFunctions.ccUpdateAfterSack)();
                                    lastPath->requiresRtx = false;
                                    EV << simTime() << "\tCR-1 on path "
                                       << lastPath->remoteAddress << endl;
                                }
                            }
                            else {
                                EV << simTime() << "\tCR-4 on path "
                                   << path->remoteAddress << " (lastTX="
                                   << simTime() - chunk->sendTime
                                   << ", TSN " << chunk->tsn << ")" << endl;

                                lastPath->requiresRtx = true;
                                (this->*ccFunctions.ccUpdateBeforeSack)();
                                (this->*ccFunctions.ccUpdateAfterSack)();
                                lastPath->requiresRtx = false;
                            }
                        }
                    }
                }
            }
        }
    }
    // ====== Test ===========================================================
    else if (state->cmtChunkReschedulingVariant == SCTPStateVariables::CCRV_Test) {
        abort();
    }
}

void SCTPAssociation::sendSACKviaSelectedPath(SCTPMessage *sctpMsg)
{
    SCTPPathVariables *sackPath =
        (state->lastDataSourceList.size() > 0) ? state->lastDataSourceList.front() :
        state->lastDataSourcePath;
    assert(sackPath != NULL);

    if (state->allowCMT) {
        if (state->cmtSackPath == SCTPStateVariables::CSP_RoundRobin) {
            /* Observation:
               For same bandwidths but different delays, chunks on both paths arrive
               with same interleave time =>
               Chunk on A / Chunk on B => SACK on B
               Chunk on A / Chunk on B => SACK on B
               Chunk on A / Chunk on B => SACK on B
               Chunk on A / Chunk on B => SACK on B
               Chunk on A / Chunk on B => SACK on B
               ...
               RESULT: Losts of SACKs on B, no SACK on A  =>  Bad performance! */

            // Solution is now to make a round-robin selection among paths,
            // taking the path with the longest time passed since last SACK.
            for (std::list<SCTPPathVariables *>::iterator iterator = state->lastDataSourceList.begin();
                 iterator != state->lastDataSourceList.end(); iterator++)
            {
                SCTPPathVariables *path = *iterator;
                if (path->lastSACKSent < sackPath->lastSACKSent) {
                    sackPath = path;
                }
            }
        }
        else if (state->cmtSackPath == SCTPStateVariables::CSP_SmallestSRTT) {
            /* Instead of RR among last DATA paths, send SACK on
               the DATA path having the smallest SRTT. */
            for (std::list<SCTPPathVariables *>::iterator iterator = state->lastDataSourceList.begin();
                 iterator != state->lastDataSourceList.end(); iterator++)
            {
                SCTPPathVariables *path = *iterator;
                if (path->srtt < sackPath->srtt) {
                    sackPath = path;
                }
            }
        }
        else if (state->cmtSackPath == SCTPStateVariables::CSP_Primary) {
            sackPath = state->getPrimaryPath();
        }
    }
    EV_INFO << assocId << ": sending SACK to " << sackPath->remoteAddress << endl;
    sendToIP(sctpMsg, sackPath->remoteAddress);
    sackPath->lastSACKSent = simTime();
    state->lastDataSourceList.clear();    // Clear the address list!
}

void SCTPAssociation::bytesAllowedToSend(SCTPPathVariables *path,
        const bool firstPass)
{
    assert(path != NULL);

    bytes.chunk = false;
    bytes.packet = false;
    bytes.bytesToSend = 0;

    EV_INFO << "bytesAllowedToSend(" << path->remoteAddress << "):"
            << " osb=" << path->outstandingBytes << " cwnd=" << path->cwnd << endl;

    // ====== First transmission =============================================
    if (!state->firstDataSent) {
        bytes.chunk = true;
    }
    // ====== Transmission allowed by peer's receiver window? ================
    else if ((state->peerWindowFull || (state->peerAllowsChunks && state->peerMsgRwnd <= 0)) && (path->outstandingBytes == 0)) {
        // Zero Window Probing
        EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): zeroWindowProbing" << endl;
        state->zeroWindowProbing = true;
        bytes.chunk = true;
    }
    // ====== Retransmissions ================================================
    else {
        CounterMap::const_iterator it = qCounter.roomTransQ.find(path->remoteAddress);
        EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): bytes in transQ=" << it->second << endl;
        if (it->second > 0) {
            const int32 allowance = path->cwnd - path->outstandingBytes;
            EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd-osb=" << allowance << endl;
            if (state->peerRwnd < path->pmtu) {
                bytes.bytesToSend = 0;
                bytes.chunk = true;
                EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): one chunk" << endl;
                // RFC4960: When a Fast Retransmit is being performed, the sender SHOULD ignore the value
                // of cwnd and SHOULD NOT delay retransmission for this single packet.
                return;
            }
            else if (allowance > 0) {
                CounterMap::const_iterator bit = qCounter.bookedTransQ.find(path->remoteAddress);
                if (bit->second > (uint32)allowance) {
                    bytes.bytesToSend = allowance;
                    EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd does not allow all RTX" << endl;
                    return;    // More bytes available than allowed -> just return what is allowed.
                }
                else {
                    bytes.bytesToSend = bit->second;
                    EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd allows more than those "
                              << bytes.bytesToSend << " bytes for retransmission" << endl;
                }
            }
            else {    // You may retransmit one packet
                bytes.packet = true;
                EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): allowance<=0: retransmit one packet" << endl;
            }
        }

        // ====== New transmissions ===========================================
        if (!bytes.chunk && !bytes.packet) {
            (this->*ccFunctions.ccUpdateMaxBurst)(path);

            // ====== Get cwnd value to use, according to maxBurstVariant ======
            uint32 myCwnd = path->cwnd;
            if ((state->maxBurstVariant == SCTPStateVariables::MBV_UseItOrLoseItTempCwnd) ||
                (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimitingTempCwnd))
            {
                myCwnd = path->tempCwnd;
            }
            // ====== Obtain byte allowance ====================================
            if ((((state->peerAllowsChunks) &&
                  (path->outstandingBytes < myCwnd) &&
                  (!state->peerWindowFull) &&
                  (state->peerMsgRwnd > 0))
                 ||
                 ((!state->peerAllowsChunks) &&
                  (path->outstandingBytes < myCwnd) &&
                  (!state->peerWindowFull)))
                &&
                ((path->blockingTimeout < 0.0) ||    /* Chunk Rescheduling: no new transmission when blocking is active! */
                 (simTime() >= path->blockingTimeout)))
            {
                EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "):"
                          << " bookedSumSendStreams=" << qCounter.bookedSumSendStreams
                          << " bytes.bytesToSend=" << bytes.bytesToSend << endl;
                const int32 allowance = myCwnd - path->outstandingBytes - bytes.bytesToSend;
                if (allowance > 0) {
                    if (qCounter.bookedSumSendStreams > (uint32)allowance) {
                        bytes.bytesToSend = myCwnd - path->outstandingBytes;
                        EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): bytesToSend are limited by cwnd: "
                                  << bytes.bytesToSend << endl;
                    }
                    else {
                        bytes.bytesToSend += qCounter.bookedSumSendStreams;
                        EV_DETAIL << "bytesAllowedToSend(" << path->remoteAddress << "): send all stored bytes: "
                                  << bytes.bytesToSend << endl;
                    }
                }
            }
        }
    }

    EV_INFO << "bytesAllowedToSend(" << path->remoteAddress << "):"
            << " osb=" << path->outstandingBytes
            << " cwnd=" << path->cwnd
            << " tempCwnd=" << path->tempCwnd
            << " bytes.packet=" << (bytes.packet ? "YES!" : "no")
            << " bytes.chunk=" << (bytes.chunk ? "YES!" : "no")
            << " bytes.bytesToSend=" << bytes.bytesToSend << endl;
}

void SCTPAssociation::sendOnPath(SCTPPathVariables *pathId, bool firstPass)
{
    // ====== Variables ======================================================
    SCTPPathVariables *path = NULL;    // Path to send next message to
    SCTPMessage *sctpMsg = NULL;
    SCTPSackChunk *sackChunk = NULL;
    SCTPDataChunk *chunkPtr = NULL;
    SCTPForwardTsnChunk *forwardChunk = NULL;

    uint16 chunksAdded = 0;
    uint16 dataChunksAdded = 0;
    uint32 totalChunksSent = 0;
    uint32 totalPacketsSent = 0;
    uint32 outstandingBytes = 0;

    uint32 tcount = 0;    // Bytes in transmission queue on the selected path
    uint32 Tcount = 0;    // Bytes in transmission queue on all paths
    uint32 scount = 0;    // Bytes in send streams
    int32 bytesToSend = 0;

    bool headerCreated = false;
    bool sendOneMorePacket = false;
    bool sendingAllowed = true;
    bool authAdded = false;
    bool sackAdded = false;
    bool forwardPresent = false;

    // ====== Perform Chunk Rescheduling =====================================
    if ((state->allowCMT) &&
        (state->cmtChunkReschedulingVariant != SCTPStateVariables::CCRV_None))
    {
        chunkReschedulingControl((pathId == NULL) ? state->getPrimaryPath() : pathId);
    }

    // ====== Obtain path ====================================================
    EV_INFO << endl << "##### sendAll(";
    if (pathId) {
        EV_INFO << pathId->remoteAddress;
    }
    EV_INFO << ") at t=" << simTime() << " #####" << endl;
    while (sendingAllowed) {
        headerCreated = false;
        if (state->bytesToRetransmit > 0) {
            // There are bytes in the transmissionQ. They have to be sent first.
            path = choosePathForRetransmission();
            assert(path != NULL);
        }
        else {
            if (pathId == NULL) {    // No path given => use primary path.
                path = state->getPrimaryPath();
            }
            else {
                path = pathId;
            }
        }
        if ((state->maxBurstVariant == SCTPStateVariables::MBV_MaxBurst) ||
            (state->maxBurstVariant == SCTPStateVariables::MBV_AggressiveMaxBurst))
        {
            if (state->lastTransmission < simTime()) {
                state->packetsInTotalBurst = 0;
            }
            if (path->lastTransmission < simTime()) {
                // T.D. 18.07.2011: Only reset packetsInBurst once per time-stamp!
                path->packetsInBurst = 0;
            }
        }
        // packetsInBurst must be checked here!
        // sendOnPath() may be called multiple times at the same simTime!
        if (((state->maxBurstVariant == SCTPStateVariables::MBV_MaxBurst) ||
             (state->maxBurstVariant == SCTPStateVariables::MBV_AggressiveMaxBurst) ||
             (state->maxBurstVariant == SCTPStateVariables::MBV_TotalMaxBurst)) &&
            (path->packetsInBurst >= state->maxBurst))
        {
            break;
        }
        // TotalMaxBurst variant: limit bursts on all paths.
        if ((state->maxBurstVariant == SCTPStateVariables::MBV_TotalMaxBurst) &&
            (state->packetsInTotalBurst >= state->maxBurst))
        {
            break;
        }
        outstandingBytes = path->outstandingBytes;
        assert((int32)outstandingBytes >= 0);
        CounterMap::iterator tq = qCounter.roomTransQ.find(path->remoteAddress);
        tcount = tq->second;
        Tcount = getAllTransQ();
        scount = qCounter.roomSumSendStreams;    // includes header and padding
        EV_INFO << "\nsendAll: on " << path->remoteAddress << ":"
                << " tcount=" << tcount
                << " Tcount=" << Tcount
                << " scount=" << scount
                << " nextTSN=" << state->nextTSN << endl;

        bool sackOnly;
        bool sackWithData;
        timeForSack(sackOnly, sackWithData);
        if ((tcount == 0 && scount == 0) || (!state->allowCMT && tcount == 0 && Tcount > 0)) {
            // ====== No DATA chunks to send ===================================
            EV_DETAIL << "No DATA chunk available!" << endl;
            if (!sackOnly) {    // SACK?, no data to send
                EV_DETAIL << "No SACK to send either" << endl;
                return;
            }
            else {
                bytes.bytesToSend = 0;
            }
        }
        else {
            bytesAllowedToSend(path, firstPass);
        }
        bytesToSend = bytes.bytesToSend;

        // As there is at least a SACK to be sent, a header can be created

        if (state->sctpMsg) {
            EV_DETAIL << "packet was stored -> load packet" << endl;
            loadPacket(path, &sctpMsg, &chunksAdded, &dataChunksAdded, &authAdded);
            headerCreated = true;
        }
        else if (bytesToSend > 0 || bytes.chunk || bytes.packet || sackWithData || sackOnly || forwardPresent) {
            sctpMsg = new SCTPMessage("send");
            sctpMsg->setByteLength(SCTP_COMMON_HEADER);
            headerCreated = true;
            chunksAdded = 0;
        }

        if (sackWithData || sackOnly) {
            // SACK can be sent
            assert(headerCreated == true);
            sackChunk = createSack();
            // CMT DAC
            if ((state->allowCMT == true) && (state->cmtUseDAC == true)) {
                EV << "Adding dacPacketsRcvd=" << (unsigned int)dacPacketsRcvd << " to SACK" << endl;
                sackChunk->setDacPacketsRcvd(dacPacketsRcvd);
            }
            dacPacketsRcvd = 0;

            chunksAdded++;
            totalChunksSent++;
            // ------ Create AUTH chunk, if necessary --------------------------
            authAdded = addAuthChunkIfNecessary(sctpMsg, SACK, authAdded);

            // ------ Add SACK chunk -------------------------------------------
            sctpMsg->addChunk(sackChunk);
            sackAdded = true;
            EV_DETAIL << assocId << ": SACK added, chunksAdded now " << chunksAdded << " sackOnly=" << sackOnly << " sackWithData=" << sackWithData << "\n";
            if (sackOnly && !(bytesToSend > 0 || bytes.chunk || bytes.packet)) {
                // There is no data to be sent, just the SACK
                path->lastTransmission = simTime();
                path->packetsInBurst++;
                state->lastTransmission = simTime();
                state->packetsInTotalBurst++;
                if (dataChunksAdded > 0) {
                    state->ssNextStream = true;
                }
                state->ackState = 0;
                // Stop SACK timer if it is running...
                stopTimer(SackTimer);
                sctpAlgorithm->sackSent();
                state->sackAllowed = false;
                sendSACKviaSelectedPath(sctpMsg);
                return;
            }
        }
        // ====== FORWARD_TSN =================================================
        if (!forwardPresent && !state->stopSending) {
            if (peekAbandonedChunk(path) != NULL) {
                forwardChunk = createForwardTsnChunk(path->remoteAddress);
                chunksAdded++;
                totalChunksSent++;
                state->ackPointAdvanced = false;
                if (!headerCreated) {
                    sctpMsg = new SCTPMessage("send");
                    sctpMsg->setByteLength(SCTP_COMMON_HEADER);
                    headerCreated = true;
                    chunksAdded = 0;
                }
                // ------ Create AUTH chunk, if necessary -----------------------
                authAdded = addAuthChunkIfNecessary(sctpMsg, FORWARD_TSN, authAdded);
                sctpMsg->addChunk(forwardChunk);
                forwardPresent = true;
                if (!path->T3_RtxTimer->isScheduled()) {
                    // Start retransmission timer, if not scheduled before
                    startTimer(path->T3_RtxTimer, path->pathRto);
                }
                if (bytesToSend == 0) {
                    sendToIP(sctpMsg, path->remoteAddress);
                    forwardPresent = false;
                    headerCreated = false;
                    chunksAdded = 0;
                    sctpMsg = NULL;
                }
            }
        }

        // ####################################################################
        // #### Data Transmission                                          ####
        // ####################################################################

        bool packetFull = false;

        while (!packetFull && headerCreated) {
            assert(headerCreated == true);
            EV_DETAIL << assocId << ": bytesToSend=" << bytesToSend
                      << " bytes.chunk=" << bytes.chunk
                      << " bytes.packet=" << bytes.packet << endl;

            // ====== How many bytes may be transmitted in next packet? ========
            int32 allowance = path->pmtu;    // Default behaviour: send 1 path MTU
            // Restrict amount of data to send to cwnd size.
            if ((state->strictCwndBooking) &&
                (sctpPathMap.size() > 1))    // strict behaviour only for more than 1 paths!
            {    // T.D. 19.01.2010: bytesToSend may be less than 1 MTU on this path.
                 // Allow overbooking in second pass, if *total* cwnd allows it.
                if (!firstPass) {
                    // No "one packet"/"one chunk" in second pass!
                    bytes.packet = false;
                    bytes.chunk = false;
                }

                if (bytes.chunk) {
                    // Send 1 chunk: allow one path MTU.
                }
                else if (bytes.packet) {
                    // Send 1 chunk: allow one path MTU.
                }
                else if ((path->outstandingBytes == 0) && (firstPass)) {
                    // No outstanding data and first pass: allow one path MTU.
                }
                else {    // There *may* be something more to send ...
                    if ((int32)path->cwnd - (int32)path->outstandingBytes >= (int32)path->pmtu) {
                        // Enough space -> allow one path MTU
                    }
                    else {
                        if (firstPass) {
                            // In first pass, disallow overbooking.
                            allowance = 0;
                            bytesToSend = 0;
                        }
                        else {
                            if (!state->allowCMT) {
                                // For non-CMT in second pass, allow 1 more MTU.
                            }
                            else {
                                // Not CMT in second pass, check total space ...
                                int32 totalOutstanding = 0;
                                int32 totalCwnd = 0;
                                for (SCTPPathMap::const_iterator pathMapIterator = sctpPathMap.begin();
                                     pathMapIterator != sctpPathMap.end(); pathMapIterator++)
                                {
                                    const SCTPPathVariables *myPath = pathMapIterator->second;
                                    totalOutstanding += myPath->outstandingBytes;
                                    totalCwnd += myPath->cwnd;
                                }
                                if ((int32)(totalCwnd - totalOutstanding) < (int32)(path->pmtu)) {
                                    // ... and disallow overbooking if there is no more space for 1 MTU
                                    allowance = 0;
                                    bytesToSend = 0;
                                }
                                else {
                                    // ... and allow 1 MTU if there is still space
                                }
                            }
                        }
                    }
                }
            }
            else {
                if ((bytesToSend > 0) || (bytes.chunk) || (bytes.packet)) {
                    // Allow 1 more MTU
                }
                else {
                    // No more sending allowed.
                    allowance = 0;
                    bytesToSend = 0;
                }
            }
            if ((allowance > 0) || (bytes.chunk) || (bytes.packet)) {
                bool firstTime = false;    // Is DATA chunk send for the first time?
                SCTPDataVariables *datVar = NULL;
                // ------ Create AUTH chunk, if necessary -----------------------
                authAdded = addAuthChunkIfNecessary(sctpMsg, DATA, authAdded);
                if (tcount > 0) {
                    // ====== Retransmission ========================================
                    // If bytes.packet is true, one packet is allowed to be retransmitted!
                    datVar = getOutboundDataChunk(path,
                                path->pmtu - sctpMsg->getByteLength() - 20,
                                (bytes.packet == true) ? path->pmtu : allowance);
                    if (datVar == NULL) {
                        if (chunksAdded == 1 && sackAdded) {
                            datVar = getOutboundDataChunk(path,
                                        path->pmtu - sctpMsg->getByteLength() + sackChunk->getByteLength() - 20,
                                        (bytes.packet == true) ? path->pmtu : allowance);
                            if (!sackOnly) {
                                sctpMsg->removeChunk();
                                EV_DETAIL << "RTX: Remove SACK chunk\n";
                                delete sackChunk;
                                chunksAdded--;
                                sackAdded = false;
                            }
                            else {
                                path->lastTransmission = simTime();
                                path->packetsInBurst++;
                                state->lastTransmission = simTime();
                                state->packetsInTotalBurst++;
                                if (dataChunksAdded > 0) {
                                    state->ssNextStream = true;
                                }
                                state->ackState = 0;
                                // Stop SACK timer if it is running...
                                stopTimer(SackTimer);
                                sctpAlgorithm->sackSent();
                                state->sackAllowed = false;
                                EV_DETAIL << "RTX: send only SACK\n";
                                sendSACKviaSelectedPath(sctpMsg);
                                if (datVar != NULL) {
                                    sctpMsg = new SCTPMessage("send");
                                    sctpMsg->setByteLength(SCTP_COMMON_HEADER);
                                    headerCreated = true;
                                    sackAdded = false;
                                    chunksAdded = 0;
                                }
                            }
                        }
                    }

                    // Check for FORWARD-TSN again, might just have been triggered...
                    if (datVar == NULL && !forwardPresent && !state->stopSending) {
                        if (peekAbandonedChunk(path) != NULL) {
                            forwardChunk = createForwardTsnChunk(path->remoteAddress);
                            chunksAdded++;
                            totalChunksSent++;
                            state->ackPointAdvanced = false;
                            // ------ Create AUTH chunk, if necessary -----------------------
                            authAdded = addAuthChunkIfNecessary(sctpMsg, FORWARD_TSN, authAdded);
                            sctpMsg->addChunk(forwardChunk);
                            forwardPresent = true;
                            if (!path->T3_RtxTimer->isScheduled()) {
                                // Start retransmission timer, if not scheduled before
                                startTimer(path->T3_RtxTimer, path->pathRto);
                            }
                        }
                    }

                    if (datVar != NULL) {
                        assert(datVar->getNextDestinationPath() == path);
                        datVar->numberOfRetransmissions++;
                        if (chunkHasBeenAcked(datVar) == false) {
                            EV_DETAIL << simTime() << ": Retransmission #" << datVar->numberOfRetransmissions
                                      << " of TSN " << datVar->tsn
                                      << " on path " << datVar->getNextDestination()
                                      << " (last was " << datVar->getLastDestination() << ")" << endl;
                            // The chunk is going to be retransmitted on another path.
                            // On the original path, it is necessary to find another
                            // PseudoCumAck!
                            if (datVar->getLastDestinationPath() != datVar->getNextDestinationPath()) {
                                SCTPPathVariables *oldPath = datVar->getLastDestinationPath();
                                oldPath->findPseudoCumAck = true;
                                oldPath->findRTXPseudoCumAck = true;
                                SCTPPathVariables *newPath = datVar->getNextDestinationPath();
                                newPath->findPseudoCumAck = true;
                                newPath->findRTXPseudoCumAck = true;
                            }
                            datVar->wasDropped = false;
                            datVar->countsAsOutstanding = true;
                            datVar->hasBeenReneged = false;
                            increaseOutstandingBytes(datVar, path);    // NOTE: path == datVar->getNextDestinationPath()
                        }
                    }
                }
                // ====== First Transmission ====================================
                else if (((scount > 0) && (!state->nagleEnabled)) ||    // Data to send and Nagle off
                         ((uint32)scount >= path->pmtu - 32 - 20) ||    // Data to fill at least one path MTU
                         ((scount > 0) && (state->nagleEnabled) && ((outstandingBytes == 0) || (sackOnly && sackAdded))))    // Data to send, Nagle on and no outstanding bytes
                {    // ====== Buffer Splitting ===================================
                    bool rejected = false;
                    const uint32 bytesOnPath = (state->cmtBufferSplittingUsesOSB == true) ?
                        path->outstandingBytes : path->queuedBytes;
                    if (state->allowCMT) {
                        // ------ Sender Side -------------------------------------
                        if ((state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_SenderOnly) ||
                            (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_BothSides))
                        {
                            // Limit is 1/n of current sender-side buffer allocation
                            const uint32 limit = ((state->sendQueueLimit != 0) ? state->sendQueueLimit : 0xffffffff) / sctpPathMap.size();
                            if (bytesOnPath + path->pmtu > limit) {
                                rejected = true;
                                EV << simTime() << ":\tSenderBufferSplitting: Rejecting transmission on "
                                   << path->remoteAddress << ", since "
                                   << bytesOnPath << " + " << path->pmtu << " > "
                                   << state->sendQueueLimit / sctpPathMap.size() << endl;
                            }
                        }

                        // ------ Receiver Side -----------------------------------
                        if ((rejected == false) &&
                            ((state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_ReceiverOnly) ||
                             (state->cmtBufferSplitVariant == SCTPStateVariables::CBSV_BothSides)))
                        {
                            // Limit is 1/n of current receiver-side buffer allocation
                            const uint32 limit = (state->peerRwnd + state->outstandingBytes)
                                / sctpPathMap.size();
                            if (bytesOnPath + path->pmtu > limit + path->pmtu) {
                                // T.D. 09.07.2011: Allow overbooking by up to 1 MTU ...
                                EV << simTime() << ":\tReceiverBufferSplitting: Rejecting transmission on "
                                   << path->remoteAddress << ", since "
                                   << bytesOnPath + path->pmtu << " > " << limit << endl;
                                rejected = true;
                            }
                        }
                    }

                    // ====== Buffer Splitting ===================================
                    if (((state->allowCMT) || (path == state->getPrimaryPath())) &&
                        (!rejected))
                    {
                        // ------ Dequeue data message ----------------------------
                        EV_DETAIL << assocId << "sendAll:     sctpMsg->length=" << sctpMsg->getByteLength()
                                  << " length datMsg=" << path->pmtu - sctpMsg->getByteLength() - 20 << endl;
                        SCTPDataMsg *datMsg = dequeueOutboundDataMsg(path, path->pmtu - sctpMsg->getByteLength() - 20,
                                    allowance);
                        if (datMsg == NULL) {
                            if (chunksAdded == 1 && sackAdded) {
                                datMsg = dequeueOutboundDataMsg(path, path->pmtu - sctpMsg->getByteLength() + sackChunk->getByteLength() - 20,
                                            allowance);
                                if (!sackOnly) {
                                    sctpMsg->removeChunk();
                                    EV_DETAIL << assocId << ": delete SACK chunk to make room for datMsg (" << &datMsg << "). scount=" << scount << "\n";
                                    delete sackChunk;
                                    chunksAdded--;
                                    sackAdded = false;
                                }
                                else {
                                    path->lastTransmission = simTime();
                                    path->packetsInBurst++;
                                    state->lastTransmission = simTime();
                                    state->packetsInTotalBurst++;
                                    if (dataChunksAdded > 0) {
                                        state->ssNextStream = true;
                                    }
                                    state->ackState = 0;
                                    // Stop SACK timer if it is running...
                                    stopTimer(SackTimer);
                                    sctpAlgorithm->sackSent();
                                    state->sackAllowed = false;
                                    EV_DETAIL << assocId << ": send SACK and make new header for datMsg (" << &datMsg << "). scount=" << scount << "\n";
                                    sendSACKviaSelectedPath(sctpMsg);
                                    if (datMsg != NULL) {
                                        sctpMsg = new SCTPMessage("send");
                                        sctpMsg->setByteLength(SCTP_COMMON_HEADER);
                                        headerCreated = true;
                                        sackAdded = false;
                                        chunksAdded = 0;
                                    }
                                }
                            }
                        }
                        // ------ Handle data message -----------------------------
                        if (datMsg) {
                            firstTime = true;

                            state->queuedMessages--;
                            if ((state->queueLimit > 0) &&
                                (state->queuedMessages < state->queueLimit) &&
                                (state->queueUpdate == false))
                            {
                                // Tell upper layer readiness to accept more data
                                sendIndicationToApp(SCTP_I_SEND_MSG);
                                state->queueUpdate = true;
                            }

                            datVar = makeDataVarFromDataMsg(datMsg, path);
                            delete datMsg;

                            EV_DETAIL << assocId << ":: sendAll: chunk " << datVar << " dequeued from StreamQ "
                                      << datVar->sid << ": tsn=" << datVar->tsn
                                      << ", bytes now " << qCounter.roomSumSendStreams << "\n";
                        }
                        // ------ No data message has been dequeued ---------------
                        else {
                            EV_DETAIL << assocId << ": No data message has been dequeued" << endl;
                            // ------ Are there any chunks to send? ----------------
                            if (chunksAdded == 0) {
                                // No -> nothing more to do.
                                if (state->sctpMsg == sctpMsg) {
                                    state->sctpMsg = NULL;
                                    state->packetBytes = 0;
                                }
                                packetFull = true;    // chunksAdded==0, packetFull==true => leave inner while loop
                            }
                            else {
                                // Yes.
                                if (state->nagleEnabled && ((outstandingBytes > 0) && !(sackOnly && sackAdded)) &&
                                    nextChunkFitsIntoPacket(path, path->pmtu - sctpMsg->getByteLength() - 20) &&
                                    (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                                {
                                    EV_DETAIL << "Nagle: Packet has to be stored\n";
                                    storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, authAdded);
                                    sctpMsg = NULL;
                                    chunksAdded = 0;
                                }
                                packetFull = true;    // chunksAdded==0, packetFull==true => leave inner while loop
                                EV_DETAIL << "sendAll: packetFull: msg length = " << sctpMsg->getByteLength() + 20 << "\n";
                            }
                        }
                    }
                    else if (chunksAdded == 1 && sackAdded && !sackOnly) {
                        sctpMsg->removeChunk();
                        EV_DETAIL << "Nagle or no data: Remove SACK chunk, delete sctpmsg" << endl;
                        delete sackChunk;
                        packetFull = true;
                        sackAdded = false;
                        chunksAdded--;
                    }
                    else if ((chunksAdded == 1 && sackAdded && sackOnly) || headerCreated) {
                        packetFull = true;
                    }
                }
                else if (chunksAdded == 1 && sackAdded && !sackOnly) {
                    sctpMsg->removeChunk();
                    EV_DETAIL << "Nagle or no data: Remove SACK chunk, delete sctpmsg\n";
                    delete sackChunk;
                    packetFull = true;
                    sackAdded = false;
                    chunksAdded--;
                }
                else if (chunksAdded == 1 && sackAdded && sackOnly) {
                    packetFull = true;
                }
                else if (datVar == NULL || chunksAdded == 0) {
                    EV_DETAIL << "HeaderCreated=" << headerCreated << ", chunksAdded=" << chunksAdded << " datVar=" << datVar << "\n";
                    if (headerCreated) {
                        packetFull = true;
                    }
                }

                // ------ Handle DATA chunk -------------------------------------
                if (datVar != NULL && !packetFull) {
                    // ------ Assign TSN -----------------------------------------
                    if (firstTime) {
                        assert(datVar->tsn == 0);
                        datVar->tsn = state->nextTSN;
                        EV_DETAIL << "sendAll: set TSN=" << datVar->tsn
                                  << " sid=" << datVar->sid << ", ssn=" << datVar->ssn << "\n";
                        state->nextTSN++;
                        path->vectorPathSentTSN->record(datVar->tsn);
                    }
                    else {
                        if (datVar->hasBeenFastRetransmitted) {
                            path->vectorPathTSNFastRTX->record(datVar->tsn);
                        }
                        else {
                            path->vectorPathTSNTimerBased->record(datVar->tsn);
                        }
                    }

                    SCTP::AssocStatMap::iterator iterator = sctpMain->assocStatMap.find(assocId);
                    iterator->second.transmittedBytes += datVar->len / 8;

                    datVar->setLastDestination(path);
                    datVar->countsAsOutstanding = true;
                    datVar->hasBeenReneged = false;
                    datVar->sendTime = simTime();    //I.R. to send Fast RTX just once a RTT
                    if (datVar->firstSendTime == 0) {
                        datVar->firstSendTime = simTime();
                    }

                    // ------ First transmission of datVar -----------------------
                    if (datVar->numberOfTransmissions == 0) {
                        EV_DETAIL << "sendAll: " << simTime() << " firstTime, TSN "
                                  << datVar->tsn << ": lastDestination set to "
                                  << datVar->getLastDestination() << endl;

                        if (!state->firstDataSent) {
                            state->firstDataSent = true;
                        }
                        EV_DETAIL << "sendAll: insert in retransmissionQ tsn=" << datVar->tsn << "\n";
                        if (!retransmissionQ->checkAndInsertChunk(datVar->tsn, datVar)) {
                            throw cRuntimeError("Cannot add datVar to retransmissionQ!");
                            // Falls es hier aufschlaegt, muss ueberlegt werden, ob es OK ist, dass datVars nicht eingefuegt werden koennen.
                        }
                        else {
                            EV_DETAIL << "sendAll: size of retransmissionQ=" << retransmissionQ->getQueueSize() << "\n";
                            unackChunk(datVar);
                            increaseOutstandingBytes(datVar, path);
                            datVar->queuedOnPath = path;
                            datVar->queuedOnPath->queuedBytes += datVar->booksize;
                            datVar->queuedOnPath->statisticsPathQueuedSentBytes->record(path->queuedBytes);

                            state->queuedSentBytes += datVar->booksize;
                            statisticsQueuedSentBytes->record(state->queuedSentBytes);
                        }
                    }

                    /* datVar is already in the retransmissionQ */
                    datVar->numberOfTransmissions++;
                    datVar->gapReports = 0;
                    datVar->hasBeenFastRetransmitted = false;
                    EV_DETAIL << "sendAll(): adding new outbound data datVar to packet (tsn=" << datVar->tsn << ")...!!!\n";

                    chunkPtr = transformDataChunk(datVar);

                    /* update counters */
                    totalChunksSent++;
                    chunksAdded++;
                    dataChunksAdded++;
                    EV_DETAIL << assocId << ": DataChunk with TSN=" << chunkPtr->getTsn() << " and length " << chunkPtr->getByteLength() << " added\n";
                    sctpMsg->addChunk(chunkPtr);
                    if (datVar->numberOfTransmissions > 1) {
                        CounterMap::iterator tq = qCounter.roomTransQ.find(path->remoteAddress);
                        if (tq->second > 0) {
                            if (transmissionQ->getSizeOfFirstChunk(path->remoteAddress) > path->pmtu - sctpMsg->getByteLength() - 20)
                                packetFull = true;
                        }
                        else if (nextChunkFitsIntoPacket(path, path->pmtu - sctpMsg->getByteLength() - 20) == false) {
                            packetFull = true;
                        }
                    }
                    else {
                        if (nextChunkFitsIntoPacket(path, path->pmtu - sctpMsg->getByteLength() - 20) == false) {
                            packetFull = true;
                        }
                    }

                    state->peerRwnd -= (datVar->booksize + state->bytesToAddPerPeerChunk);
                    if (state->peerAllowsChunks) {
                        state->peerMsgRwnd--;
                    }
                    if ((bytes.chunk == false) && (bytes.packet == false)) {
                        bytesToSend -= datVar->booksize;
                    }
                    else if (bytes.chunk) {
                        bytes.chunk = false;
                    }
                    else if ((bytes.packet) && (packetFull)) {
                        bytes.packet = false;
                    }

                    if (bytesToSend <= 0) {
                        if ((!packetFull) && (qCounter.roomSumSendStreams > path->pmtu - 32 - 20 || tcount > 0)) {
                            sendOneMorePacket = true;
                            bytes.packet = true;
                            EV_DETAIL << assocId << ": sendAll: one more packet allowed\n";
                        }
                        else {
                            if (state->nagleEnabled && (outstandingBytes > 0) &&
                                nextChunkFitsIntoPacket(path, path->pmtu - sctpMsg->getByteLength() - 20) &&
                                (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                            {
                                storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, authAdded);
                                sctpMsg = NULL;
                                chunksAdded = 0;
                                packetFull = true;    // chunksAdded==0, packetFull==true => leave inner while loop
                            }
                            else {
                                packetFull = true;
                            }
                        }
                        bytesToSend = 0;
                    }
                    else if ((qCounter.roomSumSendStreams == 0) && (tq->second == 0)) {
                        packetFull = true;
                        EV_DETAIL << "sendAll: no data in send and transQ: packet full\n";
                    }
                    EV_DETAIL << "sendAll: bytesToSend after reduction: " << bytesToSend << "\n";
                }    // end if (datVar != NULL && !packetFull)
                     // ------ There is no DATA chunk, only control chunks possible --
                else {
                    if (chunksAdded == 0) {    // Nothing to do -> return
                        packetFull = true;    // chunksAdded==0, packetFull==true => leave inner while loop
                    }
                    else {
                        packetFull = true;
                        EV_DETAIL << assocId << ": sendAll: packetFull: msg length = " << sctpMsg->getByteLength() + 20 << "\n";
                        datVar = NULL;
                    }
                }

                // ====== Send packet ===========================================
                if (packetFull) {
                    if (chunksAdded == 0) {    // Nothing to send
                        delete sctpMsg;
                        sendingAllowed = false;    // sendingAllowed==false => leave outer while loop
                    }
                    else {
                        EV_DETAIL << assocId << ":: sendAll: " << simTime() << "    packet full:"
                                  << "    totalLength=" << sctpMsg->getBitLength() / 8 + 20
                                  << ",    path=" << path->remoteAddress
                                  << "     " << dataChunksAdded << " chunks added, outstandingBytes now "
                                  << path->outstandingBytes << "\n";

                        /* new chunks would exceed MTU, so we send old packet and build a new one */
                        /* this implies that at least one data chunk is send here */
                        if (dataChunksAdded > 0) {
                            if (!path->T3_RtxTimer->isScheduled()) {
                                // Start retransmission timer, if not scheduled before
                                startTimer(path->T3_RtxTimer, path->pathRto);
                            }
                            else {
                                EV_DETAIL << "sendAll: RTX Timer already scheduled -> no need to schedule it\n";
                            }
                        }
                        if (sendOneMorePacket) {
                            sendOneMorePacket = false;
                            bytesToSend = 0;
                            bytes.packet = false;
                            chunkPtr->setIBit(sctpMain->sackNow);
                        }
                        // Set I-bit when this is the final packet for this path!
                        if (state->strictCwndBooking) {
                            const int32 a = (int32)path->cwnd - (int32)path->outstandingBytes;
                            if (((a > 0) && (nextChunkFitsIntoPacket(path, a) == false)) || (!firstPass)) {
                                chunkPtr->setIBit(sctpMain->sackNow);
                            }
                        }
                        if (dataChunksAdded > 0) {
                            state->ssNextStream = true;
                        }
                        EV_DETAIL << assocId << ":sendToIP: packet size=" << sctpMsg->getByteLength() << " numChunks=" << sctpMsg->getChunksArraySize() << "\n";
                        sendToIP(sctpMsg, path->remoteAddress);
                        pmDataIsSentOn(path);
                        totalPacketsSent++;
                        path->lastTransmission = simTime();
                        path->packetsInBurst++;
                        state->lastTransmission = simTime();
                        state->packetsInTotalBurst++;

                        // ------ Reset status ------------------------------------
                        firstTime = false;
                        headerCreated = false;
                        chunksAdded = 0;
                        dataChunksAdded = 0;
                        authAdded = false;

                        EV_INFO << "sendAll: sending Packet to path " << path->remoteAddress
                                << "  scount=" << scount
                                << "  tcount=" << tcount
                                << "  bytesToSend=" << bytesToSend << endl;
                    }
                }
                EV_INFO << "sendAll: still " << bytesToSend
                        << " bytes to send, headerCreated=" << headerCreated << endl;
            }    // if (bytesToSend > 0 || bytes.chunk || bytes.packet)
            else {
                packetFull = true;    // Leave inner while loop
                delete sctpMsg;    // T.D. 19.01.2010: Free unsent message
            }

            EV_INFO << "packetFull=" << packetFull << endl;
        }    // while(!packetFull)

        EV_INFO << "bytesToSend=" << bytesToSend
                << " bytes.chunk=" << bytes.chunk
                << " bytes.packet=" << bytes.packet << endl;
        if (!(bytesToSend > 0 || bytes.chunk || bytes.packet)) {
            sendingAllowed = false;
        }
    }    // while(sendingAllowed)

    EV_INFO << "sendAll: nothing more to send... BYE!\n";
}

uint32 SCTPAssociation::getAllTransQ()
{
    uint32 sum = 0;
    for (CounterMap::iterator tq = qCounter.roomTransQ.begin(); tq != qCounter.roomTransQ.end(); tq++) {
        sum += tq->second;
    }
    return sum;
}

} // namespace sctp

} // namespace inet

