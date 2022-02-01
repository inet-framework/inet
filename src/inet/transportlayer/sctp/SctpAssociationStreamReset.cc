//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

void SctpAssociation::retransmitReset()
{
    if (fsm->getState() == SCTP_S_SHUTDOWN_PENDING || fsm->getState() == SCTP_S_ESTABLISHED) {
        const auto& sctpmsg = makeShared<SctpHeader>();
        sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
        SctpStreamResetChunk *sctpreset = check_and_cast<SctpStreamResetChunk *>(state->resetChunk->dup());
        state->numResetRequests = sctpreset->getParametersArraySize();
        sctpreset->setSctpChunkType(RE_CONFIG);
        sctpmsg->appendSctpChunks(sctpreset);
        state->waitForResponse = true;
        EV_INFO << "retransmitStreamReset localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";
        Packet *pkt = new Packet("RE_CONFIG");
        sendToIP(pkt, sctpmsg);
    }
}

bool SctpAssociation::streamIsPending(int32_t sid)
{
    if (state->streamsPending.size() > 0 || (state->resetOutStreams.size() > 0 && state->resetPending)) {
        std::list<uint16_t>::iterator it;
        if (state->streamsPending.size() > 0) {
            for (it = state->streamsPending.begin(); it != state->streamsPending.end(); it++) {
                if (sid == (*it)) {
                    return true;
                }
            }
        }
        if (state->resetOutStreams.size() > 0) {
            for (it = state->resetOutStreams.begin(); it != state->resetOutStreams.end(); it++) {
                if (sid == (*it)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void SctpAssociation::checkStreamsToReset()
{
    if (!state->resetPending && !state->fragInProgress && (state->resetRequested || state->incomingRequest != nullptr) && (state->outstandingBytes == 0 || state->streamsPending.size() > 0)) {
        if (state->localRequestType == RESET_OUTGOING || state->localRequestType == RESET_BOTH || state->peerRequestType == RESET_INCOMING) {
            state->streamsToReset.clear();
            std::list<uint16_t>::iterator it;
            for (it = state->streamsPending.begin(); it != state->streamsPending.end();) {
                if (getBytesInFlightOfStream(*it) == 0) {
                    state->streamsToReset.push_back(*it);
                    it = state->streamsPending.erase(it);
                }
                else
                    ++it;
            }
        }
        if (!state->resetPending && state->resetRequested &&
            (state->localRequestType == SSN_TSN ||
             state->localRequestType == ADD_OUTGOING ||
             state->localRequestType == ADD_INCOMING))
        {
            sendStreamResetRequest(state->resetInfo);
            state->resetPending = true;
        }
        if (state->resetDeferred && (state->streamsToReset.size() > 0 || state->peerRequestType == SSN_TSN)) {
            switch (state->peerRequestType) {
                case SSN_TSN: {
                    SctpSsnTsnResetRequestParameter *ssnParam = check_and_cast<SctpSsnTsnResetRequestParameter *>(state->incomingRequest);
                    processSsnTsnResetRequestArrived(ssnParam);
                    if (state->sendResponse == PERFORMED_WITH_OPTION) {
                        resetExpectedSsns();
                        state->sendResponse = 0;
                        sendStreamResetResponse(ssnParam, PERFORMED, true);
                    }
                    delete ssnParam;
                    break;
                }
                case RESET_INCOMING: {
                    SctpIncomingSsnResetRequestParameter *inParam = check_and_cast<SctpIncomingSsnResetRequestParameter *>(state->incomingRequest->dup());
//                    state->incomingRequest->setName("StateSackIncoming");
//                    inParam->setName("checkInParam");
                    processIncomingResetRequestArrived(inParam);
                    if (state->incomingRequestSet && state->incomingRequest != nullptr) {
                        delete state->incomingRequest;
                        state->incomingRequest = nullptr;
                        state->incomingRequestSet = false;
                    }
                    delete inParam;
                    break;
                }
            }
            state->resetDeferred = false;
        }
        else if ((state->localRequestType == RESET_OUTGOING || state->peerRequestType == RESET_INCOMING || state->localRequestType == RESET_BOTH) &&
                 (state->streamsToReset.size() > 0))
        {
            if (state->peerRequestType == RESET_INCOMING)
                state->localRequestType = RESET_OUTGOING;
            sendStreamResetRequest(state->resetInfo);
            state->resetPending = true;
        }
    }
}

void SctpAssociation::sendOutgoingResetRequest(SctpIncomingSsnResetRequestParameter *requestParam)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    uint16_t len = 0;
    uint32_t srsn = 0;
    if ((!(getPath(remoteAddr)->ResetTimer->isScheduled())) || state->requestsOverlap) {
        state->requestsOverlap = false;
        SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
        resetChunk->setSctpChunkType(RE_CONFIG);
        resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        SctpOutgoingSsnResetRequestParameter *outResetParam;
        outResetParam = new SctpOutgoingSsnResetRequestParameter();
        outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
        SctpStateVariables::RequestData *resDat = state->findTypeInRequests(OUTGOING_RESET_REQUEST_PARAMETER);
        if (resDat != nullptr && resDat->sn == state->streamResetSequenceNumber - 1) {
            srsn = state->streamResetSequenceNumber - 1;
        }
        else {
            srsn = state->streamResetSequenceNumber;
            state->requests[srsn].result = 100;
            state->requests[srsn].type = OUTGOING_RESET_REQUEST_PARAMETER;
            state->requests[srsn].sn = srsn;
            state->numResetRequests++;
        }
        outResetParam->setSrReqSn(srsn++);
        outResetParam->setSrResSn(requestParam->getSrReqSn());
        if (state->findPeerRequestNum(requestParam->getSrReqSn())) {
            state->peerRequests[requestParam->getSrReqSn()].result = PERFORMED;
            state->firstPeerRequest = false;
            if (state->incomingRequestSet && state->incomingRequest != nullptr) {
                delete state->incomingRequest;
                state->incomingRequest = nullptr;
                state->incomingRequestSet = false;
            }
        }
        outResetParam->setLastTsn(state->nextTsn - 1);
        if (state->peerStreamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->peerStreamsToReset.size());
            uint16_t i = 0;
            for (std::list<uint16_t>::iterator it = state->peerStreamsToReset.begin(); it != state->peerStreamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn - 1].streams.push_back(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->peerStreamsToReset.size() * 2;
            state->peerStreamsToReset.clear();
        }
        else if (state->streamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
            uint16_t i = 0;
            for (std::list<uint16_t>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn - 1].streams.push_back(outResetParam->getStreamNumbers(i));
                resetSsn(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->streamsToReset.size() * 2;
            state->streamsToReset.clear();
        }
        else if (requestParam->getStreamNumbersArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(requestParam->getStreamNumbersArraySize());
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                outResetParam->setStreamNumbers(i, requestParam->getStreamNumbers(i));
                state->requests[srsn - 1].streams.push_back(requestParam->getStreamNumbers(i));
            }
            len = requestParam->getStreamNumbersArraySize() * 2;
        }
        outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len);
        resetChunk->addParameter(outResetParam);
        state->streamResetSequenceNumber = srsn;
        state->resetRequested = true;

        SctpResetTimer *rt = new SctpResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn - 1);
        rt->setOutAcked(false);

        const auto& msg = makeShared<SctpHeader>();
        msg->setChunkLength(B(SCTP_COMMON_HEADER));
        msg->setSrcPort(localPort);
        msg->setDestPort(remotePort);
        msg->appendSctpChunks(resetChunk);
        state->localRequestType = RESET_OUTGOING;
        if (state->resetChunk != nullptr) {
            delete state->resetChunk;
            state->resetChunk = nullptr;
        }
        state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
        if (qCounter.roomSumSendStreams != 0) {
            storePacket(getPath(remoteAddr), msg, 1, 0, false);
            state->bundleReset = true;
            sendOnPath(getPath(remoteAddr), true);
            state->bundleReset = false;
        }
        else {
            Packet *pkt = new Packet("RE_CONFIG");
            sendToIP(pkt, msg, remoteAddr);
        }
        if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
            delete PK(getPath(remoteAddr)->ResetTimer)->decapsulate();
        }
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
            stopTimer(getPath(remoteAddr)->ResetTimer);
        }
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
    }
}

void SctpAssociation::sendBundledOutgoingResetAndResponse(SctpIncomingSsnResetRequestParameter *requestParam)
{
    EV_INFO << "sendBundledOutgoingResetAndResponse to " << remoteAddr << "\n";
    uint16_t len = 0;
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
        resetChunk->setSctpChunkType(RE_CONFIG);
        resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        uint32_t srsn = state->streamResetSequenceNumber;
        SctpOutgoingSsnResetRequestParameter *outResetParam;
        outResetParam = new SctpOutgoingSsnResetRequestParameter();
        outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
        state->requests[srsn].result = 100;
        state->requests[srsn].type = OUTGOING_RESET_REQUEST_PARAMETER;
        state->numResetRequests++;
        outResetParam->setSrReqSn(srsn++);
        outResetParam->setSrResSn(requestParam->getSrReqSn());
        outResetParam->setLastTsn(state->nextTsn - 1);
        if (state->streamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
            uint16_t i = 0;
            for (std::list<uint16_t>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                resetSsn(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->streamsToReset.size() * 2;
            state->streamsToReset.clear();
        }
        else if (requestParam->getStreamNumbersArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(requestParam->getStreamNumbersArraySize());
            for (uint16_t i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                outResetParam->setStreamNumbers(i, requestParam->getStreamNumbers(i));
            }
            len = requestParam->getStreamNumbersArraySize() * 2;
        }
        outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len);
        resetChunk->addParameter(outResetParam);
        state->streamResetSequenceNumber = srsn;

        SctpStreamResetChunk *resetResponseChunk;
        EV_INFO << "sendbundledStreamResetResponse to " << remoteAddr << "\n";
        resetResponseChunk = new SctpStreamResetChunk();
        resetResponseChunk->setSctpChunkType(RE_CONFIG);
        resetResponseChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        SctpStreamResetResponseParameter *responseParam = new SctpStreamResetResponseParameter();
        responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
        responseParam->setSrResSn(requestParam->getSrReqSn());
        responseParam->setResult(PERFORMED);
        responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
        resetResponseChunk->addParameter(responseParam);
        state->resetRequested = false;

        SctpResetTimer *rt = new SctpResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn - 1);
        rt->setOutAcked(false);

        const auto& msg = makeShared<SctpHeader>();
        msg->setChunkLength(B(SCTP_COMMON_HEADER));
        msg->setSrcPort(localPort);
        msg->setDestPort(remotePort);
        msg->appendSctpChunks(resetChunk);
        msg->appendSctpChunks(resetResponseChunk);
        state->localRequestType = RESET_OUTGOING;
        if (state->resetChunk != nullptr) {
            delete state->resetChunk;
            state->resetChunk = nullptr;
        }
        state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
//        state->resetChunk->setName("State_Resetchunk");
        if (qCounter.roomSumSendStreams != 0) {
            storePacket(getPath(remoteAddr), msg, 1, 0, false);
            state->bundleReset = true;
            sendOnPath(getPath(remoteAddr), true);
            state->bundleReset = false;
        }
        else {
            Packet *pkt = new Packet("RE_CONFIG");
            sendToIP(pkt, msg, remoteAddr);
        }
        if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
            PK(getPath(remoteAddr)->ResetTimer)->decapsulate();
        }
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
            stopTimer(getPath(remoteAddr)->ResetTimer);
        }
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
    }
}

SctpParameter *SctpAssociation::makeOutgoingStreamResetParameter(uint32_t srsn, SctpResetReq *info)
{
    SctpOutgoingSsnResetRequestParameter *outResetParam;
    outResetParam = new SctpOutgoingSsnResetRequestParameter();
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn);
    outResetParam->setSrResSn(state->expectedStreamResetSequenceNumber - 1);
    outResetParam->setLastTsn(state->nextTsn - 1);
    state->requests[srsn].lastTsn = outResetParam->getLastTsn();
    if (state->outstandingBytes == 0 && state->streamsToReset.size() == 0) {
        if (info->getStreamsArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(info->getStreamsArraySize());
            for (uint i = 0; i < info->getStreamsArraySize(); i++) {
                outResetParam->setStreamNumbers(i, (uint16_t)info->getStreams(i));
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn].streams.push_back(outResetParam->getStreamNumbers(i));
            }
        }
    }
    else if (state->streamsToReset.size() > 0) {
        outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
        uint16_t i = 0;
        for (std::list<uint16_t>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
            outResetParam->setStreamNumbers(i, *it);
            state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
            state->requests[srsn].streams.push_back(outResetParam->getStreamNumbers(i));
            i++;
        }
        state->streamsToReset.clear();
    }
    state->localRequestType = RESET_OUTGOING;
    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + (outResetParam->getStreamNumbersArraySize() * 2));
    return outResetParam;
}

SctpParameter *SctpAssociation::makeIncomingStreamResetParameter(uint32_t srsn, SctpResetReq *info)
{
    SctpIncomingSsnResetRequestParameter *inResetParam;
    inResetParam = new SctpIncomingSsnResetRequestParameter();
    inResetParam->setParameterType(INCOMING_RESET_REQUEST_PARAMETER);
    inResetParam->setSrReqSn(srsn);
    if (info->getStreamsArraySize() > 0) {
        inResetParam->setStreamNumbersArraySize(info->getStreamsArraySize());
        for (uint i = 0; i < info->getStreamsArraySize(); i++) {
            inResetParam->setStreamNumbers(i, info->getStreams(i));
            state->requests[srsn].streams.push_back(inResetParam->getStreamNumbers(i));
            state->resetInStreams.push_back(inResetParam->getStreamNumbers(i));
        }
    }
    state->localRequestType = RESET_INCOMING;
    inResetParam->setByteLength(SCTP_INCOMING_RESET_REQUEST_PARAMETER_LENGTH + (info->getStreamsArraySize() * 2));
    return inResetParam;
}

SctpParameter *SctpAssociation::makeAddStreamsRequestParameter(uint32_t srsn, SctpResetReq *info)
{
    SctpAddStreamsRequestParameter *addStreams = new SctpAddStreamsRequestParameter();
    if (info->getInstreams() > 0) {
        addStreams->setParameterType(ADD_INCOMING_STREAMS_REQUEST_PARAMETER);
        addStreams->setNumberOfStreams(info->getInstreams());
        state->localRequestType = ADD_INCOMING;
        state->requests[srsn].type = ADD_INCOMING_STREAMS_REQUEST_PARAMETER;
        state->numAddedInStreams = addStreams->getNumberOfStreams();
    }
    else if (info->getOutstreams() > 0) {
        addStreams->setParameterType(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
        addStreams->setNumberOfStreams(info->getOutstreams());
        state->localRequestType = ADD_OUTGOING;
        state->requests[srsn].type = ADD_OUTGOING_STREAMS_REQUEST_PARAMETER;
        state->numAddedOutStreams = addStreams->getNumberOfStreams();
    }
    addStreams->setSrReqSn(srsn);
    addStreams->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
    return addStreams;
}

SctpParameter *SctpAssociation::makeSsnTsnResetParameter(uint32_t srsn)
{
    SctpSsnTsnResetRequestParameter *resetParam;
    resetParam = new SctpSsnTsnResetRequestParameter();
    resetParam->setParameterType(SSN_TSN_RESET_REQUEST_PARAMETER);
    resetParam->setSrReqSn(srsn);
    resetParam->setByteLength(SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH);
    state->localRequestType = SSN_TSN;
    return resetParam;
}

void SctpAssociation::sendOutgoingRequestAndResponse(uint32_t inRequestSn, uint32_t outRequestSn)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resChunk = new SctpStreamResetChunk();
    resChunk->setSctpChunkType(RE_CONFIG);
    resChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32_t srsn = state->streamResetSequenceNumber;
    SctpResetTimer *rt = new SctpResetTimer();
    SctpOutgoingSsnResetRequestParameter *outResetParam;
    outResetParam = new SctpOutgoingSsnResetRequestParameter();
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn++);
    outResetParam->setSrResSn(inRequestSn);
    outResetParam->setLastTsn(state->nextTsn - 1);
    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH);
    resChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn;
    SctpStreamResetResponseParameter *responseParam = new SctpStreamResetResponseParameter();
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(outRequestSn);
    responseParam->setResult(1);
    responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resChunk->addParameter(responseParam);
    rt->setInSN(srsn - 1);
    rt->setInAcked(false);
    rt->setOutSN(outRequestSn);
    rt->setOutAcked(false);
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = resChunk->dup();
    msg->appendSctpChunks(resChunk);
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SctpAssociation::sendOutgoingRequestAndResponse(SctpIncomingSsnResetRequestParameter *inRequestParam,
        SctpOutgoingSsnResetRequestParameter *outRequestParam)
{
    uint16_t len = 0;
    EV_INFO << "sendOutgoingRequestAndResponse to " << remoteAddr << "\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resChunk = new SctpStreamResetChunk();
    resChunk->setSctpChunkType(RE_CONFIG);
    resChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32_t srsn = state->streamResetSequenceNumber;
    SctpResetTimer *rt = new SctpResetTimer();
    SctpOutgoingSsnResetRequestParameter *outResetParam;
    outResetParam = new SctpOutgoingSsnResetRequestParameter();
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn);
    outResetParam->setSrResSn(inRequestParam->getSrReqSn());
    outResetParam->setLastTsn(state->nextTsn - 1);
    if (inRequestParam->getStreamNumbersArraySize() > 0) {
        outResetParam->setStreamNumbersArraySize(inRequestParam->getStreamNumbersArraySize());
        for (uint i = 0; i < inRequestParam->getStreamNumbersArraySize(); i++) {
            outResetParam->setStreamNumbers(i, (uint16_t)inRequestParam->getStreamNumbers(i));
            state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
            state->requests[srsn].streams.push_back(outResetParam->getStreamNumbers(i));
            len++;
        }
    }
    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len * 2);
    resChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn++;
    SctpStreamResetResponseParameter *responseParam = new SctpStreamResetResponseParameter();
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(outRequestParam->getSrReqSn());
    responseParam->setResult(1);
    responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resChunk->addParameter(responseParam);
    rt->setInSN(srsn - 1);
    rt->setInAcked(false);
    rt->setOutSN(outRequestParam->getSrReqSn());
    rt->setOutAcked(false);
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    auto it = sctpMain->assocStatMap.find(assocId);
    it->second.numResetRequestsSent++;
    state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resChunk->dup());
//    state->resetChunk->setName("stateRstChunk");
    msg->appendSctpChunks(resChunk);
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg, remoteAddr);
    if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
        PK(getPath(remoteAddr)->ResetTimer)->decapsulate();
    }
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SctpAssociation::sendStreamResetRequest(SctpResetReq *rinfo)
{
    EV_INFO << "StreamReset:sendStreamResetRequest\n";
    SctpParameter *param;
    uint16_t type;
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32_t srsn = state->streamResetSequenceNumber;
    SctpResetTimer *rt = new SctpResetTimer();
    auto it = sctpMain->assocStatMap.find(assocId);
    if (rinfo)
        type = rinfo->getRequestType();
    else
        type = state->localRequestType;
    switch (type) {
        case RESET_OUTGOING:
            EV_INFO << "RESET_OUTGOING\n";
            state->requests[srsn].result = 100;
            state->requests[srsn].type = OUTGOING_RESET_REQUEST_PARAMETER;
            state->requests[srsn].sn = srsn;
            param = makeOutgoingStreamResetParameter(srsn, rinfo);
            resetChunk->addParameter(param);
            rt->setInSN(0);
            rt->setInAcked(true);
            rt->setOutSN(srsn);
            rt->setOutAcked(false);
            it->second.numResetRequestsSent++;
            state->numResetRequests++;

            state->waitForResponse = true;
            state->resetRequested = true;
            break;

        case RESET_INCOMING:
            EV_INFO << "RESET_INCOMING\n";
            state->requests[srsn].result = 100;
            state->requests[srsn].type = INCOMING_RESET_REQUEST_PARAMETER;
            state->requests[srsn].sn = srsn;
            param = makeIncomingStreamResetParameter(srsn, rinfo);
            resetChunk->addParameter(param);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            rt->setOutSN(0);
            rt->setOutAcked(true);
            it->second.numResetRequestsSent++;
            state->numResetRequests++;
            state->waitForResponse = true;
            break;

        case RESET_BOTH:
            EV_INFO << "RESET_BOTH\n";
            SctpParameter *outParam;
            state->requests[srsn].result = 100;
            state->requests[srsn].type = OUTGOING_RESET_REQUEST_PARAMETER;
            state->requests[srsn].sn = srsn;
            outParam = makeOutgoingStreamResetParameter(srsn, rinfo);
            rt->setOutSN(srsn++);
            rt->setOutAcked(false);
            resetChunk->addParameter(outParam);
            state->requests[srsn].result = 100;
            state->requests[srsn].type = INCOMING_RESET_REQUEST_PARAMETER;
            state->requests[srsn].sn = srsn;
            SctpParameter *inParam;
            inParam = makeIncomingStreamResetParameter(srsn, rinfo);
            resetChunk->addParameter(inParam);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            it->second.numResetRequestsSent += 2;
            state->numResetRequests += 2;
            state->waitForResponse = true;
            break;

        case SSN_TSN:
            param = makeSsnTsnResetParameter(srsn);
            resetChunk->addParameter(param);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            rt->setOutSN(srsn);
            rt->setOutAcked(false);
            state->requests[srsn].result = 100;
            state->requests[srsn].type = SSN_TSN_RESET_REQUEST_PARAMETER;
            it->second.numResetRequestsSent++;
            state->numResetRequests++;
            break;

        case ADD_INCOMING:
        case ADD_OUTGOING:
            EV_INFO << "ADD_INCOMING or ADD_OUTGOING\n";
            state->requests[srsn].result = 100;
            param = makeAddStreamsRequestParameter(srsn, rinfo);
            resetChunk->addParameter(param);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            rt->setOutSN(0);
            rt->setOutAcked(true);
            it->second.numResetRequestsSent++;
            state->numResetRequests++;
            break;
        default: EV_INFO << "Request type %d not known\n";

    }
    state->streamResetSequenceNumber = ++srsn;
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
    msg->appendSctpChunks(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    }
    else {
        Packet *pkt = new Packet("RE_CONFIG");
        sendToIP(pkt, msg, remoteAddr);
    }
    if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
        PK(getPath(remoteAddr)->ResetTimer)->decapsulate();
    }
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
        stopTimer(getPath(remoteAddr)->ResetTimer);
    }
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SctpAssociation::sendAddOutgoingStreamsRequest(uint16_t numStreams)
{
    EV_INFO << "StreamReset:sendAddOutgoingStreamsRequest\n";
    uint32_t srsn = 0;
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SctpStateVariables::RequestData *resDat = state->findTypeInRequests(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
    if (resDat != nullptr && resDat->sn == state->streamResetSequenceNumber - 1) {
        srsn = state->streamResetSequenceNumber - 1;
    }
    else {
        srsn = state->streamResetSequenceNumber;
        state->requests[srsn].result = 100;
        state->requests[srsn].type = ADD_OUTGOING_STREAMS_REQUEST_PARAMETER;
        state->requests[srsn].sn = srsn;
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numResetRequestsSent++;
    }
    SctpAddStreamsRequestParameter *addStreams = new SctpAddStreamsRequestParameter();
    addStreams->setParameterType(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
    addStreams->setNumberOfStreams(numStreams);
    state->numAddedOutStreams = addStreams->getNumberOfStreams();
    state->localRequestType = ADD_OUTGOING;
    addStreams->setSrReqSn(srsn);
    addStreams->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
    resetChunk->addParameter(addStreams);
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
//    state->resetChunk->setName("stateAddResetChunk");
    msg->appendSctpChunks(resetChunk);
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg, remoteAddr);
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        SctpResetTimer *rt = new SctpResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn);
        rt->setOutAcked(false);
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
    }
    state->streamResetSequenceNumber = ++srsn;
}

void SctpAssociation::sendAddInAndOutStreamsRequest(SctpResetReq *info)
{
    EV_INFO << "StreamReset:sendAddInandStreamsRequest\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpResetTimer *rt = new SctpResetTimer();
    SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32_t srsn = state->streamResetSequenceNumber;
    SctpAddStreamsRequestParameter *addOutStreams = new SctpAddStreamsRequestParameter();
    addOutStreams->setParameterType(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
    addOutStreams->setNumberOfStreams(info->getOutstreams());
    state->numAddedOutStreams = info->getOutstreams();
    rt->setOutSN(srsn);
    rt->setOutAcked(false);
    state->numResetRequests++;
    state->requests[srsn].result = 100;
    state->requests[srsn].type = ADD_OUTGOING_STREAMS_REQUEST_PARAMETER;
    addOutStreams->setSrReqSn(srsn++);
    addOutStreams->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
    SctpAddStreamsRequestParameter *addInStreams = new SctpAddStreamsRequestParameter();
    addInStreams->setParameterType(ADD_INCOMING_STREAMS_REQUEST_PARAMETER);
    addInStreams->setNumberOfStreams(info->getInstreams());
    state->numAddedInStreams = info->getInstreams();
    rt->setInSN(srsn);
    rt->setInAcked(false);
    state->numResetRequests++;
    state->requests[srsn].result = 100;
    state->requests[srsn].type = ADD_INCOMING_STREAMS_REQUEST_PARAMETER;
    addInStreams->setSrReqSn(srsn++);
    addInStreams->setByteLength(SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH);
    resetChunk->addParameter(addOutStreams);
    resetChunk->addParameter(addInStreams);
    state->localRequestType = ADD_BOTH;
    state->streamResetSequenceNumber = srsn;
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = check_and_cast<SctpStreamResetChunk *>(resetChunk->dup());
//    state->resetChunk->setName("stateAddInOutResetChunk");
    msg->appendSctpChunks(resetChunk);
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SctpAssociation::resetGapLists()
{
    uint32_t newCumAck = state->gapList.getHighestTsnReceived() + (1 << 31);
    state->gapList.resetGaps(newCumAck);
}

void SctpAssociation::sendStreamResetResponse(SctpSsnTsnResetRequestParameter *requestParam, int result, bool options)
{
    uint32_t len = 0;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << " with options\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpStreamResetChunk *resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SctpStreamResetResponseParameter *responseParam = new SctpStreamResetResponseParameter();
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(requestParam->getSrReqSn());
    responseParam->setResult(result);
    state->peerRequests[requestParam->getSrReqSn()].result = result;
    len = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH;
    if (options && result == PERFORMED) {
        responseParam->setSendersNextTsn(state->nextTsn);
        responseParam->setReceiversNextTsn(state->gapList.getHighestTsnReceived() + (1 << 31) + 1);
        resetSsns();
        resetExpectedSsns();
        state->stopOldData = true;
        if (state->incomingRequestSet && state->incomingRequest != nullptr) {
            delete state->incomingRequest;
            state->incomingRequest = nullptr;
            state->incomingRequestSet = false;
        }
        len += 8;
        resetGapLists();
        state->peerTsnAfterReset = state->gapList.getHighestTsnReceived() + (1 << 31);
        state->firstPeerRequest = false;
    }
    responseParam->setByteLength(len);
    resetChunk->addParameter(responseParam);
    msg->appendSctpChunks(resetChunk);
    stopTimer(SackTimer);
    msg->appendSctpChunks(createSack());
    state->ackState = 0;
    state->sackAlreadySent = true;
    Packet *pkt = new Packet("RE_CONFIG");
    sendToIP(pkt, msg);
}

void SctpAssociation::sendStreamResetResponse(uint32_t srrsn, int result)
{
    SctpStreamResetChunk *resetChunk;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << "\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SctpStreamResetResponseParameter *responseParam = new SctpStreamResetResponseParameter();
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(srrsn);
    responseParam->setResult(result);
    state->peerRequests[srrsn].result = result;
    responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(responseParam);
    msg->appendSctpChunks(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    }
    else {
        msg->appendSctpChunks(createSack());
        stopTimer(SackTimer);
        state->ackState = 0;
        state->sackAlreadySent = true;
        if (state->incomingRequestSet && state->incomingRequest != nullptr) {
            delete state->incomingRequest;
            state->incomingRequest = nullptr;
            state->incomingRequestSet = false;
        }
        Packet *pkt = new Packet("RE_CONFIG");
        sendToIP(pkt, msg, remoteAddr);
    }
    if (result != DEFERRED) {
        state->resetRequested = false;
        state->firstPeerRequest = false;
    }
}

void SctpAssociation::sendDoubleStreamResetResponse(uint32_t insrrsn, uint16_t inresult, uint32_t outsrrsn, uint16_t outresult)
{
    SctpStreamResetChunk *resetChunk;
    EV_INFO << "sendDoubleStreamResetResponse to " << remoteAddr << "\n";
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    resetChunk = new SctpStreamResetChunk();
    resetChunk->setSctpChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SctpStreamResetResponseParameter *outResponseParam = new SctpStreamResetResponseParameter();
    outResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    outResponseParam->setSrResSn(outsrrsn);
    outResponseParam->setResult(outresult);
    outResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(outResponseParam);
    SctpStreamResetResponseParameter *inResponseParam = new SctpStreamResetResponseParameter();
    inResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    inResponseParam->setSrResSn(insrrsn);
    inResponseParam->setResult(inresult);
    state->peerRequests[insrrsn].result = inresult;
    state->peerRequests[outsrrsn].result = outresult;
    inResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(inResponseParam);
    msg->appendSctpChunks(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    }
    else {
        Packet *pkt = new Packet("RE_CONFIG");
        sendToIP(pkt, msg, remoteAddr);
    }
    if (outresult == PERFORMED || outresult == DENIED) {
        state->resetRequested = false;
        state->firstPeerRequest = false;
    }
}

void SctpAssociation::resetExpectedSsns()
{
    for (auto& elem : receiveStreams)
        elem.second->setExpectedStreamSeqNum(0);
    EV_INFO << "Expected Ssns have been resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_RCV_STREAMS_RESETTED);
}

void SctpAssociation::resetExpectedSsn(uint16_t id)
{
    auto iterator = receiveStreams.find(id);
    iterator->second->setExpectedStreamSeqNum(0);
    EV_INFO << "Expected Ssn " << id << " has been resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_RCV_STREAMS_RESETTED);
}

uint32_t SctpAssociation::getExpectedSsnOfStream(uint16_t id)
{
    uint16_t str;
    auto iterator = receiveStreams.find(id);
    str = iterator->second->getExpectedStreamSeqNum();
    return str;
}

uint32_t SctpAssociation::getSsnOfStream(uint16_t id)
{
    auto iterator = sendStreams.find(id);
    return iterator->second->getNextStreamSeqNum();
}

void SctpAssociation::resetSsns()
{
    for (auto& elem : sendStreams)
        elem.second->setNextStreamSeqNum(0);
    EV_INFO << "all SSns resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
}

void SctpAssociation::resetSsn(uint16_t id)
{
    auto iterator = sendStreams.find(id);
    iterator->second->setNextStreamSeqNum(0);
    EV_INFO << "SSn " << id << " resetted on " << localAddr << "\n";
}

bool SctpAssociation::sendStreamPresent(uint32_t id)
{
    return containsKey(sendStreams, id);
}

bool SctpAssociation::receiveStreamPresent(uint32_t id)
{
    return containsKey(receiveStreams, id);
}

} // namespace sctp
} // namespace inet

