//
// Copyright (C) 2005-2010 by Irene Ruengeler
// Copyright (C) 2009-2015 by Thomas Dreibholz
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

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpAlgorithm.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

//
// Event processing code
//

void SctpAssociation::process_ASSOCIATE(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg)
{
    L3Address lAddr, rAddr;
    SctpOpenReq *openCmd = check_and_cast<SctpOpenReq *>(sctpCommand);
    auto request = check_and_cast<Request *>(msg);
    auto& tags = getTags(request);
    auto interfaceReq = tags.findTag<InterfaceReq>();
    if (interfaceReq && interfaceReq->getInterfaceId() != -1) {
        sctpMain->setInterfaceId(interfaceReq->getInterfaceId());
    }

    EV_INFO << "SctpAssociationEventProc:process_ASSOCIATE\n";

    switch (fsm->getState()) {
        case SCTP_S_CLOSED:
            if (msg->getContextPointer() != NULL) {
                sctpMain->setSocketOptions((SocketOptions*) (msg->getContextPointer()));
            }
            initAssociation(openCmd);
            state->active = true;
            localAddressList = openCmd->getLocalAddresses();
            EV_INFO << "process_ASSOCIATE: number of local addresses=" << localAddressList.size() << "\n";
            lAddr = openCmd->getLocalAddresses().front();
            if (!(openCmd->getRemoteAddresses().empty())) {
                remoteAddressList = openCmd->getRemoteAddresses();
                rAddr = openCmd->getRemoteAddresses().front();
            }
            else
                rAddr = openCmd->getRemoteAddr();
            localPort = openCmd->getLocalPort();
            remotePort = openCmd->getRemotePort();
            EV_DETAIL << "open active with fd=" << fd << " and assocId=" << assocId << endl;
            fd = openCmd->getFd();
            state->streamReset = openCmd->getStreamReset();
            state->prMethod = openCmd->getPrMethod();
            state->numRequests = openCmd->getNumRequests();
            state->appLimited = openCmd->getAppLimited();
            if (rAddr.isUnspecified() || remotePort == 0)
                throw cRuntimeError("Error processing command OPEN_ACTIVE: remote address and port must be specified");

            if (localPort == 0) {
                localPort = sctpMain->getEphemeralPort();
            }
            EV_INFO << "OPEN: " << lAddr << ":" << localPort << " --> " << rAddr << ":" << remotePort << "\n";

            sctpMain->updateSockPair(this, lAddr, rAddr, localPort, remotePort);
            state->localRwnd = sctpMain->par("arwnd");
            sendInit();
            startTimer(T1_InitTimer, state->initRexmitTimeout);
            break;

        default:
            throw cRuntimeError("Error processing command OPEN_ACTIVE: connection already exists");
    }
}

void SctpAssociation::process_OPEN_PASSIVE(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg)
{
    L3Address lAddr;
    int16 localPort;

    SctpOpenReq *openCmd = check_and_cast<SctpOpenReq *>(sctpCommand);

    EV_DEBUG << "SctpAssociationEventProc:process_OPEN_PASSIVE\n";

    switch (fsm->getState()) {
        case SCTP_S_CLOSED:
            if (msg->getContextPointer() != NULL)
                sctpMain->setSocketOptions((SocketOptions*) (msg->getContextPointer()));
            initAssociation(openCmd);
            state->fork = openCmd->getFork();
            localAddressList = openCmd->getLocalAddresses();
            EV_DEBUG << "process_OPEN_PASSIVE: number of local addresses=" << localAddressList.size() << "\n";
            lAddr = openCmd->getLocalAddresses().front();
            localPort = openCmd->getLocalPort();
            inboundStreams = openCmd->getInboundStreams();
            outboundStreams = openCmd->getOutboundStreams();
            listening = true;
            fd = openCmd->getFd();
            EV_DETAIL << "open listening socket with fd=" << fd << " and assocId=" << assocId << endl;
            state->localRwnd = sctpMain->par("arwnd");
            state->localMsgRwnd = sctpMain->par("messageAcceptLimit");
            state->streamReset = openCmd->getStreamReset();
            state->numRequests = openCmd->getNumRequests();
            state->messagesToPush = openCmd->getMessagesToPush();

            if (localPort == 0)
                throw cRuntimeError("Error processing command OPEN_PASSIVE: local port must be specified");

            EV_DEBUG << "Assoc " << assocId << "::Starting to listen on: " << lAddr << ":" << localPort << "\n";

            sctpMain->updateSockPair(this, lAddr, L3Address(), localPort, 0);
            break;

        default:
            throw cRuntimeError("Error processing command OPEN_PASSIVE: connection already exists");
    }
}

void SctpAssociation::process_SEND(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg)
{
    SctpSendReq *sendCommand = check_and_cast<SctpSendReq *>(sctpCommand);

    if (fsm->getState() != SCTP_S_ESTABLISHED) {
        // TD 12.03.2009: since SCTP_S_ESTABLISHED is the only case, the
        // switch(...)-block has been removed for enhanced readability.
        EV_DEBUG << "process_SEND: state is not SCTP_S_ESTABLISHED -> returning" << endl;
        return;
    }

    EV_INFO << "process_SEND:"
             << " assocId=" << assocId
             << " localAddr=" << localAddr
             << " remoteAddr=" << remoteAddr
             << " cmdRemoteAddr=" << sendCommand->getRemoteAddr()
             << " cmdPrimary=" << (sendCommand->getPrimary() ? "true" : "false")
             << " appGateIndex=" << appGateIndex
             << " streamId=" << sendCommand->getSid() << endl;

    Packet *applicationPacket = check_and_cast<Packet *>(msg);
    const auto& applicationData = applicationPacket->peekDataAsBytes();
    int sendBytes = B(applicationData->getChunkLength()).get();
    EV_INFO << "got msg of length " << applicationData->getChunkLength() << " sendBytes=" << sendBytes << endl;

    auto iter = sctpMain->assocStatMap.find(assocId);
    iter->second.sentBytes += sendBytes;

    // ------ Prepare SctpDataMsg -----------------------------------------
    const uint32 streamId = sendCommand->getSid();
    const uint32 sendUnordered = sendCommand->getSendUnordered();
    const uint32 ppid = sendCommand->getPpid();
    SctpSendStream *stream = nullptr;
    auto associter = sendStreams.find(streamId);
    if (associter != sendStreams.end()) {
        stream = associter->second;
    }
    else {
        throw cRuntimeError("Stream with id %d not found", streamId);
    }

    SctpDataMsg *datMsg = new SctpDataMsg();
    SctpSimpleMessage *smsg = new SctpSimpleMessage("data");
    smsg->setDataArraySize(sendBytes);
    std::vector<uint8_t> vec = applicationData->getBytes();
    for (int i = 0; i < sendBytes; i++)
        smsg->setData(i, vec[i]);
    smsg->setDataLen(sendBytes);
    smsg->setEncaps(false);
    smsg->setByteLength(sendBytes);
    datMsg->encapsulate(smsg);
    datMsg->setSid(streamId);
    datMsg->setPpid(ppid);
    datMsg->setEnqueuingTime(simTime());
    datMsg->setSackNow(sendCommand->getSackNow());

    // ------ PR-SCTP & Drop messages to free buffer space ----------------
    datMsg->setPrMethod(sendCommand->getPrMethod());
    switch (sendCommand->getPrMethod()) {
        case PR_TTL:
            if (sendCommand->getPrValue() > 0) {
                datMsg->setExpiryTime(simTime() + sendCommand->getPrValue());
            }
            break;

        case PR_RTX:
            datMsg->setRtx((uint32)sendCommand->getPrValue());
            break;

        case PR_PRIO:
            datMsg->setPriority((uint32)sendCommand->getPrValue());
            state->queuedDroppableBytes += PK(msg)->getByteLength();
            break;
    }

    if ((state->appSendAllowed) &&
        (state->sendQueueLimit > 0) &&
        (state->queuedDroppableBytes > 0) &&
        ((uint64)state->sendBuffer >= state->sendQueueLimit))
    {
        uint32 lowestPriority;
        cQueue *strq;
        int64 dropsize = state->sendBuffer - state->sendQueueLimit;

        if (sendUnordered)
            strq = stream->getUnorderedStreamQ();
        else
            strq = stream->getStreamQ();

        while (dropsize >= 0 && state->queuedDroppableBytes > 0) {
            lowestPriority = 0;

            // Find lowest priority
            for (cQueue::Iterator iter(*strq); !iter.end(); iter++) {
                SctpDataMsg *msg = (SctpDataMsg *)(*iter);

                if (msg->getPriority() > lowestPriority)
                    lowestPriority = msg->getPriority();
            }

            // If just passed message has the lowest priority,
            // drop it and we're done.
            if (datMsg->getPriority() > lowestPriority) {
                EV_DEBUG << "msg will be abandoned, buffer is full and priority too low ("
                         << datMsg->getPriority() << ")\n";
                state->queuedDroppableBytes -= PK(msg)->getByteLength();
                delete datMsg;
                delete smsg;
                delete msg;
                sendIndicationToApp(SCTP_I_ABANDONED);
                return;
            }
        }
    }

    // ------ Set initial destination address -----------------------------
    if (sendCommand->getPrimary()) {
        if (sendCommand->getRemoteAddr().isUnspecified()) {
            datMsg->setInitialDestination(remoteAddr);
        }
        else {
            datMsg->setInitialDestination(sendCommand->getRemoteAddr());
        }
    }
    else {
        datMsg->setInitialDestination(state->getPrimaryPathIndex());
    }

    // ------ Optional padding and size calculations ----------------------
    if (state->padding) {
        datMsg->setBooksize(ADD_PADDING(smsg->getByteLength() + state->header));
    }
    else {
        datMsg->setBooksize(smsg->getByteLength() + state->header);
    }

    qCounter.roomSumSendStreams += ADD_PADDING(smsg->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
    qCounter.bookedSumSendStreams += datMsg->getBooksize();
    // Add chunk size to sender buffer size
    state->sendBuffer += smsg->getByteLength();

    datMsg->setMsgNum(++state->msgNum);

    // ------ Ordered/Unordered modes -------------------------------------
    if (sendUnordered == 1) {
        datMsg->setOrdered(false);
        stream->getUnorderedStreamQ()->insert(datMsg);
    }
    else {
        datMsg->setOrdered(true);
        stream->getStreamQ()->insert(datMsg);

        sendQueue->record(stream->getStreamQ()->getLength());
    }
    EV_INFO << "Size of send queue " << stream->getStreamQ()->getLength() << endl;
    // ------ Send buffer full? -------------------------------------------
    if ((state->appSendAllowed) &&
        (state->sendQueueLimit > 0) &&
        ((uint64)state->sendBuffer >= state->sendQueueLimit)) {
        // If there are not enough messages that could be dropped,
        // the buffer is really full and the app has to be notified.
        if (state->queuedDroppableBytes < state->sendBuffer - state->sendQueueLimit) {
            sendIndicationToApp(SCTP_I_SENDQUEUE_FULL);
            state->appSendAllowed = false;
        }
    }

    state->queuedMessages++;
    if ((state->queueLimit > 0) && (state->queuedMessages > state->queueLimit)) {
        state->queueUpdate = false;
    }
    EV_INFO << "process_SEND:"
             << " last=" << sendCommand->getLast()
             << "    queueLimit=" << state->queueLimit << endl;

    // ------ Call sendCommandInvoked() to send message -------------------
    // sendCommandInvoked() itself will call sendOnAllPaths() ...
    if (sendCommand->getLast() == true) {
        if (sendCommand->getPrimary()) {
            sctpAlgorithm->sendCommandInvoked(nullptr);
        }
        else {
            sctpAlgorithm->sendCommandInvoked(getPath(datMsg->getInitialDestination()));
        }
    }
}

void SctpAssociation::process_RECEIVE_REQUEST(SctpEventCode& event, SctpCommandReq *sctpCommand)
{
    EV_INFO << "SctpAssociation::process_RECEIVE_REQUEST\n";
    SctpSendReq *sendCommand = check_and_cast<SctpSendReq *>(sctpCommand);
    if ((uint32)sendCommand->getSid() > inboundStreams || sendCommand->getSid() < 0) {
        EV_DEBUG << "Application tries to read from invalid stream id....\n";
    }
    state->numMsgsReq[sendCommand->getSid()] += sendCommand->getNumMsgs();
    pushUlp();
}

void SctpAssociation::process_PRIMARY(SctpEventCode& event, SctpCommandReq *sctpCommand)
{
    SctpPathInfo *pinfo = check_and_cast<SctpPathInfo *>(sctpCommand);
    state->setPrimaryPath(getPath(pinfo->getRemoteAddress()));
}

void SctpAssociation::process_STREAM_RESET(SctpCommandReq *sctpCommand)
{
    EV_INFO << "process_STREAM_RESET request arriving from App\n";
    SctpResetReq *rinfo = check_and_cast<SctpResetReq *>(sctpCommand);
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        if (rinfo->getRequestType() == ADD_BOTH) {
            sendAddInAndOutStreamsRequest(rinfo);
        } else if (!state->fragInProgress && state->outstandingBytes == 0) {
            sendStreamResetRequest(rinfo);
            if (rinfo->getRequestType() == RESET_OUTGOING || rinfo->getRequestType() == RESET_BOTH ||
                    rinfo->getRequestType() == SSN_TSN || rinfo->getRequestType() == ADD_INCOMING ||
                    rinfo->getRequestType() == ADD_OUTGOING) {
                state->resetPending = true;
            }
        } else if (state->outstandingBytes > 0) {
            if (rinfo->getRequestType() == RESET_OUTGOING || rinfo->getRequestType() == RESET_INCOMING || rinfo->getRequestType() == RESET_BOTH) {
                if (rinfo->getStreamsArraySize() > 0) {
                    for (uint16 i = 0; i < rinfo->getStreamsArraySize(); i++) {
                        if ((getBytesInFlightOfStream(rinfo->getStreams(i)) > 0) ||
                                getFragInProgressOfStream(rinfo->getStreams(i)) ||
                                !orderedQueueEmptyOfStream(rinfo->getStreams(i)) ||
                                !unorderedQueueEmptyOfStream(rinfo->getStreams(i))) {
                            state->streamsPending.push_back(rinfo->getStreams(i));
                        } else {
                            state->streamsToReset.push_back(rinfo->getStreams(i));
                        }
                    }
                } else {
                    if (rinfo->getRequestType() == RESET_OUTGOING) {
                        for (uint16 i = 0; i < outboundStreams; i++) {
                            if ((getBytesInFlightOfStream(i) > 0) || getFragInProgressOfStream(i)) {
                                state->streamsPending.push_back(i);
                            } else {
                                state->streamsToReset.push_back(i);
                            }
                        }
                    }
                }
                if (state->streamsToReset.size() > 0) {
                    sendStreamResetRequest(rinfo);
                    state->resetPending = true;
                }
            }
            if ((rinfo->getRequestType() == SSN_TSN) ||
                    (rinfo->getRequestType() == ADD_INCOMING) ||
                    (rinfo->getRequestType() == ADD_OUTGOING)) {
                state->resetInfo = rinfo;
               // state->resetInfo->setName("state-resetLater");
                state->localRequestType = state->resetInfo->getRequestType();
            }
            if (!state->resetPending || state->streamsPending.size() > 0) {
                state->resetInfo = rinfo->dup();
               // state->resetInfo->setName("state-resetInfo");
                state->localRequestType = state->resetInfo->getRequestType();
            }
        }

        state->resetRequested = true;
    }
}

void SctpAssociation::process_QUEUE_MSGS_LIMIT(const SctpCommandReq *sctpCommand)
{
    const SctpInfoReq *qinfo = check_and_cast<const SctpInfoReq *>(sctpCommand);
    state->queueLimit = qinfo->getText();
}

void SctpAssociation::process_QUEUE_BYTES_LIMIT(const SctpCommandReq *sctpCommand)
{
    const SctpInfoReq *qinfo = check_and_cast<const SctpInfoReq *>(sctpCommand);
    state->sendQueueLimit = qinfo->getText();
}

void SctpAssociation::process_CLOSE(SctpEventCode& event)
{
    EV_DEBUG << "SctpAssociationEventProc:process_CLOSE; assoc=" << assocId << endl;
    switch (fsm->getState()) {
        case SCTP_S_ESTABLISHED:
            sendOnAllPaths(state->getPrimaryPath());
            sendShutdown();
            break;

        case SCTP_S_SHUTDOWN_RECEIVED:
            if (getOutstandingBytes() == 0) {
                sendShutdownAck(remoteAddr);
            }
            break;
    }
}

void SctpAssociation::process_ABORT(SctpEventCode& event)
{
    EV_DEBUG << "SctpAssociationEventProc:process_ABORT; assoc=" << assocId << endl;
    switch (fsm->getState()) {
        case SCTP_S_ESTABLISHED:
            sendOnAllPaths(state->getPrimaryPath());
            sendAbort();
            break;
    }
}

void SctpAssociation::process_STATUS(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg)
{
    auto& tags = getTags(msg);
    SctpStatusReq *statusInfo = tags.addTagIfAbsent<SctpStatusReq>();
    statusInfo->setState(fsm->getState());
    statusInfo->setStateName(stateName(fsm->getState()));
    statusInfo->setPathId(remoteAddr);
    statusInfo->setActive(getPath(remoteAddr)->activePath);
    sendToApp(msg);
}

} // namespace sctp
} // namespace inet

