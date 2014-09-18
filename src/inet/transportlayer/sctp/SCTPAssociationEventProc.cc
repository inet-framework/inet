//
// Copyright (C) 2005-2010 by Irene Ruengeler
// Copyright (C) 2009-2012 by Thomas Dreibholz
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
#include "inet/transportlayer/sctp/SCTP.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPAlgorithm.h"

namespace inet {

namespace sctp {

//
// Event processing code
//

void SCTPAssociation::process_ASSOCIATE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    L3Address lAddr, rAddr;

    SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);

    EV_INFO << "SCTPAssociationEventProc:process_ASSOCIATE\n";

    switch (fsm->getState()) {
        case SCTP_S_CLOSED:
            initAssociation(openCmd);
            state->active = true;
            localAddressList = openCmd->getLocalAddresses();
            lAddr = openCmd->getLocalAddresses().front();
            if (!(openCmd->getRemoteAddresses().empty())) {
                remoteAddressList = openCmd->getRemoteAddresses();
                rAddr = openCmd->getRemoteAddresses().front();
            }
            else
                rAddr = openCmd->getRemoteAddr();
            localPort = openCmd->getLocalPort();
            remotePort = openCmd->getRemotePort();
            state->streamReset = openCmd->getStreamReset();
            state->prMethod = openCmd->getPrMethod();
            state->numRequests = openCmd->getNumRequests();
            if (rAddr.isUnspecified() || remotePort == 0)
                throw cRuntimeError("Error processing command OPEN_ACTIVE: remote address and port must be specified");

            if (localPort == 0) {
                localPort = sctpMain->getEphemeralPort();
            }
            EV_INFO << "OPEN: " << lAddr << ":" << localPort << " --> " << rAddr << ":" << remotePort << "\n";

            sctpMain->updateSockPair(this, lAddr, rAddr, localPort, remotePort);
            state->localRwnd = (long)sctpMain->par("arwnd");
            sendInit();
            startTimer(T1_InitTimer, state->initRexmitTimeout);
            break;

        default:
            throw cRuntimeError("Error processing command OPEN_ACTIVE: connection already exists");
    }
}

void SCTPAssociation::process_OPEN_PASSIVE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    L3Address lAddr;
    int16 localPort;

    SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);

    EV_DEBUG << "SCTPAssociationEventProc:process_OPEN_PASSIVE\n";

    switch (fsm->getState()) {
        case SCTP_S_CLOSED:
            initAssociation(openCmd);
            state->fork = openCmd->getFork();
            localAddressList = openCmd->getLocalAddresses();
            EV_DEBUG << "process_OPEN_PASSIVE: number of local addresses=" << localAddressList.size() << "\n";
            lAddr = openCmd->getLocalAddresses().front();
            localPort = openCmd->getLocalPort();
            inboundStreams = openCmd->getInboundStreams();
            outboundStreams = openCmd->getOutboundStreams();
            state->localRwnd = (long)sctpMain->par("arwnd");
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

void SCTPAssociation::process_SEND(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    SCTPSendCommand *sendCommand = check_and_cast<SCTPSendCommand *>(sctpCommand);

    if (fsm->getState() != SCTP_S_ESTABLISHED) {
        // TD 12.03.2009: since SCTP_S_ESTABLISHED is the only case, the
        // switch(...)-block has been removed for enhanced readability.
        EV_DEBUG << "process_SEND: state is not SCTP_S_ESTABLISHED -> returning" << endl;
        return;
    }

    EV_DEBUG << "process_SEND:"
             << " assocId=" << assocId
             << " localAddr=" << localAddr
             << " remoteAddr=" << remoteAddr
             << " cmdRemoteAddr=" << sendCommand->getRemoteAddr()
             << " cmdPrimary=" << (sendCommand->getPrimary() ? "true" : "false")
             << " appGateIndex=" << appGateIndex
             << " streamId=" << sendCommand->getSid() << endl;

    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>((msg->decapsulate()));
    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
    iter->second.sentBytes += smsg->getBitLength() / 8;

    // ------ Prepare SCTPDataMsg -----------------------------------------
    const uint32 streamId = sendCommand->getSid();
    const uint32 sendUnordered = sendCommand->getSendUnordered();
    const uint32 ppid = sendCommand->getPpid();
    SCTPSendStream *stream = NULL;
    SCTPSendStreamMap::iterator associter = sendStreams.find(streamId);
    if (associter != sendStreams.end()) {
        stream = associter->second;
    }
    else {
        throw cRuntimeError("Stream with id %d not found", streamId);
    }

    char name[64];
    snprintf(name, sizeof(name), "SDATA-%d-%d", streamId, state->msgNum);
    smsg->setName(name);

    SCTPDataMsg *datMsg = new SCTPDataMsg();
    datMsg->encapsulate(smsg);
    datMsg->setSid(streamId);
    datMsg->setPpid(ppid);
    datMsg->setEnqueuingTime(simulation.getSimTime());
    datMsg->setSackNow(sendCommand->getSackNow());

    // ------ PR-SCTP & Drop messages to free buffer space ----------------
    datMsg->setPrMethod(sendCommand->getPrMethod());
    switch (sendCommand->getPrMethod()) {
        case PR_TTL:
            if (sendCommand->getPrValue() > 0) {
                datMsg->setExpiryTime(simulation.getSimTime() + sendCommand->getPrValue());
            }
            break;

        case PR_RTX:
            datMsg->setRtx((uint32)sendCommand->getPrValue());
            break;

        case PR_PRIO:
            datMsg->setPriority((uint32)sendCommand->getPrValue());
            state->queuedDroppableBytes += msg->getByteLength();
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
        SCTPDataMsg *dropmsg;

        if (sendUnordered)
            strq = stream->getUnorderedStreamQ();
        else
            strq = stream->getStreamQ();

        while (dropsize >= 0 && state->queuedDroppableBytes > 0) {
            lowestPriority = 0;
            dropmsg = NULL;

            // Find lowest priority
            for (cQueue::Iterator iter(*strq); !iter.end(); iter++) {
                SCTPDataMsg *msg = (SCTPDataMsg *)iter();

                if (msg->getPriority() > lowestPriority)
                    lowestPriority = msg->getPriority();
            }

            // If just passed message has the lowest priority,
            // drop it and we're done.
            if (datMsg->getPriority() > lowestPriority) {
                EV_DEBUG << "msg will be abandoned, buffer is full and priority too low ("
                         << datMsg->getPriority() << ")\n";
                state->queuedDroppableBytes -= msg->getByteLength();
                delete smsg;
                delete msg;
                sendIndicationToApp(SCTP_I_ABANDONED);
                return;
            }

            // Find oldest message with lowest priority
            for (cQueue::Iterator iter(*strq); !iter.end(); iter++) {
                SCTPDataMsg *msg = (SCTPDataMsg *)iter();

                if (msg->getPriority() == lowestPriority) {
                    if (!dropmsg ||
                        (dropmsg && dropmsg->getEnqueuingTime() < msg->getEnqueuingTime()))
                        lowestPriority = msg->getPriority();
                }
            }

            if (dropmsg) {
                strq->remove(dropmsg);
                dropsize -= dropmsg->getByteLength();
                state->queuedDroppableBytes -= dropmsg->getByteLength();
                SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>((msg->decapsulate()));
                delete smsg;
                delete dropmsg;
                sendIndicationToApp(SCTP_I_ABANDONED);
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

    qCounter.roomSumSendStreams += ADD_PADDING(smsg->getBitLength() / 8 + SCTP_DATA_CHUNK_LENGTH);
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

        // ------ Send buffer full? -------------------------------------------
        if ((state->appSendAllowed) &&
            (state->sendQueueLimit > 0) &&
            ((uint64)state->sendBuffer >= state->sendQueueLimit))
        {
            // If there are not enough messages that could be dropped,
            // the buffer is really full and the app has to be notified.
            if (state->queuedDroppableBytes < state->sendBuffer - state->sendQueueLimit) {
                sendIndicationToApp(SCTP_I_SENDQUEUE_FULL);
                state->appSendAllowed = false;
            }
        }
    }

    state->queuedMessages++;
    if ((state->queueLimit > 0) && (state->queuedMessages > state->queueLimit)) {
        state->queueUpdate = false;
    }
    EV_DEBUG << "process_SEND:"
             << " last=" << sendCommand->getLast()
             << "    queueLimit=" << state->queueLimit << endl;

    // ------ Call sendCommandInvoked() to send message -------------------
    // sendCommandInvoked() itself will call sendOnAllPaths() ...
    if (sendCommand->getLast() == true) {
        if (sendCommand->getPrimary()) {
            sctpAlgorithm->sendCommandInvoked(NULL);
        }
        else {
            sctpAlgorithm->sendCommandInvoked(getPath(datMsg->getInitialDestination()));
        }
    }
}

void SCTPAssociation::process_RECEIVE_REQUEST(SCTPEventCode& event, SCTPCommand *sctpCommand)
{
    SCTPSendCommand *sendCommand = check_and_cast<SCTPSendCommand *>(sctpCommand);
    if ((uint32)sendCommand->getSid() > inboundStreams || sendCommand->getSid() < 0) {
        EV_DEBUG << "Application tries to read from invalid stream id....\n";
    }
    state->numMsgsReq[sendCommand->getSid()] += sendCommand->getNumMsgs();
    pushUlp();
}

void SCTPAssociation::process_PRIMARY(SCTPEventCode& event, SCTPCommand *sctpCommand)
{
    SCTPPathInfo *pinfo = check_and_cast<SCTPPathInfo *>(sctpCommand);
    state->setPrimaryPath(getPath(pinfo->getRemoteAddress()));
}

void SCTPAssociation::process_STREAM_RESET(SCTPCommand *sctpCommand)
{
    EV_DEBUG << "process_STREAM_RESET request arriving from App\n";
    SCTPResetInfo *rinfo = check_and_cast<SCTPResetInfo *>(sctpCommand);
    if (!(getPath(remoteAddr)->ResetTimer->isScheduled())) {
        sendStreamResetRequest(rinfo->getRequestType());
        if (rinfo->getRequestType() == RESET_OUTGOING || rinfo->getRequestType() == RESET_BOTH || rinfo->getRequestType() == SSN_TSN)
            state->resetPending = true;
    }
}

void SCTPAssociation::process_QUEUE_MSGS_LIMIT(const SCTPCommand *sctpCommand)
{
    const SCTPInfo *qinfo = check_and_cast<const SCTPInfo *>(sctpCommand);
    state->queueLimit = qinfo->getText();
    EV_DEBUG << "state->queueLimit set to " << state->queueLimit << "\n";
}

void SCTPAssociation::process_QUEUE_BYTES_LIMIT(const SCTPCommand *sctpCommand)
{
    const SCTPInfo *qinfo = check_and_cast<const SCTPInfo *>(sctpCommand);
    state->sendQueueLimit = qinfo->getText();
}

void SCTPAssociation::process_CLOSE(SCTPEventCode& event)
{
    EV_DEBUG << "SCTPAssociationEventProc:process_CLOSE; assoc=" << assocId << endl;
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

void SCTPAssociation::process_ABORT(SCTPEventCode& event)
{
    EV_DEBUG << "SCTPAssociationEventProc:process_ABORT; assoc=" << assocId << endl;
    switch (fsm->getState()) {
        case SCTP_S_ESTABLISHED:
            sendOnAllPaths(state->getPrimaryPath());
            sendAbort();
            break;
    }
}

void SCTPAssociation::process_STATUS(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    SCTPStatusInfo *statusInfo = new SCTPStatusInfo();
    statusInfo->setState(fsm->getState());
    statusInfo->setStateName(stateName(fsm->getState()));
    statusInfo->setPathId(remoteAddr);
    statusInfo->setActive(getPath(remoteAddr)->activePath);
    msg->setControlInfo(statusInfo);
    sendToApp(msg);
}

} // namespace sctp

} // namespace inet

