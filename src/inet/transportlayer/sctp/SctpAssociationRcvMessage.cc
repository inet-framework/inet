//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <assert.h>
#include <string.h>

#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef INET_WITH_IPv6

#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpAlgorithm.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpHeader_m.h"
#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {
namespace sctp {

void SctpAssociation::decreaseOutstandingBytes(SctpDataVariables *chunk)
{
    SctpPathVariables *lastPath = chunk->getLastDestinationPath();

    if (chunk->countsAsOutstanding) {
        assert(lastPath->outstandingBytes >= chunk->booksize);
        lastPath->outstandingBytes -= chunk->booksize;
        lastPath->statisticsPathOutstandingBytes->record(lastPath->outstandingBytes);
        state->outstandingBytes -= chunk->booksize;
        SctpSendStream *stream = nullptr;
        auto associter = sendStreams.find(chunk->sid);
        if (associter != sendStreams.end()) {
            stream = associter->second;
        }
        else {
            throw cRuntimeError("Stream with id %d not found", chunk->sid);
        }
        stream->setBytesInFlight(stream->getBytesInFlight() - chunk->booksize);
        assert((int64_t)state->outstandingBytes >= 0);
        statisticsOutstandingBytes->record(state->outstandingBytes);

        chunk->countsAsOutstanding = false;

        auto iterator = qCounter.roomRetransQ.find(lastPath->remoteAddress);
        state->outstandingMessages--;
        if (state->osbWithHeader)
            iterator->second -= ADD_PADDING(chunk->booksize);
        else
            iterator->second -= ADD_PADDING(chunk->booksize + SCTP_DATA_CHUNK_LENGTH);
    }
}

// bool SctpAssociation::process_RCV_Message(const Ptr<const SctpHeader>& sctpmsg,
bool SctpAssociation::process_RCV_Message(SctpHeader *sctpmsg,
        const L3Address& src,
        const L3Address& dest)
{
    // ====== Header checks ==================================================
    EV_DEBUG << getFullPath() << " SctpAssociationRcvMessage:process_RCV_Message"
             << " localAddr=" << localAddr
             << " remoteAddr=" << remoteAddr << endl;
    state->pktDropSent = false;

    SctpPathVariables *path = getPath(src);
    const uint16_t srcPort = sctpmsg->getDestPort();
    const uint16_t destPort = sctpmsg->getSrcPort();
    const uint32_t numberOfChunks = sctpmsg->getSctpChunksArraySize();
    EV_DETAIL << "numberOfChunks=" << numberOfChunks << endl;

//    state->sctpmsg = sctpmsg->dup();
    bool authenticationNecessary = state->peerAuth;
    state->sackAlreadySent = false;

    if (fsm->getState() != SCTP_S_CLOSED &&
        fsm->getState() != SCTP_S_COOKIE_WAIT &&
        fsm->getState() != SCTP_S_COOKIE_ECHOED &&
        fsm->getState() != SCTP_S_SHUTDOWN_ACK_SENT &&
        simTime() > state->lastAssocThroughputTime + state->throughputInterval &&
        assocThroughputVector != nullptr)
    {
        assocThroughputVector->record(state->assocThroughput / (simTime() - state->lastAssocThroughputTime) / 1000);
        state->lastAssocThroughputTime = simTime();
        state->assocThroughput = 0;
    }
    state->assocThroughput += B(sctpmsg->getChunkLength()).get();

    // ====== Handle chunks ==================================================
    bool trans = true;
    bool sendAllowed = false;
    bool dupReceived = false;
    bool dataChunkReceived = false;
    bool shutdownCalled = false;
    bool sackWasReceived = false;
    for (uint32_t i = 0; i < numberOfChunks; i++) {
//        SctpChunk *header = (SctpChunk *)(sctpmsg->getSctpChunks(0));
        SctpChunk *header = sctpmsg->removeFirstChunk();
        const uint8_t type = header->getSctpChunkType();
        EV_DEBUG << "Header length: " << header->getByteLength() << endl;

        if ((type != INIT) &&
            (type != ABORT) &&
            (type != ERRORTYPE) &&
            (sctpmsg->getVTag() != peerVTag))
        {
            EV_WARN << " VTag " << sctpmsg->getVTag() << " incorrect. Should be "
                    << peerVTag << " localVTag=" << localVTag << endl;
            return true;
        }

        if (authenticationNecessary) {
            if (type == AUTH) {
                EV_INFO << "AUTH received" << endl;
                auto authChunk = check_and_cast<SctpAuthenticationChunk *>(header);
                if (authChunk->getHMacIdentifier() != 1) {
                    sendHMacError(authChunk->getHMacIdentifier());
                    auto it = sctpMain->assocStatMap.find(assocId);
                    it->second.numAuthChunksRejected++;
                    delete authChunk;
                    return true;
                }
                if (authChunk->getHMacOk() == false) {
                    delete authChunk;
                    auto it = sctpMain->assocStatMap.find(assocId);
                    it->second.numAuthChunksRejected++;
                    return true;
                }
                authenticationNecessary = false;
                auto it = sctpMain->assocStatMap.find(assocId);
                it->second.numAuthChunksAccepted++;
                delete authChunk;
                continue;
            }
            else {
                if (typeInChunkList(type)) {
                    return true;
                }
            }
        }

        switch (type) {
            case INIT: {
                EV_INFO << "INIT received" << endl;
                auto initChunk = check_and_cast<SctpInitChunk *>(header);
                if ((initChunk->getNoInStreams() != 0) &&
                    (initChunk->getNoOutStreams() != 0) &&
                    (initChunk->getInitTag() != 0))
                {
                    trans = processInitArrived(initChunk, srcPort, destPort);
                }
                i = numberOfChunks - 1;
                delete initChunk;
                break;
            }

            case INIT_ACK:
                EV_INFO << "INIT_ACK received" << endl;
                if (fsm->getState() == SCTP_S_COOKIE_WAIT) {
                    auto initAckChunk = check_and_cast<SctpInitAckChunk *>(header);
                    if ((initAckChunk->getNoInStreams() != 0) &&
                        (initAckChunk->getNoOutStreams() != 0) &&
                        (initAckChunk->getInitTag() != 0))
                    {
                        trans = processInitAckArrived(initAckChunk);
                    }
                    else if (initAckChunk->getInitTag() == 0) {
                        sendAbort();
                        sctpMain->removeAssociation(this);
                        return true;
                    }
                    i = numberOfChunks - 1;
                    delete initAckChunk;
                }
                else {
                    EV_INFO << "INIT_ACK will be ignored" << endl;
                }
                break;

            case COOKIE_ECHO: {
                EV_INFO << "COOKIE_ECHO received" << endl;
                auto cookieEchoChunk = check_and_cast<SctpCookieEchoChunk *>(header);
                trans = processCookieEchoArrived(cookieEchoChunk, src);
                delete cookieEchoChunk;
                break;
            }

            case COOKIE_ACK:
                EV_INFO << "COOKIE_ACK received" << endl;
                if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                    auto cookieAckChunk = check_and_cast<SctpCookieAckChunk *>(header);
                    trans = processCookieAckArrived();
                    delete cookieAckChunk;
                }
                break;

            case DATA:
                EV_INFO << "DATA received" << endl;
                if (fsm->getState() == SCTP_S_CLOSED) {
                    EV_INFO << "DATA in state closed: send ABORT\n";
                    sendAbort(1);
                    sctpMain->removeAssociation(this);
                    return true;
                }
                if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                    trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
                }
                if (state->stopReading) {
                    if (state->shutdownChunk) {
                        delete state->shutdownChunk;
                        state->shutdownChunk = nullptr;
                    }
                    delete header;
                    sendAbort();
                  /*  if (state->sctpmsg) {
                        delete state->sctpmsg;
                        state->sctpmsg = nullptr;
                    }*/
                    sctpMain->removeAssociation(this);
                    return true;
                }
                if (!(fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED || fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)) {
                    auto dataChunk = check_and_cast<SctpDataChunk *>(header);
                    if ((dataChunk->getByteLength() - SCTP_DATA_CHUNK_LENGTH) > 0) {
                        dacPacketsRcvd++;
                        const SctpEventCode event = processDataArrived(dataChunk);
                        if (event == SCTP_E_DELIVERED) {
                            if ((state->streamReset) && (state->incomingRequest != nullptr || state->resetRequested) &&
                                ((state->lastTsnBeforeReset == state->gapList.getHighestTsnReceived()) ||
                                 ((state->lastTsnBeforeReset == state->lastTsnReceived))))
                            {
                                resetExpectedSsns();
                                if (state->inOut) {
                                    sendOutgoingRequestAndResponse(state->inRequestSn, state->peerRequestSn);
                                }
                                else {
                                    sendStreamResetResponse(state->peerRequestSn, PERFORMED);
                                }
                            }
                            dataChunkReceived = true;
                            state->sackAllowed = true;
                        }
                        else if (event == SCTP_E_SEND) {
                            dataChunkReceived = true;
                            state->sackAllowed = true;
                        }
                        else if (event == SCTP_E_DUP_RECEIVED) {
                            dupReceived = true;
                        }
                        else if (event == SCTP_E_ABORT) {
                            sendAbort();
                            sctpMain->removeAssociation(this);
                            return true;
                        }
                        else {
                            dataChunkReceived = false;
                            state->sackAllowed = false;
                        }
                    }
                    else {
                        sendAbort();
                        sctpMain->removeAssociation(this);
                        return true;
                    }
                    delete dataChunk;
                }
                trans = true;
                break;

            case SACK:
            case NR_SACK: {
                EV_INFO << "SACK chunk received" << endl;
                auto sackChunk = check_and_cast<SctpSackChunk *>(header);
                processSackArrived(sackChunk);
                trans = true;
                sendAllowed = true;
                delete sackChunk;
                if (getOutstandingBytes() == 0 && transmissionQ->getQueueSize() == 0 && qCounter.roomSumSendStreams == 0) {
                    if (fsm->getState() == SCTP_S_SHUTDOWN_PENDING) {
                        EV_DETAIL << "No more packets: send SHUTDOWN" << endl;
                        sendShutdown();
                        trans = performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
                        shutdownCalled = true;
                    }
                    else if (fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED) {
                        EV_DETAIL << "No more outstanding" << endl;
                        sendShutdownAck(remoteAddr);
                    }
                }
                sackWasReceived = true;
                break;
            }

            case ABORT: {
                auto abortChunk = check_and_cast<SctpAbortChunk *>(header);
                EV_INFO << "ABORT with T-Bit "
                        << abortChunk->getT_Bit() << " received" << endl;
                if (sctpmsg->getVTag() == localVTag || sctpmsg->getVTag() == peerVTag) {
                    sendIndicationToApp(SCTP_I_ABORT);
                    trans = performStateTransition(SCTP_E_ABORT);
                }
                delete abortChunk;
                break;
            }

            case HEARTBEAT: {
                EV_INFO << "HEARTBEAT received" << endl;
                auto heartbeatChunk = check_and_cast<SctpHeartbeatChunk *>(header);
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
            }

            case HEARTBEAT_ACK: {
                EV_INFO << "HEARTBEAT_ACK received" << endl;
                if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                    trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
                }
                auto heartbeatAckChunk = check_and_cast<SctpHeartbeatAckChunk *>(header);
                if (path) {
                    processHeartbeatAckArrived(heartbeatAckChunk, path);
                }
                trans = true;
                delete heartbeatAckChunk;
                break;
            }

            case SHUTDOWN: {
                EV_INFO << "SHUTDOWN received" << endl;
                auto shutdownChunk = check_and_cast<SctpShutdownChunk *>(header);
                if (shutdownChunk->getCumTsnAck() > state->lastTsnAck) {
                    simtime_t rttEstimation = SIMTIME_MAX;
                    dequeueAckedChunks(shutdownChunk->getCumTsnAck(),
                            getPath(remoteAddr), rttEstimation);
                    state->lastTsnAck = shutdownChunk->getCumTsnAck();
                }
                trans = performStateTransition(SCTP_E_RCV_SHUTDOWN);
                sendIndicationToApp(SCTP_I_SHUTDOWN_RECEIVED);
                trans = true;
                delete shutdownChunk;
               /* if (state->resetChunk != nullptr) {
                    delete state->resetChunk;
                }*/
                break;
            }

            case SHUTDOWN_ACK:
                EV_INFO << "SHUTDOWN_ACK received" << endl;
                if (fsm->getState() != SCTP_S_ESTABLISHED) {
                    auto shutdownAckChunk = check_and_cast<SctpShutdownAckChunk *>(header);
                    sendShutdownComplete();
                    stopTimers();
                    stopTimer(T2_ShutdownTimer);
                    stopTimer(T5_ShutdownGuardTimer);
                    EV_DETAIL << "state=" << stateName(fsm->getState()) << endl;
                    if ((fsm->getState() == SCTP_S_SHUTDOWN_SENT) ||
                        (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT))
                    {
                        trans = performStateTransition(SCTP_E_RCV_SHUTDOWN_ACK);
                        sendIndicationToApp(SCTP_I_CLOSED);
                        if (state->shutdownChunk) {
                            delete state->shutdownChunk;
                            state->shutdownChunk = nullptr;
                        }
                    }
                    delete shutdownAckChunk;
                    if (state->resetChunk != nullptr) {
                        delete state->resetChunk;
                    }
                }
                break;

            case SHUTDOWN_COMPLETE: {
                EV_INFO << "Shutdown Complete arrived" << endl;
                auto shutdownCompleteChunk = check_and_cast<SctpShutdownCompleteChunk *>(header);
                trans = performStateTransition(SCTP_E_RCV_SHUTDOWN_COMPLETE);
                sendIndicationToApp(SCTP_I_PEER_CLOSED); // necessary for NAT-Rendezvous
                if (trans == true) {
                    stopTimers();
                }
                stopTimer(T2_ShutdownTimer);
                stopTimer(T5_ShutdownGuardTimer);
                delete state->shutdownAckChunk;
                delete shutdownCompleteChunk;
                break;
            }

            case FORWARD_TSN: {
                EV_INFO << "FORWARD_TSN received" << endl;
                auto forwChunk = check_and_cast<SctpForwardTsnChunk *>(header);
                processForwardTsnArrived(forwChunk);
                trans = true;
                sendAllowed = true;
                dataChunkReceived = true;
                delete forwChunk;
                break;
            }

            case RE_CONFIG: {
                EV_INFO << "StreamReset received" << endl;
                if (fsm->getState() != SCTP_S_ESTABLISHED && fsm->getState() != SCTP_S_SHUTDOWN_PENDING) {
                    delete header;
                    break;
                }
                auto strResChunk = check_and_cast<SctpStreamResetChunk *>(header);
                processStreamResetArrived(strResChunk);
                trans = true;
                sendAllowed = true;
                delete strResChunk;
                break;
            }

            case ASCONF:
                EV_INFO << "ASCONF received" << endl;
                if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                    trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
                }
                SctpAsconfChunk *asconfChunk;
                asconfChunk = check_and_cast<SctpAsconfChunk *>(header);
                processAsconfArrived(asconfChunk);
                trans = true;
                delete asconfChunk;
                break;

            case ASCONF_ACK: {
                EV_INFO << "ASCONF_ACK received" << endl;
                auto asconfAckChunk = check_and_cast<SctpAsconfAckChunk *>(header);
                processAsconfAckArrived(asconfAckChunk);
                trans = true;
                delete state->asconfChunk;
                delete asconfAckChunk;
                break;
            }

            case PKTDROP:
                EV_INFO << "PKTDROP received" << endl;
                if (sctpMain->pktdrop) {
                    auto packetDropChunk = check_and_cast<SctpPacketDropChunk *>(header);
                    if (packetDropChunk->getBFlag() && !packetDropChunk->getMFlag())
                        processPacketDropArrived(packetDropChunk);

                    trans = true;
                    sendAllowed = true;
                    delete packetDropChunk;
                }
                break;

            case ERRORTYPE: {
                EV_INFO << "ERROR received" << endl;
                auto errorChunk = check_and_cast<SctpErrorChunk *>(header);
                processErrorArrived(errorChunk);
                trans = true;
                delete errorChunk;
                break;
            }

            default:
                EV_ERROR << "different type" << endl; // TODO
                break;
        }

        if (i == numberOfChunks - 1 && ((dataChunkReceived && !state->sackAlreadySent) || dupReceived)) {
            sendAllowed = true;
            EV_DEBUG << "i=" << i << " sendAllowed=true; scheduleSack" << endl;
            scheduleSack();
            if (fsm->getState() == SCTP_S_SHUTDOWN_SENT && state->ackState >= sackFrequency) {
                sendSack();
            }
        }

        // Send any new DATA chunks, SACK chunks, HEARTBEAT chunks etc.
        EV_DETAIL << "SctpAssociationRcvMessage: send new data? state=" << stateName(fsm->getState())
                  << " sendAllowed=" << sendAllowed
                  << " shutdownCalled=" << shutdownCalled << endl;
        if (((fsm->getState() == SCTP_S_ESTABLISHED) ||
             (fsm->getState() == SCTP_S_SHUTDOWN_PENDING) ||
             (fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED)) &&
            (sendAllowed) &&
            (!shutdownCalled))
        {
            sendOnAllPaths(state->getPrimaryPath());
        }
    } // end of for-loop
    if (state->sendResponse > 0) {
        if (state->sendResponse == PERFORMED_WITH_OPTION) {
            resetExpectedSsns();
            if (state->incomingRequest != nullptr)
                sendStreamResetResponse(check_and_cast<SctpSsnTsnResetRequestParameter *>(state->incomingRequest), PERFORMED, true);
        }
        else if (state->sendResponse == PERFORMED_WITH_ADDOUT) {
            sendAddOutgoingStreamsRequest(check_and_cast<SctpAddStreamsRequestParameter *>(state->incomingRequest)->getNumberOfStreams());
            state->numResetRequests++;
            sendStreamResetResponse(check_and_cast<SctpAddStreamsRequestParameter *>(state->incomingRequest)->getSrReqSn(), PERFORMED);
        }
        else {
            sendStreamResetResponse(state->responseSn, state->sendResponse);
        }
        state->sendResponse = 0;
        state->responseSn = 0;
    }
    if (sackWasReceived) {
        checkStreamsToReset();
        sackWasReceived = false;
    }

    // ====== Clean-up =======================================================
    /*  if (!state->pktDropSent) {
          disposeOf(state->sctpmsg);
          EV_DEBUG << "state->sctpmsg was disposed" << endl;
      }*/
    return trans;
}

bool SctpAssociation::processInitArrived(SctpInitChunk *initchunk, int32_t srcPort, int32_t destPort)
{
    SctpAssociation *assoc;
    char timerName[64];
    bool trans = false;
    uint16_t type;
    AddressVector adv;

    EV_TRACE << "processInitArrived\n";
    if (fsm->getState() == SCTP_S_CLOSED) {
        EV_INFO << "fork=" << state->fork << " initReceived=" << state->initReceived << "\n";
        if (state->fork && !state->initReceived) {
            EV_TRACE << "cloneAssociation\n";
            assoc = cloneAssociation();
            EV_TRACE << "addForkedAssociation\n";
            sctpMain->addForkedAssociation(this, assoc, localAddr, remoteAddr, srcPort, destPort);
            assoc->listening = true;
            this->listening = false;

            EV_INFO << "Connection forked: this connection got new assocId=" << assocId << ", "
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
        else {
            sctpMain->updateSockPair(this, localAddr, remoteAddr, srcPort, destPort);
        }
        if (!state->initReceived) {
            state->initReceived = true;
            state->initialPrimaryPath = remoteAddr;
            state->setPrimaryPath(getPath(remoteAddr));
            if (initchunk->getAddressesArraySize() == 0) {
                EV_INFO << " get new path for " << remoteAddr << "\n";
                SctpPathVariables *rPath = new SctpPathVariables(remoteAddr, this, rt);
                sctpPathMap[rPath->remoteAddress] = rPath;
                qCounter.roomTransQ[rPath->remoteAddress] = 0;
                qCounter.bookedTransQ[rPath->remoteAddress] = 0;
                qCounter.roomRetransQ[rPath->remoteAddress] = 0;
            }
            initPeerTsn = initchunk->getInitTsn();
            state->gapList.setInitialCumAckTsn(initPeerTsn - 1);
            state->initialPeerRwnd = initchunk->getA_rwnd();
            if (initchunk->getMsg_rwnd() > 0) {
                state->peerAllowsChunks = true;
                state->initialPeerMsgRwnd = initchunk->getMsg_rwnd();
                state->peerMsgRwnd = state->initialPeerMsgRwnd;
            }
            state->expectedStreamResetSequenceNumber = initPeerTsn;
            state->peerRwnd = state->initialPeerRwnd;
            statisticsPeerRwnd->record(state->peerRwnd);
            localVTag = initchunk->getInitTag();
            numberOfRemoteAddresses = initchunk->getAddressesArraySize();
            state->localAddresses.clear();
            if (localAddressList.front().isUnspecified()) {
                for (int32_t i = 0; i < ift->getNumInterfaces(); ++i) {
#ifdef INET_WITH_IPv4
                    if (auto ipv4Data = ift->getInterface(i)->findProtocolData<Ipv4InterfaceData>()) {
                        adv.push_back(ipv4Data->getIPAddress());
                    }
                    else
#endif // ifdef INET_WITH_IPv4
#ifdef INET_WITH_IPv6
                    if (auto ipv6Data = ift->getInterface(i)->findProtocolData<Ipv6InterfaceData>()) {
                        adv.push_back(ipv6Data->getAddress(0));
                    }
                    else
#endif // ifdef INET_WITH_IPv6
                        throw cRuntimeError("INET was compiled without IPv4/IPv6 support");
                }
            }
            else {
                adv = localAddressList;
            }
            int rlevel = getAddressLevel(remoteAddr);
            if (adv.size() == 1) {
                state->localAddresses.push_back((*adv.begin()));
            }
            else if (rlevel > 0) {
                for (auto& elem : adv) {
                    if (getAddressLevel((elem)) >= rlevel) {
                        sctpMain->addLocalAddress(this, (elem));
                        state->localAddresses.push_back((elem));
                    }
                }
            }
            for (uint32_t j = 0; j < initchunk->getAddressesArraySize(); j++) {
                // skip IPv6 because we can't send to them yet
                if (initchunk->getAddresses(j).getType() == L3Address::IPv6)
                    continue;
                // set path variables for this pathlocalAddresses
                if (!getPath(initchunk->getAddresses(j))) {
                    SctpPathVariables *path = new SctpPathVariables(initchunk->getAddresses(j), this, rt);
                    EV_INFO << " get new path for " << initchunk->getAddresses(j) << "\n";
                    for (auto& elem : state->localAddresses) {
                        if (sctpMain->addRemoteAddress(this, (elem), initchunk->getAddresses(j))) {
                            this->remoteAddressList.push_back(initchunk->getAddresses(j));
                        }
                    }
                    sctpPathMap[path->remoteAddress] = path;
                    qCounter.roomTransQ[path->remoteAddress] = 0;
                    qCounter.bookedTransQ[path->remoteAddress] = 0;
                    qCounter.roomRetransQ[path->remoteAddress] = 0;
                }
            }
            if (!containsKey(sctpPathMap, remoteAddr)) {
                SctpPathVariables *path = new SctpPathVariables(remoteAddr, this, rt);
                EV_INFO << "Get new path for " << remoteAddr << "\n";
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr] = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
            }
            if (initchunk->getHmacTypesArraySize() != 0) {
                state->peerAuth = true;
                for (uint32_t j = 0; j < initchunk->getSctpChunkTypesArraySize(); j++) {
                    type = initchunk->getSctpChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE) {
                        state->peerChunkList.push_back(type);
                    }
                }
            }
            EV_DETAIL << "number supported extensions:" << initchunk->getSepChunksArraySize() << "\n";
            if (initchunk->getSepChunksArraySize() > 0) {
                for (uint32_t i = 0; i < initchunk->getSepChunksArraySize(); i++) {
                    if (initchunk->getSepChunks(i) == RE_CONFIG) {
                        state->peerStreamReset = true;
                        continue;
                    }
                    if (initchunk->getSepChunks(i) == PKTDROP) {
                        state->peerPktDrop = true;
                        EV_DEBUG << "set peerPktDrop to true\n";
                        continue;
                    }
                }
            }
            trans = performStateTransition(SCTP_E_RCV_INIT);
            if (trans) {
                sendInitAck(initchunk);
            }
        }
        else if (fsm->getState() == SCTP_S_CLOSED) {
            trans = performStateTransition(SCTP_E_RCV_INIT);
            if (trans) {
                sendInitAck(initchunk);
            }
        }
        else {
            trans = true;
        }
    }
    else if (fsm->getState() == SCTP_S_COOKIE_WAIT) { // INIT-Collision
        EV_INFO << "INIT collision: send Init-Ack\n";
        if (initchunk->getHmacTypesArraySize() != 0) {
            state->peerAuth = true;
            if (state->peerChunkList.size() == 0) {
                for (uint32_t j = 0; j < initchunk->getSctpChunkTypesArraySize(); j++) {
                    type = initchunk->getSctpChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE) {
                        state->peerChunkList.push_back(type);
                    }
                }
            }
        }
        sendInitAck(initchunk);
        trans = true;
    }
    else if (fsm->getState() == SCTP_S_COOKIE_ECHOED || fsm->getState() == SCTP_S_ESTABLISHED) {
        // check, whether a new address has been added
        bool addressPresent = false;
        for (uint32_t j = 0; j < initchunk->getAddressesArraySize(); j++) {
            if (initchunk->getAddresses(j).getType() == L3Address::IPv6)
                continue;
            for (auto& elem : remoteAddressList) {
                if ((elem) == (initchunk->getAddresses(j))) {
                    addressPresent = true;
                    break;
                }
            }
            if (!addressPresent) {
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

bool SctpAssociation::processInitAckArrived(SctpInitAckChunk *initAckChunk)
{
    bool trans = false;
    uint16_t type;

    if (fsm->getState() == SCTP_S_COOKIE_WAIT) {
        EV_INFO << "State is COOKIE_WAIT, Cookie_Echo has to be sent\n";
        stopTimer(T1_InitTimer);
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
        trans = performStateTransition(SCTP_E_RCV_INIT_ACK);
//        delete state->initChunk; will be deleted when state ESTABLISHED is entered
        if (trans) {
            initPeerTsn = initAckChunk->getInitTsn();
            localVTag = initAckChunk->getInitTag();
            state->gapList.setInitialCumAckTsn(initPeerTsn - 1);
            state->initialPeerRwnd = initAckChunk->getA_rwnd();
            state->peerRwnd = state->initialPeerRwnd;
            statisticsPeerRwnd->record(state->peerRwnd);
            if (initAckChunk->getMsg_rwnd() > 0) {
                state->peerAllowsChunks = true;
                state->initialPeerMsgRwnd = initAckChunk->getMsg_rwnd();
                state->peerMsgRwnd = state->initialPeerMsgRwnd;
            }
            state->expectedStreamResetSequenceNumber = initPeerTsn;
            remoteAddressList.clear();
            numberOfRemoteAddresses = initAckChunk->getAddressesArraySize();
            EV_INFO << "number of remote addresses in initAck=" << numberOfRemoteAddresses << "\n";
            for (uint32_t j = 0; j < numberOfRemoteAddresses; j++) {
                if (initAckChunk->getAddresses(j).getType() == L3Address::IPv6)
                    continue;
                for (auto& elem : state->localAddresses) {
                    if (!((elem).isUnspecified())) {
                        EV_INFO << "addPath " << initAckChunk->getAddresses(j) << "\n";
                        if (sctpMain->addRemoteAddress(this, (elem), initAckChunk->getAddresses(j))) {
                            this->remoteAddressList.push_back(initAckChunk->getAddresses(j));
                            addPath(initAckChunk->getAddresses(j));
                        }
                    }
                }
            }
            if (!containsKey(sctpPathMap, remoteAddr)) {
                EV_INFO << " get new path for " << remoteAddr << "\n";
                SctpPathVariables *path = new SctpPathVariables(remoteAddr, this, rt);
                sctpPathMap[remoteAddr] = path;
                qCounter.roomTransQ[remoteAddr] = 0;
                qCounter.roomRetransQ[remoteAddr] = 0;
                qCounter.bookedTransQ[remoteAddr] = 0;
            }
            state->initialPrimaryPath = remoteAddr;
            state->setPrimaryPath(getPath(remoteAddr));
            inboundStreams = ((initAckChunk->getNoOutStreams() < inboundStreams) ? initAckChunk->getNoOutStreams() : inboundStreams);
            outboundStreams = ((initAckChunk->getNoInStreams() < outboundStreams) ? initAckChunk->getNoInStreams() : outboundStreams);
            (this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
            if (initAckChunk->getHmacTypesArraySize() != 0) {
                state->peerAuth = true;
                for (uint32_t j = 0; j < initAckChunk->getSctpChunkTypesArraySize(); j++) {
                    type = initAckChunk->getSctpChunkTypes(j);
                    if (type != INIT && type != INIT_ACK && type != AUTH && type != SHUTDOWN_COMPLETE) {
                        state->peerChunkList.push_back(type);
                    }
                }
            }
            if (initAckChunk->getSepChunksArraySize() > 0) {
                for (uint32_t i = 0; i < initAckChunk->getSepChunksArraySize(); i++) {
                    if (initAckChunk->getSepChunks(i) == RE_CONFIG) {
                        state->peerStreamReset = true;
                        continue;
                    }
                    if (initAckChunk->getSepChunks(i) == PKTDROP) {
                        state->peerPktDrop = true;
                        continue;
                    }
                }
            }
            sendCookieEcho(initAckChunk);
        }
        startTimer(T1_InitTimer, state->initRexmitTimeout);
    }
    else
        EV_DETAIL << "State=" << fsm->getState() << "\n";
    printSctpPathMap();
    return trans;
}

bool SctpAssociation::processCookieEchoArrived(SctpCookieEchoChunk *cookieEcho, L3Address addr)
{
    bool trans = false;

    const SctpCookie *cookie = cookieEcho->getStateCookie();
    if (cookie->getCreationTime() + simtime_t(sctpMain->par("validCookieLifetime")) < simTime()) {
        EV_INFO << "stale Cookie: sendAbort\n";
        sendAbort();
        delete cookie;
        return trans;
    }
    if (fsm->getState() == SCTP_S_CLOSED) {
        if (cookie->getLocalTag() != localVTag || cookie->getPeerTag() != peerVTag) {
            bool same = true;
            for (int32_t i = 0; i < 32; i++) {
                if (cookie->getLocalTieTag(i) != state->localTieTag[i]) {
                    same = false;
                    break;
                }
                if (cookie->getPeerTieTag(i) != state->peerTieTag[i]) {
                    same = false;
                    break;
                }
            }
            if (!same) {
                sendAbort();
                delete cookie;
                return trans;
            }
        }
        EV_INFO << "State is CLOSED, Cookie_Ack has to be sent\n";
        trans = performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
        if (trans)
            sendCookieAck(addr); // send to address
    }
    else if (fsm->getState() == SCTP_S_ESTABLISHED || fsm->getState() == SCTP_S_COOKIE_WAIT || fsm->getState() == SCTP_S_COOKIE_ECHOED) {
        EV_INFO << "State is not CLOSED, but COOKIE_ECHO received. Compare the Tags\n";
        // case A: Peer restarted, retrieve information from cookie
        if (cookie->getLocalTag() != localVTag && cookie->getPeerTag() != peerVTag) {
            bool same = true;
            for (int32_t i = 0; i < 32; i++) {
                if (cookie->getLocalTieTag(i) != state->localTieTag[i]) {
                    same = false;
                    break;
                }
                if (cookie->getPeerTieTag(i) != state->peerTieTag[i]) {
                    same = false;
                    break;
                }
            }
            if (same) {
                localVTag = cookie->getLocalTag();
                peerVTag = cookie->getPeerTag();
                sendCookieAck(addr);
            }
        }
        // case B: Setup collision, retrieve information from cookie
        else if (cookie->getPeerTag() == peerVTag && (cookie->getLocalTag() != localVTag || cookie->getLocalTag() == 0)) {
            localVTag = cookie->getLocalTag();
            peerVTag = cookie->getPeerTag();
            performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
            sendCookieAck(addr);
        }
        else if (cookie->getPeerTag() == peerVTag && cookie->getLocalTag() == localVTag) {
            sendCookieAck(addr); // send to address src
        }
        trans = true;
    }
    else {
        EV_DETAIL << "State=" << fsm->getState() << "\n";
        trans = true;
    }
    delete cookie;
    return trans;
}

bool SctpAssociation::processCookieAckArrived()
{
    bool trans = false;

    if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
        stopTimer(T1_InitTimer);
        trans = performStateTransition(SCTP_E_RCV_COOKIE_ACK);
        if (state->cookieChunk->getCookieArraySize() == 0) {
            delete state->cookieChunk->getStateCookie();
        }
        delete state->cookieChunk;
        return trans;
    }
    else
        EV_DETAIL << "State=" << fsm->getState() << "\n";

    return trans;
}

void SctpAssociation::tsnWasReneged(SctpDataVariables *chunk,
        const SctpPathVariables *sackPath,
        const int type)
{
    if ((state->allowCMT) && (state->cmtSmartReneging) &&
        (sackPath != chunk->ackedOnPath))
    {
        return;
    }
    EV_INFO << "TSN " << chunk->tsn << " has been reneged (type "
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

// SACK processing code iterates over all TSNs in the RTX queue.
// Calls cucProcessGapReports() for each TSN, setting isAcked=TRUE
// for chunks being acked, isAcked=FALSE otherwise.
void SctpAssociation::cucProcessGapReports(const SctpDataVariables *chunk,
        SctpPathVariables *path,
        const bool isAcked)
{
    // We only care for newly acked chunks.
    // Therefore, the previous state must be "unacked".
    if (chunkHasBeenAcked(chunk) == false) {
        // For CUCv2, it has to be checked whether it is the first transmission.
        // Otherwise, the behaviour will be like CUCv1 -> decreasing PseudoCumAck on T3 RTX!
        if ((path->findPseudoCumAck == true) &&
            ((chunk->numberOfRetransmissions == 0) ||
             (state->cmtCUCVariant == SctpStateVariables::CUCV_PseudoCumAck)))
        {
            path->pseudoCumAck = chunk->tsn;
            path->findPseudoCumAck = false;
        }
        if ((isAcked) && /* Not acked before and acked now => ack for the first time */
            (path->pseudoCumAck == chunk->tsn))
        {
            path->newPseudoCumAck = true;
            path->findPseudoCumAck = true;
        }

        // CUCv2
        if ((path->findRTXPseudoCumAck == true) &&
            (chunk->numberOfRetransmissions > 0))
        {
            path->rtxPseudoCumAck = chunk->tsn;
            path->findRTXPseudoCumAck = false;
        }
        if ((isAcked) && /* Not acked before and acked now => ack for the first time */
            (path->rtxPseudoCumAck == chunk->tsn))
        {
            path->newRTXPseudoCumAck = true;
            path->findRTXPseudoCumAck = true;
        }
    }
}

SctpEventCode SctpAssociation::processSackArrived(SctpSackChunk *sackChunk)
{
    simtime_t rttEstimation = SIMTIME_MAX;
    const uint64_t sendBufferBeforeUpdate = state->sendBuffer;
    SctpPathVariables *path = getPath(remoteAddr); // Path for *this* SACK!
    const uint64_t arwnd = sackChunk->getA_rwnd();
    const uint32_t tsna = sackChunk->getCumTsnAck();
    uint32_t highestNewAck = tsna; // Highest newly acked TSN
    const uint16_t numDups = sackChunk->getNumDupTsns();
    Sctp::AssocStat *assocStat = sctpMain->getAssocStat(assocId);
    bool dropFilledGap = false;
    const uint32_t msgRwnd = sackChunk->getMsg_rwnd();

    // ====== Put information from SACK into GapList =========================
    SctpGapList sackGapList;
    sackGapList.setInitialCumAckTsn(sackChunk->getCumTsnAck());
    uint32_t lastTsn = sackChunk->getCumTsnAck();
    for (uint32_t i = 0; i < sackChunk->getNumGaps(); i++) {
        uint32_t tsn = sackChunk->getGapStart(i);
        assert(tsnLt(lastTsn + 1, tsn));
        lastTsn = tsn;
        while (tsnLe(tsn, sackChunk->getGapStop(i))) {
            bool dummy;
            sackGapList.updateGapList(tsn, dummy, true); // revokable TSN
            tsn++;
        }
        lastTsn = sackChunk->getGapStop(i);
    }
    if (assocStat) {
        assocStat->sumRGapRanges += ((sackChunk->getCumTsnAck() <= lastTsn) ?
                                     (uint64_t)(lastTsn - sackChunk->getCumTsnAck()) :
                                     (uint64_t)(sackChunk->getCumTsnAck() - lastTsn));
    }
    if (sackChunk->getNrSubtractRGaps() == false) {
        lastTsn = sackChunk->getCumTsnAck();
        for (uint32_t i = 0; i < sackChunk->getNumNrGaps(); i++) {
            uint32_t tsn = sackChunk->getNrGapStart(i);
            assert(tsnLt(lastTsn + 1, tsn));
            lastTsn = tsn;
            while (tsnLe(tsn, sackChunk->getNrGapStop(i))) {
                bool dummy;
                sackGapList.updateGapList(tsn, dummy, false); // non-revokable TSN
                tsn++;
            }
            lastTsn = sackChunk->getNrGapStop(i);
        }
    }
    else {
        lastTsn = sackChunk->getCumTsnAck();
        for (uint32_t i = 0; i < sackChunk->getNumNrGaps(); i++) {
            uint32_t tsn = sackChunk->getNrGapStart(i);
            assert(tsnLt(lastTsn + 1, tsn));
            lastTsn = tsn;
            while (tsnLe(tsn, sackChunk->getNrGapStop(i))) {
                if (sackGapList.tsnIsRevokable(tsn) == false) {
                    bool dummy;
                    sackGapList.updateGapList(tsn, dummy, false); // non-revokable TSN
                }
                tsn++;
            }
            lastTsn = sackChunk->getNrGapStop(i);
        }
    }
    if (assocStat) {
        assocStat->sumNRGapRanges += (sackChunk->getCumTsnAck() <= lastTsn) ?
            (uint64_t)(lastTsn - sackChunk->getCumTsnAck()) :
            (uint64_t)(sackChunk->getCumTsnAck() - lastTsn);
    }
    const uint16_t numGaps = sackGapList.getNumGaps(SctpGapList::GT_Any);

    // ====== Print some information =========================================
    EV_DETAIL << "##### SACK Processing: TSNa=" << tsna << " #####" << endl;
    for (auto& elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
        EV_DETAIL << "Path " << myPath->remoteAddress << ":\t"
                  << "outstanding=" << path->outstandingBytes << "\t"
                  << "T3scheduled=" << path->T3_RtxTimer->getArrivalTime() << " "
                  << (path->T3_RtxTimer->isScheduled() ? "[ok]" : "[NOT SCHEDULED]") << "\t"
                  << "findPseudoCumAck=" << ((myPath->findPseudoCumAck == true) ? "true" : "false") << "\t"
                  << "pseudoCumAck=" << myPath->pseudoCumAck << "\t"
                  << "newPseudoCumAck=" << ((myPath->newPseudoCumAck == true) ? "true" : "false") << "\t"
                  << "findRTXPseudoCumAck=" << ((myPath->findRTXPseudoCumAck == true) ? "true" : "false") << "\t"
                  << "rtxPseudoCumAck=" << myPath->rtxPseudoCumAck << "\t"
                  << "newRTXPseudoCumAck=" << ((myPath->newRTXPseudoCumAck == true) ? "true" : "false") << "\t"
                  << endl;
    }

    EV_INFO << "Before processSackArrived for path " << path->remoteAddress
            << " with tsna=" << tsna << ":" << endl;

    // ====== SACK Sequence Number Check =====================================
    EV_INFO << "SACK Seq Number = " << sackChunk->getSackSeqNum() << endl;
    if ((state->checkSackSeqNumber == true) &&
        (sackChunk->getSackSeqNum() <= state->incomingSackSeqNum))
    {
        EV_DETAIL << "Out-of-data SACK: " << sackChunk->getSackSeqNum()
                  << " < " << state->incomingSackSeqNum << endl;
        return SCTP_E_IGNORE;
    }
    state->incomingSackSeqNum = sackChunk->getSackSeqNum();

    // ====== Record statistics ==============================================
    numGapBlocks->record(numGaps);
    statisticsRevokableGapBlocksInLastSACK->record(sackGapList.getNumGaps(SctpGapList::GT_Revokable));
    statisticsNonRevokableGapBlocksInLastSACK->record(sackGapList.getNumGaps(SctpGapList::GT_NonRevokable));

    path->vectorPathAckedTsnCumAck->record(tsna);
    if (assocStat) {
        assocStat->numDups += numDups;
    }

    // ====== Initialize some variables ======================================
    for (auto& elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
        // T.D. 26.03.09: Remember outstanding bytes before this update
        // Values are necessary for updating the congestion window!
        myPath->outstandingBytesBeforeUpdate = myPath->outstandingBytes; // copy from myPath, not from path!
        myPath->requiresRtx = false;
        myPath->lowestTsnRetransmitted = false;
        myPath->findLowestTsn = true;
        myPath->gapAckedChunksInLastSACK = 0;
        myPath->gapNRAckedChunksInLastSACK = 0;
        myPath->gapUnackedChunksInLastSACK = 0;
        myPath->newlyAckedBytes = 0;
        myPath->newCumAck = false; // Check whether CumAck affects this path
        // for all destinations, set newPseudoCumAck to FALSE.
        myPath->newPseudoCumAck = false;
        myPath->newRTXPseudoCumAck = false; // CUCv2
        myPath->sawNewAck = false;
        myPath->lowestNewAckInSack = 0;
        myPath->highestNewAckInSack = 0;
        myPath->newOldestChunkSendTime = simTime() + 9999.99; // initialize to more than simTime()
        if (myPath == path) {
            myPath->lastAckTime = simTime();
        }
    }

    // ====== Zero Window Probing ============================================
    if ((state->peerAllowsChunks) && (msgRwnd > 0) && (state->zeroWindowProbing)) {
        state->zeroWindowProbing = false;
    }
    if ((state->zeroWindowProbing) && (arwnd > 0)) {
        state->zeroWindowProbing = false;
    }

    // #######################################################################
    // #### Processing of CumAck                                          ####
    // #######################################################################

    if (tsnGt(tsna, state->lastTsnAck)) { // Handle new CumAck
        EV_INFO << "===== Handling new CumAck for TSN " << tsna << " =====" << endl;

        SctpDataVariables *myChunk = retransmissionQ->getChunk(state->lastTsnAck + 1);
        if ((myChunk != nullptr) && (myChunk->wasPktDropped) &&
            (myChunk->getLastDestinationPath()->fastRecoveryActive))
        {
            dropFilledGap = true;
            EV_DETAIL << "TSN " << myChunk->tsn << " filled gap" << endl;
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
        // ====== Slow Path RTT Calculation ===================================
        if ((state->allowCMT == true) &&
            (state->cmtSlowPathRTTUpdate == true) &&
            (path->waitingForRTTCalculaton == true) &&
            ((path->tsnForRTTCalculation == tsna) ||
             (tsnLt(path->tsnForRTTCalculation, tsna))))
        {
            // Got update from CumAck -> no need for Slow Path RTT calculation
            path->waitingForRTTCalculaton = false;
        }
    }
    else if (tsnLt(tsna, state->lastTsnAck)) {
        EV_DETAIL << "Stale CumAck (" << tsna << " < " << state->lastTsnAck << ")"
                  << endl;
        // ====== Slow Path RTT Calculation ===================================
        if ((state->allowCMT == true) &&
            (state->cmtSlowPathRTTUpdate == true) &&
            (path->waitingForRTTCalculaton == true))
        {
            // Slow Path Update
            // Look for a CumAck or GapAck of the remembered chunk on this path.
            // If it has been found, we can compute the RTT of this path.

            // ====== Look for matching CumAck first ===========================
            bool renewRTT = false;
            if ((tsnLt(path->tsnForRTTCalculation, tsna)) ||
                (path->tsnForRTTCalculation == tsna))
            {
                renewRTT = true;
            }
            // ====== Look for matching GapAck =================================
            else if ((numGaps > 0) &&
                     ((path->tsnForRTTCalculation == sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1)) ||
                      (tsnLt(path->tsnForRTTCalculation, sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1)))))
            {
                for (int32_t key = 0; key < numGaps; key++) {
                    const uint32_t lo = sackGapList.getGapStart(SctpGapList::GT_Any, key);
                    const uint32_t hi = sackGapList.getGapStop(SctpGapList::GT_Any, key);
                    if ((path->tsnForRTTCalculation == lo) ||
                        (path->tsnForRTTCalculation == hi) ||
                        (tsnLt(lo, path->tsnForRTTCalculation) &&
                         tsnLt(path->tsnForRTTCalculation, hi)))
                    {
                        renewRTT = true;
                        break;
                    }
                }
            }

            if (renewRTT) {
                rttEstimation = simTime() - path->txTimeForRTTCalculation;
                path->waitingForRTTCalculaton = false;
                pmRttMeasurement(path, rttEstimation);

                EV_DETAIL << simTime() << ": SlowPathRTTUpdate from stale SACK - rtt="
                          << rttEstimation << " from TSN "
                          << path->tsnForRTTCalculation
                          << " on path " << path->remoteAddress
                          << " => RTO=" << path->pathRto << endl;
            }
        }
        return SCTP_E_IGNORE;
    }

    // ====== Handle reneging ================================================
    if ((numGaps == 0) && (tsnLt(tsna, state->highestTsnAcked))) {
        // Reneging, type 0:
        // In a previous SACK, chunks up to highestTsnAcked have been acked.
        // This SACK contains a CumAck < highestTsnAcked
        //      => rereg TSNs from CumAck+1 to highestTsnAcked
        //      => new highestTsnAcked = CumAck
        EV_DETAIL << "numGaps=0 && tsna " << tsna
                  << " < highestTsnAcked " << state->highestTsnAcked << endl;
        uint32_t i = state->highestTsnAcked;
        while (i >= tsna + 1) {
            SctpDataVariables *myChunk = retransmissionQ->getChunk(i);
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

    if ((numGaps > 0) && (!retransmissionQ->payloadQueue.empty())) {
        EV_INFO << "===== Handling GapAcks after CumTSNAck " << tsna << " =====" << endl;
        EV_INFO << "We got " << numGaps << " gap reports" << endl;

        // We got fragment reports... check for newly acked chunks.
        const uint32_t queuedChunks = retransmissionQ->payloadQueue.size();
        EV_DETAIL << "Number of chunks in retransmissionQ: " << queuedChunks
                  << " highestGapStop: " << sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1)
                  << " highestTsnAcked: " << state->highestTsnAcked << endl;

        // ====== Handle reneging =============================================
        // highest gapStop smaller than highestTsnAcked: there might have been reneging
        if (tsnLt(sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1), state->highestTsnAcked)) {
            // Reneging, type 2:
            // In a previous SACK, chunks up to highestTsnAcked have been acked.
            // This SACK contains a last gap ack < highestTsnAcked
            //      => rereg TSNs from last gap ack to highestTsnAcked
            //      => new highestTsnAcked = last gap ack
            uint32_t i = state->highestTsnAcked;
            while (i >= sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1) + 1) {
                // ====== Looking up TSN in retransmission queue ================
                SctpDataVariables *myChunk = retransmissionQ->getChunk(i);
                if (myChunk) {
                    if (chunkHasBeenAcked(myChunk)) {
                        EV_INFO << "TSN " << i << " was found. It has been un-acked." << endl;
                        tsnWasReneged(myChunk, path, 2);
                        EV_DETAIL << "highestTsnAcked now " << state->highestTsnAcked << endl;
                    }
                }
                else {
                    EV_INFO << "TSN " << i << " not found in retransmissionQ" << endl;
                }
                i--;
            }
            state->highestTsnAcked = sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1);
        }

        // ====== Looking for changes in the gap reports ======================
        EV_INFO << "Looking for changes in gap reports" << endl;
        // Get Pseudo CumAck for paths
        uint32_t lo1 = tsna;
        uint32_t tsnCheck = tsna + 1; // Just to make sure that no TSN is misssed
        for (int32_t key = 0; key < numGaps; key++) {
            const uint32_t lo = sackGapList.getGapStart(SctpGapList::GT_Any, key);
            const uint32_t hi = sackGapList.getGapStop(SctpGapList::GT_Any, key);

            // ====== Iterate over TSNs *not* listed in gap reports ============
            for (uint32_t pos = lo1 + 1; pos <= lo - 1; pos++) {
                assert(tsnCheck == pos);
                tsnCheck++;
                SctpDataVariables *myChunk = retransmissionQ->getChunk(pos);
                if (myChunk) {
                    SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
                    assert(myChunkLastPath != nullptr);
                    // T.D. 22.11.09: CUCv2 - chunk is *not* acked
                    cucProcessGapReports(myChunk, myChunkLastPath, false);
                }
            }
            lo1 = sackGapList.getGapStop(SctpGapList::GT_Any, key);
            // ====== Iterate over TSNs in gap reports =========================
            EV_INFO << "Examine TSNs between " << lo << " and " << hi << endl;
            for (uint32_t pos = lo; pos <= hi; pos++) {
                bool chunkFirstTime = true;
                assert(tsnCheck == pos);
                tsnCheck++;
                SctpDataVariables *myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                if (myChunk) {
                    if (chunkHasBeenAcked(myChunk) == false) {
                        SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
                        assert(myChunkLastPath != nullptr);
                        // CUCv2 - chunk is acked
                        cucProcessGapReports(myChunk, myChunkLastPath, true);
                        // This chunk has been acked newly.
                        // Let's process this new acknowledgement!
                        handleChunkReportedAsAcked(highestNewAck, rttEstimation, myChunk,
                                path /* i.e. the SACK path for RTT measurement! */,
                                sackGapList.tsnIsNonRevokable(myChunk->tsn));
                    }
                    else {
                        // Slow Path RTT Calculation
                        if ((path->tsnForRTTCalculation == myChunk->tsn) &&
                            (path->waitingForRTTCalculaton == true) &&
                            (state->allowCMT == true) &&
                            (state->cmtSlowPathRTTUpdate == true) &&
                            (myChunk->getLastDestinationPath() == path))
                        {
                            const simtime_t rttEstimation = simTime() - path->txTimeForRTTCalculation;
                            path->waitingForRTTCalculaton = false;
                            pmRttMeasurement(path, rttEstimation);

                            EV << simTime() << ": SlowPathRTTUpdate from gap report - rtt="
                               << rttEstimation << " from TSN "
                               << path->tsnForRTTCalculation
                               << " on path " << path->remoteAddress
                               << " => RTO=" << path->pathRto << endl;
                        }
                    }
                }
                // ====== R-acked chunk became NR-acked =========================
                else if (sackGapList.tsnIsNonRevokable(pos)) {
                    bool chunkFirstTime = false;
                    SctpDataVariables *myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                    if (myChunk) {
                        // myChunk != nullptr -> R-acked before, but not NR-acked
                        handleChunkReportedAsAcked(highestNewAck, rttEstimation, myChunk,
                                path /* i.e. the SACK path for RTT measurement! */,
                                sackGapList.tsnIsNonRevokable(myChunk->tsn));
                        // All NR-acked chunks have chunkMap->getChunk(pos) == nullptr!
                    }
                }
            }
        }
        state->highestTsnAcked = sackGapList.getGapStop(SctpGapList::GT_Any, numGaps - 1);

        // ====== Examine chunks between the gap reports ======================
        // They might have to be retransmitted or they could have been removed
        uint32_t lo = tsna;
        for (int32_t key = 0; key < numGaps; key++) {
            const uint32_t hi = sackGapList.getGapStart(SctpGapList::GT_Any, key);
            for (uint32_t pos = lo + 1; pos <= hi - 1; pos++) {
                bool chunkFirstTime = true;
                SctpDataVariables *myChunk = retransmissionQ->getChunkFast(pos, chunkFirstTime);
                if (myChunk) {
                    handleChunkReportedAsMissing(sackChunk, highestNewAck, myChunk,
                            path /* i.e. the SACK path for RTT measurement! */);
                }
                else {
                    EV_INFO << "TSN " << pos << " not found in retransmissionQ" << endl;
                }
            }
            lo = sackGapList.getGapStop(SctpGapList::GT_Any, key);
        }

        // ====== Validity checks =============================================
        path->vectorPathAckedTsnGapAck->record(state->highestTsnAcked);
    }

    // ====== Buffer space may have been gained => tell application ==========
    if (sendBufferBeforeUpdate != state->sendBuffer) {
        generateSendQueueAbatedIndication(sendBufferBeforeUpdate - state->sendBuffer);
    }

    // ====== Update Fast Recovery status, according to SACK =================
    updateFastRecoveryStatus(state->lastTsnAck);

    // ====== Update RTT measurement for newly acked data chunks =============
    if (rttEstimation < SIMTIME_MAX) {
        EV_DETAIL << simTime() << ": SACK: rtt=" << rttEstimation
                  << ", path=" << path->remoteAddress << endl;
        pmRttMeasurement(path, rttEstimation);
    }

    // ====== Record statistics ==============================================
    for (auto& elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
        myPath->statisticsPathGapAckedChunksInLastSACK->record(myPath->gapAckedChunksInLastSACK);
        myPath->statisticsPathGapNRAckedChunksInLastSACK->record(myPath->gapNRAckedChunksInLastSACK);
        myPath->statisticsPathGapUnackedChunksInLastSACK->record(myPath->gapUnackedChunksInLastSACK);
    }

    // #######################################################################
    // #### Receiver Window Management                                    ####
    // #######################################################################

    const uint32_t osb = getOutstandingBytes();
    if (state->bytesToAddPerPeerChunk > 0) {
        state->peerRwnd = arwnd - osb - (state->outstandingMessages * state->bytesToAddPerPeerChunk);
    }
    else if (state->peerAllowsChunks) {
        state->peerMsgRwnd = msgRwnd - state->outstandingMessages;
        state->peerRwnd = arwnd - osb;
        if ((int32_t)(state->peerMsgRwnd) < 0) {
            state->peerMsgRwnd = 0;
        }
        if (state->peerMsgRwnd > state->initialPeerMsgRwnd) {
            state->peerMsgRwnd = state->initialPeerMsgRwnd;
        }
    }
    else {
        state->peerRwnd = arwnd - osb;
    }
    if ((int32_t)(state->peerRwnd) < 0) {
        state->peerRwnd = 0;
    }
    // position of statement changed 20.07.05 I.R.
    if ((int32_t)(state->peerRwnd) < 0) {
        state->peerRwnd = 0;
    }
    if (state->peerRwnd > state->initialPeerRwnd) {
        state->peerRwnd = state->initialPeerRwnd;
    }
    if (state->peerAllowsChunks && msgRwnd == 0) {
        state->peerWindowFull = true;
    }
    if (arwnd == 1 || state->peerRwnd < state->swsLimit || arwnd == 0) {
        EV_INFO << "processSackArrived: arwnd=" << arwnd
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
    EV_DEBUG << "Before ccUpdateBytesAcked: ";
    for (auto& elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
        const L3Address& myPathId = myPath->remoteAddress;

        if (myPath->newPseudoCumAck) {
            myPath->vectorPathPseudoCumAck->record(myPath->pseudoCumAck);
        }
        if (myPath->newRTXPseudoCumAck) {
            myPath->vectorPathRTXPseudoCumAck->record(myPath->rtxPseudoCumAck);
        }
        if (myPath->newlyAckedBytes > 0) {
            // Only call ccUpdateBytesAcked() when there are
            // acked bytes on this path!
            bool advanceWindow = myPath->newPseudoCumAck || myPath->newRTXPseudoCumAck;
            if (state->allowCMT == true) {
                if (state->cmtCUCVariant == SctpStateVariables::CUCV_PseudoCumAckV2) {
                    advanceWindow = myPath->newPseudoCumAck || myPath->newRTXPseudoCumAck;
                }
                else if (state->cmtCUCVariant == SctpStateVariables::CUCV_PseudoCumAck) {
                    advanceWindow = myPath->newPseudoCumAck;
                }
                else if (state->cmtCUCVariant == SctpStateVariables::CUCV_Normal) {
                    advanceWindow = myPath->newCumAck;
                }
            }
            EV_DETAIL << simTime() << ":\tCC " << myPath->newlyAckedBytes
                      << " newly acked on path " << myPathId << ";"
                      << "\tpath->newPseudoCumAck=" << ((myPath->newPseudoCumAck == true) ? "true" : "false")
                      << "\tpath->newRTXPseudoCumAck=" << ((myPath->newRTXPseudoCumAck == true) ? "true" : "false")
                      << "\tdropFilledGap=" << ((dropFilledGap == true) ? "true" : "false")
                      << "\t->\tadvanceWindow=" << advanceWindow << endl;

            (this->*ccFunctions.ccUpdateBytesAcked)(myPath, myPath->newlyAckedBytes,
                    (advanceWindow && dropFilledGap) ? false :
                    advanceWindow);
            if (state->maxBurstVariant == SctpStateVariables::MBV_MaxBurst) {
                myPath->packetsInBurst = 0;
            }
        }
    }

    // ====== Update congestion windows on paths (handling decreases) ========
    EV_DEBUG << "Before ccUpdateAfterSack with tsna=" << tsna << endl;
    // ccUpdateAfterSack() will iterate over all paths.
    (this->*ccFunctions.ccUpdateAfterSack)();

    // #######################################################################
    // #### Path Management                                               ####
    // #######################################################################

    if ((state->allowCMT == true) &&
        (state->cmtSmartT3Reset == true))
    {
        // ====== Find oldest unacked chunk on each path ======================
        for (SctpQueue::PayloadQueue::const_iterator iterator = retransmissionQ->payloadQueue.begin();
             iterator != retransmissionQ->payloadQueue.end(); ++iterator)
        {
            const SctpDataVariables *myChunk = iterator->second;
            SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
            if (!chunkHasBeenAcked(myChunk)) {
                if (myChunkLastPath->newOldestChunkSendTime > myChunk->sendTime) {
                    EV_DETAIL << "TSN " << myChunk->tsn << " is new oldest on path "
                              << myChunkLastPath->remoteAddress << ", rel send time is "
                              << simTime() - myChunk->sendTime << " ago" << endl;
                    myChunkLastPath->newOldestChunkSendTime = myChunk->sendTime;
                    myChunkLastPath->oldestChunkTsn = myChunk->tsn;
                }
            }
        }
    }

    // ====== Need to stop or restart T3 timer? ==============================
    for (auto& elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
        const L3Address& myPathId = myPath->remoteAddress;

        // ====== Smart T3 Reset ===============================================
        bool updatedOldestChunkSendTime = false;
        if ((state->allowCMT == true) &&
            (state->cmtSmartT3Reset == true))
        {
            // ====== Has oldest chunk send time been updated? =================
            if (myPath->newOldestChunkSendTime > simTime()) {
                myPath->newOldestChunkSendTime = myPath->oldestChunkSendTime;
                // newOldestChunkSendTime > simTime => no old chunk found:
                // Set newOldestChunkSendTime to oldestChunkSendTime
            }
            else if (myPath->newOldestChunkSendTime != myPath->oldestChunkSendTime) {
                // Update oldestChunkSendTime.
                myPath->oldestChunkSendTime = myPath->newOldestChunkSendTime;
                updatedOldestChunkSendTime = true;
            }
            assert(myPath->oldestChunkSendTime <= simTime());
        }
        if (myPath->outstandingBytes == 0) {
            // T.D. 07.01.2010: Only stop T3 timer when there is nothing more to send on this path!
            if (qCounter.roomTransQ.find(myPath->remoteAddress)->second == 0) {
                // Stop T3 timer, if there are no more outstanding bytes.
                stopTimer(myPath->T3_RtxTimer);
                myPath->oldestChunkSendTime = SIMTIME_ZERO;
            }
        }
        else if (myPath->newCumAck) { // Only care for CumAcks here!
            // NOTE: Due to the existence of retransmissions *before* PseudoCumAck for CUCv2,
            //       it is *not* possible to check PseudoCumAck here!
            //       This would miss retransmissions -> chunks would never be retransmitted!
            stopTimer(myPath->T3_RtxTimer);
            startTimer(myPath->T3_RtxTimer, myPath->pathRto);
        }
        else if (updatedOldestChunkSendTime) { // Smart T3 Reset
            stopTimer(myPath->T3_RtxTimer);
            startTimer(myPath->T3_RtxTimer, myPath->pathRto);
        }
        else {
            /* Also restart T3 timer, when lowest TSN is rtx'ed */
            if (myPath->lowestTsnRetransmitted == true) {
                EV_INFO << "Lowest TSN retransmitted => restart of T3 timer for path "
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

void SctpAssociation::handleChunkReportedAsAcked(uint32_t& highestNewAck,
        simtime_t& rttEstimation,
        SctpDataVariables *myChunk,
        SctpPathVariables *sackPath,
        const bool sackIsNonRevokable)
{
    SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
    // SFR algorithm
    if (state->allowCMT == true) {
        EV << "TSN " << myChunk->tsn << " on path " << myChunkLastPath->remoteAddress << ":\t"
           << "findPseudoCumAck=" << ((myChunkLastPath->findPseudoCumAck == true) ? "true" : "false") << "\t"
           << "pseudoCumAck=" << myChunkLastPath->pseudoCumAck << "\t"
           << "newPseudoCumAck=" << ((myChunkLastPath->newPseudoCumAck == true) ? "true" : "false") << "\t"
           << "findRTXPseudoCumAck=" << ((myChunkLastPath->findRTXPseudoCumAck == true) ? "true" : "false") << "\t"
           << "rtxPseudoCumAck=" << myChunkLastPath->rtxPseudoCumAck << "\t"
           << "newRTXPseudoCumAck=" << ((myChunkLastPath->newRTXPseudoCumAck == true) ? "true" : "false") << "\t"
           << endl;

        // This chunk has not been acked before -> new ack on its myChunkLastPath.
        if (myChunkLastPath->sawNewAck == false) {
            EV << "TSN " << myChunk->tsn << " on path " << myChunkLastPath->remoteAddress << ":\t"
               << "Saw new ack -> setting highestNewAckInSack!" << endl;
            myChunkLastPath->sawNewAck = true;
        }

        // Smart Fast RTX
        // If chunk has already been transmitted on another path, do not consider it
        // for fast RTX handling!
        if ((!myChunk->hasBeenTimerBasedRtxed) ||
            (state->cmtSmartFastRTX == false))
        {
            if (myChunkLastPath->lowestNewAckInSack == 0) {
                myChunkLastPath->lowestNewAckInSack = myChunk->tsn; // The lowest TSN acked
            }
            if (myChunkLastPath->highestNewAckInSack == 0) {
                myChunkLastPath->highestNewAckInSack = myChunk->tsn; // The highest TSN acked so far
            }
            else if (tsnLt(myChunkLastPath->highestNewAckInSack, myChunk->tsn)) {
                myChunkLastPath->highestNewAckInSack = myChunk->tsn; // The highest TSN acked so far
            }
        }
    }
    if ((myChunk->numberOfTransmissions == 1) &&
        (myChunk->hasBeenMoved == false) &&
        (myChunk->hasBeenReneged == false))
    {
        if (myChunkLastPath == sackPath) {
            const simtime_t timeDifference = simTime() - myChunk->sendTime;
            if ((timeDifference < rttEstimation) || (rttEstimation == -1.0)) {
                rttEstimation = timeDifference;
            }
            EV_DETAIL << simTime() << " processSackArrived: computed rtt time diff == "
                      << timeDifference << " for TSN " << myChunk->tsn << endl;
        }
        else {
            if ((state->allowCMT == true) &&
                (state->cmtSlowPathRTTUpdate == true) &&
                (myChunkLastPath->waitingForRTTCalculaton == false))
            {
                // numberOfTransmissions==1, hasBeenReneged==false
                // T.D. 25.02.2010: Slow Path Update
                // Remember this chunk's TSN and send time in order to update the
                // path's RTT using a stale SACK on its own path.
                myChunkLastPath->tsnForRTTCalculation = myChunk->tsn;
                myChunkLastPath->txTimeForRTTCalculation = myChunk->sendTime;
                myChunkLastPath->waitingForRTTCalculaton = true;
            }
        }
    }
    if ((myChunk->hasBeenAbandoned == false) &&
        (myChunk->hasBeenReneged == false) &&
        (myChunk->hasBeenAcked == false))
    {
        EV_DETAIL << simTime() << ": GapAcked TSN " << myChunk->tsn
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

void SctpAssociation::handleChunkReportedAsMissing(const SctpSackChunk *sackChunk,
        const uint32_t highestNewAck,
        SctpDataVariables *myChunk,
        SctpPathVariables *sackPath)
{
    SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
    EV_INFO << "TSN " << myChunk->tsn << " is missing in gap reports (last "
            << myChunkLastPath->remoteAddress << ") ";
    if (!chunkHasBeenAcked(myChunk)) {
        EV_DETAIL << "has not been acked, highestNewAck=" << highestNewAck
                  << " countsAsOutstanding=" << myChunk->countsAsOutstanding << endl;

        // ===== Check whether a Fast Retransmission is necessary =============
        // Non-CMT behaviour: check for highest TSN
        uint32_t chunkReportedAsMissing = (highestNewAck > myChunk->tsn || (myChunk->getLastDestinationPath()->fastRecoveryActive && state->highestTsnAcked >= myChunk->tsn)) ? 1 : 0;

        // Split Fast Retransmission (SFR) algorithm for CMT
        if ((state->allowCMT == true) && (state->cmtUseSFR == true)) {
            chunkReportedAsMissing = 0; // Default: do not assume chunk as missing.

            // If there has been another chunk with highest TSN acked on this path,
            // the current one is missing.
            if ((myChunkLastPath->sawNewAck) &&
                (tsnGt(myChunkLastPath->highestNewAckInSack, myChunk->tsn)))
            {
                if (state->cmtUseDAC == false) {
                    chunkReportedAsMissing = 1;
                }
                else {
                    // ------ DAC algorithm at sender side -----------
                    // Is there a newly acked TSN on another path?
                    bool sawNewAckOnlyOnThisPath = true;
                    for (auto& elem : sctpPathMap) {
                        const SctpPathVariables *otherPath = elem.second;
                        if ((otherPath != myChunkLastPath) && (otherPath->sawNewAck)) {
                            sawNewAckOnlyOnThisPath = false;
                            break;
                        }
                    }
                    if (sawNewAckOnlyOnThisPath == true) {
                        // All newly acked TSNs were sent on the same path
                        EV << "SplitFastRTX + DAC: all on same path:   "
                           << "TSN=" << myChunk->tsn
                           << " lowestNewAckInSack=" << myChunkLastPath->lowestNewAckInSack
                           << " highestNewAckInSack=" << myChunkLastPath->highestNewAckInSack
                           << " (on path " << myChunkLastPath->remoteAddress << ")" << endl;
                        // Are there newly acked TSNs ta, tb, so that ta < myChunk->tsn < tb?
                        // myChunkLastPath->highestNewAckInSack is highest newly acked TSN on the current path
                        //   -> since all TSNs were on this path, this value can be used as tb
                        // lowestNewAckInSack is the lowest newly acked TSN of this SACK
                        //   -> since all TSNs were on the same path, this value can be used as ta
                        if (tsnLt(myChunkLastPath->lowestNewAckInSack, myChunk->tsn) &&
                            tsnLt(myChunk->tsn, myChunkLastPath->highestNewAckInSack))
                        {
                            EV << "   => conservative increment of 1" << endl;
                            chunkReportedAsMissing = 1;
                        }
                        else if (tsnGt(myChunkLastPath->lowestNewAckInSack, myChunk->tsn)) { // All newly acked TSNs are larger than myChunk->tsn
                            EV << "   => reported increment of dacPacketsRcvd=" << (unsigned int)sackChunk->getDacPacketsRcvd() << endl;
                            chunkReportedAsMissing = sackChunk->getDacPacketsRcvd();
                        }
                    }
                    else {
                        // Mixed SACKS: newly acked TSNs were sent to multiple paths
                        EV << "SplitFastRTX + DAC: mixed acks, increment is 1" << endl;
                        chunkReportedAsMissing = 1;
                    }
                }
            } // else: There is no need to increment the missing count.

            EV << "SplitFastRTX: chunkReportedAsMissing="
               << chunkReportedAsMissing << ", "
               << "sawNewAck=" << myChunkLastPath->sawNewAck << ", "
               << "lowestNewAckInSack=" << myChunkLastPath->lowestNewAckInSack
               << "highestNewAckInSack=" << myChunkLastPath->highestNewAckInSack << ", "
               << "TSN=" << myChunk->tsn << endl;
        }
        if (chunkReportedAsMissing > 0) {
            myChunk->gapReports += chunkReportedAsMissing;
            myChunkLastPath->gapUnackedChunksInLastSACK++;

            if (myChunk->gapReports >= state->numGapReports) {
                bool fastRtx = false;
                switch (state->rtxMethod) {
                    case 0: // Only one Fast RTX after 3 Gap reports
                        fastRtx = ((myChunk->hasBeenFastRetransmitted == false) &&
                                   ((myChunk->numberOfRetransmissions == 0) ||
                                    ((myChunk->hasBeenMoved) &&
                                     (myChunk->countsAsOutstanding) &&
                                     (state->movedChunkFastRTXFactor > 0) &&
                                     ((simTime() - myChunk->sendTime) > state->movedChunkFastRTXFactor * myChunkLastPath->srtt))));
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
                    if (myChunk->hasBeenMoved) {
                        EV << simTime() << ": MovedFastRTX for TSN " << myChunk->tsn << endl;
                    }
                    myChunk->hasBeenMoved = false; // Just trigger *one* fast RTX ...
                    // ====== Add chunk to transmission queue ========
                    if (transmissionQ->getChunk(myChunk->tsn) == nullptr) {
                        if (!chunkMustBeAbandoned(myChunk, sackPath)) {
                            Sctp::AssocStat *assocStat = sctpMain->getAssocStat(assocId);
                            if (assocStat) {
                                assocStat->numFastRtx++;
                            }
                        }
                        myChunk->hasBeenFastRetransmitted = true;
                        myChunk->sendForwardIfAbandoned = true;

                        EV_DETAIL << simTime() << ": Fast RTX for TSN "
                                  << myChunk->tsn << " on path " << myChunk->getLastDestination() << endl;
                        myChunkLastPath->numberOfFastRetransmissions++;

                        myChunk->setNextDestination(getNextDestination(myChunk));
                        SctpPathVariables *myChunkNextPath = myChunk->getNextDestinationPath();
                        assert(myChunkNextPath != nullptr);

                        if (myChunk->countsAsOutstanding) {
                            decreaseOutstandingBytes(myChunk);
                        }
                        if (!transmissionQ->checkAndInsertChunk(myChunk->tsn, myChunk)) {
                            EV_DETAIL << "Fast RTX: cannot add message/chunk (TSN="
                                      << myChunk->tsn << ") to the transmissionQ" << endl;
                        }
                        else {
                            myChunk->enqueuedInTransmissionQ = true;
                            auto q = qCounter.roomTransQ.find(myChunk->getNextDestination());
                            q->second += ADD_PADDING(myChunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                            auto qb = qCounter.bookedTransQ.find(myChunk->getNextDestination());
                            qb->second += myChunk->booksize;
                        }
                        myChunkNextPath->requiresRtx = true;
                        if (myChunkNextPath->findLowestTsn == true) {
                            // TD 08.12.09: fixed detection of lowest TSN retransmitted
                            myChunkNextPath->lowestTsnRetransmitted = true;
                        }
                    }
                }
            }
        }
        myChunkLastPath->findLowestTsn = false;
    }
    else {
        // Reneging, type 1:
        // A chunk in the gap blocks has been un-acked => reneg it.
        tsnWasReneged(myChunk, sackPath, 1);
    }
}

void SctpAssociation::nonRenegablyAckChunk(SctpDataVariables *chunk,
        SctpPathVariables *sackPath,
        simtime_t& rttEstimation,
        Sctp::AssocStat *assocStat)
{
    SctpPathVariables *lastPath = chunk->getLastDestinationPath();
    assert(lastPath != nullptr);

    // ====== Bookkeeping ====================================================
    // Subtract chunk size from sender buffer size
    state->sendBuffer -= chunk->len / 8;

    // Subtract chunk size from the queue size of its stream
    auto streamIterator = sendStreams.find(chunk->sid);
    assert(streamIterator != sendStreams.end());
    SctpSendStream *stream = streamIterator->second;
    assert(stream != nullptr);
    cPacketQueue *streamQ = (chunk->ordered == false) ? stream->getUnorderedStreamQ() : stream->getStreamQ();
    assert(streamQ != nullptr);

    if (chunk->priority > 0) {
        state->queuedDroppableBytes -= chunk->len / 8;
    }

    if ((chunk->hasBeenCountedAsNewlyAcked == false) &&
        (chunk->hasBeenAcked == false))
    {
        if ((state->cmtMovedChunksReduceCwnd == false) ||
            (chunk->hasBeenMoved == false))
        {
            chunk->hasBeenCountedAsNewlyAcked = true;
            // The chunk has not been acked before.
            // Therefore, its size may *once* be counted as newly acked.
            lastPath->newlyAckedBytes += chunk->booksize;
        }
    }

    assert(chunk->queuedOnPath->queuedBytes >= chunk->booksize);
    chunk->queuedOnPath->queuedBytes -= chunk->booksize;
    chunk->queuedOnPath->statisticsPathQueuedSentBytes->record(chunk->queuedOnPath->queuedBytes);
    chunk->queuedOnPath = nullptr;

    assert(state->queuedSentBytes >= chunk->booksize);
    state->queuedSentBytes -= chunk->booksize;
    statisticsQueuedSentBytes->record(state->queuedSentBytes);
    if (assocStat) {
        assocStat->ackedBytes += chunk->len / 8;
    }
    if ((assocStat) && (fairTimer)) {
        assocStat->fairAckedBytes += chunk->len / 8;
    }

    if ((state->allowCMT == true) &&
        (state->cmtSlowPathRTTUpdate == true) &&
        (lastPath->waitingForRTTCalculaton == false) &&
        (lastPath != sackPath) &&
        (chunk->numberOfTransmissions == 1) &&
        (chunk->hasBeenMoved == false) &&
        (chunk->hasBeenReneged == false))
    {
        // Slow Path Update
        // Remember this chunk's TSN and send time in order to update the
        // path's RTT using a stale SACK on its own path.
        lastPath->tsnForRTTCalculation = chunk->tsn;
        lastPath->txTimeForRTTCalculation = chunk->sendTime;
        lastPath->waitingForRTTCalculaton = true;
    }

    // ====== RTT calculation ================================================
    if ((chunkHasBeenAcked(chunk) == false) && (chunk->countsAsOutstanding)) {
        if ((chunk->numberOfTransmissions == 1) && (lastPath == sackPath) && (chunk->hasBeenMoved == false)) {
            const simtime_t timeDifference = simTime() - chunk->sendTime;
            if ((timeDifference < rttEstimation) || (rttEstimation == SIMTIME_MAX)) {
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
        auto q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
        auto qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
        qb->second -= chunk->booksize;
    }

    // ====== Remove chunk from retransmission queue =========================
    chunk = retransmissionQ->getAndExtractChunk(chunk->tsn);
    if (chunk->userData != nullptr) {
        delete chunk->userData;
    }
    delete chunk;
}

void SctpAssociation::renegablyAckChunk(SctpDataVariables *chunk,
        SctpPathVariables *sackPath)
{
    // ====== Bookkeeping ====================================================
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }

    if ((chunk->hasBeenCountedAsNewlyAcked == false) &&
        (chunk->hasBeenAcked == false))
    {
        if ((state->cmtMovedChunksReduceCwnd == false) ||
            (chunk->hasBeenMoved == false))
        {
            chunk->hasBeenCountedAsNewlyAcked = true;
            // The chunk has not been acked before.
            // Therefore, its size may *once* be counted as newly acked.
            chunk->getLastDestinationPath()->newlyAckedBytes += chunk->booksize;
        }
    }

    // ====== Acknowledge chunk =============================================
    ackChunk(chunk, sackPath);
    chunk->gapReports = 0;

    // ====== Remove chunk from transmission queue ===========================
    if (transmissionQ->getChunk(chunk->tsn)) { // I.R. 02.01.2007
        EV_INFO << "Found TSN " << chunk->tsn << " in transmissionQ -> remote it" << endl;
        transmissionQ->removeMsg(chunk->tsn);
        chunk->enqueuedInTransmissionQ = false;
        auto q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
        auto qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
        qb->second -= chunk->booksize;
    }
}

void SctpAssociation::generateSendQueueAbatedIndication(const uint64_t bytes)
{
    if (state->sendBuffer < state->sendQueueLimit) {
        // Just send SCTP_I_SENDQUEUE_ABATED once, after all newly acked
        // chunks have been dequeued.
        // Only send indication if the sendBuffer size has dropped below the sendQueueLimit
//        assert(state->lastSendQueueAbated < simTime());
        state->appSendAllowed = true;
        EV_INFO << simTime() << ":\tSCTP_I_SENDQUEUE_ABATED("
                << bytes << ") to refill buffer "
                << state->sendBuffer << "/" << state->sendQueueLimit << endl;

        Indication *msg = new Indication(indicationName(SCTP_I_SENDQUEUE_ABATED), SCTP_I_SENDQUEUE_ABATED);

        msg->addTag<SocketInd>()->setSocketId(assocId);
        auto& sendQueueAbatedIndication = msg->addTag<SctpSendQueueAbatedReq>();
        sendQueueAbatedIndication->setSocketId(assocId);
        sendQueueAbatedIndication->setLocalAddr(localAddr);
        sendQueueAbatedIndication->setRemoteAddr(remoteAddr);
        sendQueueAbatedIndication->setNumMsgs(bytes); // NOTE: Legacy API!
        sendQueueAbatedIndication->setQueuedForStreamArraySize(sendStreams.size());
        unsigned int streamID = 0;
        for (auto& elem : sendStreams) {
            const SctpSendStream *stream = elem.second;
            sendQueueAbatedIndication->setQueuedForStream(streamID, stream->getUnorderedStreamQ()->getByteLength() + stream->getStreamQ()->getByteLength());
            streamID++;
        }

        sendQueueAbatedIndication->setBytesAvailable(state->sendQueueLimit - state->sendBuffer);
        sendQueueAbatedIndication->setBytesQueued(state->sendBuffer);
        sendQueueAbatedIndication->setBytesLimit(state->sendQueueLimit);

        sctpMain->send(msg, "appOut");

        state->lastSendQueueAbated = simTime();
    }
}

void SctpAssociation::dequeueAckedChunks(const uint32_t tsna,
        SctpPathVariables *sackPath,
        simtime_t& rttEstimation)
{
    Sctp::AssocStat *assocStat = sctpMain->getAssocStat(assocId);

    // Set it ridiculously high
    rttEstimation = SIMTIME_MAX;

    // Are there chunks in the retransmission queue ? If Yes -> check for dequeue.
    auto iterator = retransmissionQ->payloadQueue.begin();
    while (iterator != retransmissionQ->payloadQueue.end()) {
        SctpDataVariables *chunk = iterator->second;
        if (tsnGe(tsna, chunk->tsn)) {
            EV_DETAIL << simTime() << ": CumAcked TSN " << chunk->tsn
                      << " on path " << chunk->getLastDestination() << endl;

            if (!chunkHasBeenAcked(chunk)) {
                SctpPathVariables *lastPath = chunk->getLastDestinationPath();
                // CumAck affects lastPath -> reset its T3 timer later.
                lastPath->newCumAck = true;
                // CumAck of SACK has acknowledged this chunk. Handle Pseudo CumAck.
                lastPath->findPseudoCumAck = true;
                lastPath->newPseudoCumAck = true;
                // T.D. 22.11.09: CUCv2
                lastPath->findRTXPseudoCumAck = true;
                lastPath->newRTXPseudoCumAck = true;
            }

            nonRenegablyAckChunk(chunk, sackPath, rttEstimation, assocStat);
        }
        else {
            break;
        }
        iterator = retransmissionQ->payloadQueue.begin();
    }

    EV_INFO << "dequeueAckedChunks(): rttEstimation=" << rttEstimation << endl;
}

SctpEventCode SctpAssociation::processForwardTsnArrived(SctpForwardTsnChunk *fwChunk)
{
    EV_TRACE << "processForwardTsnArrived\n";
    EV_INFO << "last state->cTsnAck=" << state->gapList.getCumAckTsn() << " fwCumAck=" << fwChunk->getNewCumTsn() << "\n";

    /* Ignore old FORWARD_TSNs, probably stale retransmits. */
    if (state->gapList.getCumAckTsn() >= fwChunk->getNewCumTsn()) {
        return SCTP_E_IGNORE;
    }

    for (uint32_t i = 0; i < fwChunk->getSidArraySize(); i++) {
        if (fwChunk->getSsn(i) != -1) {
            auto iter = receiveStreams.find(fwChunk->getSid(i));
            SctpReceiveStream *rStream = iter->second;

            /* Uncomment the folloing to drop gap-acknowledged messages
             * between two abandonend messages rather then delivering them.
             */

            if (rStream->getOrderedQ()->getQueueSize() > 0)
                rStream->setExpectedStreamSeqNum(rStream->getOrderedQ()->getFirstSsnInQueue(fwChunk->getSid(i)));
            else if (rStream->getExpectedStreamSeqNum() <= fwChunk->getSsn(i))
                rStream->setExpectedStreamSeqNum(fwChunk->getSsn(i) + 1);
            if (rStream->getExpectedStreamSeqNum() > 65535) {
                rStream->setExpectedStreamSeqNum(0);
            }
            sendDataArrivedNotification(fwChunk->getSid(i));
            calculateRcvBuffer();
        }
    }
    /* Update Gap lists with abandoned TSNs and advance CumTSNAck */
    for (uint32_t i = state->gapList.getCumAckTsn() + 1; i <= fwChunk->getNewCumTsn(); i++) {
        if (i > state->gapList.getCumAckTsn() && !state->gapList.tsnInGapList(i)) {
            bool dummy;
            state->gapList.updateGapList(i, dummy, false);
            state->gapList.tryToAdvanceCumAckTsn();
        }
    }
    return SCTP_E_IGNORE;
}

SctpEventCode SctpAssociation::processDataArrived(SctpDataChunk *dataChunk)
{
    const uint32_t tsn = dataChunk->getTsn();
    SctpPathVariables *path = getPath(remoteAddr);

    state->newChunkReceived = false;
    state->lastTsnReceived = tsn;

    bool found = false;
    for (auto& elem : state->lastDataSourceList) {
        if (elem == path) {
            found = true;
            break;
        }
    }
    if (!found) {
        state->lastDataSourceList.push_back(path);
    }
    state->lastDataSourcePath = path;

    EV_INFO << simTime() << " SctpAssociation::processDataArrived TSN=" << tsn << endl;
    path->vectorPathReceivedTsn->record(tsn);
    if (dataChunk->getIBit()) {
        state->ackState = sackFrequency;
    }
    calculateRcvBuffer();

    const uint32_t payloadLength = dataChunk->getByteLength() - SCTP_DATA_CHUNK_LENGTH;

    EV_DETAIL << "state->bytesRcvd=" << state->bytesRcvd << endl;
    if (payloadLength == 0) {
        EV_DETAIL << "No user data. Send ABORT" << endl;
        return SCTP_E_ABORT;
    }
    state->bytesRcvd += payloadLength;
    EV_DETAIL << "state->bytesRcvd now=" << state->bytesRcvd << endl;
    path->numberOfBytesReceived += payloadLength;
    auto iter = sctpMain->assocStatMap.find(assocId);
    iter->second.rcvdBytes += payloadLength;

    if (state->stopReceiving) {
        return SCTP_E_SEND;
    }

    // ====== Duplicate: tsn < CumAckTsn =====================================
    if (tsnLe(tsn, state->gapList.getCumAckTsn())) {
        if (state->stopOldData) {
            if (tsnGe(tsn, state->peerTsnAfterReset)) {
                state->stopOldData = false;
            }
            return SCTP_E_SEND;
        }
        else {
            EV_DETAIL << simTime() << ": Duplicate TSN " << tsn << " (smaller than CumAck)" << endl;
            state->dupList.push_back(tsn);
            state->dupList.unique();
            path->numberOfDuplicates++;

            return SCTP_E_DUP_RECEIVED;
        }
    }

    // ====== Duplicate ======================================================
    if (tsnIsDuplicate(tsn)) {
        // TSN value is duplicate within a fragment
        EV_DETAIL << "Duplicate TSN " << tsn << " (copy)" << endl;
        state->dupList.push_back(tsn);
        state->dupList.unique();
        path->numberOfDuplicates++;
        return SCTP_E_SEND;
    }

    // ====== Out of receiver buffer space? ==================================
    calculateRcvBuffer();
    if (((state->messageAcceptLimit > 0) &&
         (state->localMsgRwnd - state->bufferedMessages <= 0)) ||
        ((state->messageAcceptLimit == 0) &&
         ((int32_t)(state->localRwnd - state->queuedReceivedBytes
                    - state->bufferedMessages * state->bytesToAddPerRcvdChunk) <= 0)))
    {
        state->ackState = sackFrequency;

        if (tsnGt(tsn, state->gapList.getHighestTsnReceived())) {
            EV_DETAIL << "DROP: " << (int)tsn << " high=" << (int)state->gapList.getHighestTsnReceived()
                      << " Q=" << (int)state->queuedReceivedBytes << " Rwnd=" << (int)state->localRwnd << endl;
            if ((!state->pktDropSent) && (sctpMain->pktdrop) && (state->peerPktDrop)) {
                EV_DETAIL << "Receive buffer full (case 1): sendPacketDrop" << endl;
                sendPacketDrop(false);
            }
            iter->second.numDropsBecauseNewTsnGreaterThanHighestTsn++;
            return SCTP_E_SEND;
            //          }  ????
        }
        else if ((tsn < state->gapList.getHighestTsnReceived()) &&
                 (state->disableReneging == false) &&
                 (!makeRoomForTsn(tsn, dataChunk->getByteLength() - SCTP_DATA_CHUNK_LENGTH * 8, dataChunk->getUBit())))
        {
            if ((!state->pktDropSent) && (sctpMain->pktdrop) && (state->peerPktDrop)) {
                EV_DETAIL << "Receive buffer full (case 2): sendPacketDrop" << endl;
                sendPacketDrop(false);
            }
            iter->second.numDropsBecauseNoRoomInBuffer++;
            return SCTP_E_SEND;
        }
    }

    // ====== Update of CumAckTsn ============================================
    state->gapList.updateGapList(tsn, state->newChunkReceived,
            (state->disableReneging == false) ? true : false);
    state->gapList.tryToAdvanceCumAckTsn();
    EV_DETAIL << "cumAckTsn=" << state->gapList.getCumAckTsn()
              << " highestTsnReceived=" << state->gapList.getHighestTsnReceived() << endl;

    // ====== Silly Window Syndrome Avoidance ================================
    if (state->swsAvoidanceInvoked) {
        // swsAvoidanceInvoked => schedule a SACK to be sent at once in this case
        EV_TRACE << "swsAvoidanceInvoked" << endl;
        state->ackState = sackFrequency;
    }

    if (dataChunk->getSid() >= inboundStreams) {
        sendInvalidStreamError(dataChunk->getSid());
        return SCTP_E_IGNORE;
    }
    // ====== Enqueue new chunk ==============================================
    SctpEventCode event = SCTP_E_SEND;
    if (state->newChunkReceived) {
        auto iter = receiveStreams.find(dataChunk->getSid());
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
                    for (uint16_t i = 0; i < inboundStreams; i++) {
                        if (!containsKey(streamThroughputVectors, i)) {
                            char vectorName[128];
                            snprintf(vectorName, sizeof(vectorName), "Stream %d Throughput", i);
                            streamThroughputVectors[i] = new cOutVector(vectorName);
                        }

                        streamThroughputVectors[i]->record(state->streamThroughput[i]
                                / (simTime() - state->lastThroughputTime) / 1024);
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

    return event;
}

SctpEventCode SctpAssociation::processHeartbeatAckArrived(SctpHeartbeatAckChunk *hback,
        SctpPathVariables *path)
{
    path->numberOfHeartbeatAcksRcvd++;
    path->vectorPathRcvdHbAck->record(path->numberOfHeartbeatAcksRcvd);
    /* hb-ack goes to pathmanagement, reset error counters, stop timeout timer */
    const L3Address addr = hback->getRemoteAddr();
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
        path->heartbeatTimeout = sctpMain->getHbInterval() + path->pathRto;
    }

    if (path->activePath == false) {
        EV_INFO << "HB ACK arrived activePath=false. remoteAddress=" << path->remoteAddress
                << " initialPP=" << state->initialPrimaryPath << endl;
        path->activePath = true;
        if (state->reactivatePrimaryPath && path->remoteAddress == state->initialPrimaryPath) {
            state->setPrimaryPath(path);
        }
        EV_DETAIL << "primaryPath now " << state->getPrimaryPathIndex() << endl;
    }
    EV_INFO << "Received HB ACK chunk...resetting error counters on path " << addr
            << ", rttEstimation=" << rttEstimation << endl;
    path->pathErrorCount = 0;
    return SCTP_E_IGNORE;
}

void SctpAssociation::processOutgoingResetRequestArrived(SctpOutgoingSsnResetRequestParameter *requestParam)
{
    EV_TRACE << "processOutgoingResetRequestArrived\n";
    if (!state->firstPeerRequest && (requestParam->getSrReqSn() < (state->peerRequestSn))) {
        // Retransmission
        sendStreamResetResponse(requestParam->getSrReqSn(), NO_RESET);
        return;
    }

    if (state->findRequestNum(requestParam->getSrResSn())) {
        state->requests[requestParam->getSrResSn()].result = PERFORMED;
        state->numResetRequests--;
        state->waitForResponse = false;
    }
    if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
        if (state->numResetRequests == 0) {
            SctpResetTimer *tm = check_and_cast<SctpResetTimer *>(PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
            if ((tm->getOutSN() == requestParam->getSrResSn() && !tm->getOutAcked()) ||
                (tm->getInSN() == requestParam->getSrResSn() && !tm->getInAcked()))
            {
                stopTimer(getPath(remoteAddr)->ResetTimer);
                delete state->resetChunk;
                state->resetChunk = nullptr;
                state->resetPending = false;
                state->localRequestType = 0;
                state->stopReceiving = false;
            }
            delete tm;
        }
    }
    if (requestParam->getStreamNumbersArraySize() > 0) {
        uint16_t count = 0;
        for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
            if (!receiveStreamPresent(requestParam->getStreamNumbers(i))) {
                sendStreamResetResponse(requestParam->getSrReqSn(), DENIED);
                return;
            }
            if (getExpectedSsnOfStream(requestParam->getStreamNumbers(i)) != 0) {
                count++;
            }
        }
        if (count == 0 && !(tsnGt(requestParam->getLastTsn(), state->gapList.getHighestTsnReceived()))) {
            sendStreamResetResponse(requestParam->getSrReqSn(), NOTHING_TO_DO);
            return;
        }
    }
    if (state->streamReset && !(tsnGt(requestParam->getLastTsn(), state->gapList.getHighestTsnReceived()))) {
        if (requestParam->getStreamNumbersArraySize() > 0) {
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                resetExpectedSsn(requestParam->getStreamNumbers(i));
            }
        }
        else {
            resetExpectedSsns();
        }
        EV_DETAIL << "processOutgoingResetRequestArrived: resetExpectedSsns\n";
        if (state->sendResponse == PERFORMED) {
            sendStreamResetResponse(requestParam->getSrReqSn(), PERFORMED);
            state->sendResponse = 0;
        }
        else {
            state->sendResponse = PERFORMED;
            state->responseSn = requestParam->getSrReqSn();
        }
    }
    else if (tsnGt(requestParam->getLastTsn(), state->gapList.getHighestTsnReceived())) {
        state->lastTsnBeforeReset = requestParam->getLastTsn();
        state->peerRequestSn = requestParam->getSrReqSn();
        sendStreamResetResponse(requestParam->getSrReqSn(), DEFERRED);
        state->incomingRequest = requestParam->dup();
        state->incomingRequestSet = true;
        state->resetDeferred = true;
//        state->firstPeerRequest = false;
    }
    else {
        sendStreamResetResponse(requestParam->getSrReqSn(), PERFORMED);
    }
}

void SctpAssociation::processIncomingResetRequestArrived(SctpIncomingSsnResetRequestParameter *requestParam)
{
    uint16_t num = 0;
    if (!state->firstPeerRequest && (requestParam->getSrReqSn() < (state->peerRequestSn))) {
        // Retransmission
        sendStreamResetResponse(requestParam->getSrReqSn(), NO_RESET);
        return;
    }
    if (!state->fragInProgress && state->outstandingBytes == 0) {
        if (requestParam->getStreamNumbersArraySize() > 0) {
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                if (!sendStreamPresent(requestParam->getStreamNumbers(i))) {
                    sendStreamResetResponse(requestParam->getSrReqSn(), DENIED);
                    return;
                }
                if (getSsnOfStream(requestParam->getStreamNumbers(i)) == 0) {
                    num++;
                }
            }
            if (num == requestParam->getStreamNumbersArraySize()) {
                sendStreamResetResponse(requestParam->getSrReqSn(), NOTHING_TO_DO);
                return;
            }
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                if (!state->findPeerStreamToReset(requestParam->getStreamNumbers(i))) {

//                    resetSsn(requestParam->getStreamNumbers(i));
                    state->peerStreamsToReset.push_back(requestParam->getStreamNumbers(i));
                }
            }
        }
        else {
            resetSsns();
        }
        sendOutgoingResetRequest(requestParam);
//        sendBundledOutgoingResetAndResponse(requestParam);
        EV_TRACE << "processIncomingResetRequestArrived: sendOutgoingResetRequestArrived returned\n";
        state->resetPending = true;
    }
    else {
        if (requestParam->getStreamNumbersArraySize() > 0) {
            state->streamsPending.clear();
            state->streamsToReset.clear();
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                if (!sendStreamPresent(requestParam->getStreamNumbers(i))) {
                    sendStreamResetResponse(requestParam->getSrReqSn(), DENIED);
                    return;
                }
                if ((getBytesInFlightOfStream(requestParam->getStreamNumbers(i)) > 0) || getFragInProgressOfStream(requestParam->getStreamNumbers(i))) {
                    state->streamsPending.push_back(requestParam->getStreamNumbers(i));
                }
                else {
                    state->streamsToReset.push_back(requestParam->getStreamNumbers(i));
                    resetSsn(requestParam->getStreamNumbers(i));
                }
            }
        }
        else {
            for (uint16_t i = 0; i < outboundStreams; i++) {
                if (getBytesInFlightOfStream(i) > 0) {
                    state->streamsPending.push_back(i);
                }
                else {
                    state->streamsToReset.push_back(i);
                    resetSsn(i);
                }
            }
        }
        if (state->streamsToReset.size() > 0) {
//            sendBundledOutgoingResetAndResponse(requestParam);
            sendOutgoingResetRequest(requestParam);
            state->resetPending = true;
        }
        if (!state->resetPending) {
            sendStreamResetResponse(requestParam->getSrReqSn(), DEFERRED);
            state->resetDeferred = true;
        }
        state->peerRequestSn = requestParam->getSrReqSn();
        state->peerRequestType = RESET_INCOMING;
        state->localRequestType = RESET_OUTGOING;
        if (state->streamsPending.size() == 0 && state->incomingRequestSet && state->incomingRequest != nullptr) {
            delete state->incomingRequest;
            state->incomingRequest = nullptr;
            state->incomingRequestSet = false;
        }
        state->incomingRequest = ((SctpParameter *)requestParam)->dup(); // FIXME is the c-style conversion need here?, duplicate only SctpParameter part!!!
//        state->incomingRequest->setName("StateIncoming");
        state->incomingRequestSet = true;
    }
}

void SctpAssociation::processSsnTsnResetRequestArrived(SctpSsnTsnResetRequestParameter *requestParam)
{
    EV_TRACE << "processSSNTsnResetRequestArrived\n";
    if (!state->firstPeerRequest && (requestParam->getSrReqSn() < (state->peerRequestSn))) {
        // Retransmission
        sendStreamResetResponse(requestParam->getSrReqSn(), NO_RESET);
        return;
    }
    if (!state->fragInProgress && state->outstandingBytes == 0) {
        state->advancedPeerAckPoint = state->nextTsn - 1;
        state->stopOldData = true;
//        resetExpectedSsns();
        if (state->sendResponse == PERFORMED) {
            sendStreamResetResponse(requestParam, PERFORMED, true);
            state->sendResponse = 0;
        }
        else {
            state->sendResponse = PERFORMED_WITH_OPTION;
            state->responseSn = requestParam->getSrReqSn();
            state->peerRequestType = SSN_TSN;
            state->incomingRequest = requestParam->dup();
            state->incomingRequestSet = true;
        }
    }
    else {
        sendStreamResetResponse(requestParam, DEFERRED, true);
        state->incomingRequest = requestParam->dup();
        state->incomingRequestSet = true;
//        state->incomingRequest->setName("SSNDeferred");
        state->peerRequestSn = requestParam->getSrReqSn();
//        state->firstPeerRequest = false;
        state->peerRequestType = SSN_TSN;
        state->resetDeferred = true;
    }
}

void SctpAssociation::processResetResponseArrived(SctpStreamResetResponseParameter *responseParam)
{
    EV_INFO << "processResetResponseArrived \n";
    if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
        if (getPath(remoteAddr)->ResetTimer->hasEncapsulatedPacket() &&
            (state->numResetRequests == 0 || (state->getNumRequestsNotPerformed() == 1 && responseParam->getResult() != DEFERRED)))
        {
            SctpResetTimer *tm = check_and_cast<SctpResetTimer *>(getPath(remoteAddr)->ResetTimer->decapsulate());
            EV_INFO << "SrResSn=" << responseParam->getSrResSn() << " tmOut=" << tm->getOutSN() << " tmIn= " << tm->getInSN() << "\n";
            if (tm->getOutSN() == responseParam->getSrResSn() || tm->getInSN() == responseParam->getSrResSn() || responseParam->getResult() > DEFERRED) {
                stopTimer(getPath(remoteAddr)->ResetTimer);
                delete state->resetChunk;
                state->resetChunk = nullptr;
                getPath(remoteAddr)->pathErrorCount = 0;
                if (state->streamsPending.size() == 0) {
                    delete state->resetInfo;
                    state->resetInfo = nullptr;
                }
            }
            delete tm;
            if (responseParam->getResult() > DEFERRED) {
                state->resetPending = false;
                state->resetRequested = false;
                state->waitForResponse = false;
                state->streamsPending.clear();
                state->resetOutStreams.clear();
                state->resetInStreams.clear();
                return;
            }
        }
    }
    else {
        if ((PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket())) {
            delete (PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
        }
    }
    if (state->requests[responseParam->getSrResSn()].result != PERFORMED) {
        if (responseParam->getResult() == PERFORMED) {
            if (state->resetRequested) {
                if (state->resetOutStreams.size() > 0) {
                    for (auto& elem : state->resetOutStreams) {
                        resetSsn((elem));
                    }
                    state->resetOutStreams.clear();
                }
                else if (state->numAddedOutStreams == 0 && state->numAddedInStreams == 0) {
                    resetSsns();
                }
                if (state->streamsPending.size() == 0)
                    state->resetRequested = false;
            }
            state->resetPending = false;
            state->waitForResponse = false;
            if (responseParam->getReceiversNextTsn() != 0) {
                state->nextTsn = responseParam->getReceiversNextTsn();
                state->lastTsnAck = responseParam->getReceiversNextTsn() - 1;
                state->gapList.forwardCumAckTsn(responseParam->getSendersNextTsn() - 1);
                state->peerTsnAfterReset = responseParam->getSendersNextTsn();
                state->stopReceiving = false;
                state->stopOldData = true;
                if (state->resetInStreams.size() > 0) {
                    for (auto& elem : state->resetInStreams) {
                        resetExpectedSsn((elem));
                    }
                    state->resetInStreams.clear();
                }
                else {
                    resetExpectedSsns();
                }
//                sendSack();
            }
            if (state->localRequestType == ADD_BOTH) {
                if (state->numAddedOutStreams > 0) {
                    outboundStreams += state->numAddedOutStreams;
                    (this->*ssFunctions.ssAddOutStreams)(state->numAddedOutStreams);
                    state->numAddedOutStreams = 0;
                }
                else if (state->numAddedInStreams > 0) {
                    inboundStreams += state->numAddedInStreams;
                    (this->*ssFunctions.ssAddInStreams)(state->numAddedInStreams);
                    state->numAddedInStreams = 0;
                }
            }
            else if (state->localRequestType == ADD_INCOMING) {
                inboundStreams += state->numAddedInStreams;
                (this->*ssFunctions.ssAddInStreams)(state->numAddedInStreams);
                state->numAddedInStreams = 0;
            }
            else if (state->localRequestType == ADD_OUTGOING) {
                outboundStreams += state->numAddedOutStreams;
                (this->*ssFunctions.ssAddOutStreams)(state->numAddedOutStreams);
                state->numAddedOutStreams = 0;
            }
            sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
            auto it = sctpMain->assocStatMap.find(assocId);
            it->second.numResetRequestsPerformed++;
        }
        else {
            EV_INFO << "Reset Request failed. Send indication to app.\n";
            if (responseParam->getResult() == DEFERRED) {
                state->numResetRequests++;
            }
            else {
                state->resetPending = false;
                sendIndicationToApp(SCTP_I_RESET_REQUEST_FAILED);
            }
        }
        state->requests[responseParam->getSrResSn()].result = responseParam->getResult();
    }
    if (state->incomingRequestSet && state->incomingRequest != nullptr && state->streamsPending.size() == 0) {
        delete state->incomingRequest;
        state->incomingRequest = nullptr;
        state->incomingRequestSet = false;
    }
}

SctpEventCode SctpAssociation::processInAndOutResetRequestArrived(SctpIncomingSsnResetRequestParameter *inRequestParam,
        SctpOutgoingSsnResetRequestParameter *outRequestParam)
{
    uint32_t inSrSn = 0;
    uint32_t outSrSn = 0;
    uint16_t inRes = 0;
    uint16_t outRes = 0;
    uint16_t num = 0;
    if (tsnGt(outRequestParam->getLastTsn(), state->gapList.getHighestTsnReceived())) {
        state->lastTsnBeforeReset = outRequestParam->getLastTsn();
        state->peerRequestSn = outRequestParam->getSrReqSn();
        state->inRequestSn = inRequestParam->getSrReqSn();
        state->inOut = true;
    }
    else {
        if (outRequestParam->getStreamNumbersArraySize() > 0) {
            num = 0;
            for (uint16_t i = 0; i < outRequestParam->getStreamNumbersArraySize(); i++) {
                if (!receiveStreamPresent(outRequestParam->getStreamNumbers(i))) {
                    outSrSn = outRequestParam->getSrReqSn();
                    outRes = DENIED;
                    num++;
                    break;
                }
            }
            if (num == 0) {
                for (uint16_t i = 0; i < outRequestParam->getStreamNumbersArraySize(); i++) {
                    resetExpectedSsn(outRequestParam->getStreamNumbers(i));
                }
            }
        }
        else {
            resetExpectedSsns();
            EV_DETAIL << "processInAndOutResetRequestArrived: resetExpectedSsns\n";
            sendOutgoingRequestAndResponse(inRequestParam->getSrReqSn(), outRequestParam->getSrReqSn());
        }
        if (inRequestParam->getStreamNumbersArraySize() > 0) {
            num = 0;
            for (uint16_t i = 0; i < inRequestParam->getStreamNumbersArraySize(); i++) {
                if (!receiveStreamPresent(inRequestParam->getStreamNumbers(i))) {
                    inSrSn = inRequestParam->getSrReqSn();
                    inRes = DENIED;
                    num++;
//                    sendStreamResetResponse(outRequestParam->getSrReqSn(), DENIED);
                    break;
                }
            }
            if (num == 0) {
                sendOutgoingRequestAndResponse(inRequestParam, outRequestParam);
            }
        }
    }
    if (inSrSn > 0 && outSrSn > 0) {
        sendDoubleStreamResetResponse(inSrSn, inRes, outSrSn, outRes);
        state->resetPending = true;
    }
    return SCTP_E_IGNORE;
}

void SctpAssociation::processAddInAndOutResetRequestArrived(const SctpAddStreamsRequestParameter *addInRequestParam,
        SctpAddStreamsRequestParameter *addOutRequestParam)
{
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk("AddInOut_CONFIG");
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32_t srsn = state->streamResetSequenceNumber;
    SctpResetTimer *rt = new SctpResetTimer();
    auto it = sctpMain->assocStatMap.find(assocId);
    SctpAddStreamsRequestParameter *addStreams = new SctpAddStreamsRequestParameter("Add_Streams");
    /*  SctpAddStreamsRequestParameter *addStreams = new SctpAddStreamsRequestParameter();*/
    addStreams->setParameterType(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
    addStreams->setNumberOfStreams(addInRequestParam->getNumberOfStreams());
    state->numAddedOutStreams = addStreams->getNumberOfStreams();
    state->localRequestType = ADD_OUTGOING;
    addStreams->setSrReqSn(srsn);
    state->requests[srsn].result = 100;
    state->requests[srsn].type = ADD_OUTGOING_STREAMS_REQUEST_PARAMETER;
    addStreams->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
    resetChunk->addParameter(addStreams);

    SctpStreamResetChunk *responseChunk = new SctpStreamResetChunk("responseRE_CONFIG");
    responseChunk->setSctpChunkType(RE_CONFIG);
    responseChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SctpStreamResetResponseParameter *outResponseParam = new SctpStreamResetResponseParameter("Out_Response_Param");
    /*  SctpStreamResetResponseParameter *outResponseParam = new SctpStreamResetResponseParameter();*/
    outResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    outResponseParam->setSrResSn(addOutRequestParam->getSrReqSn());
    outResponseParam->setResult(PERFORMED);
    outResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    responseChunk->addParameter(outResponseParam);
    SctpStreamResetResponseParameter *inResponseParam = new SctpStreamResetResponseParameter("In_Response_Param");
    /* SctpStreamResetResponseParameter *inResponseParam = new SctpStreamResetResponseParameter();*/
    inResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    inResponseParam->setSrResSn(addInRequestParam->getSrReqSn());
    inResponseParam->setResult(PERFORMED);
    state->peerRequests[addInRequestParam->getSrReqSn()].result = PERFORMED;
    state->peerRequests[addOutRequestParam->getSrReqSn()].result = PERFORMED;
    inResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    responseChunk->addParameter(inResponseParam);
    msg->appendSctpChunks(resetChunk);
    msg->appendSctpChunks(responseChunk);
    rt->setInSN(0);
    rt->setInAcked(true);
    rt->setOutSN(srsn);
    rt->setOutAcked(false);
    it->second.numResetRequestsSent++;
    state->streamResetSequenceNumber = ++srsn;
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
//    state->resetChunk->setName("stateAddResetChunk");
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

SctpEventCode SctpAssociation::processOutAndResponseArrived(SctpOutgoingSsnResetRequestParameter *outRequestParam,
        SctpStreamResetResponseParameter *responseParam)
{
    EV_INFO << "processOutAndResponseArrived\n";
    if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
        if (state->numResetRequests == 0) {
            SctpResetTimer *tm = check_and_cast<SctpResetTimer *>(getPath(remoteAddr)->ResetTimer->decapsulate());
            if (tm->getOutSN() == responseParam->getSrResSn()) {
                stopTimer(getPath(remoteAddr)->ResetTimer);
                delete state->resetChunk;
                state->resetChunk = nullptr;
            }
            delete tm;
        }
    }
    if (state->requests[outRequestParam->getSrResSn()].result != PERFORMED) {
        if (outRequestParam->getStreamNumbersArraySize() > 0) {
            for (uint16_t i = 0; i < outRequestParam->getStreamNumbersArraySize(); i++) {
                resetExpectedSsn(outRequestParam->getStreamNumbers(i));
            }
        }
        else {
            resetExpectedSsns();
        }
        state->requests[outRequestParam->getSrResSn()].result = PERFORMED;
    }
    sendStreamResetResponse(outRequestParam->getSrReqSn(), PERFORMED);
    if (responseParam->getResult() == PERFORMED) {
        if (state->requests[responseParam->getSrResSn()].result != PERFORMED) {
            std::list<uint16_t>::iterator iter;
            if (state->requests[responseParam->getSrResSn()].streams.size() > 0) {
                for (iter = state->requests[responseParam->getSrResSn()].streams.begin(); iter != state->requests[responseParam->getSrResSn()].streams.end(); iter++) {
                    resetSsn((*iter));
                }
            }
            else {
                resetSsns();
            }
//            resetExpectedSsns();
            state->resetPending = false;
            sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
            auto it = sctpMain->assocStatMap.find(assocId);
            it->second.numResetRequestsPerformed++;
        }
    }
    else {
        EV_INFO << "Reset Request failed. Send indication to app.\n";
        state->resetPending = false;
        sendIndicationToApp(SCTP_I_RESET_REQUEST_FAILED);
    }
    state->requests[responseParam->getSrResSn()].result = responseParam->getResult();
    return SCTP_E_IGNORE;
}

SctpEventCode SctpAssociation::processStreamResetArrived(SctpStreamResetChunk *resetChunk)
{
    EV_INFO << "processStreamResetArrived\n";
    SctpParameter *parameter, *nextParam;
    std::map<uint32_t, SctpStateVariables::RequestData>::reverse_iterator rit;
    uint32_t numberOfParameters = resetChunk->getParametersArraySize();
    if (numberOfParameters == 0)
        return SCTP_E_IGNORE;
    for (uint16_t i = 0; i < numberOfParameters; i++) {
        parameter = (SctpParameter *)(resetChunk->getParameters(i));
        switch (parameter->getParameterType()) {
            case OUTGOING_RESET_REQUEST_PARAMETER: {
                SctpOutgoingSsnResetRequestParameter *outRequestParam;
                outRequestParam = check_and_cast<SctpOutgoingSsnResetRequestParameter *>(parameter);
                if (state->findPeerRequestNum(outRequestParam->getSrReqSn())) {
                    // retransmission
                    if (state->peerRequests[outRequestParam->getSrReqSn()].type != outRequestParam->getParameterType()) {
                        sendStreamResetResponse(outRequestParam->getSrReqSn(), NO_RESET);
                        return SCTP_E_IGNORE;
                    }
                    if (state->peerRequests[outRequestParam->getSrReqSn()].result != 100) {
                        std::map<uint32_t, SctpStateVariables::RequestData>::reverse_iterator rit;
                        rit = state->peerRequests.rbegin();
                        if (state->peerRequests[outRequestParam->getSrReqSn()].sn == rit->first) {
                            // send response with same result
                            sendStreamResetResponse(outRequestParam->getSrReqSn(), state->peerRequests[outRequestParam->getSrReqSn()].result);
                            return SCTP_E_IGNORE;
                        }
                        else {
                            sendStreamResetResponse(outRequestParam->getSrReqSn(), NO_RESET);
                            return SCTP_E_IGNORE;
                        }
                    }
                }
                if (state->firstPeerRequest && outRequestParam->getSrReqSn() != state->expectedStreamResetSequenceNumber) {
                    if (state->peerRequests.size() > 0) {
                        sendStreamResetResponse(outRequestParam->getSrReqSn(), REQUEST_IN_PROGRESS);
                    }
                    else {
                        sendStreamResetResponse(outRequestParam->getSrReqSn(), NO_RESET);
                    }
                    break;
                }
                state->peerRequests[outRequestParam->getSrReqSn()].sn = outRequestParam->getSrReqSn();
                state->peerRequests[outRequestParam->getSrReqSn()].result = 100;
                state->peerRequests[outRequestParam->getSrReqSn()].type = outRequestParam->getParameterType();
                if (numberOfParameters > i + 1u) {
                    nextParam = (SctpParameter *)(resetChunk->getParameters(i + 1)); // FIXME const_cast
                    if (nextParam->getParameterType() != INCOMING_RESET_REQUEST_PARAMETER &&
                        nextParam->getParameterType() != STREAM_RESET_RESPONSE_PARAMETER)
                    {
                        processOutgoingResetRequestArrived(outRequestParam);
                        delete nextParam;
                    }
                    else {
                        switch (nextParam->getParameterType()) {
                            case INCOMING_RESET_REQUEST_PARAMETER:
                                SctpIncomingSsnResetRequestParameter *inRequestParam;
                                inRequestParam = check_and_cast<SctpIncomingSsnResetRequestParameter *>(nextParam);
                                state->peerRequests[inRequestParam->getSrReqSn()].sn = inRequestParam->getSrReqSn();
                                state->peerRequests[inRequestParam->getSrReqSn()].result = 100;
                                state->peerRequests[inRequestParam->getSrReqSn()].type = inRequestParam->getParameterType();
                                processInAndOutResetRequestArrived(inRequestParam, outRequestParam);
                                i++;
                                break;

                            case STREAM_RESET_RESPONSE_PARAMETER:
                                SctpStreamResetResponseParameter *responseParam;
                                responseParam = check_and_cast<SctpStreamResetResponseParameter *>(nextParam);
                                state->numResetRequests -= 2;
                                processOutAndResponseArrived(outRequestParam, responseParam);
                                i++;
                                break;
                        }
                    }
                }
                else {
                    processOutgoingResetRequestArrived(outRequestParam);
                }
                break;
            }

            case INCOMING_RESET_REQUEST_PARAMETER: {
                SctpIncomingSsnResetRequestParameter *inRequestParam;
                inRequestParam = check_and_cast<SctpIncomingSsnResetRequestParameter *>(parameter);
                rit = state->peerRequests.rbegin();
                if (state->firstPeerRequest && (inRequestParam->getSrReqSn() != state->expectedStreamResetSequenceNumber)) {
                    sendStreamResetResponse(inRequestParam->getSrReqSn(), NO_RESET);
                    return SCTP_E_IGNORE;
                }
                if (!state->firstPeerRequest &&
                    (((inRequestParam->getSrReqSn() < (state->peerRequestSn)) &&
                      (inRequestParam->getSrReqSn() != rit->first + 1)) ||
                     (state->findPeerRequestNum(inRequestParam->getSrReqSn()) && state->peerRequests[inRequestParam->getSrReqSn()].type != inRequestParam->getParameterType())))
                {
                    sendStreamResetResponse(inRequestParam->getSrReqSn(), NO_RESET);
                    return SCTP_E_IGNORE;
                }
                rit = state->requests.rbegin();
                if (rit->second.type == OUTGOING_RESET_REQUEST_PARAMETER && (rit->second.result == 100 || rit->second.result == PERFORMED)) {
                    state->requestsOverlap = true;
                    if (state->requests[rit->first].lastTsn == state->nextTsn - 1) {
                        if (inRequestParam->getStreamNumbersArraySize() > 0) {
                            uint16_t match = 0;
                            for (uint i = 0; i < inRequestParam->getStreamNumbersArraySize(); i++) {
                                std::list<uint16_t>::iterator it;
                                for (it = state->requests[rit->first].streams.begin(); it != state->requests[rit->first].streams.end(); it++) {
                                    if ((*it) == inRequestParam->getStreamNumbers(i))
                                        match++;
                                }
                            }
                            if (match == inRequestParam->getStreamNumbersArraySize()) {
                                sendStreamResetResponse(inRequestParam->getSrReqSn(), NOTHING_TO_DO);
                                return SCTP_E_IGNORE;
                            }
                        }
                        else {
                            sendStreamResetResponse(inRequestParam->getSrReqSn(), NOTHING_TO_DO);
                            return SCTP_E_IGNORE;
                        }
                    }
                }
                state->peerRequests[inRequestParam->getSrReqSn()].sn = inRequestParam->getSrReqSn();
                state->peerRequests[inRequestParam->getSrReqSn()].result = 100;
                state->peerRequests[inRequestParam->getSrReqSn()].type = inRequestParam->getParameterType();
                if (numberOfParameters > i + 1u) {
                    nextParam = (SctpParameter *)(resetChunk->getParameters(i + 1));
                    if (nextParam->getParameterType() != OUTGOING_RESET_REQUEST_PARAMETER) {
                        processIncomingResetRequestArrived(inRequestParam);
                    }
                    else {
                        if (nextParam->getParameterType() == OUTGOING_RESET_REQUEST_PARAMETER) {
                            i++;
                            SctpOutgoingSsnResetRequestParameter *outRequestParam;
                            outRequestParam = check_and_cast<SctpOutgoingSsnResetRequestParameter *>(nextParam->dup());
                            state->peerRequests[outRequestParam->getSrReqSn()].sn = outRequestParam->getSrReqSn();
                            state->peerRequests[outRequestParam->getSrReqSn()].result = 100;
                            state->peerRequests[outRequestParam->getSrReqSn()].type = outRequestParam->getParameterType();
                            processInAndOutResetRequestArrived(inRequestParam, outRequestParam);
                            delete outRequestParam;
                        }
                    }
                }
                else {
                    processIncomingResetRequestArrived(inRequestParam);
                }
                break;
            }

            case SSN_TSN_RESET_REQUEST_PARAMETER:
                SctpSsnTsnResetRequestParameter *ssnRequestParam;
                ssnRequestParam = check_and_cast<SctpSsnTsnResetRequestParameter *>(parameter);
                rit = state->peerRequests.rbegin();
                if (state->firstPeerRequest && ssnRequestParam->getSrReqSn() != state->expectedStreamResetSequenceNumber) {
                    if (state->peerRequests.size() > 0) {
                        sendStreamResetResponse(ssnRequestParam->getSrReqSn(), REQUEST_IN_PROGRESS);
                    }
                    else {
                        sendStreamResetResponse(ssnRequestParam->getSrReqSn(), NO_RESET);
                    }
                    break;
                }
                if (!state->firstPeerRequest &&
                    ((((ssnRequestParam->getSrReqSn() < (state->peerRequestSn)) && (ssnRequestParam->getSrReqSn() != rit->first + 1))) ||
                     ((state->findPeerRequestNum(ssnRequestParam->getSrReqSn())) &&
                      state->peerRequests[ssnRequestParam->getSrReqSn()].type != ssnRequestParam->getParameterType())))
                {
                    sendStreamResetResponse(ssnRequestParam->getSrReqSn(), NO_RESET);
                    return SCTP_E_IGNORE;
                }
                if (state->nextTsn == state->streamResetSequenceNumber && state->gapList.getCumAckTsn() < initPeerTsn) {
                    sendStreamResetResponse(ssnRequestParam, NOTHING_TO_DO, true);
                    break;
                }
                state->peerRequests[ssnRequestParam->getSrReqSn()].sn = ssnRequestParam->getSrReqSn();
                state->peerRequests[ssnRequestParam->getSrReqSn()].result = 100;
                state->peerRequests[ssnRequestParam->getSrReqSn()].type = ssnRequestParam->getParameterType();
                processSsnTsnResetRequestArrived(ssnRequestParam);
                break;

            case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                SctpAddStreamsRequestParameter *addStreamsParam =
                    check_and_cast<SctpAddStreamsRequestParameter *>(parameter);
                if (addStreamsParam->getNumberOfStreams() == 0) {
                    sendStreamResetResponse(addStreamsParam->getSrReqSn(), NOTHING_TO_DO);
                    return SCTP_E_IGNORE;
                }
                rit = state->peerRequests.rbegin();
                if ((state->firstPeerRequest && addStreamsParam->getSrReqSn() != state->expectedStreamResetSequenceNumber) ||
                    (!state->firstPeerRequest && ((addStreamsParam->getSrReqSn() < (state->peerRequestSn)) &&
                                                  (addStreamsParam->getSrReqSn() != rit->first + 1))) ||
                    (state->findPeerRequestNum(addStreamsParam->getSrReqSn()) &&
                     state->peerRequests[addStreamsParam->getSrReqSn()].type != addStreamsParam->getParameterType()))
                {
                    sendStreamResetResponse(addStreamsParam->getSrReqSn(), NO_RESET);
                    return SCTP_E_IGNORE;
                }
                if (!(addStreamsParam->getNumberOfStreams() + outboundStreams <= 65535)) {
                    sendStreamResetResponse(addStreamsParam->getSrReqSn(), DENIED);
                    return SCTP_E_IGNORE;
                }
                state->peerRequests[addStreamsParam->getSrReqSn()].sn = addStreamsParam->getSrReqSn();
                state->peerRequests[addStreamsParam->getSrReqSn()].result = 100;
                state->peerRequests[addStreamsParam->getSrReqSn()].type = addStreamsParam->getParameterType();
                if (numberOfParameters > i + 1u) {
                    nextParam = (SctpParameter *)(resetChunk->getParameters(i + 1));
                    if (nextParam->getParameterType() == ADD_OUTGOING_STREAMS_REQUEST_PARAMETER) {
                        SctpAddStreamsRequestParameter *addOutStreamsParam;
                        addOutStreamsParam = check_and_cast<SctpAddStreamsRequestParameter *>(nextParam);
                        state->peerRequests[addOutStreamsParam->getSrReqSn()].sn = addOutStreamsParam->getSrReqSn();
                        state->peerRequests[addOutStreamsParam->getSrReqSn()].result = 100;
                        state->peerRequests[addOutStreamsParam->getSrReqSn()].type = addOutStreamsParam->getParameterType();
                        processAddInAndOutResetRequestArrived(addStreamsParam, addOutStreamsParam);
                        i++;
                    }
                }
                else {
                    state->peerRequestSn = addStreamsParam->getSrReqSn();
                    state->peerRequestType = ADD_INCOMING;
                    state->incomingRequest = ((SctpParameter *)addStreamsParam)->dup(); // FIXME is the c-style conversion need here?, this is not a correct dup, duplicate only the SctpParameter part of addStreamsParam
                    state->incomingRequestSet = true;
//                    state->incomingRequest->setName("stateIncoming");
                    state->sendResponse = PERFORMED_WITH_ADDOUT;
                }
                break;
            }

            case ADD_OUTGOING_STREAMS_REQUEST_PARAMETER: {
                SctpAddStreamsRequestParameter *addOutStreamsParam =
                    check_and_cast<SctpAddStreamsRequestParameter *>(parameter);
                if (addOutStreamsParam->getNumberOfStreams() == 0) {
                    sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), NOTHING_TO_DO);
                    return SCTP_E_IGNORE;
                }
                rit = state->peerRequests.rbegin();
                if ((!(addOutStreamsParam->getNumberOfStreams() + inboundStreams <= 65535)) ||
                    (state->appLimited && (addOutStreamsParam->getNumberOfStreams() + inboundStreams > initInboundStreams)))
                {
                    sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), DENIED);
                    return SCTP_E_IGNORE;
                }
                if (state->findPeerRequestNum(addOutStreamsParam->getSrReqSn())) {
                    // retransmission
                    if (state->peerRequests[addOutStreamsParam->getSrReqSn()].type != addOutStreamsParam->getParameterType()) {
                        sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), NO_RESET);
                        return SCTP_E_IGNORE;
                    }
                    if (state->peerRequests[addOutStreamsParam->getSrReqSn()].result != 100) {
                        if (state->peerRequests[addOutStreamsParam->getSrReqSn()].sn == rit->first) {
                            // send response with same result
                            sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), state->peerRequests[addOutStreamsParam->getSrReqSn()].result);
                            return SCTP_E_IGNORE;
                        }
                        else {
                            sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), NO_RESET);
                            return SCTP_E_IGNORE;
                        }
                    }
                }
                if ((state->firstPeerRequest && addOutStreamsParam->getSrReqSn() != state->expectedStreamResetSequenceNumber) ||
                    (!state->firstPeerRequest && ((addOutStreamsParam->getSrReqSn() < (state->peerRequestSn)) &&
                                                  (addOutStreamsParam->getSrReqSn() != rit->first + 1))))
                {
                    // Retransmission
                    sendStreamResetResponse(addOutStreamsParam->getSrReqSn(), NO_RESET);
                    return SCTP_E_IGNORE;
                }
                state->peerRequests[addOutStreamsParam->getSrReqSn()].sn = addOutStreamsParam->getSrReqSn();
                state->peerRequests[addOutStreamsParam->getSrReqSn()].result = 100;
                state->peerRequests[addOutStreamsParam->getSrReqSn()].type = addOutStreamsParam->getParameterType();
                if (state->localRequestType == 0 || state->localRequestType == ADD_INCOMING || state->localRequestType == ADD_BOTH) {
                    inboundStreams += addOutStreamsParam->getNumberOfStreams();
                    if (state->resetRequested && state->numAddedInStreams > 0)
                        state->numAddedInStreams = 0;
                    (this->*ssFunctions.ssAddInStreams)(addOutStreamsParam->getNumberOfStreams());
                }
                if (numberOfParameters > i + 1u) {
                    nextParam = (SctpParameter *)(resetChunk->getParameters(i + 1));
                    if (nextParam->getParameterType() == ADD_INCOMING_STREAMS_REQUEST_PARAMETER) {
                        SctpAddStreamsRequestParameter *addInStreamsParam;
                        addInStreamsParam = check_and_cast<SctpAddStreamsRequestParameter *>(nextParam);
                        state->peerRequests[addInStreamsParam->getSrReqSn()].sn = addInStreamsParam->getSrReqSn();
                        state->peerRequests[addInStreamsParam->getSrReqSn()].result = 100;
                        state->peerRequests[addInStreamsParam->getSrReqSn()].type = addInStreamsParam->getParameterType();
                        processAddInAndOutResetRequestArrived(addInStreamsParam, addOutStreamsParam);
                        i++;
                    }
                    else {
                        state->peerRequestSn = addOutStreamsParam->getSrReqSn();
                        state->peerRequestType = ADD_OUTGOING;
                        state->sendResponse = PERFORMED;
                        state->responseSn = addOutStreamsParam->getSrReqSn();
                        state->firstPeerRequest = false;
//                        state->numAddedInStreams = addOutStreamsParam->getNumberOfStreams();
                        state->resetRequested = true;
                    }
                }
                else {
                    state->peerRequestSn = addOutStreamsParam->getSrReqSn();
                    state->peerRequestType = ADD_OUTGOING;
                    state->sendResponse = PERFORMED;
                    state->responseSn = addOutStreamsParam->getSrReqSn();
//                    state->numAddedInStreams = addOutStreamsParam->getNumberOfStreams();
                    state->firstPeerRequest = false;
                    state->resetRequested = true;
                }
                break;
            }

            case STREAM_RESET_RESPONSE_PARAMETER: {
                SctpStreamResetResponseParameter *responseParam;
                responseParam = check_and_cast<SctpStreamResetResponseParameter *>(parameter);
                if (!state->findRequestNum(responseParam->getSrResSn())) {
                    delete state->resetChunk;
                    state->resetChunk = nullptr;
                    break;
                }
                if (numberOfParameters > i + 1u) {
                    nextParam = (SctpParameter *)(resetChunk->getParameters(i + 1));
                    if (nextParam->getParameterType() != OUTGOING_RESET_REQUEST_PARAMETER) {
                        if (responseParam->getResult() != DEFERRED)
                            state->numResetRequests--;
                        processResetResponseArrived(responseParam);
                    }
                    else {
                        switch (nextParam->getParameterType()) {
                            case OUTGOING_RESET_REQUEST_PARAMETER: {
                                SctpOutgoingSsnResetRequestParameter *outRequestParam;
                                outRequestParam = check_and_cast<SctpOutgoingSsnResetRequestParameter *>(nextParam);
                                if ((state->firstPeerRequest && outRequestParam->getSrReqSn() != state->expectedStreamResetSequenceNumber) ||
                                    (!state->firstPeerRequest && (outRequestParam->getSrReqSn() < (state->peerRequestSn))))
                                {
                                    // Retransmission
                                    sendStreamResetResponse(outRequestParam->getSrReqSn(), NO_RESET);
                                    return SCTP_E_IGNORE;
                                }
                                state->peerRequests[outRequestParam->getSrReqSn()].sn = outRequestParam->getSrReqSn();
                                state->peerRequests[outRequestParam->getSrReqSn()].result = 100;
                                state->peerRequests[outRequestParam->getSrReqSn()].type = outRequestParam->getParameterType();
                                state->numResetRequests -= 2;
                                processOutAndResponseArrived(outRequestParam, responseParam);
                                i++;
                                break;
                            }
                        }
                    }
                }
                else {
                    if (responseParam->getResult() != DEFERRED)
                        state->numResetRequests--;
                    processResetResponseArrived(responseParam);
                }
                break;
            }
        }
    }
    return SCTP_E_IGNORE;
}

SctpEventCode SctpAssociation::processAsconfArrived(SctpAsconfChunk *asconfChunk)
{
    SctpParameter *sctpParam;
    SctpPathVariables *path;
    L3Address addr;
    std::vector<L3Address> locAddr;
    SctpAuthenticationChunk *authChunk;
    EV_INFO << "Asconf arrived " << asconfChunk->getName() << "\n";
//    SctpHeader *sctpAsconfAck = new SctpHeader("ASCONF_ACK");
    const auto& sctpAsconfAck = makeShared<SctpHeader>();
    sctpAsconfAck->setChunkLength(B(SCTP_COMMON_HEADER));
    sctpAsconfAck->setSrcPort(localPort);
    sctpAsconfAck->setDestPort(remotePort);
    if (state->auth && state->peerAuth) {
        authChunk = createAuthChunk();
        sctpAsconfAck->appendSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    if (state->numberAsconfReceived > 0 || (state->numberAsconfReceived == 0 && asconfChunk->getSerialNumber() == initPeerTsn + state->numberAsconfReceived)) {
        SctpAsconfAckChunk *asconfAckChunk = createAsconfAckChunk(asconfChunk->getSerialNumber());
        state->numberAsconfReceived++;
        int32_t count = asconfChunk->getAsconfParamsArraySize();
        EV_DETAIL << "Number of Asconf parameters=" << count << "\n";
        for (int32_t c = 0; c < count; c++) {
            sctpParam = (SctpParameter *)(asconfChunk->removeAsconfParam());
            switch (sctpParam->getParameterType()) {
                case ADD_IP_ADDRESS:
                    EV_INFO << "ADD_IP_PARAMETER\n";
                    SctpAddIPParameter *ipParam;
                    ipParam = check_and_cast<SctpAddIPParameter *>(sctpParam);
                    addr = ipParam->getAddressParam();
                    if (addr.isUnspecified()) {
                        EV_INFO << "no address specified, add natted address " << remoteAddr << "\n";
                        addr = remoteAddr;
                        sendIndicationToApp(SCTP_I_ADDRESS_ADDED);
                    }
                    for (auto& elem : state->localAddresses) {
                        if (sctpMain->addRemoteAddress(this, (elem), addr)) {
                            addPath(addr);
                            EV_INFO << "add remote address " << addr << " to local address " << (elem) << "\n";
                            this->remoteAddressList.push_back(addr);
                        }
                    }
                    path = getPath(addr);
                    if (sctpMain->getEnableHeartbeats()) {
                        stopTimer(path->HeartbeatTimer);
                        stopTimer(path->HeartbeatIntervalTimer);
                        path->statisticsPathRTO->record(path->pathRto);
                        startTimer(path->HeartbeatIntervalTimer, path->pathRto);
                        path->forceHb = true;
                    }
                    else
                        path->confirmed = true;
                    delete ipParam;
                    break;

                case DELETE_IP_ADDRESS:
                    SctpDeleteIPParameter *delParam;
                    delParam = check_and_cast<SctpDeleteIPParameter *>(sctpParam);
                    addr = delParam->getAddressParam();
                    if (state->localAddresses.size() == 1) {
                        SctpErrorCauseParameter *errorParam;
                        errorParam = new SctpErrorCauseParameter("ErrorCause");
                        errorParam->setParameterType(ERROR_CAUSE_INDICATION);
                        errorParam->setResponseCorrelationId(delParam->getRequestCorrelationId());
                        errorParam->setErrorCauseType(ERROR_DELETE_LAST_IP_ADDRESS);
                        errorParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 4);
                        errorParam->encapsulate(delParam->dup());
                        // FIXME is the c-style conversion need here?
                        asconfAckChunk->addAsconfResponse(errorParam);
                    }
                    else if (addr == remoteAddr) {
                        EV_INFO << "addr=remoteAddr, make Error Parameter\n";
                        SctpErrorCauseParameter *errParam;
                        errParam = new SctpErrorCauseParameter("ErrorCause");
//                        errParam = new SctpErrorCauseParameter();
                        errParam->setParameterType(ERROR_CAUSE_INDICATION);
                        errParam->setResponseCorrelationId(delParam->getRequestCorrelationId());
                        errParam->setErrorCauseType(ERROR_DELETE_SOURCE_ADDRESS);
                        errParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 4);
                        errParam->encapsulate(delParam->dup());
                        asconfAckChunk->addAsconfResponse(errParam);
                    }
                    else {
                        locAddr = state->localAddresses;
                        sctpMain->removeRemoteAddressFromAllAssociations(this, addr, locAddr);
                        removePath(addr);
                        EV_INFO << "remove path from address " << addr << "\n";
                        asconfAckChunk->addAsconfResponse(createSuccessIndication(delParam->getRequestCorrelationId()));
                    }
                    delete delParam;
                    break;

                case SET_PRIMARY_ADDRESS:
                    EV_INFO << "SET_PRIMARY_ADDRESS\n";
                    SctpSetPrimaryIPParameter *priParam;
                    priParam = check_and_cast<SctpSetPrimaryIPParameter *>(sctpParam);
                    addr = priParam->getAddressParam();
                    if (addr.isUnspecified()) {
                        EV_INFO << "no address specified, add natted address " << remoteAddr << "\n";
                        addr = remoteAddr;
                    }
                    for (auto& elem : remoteAddressList) {
                        if ((elem) == addr) {
                            if (getPath(addr)->confirmed == true) {
                                state->setPrimaryPath(getPath(addr));
                                EV_INFO << "set primaryPath to " << addr << "\n";
                            }
                            else {
                                getPath(addr)->primaryPathCandidate = true;
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
        sctpAsconfAck->appendSctpChunks(asconfAckChunk);
        Packet *pkt = new Packet("ASCONF-ACK");
        sendToIP(pkt, sctpAsconfAck, remoteAddr);
        if (StartAddIP->isScheduled()) {
            stopTimer(StartAddIP);
            state->corrIdNum = state->asconfSn;
            const char *type = sctpMain->par("addIpType").stringValue();
            sendAsconf(type, false);
        }
    }
    return SCTP_E_IGNORE;
}

SctpEventCode SctpAssociation::processAsconfAckArrived(SctpAsconfAckChunk *asconfAckChunk)
{
    SctpParameter *sctpParam;
    L3Address addr;
    SctpAsconfChunk *sctpasconf;
    std::vector<uint32_t> errorCorrId;
    bool errorFound = false;

    sctpasconf = check_and_cast<SctpAsconfChunk *>(state->asconfChunk->dup());
    if (asconfAckChunk->getSerialNumber() == sctpasconf->getSerialNumber()) {
        stopTimer(getPath(remoteAddr)->AsconfTimer);
        state->errorCount = 0;
        state->asconfOutstanding = false;
        getPath(remoteAddr)->pathErrorCount = 0;
        std::vector<L3Address> remAddr = remoteAddressList;
        for (uint32_t j = 0; j < asconfAckChunk->getAsconfResponseArraySize(); j++) {
            sctpParam = asconfAckChunk->getAsconfResponse(j);
            if (sctpParam->getParameterType() == ERROR_CAUSE_INDICATION) {
                SctpErrorCauseParameter *error = check_and_cast<SctpErrorCauseParameter *>(sctpParam);
                errorCorrId.push_back(error->getResponseCorrelationId());
                EV_INFO << "error added with id " << error->getResponseCorrelationId() << "\n";
            }
        }
        for (uint32_t i = 0; i < sctpasconf->getAsconfParamsArraySize(); i++) {
            sctpParam = CHK(sctpasconf->removeAsconfParam());
            errorFound = false;
            switch (sctpParam->getParameterType()) {
                case ADD_IP_ADDRESS:
                    SctpAddIPParameter *ipParam;
                    ipParam = check_and_cast<SctpAddIPParameter *>(sctpParam);
                    if (errorCorrId.size() > 0) {
                        for (auto& elem : errorCorrId)
                            if ((elem) == ipParam->getRequestCorrelationId()) {
                                errorFound = true;
                                break;
                            }
                    }
                    if (errorFound == true) {
                        delete ipParam;
                        break;
                    }
                    addr = ipParam->getAddressParam();
                    if (addr.isUnspecified()) {
                        addr = localAddr;
                        sendIndicationToApp(SCTP_I_ADDRESS_ADDED);
                    }
                    sctpMain->addLocalAddressToAllRemoteAddresses(this, addr, remAddr);
                    state->localAddresses.push_back(addr);
                    delete ipParam;
                    break;

                case DELETE_IP_ADDRESS:
                    SctpDeleteIPParameter *delParam;
                    delParam = check_and_cast<SctpDeleteIPParameter *>(sctpParam);
                    if (errorCorrId.size() > 0) {
                        for (auto& elem : errorCorrId) {
                            if ((elem) == delParam->getRequestCorrelationId()) {
                                errorFound = true;
                                break;
                            }
                        }
                    }
                    if (errorFound == true) {
                        delete delParam;
                        break;
                    }
                    addr = delParam->getAddressParam();
                    sctpMain->removeLocalAddressFromAllRemoteAddresses(this, addr, remAddr);
                    for (auto j = state->localAddresses.begin(); j != state->localAddresses.end(); j++) {
                        if ((*j) == addr) {
                            EV_DETAIL << "erase address " << (*j) << "\n";
                            state->localAddresses.erase(j);
                            break;
                        }
                    }
                    delete delParam;
                    break;

                case SET_PRIMARY_ADDRESS:
                    SctpSetPrimaryIPParameter *priParam;
                    priParam = check_and_cast<SctpSetPrimaryIPParameter *>(sctpParam);
                    if (errorCorrId.size() > 0) {
                        for (auto& elem : errorCorrId) {
                            if ((elem) == priParam->getRequestCorrelationId()) {
                                errorFound = true;
                                break;
                            }
                        }
                    }
                    if (errorFound == true) {
                        delete priParam;
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

bool SctpAssociation::processPacketDropArrived(SctpPacketDropChunk *packetDropChunk)
{
#if 0
    bool dataReceived = false;

    if (packetDropChunk->getMFlag() == false) {
        EV_TRACE << "processPacketDropArrived" << endl;
        if (packetDropChunk->getEncapsulatedPacket() != nullptr) {
            SctpHeader *sctpmsg = (SctpHeader *)(packetDropChunk->decapsulate());
            const uint32_t numberOfChunks = sctpmsg->getChunksArraySize();
            EV_DETAIL << "numberOfChunks=" << numberOfChunks << endl;
            for (uint32_t i = 0; i < numberOfChunks; i++) {
                SctpChunk *chunk = (SctpChunk *)(sctpmsg->removeChunk());
                const uint8_t type = chunk->getSctpChunkType();
                switch (type) {
                    case DATA: {
                        SctpDataChunk *dataChunk = check_and_cast<SctpDataChunk *>(chunk);
                        const uint32_t tsn = dataChunk->getTsn();
                        auto pq = retransmissionQ->payloadQueue.find(tsn);
                        if ((pq != retransmissionQ->payloadQueue.end()) &&
                            (!chunkHasBeenAcked(pq->second)))
                        {
                            EV_DETAIL << simTime() << ": Packet Drop for TSN "
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
                        if (peekAbandonedChunk(getPath(remoteAddr)) != nullptr) {
                            const auto& sctpmsg = makeShared<SctpHeader>();
                            sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
                            SctpForwardTsnChunk *forwardChunk = createForwardTsnChunk(remoteAddr);
                            if (state->auth && state->peerAuth && typeInChunkList(FORWARD_TSN)) {
                                SctpAuthenticationChunk *authChunk = createAuthChunk();
                                sctpmsg->appendSctpChunks(authChunk);
                            }
                            sctpmsg->appendSctpChunks(forwardChunk);
                        }
                        break;
                    default:
                        throw cRuntimeError("unknown chunk type");
                        break;

                }
                delete chunk;
            }
            disposeOf(sctpmsg);
        }
        else {
            EV_INFO << "no chunk encapsulated" << endl;
        }
        state->peerRwnd = packetDropChunk->getMaxRwnd()
            - packetDropChunk->getQueuedData()
            - getOutstandingBytes();
        statisticsPeerRwnd->record(state->peerRwnd);
        return dataReceived;
    }
#endif
    return false;
}

void SctpAssociation::processErrorArrived(SctpErrorChunk *errorChunk)
{
    uint32_t parameterType;
    for (uint32_t i = 0; i < errorChunk->getParametersArraySize(); i++) {
        SctpParameter *param = (SctpParameter *)errorChunk->getParameters(i);
        parameterType = param->getParameterType();
        switch (parameterType) {
            case MISSING_NAT_ENTRY: {
                if (sctpMain->par("addIP").boolValue()) {
                    if (StartAddIP->isScheduled())
                        stopTimer(StartAddIP);
                    state->corrIdNum = state->asconfSn;
                    const char *type = sctpMain->par("addIpType").stringValue();
                    sendAsconf(type, true);
                }
                break;
            }

            case UNSUPPORTED_HMAC: {
                sendAbort();
                break;
            }
        }
    }
}

void SctpAssociation::process_TIMEOUT_INIT_REXMIT(SctpEventCode& event)
{
    if (++state->initRetransCounter > (int32_t)sctpMain->getMaxInitRetrans()) {
        EV_INFO << "Retransmission count during connection setup exceeds " << (int32_t)sctpMain->getMaxInitRetrans() << ", giving up\n";
        sendIndicationToApp(SCTP_I_CLOSED);
        sendAbort();
        return;
    }
    EV_INFO << "Performing retransmission #" << state->initRetransCounter << "\n";
    switch (fsm->getState()) {
        case SCTP_S_COOKIE_WAIT:
            retransmitInit();
            break;

        case SCTP_S_COOKIE_ECHOED:
            retransmitCookieEcho();
            break;

        default:
            throw cRuntimeError("Internal error: INIT-REXMIT timer expired while in state %s",
                    stateName(fsm->getState()));
    }
    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > sctpMain->getMaxInitRetransTimeout()) {
        state->initRexmitTimeout = sctpMain->getMaxInitRetransTimeout();
    }
    startTimer(T1_InitTimer, state->initRexmitTimeout);
}

void SctpAssociation::process_TIMEOUT_SHUTDOWN(SctpEventCode& event)
{
    if (++state->errorCount > (uint32_t)sctpMain->getAssocMaxRtx()) {
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }

    EV_INFO << "Performing shutdown retransmission. Assoc error count now " << state->errorCount << " \n";
    if (fsm->getState() == SCTP_S_SHUTDOWN_SENT) {
        retransmitShutdown();
    }
    else if (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)
        retransmitShutdownAck();

    state->initRexmitTimeout *= 2;
    if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
}

void SctpAssociation::process_TIMEOUT_HEARTBEAT_INTERVAL(SctpPathVariables *path, bool force)
{
    EV_INFO << "HB Interval timer expired -- sending new HB REQ on path " << path->remoteAddress << "\n";
    /* restart hb_send_timer on this path */
    stopTimer(path->HeartbeatIntervalTimer);
    stopTimer(path->HeartbeatTimer);
    path->heartbeatIntervalTimeout = sctpMain->getHbInterval() + path->pathRto;
    path->heartbeatTimeout = path->pathRto;
    startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);

    if (sctpMain->getEnableHeartbeats() && (simTime() - path->lastAckTime > path->heartbeatIntervalTimeout / 2 || path->forceHb || state->sendHeartbeatsOnActivePaths)) {
        sendHeartbeat(path);
        startTimer(path->HeartbeatTimer, path->heartbeatTimeout);

        path->forceHb = false;
    }
}

void SctpAssociation::process_TIMEOUT_HEARTBEAT(SctpPathVariables *path)
{
    bool oldState = path->activePath;

    /* check if error counters must be increased */
    if (path->activePath) {
        state->errorCount++;
        path->pathErrorCount++;

        EV_INFO << "HB timeout timer expired for path " << path->remoteAddress << " --> Increase Error Counters (Assoc: " << state->errorCount << ", Path: " << path->pathErrorCount << ")\n";
    }

    /* RTO must be doubled for this path ! */
    path->pathRto = (simtime_t)min(2 * path->pathRto.dbl(), sctpMain->getRtoMax());
    path->statisticsPathRTO->record(path->pathRto);
    /* check if any thresholds are exceeded, and if so, check if ULP must be notified */
    if (state->errorCount > (uint32_t)sctpMain->getAssocMaxRtx()) {
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }
    else {
        /* set path state to INACTIVE, if the path error counter is exceeded */
        if (path->pathErrorCount > (uint32_t)sctpMain->getPathMaxRetrans()) {
            oldState = path->activePath;
            path->activePath = false;
            if (path == state->getPrimaryPath()) {
                state->setPrimaryPath(getNextPath(path));
            }
            EV_DETAIL << "pathErrorCount now " << path->pathErrorCount
                      << "; PP now " << state->getPrimaryPathIndex() << endl;
        }
        /* then: we can check, if all paths are INACTIVE ! */
        if (allPathsInactive()) {
            EV_DETAIL << "sctp_do_hb_to_timer() : ALL PATHS INACTIVE --> closing ASSOC\n";
            sendIndicationToApp(SCTP_I_CONN_LOST);
            return;
        }
        else if (path->activePath == false && oldState == true) { // FIXME oldState may be uninitialized
            /* notify the application, in case the PATH STATE has changed from ACTIVE to INACTIVE */
            pathStatusIndication(path, false);
        }
    }
}

void SctpAssociation::stopTimers()
{
    for (auto& elem : sctpPathMap) {
        stopTimer(elem.second->HeartbeatTimer);
        stopTimer(elem.second->HeartbeatIntervalTimer);
    }
}

void SctpAssociation::stopTimer(cMessage *timer)
{
    EV_INFO << "stopTimer " << timer->getName() << endl;
    if (timer->isScheduled()) {
        cancelEvent(timer);
    }
}

void SctpAssociation::startTimer(cMessage *timer, const simtime_t& timeout)
{
    EV_DETAIL << "startTimer " << timer->getName() << " with timeout "
              << timeout << " to expire at " << simTime() + timeout << endl;
    scheduleTimeout(timer, timeout);
}

void SctpAssociation::process_TIMEOUT_RESET(SctpPathVariables *path)
{
    int32_t value;
    std::map<uint32_t, SctpStateVariables::RequestData>::reverse_iterator rit;
    rit = state->requests.rbegin();
    if (rit->second.result == DEFERRED) {
        value = 1;
    }
    else {
        value = updateCounters(path);
    }
    if (value == 1) {
        EV_DETAIL << "Performing timeout reset" << endl;
        retransmitReset();

        /* increase the RTO (by doubling it) */
        path->pathRto = min(2 * path->pathRto.dbl(), sctpMain->getRtoMax());
        path->statisticsPathRTO->record(path->pathRto);
        startTimer(path->ResetTimer, path->pathRto);
    }
}

int32_t SctpAssociation::updateCounters(SctpPathVariables *path)
{
    bool notifyUlp = false;
    if (++state->errorCount > (uint32_t)sctpMain->getAssocMaxRtx()) {
        EV_DETAIL << "Retransmission count during connection setup exceeds " << (int32_t)sctpMain->getAssocMaxRtx() << ", giving up\n";
        sendIndicationToApp(SCTP_I_CLOSED);
        sendAbort();
        sctpMain->removeAssociation(this);
        return 0;
    }
    else if (++path->pathErrorCount > (uint32_t)sctpMain->getPathMaxRetrans()) {
        if (path->activePath) {
            /* tell the source */
            notifyUlp = true;
        }

        path->activePath = false;
        if (path == state->getPrimaryPath()) {
            state->setPrimaryPath(getNextPath(path));
        }
        EV_DETAIL << "process_TIMEOUT_RESET(" << (path->remoteAddress) << ") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE\n";
        if (allPathsInactive()) {
            EV_DETAIL << "process_TIMEOUT_RESET : ALL PATHS INACTIVE --> closing ASSOC\n";
            sendIndicationToApp(SCTP_I_CONN_LOST);
            sendAbort();
            sctpMain->removeAssociation(this);
            return 0;
        }
        else if (notifyUlp) {
            /* notify the application */
            pathStatusIndication(path, false);
        }
        EV_DETAIL << "process_TIMEOUT_RESET(" << (path->remoteAddress) << ") : PATH ERROR COUNTER now " << path->pathErrorCount << "\n";
        return 2;
    }
    return 1;
}

void SctpAssociation::process_TIMEOUT_ASCONF(SctpPathVariables *path)
{
    int32_t value;

    if ((value = updateCounters(path)) == 1) {
        retransmitAsconf();

        /* increase the RTO (by doubling it) */
        path->pathRto = min(2 * path->pathRto.dbl(), sctpMain->getRtoMax());
        path->statisticsPathRTO->record(path->pathRto);

        startTimer(path->AsconfTimer, path->pathRto);
    }
}

void SctpAssociation::process_TIMEOUT_RTX(SctpPathVariables *path)
{
    EV_DETAIL << "Processing retransmission timeout ..." << endl;

    // Stop blocking!
    if (path->BlockingTimer) {
        stopTimer(path->BlockingTimer);
    }
    path->blockingTimeout = -1.0;

    // ====== Increase the RTO (by doubling it) ==============================
    path->pathRto = min(2 * path->pathRto.dbl(), sctpMain->getRtoMax());
    path->statisticsPathRTO->record(path->pathRto);
    EV_DETAIL << "Schedule T3 based retransmission for path " << path->remoteAddress
              << " oldest chunk sent " << simTime() - path->oldestChunkSendTime << " ago"
              << " (TSN " << path->oldestChunkTsn << ")" << endl;
    EV_DEBUG << "Unacked chunks in Retransmission Queue:" << endl;
    for (SctpQueue::PayloadQueue::const_iterator iterator = retransmissionQ->payloadQueue.begin();
         iterator != retransmissionQ->payloadQueue.end(); ++iterator)
    {
        const SctpDataVariables *myChunk = iterator->second;
        if (!myChunk->hasBeenAcked) {
            const SctpPathVariables *myChunkLastPath = myChunk->getLastDestinationPath();
            EV_DEBUG << " - " << myChunk->tsn
                     << "\tsent=now-" << simTime() - myChunk->sendTime
                     << "\tlast=" << myChunkLastPath->remoteAddress
                     << "\tmoved=" << ((myChunk->hasBeenMoved == true) ? "YES!" : "no")
                     << "\tnumTX=" << myChunk->numberOfTransmissions
                     << "\tnumRTX=" << myChunk->numberOfRetransmissions
                     << "\tfastRTX=" << ((myChunk->hasBeenFastRetransmitted == true) ? "YES!" : "no")
                     << endl;
        }
    }
    EV_DEBUG << "----------------------" << endl;

    // ====== Update congestion window =======================================
    (this->*ccFunctions.ccUpdateAfterRtxTimeout)(path);

    // ====== Error Counter Handling =========================================
    if (!state->zeroWindowProbing) {
        state->errorCount++;
        path->pathErrorCount++;
        EV_DETAIL << "RTX-Timeout: errorCount increased to " << path->pathErrorCount << "  state->errorCount=" << state->errorCount << "\n";
    }
    if (state->errorCount > (uint32_t)sctpMain->getAssocMaxRtx()) {
        /* error counter exceeded terminate the association -- create an SCTPC_EV_CLOSE event and send it to myself */

        EV_DETAIL << "process_TIMEOUT_RTX : ASSOC ERROR COUNTER EXCEEDED, closing ASSOC" << endl;
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return;
    }
    else {
        if (path->pathErrorCount > static_cast<uint32_t>(sctpMain->par("pathMaxRetrans"))) {
            bool notifyUlp = false;

            EV_DETAIL << "pathErrorCount exceeded\n";
            if (path->activePath) {
                /* tell the source */
                notifyUlp = true;
            }
            path->activePath = false;
            if (path->remoteAddress == state->getPrimaryPathIndex()) {
                SctpPathVariables *nextPath = getNextPath(path);
                if (nextPath != nullptr) {
                    state->setPrimaryPath(nextPath);
                }
            }
            EV_DETAIL << "process_TIMEOUT_RTX(" << path->remoteAddress
                      << ") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE" << endl;
            if (allPathsInactive()) {
                EV_WARN << "process_TIMEOUT_RTX: ALL PATHS INACTIVE --> connection LOST!" << endl;
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
        EV_DETAIL << "process_TIMEOUT_RTX(" << path->remoteAddress
                  << ") : PATH ERROR COUNTER now " << path->pathErrorCount << endl;
    }

    // ====== Do Retransmission ==============================================
    // dequeue all chunks not acked so far and put them in the TransmissionQ
    if (!retransmissionQ->payloadQueue.empty()) {
        EV_DETAIL << "Still " << retransmissionQ->payloadQueue.size()
                  << " chunks in retransmissionQ" << endl;

        for (auto& elem : retransmissionQ->payloadQueue) {
            SctpDataVariables *chunk = elem.second;
            assert(chunk != nullptr);

            // ====== Insert chunks into TransmissionQ ============================
            // Only insert chunks that were sent to the path that has timed out
            if (!chunkMustBeAbandoned(chunk, path) && ((chunkHasBeenAcked(chunk) == false && chunk->countsAsOutstanding)
                                                       || chunk->hasBeenReneged) && (chunk->getLastDestinationPath() == path))
            {
                SctpPathVariables *nextPath = getNextDestination(chunk);
                EV_DETAIL << simTime() << ": Timer-Based RTX for TSN " << chunk->tsn
                          << ": lastDestination=" << chunk->getLastDestination()
                          << " lastPathRTO=" << chunk->getLastDestinationPath()->pathRto
                          << " nextDestination=" << nextPath->remoteAddress
                          << " nextPathRTO=" << nextPath->pathRto
                          << " waiting=" << simTime() - chunk->sendTime
                          << endl;
                nextPath->numberOfTimerBasedRetransmissions++;
                chunk->hasBeenTimerBasedRtxed = true;
                chunk->sendForwardIfAbandoned = true;
                chunk->numberOfTransmissions++;

                if (!chunk->hasBeenAbandoned) {
                    auto iter = sctpMain->assocStatMap.find(assocId);
                    iter->second.numT3Rtx++;
                }

                moveChunkToOtherPath(chunk, nextPath);
            }
        }
    }

    SctpPathVariables *nextPath = getNextPath(path);
    EV_DETAIL << "TimeoutRTX: sendOnAllPaths()" << endl;
    sendOnAllPaths(nextPath);
}

void SctpAssociation::process_TIMEOUT_BLOCKING(SctpPathVariables *path)
{
    EV_INFO << "TIMEOUT_BLOCKING on " << path->remoteAddress
            << " cwnd=" << path->cwnd << endl;
    path->blockingTimeout = -1.0;
    sendOnAllPaths(path);
}

void SctpAssociation::moveChunkToOtherPath(SctpDataVariables *chunk,
        SctpPathVariables *newPath)
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
        EV_DETAIL << "TSN " << chunk->tsn << " already in transmissionQ" << endl;
        return;
    }
    else {
        chunk->enqueuedInTransmissionQ = true;
        EV_DETAIL << "Inserting TSN " << chunk->tsn << " into transmissionQ" << endl;
        auto q = qCounter.roomTransQ.find(chunk->getNextDestination());
        q->second += ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
        auto qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
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
    if ((state->peerWindowFull) && (state->peerRwnd > 0)) {
        state->peerWindowFull = false;
    }

    statisticsPeerRwnd->record(state->peerRwnd);
}

} // namespace sctp
} // namespace inet

