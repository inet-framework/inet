//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"
#include "IPControlInfo_m.h"


void SCTPAssociation::decreaseOutstandingBytes(SCTPDataVariables* chunk)
{
    SCTPPathVariables* lastPath = chunk->getLastDestinationPath();

    assert(lastPath->outstandingBytes >= chunk->booksize);
    lastPath->outstandingBytes -= chunk->booksize;
    state->outstandingBytes -= chunk->booksize;
    assert((int64)state->outstandingBytes >= 0);

    chunk->countsAsOutstanding = false;

    CounterMap::iterator iterator = qCounter.roomRetransQ.find(lastPath->remoteAddress);
        iterator->second -= ADD_PADDING(chunk->booksize + SCTP_DATA_CHUNK_LENGTH);

}


bool SCTPAssociation::process_RCV_Message(SCTPMessage*       sctpmsg,
                                                        const IPvXAddress& src,
                                                        const IPvXAddress& dest)
{
    // ====== Header checks ==================================================
    sctpEV3 << getFullPath()  << " SCTPAssociationRcvMessage:process_RCV_Message"
              << " localAddr="  << localAddr
              << " remoteAddr=" << remoteAddr << endl;
    if ((sctpmsg->hasBitError() || !sctpmsg->getChecksumOk()))
    {
        if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType() == INIT_ACK) {
            stopTimer(T1_InitTimer);
            sctpEV3 << "InitAck with bit-error. Retransmit Init" << endl;
            retransmitInit();
            startTimer(T1_InitTimer,state->initRexmitTimeout);
        }
        if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType() == COOKIE_ACK) {
            stopTimer(T1_InitTimer);
            sctpEV3 << "CookieAck with bit-error. Retransmit CookieEcho" << endl;
            retransmitCookieEcho();
            startTimer(T1_InitTimer,state->initRexmitTimeout);
        }
    }

    SCTPPathVariables* path      = getPath(src);
    const uint16 srcPort         = sctpmsg->getDestPort();
    const uint16 destPort        = sctpmsg->getSrcPort();
    const uint32 numberOfChunks = sctpmsg->getChunksArraySize();
    sctpEV3 << "numberOfChunks=" <<numberOfChunks << endl;

    state->sctpmsg = (SCTPMessage*)sctpmsg->dup();

    // ====== Handle chunks ==================================================
    bool trans              = true;
    bool sendAllowed        = false;
    bool dupReceived        = false;
    bool dataChunkReceived  = false;
    bool dataChunkDelivered = false;
    bool shutdownCalled     = false;
    for (uint32 i = 0; i < numberOfChunks; i++) {
        const SCTPChunk* header = (const SCTPChunk*)(sctpmsg->removeChunk());
        const uint8       type  = header->getChunkType();

        if ((type != INIT) &&
             (type != ABORT) &&
             (type != ERRORTYPE) &&
             (sctpmsg->getTag() != peerVTag)) {
            sctpEV3 << " VTag "<< sctpmsg->getTag() << " incorrect. Should be "
                      << peerVTag << " localVTag=" << localVTag << endl;
            return true;
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
            trans = processCookieEchoArrived(cookieEchoChunk,src);
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
                        dataChunkReceived    = true;
                        dataChunkDelivered = true;
                        state->sackAllowed = true;
                    }
                    else if (event==SCTP_E_SEND || event==SCTP_E_IGNORE) {
                        dataChunkReceived    = true;
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
        {
            sctpEV3 << "SCTPAssociationRcvMessage: SACK received" << endl;
            const int32 scount = qCounter.roomSumSendStreams;
            SCTPSackChunk* sackChunk;
            sackChunk = check_and_cast<SCTPSackChunk*>(header);
            processSackArrived(sackChunk);
            trans           = true;
            sendAllowed = true;
            delete sackChunk;
            if (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && scount==0) {
                if (fsm->getState() == SCTP_S_SHUTDOWN_PENDING) {
                    sctpEV3 << "No more packets: send SHUTDOWN" << endl;
                    sendShutdown();
                    trans               = performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
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
                path->pathRcvdHb->record(path->numberOfHeartbeatsRcvd);
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
        default: sctpEV3<<"different type" << endl;
            break;
        }

        if (i==numberOfChunks-1 && (dataChunkReceived || dupReceived)) {
            sendAllowed=true;
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
    disposeOf(state->sctpmsg);
    return trans;
}

bool SCTPAssociation::processInitArrived(SCTPInitChunk* initchunk, int32 srcPort, int32 destPort)
{
    SCTPAssociation* assoc;
    char timerName[128];
    bool trans = false;
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
                qCounter.roomTransQ[rPath->remoteAddress]     = 0;
                qCounter.bookedTransQ[rPath->remoteAddress] = 0;
                qCounter.roomRetransQ[rPath->remoteAddress] = 0;
            }
            initPeerTsn=initchunk->getInitTSN();
            state->cTsnAck = initPeerTsn - 1;
            state->initialPeerRwnd = initchunk->getA_rwnd();
            state->peerRwnd = state->initialPeerRwnd;
            localVTag= initchunk->getInitTag();
            numberOfRemoteAddresses =initchunk->getAddressesArraySize();
            IInterfaceTable *ift = interfaceTableAccess.get();
            state->localAddresses.clear();
            if (localAddressList.front() == IPvXAddress("0.0.0.0"))
            {
                for (int32 i=0; i<ift->getNumInterfaces(); ++i)
                {
                    if (ift->getInterface(i)->ipv4Data()!=NULL)
                    {
                        adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
                    }
                    else if (ift->getInterface(i)->ipv6Data()!=NULL)
                    {
                        adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(0));
                    }
                }
            }
            else
            {
                adv = localAddressList;
            }
            uint32 rlevel = getLevel(remoteAddr);
            if (rlevel>0)
                for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
                {
                    if (getLevel((*i))>=rlevel)
                    {
                        sctpMain->addLocalAddress(this, (*i));
                        state->localAddresses.push_back((*i));
                    }
                }
            for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
            {
                // skip IPv6 because we can't send to them yet
                if (initchunk->getAddresses(j).isIPv6())
                    continue;
                // set path variables for this pathlocalAddresses
                if (!getPath(initchunk->getAddresses(j)))
                {
                    SCTPPathVariables* path = new SCTPPathVariables(initchunk->getAddresses(j), this);
                    sctpEV3<<__LINE__<<" get new path for "<<initchunk->getAddresses(j)<<" ptr="<<path<<"\n";
                    for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
                    {
                        sctpMain->addRemoteAddress(this,(*k), initchunk->getAddresses(j));
                        this->remoteAddressList.push_back(initchunk->getAddresses(j));
                    }
                    sctpPathMap[path->remoteAddress] = path;
                    qCounter.roomTransQ[path->remoteAddress]     = 0;
                    qCounter.bookedTransQ[path->remoteAddress] = 0;
                    qCounter.roomRetransQ[path->remoteAddress] = 0;
                }
            }
            SCTPPathMap::iterator ite=sctpPathMap.find(remoteAddr);
            if (ite==sctpPathMap.end())
            {
                SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
                sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<" ptr="<<path<<"\n";
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr]  = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
            }
            trans = performStateTransition(SCTP_E_RCV_INIT);
            if (trans) {
                sendInitAck(initchunk);
            }
        }
        else if (fsm->getState() == SCTP_S_CLOSED)
        {
            trans=performStateTransition(SCTP_E_RCV_INIT);
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
        sendInitAck(initchunk);
        trans=true;
    }
    else if (fsm->getState() == SCTP_S_COOKIE_ECHOED || fsm->getState() == SCTP_S_ESTABLISHED)
    {
        // check, whether a new address has been added
        bool addressPresent = false;
        for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
        {
            if (initchunk->getAddresses(j).isIPv6())
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
        trans=true;
    }
    else if (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)
        trans = true;
    printSctpPathMap();
    return trans;
}


bool SCTPAssociation::processInitAckArrived(SCTPInitAckChunk* initAckChunk)
{
    bool trans = false;
    if (fsm->getState() == SCTP_S_COOKIE_WAIT)
    {
        sctpEV3<<"State is COOKIE_WAIT, Cookie_Echo has to be sent\n";
        stopTimer(T1_InitTimer);
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
        trans=performStateTransition(SCTP_E_RCV_INIT_ACK);
        //delete state->initChunk; will be deleted when state ESTABLISHED is entered
        if (trans)
        {
            state->initialPrimaryPath = remoteAddr;
            state->setPrimaryPath(getPath(remoteAddr));
            initPeerTsn=initAckChunk->getInitTSN();
            localVTag= initAckChunk->getInitTag();
            state->cTsnAck = initPeerTsn - 1;
            state->initialPeerRwnd = initAckChunk->getA_rwnd();
            state->peerRwnd = state->initialPeerRwnd;
            remoteAddressList.clear();
            numberOfRemoteAddresses =initAckChunk->getAddressesArraySize();
            sctpEV3<<"number of remote addresses in initAck="<<numberOfRemoteAddresses<<"\n";
            for (uint32 j=0; j<numberOfRemoteAddresses; j++)
            {
                if (initAckChunk->getAddresses(j).isIPv6())
                    continue;
                for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
                {
                    if (!((*k).isUnspecified()))
                    {
                        sctpEV3<<"addPath "<<initAckChunk->getAddresses(j)<<"\n";
                        sctpMain->addRemoteAddress(this,(*k), initAckChunk->getAddresses(j));
                        this->remoteAddressList.push_back(initAckChunk->getAddresses(j));
                        addPath(initAckChunk->getAddresses(j));
                    }
                }
            }
            SCTPPathMap::iterator ite=sctpPathMap.find(remoteAddr);
            if (ite==sctpPathMap.end())
            {
                sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<"\n";
                SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr]  = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
            }
            inboundStreams   = ((initAckChunk->getNoOutStreams()<inboundStreams)?initAckChunk->getNoOutStreams():inboundStreams);
            outboundStreams = ((initAckChunk->getNoInStreams()<outboundStreams)?initAckChunk->getNoInStreams():outboundStreams);
            (this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
            sendCookieEcho(initAckChunk);
        }
        startTimer(T1_InitTimer, state->initRexmitTimeout);
    }
    else
        sctpEV3<<"State="<<fsm->getState()<<"\n";
    printSctpPathMap();
    return trans;
}



bool SCTPAssociation::processCookieEchoArrived(SCTPCookieEchoChunk* cookieEcho, IPvXAddress addr)
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
            bool same=true;
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
        trans=performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
        if (trans)
            sendCookieAck(addr);//send to address
    }
    else if (fsm->getState() == SCTP_S_ESTABLISHED || fsm->getState() == SCTP_S_COOKIE_WAIT || fsm->getState() == SCTP_S_COOKIE_ECHOED)
    {
        sctpEV3<<"State is not CLOSED, but COOKIE_ECHO received. Compare the Tags\n";
        // case A: Peer restarted, retrieve information from cookie
        if (cookie->getLocalTag()!=localVTag && cookie->getPeerTag() != peerVTag )
        {
            bool same=true;
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
        trans=true;
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
    bool trans=false;

    if (fsm->getState() == SCTP_S_COOKIE_ECHOED)
    {
        stopTimer(T1_InitTimer);
        trans=performStateTransition(SCTP_E_RCV_COOKIE_ACK);
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
                                                const int                    type)
{

#ifdef PKT
    if (transmissionQ->getQueueSize() > 0) {
        for (SCTPQueue::PayloadQueue::iterator tq = transmissionQ->payloadQueue.begin();
              tq != transmissionQ->payloadQueue.end(); tq++) {
            if (tq->second->tsn > chunk->tsn) {
                if (!transmissionQ->checkAndInsertChunk(chunk->tsn, chunk)) {
                    sctpEV3 << "tsnWasReneged: cannot add message/chunk (TSN="
                              << chunk->tsn <<") to the transmissionQ" << endl;
                }
                else {
                    chunk->enqueuedInTransmissionQ = true;
                    chunk->setNextDestination(chunk->getLastDestinationPath());
                    CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
                    q->second+=ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                    CounterMap::iterator qb=qCounter.bookedTransQ.find(chunk->getNextDestination());
                    qb->second+=chunk->booksize;
                    return;
                }
            }
        }
    }
#endif
    sctpEV3 << "TSN " << chunk->tsn << " has been reneged (type "
              << type << ")" << endl;
    unackChunk(chunk);
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }
    chunk->hasBeenReneged = true;
    chunk->gapReports        = 1;
    if (!chunk->getLastDestinationPath()->T3_RtxTimer->isScheduled()) {
        startTimer(chunk->getLastDestinationPath()->T3_RtxTimer,
                      chunk->getLastDestinationPath()->pathRto);
    }
}




SCTPEventCode SCTPAssociation::processSackArrived(SCTPSackChunk* sackChunk)
{
    simtime_t            rttEstimation            = MAXTIME;
    bool                     ctsnaAdvanced            = false;
    SCTPPathVariables* path                       = getPath(remoteAddr);    // Path for *this* SACK!
    const uint64         arwnd                    = sackChunk->getA_rwnd();
    const uint32         tsna                         = sackChunk->getCumTsnAck();
    uint32               highestNewAck            = tsna;   // Highest newly acked TSN
    const uint16         numGaps                      = sackChunk->getNumGaps();
    bool                     getChunkFastFirstTime = true;

    // ====== Print some information =========================================
    sctpEV3 << "##### SACK Processing: TSNa=" << tsna << " #####" << endl;
    for(SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        sctpEV3 << "Path " << myPath->remoteAddress << ":\t"
                  << "outstanding="          << path->outstandingBytes << "\t"
                  << "T3scheduled="          << path->T3_RtxTimer->getArrivalTime() << " "
                  << (path->T3_RtxTimer->isScheduled() ? "[ok]" : "[NOT SCHEDULED]") << "\t"
                  << endl;
    }



    // ====== Initialize some variables ======================================
    for(SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath = piter->second;
        // T.D. 26.03.09: Remember outstanding bytes before this update
        // Values are necessary for updating the congestion window!
        myPath->outstandingBytesBeforeUpdate = myPath->outstandingBytes;     // T.D. 14.11.09: Bugfix - copy from myPath, not from path!
        myPath->requiresRtx                      = false;
        myPath->lowestTSNRetransmitted       = false;
        myPath->findLowestTSN                    = true;
        myPath->gapAcksInLastSACK                = 0;
        myPath->gapNAcksInLastSACK               = 0;
        myPath->newlyAckedBytes                  = 0;
        if (myPath == path) {
            myPath->lastAckTime = simTime();
        }
    }


    // ====== Zero Window Probing ============================================
    if ( (state->zeroWindowProbing) && (arwnd > 0) ) {
        state->zeroWindowProbing = false;
    }


    // #######################################################################
    // #### Processing of CumAck                                                         ####
    // #######################################################################

    if (tsnGt(tsna, state->lastTsnAck)) {    // Handle new CumAck
        sctpEV3 << "===== Handling new CumAck for TSN " << tsna << " =====" << endl;


        // We have got new chunks acked, and our cum ack point is advanced ...
        // Depending on the parameter osbWithHeader ackedBytes are with or without the header bytes.
        // T.D. 23.03.09: path->newlyAckedBytes is updated in dequeueAckedChunks()!
        dequeueAckedChunks(tsna, path, rttEstimation); // chunks with tsn between lastTsnAck and tsna are removed from the transmissionQ and the retransmissionQ; outstandingBytes are decreased

        state->lastTsnAck = tsna;
        ctsnaAdvanced = true;
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
            if(myChunk) {
                if(chunkHasBeenAcked(myChunk)) {
                    tsnWasReneged(myChunk,
                                      0);
                }
            }
            i--;
        }
        state->highestTsnAcked = tsna;
    }


    // #######################################################################
    // #### Processing of GapAcks                                                        ####
    // #######################################################################

    if ((numGaps > 0) && (!retransmissionQ->payloadQueue.empty()) ) {
        sctpEV3 << "===== Handling GapAcks after CumTSNAck " << tsna << " =====" << endl;
        sctpEV3 << "We got " << numGaps << " gap reports" << endl;

        // We got fragment reports... check for newly acked chunks.
        const uint32 queuedChunks = retransmissionQ->payloadQueue.size();
        sctpEV3 << "Number of chunks in retransmissionQ: " << queuedChunks
                  <<" highestGapStop: "  << sackChunk->getGapStop(numGaps-1)
                  <<" highestTsnAcked: " << state->highestTsnAcked << endl;

        // ====== Handle reneging =============================================
        // highest gapStop smaller than highestTsnAcked: there might have been reneging
        if (tsnLt(sackChunk->getGapStop(numGaps-1), state->highestTsnAcked)) {
            // Reneging, type 2:
            // In a previous SACK, chunks up to highestTsnAcked have been acked.
            // This SACK contains a last gap ack < highestTsnAcked
            //      => rereg TSNs from last gap ack to highestTsnAcked
            //      => new highestTsnAcked = last gap ack
            uint32 i = state->highestTsnAcked;
            while (i >= sackChunk->getGapStop(numGaps - 1) + 1) {
                // ====== Looking up TSN in retransmission queue ================
                SCTPDataVariables* myChunk = retransmissionQ->getChunk(i);
                if(myChunk) {
                    if (chunkHasBeenAcked(myChunk)) {
                        sctpEV3 << "TSN " << i << " was found. It has been un-acked." << endl;
                        tsnWasReneged(myChunk,
                                          2);
                        sctpEV3 << "highestTsnAcked now " << state->highestTsnAcked << endl;
                    }
                }
                else {
                    sctpEV3 << "TSN " << i << " not found in retransmissionQ" << endl;
                }
                i--;
            }
            state->highestTsnAcked = sackChunk->getGapStop(numGaps - 1);
        }

        // ====== Looking for changes in the gap reports ======================
        sctpEV3 << "Looking for changes in gap reports" << endl;
        for (int32 key = 0;key < numGaps; key++) {
            const uint32 lo = sackChunk->getGapStart(key);
            const uint32 hi = sackChunk->getGapStop(key);


            // ====== Iterate over TSNs in gap reports =========================
            sctpEV3 << "Examine TSNs between " << lo << " and " << hi << endl;
            for (uint32 pos = lo; pos <= hi; pos++) {
                SCTPDataVariables* myChunk = retransmissionQ->getChunkFast(pos, getChunkFastFirstTime);
                if (myChunk) {
                    if(chunkHasBeenAcked(myChunk) == false) {
                        SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
                        assert(myChunkLastPath != NULL);
                        // T.D. 02.02.2010: This chunk has been acked newly.
                        //                        Let's process this new acknowledgement!
                        handleChunkReportedAsAcked(highestNewAck, rttEstimation, myChunk,
                                                            path /* i.e. the SACK path for RTT measurement! */);
                    }
                }
            }
        }
        state->highestTsnAcked = sackChunk->getGapStop(numGaps-1);

        // ====== Examine chunks between the gap reports ======================
        // They might have to be retransmitted or they could have been removed
        uint32 lo = tsna;
        for (int32 key = 0; key < numGaps; key++) {
            const uint32 hi = sackChunk->getGapStart(key);
            for (uint32 pos = lo+1; pos <= hi - 1; pos++) {
                SCTPDataVariables* myChunk = retransmissionQ->getChunkFast(pos, getChunkFastFirstTime);
                if(myChunk) {
                    handleChunkReportedAsMissing(sackChunk, highestNewAck, myChunk,
                                                          path /* i.e. the SACK path for RTT measurement! */);
                }
                else {
                    sctpEV3 << "TSN " << pos << " not found in retransmissionQ" << endl;
                }
            }
            lo = sackChunk->getGapStop(key);
        }


        // ====== Validity checks =============================================
    }


    // ====== Update Fast Recovery status, according to SACK =================
    updateFastRecoveryStatus(state->lastTsnAck);

    // ====== Update RTT measurement for newly acked data chunks =============
    sctpEV3 << simTime() << ": SACK: rtt=" << rttEstimation
              << ", path=" << path->remoteAddress << endl;
    pmRttMeasurement(path, rttEstimation);



    // #######################################################################
    // #### Receiver Window Management                                               ####
    // #######################################################################

    const uint32 osb = getOutstandingBytes();
    state->peerRwnd = arwnd - osb;

    // position of statement changed 20.07.05 I.R.
    if ((int32)(state->peerRwnd) < 0) {
        state->peerRwnd = 0;
    }
    if (state->peerRwnd > state->initialPeerRwnd) {
        state->peerRwnd = state->initialPeerRwnd;
    }
    if (arwnd == 1 || state->peerRwnd < state->swsLimit || arwnd == 0) {
        sctpEV3 << "processSackArrived: arwnd=" << arwnd
                  << " state->peerRwnd=" << state->peerRwnd
                  << " set peerWindowFull" << endl;
        state->peerWindowFull = true;
    }
    else
    {
        state->peerWindowFull    = false;
        state->zeroWindowProbing = false;
    }
#ifdef PVIATE
    advancePeerTsn();
#endif

    // ====== Need for zero-window probing? ==================================
    if(osb == 0) {
        if (arwnd == 0)
            state->zeroWindowProbing = true;
    }


    // #######################################################################
    // #### Congestion Window Management                                             ####
    // #######################################################################

    sctpEV3 << "Before ccUpdateBytesAcked: ";
    for(SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath    = piter->second;
        const IPvXAddress& myPathId = myPath->remoteAddress;


        if(myPath->newlyAckedBytes > 0) {
            // T.D. 07.10.2009: Only call ccUpdateBytesAcked() when there are
            //                        acked bytes on this path!
            const bool advanceWindow = myPath->newCumAck;

            sctpEV3 << simTime() << ":\tCC " << myPath->newlyAckedBytes
                      << " newly acked on path " << myPathId << ";"
                      << "\t->\tadvanceWindow="       << advanceWindow << endl;

            (this->*ccFunctions.ccUpdateBytesAcked)(myPath, myPath->newlyAckedBytes, advanceWindow);
        }
    }

    // ====== Update congestion windows on paths (handling decreases) ========
    sctpEV3 << "Before ccUpdateAfterSack with tsna=" << tsna << endl;
    // ccUpdateAfterSack() will iterate over all paths.
    (this->*ccFunctions.ccUpdateAfterSack)();


    // #######################################################################
    // #### Path Management                                                              ####
    // #######################################################################

    // ====== Need to stop or restart T3 timer? ==============================
    for(SCTPPathMap::iterator piter = sctpPathMap.begin(); piter != sctpPathMap.end(); piter++) {
        SCTPPathVariables* myPath    = piter->second;
        const IPvXAddress& myPathId = myPath->remoteAddress;

        if (myPath->outstandingBytes == 0) {
            // T.D. 07.01.2010: Only stop T3 timer when there is nothing more to send on this path!
            if(qCounter.roomTransQ.find(myPath->remoteAddress)->second == 0) {
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
            if(myPath->lowestTSNRetransmitted == true) {
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
                                                                 SCTPPathVariables* sackPath)
{
    SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
    if ( (myChunk->numberOfTransmissions == 1) &&
          (myChunk->hasBeenReneged == false) ) {
        if (myChunkLastPath == sackPath) {
            const simtime_t timeDifference = simTime() - myChunk->sendTime;
            if((timeDifference < rttEstimation) || (rttEstimation == -1.0)) {
                rttEstimation = timeDifference;
            }
            sctpEV3 << simTime()          << " processSackArrived: computed rtt time diff == "
                      << timeDifference << " for TSN "<< myChunk->tsn << endl;
        }
    }
    if ( (myChunk->hasBeenAbandoned == false)    &&
          (myChunk->hasBeenReneged == false) ) {
        myChunkLastPath->newlyAckedBytes += (myChunk->booksize);

        sctpEV3 << simTime() << ": GapAcked TSN " << myChunk->tsn
                  << " on path " << myChunkLastPath->remoteAddress << endl;

        if (myChunk->tsn > highestNewAck) {
            highestNewAck = myChunk->tsn;
        }
        ackChunk(myChunk);
        if (myChunk->countsAsOutstanding) {
            decreaseOutstandingBytes(myChunk);
        }
        if (transmissionQ->getChunk(myChunk->tsn)) {      // I.R. 02.01.07
            sctpEV3 << "Found TSN " << myChunk->tsn << " in transmissionQ -> remote it" << endl;
            transmissionQ->removeMsg(myChunk->tsn);
            myChunk->enqueuedInTransmissionQ = false;
            CounterMap::iterator q = qCounter.roomTransQ.find(myChunk->getNextDestination());
            q->second -= ADD_PADDING(myChunk->len/8+SCTP_DATA_CHUNK_LENGTH);
            CounterMap::iterator qb = qCounter.bookedTransQ.find(myChunk->getNextDestination());
            qb->second -= myChunk->booksize;
        }
        myChunk->gapReports = 0;
    }
}


void SCTPAssociation::handleChunkReportedAsMissing(const SCTPSackChunk*      sackChunk,
                                                                    const uint32                 highestNewAck,
                                                                    SCTPDataVariables*       myChunk,
                                                                    const SCTPPathVariables* sackPath)
{
    SCTPPathVariables* myChunkLastPath = myChunk->getLastDestinationPath();
    sctpEV3 << "TSN " << myChunk->tsn << " is missing in gap reports (last "
              << myChunkLastPath->remoteAddress << ") ";
    if (!chunkHasBeenAcked(myChunk)) {
        sctpEV3 << "has not been acked, highestNewAck=" << highestNewAck
                  << " countsAsOutstanding=" << myChunk->countsAsOutstanding << endl;
        const uint32 chunkReportedAsMissing = (highestNewAck > myChunk->tsn) ? 1 : 0;
        if (chunkReportedAsMissing > 0) {
            // T.D. 15.04.09: Increase gapReports by chunkReportedAsMissing.
            // Fixed bug here: gapReports += chunkReportedAsMissing instead of gapReports = chunkReportedAsMissing.
            /*
            printf("GapReports for TSN %u [ret=%d,fast=%s] at t=%s:  %d --> %d by %d\n",
                        myChunk->tsn,
                        myChunk->numberOfRetransmissions,
                        (myChunk->hasBeenFastRetransmitted == true) ? "YES" : "no",
                        simTime().str().c_str(),
                        myChunk->gapReports,
                        myChunk->gapReports + chunkReportedAsMissing,
                        chunkReportedAsMissing);
            */
            myChunk->gapReports += chunkReportedAsMissing;

            myChunkLastPath->gapNAcksInLastSACK++;

            if (myChunk->gapReports >= state->numGapReports) {
                bool fastRtx = false;
                fastRtx = ((myChunk->hasBeenFastRetransmitted == false) &&
                              (myChunk->numberOfRetransmissions == 0));
                if (fastRtx) {

                    // ====== Add chunk to transmission queue ========
                    SCTPQueue::PayloadQueue::iterator it = transmissionQ->payloadQueue.find(myChunk->tsn);
                    if (transmissionQ->getChunk(myChunk->tsn) == NULL) {
                        SCTP::AssocStat* assocStat = sctpMain->getAssocStat(assocId);
                        if(assocStat) {
                            assocStat->numFastRtx++;
                        }
                        myChunk->hasBeenFastRetransmitted = true;

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
                        if(myChunkNextPath->findLowestTSN == true) {
                            // TD 08.12.09: fixed detection of lowest TSN retransmitted
                            myChunkNextPath->lowestTSNRetransmitted = true;
                        }
                    }
                }
            }
        }
        else {
            myChunk->hasBeenFastRetransmitted = false;
            sctpEV3 << "TSN " << myChunk->tsn << " countsAsOutstanding="
                        << myChunk->countsAsOutstanding << endl;
            if (highestNewAck > myChunk->tsn) {
                myChunk->gapReports++;
            }
            myChunkLastPath->gapAcksInLastSACK++;
        }
        myChunkLastPath->findLowestTSN = false;
    }
    else {
        // Reneging, type 1:
        // A chunk in the gap blocks has been un-acked => reneg it.
                    tsnWasReneged(myChunk,
                                      1);
    }
}


uint32 SCTPAssociation::dequeueAckedChunks(const uint32       tsna,
                                                         SCTPPathVariables* sackPath,
                                                         simtime_t&           rttEstimation)
{
    SCTP::AssocStat* assocStat                   = sctpMain->getAssocStat(assocId);
    uint32            newlyAckedBytes            = 0;
    uint64            sendBufferBeforeUpdate = state->sendBuffer;

    // Set it ridiculously high
    rttEstimation = MAXTIME;

    // Are there chunks in the retransmission queue ? If Yes -> check for dequeue.
    SCTPQueue::PayloadQueue::iterator iterator = retransmissionQ->payloadQueue.begin();
    while (iterator != retransmissionQ->payloadQueue.end()) {
        SCTPDataVariables* chunk = iterator->second;
        if (tsnGe(tsna, chunk->tsn)) {
            // Dequeue chunk, cause it has been acked
            if (transmissionQ->getChunk(chunk->tsn)) {
                transmissionQ->removeMsg(chunk->tsn);
                chunk->enqueuedInTransmissionQ = false;
                CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
                q->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
                qb->second -= chunk->booksize;
            }

            chunk = retransmissionQ->getAndExtractChunk(chunk->tsn);
            state->sendBuffer -= chunk->len/8;

            SCTPPathVariables* lastPath = chunk->getLastDestinationPath();
            assert(lastPath != NULL);

            if (!chunkHasBeenAcked(chunk)) {
                newlyAckedBytes += (chunk->booksize);

                sctpEV3 << simTime() << ": CumAcked TSN " << chunk->tsn
                          << " on path " << chunk->getLastDestination() << endl;

                lastPath->newlyAckedBytes += (chunk->booksize);

                // T.D. 05.12.09: CumAck affects lastPath -> reset its T3 timer later.
                lastPath->newCumAck = true;
            }


            if(assocStat) {
                assocStat->ackedBytes += chunk->len/8;
            }

            if ((chunkHasBeenAcked(chunk) == false) && (chunk->countsAsOutstanding)) {
                ackChunk(chunk);
                if ((chunk->numberOfTransmissions == 1) && (chunk->getLastDestinationPath() == sackPath)) {
                    const simtime_t timeDifference = simTime() - chunk->sendTime;
                    if ((timeDifference < rttEstimation) || (rttEstimation == MAXTIME)) {
                        rttEstimation = timeDifference;
                    }
                }
                decreaseOutstandingBytes(chunk);
            }
            if (chunk->userData != NULL) {
                delete chunk->userData;
            }
            delete chunk;
        }
        else {
            break;
        }
        iterator = retransmissionQ->payloadQueue.begin();
    }

    if(sendBufferBeforeUpdate != state->sendBuffer && state->sendBuffer < state->sendQueueLimit) {
        // T.D. 06.02.2010: Just send SCTP_I_SENDQUEUE_ABATED once, after all newly acked
        //                        chunks have been dequeued.
        // I.R. only send indication if the sendBuffer size has dropped below the sendQueueLimit
        assert(state->lastSendQueueAbated < simTime());
        state->appSendAllowed = true;
        sctpEV3 << simTime() << ":\tSCTP_I_SENDQUEUE_ABATED("
                  << sendBufferBeforeUpdate - state->sendBuffer << ") to refill buffer "
                  << state->sendBuffer << "/" << state->sendQueueLimit << endl;
        sendIndicationToApp(SCTP_I_SENDQUEUE_ABATED, sendBufferBeforeUpdate - state->sendBuffer);
        state->lastSendQueueAbated = simTime();
    }

    sctpEV3 << "dequeueAckedChunks(): newlyAckedBytes=" << newlyAckedBytes
              << ", rttEstimation=" << rttEstimation << endl;

    return (newlyAckedBytes);
}



SCTPEventCode SCTPAssociation::processDataArrived(SCTPDataChunk* dataChunk)
{
    bool                     checkCtsnaChange = false;
    const uint32         tsn                    = dataChunk->getTsn();
    SCTPPathVariables* path                 = getPath(remoteAddr);

    state->newChunkReceived       = false;
    state->lastTsnReceived        = tsn;
    state->lastDataSourceAddress = remoteAddr;

    sctpEV3 << simTime() << " SCTPAssociation::processDataArrived TSN=" << tsn << endl;
    path->pathRcvdTSN->record(tsn);

    SCTPSimpleMessage* smsg = check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
    dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
    dataChunk->encapsulate(smsg);
    const uint32 payloadLength = dataChunk->getBitLength()/8-SCTP_DATA_CHUNK_LENGTH;
    sctpEV3 << "state->bytesRcvd=" << state->bytesRcvd << endl;
    state->bytesRcvd+=payloadLength;
    sctpEV3 << "state->bytesRcvd now=" << state->bytesRcvd << endl;
    SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
    iter->second.rcvdBytes+=dataChunk->getBitLength()/8-SCTP_DATA_CHUNK_LENGTH;

    if (state->numGaps == 0) {
        state->highestTsnReceived = state->cTsnAck;
    }
    else {
        state->highestTsnReceived = state->gapStopList[state->numGaps-1];
    }
    if (state->stopReceiving) {
        return SCTP_E_IGNORE;
    }

    if (tsnLe(tsn, state->cTsnAck)) {
            sctpEV3 << "Duplicate TSN " << tsn << " (smaller than CumAck)" << endl;
            state->dupList.push_back(tsn);
            state->dupList.unique();
            delete check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
            return SCTP_E_DUP_RECEIVED;
    }


    if ((int32)(state->localRwnd-state->queuedReceivedBytes) <= 0) {
        if (tsnGt(tsn, state->highestTsnReceived)) {
            return SCTP_E_IGNORE;
        }
        // changed 06.07.05 I.R.
        else if ((!tsnIsDuplicate(tsn)) &&
                    (tsn < state->highestTsnStored)) {
            if (!makeRoomForTsn(tsn, dataChunk->getBitLength()-SCTP_DATA_CHUNK_LENGTH*8, dataChunk->getUBit())) {
                delete check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
                return SCTP_E_IGNORE;
            }
        }
    }
    if (tsnGt(tsn, state->highestTsnReceived)) {
        sctpEV3 << "highestTsnReceived=" << state->highestTsnReceived
                  << " tsn=" << tsn << endl;
        state->highestTsnReceived = state->highestTsnStored = tsn;
        if (tsn == initPeerTsn) {
            state->cTsnAck = tsn;
        }
        else {
            sctpEV3 << "Update fragment list" << endl;
            checkCtsnaChange = updateGapList(tsn);
        }
        state->newChunkReceived = true;
    }
    else if (tsnIsDuplicate(tsn)) {
        // TSN value is duplicate within a fragment
        sctpEV3 << "Duplicate TSN " << tsn << " (copy)" << endl;
        state->dupList.push_back(tsn);
        state->dupList.unique();
        return SCTP_E_IGNORE;
    }
    else {
        checkCtsnaChange = updateGapList(tsn);
    }
    if (state->swsAvoidanceInvoked) {
        // swsAvoidanceInvoked => schedule a SACK to be sent at once in this case
        sctpEV3 << "swsAvoidanceInvoked" << endl;
        state->ackState = sackFrequency;
    }

    if (checkCtsnaChange) {
        advanceCtsna();
    }
    sctpEV3 << "cTsnAck=" << state->cTsnAck
              << " highestTsnReceived=" << state->highestTsnReceived << endl;

    SCTPEventCode event = SCTP_E_SEND;
    if (state->newChunkReceived) {
        SCTPReceiveStreamMap::iterator iter = receiveStreams.find(dataChunk->getSid());
        const int ret = iter->second->enqueueNewDataChunk(makeVarFromMsg(dataChunk));
        if (ret > 0) {
            state->queuedReceivedBytes+=payloadLength;

            event = SCTP_E_DELIVERED;
            if (ret < 3) {
                sendDataArrivedNotification(dataChunk->getSid());
                putInDeliveryQ(dataChunk->getSid());
            }
        }
        state->newChunkReceived = false;
    }

    return(event);
}


SCTPEventCode SCTPAssociation::processHeartbeatAckArrived(SCTPHeartbeatAckChunk* hback,
                                                                             SCTPPathVariables*     path)
{
    path->numberOfHeartbeatAcksRcvd++;
    path->pathRcvdHbAck->record(path->numberOfHeartbeatAcksRcvd);
    /* hb-ack goes to pathmanagement, reset error counters, stop timeout timer */
    const IPvXAddress addr          = hback->getRemoteAddr();
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
    switch(fsm->getState())
    {
        case SCTP_S_COOKIE_WAIT: retransmitInit(); break;
        case SCTP_S_COOKIE_ECHOED: retransmitCookieEcho(); break;
        default:     opp_error("Internal error: INIT-REXMIT timer expired while in state %s", stateName(fsm->getState()));
    }
    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    startTimer(T1_InitTimer,state->initRexmitTimeout);
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
    if(fsm->getState() == SCTP_S_SHUTDOWN_SENT)
    {
        retransmitShutdown();
    }
    else if (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)
        retransmitShutdownAck();

    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    startTimer(T2_ShutdownTimer,state->initRexmitTimeout);
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
    if ((simTime() - path->lastAckTime > path->heartbeatIntervalTimeout/2) || path->forceHb)
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
        if (path->pathErrorCount >   (uint32)sctpMain->par("pathMaxRetrans"))
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

    ev << "stopTimer " << timer->getName() << endl;
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



int32 SCTPAssociation::updateCounters(SCTPPathVariables* path)
{
    bool notifyUlp = false;
    if (++state->errorCount >=   (uint32)sctpMain->par("assocMaxRetrans"))
    {
        sctpEV3 << "Retransmission count during connection setup exceeds " << (int32)sctpMain->par("assocMaxRetrans") << ", giving up\n";
        sendIndicationToApp(SCTP_I_CLOSED);
        sendAbort();
        sctpMain->removeAssociation(this);
        return 0;
    }
    else if (++path->pathErrorCount >=  (uint32)sctpMain->par("pathMaxRetrans"))
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



int32 SCTPAssociation::process_TIMEOUT_RTX(SCTPPathVariables* path)
{
    sctpEV3 << "Processing retransmission timeout ..." << endl;

    // ====== Increase the RTO (by doubling it) ==============================
    path->pathRto = min(2 * path->pathRto.dbl(), sctpMain->par("rtoMax"));
    path->statisticsPathRTO->record(path->pathRto);
    sctpEV3 << "Schedule T3 based retransmission for path "<< path->remoteAddress << endl;

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
        return 0;

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
                return 0;
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
            if ( ((chunkHasBeenAcked(chunk) == false && chunk->countsAsOutstanding) || chunk->hasBeenReneged) &&
                  (chunk->getLastDestinationPath() == path) ) {
                sctpEV3 << simTime() << ": Timer-Based RTX for TSN "
                          << chunk->tsn << " on path " << chunk->getLastDestination() << endl;
                chunk->getLastDestinationPath()->numberOfTimerBasedRetransmissions++;
                SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
                iter->second.numT3Rtx++;

                moveChunkToOtherPath(chunk, getNextDestination(chunk));
            }
        }
    }


    SCTPPathVariables* nextPath = getNextPath(path);
    sctpEV3 << "TimeoutRTX: sendOnAllPaths()" << endl;
    sendOnAllPaths(nextPath);

    return 0;
}


void SCTPAssociation::moveChunkToOtherPath(SCTPDataVariables* chunk,
                                                         SCTPPathVariables* newPath)
{
    // ====== Prepare next destination =======================================
    SCTPPathVariables* lastPath = chunk->getLastDestinationPath();
    chunk->hasBeenFastRetransmitted = false;
    chunk->gapReports                     = 0;
    chunk->setNextDestination(newPath);
    sctpEV3 << simTime() << ": Timer-Based RTX for TSN " << chunk->tsn
              << ": lastDestination=" << chunk->getLastDestination()
              << " nextDestination="  << chunk->getNextDestination() << endl;

    // ======= Remove chunk's booksize from outstanding bytes ================
    // T.D. 12.02.2010: This case may happen when using sender queue control!
    if(chunk->countsAsOutstanding) {
        assert(lastPath->outstandingBytes >= chunk->booksize);
        lastPath->outstandingBytes -= chunk->booksize;
        assert((int32)lastPath->outstandingBytes >= 0);
        state->outstandingBytes -= chunk->booksize;
        assert((int64)state->outstandingBytes >= 0);
        chunk->countsAsOutstanding = false;
        // T.D. 12.02.2010: No Timer-Based RTX is necessary any more when there
        //                        are no outstanding bytes!
        if(lastPath->outstandingBytes == 0) {
            stopTimer(lastPath->T3_RtxTimer);
        }
    }


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

        if (chunk->countsAsOutstanding) {
            decreaseOutstandingBytes(chunk);
        }
        state->peerRwnd += (chunk->booksize);
    }
    if (state->peerRwnd > state->initialPeerRwnd) {
        state->peerRwnd = state->initialPeerRwnd;
    }
    sctpEV3 << "T3 Timeout: Chunk (TSN=" << chunk->tsn
                << ") has been requeued in transmissionQ, rwnd was set to "
                << state->peerRwnd << endl;
}
