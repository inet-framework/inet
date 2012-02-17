//
// Copyright (C) 2005-2010 by Irene Ruengeler
// Copyright (C) 2009-2010 by Thomas Dreibholz
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
#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPControlInfo_m.h"
#include "SCTPAlgorithm.h"

//
// Event processing code
//

void SCTPAssociation::process_ASSOCIATE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    IPvXAddress lAddr, rAddr;

    SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);

    ev << "SCTPAssociationEventProc:process_ASSOCIATE\n";

    switch (fsm->getState())
    {
        case SCTP_S_CLOSED:
            initAssociation(openCmd);
            state->active = true;
            localAddressList = openCmd->getLocalAddresses();
            lAddr = openCmd->getLocalAddresses().front();
            if (!(openCmd->getRemoteAddresses().empty()))
            {
                remoteAddressList = openCmd->getRemoteAddresses();
                rAddr = openCmd->getRemoteAddresses().front();
            }
            else
                rAddr = openCmd->getRemoteAddr();
            localPort = openCmd->getLocalPort();
            remotePort = openCmd->getRemotePort();
            state->numRequests = openCmd->getNumRequests();
            if (rAddr.isUnspecified() || remotePort==0)
                opp_error("Error processing command OPEN_ACTIVE: remote address and port must be specified");

            if (localPort==0)
            {
                localPort = sctpMain->getEphemeralPort();
            }
            ev << "OPEN: " << lAddr << ":" << localPort << " --> " << rAddr << ":" << remotePort << "\n";

            sctpMain->updateSockPair(this, lAddr, rAddr, localPort, remotePort);
            state->localRwnd = (long)sctpMain->par("arwnd");
            sendInit();
            startTimer(T1_InitTimer, state->initRexmitTimeout);
            break;

        default:
            opp_error("Error processing command OPEN_ACTIVE: connection already exists");
    }

}

void SCTPAssociation::process_OPEN_PASSIVE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
    IPvXAddress lAddr;
    int16 localPort;

    SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);

    sctpEV3 << "SCTPAssociationEventProc:process_OPEN_PASSIVE\n";

    switch (fsm->getState())
    {
        case SCTP_S_CLOSED:
            initAssociation(openCmd);
            state->fork = openCmd->getFork();
            localAddressList = openCmd->getLocalAddresses();
            sctpEV3 << "process_OPEN_PASSIVE: number of local addresses=" << localAddressList.size() << "\n";
            lAddr = openCmd->getLocalAddresses().front();
            localPort = openCmd->getLocalPort();
            inboundStreams = openCmd->getInboundStreams();
            outboundStreams = openCmd->getOutboundStreams();
            state->localRwnd = (long)sctpMain->par("arwnd");
            state->numRequests = openCmd->getNumRequests();
            state->messagesToPush = openCmd->getMessagesToPush();

            if (localPort==0)
                opp_error("Error processing command OPEN_PASSIVE: local port must be specified");
            sctpEV3 << "Assoc " << assocId << "::Starting to listen on: " << lAddr << ":" << localPort << "\n";

            sctpMain->updateSockPair(this, lAddr, IPvXAddress(), localPort, 0);
            break;
        default:
            opp_error("Error processing command OPEN_PASSIVE: connection already exists");
    }
}

void SCTPAssociation::process_SEND(SCTPEventCode& event, SCTPCommand* sctpCommand, cPacket* msg)
{
    SCTPSendCommand* sendCommand = check_and_cast<SCTPSendCommand*>(sctpCommand);

    if (fsm->getState() != SCTP_S_ESTABLISHED) {
        // TD 12.03.2009: since SCTP_S_ESTABLISHED is the only case, the
        // switch(...)-block has been removed for enhanced readability.
        sctpEV3 << "process_SEND: state is not SCTP_S_ESTABLISHED -> returning" << endl;
        return;
    }

    sctpEV3 << "process_SEND:"
            << " assocId=" << assocId
            << " localAddr=" << localAddr
            << " remoteAddr=" << remoteAddr
            << " cmdRemoteAddr=" << sendCommand->getRemoteAddr()
            << " cmdPrimary=" << (sendCommand->getPrimary() ? "true" : "false")
            << " appGateIndex=" << appGateIndex
            << " streamId=" << sendCommand->getSid() << endl;

    SCTPSimpleMessage* smsg = check_and_cast<SCTPSimpleMessage*>((msg->decapsulate()));
    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
    iter->second.sentBytes += smsg->getBitLength() / 8;

    // ------ Prepare SCTPDataMsg -----------------------------------------
    const uint32 streamId = sendCommand->getSid();
    const uint32 sendUnordered = sendCommand->getSendUnordered();
    const uint32 ppid = sendCommand->getPpid();
    SCTPSendStream* stream = NULL;
    SCTPSendStreamMap::iterator associter = sendStreams.find(streamId);
    if (associter != sendStreams.end()) {
        stream = associter->second;
    }
    else {
        opp_error("stream with id %d not found", streamId);
    }

    char name[64];
    snprintf(name, sizeof(name), "SDATA-%d-%d", streamId, state->msgNum);
    smsg->setName(name);

    SCTPDataMsg* datMsg = new SCTPDataMsg();
    datMsg->encapsulate(smsg);
    datMsg->setSid(streamId);
    datMsg->setPpid(ppid);
    datMsg->setEnqueuingTime(simulation.getSimTime());

    // ------ Set initial destination address -----------------------------
    if (sendCommand->getPrimary()) {
        if (sendCommand->getRemoteAddr() == IPvXAddress("0.0.0.0")) {
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
    datMsg->setBooksize(smsg->getBitLength() / 8 + state->header);
    qCounter.roomSumSendStreams += ADD_PADDING(smsg->getBitLength() / 8 + SCTP_DATA_CHUNK_LENGTH);
    qCounter.bookedSumSendStreams += datMsg->getBooksize();
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

        if ((state->appSendAllowed) &&
                (state->sendQueueLimit > 0) &&
                ((uint64)state->sendBuffer >= state->sendQueueLimit) ) {
            sendIndicationToApp(SCTP_I_SENDQUEUE_FULL);
            state->appSendAllowed = false;
        }
        sendQueue->record(stream->getStreamQ()->getLength());
    }

    state->queuedMessages++;
    if ((state->queueLimit > 0) && (state->queuedMessages > state->queueLimit)) {
        state->queueUpdate = false;
    }
    sctpEV3 << "process_SEND:"
            << " last=" << sendCommand->getLast()
            <<"    queueLimit=" << state->queueLimit << endl;

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
    if ((uint32)sendCommand->getSid() > inboundStreams || sendCommand->getSid() < 0)
    {
        sctpEV3 << "Application tries to read from invalid stream id....\n";
    }
    state->numMsgsReq[sendCommand->getSid()] += sendCommand->getNumMsgs();
    pushUlp();
}

void SCTPAssociation::process_PRIMARY(SCTPEventCode& event, SCTPCommand *sctpCommand)
{
    SCTPPathInfo *pinfo = check_and_cast<SCTPPathInfo *>(sctpCommand);
    state->setPrimaryPath(getPath(pinfo->getRemoteAddress()));
}


void SCTPAssociation::process_QUEUE_MSGS_LIMIT(const SCTPCommand* sctpCommand)
{
    const SCTPInfo* qinfo = check_and_cast<const SCTPInfo*>(sctpCommand);
    state->queueLimit = qinfo->getText();
    sctpEV3 << "state->queueLimit set to " << state->queueLimit << "\n";
}

void SCTPAssociation::process_QUEUE_BYTES_LIMIT(const SCTPCommand* sctpCommand)
{
    const SCTPInfo* qinfo = check_and_cast<const SCTPInfo*>(sctpCommand);
    state->sendQueueLimit = qinfo->getText();
}

void SCTPAssociation::process_CLOSE(SCTPEventCode& event)
{
    sctpEV3 << "SCTPAssociationEventProc:process_CLOSE; assoc=" << assocId << endl;
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
    sctpEV3 << "SCTPAssociationEventProc:process_ABORT; assoc=" << assocId << endl;
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
