//
// Copyright (C) 2008 Irene Ruengeler
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

#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

namespace inet {

namespace sctp {
void SCTPAssociation::retransmitReset()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);
    SCTPStreamResetChunk *sctpreset = check_and_cast<SCTPStreamResetChunk *>(state->resetChunk->dup());
    state->numResetRequests = sctpreset->getParametersArraySize();
    sctpreset->setName("RESETRetransmit");
    sctpreset->setChunkType(RE_CONFIG);
    sctpmsg->addChunk(sctpreset);
    state->waitForResponse = true;
    EV_INFO << "retransmitStreamReset localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::checkStreamsToReset()
{
    if (!state->resetPending && qCounter.roomSumSendStreams == 0 && !state->fragInProgress && (state->resetRequested || state->incomingRequest != nullptr) && (state->outstandingBytes == 0 || state->streamsPending.size() > 0)) {
        if (state->localRequestType == RESET_OUTGOING || state->peerRequestType == RESET_INCOMING) {
            state->streamsToReset.clear();
            std::list<uint16>::iterator it;
            for (it = state->streamsPending.begin(); it != state->streamsPending.end(); it++) {
                if (getBytesInFlightOfStream(*it) == 0) {
                    state->streamsToReset.push_back(*it);
                    state->streamsPending.erase(it);
                }
            }
        }
        if (!state->resetPending && state->resetRequested &&
            (state->localRequestType == SSN_TSN ||
             state->localRequestType == ADD_OUTGOING ||
             state->localRequestType == ADD_INCOMING)) {
            sendStreamResetRequest(state->resetInfo);
            state->resetPending = true;
        }
        if (state->resetDeferred && (state->streamsToReset.size() > 0 || state->peerRequestType == SSN_TSN)) {
            switch (state->peerRequestType) {
                case SSN_TSN: {
                    SCTPSSNTSNResetRequestParameter *ssnParam = check_and_cast<SCTPSSNTSNResetRequestParameter *>(state->incomingRequest);
                    processSSNTSNResetRequestArrived(ssnParam);
                    if (state->sendResponse == PERFORMED_WITH_OPTION) {
                        resetExpectedSsns();
                        state->sendResponse = 0;
                        sendStreamResetResponse((SCTPSSNTSNResetRequestParameter *)state->incomingRequest, PERFORMED, true);
                    }
                    delete ssnParam;
                    break;
                }
                case RESET_INCOMING: {
                    SCTPIncomingSSNResetRequestParameter *inParam = check_and_cast<SCTPIncomingSSNResetRequestParameter *>(state->incomingRequest->dup());
                    state->incomingRequest->setName("StateSackIncoming");
                    inParam->setName("checkInParam");
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
        } else if ((state->localRequestType == RESET_OUTGOING || state->peerRequestType == RESET_INCOMING) &&
                   (state->streamsToReset.size() > 0)) {
            if (state->peerRequestType == RESET_INCOMING)
                state->localRequestType = RESET_OUTGOING;
            sendStreamResetRequest(state->resetInfo);
            state->resetPending = true;
        }
    }
}

void SCTPAssociation::sendOutgoingResetRequest(SCTPIncomingSSNResetRequestParameter *requestParam)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    uint16 len = 0;
    uint32 srsn = 0;
    if ((!(getPath(remoteAddr)->ResetTimer->isScheduled())) || state->requestsOverlap) {
        state->requestsOverlap = false;
        SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("OutRE_CONFIG");
        resetChunk->setChunkType(RE_CONFIG);
        resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        SCTPOutgoingSSNResetRequestParameter *outResetParam;
        outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_SSN_Request_Param");
        outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
        SCTPStateVariables::RequestData *resDat = state->findTypeInRequests(OUTGOING_RESET_REQUEST_PARAMETER);
        if (resDat != nullptr && resDat->sn == state->streamResetSequenceNumber - 1) {
            srsn = state->streamResetSequenceNumber - 1;
        } else {
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
        outResetParam->setLastTsn(state->nextTSN - 1);
        if (state->peerStreamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->peerStreamsToReset.size());
            uint16 i = 0;
            for (std::list<uint16>::iterator it = state->peerStreamsToReset.begin(); it != state->peerStreamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn-1].streams.push_back(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->peerStreamsToReset.size() * 2;
            state->peerStreamsToReset.clear();
        } else if (state->streamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
            uint16 i = 0;
            for (std::list<uint16>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn-1].streams.push_back(outResetParam->getStreamNumbers(i));
                resetSsn(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->streamsToReset.size() * 2;
            state->streamsToReset.clear();
        } else if (requestParam->getStreamNumbersArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(requestParam->getStreamNumbersArraySize());
            for (uint16 i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                outResetParam->setStreamNumbers(i, requestParam->getStreamNumbers(i));
                state->requests[srsn-1].streams.push_back(requestParam->getStreamNumbers(i));
            }
            len = requestParam->getStreamNumbersArraySize() * 2;
        }
        outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len);
        resetChunk->addParameter(outResetParam);
        state->streamResetSequenceNumber = srsn;
        state->resetRequested = true;

        SCTPResetTimer *rt = new SCTPResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn - 1);
        rt->setOutAcked(false);

        SCTPMessage *msg = new SCTPMessage();
        msg->setByteLength(SCTP_COMMON_HEADER);
        msg->setSrcPort(localPort);
        msg->setDestPort(remotePort);
        msg->addChunk(resetChunk);
        state->localRequestType = RESET_OUTGOING;
        if (state->resetChunk != nullptr) {
            delete state->resetChunk;
            state->resetChunk = nullptr;
        }
        state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
        if (qCounter.roomSumSendStreams != 0) {
            storePacket(getPath(remoteAddr), msg, 1, 0, false);
            state->bundleReset = true;
            rt->setName("bundleReset");
            sendOnPath(getPath(remoteAddr), true);
            state->bundleReset = false;
        } else {
            sendToIP(msg, remoteAddr);
        }
        if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
            delete (PK(getPath(remoteAddr)->ResetTimer)->decapsulate());
        }
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        if (getPath(remoteAddr)->ResetTimer->isScheduled()) {
            stopTimer(getPath(remoteAddr)->ResetTimer);
        }
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
    }
}

void SCTPAssociation::sendBundledOutgoingResetAndResponse(SCTPIncomingSSNResetRequestParameter *requestParam)
{
    EV_INFO << "sendBundledOutgoingResetAndResponse to " << remoteAddr << "\n";
    uint16 len = 0;
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("Outbundle_CONFIG");
        resetChunk->setChunkType(RE_CONFIG);
        resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        uint32 srsn = state->streamResetSequenceNumber;
        SCTPOutgoingSSNResetRequestParameter *outResetParam;
        outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_SSN_B_Request_Param");
        outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
        state->requests[srsn].result = 100;
        state->requests[srsn].type = OUTGOING_RESET_REQUEST_PARAMETER;
        state->numResetRequests++;
        outResetParam->setSrReqSn(srsn++);
        outResetParam->setSrResSn(requestParam->getSrReqSn());
        outResetParam->setLastTsn(state->nextTSN - 1);
        if (state->streamsToReset.size() > 0) {
            outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
            uint16 i = 0;
            for (std::list<uint16>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
                outResetParam->setStreamNumbers(i, *it);
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                resetSsn(outResetParam->getStreamNumbers(i));
                i++;
            }
            len = state->streamsToReset.size() * 2;
            state->streamsToReset.clear();
        } else if (requestParam->getStreamNumbersArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(requestParam->getStreamNumbersArraySize());
            for (uint16 i = 0; i < requestParam->getStreamNumbersArraySize(); i++) {
                outResetParam->setStreamNumbers(i, requestParam->getStreamNumbers(i));
            }
            len = requestParam->getStreamNumbersArraySize() * 2;
        }
        outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len);
        resetChunk->addParameter(outResetParam);
        state->streamResetSequenceNumber = srsn;

        SCTPStreamResetChunk *resetResponseChunk;
        EV_INFO << "sendbundledStreamResetResponse to " << remoteAddr << "\n";
        resetResponseChunk = new SCTPStreamResetChunk("responseRE_CONFIG");
        resetResponseChunk->setChunkType(RE_CONFIG);
        resetResponseChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
        SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
        responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
        responseParam->setSrResSn(requestParam->getSrReqSn());
        responseParam->setResult(PERFORMED);
        responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
        resetResponseChunk->addParameter(responseParam);
        state->resetRequested = false;

        SCTPResetTimer *rt = new SCTPResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn - 1);
        rt->setOutAcked(false);

        SCTPMessage *msg = new SCTPMessage();
        msg->setByteLength(SCTP_COMMON_HEADER);
        msg->setSrcPort(localPort);
        msg->setDestPort(remotePort);
        msg->addChunk(resetChunk);
        msg->addChunk(resetResponseChunk);
        state->localRequestType = RESET_OUTGOING;
        if (state->resetChunk != nullptr) {
            delete state->resetChunk;
            state->resetChunk = nullptr;
        }
        state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
        state->resetChunk->setName("State_Resetchunk");
        if (qCounter.roomSumSendStreams != 0) {
            storePacket(getPath(remoteAddr), msg, 1, 0, false);
            state->bundleReset = true;
            rt->setName("bundleReset");
            sendOnPath(getPath(remoteAddr), true);
            state->bundleReset = false;
        } else {
            sendToIP(msg, remoteAddr);
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

SCTPParameter *SCTPAssociation::makeOutgoingStreamResetParameter(uint32 srsn, SCTPResetInfo *info)
{
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn);
    outResetParam->setSrResSn(state->expectedStreamResetSequenceNumber - 1);
    outResetParam->setLastTsn(state->nextTSN - 1);
    state->requests[srsn].lastTsn = outResetParam->getLastTsn();
    if (state->outstandingBytes == 0 && state->streamsToReset.size() == 0) {
        if (info->getStreamsArraySize() > 0) {
            outResetParam->setStreamNumbersArraySize(info->getStreamsArraySize());
            for (uint i = 0; i < info->getStreamsArraySize(); i++) {
                outResetParam->setStreamNumbers(i, (uint16)info->getStreams(i));
                state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
                state->requests[srsn].streams.push_back(outResetParam->getStreamNumbers(i));
            }
        }
    } else if (state->streamsToReset.size() > 0) {
        outResetParam->setStreamNumbersArraySize(state->streamsToReset.size());
        uint16 i = 0;
        for (std::list<uint16>::iterator it = state->streamsToReset.begin(); it != state->streamsToReset.end(); ++it) {
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

SCTPParameter *SCTPAssociation::makeIncomingStreamResetParameter(uint32 srsn, SCTPResetInfo *info)
{
    SCTPIncomingSSNResetRequestParameter *inResetParam;
    inResetParam = new SCTPIncomingSSNResetRequestParameter("Incoming_Request_Param");
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

SCTPParameter *SCTPAssociation::makeAddStreamsRequestParameter(uint32 srsn, SCTPResetInfo *info)
{
    SCTPAddStreamsRequestParameter *addStreams = new SCTPAddStreamsRequestParameter("Add_Streams");
    if (info->getInstreams() > 0) {
        addStreams->setParameterType(ADD_INCOMING_STREAMS_REQUEST_PARAMETER);
        addStreams->setNumberOfStreams(info->getInstreams());
        state->localRequestType = ADD_INCOMING;
        state->requests[srsn].type = ADD_INCOMING_STREAMS_REQUEST_PARAMETER;
        state->numAddedInStreams = addStreams->getNumberOfStreams();
    } else if (info->getOutstreams() > 0) {
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

SCTPParameter *SCTPAssociation::makeSSNTSNResetParameter(uint32 srsn)
{
    SCTPSSNTSNResetRequestParameter *resetParam;
    resetParam = new SCTPSSNTSNResetRequestParameter("SSN_TSN_Request_Param");
    resetParam->setParameterType(SSN_TSN_RESET_REQUEST_PARAMETER);
    resetParam->setSrReqSn(srsn);
    resetParam->setByteLength(SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH);
    state->localRequestType = SSN_TSN;
    return resetParam;
}

void SCTPAssociation::sendOutgoingRequestAndResponse(uint32 inRequestSn, uint32 outRequestSn)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resChunk = new SCTPStreamResetChunk("ReqResRE_CONFIG");
    resChunk->setChunkType(RE_CONFIG);
    resChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPResetTimer *rt = new SCTPResetTimer();
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn++);
    outResetParam->setSrResSn(inRequestSn);
    outResetParam->setLastTsn(state->nextTSN - 1);
    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH);
    resChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn;
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
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
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resChunk->dup());
    state->resetChunk->setName("stateRstChunk");
    msg->addChunk(resChunk);
    sendToIP(msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SCTPAssociation::sendOutgoingRequestAndResponse(SCTPIncomingSSNResetRequestParameter *inRequestParam,
        SCTPOutgoingSSNResetRequestParameter *outRequestParam)
{
    uint16 len = 0;
    EV_INFO << "sendOutgoingRequestAndResponse to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resChunk = new SCTPStreamResetChunk("ReqResRE_CONFIG");
    resChunk->setChunkType(RE_CONFIG);
    resChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPResetTimer *rt = new SCTPResetTimer();
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Reset_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn++);
    outResetParam->setSrResSn(inRequestParam->getSrReqSn());
    outResetParam->setLastTsn(state->nextTSN - 1);
    if (inRequestParam->getStreamNumbersArraySize() > 0) {
        outResetParam->setStreamNumbersArraySize(inRequestParam->getStreamNumbersArraySize());
        for (uint i = 0; i < inRequestParam->getStreamNumbersArraySize(); i++) {
            outResetParam->setStreamNumbers(i, (uint16)inRequestParam->getStreamNumbers(i));
            state->resetOutStreams.push_back(outResetParam->getStreamNumbers(i));
            state->requests[srsn].streams.push_back(outResetParam->getStreamNumbers(i));
            len++;
        }
    }
    outResetParam->setByteLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + len * 2);
    resChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn;
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
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
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resChunk->dup());
    state->resetChunk->setName("stateRstChunk");
    msg->addChunk(resChunk);
    sendToIP(msg, remoteAddr);
    if (PK(getPath(remoteAddr)->ResetTimer)->hasEncapsulatedPacket()) {
        PK(getPath(remoteAddr)->ResetTimer)->decapsulate();
    }
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SCTPAssociation::sendStreamResetRequest(SCTPResetInfo *rinfo)
{
    EV_INFO << "StreamReset:sendStreamResetRequest\n";
    SCTPParameter *param;
    uint16 type;
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("myRE_CONFIG");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPResetTimer *rt = new SCTPResetTimer();
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
            SCTPParameter *outParam;
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
            SCTPParameter *inParam;
            inParam = makeIncomingStreamResetParameter(srsn, rinfo);
            resetChunk->addParameter(inParam);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            it->second.numResetRequestsSent += 2;
            state->numResetRequests += 2;
            state->waitForResponse = true;
            break;

        case SSN_TSN:
            param = makeSSNTSNResetParameter(srsn);
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
        default: printf("Request type %d not known\n", type);

    }
    state->streamResetSequenceNumber = ++srsn;
    if (state->resetChunk != nullptr) {
        state->resetChunk->setName("deletedStateReSetChunk");
        delete state->resetChunk;
        state->resetChunk = nullptr;
    }
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
    state->resetChunk->setName("stateReSetChunk");
    msg->addChunk(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        rt->setName("bundleReset");
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    } else {
        sendToIP(msg, remoteAddr);
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

void SCTPAssociation::sendAddOutgoingStreamsRequest(uint16 numStreams)
{
    EV_INFO << "StreamReset:sendAddOutgoingStreamsRequest\n";
    uint32 srsn = 0;
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("AddRE_CONFIG");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SCTPStateVariables::RequestData *resDat = state->findTypeInRequests(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER);
    if (resDat != nullptr && resDat->sn == state->streamResetSequenceNumber - 1) {
        srsn = state->streamResetSequenceNumber - 1;
    } else {
        srsn = state->streamResetSequenceNumber;
        state->requests[srsn].result = 100;
        state->requests[srsn].type = ADD_OUTGOING_STREAMS_REQUEST_PARAMETER;
        state->requests[srsn].sn = srsn;
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numResetRequestsSent++;
    }
    SCTPAddStreamsRequestParameter *addStreams = new SCTPAddStreamsRequestParameter("Add_Streams");
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
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
    state->resetChunk->setName("stateAddResetChunk");
    msg->addChunk(resetChunk);
    sendToIP(msg, remoteAddr);
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        SCTPResetTimer *rt = new SCTPResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn);
        rt->setOutAcked(false);
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
    }
    state->streamResetSequenceNumber = ++srsn;
}

void SCTPAssociation::sendAddInAndOutStreamsRequest(SCTPResetInfo *info)
{
    EV_INFO << "StreamReset:sendAddInandStreamsRequest\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPResetTimer *rt = new SCTPResetTimer();
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("AddInOutCONFIG");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPAddStreamsRequestParameter *addOutStreams = new SCTPAddStreamsRequestParameter("Add_Out_Streams");
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
    SCTPAddStreamsRequestParameter *addInStreams = new SCTPAddStreamsRequestParameter("Add_In_Streams");
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
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
    state->resetChunk->setName("stateAddInOutResetChunk");
    msg->addChunk(resetChunk);
    sendToIP(msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SCTPAssociation::resetGapLists()
{
    uint32 newCumAck = state->gapList.getHighestTSNReceived() + (1<<31);
    state->gapList.resetGaps(newCumAck);
}

void SCTPAssociation::sendStreamResetResponse(SCTPSSNTSNResetRequestParameter *requestParam, int result, bool options)
{
    uint32 len = 0;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << " with options\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(requestParam->getSrReqSn());
    responseParam->setResult(result);
    state->peerRequests[requestParam->getSrReqSn()].result = result;
    len = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH;
    if (options && result == PERFORMED) {
        responseParam->setSendersNextTsn(state->nextTSN);
        responseParam->setReceiversNextTsn(state->gapList.getHighestTSNReceived() + (1<<31) + 1);
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
        state->peerTsnAfterReset = state->gapList.getHighestTSNReceived() + (1<<31);
        state->firstPeerRequest = false;
    }
    responseParam->setByteLength(len);
    resetChunk->addParameter(responseParam);
    msg->addChunk(resetChunk);
    stopTimer(SackTimer);
    msg->addChunk(createSack());
    state->ackState = 0;
    state->sackAlreadySent = true;
    sendToIP(msg);
}

void SCTPAssociation::sendStreamResetResponse(uint32 srrsn, int result)
{
    SCTPStreamResetChunk *resetChunk;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    resetChunk = new SCTPStreamResetChunk("responseRE_CONFIG");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(srrsn);
    responseParam->setResult(result);
    state->peerRequests[srrsn].result = result;
    responseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(responseParam);
    msg->addChunk(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    } else {
        msg->addChunk(createSack());
        stopTimer(SackTimer);
        state->ackState = 0;
        state->sackAlreadySent = true;
        if (state->incomingRequestSet && state->incomingRequest != nullptr) {
            delete state->incomingRequest;
            state->incomingRequest = nullptr;
            state->incomingRequestSet = false;
        }
        sendToIP(msg, remoteAddr);
    }
    if (result != DEFERRED) {
        state->resetRequested = false;
        state->firstPeerRequest = false;
    }
}

void SCTPAssociation::sendDoubleStreamResetResponse(uint32 insrrsn, uint16 inresult, uint32 outsrrsn, uint16 outresult)
{
    SCTPStreamResetChunk *resetChunk;
    EV_INFO << "sendDoubleStreamResetResponse to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setByteLength(SCTP_COMMON_HEADER);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    resetChunk = new SCTPStreamResetChunk("responseRE_CONFIG");
    resetChunk->setChunkType(RE_CONFIG);
    resetChunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    SCTPStreamResetResponseParameter *outResponseParam = new SCTPStreamResetResponseParameter("Out_Response_Param");
    outResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    outResponseParam->setSrResSn(outsrrsn);
    outResponseParam->setResult(outresult);
    outResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(outResponseParam);
    SCTPStreamResetResponseParameter *inResponseParam = new SCTPStreamResetResponseParameter("In_Response_Param");
    inResponseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    inResponseParam->setSrResSn(insrrsn);
    inResponseParam->setResult(inresult);
    state->peerRequests[insrrsn].result = inresult;
    state->peerRequests[outsrrsn].result = outresult;
    inResponseParam->setByteLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH);
    resetChunk->addParameter(inResponseParam);
    msg->addChunk(resetChunk);
    if (qCounter.roomSumSendStreams != 0) {
        storePacket(getPath(remoteAddr), msg, 1, 0, false);
        state->bundleReset = true;
        sendOnPath(getPath(remoteAddr), true);
        state->bundleReset = false;
    } else {
        sendToIP(msg, remoteAddr);
    }
    if (outresult == PERFORMED || outresult == DENIED) {
        state->resetRequested = false;
        state->firstPeerRequest = false;
    }
}

void SCTPAssociation::resetExpectedSsns()
{
    for (auto & elem : receiveStreams)
        elem.second->setExpectedStreamSeqNum(0);
    EV_INFO << "Expected Ssns have been resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_RCV_STREAMS_RESETTED);
}

void SCTPAssociation::resetExpectedSsn(uint16 id)
{
    auto iterator = receiveStreams.find(id);
    iterator->second->setExpectedStreamSeqNum(0);
    EV_INFO << "Expected Ssn " << id <<" has been resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_RCV_STREAMS_RESETTED);
}

uint32 SCTPAssociation::getExpectedSsnOfStream(uint16 id)
{
    uint16 str;
    auto iterator = receiveStreams.find(id);
    str = iterator->second->getExpectedStreamSeqNum();
    return str;
}

uint32 SCTPAssociation::getSsnOfStream(uint16 id)
{
    auto iterator = sendStreams.find(id);
    return (iterator->second->getNextStreamSeqNum());
}


void SCTPAssociation::resetSsns()
{
    for (auto & elem : sendStreams)
        elem.second->setNextStreamSeqNum(0);
    EV_INFO << "SSns resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
}

void SCTPAssociation::resetSsn(uint16 id)
{
    auto iterator = sendStreams.find(id);
    iterator->second->setNextStreamSeqNum(0);
    EV_INFO << "SSns resetted on " << localAddr << "\n";
}

bool SCTPAssociation::sendStreamPresent(uint16 id)
{
    auto iterator = sendStreams.find(id);
    if (iterator == sendStreams.end())
        return false;
    else
        return true;
}

bool SCTPAssociation::receiveStreamPresent(uint16 id)
{
    auto iterator = receiveStreams.find(id);
    if (iterator == receiveStreams.end())
        return false;
    else
        return true;
}

} // namespace sctp

} // namespace inet
