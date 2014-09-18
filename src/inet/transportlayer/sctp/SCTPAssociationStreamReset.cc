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
    sctpmsg->setBitLength(SCTP_COMMON_HEADER * 8);
    SCTPStreamResetChunk *sctpreset = new SCTPStreamResetChunk("RESET");
    sctpreset = check_and_cast<SCTPStreamResetChunk *>(state->resetChunk->dup());
    sctpreset->setChunkType(STREAM_RESET);
    sctpmsg->addChunk(sctpreset);

    EV_INFO << "retransmitStreamReset localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::sendOutgoingResetRequest(SCTPIncomingSSNResetRequestParameter *requestParam)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(STREAM_RESET);
    resetChunk->setBitLength((SCTP_STREAM_RESET_CHUNK_LENGTH) * 8);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn++);
    outResetParam->setSrResSn(requestParam->getSrReqSn());
    outResetParam->setLastTsn(state->nextTSN - 1);
    outResetParam->setBitLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH * 8);
    resetChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn;

    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        SCTPResetTimer *rt = new SCTPResetTimer();
        rt->setInSN(0);
        rt->setInAcked(true);
        rt->setOutSN(srsn - 1);
        rt->setOutAcked(false);
        PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
        startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
        SCTPMessage *msg = new SCTPMessage();
        msg->setBitLength(SCTP_COMMON_HEADER * 8);
        msg->setSrcPort(localPort);
        msg->setDestPort(remotePort);
        msg->addChunk(resetChunk);
        state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());

        sendToIP(msg, remoteAddr);
    }
}

SCTPParameter *SCTPAssociation::makeOutgoingStreamResetParameter(uint32 srsn)
{
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn);
    outResetParam->setSrResSn(state->expectedStreamResetSequenceNumber - 3);
    outResetParam->setLastTsn(state->nextTSN - 1);
    outResetParam->setBitLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH * 8);
    return outResetParam;
}

SCTPParameter *SCTPAssociation::makeIncomingStreamResetParameter(uint32 srsn)
{
    SCTPIncomingSSNResetRequestParameter *inResetParam;
    inResetParam = new SCTPIncomingSSNResetRequestParameter("Incoming_Request_Param");
    inResetParam->setParameterType(INCOMING_RESET_REQUEST_PARAMETER);
    inResetParam->setSrReqSn(srsn);
    inResetParam->setBitLength(SCTP_INCOMING_RESET_REQUEST_PARAMETER_LENGTH * 8);
    return inResetParam;
}

SCTPParameter *SCTPAssociation::makeSSNTSNResetParameter(uint32 srsn)
{
    SCTPSSNTSNResetRequestParameter *resetParam;
    resetParam = new SCTPSSNTSNResetRequestParameter("SSN_TSN_Request_Param");
    resetParam->setParameterType(SSN_TSN_RESET_REQUEST_PARAMETER);
    resetParam->setSrReqSn(srsn);
    resetParam->setBitLength(SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH * 8);
    return resetParam;
}

void SCTPAssociation::sendOutgoingRequestAndResponse(uint32 inRequestSn, uint32 outRequestSn)
{
    EV_INFO << "sendOutgoingResetRequest to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER * 8);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(STREAM_RESET);
    resetChunk->setBitLength((SCTP_STREAM_RESET_CHUNK_LENGTH) * 8);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPResetTimer *rt = new SCTPResetTimer();
    SCTPOutgoingSSNResetRequestParameter *outResetParam;
    outResetParam = new SCTPOutgoingSSNResetRequestParameter("Outgoing_Request_Param");
    outResetParam->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
    outResetParam->setSrReqSn(srsn++);
    outResetParam->setSrResSn(inRequestSn);
    outResetParam->setLastTsn(state->nextTSN - 1);
    outResetParam->setBitLength(SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH * 8);
    resetChunk->addParameter(outResetParam);
    state->streamResetSequenceNumber = srsn;
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(outRequestSn);
    responseParam->setResult(1);
    responseParam->setBitLength(SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH * 8);
    resetChunk->addParameter(responseParam);
    rt->setInSN(srsn - 1);
    rt->setInAcked(false);
    rt->setOutSN(outRequestSn);
    rt->setOutAcked(false);
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
    msg->addChunk(resetChunk);
    sendToIP(msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SCTPAssociation::sendStreamResetRequest(uint16 type)
{
    EV_INFO << "StreamReset:sendStreamResetRequest\n";
    SCTPParameter *param;
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER * 8);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(STREAM_RESET);
    resetChunk->setBitLength((SCTP_STREAM_RESET_CHUNK_LENGTH) * 8);
    uint32 srsn = state->streamResetSequenceNumber;
    SCTPResetTimer *rt = new SCTPResetTimer();
    SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
    switch (type) {
        case RESET_OUTGOING:
            EV_INFO << "RESET_OUTGOING\n";
            param = makeOutgoingStreamResetParameter(srsn);
            resetChunk->addParameter(param);
            rt->setInSN(0);
            rt->setInAcked(true);
            rt->setOutSN(srsn);
            rt->setOutAcked(false);
            it->second.numResetRequestsSent++;
            break;

        case RESET_INCOMING:
            param = makeIncomingStreamResetParameter(srsn);
            resetChunk->addParameter(param);
            rt->setInSN(srsn);
            rt->setInAcked(false);
            rt->setOutSN(0);
            rt->setOutAcked(true);
            it->second.numResetRequestsSent++;
            break;

        case RESET_BOTH:
            SCTPParameter *inParam;
            inParam = makeIncomingStreamResetParameter(srsn);
            rt->setInSN(srsn++);
            rt->setInAcked(false);
            resetChunk->addParameter(inParam);
            SCTPParameter *outParam;
            outParam = makeOutgoingStreamResetParameter(srsn);
            resetChunk->addParameter(outParam);
            rt->setOutSN(srsn);
            rt->setOutAcked(false);
            it->second.numResetRequestsSent += 2;
            break;

        case SSN_TSN:
            param = makeSSNTSNResetParameter(srsn);
            resetChunk->addParameter(param);
            state->stopReceiving = true;
            rt->setInSN(0);
            rt->setInAcked(true);
            rt->setOutSN(srsn);
            rt->setOutAcked(false);
            it->second.numResetRequestsSent++;
            break;
    }
    state->streamResetSequenceNumber = ++srsn;
    state->resetChunk = check_and_cast<SCTPStreamResetChunk *>(resetChunk->dup());
    msg->addChunk(resetChunk);
    sendToIP(msg, remoteAddr);
    PK(getPath(remoteAddr)->ResetTimer)->encapsulate(rt);
    startTimer(getPath(remoteAddr)->ResetTimer, getPath(remoteAddr)->pathRto);
}

void SCTPAssociation::sendStreamResetResponse(SCTPSSNTSNResetRequestParameter *requestParam, bool options)
{
    uint32 len = 0;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << " with options\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER * 8);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPStreamResetChunk *resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(STREAM_RESET);
    resetChunk->setBitLength((SCTP_STREAM_RESET_CHUNK_LENGTH) * 8);
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(requestParam->getSrReqSn());
    responseParam->setResult(1);
    len = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH * 8;
    if (options) {
        responseParam->setSendersNextTsn(state->nextTSN);
        responseParam->setReceiversNextTsn(state->gapList.getHighestTSNReceived() + state->localRwnd);
        state->gapList.forwardCumAckTSN(state->gapList.getHighestTSNReceived() + state->localRwnd - 1);
        state->peerTsnAfterReset = state->gapList.getHighestTSNReceived() + state->localRwnd;
        resetSsns();
        resetExpectedSsns();
        state->stopOldData = true;
        len += 64;
    }
    responseParam->setBitLength(len);
    resetChunk->addParameter(responseParam);
    msg->addChunk(resetChunk);
    sendToIP(msg);
}

void SCTPAssociation::sendStreamResetResponse(uint32 srrsn)
{
    uint32 len = 0;
    SCTPStreamResetChunk *resetChunk;
    EV_INFO << "sendStreamResetResponse to " << remoteAddr << "\n";
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER * 8);
    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    resetChunk = new SCTPStreamResetChunk("STREAM_RESET");
    resetChunk->setChunkType(STREAM_RESET);
    resetChunk->setBitLength((SCTP_STREAM_RESET_CHUNK_LENGTH) * 8);
    SCTPStreamResetResponseParameter *responseParam = new SCTPStreamResetResponseParameter("Response_Param");
    responseParam->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
    responseParam->setSrResSn(srrsn);
    responseParam->setResult(1);
    len = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH * 8;
    responseParam->setBitLength(len);
    resetChunk->addParameter(responseParam);
    msg->addChunk(resetChunk);
    sendToIP(msg);
}

void SCTPAssociation::resetExpectedSsns()
{
    for (SCTPReceiveStreamMap::iterator iter = receiveStreams.begin(); iter != receiveStreams.end(); iter++)
        iter->second->setExpectedStreamSeqNum(0);
    EV_INFO << "Expected Ssns have been resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_RCV_STREAMS_RESETTED);
}

void SCTPAssociation::resetSsns()
{
    for (SCTPSendStreamMap::iterator iter = sendStreams.begin(); iter != sendStreams.end(); iter++)
        iter->second->setNextStreamSeqNum(0);
    EV_INFO << "SSns resetted on " << localAddr << "\n";
    sendIndicationToApp(SCTP_I_SEND_STREAMS_RESETTED);
}

} // namespace sctp

} // namespace inet

