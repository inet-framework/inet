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

#include "SCTPAssociation.h"
#include "SCTPAlgorithm.h"
#include "SCTPCommand_m.h"

#include <assert.h>
#include <algorithm>


void SCTPAssociation::increaseOutstandingBytes(SCTPDataVariables* chunk,
                                                              SCTPPathVariables* path)
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

void SCTPAssociation::storePacket(SCTPPathVariables* pathVar,
                                             SCTPMessage*         sctpMsg,
                                             const uint16         chunksAdded,
                                             const uint16         dataChunksAdded,
                                             const bool           authAdded)
{
    uint32 packetBytes = 0;
    for (uint16 i = 0; i < sctpMsg->getChunksArraySize(); i++) {
        SCTPDataVariables* chunk = retransmissionQ->payloadQueue.find(((SCTPDataChunk*)sctpMsg->getChunks(i))->getTsn())->second;
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
    sctpEV3 << "storePacket: path=" << pathVar->remoteAddress
              << " state->packetBytes=" << state->packetBytes
              << " osb=" << pathVar->outstandingBytes << " -> "
              << pathVar->outstandingBytes - state->packetBytes << endl;
    if (state->osbWithHeader)
        qCounter.roomSumSendStreams += state->packetBytes;
    else
        qCounter.roomSumSendStreams += state->packetBytes + (dataChunksAdded * SCTP_DATA_CHUNK_LENGTH);
    qCounter.bookedSumSendStreams += state->packetBytes;

}

void SCTPAssociation::loadPacket(SCTPPathVariables* pathVar,
                                            SCTPMessage**        sctpMsg,
                                            uint16*              chunksAdded,
                                            uint16*              dataChunksAdded,
                                            bool*                    authAdded)
{
    *sctpMsg = state->sctpMsg;
    state->sctpMsg = NULL;
    *chunksAdded = state->chunksAdded;
    *dataChunksAdded = state->dataChunksAdded;
    *authAdded = state->authAdded;
    sctpEV3 << "loadPacket: path=" << pathVar->remoteAddress << " osb=" << pathVar->outstandingBytes << " -> " << pathVar->outstandingBytes + state->packetBytes << endl;
    if (state->osbWithHeader) {
        qCounter.roomSumSendStreams -= state->packetBytes;
    }
    else {
        qCounter.roomSumSendStreams -= state->packetBytes + ((*dataChunksAdded) * SCTP_DATA_CHUNK_LENGTH);
    }
    qCounter.bookedSumSendStreams -= state->packetBytes;

    for (uint16 i = 0; i < (*sctpMsg)->getChunksArraySize(); i++) {
        SCTPDataVariables* chunk = retransmissionQ->payloadQueue.find(((SCTPDataChunk*)(*sctpMsg)->getChunks(i))->getTsn())->second;
        chunk->queuedOnPath = pathVar;
        chunk->queuedOnPath->queuedBytes += chunk->booksize;
        chunk->setLastDestination(pathVar);
        increaseOutstandingBytes(chunk, pathVar);
        chunk->countsAsOutstanding = true;
    }

}



SCTPDataVariables* SCTPAssociation::makeDataVarFromDataMsg(SCTPDataMsg*         datMsg,
                                                                              SCTPPathVariables* path)
{
    SCTPDataVariables* datVar = new SCTPDataVariables();

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
    SCTPSendStream*             stream = iterator->second;
    uint32                      nextSSN = stream->getNextStreamSeqNum();
    datVar->userData = datMsg->decapsulate();
    if (datMsg->getOrdered()) {
        // ------ Ordered mode: assign SSN ---------
        if (datMsg->getEBit())
        {
            datVar->ssn = nextSSN++;
        }
        else
        {
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

    return (datVar);
}

SCTPPathVariables* SCTPAssociation::choosePathForRetransmission()
{
    uint32             max = 0;
    SCTPPathVariables* temp = NULL;

    for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        SCTPPathVariables*          path = iterator->second;
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
    if ( ((state->gapList.getNumGaps(SCTPGapList::GT_Any) > 0) || (state->dupList.size() > 0)) &&
            (state->sackAllowed) ) {
        // Schedule sending of SACKs at once, when we have fragments to report
        state->ackState = sackFrequency;
        sackOnly = sackWithData = true;  // SACK necessary, regardless of data available
    }
    if (state->ackState >= sackFrequency) {
        sackOnly = sackWithData = true;  // SACK necessary, regardless of data available
    }
    else if (SackTimer->isScheduled()) {
        sackOnly = false;
        sackWithData = true;      // Only send SACK when data is present.
    }
}


void SCTPAssociation::sendOnAllPaths(SCTPPathVariables* firstPath)
{
        // ------ Send on provided path first ... -----------------------------
        if (firstPath != NULL) {
            sendOnPath(firstPath);
        }

        // ------ ... then, try sending on all other paths --------------------
        for (SCTPPathMap::iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
            SCTPPathVariables* path = iterator->second;
            if (path != firstPath) {
                sendOnPath(path);
            }
        }

}

void SCTPAssociation::sendSACKviaSelectedPath(SCTPMessage* sctpMsg)
{
    SCTPPathVariables* sackPath =
            (state->lastDataSourceList.size() > 0) ? state->lastDataSourceList.front() :
                                                     state->lastDataSourcePath;
    assert(sackPath != NULL);

    sctpEV3 << assocId << ": sending SACK to " << sackPath->remoteAddress << endl;
    sendToIP(sctpMsg, sackPath->remoteAddress);
    sackPath->lastSACKSent = simTime();
    state->lastDataSourceList.clear();   // Clear the address list!
}


void SCTPAssociation::bytesAllowedToSend(SCTPPathVariables* path,
                                                      const bool            firstPass)
{
    assert(path != NULL);

    bytes.chunk = false;
    bytes.packet = false;
    bytes.bytesToSend = 0;

    sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "):"
              << " osb=" << path->outstandingBytes << " cwnd=" << path->cwnd << endl;

    // ====== First transmission =============================================
    if (!state->firstDataSent) {
        bytes.chunk = true;
    }

    // ====== Transmission allowed by peer's receiver window? ================
    else if ((state->peerWindowFull || (state->peerAllowsChunks && state->peerMsgRwnd <= 0)) && (path->outstandingBytes == 0)) {
            // Zero Window Probing
            sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): zeroWindowProbing" << endl;
            state->zeroWindowProbing = true;
            bytes.chunk = true;
    }

    // ====== Retransmissions ================================================
    else {
        CounterMap::const_iterator it = qCounter.roomTransQ.find(path->remoteAddress);
        sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): bytes in transQ=" << it->second << endl;
        if (it->second > 0) {
            const int32 allowance = path->cwnd - path->outstandingBytes;
            sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd-osb=" << allowance << endl;
            if (state->peerRwnd < path->pmtu) {
                bytes.bytesToSend = 0;
                bytes.chunk = true;
                sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): one chunk" << endl;
                // RFC4960: When a Fast Retransmit is being performed, the sender SHOULD ignore the value
                // of cwnd and SHOULD NOT delay retransmission for this single packet.
                return;
            }
            else if (allowance > 0) {
                CounterMap::const_iterator bit = qCounter.bookedTransQ.find(path->remoteAddress);
                if (bit->second > (uint32)allowance) {
                    bytes.bytesToSend = allowance;
                    sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd does not allow all RTX" << endl;
                    return;  // More bytes available than allowed -> just return what is allowed.
                }
                else {
                    bytes.bytesToSend = bit->second;
                    sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd allows more than those "
                                 << bytes.bytesToSend << " bytes for retransmission" << endl;
                }
            }
            else {  // You may retransmit one packet
                bytes.packet = true;
                sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): allowance<=0: retransmit one packet" << endl;
            }
        }

        // ====== New transmissions ===========================================
        if (!bytes.chunk && !bytes.packet) {
            (this->*ccFunctions.ccUpdateMaxBurst)(path);

            // ====== Get cwnd value to use, according to maxBurstVariant ======
            uint32 myCwnd = path->cwnd;

            // ====== Obtain byte allowance ====================================
            if ((((state->peerAllowsChunks) &&
                    (path->outstandingBytes < myCwnd) &&
                    (!state->peerWindowFull) &&
                    (state->peerMsgRwnd > 0))
                    ||
                    ((!state->peerAllowsChunks) &&
                    (path->outstandingBytes < myCwnd) &&
                    (!state->peerWindowFull)))) {
                sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "):"
                          << " bookedSumSendStreams=" << qCounter.bookedSumSendStreams
                          << " bytes.bytesToSend="      << bytes.bytesToSend << endl;
                const int32 allowance = myCwnd - path->outstandingBytes - bytes.bytesToSend;
                if (allowance > 0) {
                    if (qCounter.bookedSumSendStreams > (uint32)allowance) {
                        bytes.bytesToSend = myCwnd - path->outstandingBytes;
                        sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): bytesToSend are limited by cwnd: "
                                  << bytes.bytesToSend << endl;
                    }
                    else {
                        bytes.bytesToSend += qCounter.bookedSumSendStreams;
                        sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): send all stored bytes: "
                                  << bytes.bytesToSend << endl;
                    }
                }
            }
        }
    }

    sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "):"
              << " osb="  << path->outstandingBytes
              << " cwnd=" << path->cwnd
              << " bytes.packet=" << (bytes.packet ? "YES!" : "no")
              << " bytes.chunk="     << (bytes.chunk    ? "YES!" : "no")
              << " bytes.bytesToSend=" << bytes.bytesToSend << endl;
}


void SCTPAssociation::sendOnPath(SCTPPathVariables* pathId, bool firstPass)
{
    // ====== Variables ======================================================
    SCTPPathVariables*  path = NULL;      // Path to send next message to
    SCTPMessage*            sctpMsg = NULL;
    SCTPSackChunk*          sackChunk = NULL;
    SCTPDataChunk*          chunkPtr = NULL;
    SCTPForwardTsnChunk* forwardChunk = NULL;

    uint16 chunksAdded = 0;
    uint16 dataChunksAdded = 0;
    uint32 totalChunksSent = 0;
    uint32 totalPacketsSent = 0;
    uint32 outstandingBytes = 0;

    uint32 tcount = 0;     // Bytes in transmission queue on the selected path
    uint32 Tcount = 0;     // Bytes in transmission queue on all paths
    uint32 scount = 0;     // Bytes in send streams
    int32 bytesToSend = 0;

    bool headerCreated = false;
    bool sendOneMorePacket = false;
    bool sendingAllowed = true;
    bool authAdded = false;
    bool sackAdded = false;
    bool forwardPresent = false;

    // ====== Obtain path ====================================================
    sctpEV3 << endl << "##### sendAll(";
    if (pathId) {
        sctpEV3 << pathId->remoteAddress;
    }
    sctpEV3 << ") at t=" << simTime() << " #####" << endl;
    while (sendingAllowed)
    {
        headerCreated = false;
        if (state->bytesToRetransmit > 0) {
            // There are bytes in the transmissionQ. They have to be sent first.
            path = choosePathForRetransmission();
            assert(path != NULL);
        }
        else {
            if (pathId == NULL) {   // No path given => use primary path.
                path = state->getPrimaryPath();
            }
            else {
                path = pathId;
            }
        }

        outstandingBytes = path->outstandingBytes;
        assert((int32)outstandingBytes >= 0);
        CounterMap::iterator tq = qCounter.roomTransQ.find(path->remoteAddress);
        tcount = tq->second;
        Tcount = getAllTransQ();
        scount = qCounter.roomSumSendStreams;     // includes header and padding
        sctpEV3 << "\nsendAll: on " << path->remoteAddress << ":"
                  << " tcount="      << tcount
                  << " Tcount=" << Tcount
                  << " scount="      << scount
                  << " nextTSN="         << state->nextTSN << endl;

        bool sackOnly;
        bool sackWithData;
        timeForSack(sackOnly, sackWithData);
        if (tcount == 0 && scount == 0) {
            // ====== No DATA chunks to send ===================================
            sctpEV3 << "No DATA chunk available!" << endl;
            if (!sackOnly) {     // SACK?, no data to send
                sctpEV3 << "No SACK to send either" << endl;
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

        if (state->sctpMsg)
        {
            sctpEV3 << "packet was stored -> load packet" << endl;
            loadPacket(path, &sctpMsg, &chunksAdded, &dataChunksAdded, &authAdded);
            headerCreated = true;
        }
        else if (bytesToSend > 0 || bytes.chunk || bytes.packet || sackWithData || sackOnly || forwardPresent) {
            sctpMsg = new SCTPMessage("send");
            sctpMsg->setByteLength(SCTP_COMMON_HEADER);
            headerCreated = true;
            chunksAdded = 0;
        }


        if (sackWithData || sackOnly)
        {
            // SACK can be sent
            assert(headerCreated==true);
            sackChunk = createSack();
            chunksAdded++;
            totalChunksSent++;
            // ------ Create AUTH chunk, if necessary --------------------------
            authAdded = addAuthChunkIfNecessary(sctpMsg, SACK, authAdded);

            // ------ Add SACK chunk -------------------------------------------
            sctpMsg->addChunk(sackChunk);
            sackAdded = true;
            sctpEV3 << assocId << ": SACK added, chunksAdded now " << chunksAdded << " sackOnly=" << sackOnly << " sackWithData=" << sackWithData << "\n";
            if (sackOnly && !(bytesToSend > 0 || bytes.chunk || bytes.packet))
            {
                // There is no data to be sent, just the SACK
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
        if (!forwardPresent && !state->stopSending)
        {
            if (peekAbandonedChunk(path) != NULL)
            {
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
            sctpEV3 << assocId << ": bytesToSend=" << bytesToSend
                      << " bytes.chunk="     << bytes.chunk
                      << " bytes.packet=" << bytes.packet << endl;

            // ====== How many bytes may be transmitted in next packet? ========
            int32 allowance = path->pmtu;     // Default behaviour: send 1 path MTU
                if ((bytesToSend > 0) || (bytes.chunk) || (bytes.packet)) {
                    // Allow 1 more MTU
                }
                else {
                    // No more sending allowed.
                    allowance = 0;
                    bytesToSend = 0;
                }

            if ((allowance > 0) || (bytes.chunk) || (bytes.packet)) {
                bool firstTime = false;   // Is DATA chunk send for the first time?
                SCTPDataVariables* datVar;
                // ------ Create AUTH chunk, if necessary -----------------------
                authAdded = addAuthChunkIfNecessary(sctpMsg, DATA, authAdded);
                if (tcount > 0)  {
                    // ====== Retransmission ========================================
                    // If bytes.packet is true, one packet is allowed to be retransmitted!
                    datVar = getOutboundDataChunk(path,
                                                  path->pmtu - sctpMsg->getByteLength() - 20,
                                                  (bytes.packet == true) ? path->pmtu : allowance);
                    if (datVar == NULL) {
                        if (chunksAdded==1 && sackAdded) {
                            datVar = getOutboundDataChunk(path,
                                    path->pmtu - sctpMsg->getByteLength() + sackChunk->getByteLength() - 20,
                                    (bytes.packet == true) ? path->pmtu : allowance);
                            if (!sackOnly)  {
                                sctpMsg->removeChunk();
                                sctpEV3 << "RTX: Remove SACK chunk\n";
                                delete sackChunk;
                                chunksAdded--;
                                sackAdded = false;
                            }
                            else {
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
                                sctpEV3 << "RTX: send only SACK\n";
                                sendSACKviaSelectedPath(sctpMsg);
                                if (datVar != NULL)  {
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
                            sctpEV3 << simTime() << ": Retransmission #" << datVar->numberOfRetransmissions
                                      << " of TSN " << datVar->tsn
                                      << " on path " << datVar->getNextDestination()
                                      << " (last was " << datVar->getLastDestination() << ")" << endl;

                            datVar->countsAsOutstanding = true;
                            datVar->hasBeenReneged = false;
                            increaseOutstandingBytes(datVar, path); // NOTE: path == datVar->getNextDestinationPath()
                        }
                    }
                }
                // ====== First Transmission ====================================
                else if ( ((scount > 0) && (!state->nagleEnabled)) || // Data to send and Nagle off
                             ((uint32)scount >= path->pmtu - 32 - 20) || // Data to fill at least one path MTU
                             ((scount > 0) && (state->nagleEnabled) && ((outstandingBytes == 0) || (sackOnly && sackAdded)) )) {   // Data to send, Nagle on and no outstanding bytes
                    if (path == state->getPrimaryPath()) {

                        // ------ Dequeue data message ----------------------------
                        sctpEV3 << assocId << "sendAll:     sctpMsg->length=" << sctpMsg->getByteLength()
                                  << " length datMsg=" << path->pmtu-sctpMsg->getByteLength() - 20 << endl;
                        SCTPDataMsg* datMsg = dequeueOutboundDataMsg(path, path->pmtu-sctpMsg->getByteLength() - 20,
                                                                                    allowance);  
                        if (datMsg == NULL) {
                            if (chunksAdded==1 && sackAdded) {
                                datMsg = dequeueOutboundDataMsg(path, path->pmtu-sctpMsg->getByteLength() + sackChunk->getByteLength() - 20,
                                        allowance);
                                if (!sackOnly)  {
                                    sctpMsg->removeChunk();
                                    sctpEV3 << assocId << ": delete SACK chunk to make room for datMsg (" << &datMsg << "). scount=" << scount << "\n";
                                    delete sackChunk;
                                    chunksAdded--;
                                    sackAdded = false;
                                }
                                else {
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
                                    sctpEV3 << assocId << ": send SACK and make new header for datMsg (" << &datMsg << "). scount=" << scount << "\n";
                                    sendSACKviaSelectedPath(sctpMsg);
                                    if (datMsg != NULL)  {
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
                                (state->queueUpdate == false)) {
                                // Tell upper layer readiness to accept more data
                                sendIndicationToApp(SCTP_I_SEND_MSG);
                                state->queueUpdate = true;
                            }

                            datVar = makeDataVarFromDataMsg(datMsg, path);
                            delete datMsg;

                            sctpEV3 << assocId << ":: sendAll: chunk " << datVar << " dequeued from StreamQ "
                                    << datVar->sid << ": tsn=" << datVar->tsn
                                    << ", bytes now " << qCounter.roomSumSendStreams << "\n";
                        }

                        // ------ No data message has been dequeued ---------------
                        else {
                            sctpEV3 << assocId << ": No data message has been dequeued" << endl;
                            // ------ Are there any chunks to send? ----------------
                            if (chunksAdded == 0) {
                                // No -> nothing more to do.
                                if (state->sctpMsg == sctpMsg)
                                {
                                    state->sctpMsg = NULL;
                                    state->packetBytes = 0;
                                }
                                packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                            }
                            else {
                                // Yes.
                                if (state->nagleEnabled && ((outstandingBytes > 0) && !(sackOnly && sackAdded)) &&
                                    nextChunkFitsIntoPacket(path, path->pmtu-sctpMsg->getByteLength() - 20) &&
                                    (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                                {
                                    sctpEV3 << "Nagle: Packet has to be stored\n";
                                    storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, authAdded);
                                    sctpMsg = NULL;
                                    chunksAdded = 0;
                                }
                                packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                                sctpEV3 << "sendAll: packetFull: msg length = " << sctpMsg->getByteLength() + 20 << "\n";
                            }
                        }
                    }
                    else if (chunksAdded==1 && sackAdded && !sackOnly)  {
                        sctpMsg->removeChunk();
                        sctpEV3 << "Nagle or no data: Remove SACK chunk, delete sctpmsg" << endl;
                        delete sackChunk;
                        packetFull = true;
                        sackAdded = false;
                        chunksAdded--;
                    }
                    else if ((chunksAdded==1 && sackAdded && sackOnly) || headerCreated )  {
                        packetFull = true;
                    }
                }
                else if (chunksAdded==1 && sackAdded && !sackOnly)  {
                    sctpMsg->removeChunk();
                    sctpEV3 << "Nagle or no data: Remove SACK chunk, delete sctpmsg\n";
                    delete sackChunk;
                    packetFull = true;
                    sackAdded = false;
                    chunksAdded--;
                }
                else if (chunksAdded==1 && sackAdded && sackOnly)  {
                    packetFull = true;
                }
                else if (datVar==NULL || chunksAdded == 0)
                {
                    sctpEV3 << "HeaderCreated=" << headerCreated << ", chunksAdded=" << chunksAdded << " datVar=" << datVar << "\n";
                    if (headerCreated)
                    {
                        packetFull = true;
                    }
                }


                // ------ Handle DATA chunk -------------------------------------
                if (datVar != NULL && !packetFull) {
                    // ------ Assign TSN -----------------------------------------
                    if (firstTime) {
                        assert(datVar->tsn == 0);
                        datVar->tsn = state->nextTSN;
                        sctpEV3 << "sendAll: set TSN=" << datVar->tsn
                                << " sid=" << datVar->sid << ", ssn=" << datVar->ssn << "\n";
                        state->nextTSN++;
                        path->vectorPathSentTSN->record(datVar->tsn);
                    }
                    else {
                        if (datVar->hasBeenFastRetransmitted)  {
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
                    datVar->sendTime = simTime(); //I.R. to send Fast RTX just once a RTT
                    if (datVar->firstSendTime == 0) {
                        datVar->firstSendTime = simTime();
                    }

                    // ------ First transmission of datVar -----------------------
                    if (datVar->numberOfTransmissions == 0) {

                        sctpEV3 << "sendAll: " << simTime() << " firstTime, TSN "
                                  << datVar->tsn    << ": lastDestination set to "
                                  << datVar->getLastDestination() << endl;

                        if (!state->firstDataSent) {
                            state->firstDataSent = true;
                        }
                        sctpEV3 << "sendAll: insert in retransmissionQ tsn=" << datVar->tsn << "\n";
                        if (!retransmissionQ->checkAndInsertChunk(datVar->tsn, datVar)) {
                            throw cRuntimeError("Cannot add datVar to retransmissionQ!");
                            // Falls es hier aufschlaegt, muss ueberlegt werden, ob es OK ist, dass datVars nicht eingefuegt werden koennen.
                        }
                        else {
                            sctpEV3 << "sendAll: size of retransmissionQ=" << retransmissionQ->getQueueSize() << "\n";
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
                    sctpEV3<<"sendAll(): adding new outbound data datVar to packet (tsn="<<datVar->tsn<<")...!!!\n";

                    chunkPtr = transformDataChunk(datVar);

                    /* update counters */
                    totalChunksSent++;
                    chunksAdded++;
                    dataChunksAdded++;
                    sctpEV3 << assocId << ": DataChunk with TSN=" << chunkPtr->getTsn() << " and length " << chunkPtr->getByteLength() << " added\n";
                    sctpMsg->addChunk(chunkPtr);
                    if (datVar->numberOfTransmissions>1)  {
                        CounterMap::iterator tq = qCounter.roomTransQ.find(path->remoteAddress);
                        if (tq->second > 0)  {
                            if (transmissionQ->getSizeOfFirstChunk(path->remoteAddress) > path->pmtu - sctpMsg->getByteLength() - 20)
                                packetFull = true;
                        }
                        else if (nextChunkFitsIntoPacket(path, path->pmtu - sctpMsg->getByteLength() - 20) == false) {
                            packetFull = true;
                        }
                    }
                    else  {
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
                            sctpEV3 << assocId << ": sendAll: one more packet allowed\n";
                        }
                        else {
                            if (state->nagleEnabled && (outstandingBytes > 0) &&
                                nextChunkFitsIntoPacket(path, path->pmtu-sctpMsg->getByteLength() - 20) &&
                                (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                            {
                                storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, authAdded);
                                sctpMsg = NULL;
                                chunksAdded = 0;
                                packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                            }
                            else {
                                packetFull = true;
                            }
                        }
                        bytesToSend = 0;
                    }
                    else if ((qCounter.roomSumSendStreams == 0) && (tq->second==0)) {
                        packetFull = true;
                        sctpEV3 << "sendAll: no data in send and transQ: packet full\n";
                    }
                    sctpEV3 << "sendAll: bytesToSend after reduction: " << bytesToSend << "\n";
                }  // end if (datVar != NULL && !packetFull)

                // ------ There is no DATA chunk, only control chunks possible --
                else {
                    if (chunksAdded == 0) {   // Nothing to do -> return
                        packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                    }
                    else {
                        packetFull = true;
                        sctpEV3 << assocId << ": sendAll: packetFull: msg length = " << sctpMsg->getByteLength() + 20 << "\n";
                        datVar = NULL;
                    }
                }


                // ====== Send packet ===========================================
                if (packetFull) {
                    if (chunksAdded == 0) {   // Nothing to send
                        delete sctpMsg;
                        sendingAllowed = false;   // sendingAllowed==false => leave outer while loop
                    }
                    else {
                        sctpEV3 << assocId << ":: sendAll: " << simTime() << "    packet full:"
                                << "    totalLength=" << sctpMsg->getBitLength() / 8 + 20
                                << ",    path="   << path->remoteAddress
                                << "     "                << dataChunksAdded << " chunks added, outstandingBytes now "
                                << path->outstandingBytes << "\n";

                        /* new chunks would exceed MTU, so we send old packet and build a new one */
                        /* this implies that at least one data chunk is send here */
                        if (dataChunksAdded > 0) {
                            if (!path->T3_RtxTimer->isScheduled()) {
                                // Start retransmission timer, if not scheduled before
                                startTimer(path->T3_RtxTimer, path->pathRto);
                            }
                            else {
                                sctpEV3 << "sendAll: RTX Timer already scheduled -> no need to schedule it\n";
                            }
                        }
                        if (sendOneMorePacket) {
                            sendOneMorePacket = false;
                            bytesToSend = 0;
                            bytes.packet = false;
                            chunkPtr->setIBit(sctpMain->sackNow);
                        }

                        if (dataChunksAdded > 0) {
                            state->ssNextStream = true;
                        }
                        sctpEV3 << assocId << ":sendToIP: packet size=" << sctpMsg->getByteLength() << " numChunks=" << sctpMsg->getChunksArraySize() << "\n";
                        sendToIP(sctpMsg, path->remoteAddress);
                        pmDataIsSentOn(path);
                        totalPacketsSent++;

                        state->lastTransmission = simTime();
                        state->packetsInTotalBurst++;

                        // ------ Reset status ------------------------------------
                        firstTime = false;
                        headerCreated = false;
                        chunksAdded = 0;
                        dataChunksAdded = 0;
                        authAdded = false;

                        sctpEV3 << "sendAll: sending Packet to path " << path->remoteAddress
                                  << "  scount=" << scount
                                  << "  tcount=" << tcount
                                  << "  bytesToSend=" << bytesToSend << endl;
                    }
                }
                sctpEV3 << "sendAll: still " << bytesToSend
                          << " bytes to send, headerCreated=" << headerCreated << endl;

            }    // if (bytesToSend > 0 || bytes.chunk || bytes.packet)
            else {
                packetFull = true;  // Leave inner while loop
                delete sctpMsg;     // T.D. 19.01.2010: Free unsent message
            }

            sctpEV3 << "packetFull=" << packetFull << endl;
        }    // while(!packetFull)

        sctpEV3 << "bytesToSend="    << bytesToSend
                  << " bytes.chunk="     << bytes.chunk
                  << " bytes.packet=" << bytes.packet << endl;
        if (!(bytesToSend > 0 || bytes.chunk || bytes.packet)) {
            sendingAllowed = false;
        }
    }    // while(sendingAllowed)

    sctpEV3 << "sendAll: nothing more to send... BYE!\n";
}


uint32 SCTPAssociation::getAllTransQ()
{
    uint32 sum = 0;
    for (CounterMap::iterator tq = qCounter.roomTransQ.begin(); tq!=qCounter.roomTransQ.end(); tq++) {
        sum += tq->second;
    }
    return sum;
}
