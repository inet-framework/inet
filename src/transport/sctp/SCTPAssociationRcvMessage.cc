//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <string.h>
#include <assert.h>

#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "SCTPMessage_m.h"
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"
#include "common.h"

#ifdef WITH_IPv4
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#endif

#include "IPv4ControlInfo_m.h"


void SCTPAssociation::decreaseOutstandingBytes(SCTPDataVariables* chunk)
{
    SCTPPathVariables* lastPath = chunk->getLastDestinationPath();

    if (chunk->countsAsOutstanding) {
        assert(lastPath->outstandingBytes >= chunk->booksize);
        lastPath->outstandingBytes -= chunk->booksize;
        lastPath->statisticsPathOutstandingBytes->record(lastPath->outstandingBytes);
        state->outstandingBytes -= chunk->booksize;
        assert((int64)state->outstandingBytes >= 0);
        statisticsOutstandingBytes->record(state->outstandingBytes);

        chunk->countsAsOutstanding = false;

        CounterMap::iterator iterator = qCounter.roomRetransQ.find(lastPath->remoteAddress);
        state->outstandingMessages--;
        if (state->osbWithHeader)
            iterator->second -= ADD_PADDING(chunk->booksize);
        else
            iterator->second -= ADD_PADDING(chunk->booksize + SCTP_DATA_CHUNK_LENGTH);
    }
}


bool SCTPAssociation::process_RCV_Message(SCTPMessage*       sctpmsg,
                                                        const Address& src,
                                                        const Address& dest)
{
    // ====== Header checks ==================================================
    sctpEV3 << getFullPath()  << " SCTPAssociationRcvMessage:process_RCV_Message"
              << " localAddr="  << localAddr
              << " remoteAddr=" << remoteAddr << endl;
    state->pktDropSent = false;
    if ((sctpmsg->hasBitError() || !sctpmsg->getChecksumOk()))
    {
        if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType() == INIT_ACK) {
            stopTimer(T1_InitTimer);
            sctpEV3 << "InitAck with bit-error. Retransmit Init" << endl;
            retransmitInit();
            startTimer(T1_InitTimer, state->initRexmitTimeout);
        }
        if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType() == COOKIE_ACK) {
            stopTimer(T1_InitTimer);
            sctpEV3 << "CookieAck with bit-error. Retransmit CookieEcho" << endl;
            retransmitCookieEcho();
            startTimer(T1_InitTimer, state->initRexmitTimeout);
        }
        if (!(sctpMain->pktdrop) || !state->peerPktDrop) {
            sctpEV3 << "Packet has bit-error. Return\n";
            return true;
        }
    }

    SCTPPathVariables* path = getPath(src);
    const uint16 srcPort = sctpmsg->getDestPort();
    const uint16 destPort = sctpmsg->getSrcPort();
    const uint32 numberOfChunks = sctpmsg->getChunksArraySize();
    sctpEV3 << "numberOfChunks=" <<numberOfChunks << endl;

    state->sctpmsg = (SCTPMessage*)sctpmsg->dup();
    bool authenticationNecessary = state->peerAuth;
    if ((sctpmsg->getChecksumOk() == false || sctpmsg->hasBitError()) &&
            (sctpMain->pktdrop) &&
            (state->peerPktDrop)) {
        sendPacketDrop(true);
        return true;
    }

    if (fsm->getState()!=SCTP_S_CLOSED &&
            fsm->getState()!=SCTP_S_COOKIE_WAIT &&
            fsm->getState()!=SCTP_S_COOKIE_ECHOED &&
            fsm->getState()!=SCTP_S_SHUTDOWN_ACK_SENT &&
            simTime() > state->lastAssocThroughputTime + state->throughputInterval &&
            assocThroughputVector != NULL) {
        assocThroughputVector->record(state->assocThroughput / (simTime() - state->lastAssocThroughputTime) / 1000);
        state->lastAssocThroughputTime = simTime();
        state->assocThroughput = 0;
    }
    state->assocThroughput += sctpmsg->getByteLength();

    // ====== Handle chunks ==================================================
    bool trans = true;
    bool sendAllowed = false;
    bool dupReceived = false;
    bool dataChunkReceived = false;
    bool shutdownCalled = false;
    for (uint32 i = 0; i < numberOfChunks; i++) {
        SCTPChunk* header = (SCTPChunk*)(sctpmsg->removeChunk());
        const uint8       type = header->getChunkType();

        if ((type != INIT) &&
             (type != ABORT) &&
             (type != ERRORTYPE) &&
             (sctpmsg->getTag() != peerVTag)) {
            sctpEV3 << " VTag "<< sctpmsg->getTag() << " incorrect. Should be "
                      << peerVTag << " localVTag=" << localVTag << endl;
            return true;
        }

        if (authenticationNecessary) {
            if (type == AUTH) {
                sctpEV3 << "SCTPAssociationRcvMessage: AUTH received" << endl;
                SCTPAuthenticationChunk* authChunk;
                authChunk = check_and_cast<SCTPAuthenticationChunk*>(header);
                if (authChunk->getHMacIdentifier() != 1) {
                    sendHMacError(authChunk->getHMacIdentifier());
                    SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
                    it->second.numAuthChunksRejected++;
                    delete authChunk;
                    return true;
                }
                if (authChunk->getHMacOk() == false) {
                    delete authChunk;
                    SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
                    it->second.numAuthChunksRejected++;
                    return true;
                }
                authenticationNecessary = false;
                SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
                it->second.numAuthChunksAccepted++;
                delete authChunk;
                continue;
            }
            else
            {
                if (typeInChunkList(type)) {
                    return true;
                }
            }
        }

        switch (type) {
        case INIT:
            sctpEV3 << "SCTPAssociationRcvMessage: INIT received" << endl;
            SCTPInitChunk* initChunk;
            initChunk = check_and_cast<SCTPInitChunk*>(header);
            if ((initChunk->getNoInStreams() != 0) &&
                 (initChunk->getNoOutStreams() != 0) &&
                 (initChunk->getInitTag() != 0)) {
                trans = processInitArrived(initChunk, srcPort, destPort);
            }
            i = numberOfChunks-1;
            delete initChunk;
            break;
        case INIT_ACK:
            sctpEV3 << "SCTPAssociationRcvMessage: INIT_ACK received" << endl;
            if (fsm->getState() == SCTP_S_COOKIE_WAIT) {
                SCTPInitAckChunk* initAckChunk;
                initAckChunk = check_and_cast<SCTPInitAckChunk*>(header);
                if ((initAckChunk->getNoInStreams() != 0) &&
                    (initAckChunk->getNoOutStreams() != 0) &&
                    (initAckChunk->getInitTag() != 0)) {
                    trans = processInitAckArrived(initAckChunk);
                }
                else if (initAckChunk->getInitTag() == 0) {
                    sendAbort();
                    sctpMain->removeAssociation(this);
                }
                i = numberOfChunks-1;
                delete initAckChunk;
            }
            else {
                sctpEV3 << "INIT_ACK will be ignored" << endl;
            }
            break;
        case COOKIE_ECHO:
            sctpEV3 << "SCTPAssociationRcvMessage: COOKIE_ECHO received" << endl;
            SCTPCookieEchoChunk* cookieEchoChunk;
            cookieEchoChunk = check_and_cast<SCTPCookieEchoChunk*>(header);
            trans = processCookieEchoArrived(cookieEchoChunk, src);
            delete cookieEchoChunk;
            break;
        case COOKIE_ACK:
            sctpEV3 << "SCTPAssociationRcvMessage: COOKIE_ACK received" << endl;
            if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                SCTPCookieAckChunk* cookieAckChunk;
                cookieAckChunk = check_and_cast<SCTPCookieAckChunk*>(header);
                trans = processCookieAckArrived();
                delete cookieAckChunk;
            }
            break;
        case DATA:
            sctpEV3 << "SCTPAssociationRcvMessage: DATA received" << endl;
            if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
            }
            if (!(fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED || fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)) {
                SCTPDataChunk* dataChunk;
                dataChunk = check_and_cast<SCTPDataChunk*>(header);
                if ((dataChunk->getByteLength()- 16) > 0) {
                    const SCTPEventCode event = processDataArrived(dataChunk);
                    if (event == SCTP_E_DELIVERED) {
                        if ((state->streamReset) &&
                                    (state->lastTsnBeforeReset == state->gapList.getHighestTSNReceived())) {
                            resetExpectedSsns();
                            if (state->inOut) {
                                sendOutgoingRequestAndResponse(state->inRequestSn, state->peerRequestSn);
                            }
                            else {
                                sendStreamResetResponse(state->peerRequestSn);
                            }
                        }
                        dataChunkReceived = true;
                        state->sackAllowed = true;
                    }
                    else if (event==SCTP_E_SEND || event==SCTP_E_IGNORE) {
                        dataChunkReceived = true;
                        state->sackAllowed = true;
                    }
                    else if (event==SCTP_E_DUP_RECEIVED) {
                        dupReceived = true;
                    }
                }
                else {
                    sendAbort();
                    sctpMain->removeAssociation(this);
                }
                delete dataChunk;
            }
            trans = true;
            break;
        case SACK:
        case NR_SACK:
        {
            sctpEV3 << "SCTPAssociationRcvMessage: SACK received" << endl;
            const int32 scount = qCounter.roomSumSendStreams;
            SCTPSackChunk* sackChunk;
            sackChunk = check_and_cast<SCTPSackChunk*>(header);
            processSackArrived(sackChunk);
            trans = true;
            sendAllowed = true;
            delete sackChunk;
            if (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && scount==0) {
                if (fsm->getState() == SCTP_S_SHUTDOWN_PENDING) {
                    sctpEV3 << "No more packets: send SHUTDOWN" << endl;
                    sendShutdown();
                    trans = performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
                    shutdownCalled = true;
                }
                else if (fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED) {
                    sctpEV3 << "No more outstanding" << endl;
                    sendShutdownAck(remoteAddr);
                }
            }
            break;
        }
        case ABORT:
            SCTPAbortChunk* abortChunk;
            abortChunk = check_and_cast<SCTPAbortChunk*>(header);
            sctpEV3 << "SCTPAssociationRcvMessage: ABORT with T-Bit "
                      << abortChunk->getT_Bit() << " received" << endl;
            if (sctpmsg->getTag() == localVTag || sctpmsg->getTag() == peerVTag) {
                sendIndicationToApp(SCTP_I_ABORT);
                trans = performStateTransition(SCTP_E_ABORT);
            }
            delete abortChunk;
            break;
        case HEARTBEAT:
            sctpEV3 << "SCTPAssociationRcvMessage: HEARTBEAT received" << endl;
            SCTPHeartbeatChunk* heartbeatChunk;
            heartbeatChunk = check_and_cast<SCTPHeartbeatChunk*>(header);
            if (!(fsm->getState() == SCTP_S_CLOSED)) {
                sendHeartbeatAck(heartbeatChunk, dest, src);
            }
            trans = true;
            delete heartbeatChunk;
            if (path) {
                path->numberOfHeartbeatsRcvd++;
                path->vectorPathRcvdHb->record(path->numberOfHeartbeatsRcvd);
            }
            break;
        case HEARTBEAT_ACK:
            sctpEV3 << "SCTPAssociationRcvMessage: HEARTBEAT_ACK received" << endl;
            if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
            }
            SCTPHeartbeatAckChunk* heartbeatAckChunk;
            heartbeatAckChunk = check_and_cast<SCTPHeartbeatAckChunk*>(header);
            if (path) {
                processHeartbeatAckArrived(heartbeatAckChunk, path);
            }
            trans = true;
            delete heartbeatAckChunk;
            break;
        case SHUTDOWN:
            sctpEV3 << "SCTPAssociationRcvMessage: SHUTDOWN received" << endl;
            SCTPShutdownChunk* shutdownChunk;
            shutdownChunk = check_and_cast<SCTPShutdownChunk*>(header);
            if (shutdownChunk->getCumTsnAck()>state->lastTsnAck) {
                simtime_t rttEstimation = MAXTIME;
                dequeueAckedChunks(shutdownChunk->getCumTsnAck(),
                                         getPath(remoteAddr), rttEstimation);
                state->lastTsnAck = shutdownChunk->getCumTsnAck();
            }
            trans = performStateTransition(SCTP_E_RCV_SHUTDOWN);
            sendIndicationToApp(SCTP_I_SHUTDOWN_RECEIVED);
            trans = true;
            delete shutdownChunk;
            break;
        case SHUTDOWN_ACK:
            sctpEV3 << "SCTPAssociationRcvMessage: SHUTDOWN_ACK received" << endl;
            if (fsm->getState()!=SCTP_S_ESTABLISHED) {
                SCTPShutdownAckChunk* shutdownAckChunk;
                shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk*>(header);
                sendShutdownComplete();
                stopTimers();
                stopTimer(T2_ShutdownTimer);
                stopTimer(T5_ShutdownGuardTimer);
                sctpEV3 << "state=" << stateName(fsm->getState()) << endl;
                if ((fsm->getState() == SCTP_S_SHUTDOWN_SENT) ||
                     (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)) {
                    trans = performStateTransition(SCTP_E_RCV_SHUTDOWN_ACK);
                    sendIndicationToApp(SCTP_I_CLOSED);
                    delete state->shutdownChunk;
                }
                delete shutdownAckChunk;
            }
            break;
        case SHUTDOWN_COMPLETE:
            sctpEV3<<"Shutdown Complete arrived" << endl;
            SCTPShutdownCompleteChunk* shutdownCompleteChunk;
            shutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk*>(header);
            trans = performStateTransition(SCTP_E_RCV_SHUTDOWN_COMPLETE);
            sendIndicationToApp(SCTP_I_PEER_CLOSED);     // necessary for NAT-Rendezvous
            if (trans == true) {
                stopTimers();
            }
            stopTimer(T2_ShutdownTimer);
            stopTimer(T5_ShutdownGuardTimer);
            delete state->shutdownAckChunk;
            delete shutdownCompleteChunk;
            break;
        case FORWARD_TSN:
            sctpEV3 << "SCTPAssociationRcvMessage: FORWARD_TSN received" << endl;
            SCTPForwardTsnChunk* forwChunk;
            forwChunk = check_and_cast<SCTPForwardTsnChunk*>(header);
            processForwardTsnArrived(forwChunk);
            trans = true;
            sendAllowed = true;
            dataChunkReceived = true;
            delete forwChunk;
            break;
        case STREAM_RESET:
            sctpEV3 << "SCTPAssociationRcvMessage: StreamReset received" << endl;
            SCTPStreamResetChunk* strResChunk;
            strResChunk = check_and_cast<SCTPStreamResetChunk*>(header);
            processStreamResetArrived(strResChunk);
            trans = true;
            sendAllowed = true;
            delete strResChunk;
            break;
        case ASCONF:
            sctpEV3 << "SCTPAssociationRcvMessage: ASCONF received" << endl;
            if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
            }
            SCTPAsconfChunk* asconfChunk;
            asconfChunk = check_and_cast<SCTPAsconfChunk*>(header);
            processAsconfArrived(asconfChunk);
            trans = true;
            delete asconfChunk;
            break;
        case ASCONF_ACK:
            sctpEV3 << "SCTPAssociationRcvMessage: ASCONF_ACK received" << endl;
            SCTPAsconfAckChunk* asconfAckChunk;
            asconfAckChunk = check_and_cast<SCTPAsconfAckChunk*>(header);
            processAsconfAckArrived(asconfAckChunk);
            trans = true;
            delete state->asconfChunk;
            delete asconfAckChunk;
            break;
        case PKTDROP:
            sctpEV3 << "SCTPAssociationRcvMessage: PKTDROP received" << endl;
            if (sctpMain->pktdrop) {
                SCTPPacketDropChunk* packetDropChunk;
                packetDropChunk = check_and_cast<SCTPPacketDropChunk*>(header);
                if (packetDropChunk->getBFlag() && !packetDropChunk->getMFlag())
                    processPacketDropArrived(packetDropChunk);

                trans = true;
                sendAllowed = true;
                delete packetDropChunk;
            }
            break;
        case ERRORTYPE:
            sctpEV3 << "SCTPAssociationRcvMessage: ERROR received" << endl;
            SCTPErrorChunk* errorChunk;
            errorChunk = check_and_cast<SCTPErrorChunk*>(header);
            processErrorArrived(errorChunk);
            trans = true;
            delete errorChunk;
            break;
        default: sctpEV3<<"different type" << endl;
            break;
        }

        if (i==numberOfChunks-1 && (dataChunkReceived || dupReceived)) {
            sendAllowed = true;
            sctpEV3 << "i=" << i << " sendAllowed=true; scheduleSack" << endl;
            scheduleSack();
            if (fsm->getState() == SCTP_S_SHUTDOWN_SENT && state->ackState>=sackFrequency) {
                sendSack();
            }
        }

        // Send any new DATA chunks, SACK chunks, HEARTBEAT chunks etc.
        sctpEV3 << "SCTPAssociationRcvMessage: send new data? state=" << stateName(fsm->getState())
                  << " sendAllowed="        << sendAllowed
                  << " shutdownCalled=" << shutdownCalled << endl;
        if (((fsm->getState() == SCTP_S_ESTABLISHED) ||
              (fsm->getState() == SCTP_S_SHUTDOWN_PENDING) ||
              (fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED)) &&
             (sendAllowed) &&
             (!shutdownCalled)) {
            sendOnAllPaths(state->getPrimaryPath());
        }
    }

    // ====== Clean-up =======================================================
    if (!state->pktDropSent) {
        disposeOf(state->sctpmsg);
        sctpEV3 << "state->sctpmsg was disposed" << endl;
    }
    return trans;
}

bool SCTPAssociation::processInitArrived(SCTPInitChunk* initchunk, int32 srcPort, int32 destPort)
{
    SCTPAssociation* assoc;
    char timerName[64];
    bool trans = false;
    uint16 type;
    InterfaceTableAccess interfaceTableAccess;
    AddressVector adv;

    sctpEV3<<"processInitArrived\n";
    if (fsm->getState() == SCTP_S_CLOSED)
    {
        sctpEV3<<"fork="<<state->fork<<" initReceived="<<state->initReceived<<"\n";
        if (state->fork && !state->initReceived)
        {
            sctpEV3<<"cloneAssociation\n";
            assoc = cloneAssociation();
            sctpEV3<<"addForkedAssociation\n";
            sctpMain->addForkedAssociation(this, assoc, localAddr, remoteAddr, srcPort, destPort);

            sctpEV3 << "Connection forked: this connection got new assocId=" << assocId << ", "
                "spinoff keeps LISTENing with assocId=" << assoc->assocId << "\n";
            snprintf(timerName, sizeof(timerName), "T2_SHUTDOWN of assoc %d", assocId);
            T2_ShutdownTimer->setName(timerName);
            snprintf(timerName, sizeof(timerName), "T5_SHUTDOWN_GUARD of assoc %d", assocId);
            T5_ShutdownGuardTimer->setName(timerName);
            snprintf(timerName, sizeof(timerName), "SACK_TIMER of assoc %d", assocId);
            SackTimer->setName(timerName);
            snprintf(timerName, sizeof(timerName), "T1_INIT of assoc %d", assocId);
            T1_InitTimer->setName(timerName);
        }
        else
        {
            sctpMain->updateSockPair(this, localAddr, remoteAddr, srcPort, destPort);

        }
        if (!state->initReceived)
        {
            state->initReceived = true;
            state->initialPrimaryPath = remoteAddr;
            state->setPrimaryPath(getPath(remoteAddr));
            if (initchunk->getAddressesArraySize()==0)
            {
                sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<"\n";
                SCTPPathVariables* rPath = new SCTPPathVariables(remoteAddr, this);
                sctpPathMap[rPath->remoteAddress] = rPath;
                qCounter.roomTransQ[rPath->remoteAddress] = 0;
                qCounter.bookedTransQ[rPath->remoteAddress] = 0;
                qCounter.roomRetransQ[rPath->remoteAddress] = 0;
            }
            initPeerTsn = initchunk->getInitTSN();
            state->gapList.setInitialCumAckTSN(initPeerTsn - 1);
            state->initialPeerRwnd = initchunk->getA_rwnd();
            if (initchunk->getMsg_rwnd()>0)
            {
                state->peerAllowsChunks = true;
                state->initialPeerMsgRwnd = initchunk->getMsg_rwnd();
                state->peerMsgRwnd = state->initialPeerMsgRwnd;
            }
            state->peerRwnd = state->initialPeerRwnd;
            statisticsPeerRwnd->record(state->peerRwnd);
            localVTag = initchunk->getInitTag();
            numberOfRemoteAddresses = initchunk->getAddressesArraySize();
            IInterfaceTable *ift = interfaceTableAccess.get();
            state->localAddresses.clear();
            if (localAddressList.front().isUnspecified())
            {
                for (int32 i=0; i<ift->getNumInterfaces(); ++i)
                {
                    if (ift->getInterface(i)->ipv4Data()!=NULL)
                    {
#ifdef WITH_IPv4
                        adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
#else
                        throw cRuntimeError("INET was compiled without IPv4 support");
#endif
                    }
                    else if (ift->getInterface(i)->ipv6Data()!=NULL)
                    {
#ifdef WITH_IPv6
                        adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(0));
#else
                        throw cRuntimeError("INET was compiled without IPv6 support");
#endif
                    }
                }
            }
            else
            {
                adv = localAddressList;
            }
            int rlevel = getAddressLevel(remoteAddr);
            if (adv.size() == 1) {
                state->localAddresses.push_back((*adv.begin()));
            } else if (rlevel>0) {
                for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
                {
                    if (getAddressLevel((*i))>=rlevel)
                    {
                        sctpMain->addLocalAddress(this, (*i));
                        state->localAddresses.push_back((*i));
                    }
                }
            }
            for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
            {
                // skip IPv6 because we can't send to them yet
                if (initchunk->getAddresses(j).getType() == Address::IPv6)
                    continue;
                // set path variables for this pathlocalAddresses
                if (!getPath(initchunk->getAddresses(j)))
                {
                    SCTPPathVariables* path = new SCTPPathVariables(initchunk->getAddresses(j), this);
                    sctpEV3<<__LINE__<<" get new path for "<<initchunk->getAddresses(j)<<" ptr="<<path<<"\n";
                    for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
                    {
                        if (sctpMain->addRemoteAddress(this, (*k), initchunk->getAddresses(j))) {
                            this->remoteAddressList.push_back(initchunk->getAddresses(j));
                        }
                    }
                    sctpPathMap[path->remoteAddress] = path;
                    qCounter.roomTransQ[path->remoteAddress] = 0;
                    qCounter.bookedTransQ[path->remoteAddress] = 0;
                    qCounter.roomRetransQ[path->remoteAddress] = 0;
                }
            }
            SCTPPathMap::iterator ite = sctpPathMap.find(remoteAddr);
            if (ite==sctpPathMap.end())
            {
                SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
                sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<" ptr="<<path<<"\n";
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr] = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
            }
            if (initchunk->getHmacTypesArraySize() != 0)
            {
                state->peerAuth = true;
                for (uint32 j=0; j<initchunk->getChunkTypesArraySize(); j++)
                {
                    type = initchunk->getChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE)
                    {
                        state->peerChunkList.push_back(type);
                    }
                }
            }
            sctpEV3 << "number supported extensions:" << initchunk->getSepChunksArraySize() << "\n";
            if (initchunk->getSepChunksArraySize() > 0)
            {
                for (uint32 i=0; i<initchunk->getSepChunksArraySize(); i++)
                {
                    if (initchunk->getSepChunks(i) == STREAM_RESET)
                    {
                        state->peerStreamReset = true;
                        break;
                    }
                    if (initchunk->getSepChunks(i) == PKTDROP)
                    {
                        state->peerPktDrop = true;
                        sctpEV3 << "set peerPktDrop to true\n";
                        break;
                    }
                }
            }
            trans = performStateTransition(SCTP_E_RCV_INIT);
            if (trans) {
                sendInitAck(initchunk);
            }
        }
        else if (fsm->getState() == SCTP_S_CLOSED)
        {
            trans = performStateTransition(SCTP_E_RCV_INIT);
            if (trans) {
                sendInitAck(initchunk);
            }
        }
        else {
            trans = true;
        }
    }
    else if (fsm->getState() == SCTP_S_COOKIE_WAIT) //INIT-Collision
    {
        sctpEV3<<"INIT collision: send Init-Ack\n";
        if (initchunk->getHmacTypesArraySize() != 0) {
            state->peerAuth = true;
            if (state->peerChunkList.size() == 0) {
                for (uint32 j=0; j<initchunk->getChunkTypesArraySize(); j++)
                {
                    type = initchunk->getChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE) {
                        state->peerChunkList.push_back(type);
                    }
                }
            }
        }
        sendInitAck(initchunk);
        trans = true;
    }
    else if (fsm->getState() == SCTP_S_COOKIE_ECHOED || fsm->getState() == SCTP_S_ESTABLISHED)
    {
        // check, whether a new address has been added
        bool addressPresent = false;
        for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
        {
            if (initchunk->getAddresses(j).getType() == Address::IPv6)
                continue;
            for (AddressVector::iterator k=remoteAddressList.begin(); k!=remoteAddressList.end(); ++k)
            {
                if ((*k)==(initchunk->getAddresses(j)))
                {
                    addressPresent = true;
                    break;
                }
            }
            if (!addressPresent)
            {
                sendAbort();
                return true;
            }
        }
        sendInitAck(initchunk);
        trans = true;
    }
    else if (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)
        trans = true;
    printSctpPathMap();
    return trans;
}


bool SCTPAssociation::processInitAckArrived(SCTPInitAckChunk* initAckChunk)
{
    bool trans = false;
    uint16 type;

    if (fsm->getState() == SCTP_S_COOKIE_WAIT)
    {
        sctpEV3<<"State is COOKIE_WAIT, Cookie_Echo has to be sent\n";
        stopTimer(T1_InitTimer);
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
        trans = performStateTransition(SCTP_E_RCV_INIT_ACK);
        //delete state->initChunk; will be deleted when state ESTABLISHED is entered
        if (trans)
        {
            state->initialPrimaryPath = remoteAddr;
            state->setPrimaryPath(getPath(remoteAddr));
            initPeerTsn = initAckChunk->getInitTSN();
            localVTag = initAckChunk->getInitTag();
            state->gapList.setInitialCumAckTSN(initPeerTsn - 1);
            state->initialPeerRwnd = initAckChunk->getA_rwnd();
            state->peerRwnd = state->initialPeerRwnd;
            statisticsPeerRwnd->record(state->peerRwnd);
            if (initAckChunk->getMsg_rwnd()>0)
            {
                state->peerAllowsChunks = true;
                state->initialPeerMsgRwnd = initAckChunk->getMsg_rwnd();
                state->peerMsgRwnd = state->initialPeerMsgRwnd;
            }
            state->expectedStreamResetSequenceNumber = initPeerTsn;
            remoteAddressList.clear();
            numberOfRemoteAddresses = initAckChunk->getAddressesArraySize();
            sctpEV3<<"number of remote addresses in initAck="<<numberOfRemoteAddresses<<"\n";
            for (uint32 j=0; j<numberOfRemoteAddresses; j++)
            {
                if (initAckChunk->getAddresses(j).getType() == Address::IPv6)
                    continue;
                for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
                {
                    if (!((*k).isUnspecified()))
                    {
                        sctpEV3<<"addPath "<<initAckChunk->getAddresses(j)<<"\n";
                        if (sctpMain->addRemoteAddress(this, (*k), initAckChunk->getAddresses(j))) {
                            this->remoteAddressList.push_back(initAckChunk->getAddresses(j));
                            addPath(initAckChunk->getAddresses(j));
                        }
                    }
                }
            }
            SCTPPathMap::iterator ite = sctpPathMap.find(remoteAddr);
            if (ite==sctpPathMap.end())
            {
                sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<"\n";
                SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr] = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
            }
            inboundStreams = ((initAckChunk->getNoOutStreams()<inboundStreams)?initAckChunk->getNoOutStreams():inboundStreams);
            outboundStreams = ((initAckChunk->getNoInStreams()<outboundStreams)?initAckChunk->getNoInStreams():outboundStreams);
            (this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
            if (initAckChunk->getHmacTypesArraySize() != 0)
            {
                state->peerAuth = true;
                for (uint32 j=0; j<initAckChunk->getChunkTypesArraySize(); j++)
                {
                    type = initAckChunk->getChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE)
                    {
                        state->peerChunkList.push_back(type);
                    }
                }

            }
            if (initAckChunk->getSepChunksArraySize() > 0)
            {
                for (uint32 i=0; i<initAckChunk->getSepChunksArraySize(); i++)
                {
                    if (initAckChunk->getSepChunks(i) == STREAM_RESET)
                    {
                        state->peerStreamReset = true;
                        break;
                    }
                    if (initAckChunk->getSepChunks(i) == PKTDROP)
                    {
                        state->peerPktDrop = true;
                        break;
                    }
                }
            }
            sendCookieEcho(initAckChunk);
        }
        startTimer(T1_InitTimer, state->initRexmitTimeout);
    }
    else
        sctpEV3<<"State="<<fsm->getState()<<"\n";
    printSctpPathMap();
    return trans;
}



bool SCTPAssociation::processCookieEchoArrived(SCTPCookieEchoChunk* cookieEcho, Address addr)
{
    bool trans = false;
    SCTPCookie* cookie = check_and_cast<SCTPCookie*>(cookieEcho->getStateCookie());
    if (cookie->getCreationTime()+(int32)sctpMain->par("validCookieLifetime")<simTime())
    {
        sctpEV3<<"stale Cookie: sendAbort\n";
        sendAbort();
        delete cookie;
        return trans;
    }
    if (fsm->getState() == SCTP_S_CLOSED)
    {
        if (cookie->getLocalTag()!=localVTag || cookie->getPeerTag() != peerVTag)
        {
            bool same = true;
            for (int32 i=0; i<32; i++)
            {
                if (cookie->getLocalTieTag(i)!=state->localTieTag[i])
                {
                    same = false;
                    break;
                }
                if (cookie->getPeerTieTag(i)!=state->peerTieTag[i])
                {
                    same = false;
                    break;
                }
            }
            if (!same)
            {
                sendAbort();
                delete cookie;
                return trans;
            }
        }
        sctpEV3<<"State is CLOSED, Cookie_Ack has to be sent\n";
        trans = performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
        if (trans)
            sendCookieAck(addr); //send to address
    }
    else if (fsm->getState() == SCTP_S_ESTABLISHED || fsm->getState() == SCTP_S_COOKIE_WAIT || fsm->getState() == SCTP_S_COOKIE_ECHOED)
    {
        sctpEV3<<"State is not CLOSED, but COOKIE_ECHO received. Compare the Tags\n";
        // case A: Peer restarted, retrieve information from cookie
        if (cookie->getLocalTag()!=localVTag && cookie->getPeerTag() != peerVTag )
        {
            bool same = true;
            for (int32 i=0; i<32; i++)
            {
                if (cookie->getLocalTieTag(i)!=state->localTieTag[i])
                {
                    same = false;
                    break;
                }
                if (cookie->getPeerTieTag(i)!=state->peerTieTag[i])
                {
                    same = false;
                    break;
                }
            }
            if (same)
            {
                localVTag = cookie->getLocalTag();
                peerVTag = cookie->getPeerTag();
                sendCookieAck(addr);
            }
        }
        // case B: Setup collision, retrieve information from cookie
        else if (cookie->getPeerTag()==peerVTag && (cookie->getLocalTag()!=localVTag || cookie->getLocalTag()==0))
        {
            localVTag = cookie->getLocalTag();
            peerVTag = cookie->getPeerTag();
            performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
            sendCookieAck(addr);
        }
        else if (cookie->getPeerTag()==peerVTag && cookie->getLocalTag()==localVTag)
        {
            sendCookieAck(addr); //send to address src
        }
        trans = true;
    }
    else
    {
        sctpEV3<<"State="<<fsm->getState()<<"\n";
        trans = true;
    }
    delete cookie;
    return trans;
}

bool SCTPAssociation::processCookieAckArrived()
{
    bool trans = false;

    if (fsm->getState() == SCTP_S_COOKIE_ECHOED)
    {
        stopTimer(T1_InitTimer);
        trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
        if (state->cookieChunk->getCookieArraySize()==0)
        {
            delete state->cookieChunk->getStateCookie();
        }
        delete state->cookieChunk;
        return trans;
    }
    else
        sctpEV3<<"State="<<fsm->getState()<<"\n";

    return trans;
}

void SCTPAssociation::tsnWasReneged(SCTPDataVariables*       chunk,
                                                const SCTPPathVariables* sackPath,
                                                const int                    type)
{
    sctpEV3 << "TSN " << chunk->tsn << " has been reneged (type "
              << type << ")" << endl;
    unackChunk(chunk);
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }
    chunk->hasBeenReneged = true;
    chunk->gapReports = 1;
    if (!chunk->getLastDestinationPath()->T3_RtxTimer->isScheduled()) {
        startTimer(chunk->getLastDestinationPath()->T3_RtxTimer,
                      chunk->getLastDestinationPath()->pathRto);
    }
}




SCTPEventCode SCTPAssociation::processSackArrived(SCTPSackChunk* sackChunk)
{
    simtime_t            rttEstimation = MAXTIME;
    const uint64         sendBufferBeforeUpdate = state->sendBuffer;
    SCTPPathVariables*   path = getPath(remoteAddr);    // Path for *this* SACK!
    const uint64         arwnd = sackChunk->getA_rwnd();
    const uint32         tsna = sackChunk->getCumTsnAck();
    uint32               highestNewAck = tsna;   // Highest newly acked TSN
    const uint16         numDups = sackChunk->getNumDupTsns();
    SCTP::AssocStat*     assocStat = sctpMain->getAssocStat(assocId);
    bool                 dropFilledGap = false;
    const uint32         msgRwnd = sackChunk->getMsg_rwnd();

    // ====== Put information from SACK into GapList =========================
    SCTPGapList sackGapList;
    sackGapList.setInitialCumAckTSN(sackChunk->getCumTsnAck());
    uint32 lastTSN = sackChunk->getCumTsnAck();
    for (uint32 i = 0; i < sackChunk->getNumGaps(); i++) {
        uint32 tsn = sackChunk->getGapStart(i);
        assert(tsnLt(lastTSN + 1, tsn)); lastTSN = tsn;
        while (tsnLe(tsn, sackChunk->getGapStop(i))) {
            bool dummy;
            sackGapList.updateGapList(tsn, dummy, true);    // revokable TSN
            tsn++;
        }
        lastTSN = sackChunk->getGapStop(i);
    }
    assocStat->sumRGapRanges += ((sackChunk->getCumTsnAck() <= lastTSN) ?
            (uint64)(lastTSN - sackChunk->getCumTsnAck()) :
            (uint64)(sackChunk->getCumTsnAck() - lastTSN));
    if (sackChunk->getNrSubtractRGaps() == false) {
        lastTSN = sackChunk->getCumTsnAck();
        for (uint32 i = 0; i < sackChunk->getNumNrGaps(); i++) {
            uint32 tsn = sackChunk->getNrGapStart(i);
            assert(tsnLt(lastTSN + 1, tsn)); lastTSN = tsn;
            while (tsnLe(tsn, sackChunk->getNrGapStop(i))) {
                bool dummy;
                sackGapList.updateGapList(tsn, dummy, false);   // non-revokable TSN
                tsn++;
            }
            lastTSN = sackChunk->getNrGapStop(i);
        }
    }
    else {
        lastTSN = sackChunk->getCumTsnAck();
        for (uint32 i = 0; i < sackChunk->getNumNrGaps(); i++) {
            uint32 tsn = sackChunk->getNrGapStart(i);
            assert(tsnLt(lastTSN + 1, tsn)); lastTSN = tsn;
            while (tsnLe(tsn, sackChunk->getNrGapStop(i))) {
                if (sackGapList.tsnIsRevokable(tsn) == false) {
                    bool dummy;
                    sackGapList.updateGapList(tsn, dummy, false);   // non-revokable TSN
                }
                tsn++;
            }
            lastTSN = sackChunk->getNrGapStop(i);
        }
    }
    assocStat->sumNRGapRanges += (sackChunk->getCumTsnAck() <= lastTSN) ?
            (uint64)(lastTSN - sackChunk->getCumTsnAck()) :
            (uint64)(sackChunk->getCumTsnAck() - lastTSN);
    const uint16 numGaps = sackGapList.getNumGaps(SCTPGapList::GT_Any);


    // ====== Print some information =========================================
    sctpEV3 << "##### SACK Processing: TSNa=" << tsna << " #####" << endl;
    for (SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        sctpEV3 << "Path " << myPath->remoteAddress << ":\t"
                  << "outstanding="          << path->outstandingBytes << "\t"
                  << "T3scheduled="          << path->T3_RtxTimer->getArrivalTime() << " "
                  << (path->T3_RtxTimer->isScheduled() ? "[ok]" : "[NOT SCHEDULED]") << "\t"
                  << endl;
    }

    sctpEV3 << "Before processSackArrived for path " << path->remoteAddress
            << " with tsna=" << tsna << ":" << endl;

    // ====== SACK Sequence Number Check =====================================
    sctpEV3 << "SACK Seq Number = " << sackChunk->getSackSeqNum() << endl;
    if ( (state->checkSackSeqNumber == true) &&
            (sackChunk->getSackSeqNum() <= state->incomingSackSeqNum) ) {
        sctpEV3 << "Out-of-data SACK: " << sackChunk->getSackSeqNum()
                      << " < " << state->incomingSackSeqNum << endl;
        return SCTP_E_IGNORE;
    }
    state->incomingSackSeqNum = sackChunk->getSackSeqNum();

    // ====== Record statistics ==============================================
    numGapBlocks->record(numGaps);
    statisticsRevokableGapBlocksInLastSACK->record(sackGapList.getNumGaps(SCTPGapList::GT_Revokable));
    statisticsNonRevokableGapBlocksInLastSACK->record(sackGapList.getNumGaps(SCTPGapList::GT_NonRevokable));

    path->vectorPathAckedTSNCumAck->record(tsna);
    if (assocStat) {
        assocStat->numDups += numDups;
    }

    // ====== Initialize some variables ======================================
    for (SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        // T.D. 26.03.09: Remember outstanding bytes before this update
        // Values are necessary for updating the congestion window!
        myPath->outstandingBytesBeforeUpdate = myPath->outstandingBytes;     // copy from myPath, not from path!
        myPath->requiresRtx = false;
        myPath->lowestTSNRetransmitted = false;
        myPath->findLowestTSN = true;
        myPath->gapAckedChunksInLastSACK = 0;
        myPath->gapNRAckedChunksInLastSACK = 0;
        myPath->gapUnackedChunksInLastSACK = 0;
        myPath->newlyAckedBytes = 0;
        myPath->newCumAck = false;   // Check whether CumAck affects this path
        if (myPath == path) {
            myPath->lastAckTime = simTime();
        }
    }


    // ====== Zero Window Probing ============================================
    if ((state->peerAllowsChunks) && (msgRwnd > 0) && (state->zeroWindowProbing) ) {
        state->zeroWindowProbing = false;
    }
    if ( (state->zeroWindowProbing) && (arwnd > 0) ) {
        state->zeroWindowProbing = false;
    }


    // #######################################################################
    // #### Processing of CumAck                                          ####
    // #######################################################################

    if (tsnGt(tsna, state->lastTsnAck)) {    // Handle new CumAck
        sctpEV3 << "===== Handling new CumAck for TSN " << tsna << " =====" << endl;

        SCTPQueue::PayloadQueue::iterator pay;
        SCTPDataVariables* myChunk = retransmissionQ->getChunk(state->lastTsnAck + 1);
        if ( (myChunk != NULL) && (myChunk->wasPktDropped) &&
                (myChunk->getLastDestinationPath()->fastRecoveryActive) ) {
            dropFilledGap = true;
            sctpEV3 << "TSN " << myChunk->tsn << " filled gap" << endl;
        }

        // We have got new chunks acked, and our cum ack point is advanced ...
        // Depending on the parameter osbWithHeader ackedBytes are with or without the header bytes.
        // T.D. 23.03.09: path->newlyAckedBytes is updated in dequeueAckedChunks()!
        dequeueAckedChunks(tsna, path, rttEstimation); // chunks with tsn between lastTsnAck and tsna are removed from the transmissionQ and the retransmissionQ; outstandingBytes are decreased

        state->lastTsnAck = tsna;
        if (tsnGt(tsna, state->advancedPeerAckPoint)) {
            state->advancedPeerAckPoint = tsna;
            state->ackPointAdvanced = true;
        }
    }
    else if (tsnLt(tsna, state->lastTsnAck)) {
        sctpEV3 << "Stale CumAck (" << tsna << " < " << state->lastTsnAck << ")"
                  << endl;
        return SCTP_E_IGNORE;
    }


    // ====== Handle reneging ================================================
    if ((numGaps == 0) && (tsnLt(tsna, state->highestTsnAcked))) {
        // Reneging, type 0:
        // In a previous SACK, chunks up to highestTsnAcked have been acked.
        // This SACK contains a CumAck < highestTsnAcked
        //      => rereg TSNs from CumAck+1 to highestTsnAcked
        //      => new highestTsnAcked = CumAck
        sctpEV3 << "numGaps=0 && tsna " << tsna
                  << " < highestTsnAcked " << state->highestTsnAcked << endl;
        uint32 i = state->highestTsnAcked;
        while (i >= tsna + 1) {
            SCTPDataVariables* myChunk = retransmissionQ->getChunk(i);
            if (myChunk) {
                if (chunkHasBeenAcked(myChunk)) {
                    tsnWasReneged(myChunk, path, 0);
                }
            }
            i--;
        }
        state->highestTsnAcked = tsna;
    }


    // #######################################################################
    // #### Processing of GapAcks                                         ####
    // #######################################################################

    if ((numGaps > 0) && (!retransmissionQ->payloadQueue.empty()) ) {
        sctpEV3 << "===== Handling GapAcks after CumTSNAck " << tsna << " =====" << endl;
        sctpEV3 << "We got " << numGaps << " gap reports" << endl;

        // We got fragment reports... check for newly acked chunks.
        const uint32 queuedChunks = retransmissionQ->payloadQueue.size();
        sctpEV3 << "Number of chunks in retransmissionQ: " << queuedChunks
                  <<" highestGapStop: " << sackGapList.getGapStop(SCTPGapList::GT_Any, numGaps-1)
                  <<" highestTsnAcked: " << state->highestTsnAcked << endl;

        // ====== Handle reneging =============================================
        // highest gapStop smaller than highestTsnAcked: there might have been reneging
        if (tsnLt(sackGapList.getGapStop(SCTPGapList::GT_Any, numGaps-1), state->highestTsnAcked)) {
            // Reneging, type 2:
            // In a previous SACK, chunks up to highestTsnAcked have been acked.
            // This SACK contains a last gap ack < highestTsnAcked
            //      => rereg TSNs from last gap ack to highestTsnAcked
            //      => new highestTsnAcked = last gap ack
            uint32 i = state->highestTsnAcked;
            while (i >= sackGapList.getGapStop(SCTPGapList::GT_Any, numGaps - 1) + 1) {
                // ====== Looking up TSN in retransmission queue ================
                SCTPDataVariables* myChunk = retransmissionQ->getChunk(i);
                if (myChunk) {
                    if (chunkHasBeenAcked(myChunk)) {
                        sctpEV3 << "TSN " << i << " was found. It has been un-acked." << endl;
                        tsnWasReneged(myChunk, path, 2);
                        sctpEV3 << "highestTsnAcked now " << state->highestTsnAcked << endl;
                    }
                }
                else {
                    sctpEV3 << "TSN " << i << " not found in retransmissionQ" << endl;
                }
                i--;
            }
            state->highestTsnAcked = sackGapList.getGapStop(SCTPGapList::GT_Any, numGaps - 1);
        }

        // ====== Looking for changes in the gap reports ======================
        sctpEV3 << "Looking for changes in gap reports" << endl;
        for (int32 key = 0; key < numGaps; key++) {
            const uint32 lo = sackGapList.getGapStart(SCTPGapList::GT_Any, key);
            const uint32 hi = sackGapList.getGapStop(SCTPGapList::GT_Any, key);


            // ====== Iterate over TSNs in gap reports =========================
            sctpEV3 << "Examine TSNs between " << lo << " and " << hi << endl;
            for (uint32 pos = lo; pos <= hi; pos++) {
            	bool chunkFirstTime = true;
                SCTPDataVariables* myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                if (myChunk) {
                    if (chunkHasBeenAcked(myChunk) == false) {
                        SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
                        assert(myChunkLastPath != NULL);
                        // T.D. 02.02.2010: This chunk has been acked newly.
                        //                        Let's process this new acknowledgement!
                        handleChunkReportedAsAcked(highestNewAck, rttEstimation, myChunk,
                                path /* i.e. the SACK path for RTT measurement! */,
                                sackGapList.tsnIsNonRevokable(myChunk->tsn));
                    }
                    else {
                    }
                }

                // ====== R-acked chunk became NR-acked =========================
                else if (sackGapList.tsnIsNonRevokable(pos)) {
                    bool chunkFirstTime = false;
                    SCTPDataVariables* myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                    if (myChunk) {
                        // myChunk != NULL -> R-acked before, but not NR-acked
                        handleChunkReportedAsAcked(highestNewAck, rttEstimation, myChunk,
                                path /* i.e. the SACK path for RTT measurement! */,
                                sackGapList.tsnIsNonRevokable(myChunk->tsn));
                        // All NR-acked chunks have chunkMap->getChunk(pos) == NULL!
                    }
                }
            }
        }
        state->highestTsnAcked = sackGapList.getGapStop(SCTPGapList::GT_Any, numGaps-1);

        // ====== Examine chunks between the gap reports ======================
        // They might have to be retransmitted or they could have been removed
        uint32 lo = tsna;
        for (int32 key = 0; key < numGaps; key++) {
            const uint32 hi = sackGapList.getGapStart(SCTPGapList::GT_Any, key);
            for (uint32 pos = lo+1; pos <= hi - 1; pos++) {
                bool chunkFirstTime = true;
                SCTPDataVariables* myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                if (myChunk) {
                    handleChunkReportedAsMissing(sackChunk, highestNewAck, myChunk,
                                                          path /* i.e. the SACK path for RTT measurement! */);
                }
                else {
                    sctpEV3 << "TSN " << pos << " not found in retransmissionQ" << endl;
                }
            }
            lo = sackGapList.getGapStop(SCTPGapList::GT_Any, key);
        }


        // ====== Validity checks =============================================
        path->vectorPathAckedTSNGapAck->record(state->highestTsnAcked);
    }


    // ====== Buffer space may have been gained => tell application ==========
    if (sendBufferBeforeUpdate != state->sendBuffer) {
        generateSendQueueAbatedIndication(sendBufferBeforeUpdate - state->sendBuffer);
    }

    // ====== Update Fast Recovery status, according to SACK =================
    updateFastRecoveryStatus(state->lastTsnAck);

    // ====== Update RTT measurement for newly acked data chunks =============
    sctpEV3 << simTime() << ": SACK: rtt=" << rttEstimation
              << ", path=" << path->remoteAddress << endl;
    pmRttMeasurement(path, rttEstimation);

    // ====== Record statistics ==============================================
    for (SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        myPath->statisticsPathGapAckedChunksInLastSACK->record(myPath->gapAckedChunksInLastSACK);
        myPath->statisticsPathGapNRAckedChunksInLastSACK->record(myPath->gapNRAckedChunksInLastSACK);
        myPath->statisticsPathGapUnackedChunksInLastSACK->record(myPath->gapUnackedChunksInLastSACK);
    }


    // #######################################################################
    // #### Receiver Window Management                                    ####
    // #######################################################################

    const uint32 osb = getOutstandingBytes();
    if (state->bytesToAddPerPeerChunk > 0) {
        state->peerRwnd = arwnd - osb  - (state->outstandingMessages * state->bytesToAddPerPeerChunk);
    }
    else if (state->peerAllowsChunks) {
        state->peerMsgRwnd = msgRwnd - state->outstandingMessages;
        state->peerRwnd = arwnd - osb;
        if ((int32)(state->peerMsgRwnd) < 0) {
            state->peerMsgRwnd = 0;
        }
        if (state->peerMsgRwnd > state->initialPeerMsgRwnd) {
            state->peerMsgRwnd = state->initialPeerMsgRwnd;
        }
    }
    else {
        state->peerRwnd = arwnd - osb;
    }

    // position of statement changed 20.07.05 I.R.
    if ((int32)(state->peerRwnd) < 0) {
        state->peerRwnd = 0;
    }
    if (state->peerRwnd > state->initialPeerRwnd) {
        state->peerRwnd = state->initialPeerRwnd;
    }
    if (state->peerAllowsChunks && msgRwnd == 0) {
        state->peerWindowFull = true;
    }
    if (arwnd == 1 || state->peerRwnd < state->swsLimit || arwnd == 0) {
        sctpEV3 << "processSackArrived: arwnd=" << arwnd
                  << " state->peerRwnd=" << state->peerRwnd
                  << " set peerWindowFull" << endl;
        state->peerWindowFull = true;
    }
    else if ((state->peerAllowsChunks && msgRwnd > 0) || !state->peerAllowsChunks) {
        state->peerWindowFull = false;
        state->zeroWindowProbing = false;
    }
    advancePeerTsn();
    statisticsArwndInLastSACK->record(arwnd);
    statisticsPeerRwnd->record(state->peerRwnd);

    // ====== Need for zero-window probing? ==================================
    if (osb == 0) {
        if ((state->peerAllowsChunks && msgRwnd == 0) || arwnd == 0)
            state->zeroWindowProbing = true;
    }


    // #######################################################################
    // #### Congestion Window Management                                  ####
    // #######################################################################

    // ======= Update congestion window of each path =========================
    sctpEV3 << "Before ccUpdateBytesAcked: ";
    for (SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        const Address& myPathId = myPath->remoteAddress;


        if (myPath->newlyAckedBytes > 0) {
            // T.D. 07.10.2009: Only call ccUpdateBytesAcked() when there are
            //                        acked bytes on this path!
            const bool advanceWindow = myPath->newCumAck;

            sctpEV3 << simTime() << ":\tCC " << myPath->newlyAckedBytes
                    << " newly acked on path " << myPathId << ";"
                    << "\tdropFilledGap=" << ((dropFilledGap == true) ? "true" : "false")
                    << "\t->\tadvanceWindow=" << advanceWindow << endl;

            (this->*ccFunctions.ccUpdateBytesAcked)(myPath, myPath->newlyAckedBytes,
                    (advanceWindow && dropFilledGap) ? false :
                            advanceWindow);
        }
    }

    // ====== Update congestion windows on paths (handling decreases) ========
    sctpEV3 << "Before ccUpdateAfterSack with tsna=" << tsna << endl;
    // ccUpdateAfterSack() will iterate over all paths.
    (this->*ccFunctions.ccUpdateAfterSack)();


    // #######################################################################
    // #### Path Management                                               ####
    // #######################################################################

    // ====== Need to stop or restart T3 timer? ==============================
    for (SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        const Address& myPathId = myPath->remoteAddress;

        if (myPath->outstandingBytes == 0) {
            // T.D. 07.01.2010: Only stop T3 timer when there is nothing more to send on this path!
            if (qCounter.roomTransQ.find(myPath->remoteAddress)->second == 0) {
                // Stop T3 timer, if there are no more outstanding bytes.
                stopTimer(myPath->T3_RtxTimer);
            }
        }
        else if (myPath->newCumAck) {
            stopTimer(myPath->T3_RtxTimer);
            startTimer(myPath->T3_RtxTimer, myPath->pathRto);
        }
        else {
            /* Also restart T3 timer, when lowest TSN is rtx'ed */
            if (myPath->lowestTSNRetransmitted == true) {
                sctpEV3 << "Lowest TSN retransmitted => restart of T3 timer for path "
                            << myPathId << endl;
                stopTimer(myPath->T3_RtxTimer);
                startTimer(myPath->T3_RtxTimer, myPath->pathRto);
            }
        }

        // ====== Clear error counter if TSNs on path have been acked =========
        if (myPath->newlyAckedBytes > 0) {
            pmClearPathCounter(myPath);
        }
    }



    return SCTP_E_IGNORE;
}


void SCTPAssociation::handleChunkReportedAsAcked(uint32&                  highestNewAck,
                                                                 simtime_t&           rttEstimation,
                                                                 SCTPDataVariables* myChunk,
                                                                 SCTPPathVariables* sackPath,
                                                                 const bool         sackIsNonRevokable)
{
    SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
    if ( (myChunk->numberOfTransmissions == 1) &&
          (myChunk->hasBeenMoved == false) &&
          (myChunk->hasBeenReneged == false) ) {
        if (myChunkLastPath == sackPath) {
            const simtime_t timeDifference = simTime() - myChunk->sendTime;
            if ((timeDifference < rttEstimation) || (rttEstimation == -1.0)) {
                rttEstimation = timeDifference;
            }
            sctpEV3 << simTime()          << " processSackArrived: computed rtt time diff == "
                      << timeDifference << " for TSN "<< myChunk->tsn << endl;
        }
    }
    if ( (myChunk->hasBeenAbandoned == false) &&
          (myChunk->hasBeenReneged == false) &&
          (myChunk->hasBeenAcked == false) ) {
        sctpEV3 << simTime() << ": GapAcked TSN " << myChunk->tsn
                  << " on path " << myChunkLastPath->remoteAddress << endl;

        if (myChunk->tsn > highestNewAck) {
            highestNewAck = myChunk->tsn;
        }

        if (sackIsNonRevokable == true) {
            myChunkLastPath->gapAckedChunksInLastSACK++;
            myChunkLastPath->gapNRAckedChunksInLastSACK++;
        }
        else {
            myChunkLastPath->gapAckedChunksInLastSACK++;
        }
    }

    // ====== Non-Renegable SACK =============================================
    if (sackIsNonRevokable == true) {
        // NOTE: nonRenegablyAckChunk() will only work with ChunkMap,
        //       since the actual chunk object will be gone ...
        nonRenegablyAckChunk(myChunk, sackPath, rttEstimation,
                sctpMain->getAssocStat(assocId));
    }

    // ====== Renegable SACK =================================================
    else {
        renegablyAckChunk(myChunk, sackPath);
    }
}


void SCTPAssociation::handleChunkReportedAsMissing(const SCTPSackChunk*      sackChunk,
                                                                    const uint32                 highestNewAck,
                                                                    SCTPDataVariables*       myChunk,
                                                                    SCTPPathVariables* sackPath)
{
    SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
    sctpEV3 << "TSN " << myChunk->tsn << " is missing in gap reports (last "
              << myChunkLastPath->remoteAddress << ") ";
    if (!chunkHasBeenAcked(myChunk)) {
        sctpEV3 << "has not been acked, highestNewAck=" << highestNewAck
                  << " countsAsOutstanding=" << myChunk->countsAsOutstanding << endl;
        const uint32 chunkReportedAsMissing = (highestNewAck > myChunk->tsn) ? 1 : 0;
        if (chunkReportedAsMissing > 0) {
            myChunk->gapReports += chunkReportedAsMissing;
            myChunkLastPath->gapUnackedChunksInLastSACK++;

            if (myChunk->gapReports >= state->numGapReports) {
                bool fastRtx = false;
                switch (state->rtxMethod) {
                    case 0: // Only one Fast RTX after 3 Gap reports
                        fastRtx = ((myChunk->hasBeenFastRetransmitted == false) &&
                                   ((myChunk->numberOfRetransmissions == 0)));
                        break;
                    case 1: // Just 1 Fast RTX per RTT
                        fastRtx = ((myChunk->hasBeenFastRetransmitted == false) &&
                                (myChunk->numberOfRetransmissions == 0 ||
                                        (simTime() - myChunk->sendTime) > myChunkLastPath->srtt));
                        break;
                    case 2: // Switch off Fast RTX
                        fastRtx = false;
                        break;
                    case 3: // Always Fast RTX
                        fastRtx = true;
                        break;
                }
                if (fastRtx) {

                    // ====== Add chunk to transmission queue ========
                    if (transmissionQ->getChunk(myChunk->tsn) == NULL) {
                        if (!chunkMustBeAbandoned(myChunk, sackPath)) {
                            SCTP::AssocStat* assocStat = sctpMain->getAssocStat(assocId);
                            if (assocStat) {
                                assocStat->numFastRtx++;
                            }
                        }
                        myChunk->hasBeenFastRetransmitted = true;
                        myChunk->sendForwardIfAbandoned = true;

                        sctpEV3 << simTime() << ": Fast RTX for TSN "
                                  << myChunk->tsn << " on path " << myChunk->getLastDestination() << endl;
                        myChunkLastPath->numberOfFastRetransmissions++;

                        myChunk->setNextDestination(getNextDestination(myChunk));
                        SCTPPathVariables* myChunkNextPath = myChunk->getNextDestinationPath();
                        assert(myChunkNextPath != NULL);

                        if (myChunk->countsAsOutstanding) {
                            decreaseOutstandingBytes(myChunk);
                        }
                        if (!transmissionQ->checkAndInsertChunk(myChunk->tsn, myChunk)) {
                            sctpEV3 << "Fast RTX: cannot add message/chunk (TSN="
                                      << myChunk->tsn << ") to the transmissionQ" << endl;
                        }
                        else    {
                            myChunk->enqueuedInTransmissionQ = true;
                            CounterMap::iterator q = qCounter.roomTransQ.find(myChunk->getNextDestination());
                            q->second += ADD_PADDING(myChunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                            CounterMap::iterator qb = qCounter.bookedTransQ.find(myChunk->getNextDestination());
                            qb->second += myChunk->booksize;
                        }
                        myChunkNextPath->requiresRtx = true;
                        if (myChunkNextPath->findLowestTSN == true) {
                            // TD 08.12.09: fixed detection of lowest TSN retransmitted
                            myChunkNextPath->lowestTSNRetransmitted = true;
                        }
                    }
                }
            }
        }
        myChunkLastPath->findLowestTSN = false;
    }
    else {
        // Reneging, type 1:
        // A chunk in the gap blocks has been un-acked => reneg it.
        tsnWasReneged(myChunk, sackPath, 1);
    }
}


void SCTPAssociation::nonRenegablyAckChunk(SCTPDataVariables* chunk,
        SCTPPathVariables* sackPath,
        simtime_t&         rttEstimation,
        SCTP::AssocStat*   assocStat)
{
    SCTPPathVariables* lastPath = chunk->getLastDestinationPath();
    assert(lastPath != NULL);

    // ====== Bookkeeping ====================================================
    // Subtract chunk size from sender buffer size
    state->sendBuffer -= chunk->len/8;

    // Subtract chunk size from the queue size of its stream
    SCTPSendStreamMap::iterator streamIterator = sendStreams.find(chunk->sid);
    assert(streamIterator != sendStreams.end());
    SCTPSendStream* stream = streamIterator->second;
    assert(stream != NULL);
    cPacketQueue* streamQ = (chunk->ordered == false) ? stream->getUnorderedStreamQ() : stream->getStreamQ();
    assert(streamQ != NULL);

    if (chunk->priority > 0) {
        state->queuedDroppableBytes -= chunk->len/8;
    }

    if ( (chunk->hasBeenCountedAsNewlyAcked == false) &&
         (chunk->hasBeenAcked == false) ) {
            chunk->hasBeenCountedAsNewlyAcked = true;
            // The chunk has not been acked before.
            // Therefore, its size may *once* be counted as newly acked.
            lastPath->newlyAckedBytes += chunk->booksize;
    }

    assert(chunk->queuedOnPath->queuedBytes >= chunk->booksize);
    chunk->queuedOnPath->queuedBytes -= chunk->booksize;
    chunk->queuedOnPath->statisticsPathQueuedSentBytes->record(chunk->queuedOnPath->queuedBytes);
    chunk->queuedOnPath = NULL;

    assert(state->queuedSentBytes >= chunk->booksize);
    state->queuedSentBytes -= chunk->booksize;
    statisticsQueuedSentBytes->record(state->queuedSentBytes);
    if (assocStat) {
        assocStat->ackedBytes += chunk->len/8;
    }
    if ( (assocStat) && (fairTimer)) {
        assocStat->fairAckedBytes += chunk->len/8;
    }

    // ====== RTT calculation ================================================
    if ((chunkHasBeenAcked(chunk) == false) && (chunk->countsAsOutstanding)) {
        if ((chunk->numberOfTransmissions == 1) && (lastPath == sackPath) && (chunk->hasBeenMoved == false)) {
            const simtime_t timeDifference = simTime() - chunk->sendTime;
            if ((timeDifference < rttEstimation) || (rttEstimation == MAXTIME)) {
                rttEstimation = timeDifference;
            }
        }
        decreaseOutstandingBytes(chunk);
    }

    // ====== Remove chunk pointer from ChunkMap =============================
    // The chunk still has to be remembered as acknowledged!
    ackChunk(chunk, sackPath);

    // ====== Remove chunk from transmission queue ===========================
    // Dequeue chunk, cause it has been acked
    if (transmissionQ->getChunk(chunk->tsn)) {
        transmissionQ->removeMsg(chunk->tsn);
        chunk->enqueuedInTransmissionQ = false;
        CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
        CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
        qb->second -= chunk->booksize;
    }

    // ====== Remove chunk from retransmission queue =========================
    chunk = retransmissionQ->getAndExtractChunk(chunk->tsn);
    if (chunk->userData != NULL) {
        delete chunk->userData;
    }
    delete chunk;
}


void SCTPAssociation::renegablyAckChunk(SCTPDataVariables* chunk,
        SCTPPathVariables* sackPath)
{
    // ====== Bookkeeping ====================================================
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }

    if ( (chunk->hasBeenCountedAsNewlyAcked == false) &&
            (chunk->hasBeenAcked == false) ) {
           chunk->hasBeenCountedAsNewlyAcked = true;
           // The chunk has not been acked before.
           // Therefore, its size may *once* be counted as newly acked.
           chunk->getLastDestinationPath()->newlyAckedBytes += chunk->booksize;
    }

    // ====== Acknowledge chunk =============================================
    ackChunk(chunk, sackPath);
    chunk->gapReports = 0;

    // ====== Remove chunk from transmission queue ===========================
    if (transmissionQ->getChunk(chunk->tsn)) {   // I.R. 02.01.2007
        sctpEV3 << "Found TSN " << chunk->tsn << " in transmissionQ -> remote it" << endl;
        transmissionQ->removeMsg(chunk->tsn);
        chunk->enqueuedInTransmissionQ = false;
        CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
        CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
        qb->second -= chunk->booksize;
    }
}


void SCTPAssociation::generateSendQueueAbatedIndication(const uint64 bytes)
{
    if (state->sendBuffer < state->sendQueueLimit) {
        // Just send SCTP_I_SENDQUEUE_ABATED once, after all newly acked
        // chunks have been dequeued.
        // Only send indication if the sendBuffer size has dropped below the sendQueueLimit
        // assert(state->lastSendQueueAbated < simTime());
        state->appSendAllowed = true;
        sctpEV3 << simTime() << ":\tSCTP_I_SENDQUEUE_ABATED("
                << bytes << ") to refill buffer "
                << state->sendBuffer << "/" << state->sendQueueLimit << endl;

        cPacket* msg = new cPacket(indicationName(SCTP_I_SENDQUEUE_ABATED));
        msg->setKind(SCTP_I_SENDQUEUE_ABATED);

        SCTPSendQueueAbated* sendQueueAbatedIndication =
                new SCTPSendQueueAbated(indicationName(SCTP_I_SENDQUEUE_ABATED));
        sendQueueAbatedIndication->setAssocId(assocId);
        sendQueueAbatedIndication->setLocalAddr(localAddr);
        sendQueueAbatedIndication->setRemoteAddr(remoteAddr);
        sendQueueAbatedIndication->setNumMsgs(bytes);   // NOTE: Legacy API!

        sendQueueAbatedIndication->setQueuedForStreamArraySize(sendStreams.size());
        unsigned int streamID = 0;
        for (SCTPSendStreamMap::iterator iterator = sendStreams.begin();
                iterator != sendStreams.end(); ++iterator) {
            const SCTPSendStream* stream = iterator->second;
            sendQueueAbatedIndication->setQueuedForStream(streamID, stream->getUnorderedStreamQ()->getByteLength()+stream->getStreamQ()->getByteLength());
            streamID++;
        }

        sendQueueAbatedIndication->setBytesAvailable(state->sendQueueLimit - state->sendBuffer);
        sendQueueAbatedIndication->setBytesQueued(state->sendBuffer);
        sendQueueAbatedIndication->setBytesLimit(state->sendQueueLimit);

        msg->setControlInfo(sendQueueAbatedIndication);
        sctpMain->sendSync(msg, "to_appl", appGateIndex);

        state->lastSendQueueAbated = simTime();
    }
}


void SCTPAssociation::dequeueAckedChunks(const uint32       tsna,
                                                         SCTPPathVariables* sackPath,
                                                         simtime_t&           rttEstimation)
{
    SCTP::AssocStat* assocStat = sctpMain->getAssocStat(assocId);

    // Set it ridiculously high
    rttEstimation = MAXTIME;

    // Are there chunks in the retransmission queue ? If Yes -> check for dequeue.
    SCTPQueue::PayloadQueue::iterator iterator = retransmissionQ->payloadQueue.begin();
    while (iterator != retransmissionQ->payloadQueue.end()) {
        SCTPDataVariables* chunk = iterator->second;
        if (tsnGe(tsna, chunk->tsn)) {
            sctpEV3 << simTime() << ": CumAcked TSN " << chunk->tsn
                    << " on path " << chunk->getLastDestination() << endl;

            if (!chunkHasBeenAcked(chunk)) {
                SCTPPathVariables* lastPath = chunk->getLastDestinationPath();
                // T.D. 05.12.09: CumAck affects lastPath -> reset its T3 timer later.
                lastPath->newCumAck = true;
            }

            nonRenegablyAckChunk(chunk, sackPath, rttEstimation, assocStat);
        }
        else {
            break;
        }
        iterator = retransmissionQ->payloadQueue.begin();
    }

    sctpEV3 << "dequeueAckedChunks(): rttEstimation=" << rttEstimation << endl;
}


SCTPEventCode SCTPAssociation::processForwardTsnArrived(SCTPForwardTsnChunk* fwChunk)
{
    sctpEV3 << "processForwardTsnArrived\n";
    sctpEV3 << "last state->cTsnAck=" << state->gapList.getCumAckTSN() << " fwCumAck=" << fwChunk->getNewCumTsn() << "\n";

    /* Ignore old FORWARD_TSNs, probably stale retransmits. */
    if (state->gapList.getCumAckTSN() >= fwChunk->getNewCumTsn()) {
        return SCTP_E_IGNORE;
    }

    for (uint32 i = 0; i < fwChunk->getSidArraySize(); i++) {
        if (fwChunk->getSsn(i) != -1) {
            SCTPReceiveStreamMap::iterator iter = receiveStreams.find(fwChunk->getSid(i));
            SCTPReceiveStream* rStream = iter->second;

            /* Uncomment the folloing to drop gap-acknowledged messages
             * between two abandonend messages rather then delivering them.
             */
          
            if (rStream->getOrderedQ()->getQueueSize() > 0)
                rStream->setExpectedStreamSeqNum(rStream->getOrderedQ()->getFirstSsnInQueue(fwChunk->getSid(i)));
            else if (rStream->getExpectedStreamSeqNum() <= fwChunk->getSsn(i))
                rStream->setExpectedStreamSeqNum(fwChunk->getSsn(i)+1);
            if (rStream->getExpectedStreamSeqNum() > 65535) {
                rStream->setExpectedStreamSeqNum(0);
            }
            sendDataArrivedNotification(fwChunk->getSid(i));
            calculateRcvBuffer();
        }
    }
    /* Update Gap lists with abandoned TSNs and advance CumTSNAck */
    for (uint32 i = state->gapList.getCumAckTSN() + 1; i <= fwChunk->getNewCumTsn(); i++) {
        if (i > state->gapList.getCumAckTSN() && !state->gapList.tsnInGapList(i)) {
            bool dummy;
            state->gapList.updateGapList(i, dummy, false);
            state->gapList.tryToAdvanceCumAckTSN();
        }
    }
    return SCTP_E_IGNORE;
}

SCTPEventCode SCTPAssociation::processDataArrived(SCTPDataChunk* dataChunk)
{
    const uint32         tsn = dataChunk->getTsn();
    SCTPPathVariables* path = getPath(remoteAddr);

    state->newChunkReceived = false;
    state->lastTsnReceived = tsn;

    bool found = false;
    for (std::list<SCTPPathVariables*>::iterator iterator = state->lastDataSourceList.begin();
            iterator != state->lastDataSourceList.end(); iterator++) {
        if (*iterator == path) {
            found = true;
            break;
        }
    }
    if (!found) {
        state->lastDataSourceList.push_back(path);
    }
    state->lastDataSourcePath = path;

    sctpEV3 << simTime() << " SCTPAssociation::processDataArrived TSN=" << tsn << endl;
    path->vectorPathReceivedTSN->record(tsn);
    if (dataChunk->getIBit()) {
        state->ackState = sackFrequency;
    }
    calculateRcvBuffer();

    SCTPSimpleMessage* smsg = check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
    dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
    dataChunk->encapsulate(smsg);
    const uint32 payloadLength = dataChunk->getByteLength()-SCTP_DATA_CHUNK_LENGTH;
    sctpEV3 << "state->bytesRcvd=" << state->bytesRcvd << endl;
    state->bytesRcvd += payloadLength;
    sctpEV3 << "state->bytesRcvd now=" << state->bytesRcvd << endl;
    path->numberOfBytesReceived += payloadLength;
    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
    iter->second.rcvdBytes += dataChunk->getByteLength()-SCTP_DATA_CHUNK_LENGTH;

    if (state->stopReceiving) {
        return SCTP_E_IGNORE;
    }

    // ====== Duplicate: tsn < CumAckTSN =====================================
    if (tsnLe(tsn, state->gapList.getCumAckTSN())) {
        if (state->stopOldData) {
            if (tsnGe(tsn, state->peerTsnAfterReset)) {
                state->stopOldData = false;
            }
            return SCTP_E_IGNORE;
        }
        else {
            sctpEV3 << simTime() << ": Duplicate TSN " << tsn << " (smaller than CumAck)" << endl;
            state->dupList.push_back(tsn);
            state->dupList.unique();
            path->numberOfDuplicates++;
            delete check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
            return SCTP_E_DUP_RECEIVED;
        }
    }

    // ====== Duplicate ======================================================
    if (tsnIsDuplicate(tsn)) {
        // TSN value is duplicate within a fragment
        sctpEV3 << "Duplicate TSN " << tsn << " (copy)" << endl;
        state->dupList.push_back(tsn);
        state->dupList.unique();
        path->numberOfDuplicates++;
        return SCTP_E_IGNORE;
    }

    // ====== Out of receiver buffer space? ==================================
    calculateRcvBuffer();
    if ( ((state->messageAcceptLimit > 0) &&
            (state->localMsgRwnd-state->bufferedMessages <= 0)) ||
            ((state->messageAcceptLimit == 0) &&
                    ((int32)(state->localRwnd-state->queuedReceivedBytes -
                            state->bufferedMessages * state->bytesToAddPerRcvdChunk) <= 0)) ) {
        state->ackState = sackFrequency;

        if (tsnGt(tsn, state->gapList.getHighestTSNReceived())) {
            sctpEV3 << "DROP: " <<  (int)tsn << " high=" << (int)state->gapList.getHighestTSNReceived() 
                << " Q=" << (int)state->queuedReceivedBytes << " Rwnd=" << (int)state->localRwnd << endl;
            if ((!state->pktDropSent) && (sctpMain->pktdrop) && (state->peerPktDrop)) {
                sctpEV3 << "Receive buffer full (case 1): sendPacketDrop" << endl;
                sendPacketDrop(false);
            }
            iter->second.numDropsBecauseNewTSNGreaterThanHighestTSN++;
            return SCTP_E_IGNORE;
            //          }  ????
        }
        else if ((tsn < state->gapList.getHighestTSNReceived()) &&
                (state->disableReneging == false) &&
                (!makeRoomForTsn(tsn, dataChunk->getBitLength()-SCTP_DATA_CHUNK_LENGTH*8, dataChunk->getUBit()))) {
            if ((!state->pktDropSent) && (sctpMain->pktdrop) && (state->peerPktDrop)) {
                sctpEV3 << "Receive buffer full (case 2): sendPacketDrop" << endl;
                sendPacketDrop(false);
            }
            iter->second.numDropsBecauseNoRoomInBuffer++;
            return SCTP_E_IGNORE;
        }
    }


    // ====== Update of CumAckTSN ============================================
    state->gapList.updateGapList(tsn, state->newChunkReceived,
            (state->disableReneging == false) ? true : false);
    state->gapList.tryToAdvanceCumAckTSN();
    sctpEV3 << "cumAckTSN=" << state->gapList.getCumAckTSN()
                   << " highestTSNReceived=" << state->gapList.getHighestTSNReceived() << endl;


    // ====== Silly Window Syndrome Avoidance ================================
    if (state->swsAvoidanceInvoked) {
        // swsAvoidanceInvoked => schedule a SACK to be sent at once in this case
        sctpEV3 << "swsAvoidanceInvoked" << endl;
        state->ackState = sackFrequency;
    }


    // ====== Enqueue new chunk ==============================================
    SCTPEventCode event = SCTP_E_SEND;
    if (state->newChunkReceived) {
        SCTPReceiveStreamMap::iterator iter = receiveStreams.find(dataChunk->getSid());
        const int ret = iter->second->enqueueNewDataChunk(makeVarFromMsg(dataChunk));
        if (ret > 0) {
            state->queuedReceivedBytes += payloadLength;
            calculateRcvBuffer();

            event = SCTP_E_DELIVERED;
            if (ret < 3) {
                state->bufferedMessages++;
                sendDataArrivedNotification(dataChunk->getSid());
                putInDeliveryQ(dataChunk->getSid());
                if (simTime() > state->lastThroughputTime + 1) {
                    for (uint16 i = 0; i < inboundStreams; i++) {
                        streamThroughputVectors[i]->record(state->streamThroughput[i]  /
                                (simTime() - state->lastThroughputTime) / 1024);
                        state->streamThroughput[i] = 0;
                    }
                    state->lastThroughputTime = simTime();
                }
                state->streamThroughput[dataChunk->getSid()] += payloadLength;
            }
            calculateRcvBuffer();
        }
        state->newChunkReceived = false;
    }

    return (event);
}

SCTPEventCode SCTPAssociation::processHeartbeatAckArrived(SCTPHeartbeatAckChunk* hback,
                                                                             SCTPPathVariables*     path)
{
    path->numberOfHeartbeatAcksRcvd++;
    path->vectorPathRcvdHbAck->record(path->numberOfHeartbeatAcksRcvd);
    /* hb-ack goes to pathmanagement, reset error counters, stop timeout timer */
    const Address addr = hback->getRemoteAddr();
    const simtime_t hbTimeField = hback->getTimeField();
    stopTimer(path->HeartbeatTimer);
    /* assume a valid RTT measurement on this path */
    simtime_t rttEstimation = simTime() - hbTimeField;
    pmRttMeasurement(path, rttEstimation);
    pmClearPathCounter(path);
    path->confirmed = true;
    path->lastAckTime = simTime();
    if (path->primaryPathCandidate == true) {
        state->setPrimaryPath(getPath(addr));
        path->primaryPathCandidate = false;
        if (path->pmtu < state->assocPmtu) {
            state->assocPmtu = path->pmtu;
        }
        path->ssthresh = state->peerRwnd;
        recordCwndUpdate(path);
        path->heartbeatTimeout = (double)sctpMain->par("hbInterval") + path->pathRto;
    }

    if (path->activePath == false) {
        sctpEV3 << "HB ACK arrived activePath=false. remoteAddress=" <<path->remoteAddress
                  << " initialPP=" << state->initialPrimaryPath << endl;
        path->activePath = true;
        if (state->reactivatePrimaryPath && path->remoteAddress == state->initialPrimaryPath) {
            state->setPrimaryPath(path);
        }
        sctpEV3 << "primaryPath now " << state->getPrimaryPathIndex() << endl;
    }
    sctpEV3 << "Received HB ACK chunk...resetting error counters on path " << addr
              <<", rttEstimation=" << rttEstimation << endl;
    path->pathErrorCount = 0;
    return SCTP_E_IGNORE;
}

void SCTPAssociation::processOutgoingResetRequestArrived(SCTPOutgoingSSNResetRequestParameter* requestParam)
{
    sctpEV3 << "processOutgoingResetRequestArrived\n";
    if (getPath(remoteAddr)->ResetTimer->isScheduled())
    {
        SCTPResetTimer* tm = check_and_cast<SCTPResetTimer*>(PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
        if (tm->getInSN() == requestParam->getSrResSn())
        {
            if (!tm->getInAcked() && tm->getOutAcked())
            {
                stopTimer(getPath(remoteAddr)->ResetTimer);
                delete state->resetChunk;
            }
            else
            {
                tm->setInAcked(true);
                PK(getPath(remoteAddr)->ResetTimer)->encapsulate(tm);
            }
        }
        delete tm;

    }
    if (tsnGt(requestParam->getLastTsn(), state->gapList.getHighestTSNReceived())) {
        state->lastTsnBeforeReset = requestParam->getLastTsn();
        state->peerRequestSn = requestParam->getSrReqSn();
    }
    else if (state->streamReset) {
        resetExpectedSsns();
        sctpEV3 << "processOutgoingResetRequestArrived: resetExpectedSsns\n";
        sendStreamResetResponse(requestParam->getSrReqSn());
    }
}

void SCTPAssociation::processIncomingResetRequestArrived(SCTPIncomingSSNResetRequestParameter* requestParam)
{
    sendOutgoingResetRequest(requestParam);
    sctpEV3 << "processIncomingResetRequestArrived: sendOutgoingResetRequestArrived returned\n";
    state->resetPending = true;
}

void SCTPAssociation::processSSNTSNResetRequestArrived(SCTPSSNTSNResetRequestParameter* requestParam)
{
    sctpEV3 << "processSSNTSNResetRequestArrived\n";
    state->advancedPeerAckPoint = state->nextTSN-1;
    state->stopOldData = true;
    sendStreamResetResponse(requestParam, true);
}


void SCTPAssociation::processResetResponseArrived(SCTPStreamResetResponseParameter* responseParam)
{
    sctpEV3 << "processResetResponseArrived \n";
    if (getPath(remoteAddr)->ResetTimer->isScheduled())
    {
        SCTPResetTimer* tm = check_and_cast<SCTPResetTimer*>(PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
        sctpEV3 << "SrResSn=" << responseParam->getSrResSn() << " tmOut=" << tm->getOutSN() << " tmIn= " << tm->getInSN() << "\n";
        if (tm->getOutSN() == responseParam->getSrResSn() || tm->getInSN() == responseParam->getSrResSn())
        {
            stopTimer(getPath(remoteAddr)->ResetTimer);
            delete state->resetChunk;
        }
        delete tm;
    }
    if (responseParam->getResult()==PERFORMED)
    {
        resetSsns();
        state->resetPending = false;
        if (responseParam->getReceiversNextTsn() != 0)
        {
            state->nextTSN = responseParam->getReceiversNextTsn();
            state->lastTsnAck = responseParam->getReceiversNextTsn() - 1;
            state->gapList.forwardCumAckTSN(responseParam->getSendersNextTsn() - 1);
            state->peerTsnAfterReset = responseParam->getSendersNextTsn();
            state->stopReceiving = false;
            state->stopOldData = true;
            sendSack();
        }
        sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numResetRequestsPerformed++;
    }
    else
    {
        sctpEV3 << "Reset Request failed. Send indication to app.\n";
        state->resetPending = false;
        sendIndicationToApp(SCTP_I_RESET_REQUEST_FAILED);
    }
}


SCTPEventCode SCTPAssociation::processInAndOutResetRequestArrived(SCTPIncomingSSNResetRequestParameter* inRequestParam,
        SCTPOutgoingSSNResetRequestParameter* outRequestParam)
{
    if (tsnGt(outRequestParam->getLastTsn(), state->gapList.getHighestTSNReceived())) {
        state->lastTsnBeforeReset = outRequestParam->getLastTsn();
        state->peerRequestSn = outRequestParam->getSrReqSn();
        state->inRequestSn = inRequestParam->getSrReqSn();
        state->inOut = true;
    }
    else {
        resetExpectedSsns();
        sctpEV3 << "processInAndOutResetRequestArrived: resetExpectedSsns\n";
        sendOutgoingRequestAndResponse(inRequestParam->getSrReqSn(), outRequestParam->getSrReqSn());
    }
    state->resetPending = true;
    return SCTP_E_IGNORE;
}


SCTPEventCode SCTPAssociation::processOutAndResponseArrived(SCTPOutgoingSSNResetRequestParameter* outRequestParam,
        SCTPStreamResetResponseParameter*     responseParam)
{
    sctpEV3 << "processOutAndResponseArrived\n";
    if (getPath(remoteAddr)->ResetTimer->isScheduled())
    {
        SCTPResetTimer* tm = check_and_cast<SCTPResetTimer*>(PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
        if (tm->getOutSN() == responseParam->getSrResSn() && tm->getInSN() == outRequestParam->getSrResSn()) {
            stopTimer(getPath(remoteAddr)->ResetTimer);
            delete state->resetChunk;
        }
        delete tm;
    }
    sendStreamResetResponse(outRequestParam->getSrReqSn());
    if (responseParam->getResult()==PERFORMED)
    {
        resetSsns();
        resetExpectedSsns();
        state->resetPending = false;
        sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numResetRequestsPerformed++;
    }
    else
    {
        sctpEV3 << "Reset Request failed. Send indication to app.\n";
        state->resetPending = false;
        sendIndicationToApp(SCTP_I_RESET_REQUEST_FAILED);
    }
    return SCTP_E_IGNORE;
}


SCTPEventCode SCTPAssociation::processStreamResetArrived(SCTPStreamResetChunk* resetChunk)
{
    SCTPParameter * parameter;
    bool requestReceived = false;
    uint32 numberOfParameters = resetChunk->getParametersArraySize();
    sctpEV3 << "processStreamResetArrived\n";

    parameter = (SCTPParameter*)(resetChunk->removeParameter());

    switch (parameter->getParameterType())
    {
        case OUTGOING_RESET_REQUEST_PARAMETER:
            SCTPOutgoingSSNResetRequestParameter* outRequestParam;
            outRequestParam = check_and_cast<SCTPOutgoingSSNResetRequestParameter*>(parameter);
            if (numberOfParameters == 1)
            {
                processOutgoingResetRequestArrived(outRequestParam);
                delete parameter;
            }
            else
            {
                parameter = (SCTPParameter*)(resetChunk->removeParameter());
                switch (parameter->getParameterType())
                {
                    case INCOMING_RESET_REQUEST_PARAMETER:
                        SCTPIncomingSSNResetRequestParameter* inRequestParam;
                        inRequestParam = check_and_cast<SCTPIncomingSSNResetRequestParameter*>(parameter);
                        processInAndOutResetRequestArrived(inRequestParam, outRequestParam);
                        delete inRequestParam;
                        delete outRequestParam;
                        break;
                    case STREAM_RESET_RESPONSE_PARAMETER:
                        SCTPStreamResetResponseParameter* responseParam;
                        responseParam = check_and_cast<SCTPStreamResetResponseParameter*>(parameter);
                        processOutAndResponseArrived(outRequestParam, responseParam);
                        delete responseParam;
                        delete outRequestParam;
                        break;
                }

            }
            requestReceived = true;
            break;
        case INCOMING_RESET_REQUEST_PARAMETER:
            SCTPIncomingSSNResetRequestParameter* inRequestParam;
            inRequestParam = check_and_cast<SCTPIncomingSSNResetRequestParameter*>(parameter);
            if (numberOfParameters == 1)
                processIncomingResetRequestArrived(inRequestParam);
            else
            {
                parameter = (SCTPParameter*)(resetChunk->removeParameter());
                if (parameter->getParameterType()==OUTGOING_RESET_REQUEST_PARAMETER)
                {
                    SCTPOutgoingSSNResetRequestParameter* outRequestParam;
                    outRequestParam = check_and_cast<SCTPOutgoingSSNResetRequestParameter*>(parameter);
                    processInAndOutResetRequestArrived(inRequestParam, outRequestParam);
                    delete outRequestParam;
                }

            }
            delete inRequestParam;
            requestReceived = true;
            break;
        case SSN_TSN_RESET_REQUEST_PARAMETER:
            SCTPSSNTSNResetRequestParameter* ssnRequestParam;
            ssnRequestParam = check_and_cast<SCTPSSNTSNResetRequestParameter*>(parameter);
            processSSNTSNResetRequestArrived(ssnRequestParam);
            requestReceived = true;
            delete ssnRequestParam;
            break;
        case STREAM_RESET_RESPONSE_PARAMETER:
            SCTPStreamResetResponseParameter* responseParam;
            responseParam = check_and_cast<SCTPStreamResetResponseParameter*>(parameter);
            if (numberOfParameters == 1)
            {
                processResetResponseArrived(responseParam);
            }
            else
            {
                parameter = (SCTPParameter*)(resetChunk->removeParameter());
                switch (parameter->getParameterType())
                {
                    case OUTGOING_RESET_REQUEST_PARAMETER:
                        SCTPOutgoingSSNResetRequestParameter* outRequestParam;
                        outRequestParam = check_and_cast<SCTPOutgoingSSNResetRequestParameter*>(parameter);
                        processOutAndResponseArrived(outRequestParam, responseParam);
                        requestReceived = true;
                        delete outRequestParam;
                        break;
                }
            }
            delete responseParam;
            break;
    }
    if (requestReceived)
        sendSack();

    return SCTP_E_IGNORE;
}


SCTPEventCode SCTPAssociation::processAsconfArrived(SCTPAsconfChunk* asconfChunk)
{
    SCTPParameter* sctpParam;
    SCTPPathVariables* path;
    Address addr;
    std::vector<Address> locAddr;
    SCTPAuthenticationChunk* authChunk;
    sctpEV3 << "Asconf arrived " << asconfChunk->getName() << "\n";
    SCTPMessage *sctpAsconfAck = new SCTPMessage("ASCONF_ACK");
    sctpAsconfAck->setBitLength(SCTP_COMMON_HEADER*8);
    sctpAsconfAck->setSrcPort(localPort);
    sctpAsconfAck->setDestPort(remotePort);
    if (state->auth && state->peerAuth)
    {
        authChunk = createAuthChunk();
        sctpAsconfAck->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    if (state->numberAsconfReceived > 0 || (state->numberAsconfReceived == 0 && asconfChunk->getSerialNumber() == initPeerTsn + state->numberAsconfReceived))
    {
        SCTPAsconfAckChunk* asconfAckChunk = createAsconfAckChunk(asconfChunk->getSerialNumber());
        state->numberAsconfReceived++;
        int32 count = asconfChunk->getAsconfParamsArraySize();
        sctpEV3 << "Number of Asconf parameters=" << count << "\n";
        for (int32 c=0; c<count; c++)
        {
            sctpParam = (SCTPParameter*)(asconfChunk->removeAsconfParam());
            switch (sctpParam->getParameterType())
            {
                case ADD_IP_ADDRESS:
                    sctpEV3 << "ADD_IP_PARAMETER\n";
                    SCTPAddIPParameter* ipParam;
                    ipParam = check_and_cast<SCTPAddIPParameter*>(sctpParam);
                    addr = ipParam->getAddressParam();
                    if (addr==Address("0.0.0.0"))
                    {
                        sctpEV3 << "no address specified, add natted address " << remoteAddr << "\n";
                        addr = remoteAddr;
                        sendIndicationToApp(SCTP_I_ADDRESS_ADDED);
                    }
                    for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
                    {
                        if (sctpMain->addRemoteAddress(this, (*k), addr)) {
                            addPath(addr);
                            sctpEV3 << "add remote address " << addr << " to local address " << (*k) << "\n";
                            this->remoteAddressList.push_back(addr);
                        }
                    }
                    path = getPath(addr);
                    if (state->enableHeartbeats)
                    {
                        stopTimer(path->HeartbeatTimer);
                        stopTimer(path->HeartbeatIntervalTimer);
                        path->statisticsPathRTO->record(path->pathRto);
                        startTimer(path->HeartbeatIntervalTimer, path->pathRto);
                        path->forceHb = true;
                    }
                    else
                        path->confirmed = true;
                    asconfAckChunk->addAsconfResponse(createSuccessIndication(ipParam->getRequestCorrelationId()));
                    delete ipParam;
                    break;
                case DELETE_IP_ADDRESS:
                    SCTPDeleteIPParameter* delParam;
                    delParam = check_and_cast<SCTPDeleteIPParameter*>(sctpParam);
                    addr = delParam->getAddressParam();
                    if (state->localAddresses.size() == 1)
                    {
                        SCTPErrorCauseParameter* errorParam;
                        errorParam = new SCTPErrorCauseParameter("ErrorCause");
                        errorParam->setParameterType(ERROR_CAUSE_INDICATION);
                        errorParam->setResponseCorrelationId(delParam->getRequestCorrelationId());
                        errorParam->setErrorCauseType(ERROR_DELETE_LAST_IP_ADDRESS);
                        errorParam->setBitLength((SCTP_ADD_IP_PARAMETER_LENGTH+4)*8);
                        errorParam->encapsulate((cPacket*)delParam->dup());
                        asconfAckChunk->addAsconfResponse(errorParam);
                    }
                    else if (addr == remoteAddr)
                    {
                        sctpEV3 << "addr=remoteAddr, make Error Parameter\n";
                        SCTPErrorCauseParameter* errParam;
                        errParam = new SCTPErrorCauseParameter("ErrorCause");
                        errParam->setParameterType(ERROR_CAUSE_INDICATION);
                        errParam->setResponseCorrelationId(delParam->getRequestCorrelationId());
                        errParam->setErrorCauseType(ERROR_DELETE_SOURCE_ADDRESS);
                        errParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH+4);
                        errParam->encapsulate((cPacket*)delParam->dup());
                        asconfAckChunk->addAsconfResponse(errParam);
                    }
                    else
                    {
                        locAddr = (std::vector<Address>) state->localAddresses;
                        sctpMain->removeRemoteAddressFromAllAssociations(this, addr, locAddr);
                        removePath(addr);
                        sctpEV3 << "remove path from address " << addr << "\n";
                        asconfAckChunk->addAsconfResponse(createSuccessIndication(delParam->getRequestCorrelationId()));
                    }
                    delete delParam;
                    break;
                case SET_PRIMARY_ADDRESS:
                    sctpEV3 << "SET_PRIMARY_ADDRESS\n";
                    SCTPSetPrimaryIPParameter* priParam;
                    priParam = check_and_cast<SCTPSetPrimaryIPParameter*>(sctpParam);
                    addr = priParam->getAddressParam();
                    if (addr==Address("0.0.0.0"))
                    {
                        sctpEV3 << "no address specified, add natted address " << remoteAddr << "\n";
                        addr = remoteAddr;
                    }
                    for (AddressVector::iterator i = remoteAddressList.begin(); i != remoteAddressList.end(); i++)
                    {
                        if ((*i) == addr)
                        {
                            if (getPath(addr)->confirmed == true)
                            {
                                state->setPrimaryPath(getPath(addr));
                                sctpEV3 << "set primaryPath to " << addr << "\n";
                            }
                            else
                            {
                                getPath(addr)->primaryPathCandidate = true;
                                //if (state->enableHeartbeats)
                                sendHeartbeat(getPath(addr));
                            }
                            break;
                        }
                    }
                    asconfAckChunk->addAsconfResponse(createSuccessIndication(priParam->getRequestCorrelationId()));
                    delete priParam;
                    break;
            }
        }
        sctpAsconfAck->addChunk(asconfAckChunk);
        sendToIP(sctpAsconfAck, remoteAddr);
		if (StartAddIP->isScheduled()) {
			stopTimer(StartAddIP);
			state->corrIdNum = state->asconfSn;
			const char* type = (const char *)sctpMain->par("addIpType");
			sendAsconf(type, false);
		}
    }
    return SCTP_E_IGNORE;
}

SCTPEventCode SCTPAssociation::processAsconfAckArrived(SCTPAsconfAckChunk* asconfAckChunk)
{
    SCTPParameter* sctpParam;
    Address addr;
    SCTPAsconfChunk *sctpasconf;
    std::vector<uint32> errorCorrId;
    std::vector<uint32>::iterator iter;
    bool errorFound = false;

    sctpasconf = check_and_cast<SCTPAsconfChunk*>(state->asconfChunk->dup());
    if (asconfAckChunk->getSerialNumber()==sctpasconf->getSerialNumber())
    {
        stopTimer(getPath(remoteAddr)->AsconfTimer);
        state->errorCount = 0;
        state->asconfOutstanding = false;
        getPath(remoteAddr)->pathErrorCount = 0;
        std::vector<Address> remAddr = (std::vector<Address>) remoteAddressList;
        for (uint32 j=0; j<asconfAckChunk->getAsconfResponseArraySize(); j++)
        {
            sctpParam = (SCTPParameter*)(asconfAckChunk->getAsconfResponse(j));
            if (sctpParam->getParameterType() == ERROR_CAUSE_INDICATION)
            {
                SCTPErrorCauseParameter* error = check_and_cast<SCTPErrorCauseParameter* >(sctpParam);
                errorCorrId.push_back(error->getResponseCorrelationId());
                sctpEV3 << "error added with id " << error->getResponseCorrelationId() << "\n";
            }
        }
        for (uint32 i=0; i<sctpasconf->getAsconfParamsArraySize(); i++)
        {
            sctpParam = check_and_cast<SCTPParameter*>(sctpasconf->removeAsconfParam());
            errorFound = false;
            switch (sctpParam->getParameterType())
            {
                case ADD_IP_ADDRESS:
                    SCTPAddIPParameter* ipParam;
                    ipParam = check_and_cast<SCTPAddIPParameter*>(sctpParam);
                    if (errorCorrId.size()>0)
                    {
                        for (iter = errorCorrId.begin(); iter != errorCorrId.end(); iter++)
                            if ((*iter) == ipParam->getRequestCorrelationId())
                            {
                                errorFound = true;
                                break;
                            }
                    }
                    if (errorFound==true) {
                        delete ipParam;
                        break;
                    }
                    addr = ipParam->getAddressParam();
                    if (addr==Address("0.0.0.0"))
                    {
                        addr = localAddr;
                        sendIndicationToApp(SCTP_I_ADDRESS_ADDED);
                    }
                    sctpMain->addLocalAddressToAllRemoteAddresses(this, addr, remAddr);
                    state->localAddresses.push_back(addr);
                    delete ipParam;
                    break;
                case DELETE_IP_ADDRESS:
                    SCTPDeleteIPParameter* delParam;
                    delParam = check_and_cast<SCTPDeleteIPParameter*>(sctpParam);
                    if (errorCorrId.size()>0)
                    {
                        for (iter = errorCorrId.begin(); iter != errorCorrId.end(); iter++)
                        {
                            if ((*iter) == delParam->getRequestCorrelationId())
                            {
                                errorFound = true;
                                break;
                            }
                        }
                    }
                    if (errorFound==true)
                    {
                        delete delParam;
                        break;
                    }
                    addr = delParam->getAddressParam();
                    sctpMain->removeLocalAddressFromAllRemoteAddresses(this, addr, remAddr);
                    for (AddressVector::iterator j=state->localAddresses.begin(); j!=state->localAddresses.end(); j++)
                    {
                        if ((*j)==addr)
                        {
                            sctpEV3 << "erase address " << (*j) << "\n";
                            state->localAddresses.erase(j);
                            break;
                        }
                    }
                    delete delParam;
                    break;
                case SET_PRIMARY_ADDRESS:
                    SCTPSetPrimaryIPParameter* priParam;
                    priParam = check_and_cast<SCTPSetPrimaryIPParameter*>(sctpParam);
                    if (errorCorrId.size()>0)
                    {
                        for (iter = errorCorrId.begin(); iter != errorCorrId.end(); iter++)
                        {
                            if ((*iter) == delParam->getRequestCorrelationId())
                            {
                                errorFound = true;
                                break;
                            }
                        }
                    }
                    if (errorFound==true) 
                    {
                        delete delParam;
                        break;
                    }
                    delete priParam;
                    break;
            }
        }
    }
    delete sctpasconf;
    return SCTP_E_IGNORE;
}

bool SCTPAssociation::processPacketDropArrived(SCTPPacketDropChunk* packetDropChunk)
{
    bool dataReceived = false;

    if (packetDropChunk->getMFlag() == false) {
        sctpEV3 << "processPacketDropArrived" << endl;
        if (packetDropChunk->getEncapsulatedPacket() != NULL) {
            SCTPMessage* sctpmsg = (SCTPMessage*)(packetDropChunk->decapsulate());
            const uint32 numberOfChunks = sctpmsg->getChunksArraySize();
            sctpEV3 << "numberOfChunks=" << numberOfChunks << endl;
            for (uint32 i = 0; i < numberOfChunks; i++) {
                SCTPChunk*  chunk = (SCTPChunk*)(sctpmsg->removeChunk());
                const uint8 type = chunk->getChunkType();
                switch (type) {
                    case DATA:
                    {
                        SCTPDataChunk* dataChunk = check_and_cast<SCTPDataChunk*>(chunk);
                        const uint32   tsn = dataChunk->getTsn();
                        SCTPQueue::PayloadQueue::iterator pq;
                        pq = retransmissionQ->payloadQueue.find(tsn);
                        if ( (pq != retransmissionQ->payloadQueue.end()) &&
                                (!chunkHasBeenAcked(pq->second)) ) {
                            sctpEV3 << simTime() << ": Packet Drop for TSN "
                                    << pq->second->tsn << " on path "
                                    << pq->second->getLastDestination()
                                    << " -> transmitting it again" << endl;
                            putInTransmissionQ(pq->first, pq->second);
                        }
                        delete dataChunk->decapsulate();
                        dataReceived = true;
                        break;
                    }
                    case SACK:
                        sendSack();
                        break;
                    case INIT:
                        stopTimer(T1_InitTimer);
                        retransmitInit();
                        startTimer(T1_InitTimer, state->initRexmitTimeout);
                        break;
                    case HEARTBEAT:
                        sendHeartbeat(getPath(remoteAddr));
                        break;
                    case HEARTBEAT_ACK:
                        break;
                    case SHUTDOWN:
                        stopTimer(T2_ShutdownTimer);
                        retransmitShutdown();
                        startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
                        break;
                    case SHUTDOWN_ACK:
                        stopTimer(T2_ShutdownTimer);
                        retransmitShutdownAck();
                        startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
                        break;
                    case COOKIE_ECHO:
                        stopTimer(T1_InitTimer);
                        retransmitCookieEcho();
                        startTimer(T1_InitTimer, state->initRexmitTimeout);
                        break;
                    case COOKIE_ACK:
                        sendCookieAck(remoteAddr);
                        break;
                    case ASCONF:
                        stopTimer(getPath(remoteAddr)->AsconfTimer);
                        retransmitAsconf();
                        startTimer(getPath(remoteAddr)->AsconfTimer, getPath(remoteAddr)->pathRto);
                        break;
                    case FORWARD_TSN:
                    {
                        if (peekAbandonedChunk(getPath(remoteAddr))!=NULL)
                        {
                            SCTPMessage* sctpmsg = new SCTPMessage();
                            sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
                            SCTPForwardTsnChunk* forwardChunk = createForwardTsnChunk(remoteAddr);
                            if (state->auth && state->peerAuth && typeInChunkList(FORWARD_TSN))
                            {
                                SCTPAuthenticationChunk* authChunk = createAuthChunk();
                                sctpmsg->addChunk(authChunk);
                            }
                            sctpmsg->addChunk(forwardChunk);
                        }
                        break;
                    }
                }
                delete chunk;
            }
            disposeOf(sctpmsg);
        }
        else {
            sctpEV3 << "no chunk encapsulated" << endl;
        }
        state->peerRwnd = packetDropChunk->getMaxRwnd() -
                packetDropChunk->getQueuedData() -
                getOutstandingBytes();
        statisticsPeerRwnd->record(state->peerRwnd);
        return dataReceived;
    }
    return false;
}


void SCTPAssociation::processErrorArrived(SCTPErrorChunk* errorChunk)
{
    uint32 parameterType;
    for (uint32 i=0; i<errorChunk->getParametersArraySize(); i++)
    {
        SCTPParameter* param = (SCTPParameter*)errorChunk->getParameters(i);
        parameterType = param->getParameterType();
        switch (parameterType)
        {
            case UNSUPPORTED_HMAC:
            {
                sendAbort();
                break;
            }
        }
    }
}

void SCTPAssociation::process_TIMEOUT_INIT_REXMIT(SCTPEventCode& event)
{
    if (++state->initRetransCounter > (int32)sctpMain->par("maxInitRetrans"))
    {
        sctpEV3 << "Retransmission count during connection setup exceeds " << (int32)sctpMain->par("maxInitRetrans") << ", giving up\n";
        sendIndicationToApp(SCTP_I_CLOSED);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }
    sctpEV3<< "Performing retransmission #" << state->initRetransCounter << "\n";
    switch (fsm->getState())
    {
        case SCTP_S_COOKIE_WAIT: retransmitInit(); break;
        case SCTP_S_COOKIE_ECHOED: retransmitCookieEcho(); break;
        default:
            throw cRuntimeError("Internal error: INIT-REXMIT timer expired while in state %s",
                    stateName(fsm->getState()));
    }
    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    startTimer(T1_InitTimer, state->initRexmitTimeout);
}

void SCTPAssociation::process_TIMEOUT_SHUTDOWN(SCTPEventCode& event)
{

    if (++state->errorCount > (uint32)sctpMain->par("assocMaxRetrans"))
    {
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }

    sctpEV3 << "Performing shutdown retransmission. Assoc error count now "<<state->errorCount<<" \n";
    if (fsm->getState() == SCTP_S_SHUTDOWN_SENT)
    {
        retransmitShutdown();
    }
    else if (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)
        retransmitShutdownAck();

    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
}


void SCTPAssociation::process_TIMEOUT_HEARTBEAT_INTERVAL(SCTPPathVariables* path, bool force)
{

    sctpEV3<<"HB Interval timer expired -- sending new HB REQ on path "<<path->remoteAddress<<"\n";
    /* restart hb_send_timer on this path */
    stopTimer(path->HeartbeatIntervalTimer);
    stopTimer(path->HeartbeatTimer);
    path->heartbeatIntervalTimeout = (double)sctpMain->par("hbInterval") +  path->pathRto;
    path->heartbeatTimeout = path->pathRto;
    startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);

    if (state->enableHeartbeats && (simTime() - path->lastAckTime > path->heartbeatIntervalTimeout/2 || path->forceHb || state->sendHeartbeatsOnActivePaths))
    {
        sendHeartbeat(path);
        startTimer(path->HeartbeatTimer, path->heartbeatTimeout);

        path->forceHb = false;
    }
}


void SCTPAssociation::process_TIMEOUT_HEARTBEAT(SCTPPathVariables* path)
{
    bool oldState;

    /* check if error counters must be increased */
    if (path->activePath)
    {
        state->errorCount++;
        path->pathErrorCount++;

        sctpEV3<<"HB timeout timer expired for path "<<path->remoteAddress<<" --> Increase Error Counters (Assoc: "<<state->errorCount<<", Path: "<<path->pathErrorCount<<")\n";
    }

    /* RTO must be doubled for this path ! */
    path->pathRto = (simtime_t)min(2 * path->pathRto.dbl(), sctpMain->par("rtoMax"));
    path->statisticsPathRTO->record(path->pathRto);
    /* check if any thresholds are exceeded, and if so, check if ULP must be notified */
    if (state->errorCount > (uint32)sctpMain->par("assocMaxRetrans"))
    {
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }
    else
    {
        /* set path state to INACTIVE, if the path error counter is exceeded */
        if (path->pathErrorCount > (uint32)sctpMain->par("pathMaxRetrans"))
        {
            oldState = path->activePath;
            path->activePath = false;
            if (path == state->getPrimaryPath()) {
                state->setPrimaryPath(getNextPath(path));
            }
            sctpEV3 << "pathErrorCount now "<< path->pathErrorCount
                      << "; PP now " << state->getPrimaryPathIndex() << endl;
        }
        /* then: we can check, if all paths are INACTIVE ! */
        if (allPathsInactive())
        {
            sctpEV3<<"sctp_do_hb_to_timer() : ALL PATHS INACTIVE --> closing ASSOC\n";
            sendIndicationToApp(SCTP_I_CONN_LOST);
            return;

        } else if (path->activePath == false && oldState == true)
        {
            /* notify the application, in case the PATH STATE has changed from ACTIVE to INACTIVE */
            pathStatusIndication(path, false);
        }

    }
}

void SCTPAssociation::stopTimers()
{
    for (SCTPPathMap::iterator j = sctpPathMap.begin(); j != sctpPathMap.end(); j++) {
        stopTimer(j->second->HeartbeatTimer);
        stopTimer(j->second->HeartbeatIntervalTimer);
    }
}

void SCTPAssociation::stopTimer(cMessage* timer)
{

    EV << "stopTimer " << timer->getName() << endl;
    if (timer->isScheduled()) {
        cancelEvent(timer);
    }
}

void SCTPAssociation::startTimer(cMessage* timer, const simtime_t& timeout)
{
    sctpEV3 << "startTimer " << timer->getName() << " with timeout "
              << timeout << " to expire at " << simTime() + timeout << endl;
    scheduleTimeout(timer, timeout);
}

void SCTPAssociation::process_TIMEOUT_RESET(SCTPPathVariables* path)
{
    int32 value;

    if ((value = updateCounters(path)) == 1)
    {
        sctpEV3 << "Performing timeout reset" << endl;
        retransmitReset();

        /* increase the RTO (by doubling it) */
        path->pathRto = min(2*path->pathRto.dbl(), sctpMain->par("rtoMax"));
        path->statisticsPathRTO->record(path->pathRto);
        startTimer(path->ResetTimer, path->pathRto);
    }
}


int32 SCTPAssociation::updateCounters(SCTPPathVariables* path)
{
    bool notifyUlp = false;
    if (++state->errorCount >= (uint32)sctpMain->par("assocMaxRetrans"))
    {
        sctpEV3 << "Retransmission count during connection setup exceeds " << (int32)sctpMain->par("assocMaxRetrans") << ", giving up\n";
        sendIndicationToApp(SCTP_I_CLOSED);
        sendAbort();
        sctpMain->removeAssociation(this);
        return 0;
    }
    else if (++path->pathErrorCount >= (uint32)sctpMain->par("pathMaxRetrans"))
    {
        if (path->activePath)
        {
            /* tell the source */
            notifyUlp = true;
        }

        path->activePath = false;
        if (path == state->getPrimaryPath()) {
            state->setPrimaryPath(getNextPath(path));
        }
        sctpEV3<<"process_TIMEOUT_RESET("<<(path->remoteAddress)<<") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE\n";
        if (allPathsInactive())
        {
            sctpEV3<<"process_TIMEOUT_RESET : ALL PATHS INACTIVE --> closing ASSOC\n";
            sendIndicationToApp(SCTP_I_CONN_LOST);
            sendAbort();
            sctpMain->removeAssociation(this);
            return 0;
        }
        else if (notifyUlp)
        {
            /* notify the application */
            pathStatusIndication(path, false);
        }
        sctpEV3<<"process_TIMEOUT_RESET("<<(path->remoteAddress)<<") : PATH ERROR COUNTER now "<<path->pathErrorCount<<"\n";
        return 2;
    }
    return 1;
}

void SCTPAssociation::process_TIMEOUT_ASCONF(SCTPPathVariables* path)
{
    int32 value;

    if ((value = updateCounters(path)) == 1)
    {
        retransmitAsconf();

        /* increase the RTO (by doubling it) */
        path->pathRto = min(2*path->pathRto.dbl(), sctpMain->par("rtoMax"));
        path->statisticsPathRTO->record(path->pathRto);

        startTimer(path->AsconfTimer, path->pathRto);
    }
}


void SCTPAssociation::process_TIMEOUT_RTX(SCTPPathVariables* path)
{
    sctpEV3 << "Processing retransmission timeout ..." << endl;

    // ====== Increase the RTO (by doubling it) ==============================
    path->pathRto = min(2 * path->pathRto.dbl(), sctpMain->par("rtoMax"));
    path->statisticsPathRTO->record(path->pathRto);
    sctpEV3 << "Schedule T3 based retransmission for path "<< path->remoteAddress << endl;
    if (SCTP::testing) {
        sctpEV3 << "Unacked chunks in Retransmission Queue:" << endl;
        for (SCTPQueue::PayloadQueue::const_iterator iterator = retransmissionQ->payloadQueue.begin();
                iterator != retransmissionQ->payloadQueue.end(); ++iterator) {
            const SCTPDataVariables* myChunk = iterator->second;
            if (!myChunk->hasBeenAcked) {
                const SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
                sctpEV3 << " - " << myChunk->tsn
                        << "\tsent=now-" << simTime() - myChunk->sendTime
                        << "\tlast=" << myChunkLastPath->remoteAddress
                        << "\tnumTX=" << myChunk->numberOfTransmissions
                        << "\tnumRTX=" << myChunk->numberOfRetransmissions
                        << "\tfastRTX=" << ((myChunk->hasBeenFastRetransmitted == true) ? "YES!" : "no")
                        << endl;
            }
        }
        sctpEV3 << "----------------------" << endl;
    }

    // ====== Update congestion window =======================================
    (this->*ccFunctions.ccUpdateAfterRtxTimeout)(path);


    // ====== Error Counter Handling =========================================
    if (!state->zeroWindowProbing) {
        state->errorCount++;
        path->pathErrorCount++;
        sctpEV3 << "RTX-Timeout: errorCount increased to "<<path->pathErrorCount<<"  state->errorCount="<<state->errorCount<<"\n";
    }
    if (state->errorCount >= (uint32)sctpMain->par("assocMaxRetrans")) {
        /* error counter exceeded terminate the association -- create an SCTPC_EV_CLOSE event and send it to myself */

        sctpEV3 << "process_TIMEOUT_RTX : ASSOC ERROR COUNTER EXCEEDED, closing ASSOC" << endl;
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }
    else {
        if (path->pathErrorCount > (uint32)sctpMain->par("pathMaxRetrans")) {
            bool notifyUlp = false;

            sctpEV3 << "pathErrorCount exceeded\n";
            if (path->activePath) {
                /* tell the source */
                notifyUlp = true;
            }
            path->activePath = false;
            if (path->remoteAddress == state->getPrimaryPathIndex()) {
                SCTPPathVariables* nextPath = getNextPath(path);
                if (nextPath != NULL) {
                    state->setPrimaryPath(nextPath);
                }
            }
            sctpEV3 << "process_TIMEOUT_RTX(" << path->remoteAddress
                      << ") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE" << endl;
            if (allPathsInactive()) {
                sctpEV3 << "process_TIMEOUT_RTX: ALL PATHS INACTIVE --> connection LOST!" << endl;
                sendIndicationToApp(SCTP_I_CONN_LOST);
                sendAbort();
                sctpMain->removeAssociation(this);
                return;
            }
            else if (notifyUlp) {
                // Send notification to the application
                pathStatusIndication(path, false);
            }
        }
        sctpEV3 << "process_TIMEOUT_RTX(" << path->remoteAddress
                  << ") : PATH ERROR COUNTER now " << path->pathErrorCount << endl;
    }


    // ====== Do Retransmission ==============================================
    // dequeue all chunks not acked so far and put them in the TransmissionQ
    if (!retransmissionQ->payloadQueue.empty()) {
        sctpEV3 << "Still " << retransmissionQ->payloadQueue.size()
                  << " chunks in retransmissionQ" << endl;

        for (SCTPQueue::PayloadQueue::iterator iterator = retransmissionQ->payloadQueue.begin();
            iterator != retransmissionQ->payloadQueue.end(); iterator++) {
            SCTPDataVariables* chunk = iterator->second;
            assert(chunk != NULL);

            // ====== Insert chunks into TransmissionQ ============================
            // Only insert chunks that were sent to the path that has timed out
            if (!chunkMustBeAbandoned(chunk, path) && ((chunkHasBeenAcked(chunk) == false && chunk->countsAsOutstanding)
                    || chunk->hasBeenReneged) && (chunk->getLastDestinationPath() == path)) {
                SCTPPathVariables* nextPath = getNextDestination(chunk);
                sctpEV3 << simTime() << ": Timer-Based RTX for TSN " << chunk->tsn
                        << ": lastDestination=" << chunk->getLastDestination()
                        << " lastPathRTO=" << chunk->getLastDestinationPath()->pathRto
                        << " nextDestination=" << nextPath->remoteAddress
                        << " nextPathRTO=" << nextPath->pathRto
                        << " waiting=" << simTime() - chunk->sendTime
                        << endl;
                nextPath->numberOfTimerBasedRetransmissions++;
                chunk->hasBeenTimerBasedRtxed = true;
                chunk->sendForwardIfAbandoned = true;

                if (!chunk->hasBeenAbandoned) {
                    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
                    iter->second.numT3Rtx++;
                }

                moveChunkToOtherPath(chunk, nextPath);
            }
        }
    }


    SCTPPathVariables* nextPath = getNextPath(path);
    sctpEV3 << "TimeoutRTX: sendOnAllPaths()" << endl;
    sendOnAllPaths(nextPath);

}


void SCTPAssociation::moveChunkToOtherPath(SCTPDataVariables* chunk,
                                                         SCTPPathVariables* newPath)
{
    // ======= Remove chunk from outstanding bytes ===========================
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }

    // ====== Prepare next destination =======================================
    chunk->hasBeenFastRetransmitted = false;
    chunk->gapReports = 0;
    chunk->setNextDestination(newPath);

    // ====== Rebook chunk on new path =======================================
    assert(chunk->queuedOnPath->queuedBytes >= chunk->booksize);
    chunk->queuedOnPath->queuedBytes -= chunk->booksize;
    chunk->queuedOnPath->statisticsPathQueuedSentBytes->record(chunk->queuedOnPath->queuedBytes);

    chunk->queuedOnPath = chunk->getNextDestinationPath();
    chunk->queuedOnPath->queuedBytes += chunk->booksize;
    chunk->queuedOnPath->statisticsPathQueuedSentBytes->record(chunk->queuedOnPath->queuedBytes);

    // ====== Perform bookkeeping ============================================
    // Check, if chunk_ptr->tsn is already in transmission queue.
    // This can happen in case multiple timeouts occur in succession.
    if (!transmissionQ->checkAndInsertChunk(chunk->tsn, chunk)) {
        sctpEV3 << "TSN " << chunk->tsn << " already in transmissionQ" << endl;
        return;
    }
    else {
        chunk->enqueuedInTransmissionQ = true;
        sctpEV3 << "Inserting TSN " << chunk->tsn << " into transmissionQ" << endl;
        CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second += ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
        CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
        qb->second += chunk->booksize;
        state->peerRwnd += (chunk->booksize + state->bytesToAddPerPeerChunk);
        if (state->peerAllowsChunks) {
            state->peerMsgRwnd++;
        }
    }
    if (state->peerRwnd > state->initialPeerRwnd) {
        state->peerRwnd = state->initialPeerRwnd;
    }
    if (state->peerAllowsChunks && state->peerMsgRwnd > state->initialPeerMsgRwnd) {
        state->peerMsgRwnd = state->initialPeerMsgRwnd;
    }

    // T.D. 02.08.2011: The peer window may not be full anymore!
    if ( (state->peerWindowFull) && (state->peerRwnd > 0) ) {
        state->peerWindowFull = false;
    }

    statisticsPeerRwnd->record(state->peerRwnd);
}
