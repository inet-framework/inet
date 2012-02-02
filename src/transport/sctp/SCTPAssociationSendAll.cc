//
// Copyright (C) 2007-2009 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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
    state->outstandingBytes += chunk->booksize;

    CounterMap::iterator iterator = qCounter.roomRetransQ.find(path->remoteAddress);
        iterator->second += ADD_PADDING(chunk->booksize + SCTP_DATA_CHUNK_LENGTH);
}

int32 SCTPAssociation::calculateBytesToSendOnPath(const SCTPPathVariables* pathVar)
{
    int32                    bytesToSend;
    const SCTPDataMsg* datMsg = peekOutboundDataMsg();
    if (datMsg != NULL) {
        const uint32 ums = datMsg->getBooksize();        // Get user message size
            const uint32 num = (uint32)floor((double)(pathVar->pmtu - 32) / (ums + SCTP_DATA_CHUNK_LENGTH));
            if (num * ums > state->peerRwnd) {
                // Receiver cannot handle data yet
                bytesToSend = 0;
            }
            else {
                // Receiver will accept data
                bytesToSend = num * ums;
            }
    }
    else {
        bytesToSend = 0;
    }
    return (bytesToSend);
}

void SCTPAssociation::storePacket(SCTPPathVariables* pathVar,
                                             SCTPMessage*         sctpMsg,
                                             const uint16         chunksAdded,
                                             const uint16         dataChunksAdded,
                                             const uint32         packetBytes,
                                             const bool           authAdded)
{
    for (uint16 i = 0; i < sctpMsg->getChunksArraySize(); i++) {
        retransmissionQ->payloadQueue.find(((SCTPDataChunk*)sctpMsg->getChunks(i))->getTsn())->second->countsAsOutstanding = false;
    }
    state->sctpMsg = sctpMsg;
    state->chunksAdded = chunksAdded;
    state->dataChunksAdded = dataChunksAdded;
    state->packetBytes = packetBytes;
    sctpEV3 << "storePacket: path=" << pathVar->remoteAddress
              << " osb=" << pathVar->outstandingBytes << " -> "
              << pathVar->outstandingBytes - state->packetBytes << endl;
    assert(pathVar->outstandingBytes >= state->packetBytes);
    pathVar->outstandingBytes -= state->packetBytes;
        qCounter.roomSumSendStreams += state->packetBytes + (dataChunksAdded * SCTP_DATA_CHUNK_LENGTH);
    qCounter.bookedSumSendStreams += state->packetBytes;

}

void SCTPAssociation::loadPacket(SCTPPathVariables* pathVar,
                                            SCTPMessage**        sctpMsg,
                                            uint16*              chunksAdded,
                                            uint16*              dataChunksAdded,
                                            uint32*              packetBytes,
                                            bool*                    authAdded)
{
    *sctpMsg = state->sctpMsg;
    *chunksAdded = state->chunksAdded;
    *dataChunksAdded = state->dataChunksAdded;
    *packetBytes = state->packetBytes;
    sctpEV3 << "loadPacket: path=" << pathVar->remoteAddress << " osb=" << pathVar->outstandingBytes << " -> " << pathVar->outstandingBytes + state->packetBytes << endl;
    pathVar->outstandingBytes += state->packetBytes;
    qCounter.bookedSumSendStreams -= state->packetBytes;

    for (uint16 i = 0; i < (*sctpMsg)->getChunksArraySize(); i++)
        retransmissionQ->payloadQueue.find(((SCTPDataChunk*)(*sctpMsg)->getChunks(i))->getTsn())->second->countsAsOutstanding = true;

}



SCTPDataVariables* SCTPAssociation::makeDataVarFromDataMsg(SCTPDataMsg*         datMsg,
                                                                              SCTPPathVariables* path)
{
    SCTPDataVariables* datVar = new SCTPDataVariables();

    datMsg->setInitialDestination(path->remoteAddress);
    datVar->setInitialDestination(path);

    datVar->bbit = datMsg->getBBit();
    datVar->ebit = datMsg->getEBit();
    datVar->enqueuingTime = datMsg->getEnqueuingTime();
    datVar->expiryTime = datMsg->getExpiryTime();
    datVar->ppid = datMsg->getPpid();
    datVar->len = datMsg->getBitLength();
    datVar->sid = datMsg->getSid();
    datVar->allowedNoRetransmissions = datMsg->getRtx();
    datVar->booksize = datMsg->getBooksize();

    // ------ Stream handling ---------------------------------------
    SCTPSendStreamMap::iterator iterator = sendStreams.find(datMsg->getSid());
    SCTPSendStream*              stream = iterator->second;
    uint32                           nextSSN = stream->getNextStreamSeqNum();
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


    return (datVar);
}

SCTPPathVariables* SCTPAssociation::choosePathForRetransmission()
{
    uint32               max = 0;
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
        if (((state->numGaps > 0) || (state->dupList.size() > 0)) &&
             (state->sackAllowed)) {
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
    else if (state->peerWindowFull)
    {
        if (path->outstandingBytes == 0) {
            // Zero Window Probing
            sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): zeroWindowProbing" << endl;
            state->zeroWindowProbing = true;
            bytes.chunk = true;
        }
    }

    // ====== Retransmissions ================================================
    else {
        CounterMap::const_iterator it = qCounter.roomTransQ.find(path->remoteAddress);
        sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): bytes in transQ=" << it->second << endl;
        if (it->second > 0) {
            const int32 allowance = path->cwnd - path->outstandingBytes;
            sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): cwnd-osb=" << allowance << endl;
            if (state->peerRwnd < path->pmtu) {
                bytes.bytesToSend = state->peerRwnd;
                sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "): rwnd<pmtu" << endl;
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
            if ((path->outstandingBytes < path->cwnd) &&
                 (!state->peerWindowFull)) {
                sctpEV3 << "bytesAllowedToSend(" << path->remoteAddress << "):"
                          << " bookedSumSendStreams=" << qCounter.bookedSumSendStreams
                          << " bytes.bytesToSend="      << bytes.bytesToSend << endl;
                const int32 allowance = path->cwnd - path->outstandingBytes - bytes.bytesToSend;
                if (allowance > 0) {
                    if (qCounter.bookedSumSendStreams > (uint32)allowance) {
                        bytes.bytesToSend = path->cwnd - path->outstandingBytes;
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

    uint16 chunksAdded = 0;
    uint16 dataChunksAdded = 0;
    uint32 totalChunksSent = 0;
    uint32 totalPacketsSent = 0;
    uint32 packetBytes = 0;
    uint32 outstandingBytes = 0;

    uint32 tcount = 0;     // Bytes in transmission queue on the selected path
    uint32 scount = 0;     // Bytes in send streams
    int32 bytesToSend = 0;

    bool headerCreated = false;
    bool rtxActive = false;
    bool sendOneMorePacket = false;
    bool sendingAllowed = true;
    bool authAdded = false;
    bool sackAdded = false;

    // ====== Obtain path ====================================================
    sctpEV3 << endl << "##### sendAll(";
    if (pathId) {
        sctpEV3 << pathId->remoteAddress;
    }
    sctpEV3 << ") #####" << endl;
    while (sendingAllowed)
    {
        headerCreated = false;
        if (state->bytesToRetransmit > 0) {
            // There are bytes in the transmissionQ. They have to be sent first.
            path = choosePathForRetransmission();
            assert(path != NULL);
            rtxActive = true;
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
        scount = qCounter.roomSumSendStreams;     // includes header and padding
        sctpEV3 << "\nsendAll: on " << path->remoteAddress << ":"
                  << " tcount="      << tcount
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

        if (state->sctpMsg)  // ??? Robin: Ist das in Ordnung???
        {
            loadPacket(path, &sctpMsg, &chunksAdded, &dataChunksAdded, &packetBytes, &authAdded);
            headerCreated = true;
        }
        else if (bytesToSend > 0 || bytes.chunk || bytes.packet || sackWithData || sackOnly) {
            sctpMsg = new SCTPMessage("send");
            //printf("%d Name=%s Pointer=%p\n", __LINE__, sctpMsg->getName(), sctpMsg);
            sctpMsg->setByteLength(SCTP_COMMON_HEADER);
            packetBytes = 0;
            headerCreated = true;
        }


        if (sackWithData || sackOnly)
        {
            // SACK can be sent
            assert(headerCreated==true);
            sackChunk = createSack();
            chunksAdded++;
            totalChunksSent++;
            // ------ Create AUTH chunk, if necessary --------------------------

            // ------ Add SACK chunk -------------------------------------------
            sctpMsg->addChunk(sackChunk);
            sackAdded = true;
            if (sackOnly)     // ????? Robin: SACK mit FORWARD_TSN????
            {
                // send the packet and leave
                //printf("%d Name=%s Pointer=%p, sctpMsg\n", __LINE__, sctpMsg->getName(), sctpMsg);
                state->ackState = 0;
                // Stop SACK timer if it is running...
                stopTimer(SackTimer);
                sctpAlgorithm->sackSent();
                state->sackAllowed = false;
                sendToIP(sctpMsg, state->lastDataSourceAddress);
                if ((bytesToSend > 0) || (bytes.chunk) || (bytes.packet)) {
                    sctpMsg = new SCTPMessage("send");
                    sctpMsg->setByteLength(SCTP_COMMON_HEADER);
                    packetBytes = 0;
                    headerCreated = true;
                    sackAdded = false;
                }
                else
                    return;
            }
        }


        // ####################################################################
        // #### Data Transmission                                                        ####
        // ####################################################################

        bool packetFull = false;

        while (!packetFull && headerCreated) {
            assert(headerCreated == true);
            sctpEV3 << "bytesToSend="    << bytesToSend
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

                // ====== Retransmission ========================================
                // T.D. 05.01.2010: If bytes.packet is true, one packet is allowed
                //                        to be retransmitted!
                SCTPDataVariables* datVar = getOutboundDataChunk(path,
                                                                                 path->pmtu - sctpMsg->getByteLength() - 20,
                                                                                 (bytes.packet == true) ? path->pmtu : allowance);
                if (chunksAdded==1 && sackAdded && !sackOnly && datVar==NULL) {
                    sctpMsg->removeChunk();
                    delete sackChunk;
                    datVar = getOutboundDataChunk(path,
                                                            path->pmtu - sctpMsg->getByteLength() - 20,
                                                            (bytes.packet == true) ? path->pmtu : allowance);
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

                // ====== First Transmission ====================================
                else if ( ((scount > 0) && (!state->nagleEnabled)) || // Data to send and Nagle off
                             ((uint32)scount >= path->pmtu - 32 - 20) || // Data to fill at least one path MTU
                             ((scount > 0) && (state->nagleEnabled) && (outstandingBytes == 0)) ) {   // Data to send, Nagle on and no outstanding bytes

                    if (path == state->getPrimaryPath()) {

                        // ------ Dequeue data message ----------------------------
                        sctpEV3 << "sendAll:     sctpMsg->length=" << sctpMsg->getByteLength()
                                  << " length datMsg=" << path->pmtu-sctpMsg->getByteLength() - 20 << endl;
                        SCTPDataMsg* datMsg = dequeueOutboundDataMsg(path->pmtu-sctpMsg->getByteLength() - 20,
                                                                                    allowance);
                        if (chunksAdded==1 && sackAdded && !sackOnly && datMsg==NULL)
                        {
                            sctpMsg->removeChunk();
                            delete sackChunk;
                            datMsg = dequeueOutboundDataMsg(path->pmtu-sctpMsg->getByteLength() - 20,
                                                                    allowance);
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

                            sctpEV3 << "sendAll: chunk " << datVar << " dequeued from StreamQ "
                                    << datVar->sid << ": tsn=" << datVar->tsn
                                    << ", bytes now " << qCounter.roomSumSendStreams << "\n";
                        }

                        // ------ No data message has been dequeued ---------------
                        else {
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
                                if (state->nagleEnabled && (outstandingBytes > 0) &&
                                    nextChunkFitsIntoPacket(path->pmtu-sctpMsg->getByteLength() - 20) &&
                                    (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                                {
                                    storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, packetBytes, authAdded);
                                    packetBytes = 0;
                                }
                                //chunksAdded = 0;
                                packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                                sctpEV3 << "sendAll: packetFull: msg length = " << sctpMsg->getBitLength() / 8 + 20 << "\n";
                            }
                        }
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
                    }

                    SCTP::AssocStatMap::iterator iterator = sctpMain->assocStatMap.find(assocId);
                    iterator->second.transmittedBytes += datVar->len / 8;

                    datVar->setLastDestination(path);
                    datVar->countsAsOutstanding = true;
                    datVar->hasBeenReneged = false;
                    datVar->sendTime = simTime(); //I.R. to send Fast RTX just once a RTT

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
                    sctpMsg->addChunk(chunkPtr);
                    if (nextChunkFitsIntoPacket(path->pmtu - sctpMsg->getByteLength() - 20) == false) {
                        // ???? Robin: Ist diese Annahme so richtig?
                        packetFull = true;
                    }

                    state->peerRwnd -= datVar->booksize;
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
                            sctpEV3 << "sendAll: one more packet allowed\n";
                        }
                        else {
                            if (state->nagleEnabled && (outstandingBytes > 0) &&
                                nextChunkFitsIntoPacket(path->pmtu-sctpMsg->getByteLength() - 20) &&
                                (sctpMsg->getByteLength() < path->pmtu - 32 - 20) && (tcount == 0))
                            {
                                storePacket(path, sctpMsg, chunksAdded, dataChunksAdded, packetBytes, authAdded);
                                packetBytes = 0;
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
                }

                // ------ There is no DATA chunk, only control chunks possible --
                else {
                    // ????? Robin: Kann dieser Fall wirklich eintreten?
                    if (chunksAdded == 0) {   // Nothing to do -> return
                        packetFull = true;  // chunksAdded==0, packetFull==true => leave inner while loop
                    }
                    else {
                        packetFull = true;
                        sctpEV3 << "sendAll: packetFull: msg length = " << sctpMsg->getBitLength() / 8 + 20 << "\n";
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
                        sctpEV3 << "sendAll: " << simTime() << "    packet full:"
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
                        }


                        sendToIP(sctpMsg, path->remoteAddress);
                        pmDataIsSentOn(path);
                        totalPacketsSent++;

                        // ------ Reset status ------------------------------------
                        if (state->sctpMsg == sctpMsg)
                        {
                            state->sctpMsg = NULL;
                            path->outstandingBytes += packetBytes;
                            packetBytes = 0;
                        }
                        headerCreated = false;
                        chunksAdded = 0;
                        dataChunksAdded = 0;
                        firstTime = false;

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
