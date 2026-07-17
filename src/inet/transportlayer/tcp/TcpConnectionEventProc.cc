//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <climits>
#include <string.h>

#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/contract/tcp/TcpTimestampingTag_m.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/flavours/TcpBaseAlgState_m.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamilyState_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

//
// Event processing code
//

void TcpConnection::process_OPEN_ACTIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    TcpOpenCommand *openCmd = check_and_cast<TcpOpenCommand *>(tcpCommand);
    L3Address localAddr, remoteAddr;
    int localPort, remotePort;

    switch (fsm.getState()) {
        case TCP_S_INIT:
            initConnection(openCmd);

            // store local/remote socket
            state->active = true;
            localAddr = openCmd->getLocalAddr();
            remoteAddr = openCmd->getRemoteAddr();
            localPort = openCmd->getLocalPort();
            remotePort = openCmd->getRemotePort();
            autoRead = openCmd->getAutoRead();

            if (remoteAddr.isUnspecified() || remotePort == -1)
                throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: remote address and port must be specified");

            if (localPort == -1) {
                localPort = tcpMain->getEphemeralPort();
                EV_DETAIL << "Assigned ephemeral port " << localPort << "\n";
            }

            EV_DETAIL << "OPEN: " << localAddr << ":" << localPort << " --> " << remoteAddr << ":" << remotePort << "\n";

            tcpMain->addSockPair(this, localAddr, remoteAddr, localPort, remotePort);

            // TCP Fast Open (RFC 7413): if the app asked for it and a cookie is
            // already cached for this destination, defer the SYN until the app's
            // first SEND arrives (process_SEND fills in the data and calls
            // sendSyn()) instead of sending a bare SYN now. If no cookie is
            // cached, send an immediate (dataless) SYN that just requests one --
            // FSM_Goto(TCP_S_SYN_SENT) below doesn't depend on sendSyn() having
            // actually been called, so deferring here is FSM-transition-transparent.
            if (openCmd->getFastOpen() && state->fastopenClientEnabled) {
                state->fastopenRequested = true;
                std::vector<uint8_t> cachedCookie;
                // isActiveFastOpenDisabled(): active blackhole detection tripped --
                // treat exactly like "no cookie cached" (still request one via a
                // bare, dataless SYN), so the connection proceeds normally, just
                // without the data-attached acceleration until the timeout elapses.
                if (!tcpMain->isActiveFastOpenDisabled() && tcpMain->getFastOpenCookie(remoteAddr, cachedCookie)) {
                    selectInitialSeqNum();
                    state->fastopenSynDeferred = true;
                    scheduleAfter(TCP_TIMEOUT_CONN_ESTAB, connEstabTimer);
                    delete openCmd;
                    delete msg;
                    return;
                }
                state->fastopenCookieRequestPending = true;
            }

            // send initial SYN
            selectInitialSeqNum();
            sendSyn();
            startSynRexmitTimer();
            scheduleAfter(TCP_TIMEOUT_CONN_ESTAB, connEstabTimer);
            break;

        default:
            throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: connection already exists");
    }

    delete openCmd;
    delete msg;
}

void TcpConnection::process_OPEN_PASSIVE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    TcpOpenCommand *openCmd = check_and_cast<TcpOpenCommand *>(tcpCommand);
    L3Address localAddr;
    int localPort;

    switch (fsm.getState()) {
        case TCP_S_INIT:
            initConnection(openCmd);

            // store local/remote socket
            state->active = false;
            state->fork = openCmd->getFork();
            autoRead = openCmd->getAutoRead();
            localAddr = openCmd->getLocalAddr();
            localPort = openCmd->getLocalPort();

            if (localPort == -1)
                throw cRuntimeError(tcpMain, "Error processing command OPEN_PASSIVE: local port must be specified");

            EV_DETAIL << "Starting to listen on: " << localAddr << ":" << localPort << "\n";

            tcpMain->addSockPair(this, localAddr, L3Address(), localPort, -1);
            break;

        default:
            throw cRuntimeError(tcpMain, "Error processing command OPEN_PASSIVE: connection already exists");
    }

    delete openCmd;
    delete msg;
}

void TcpConnection::process_ACCEPT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    TcpAcceptCommand *acceptCommand = check_and_cast<TcpAcceptCommand *>(tcpCommand);
    listeningSocketId = -1;
    sendEstabIndicationToApp();
    sendAvailableDataToApp();
    delete acceptCommand;
    delete msg;
}

void TcpConnection::process_SEND(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    // PSH-at-record-boundary is opt-in per SEND via the packet's TcpSendEorReq tag
    // (Workstream H1, MSG_EOR) rather than automatic on every SEND -- see
    // enqueueSendCommandData() and sendSegment()'s PSH-bit logic.
    Packet *packet = check_and_cast<Packet *>(msg);
    switch (fsm.getState()) {
        case TCP_S_INIT:
            throw cRuntimeError(tcpMain, "Error processing command SEND: connection not open");

        case TCP_S_LISTEN:
            EV_DETAIL << "SEND command turns passive open into active open, sending initial SYN\n";
            state->active = true;
            selectInitialSeqNum();
            sendSyn();
            startSynRexmitTimer();
            scheduleAfter(TCP_TIMEOUT_CONN_ESTAB, connEstabTimer);
            enqueueSendCommandData(packet); // queue up for later
            EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            break;

        case TCP_S_SYN_RCVD:
            enqueueSendCommandData(packet);
            if (state->fastopenSynDataAccepted) {
                // TCP Fast Open server (RFC 7413 section 4.2): the connection was
                // created from a SYN whose data was accepted, so the app already
                // read that data and may respond BEFORE the handshake-completing
                // ACK arrives -- the response transmits from SYN_RCVD (this is
                // TFO's data-exchange-during-handshake acceleration; a regular
                // SYN_RCVD connection keeps queueing until ESTABLISHED).
                EV_DETAIL << "Fast Open: sending response data during SYN_RCVD\n";
                tcpAlgorithm->sendCommandInvoked();
            }
            else {
                EV_DETAIL << "Queueing up data for sending later.\n";
                EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            }
            break;

        case TCP_S_SYN_SENT:
            if (state->fastopenSynDeferred) {
                // TCP Fast Open (RFC 7413): this is the SEND process_OPEN_ACTIVE
                // deferred the SYN for. Attach as much of it as fits in one
                // segment and send the data-bearing SYN now.
                enqueueSendCommandData(packet);
                uint32_t availableBytes = sendQueue->getBytesAvailable(state->iss + 1);
                uint32_t capBytes = state->snd_mss > 0 ? state->snd_mss : 536;
                state->fastopenSynDataLen = availableBytes < capBytes ? availableBytes : capBytes;
                // fastopenSynDeferred stays true through sendSyn() itself: it doubles
                // as writeHeaderOptions()'s signal that this is a first-ever SYN being
                // sent from SYN_SENT (not the usual TCP_S_INIT), so the SYN gets its
                // full option set despite syn_rexmit_count still being 0.
                sendSyn();
                state->fastopenSynDeferred = false;
                startSynRexmitTimer();
                // connEstabTimer was already scheduled from process_OPEN_ACTIVE.
                break;
            }
            EV_DETAIL << "Queueing up data for sending later.\n";
            enqueueSendCommandData(packet); // queue up for later
            EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            break;

        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
            enqueueSendCommandData(packet);
            EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue, plus "
                      << (state->snd_max - state->snd_una) << " bytes unacknowledged\n";
            tcpAlgorithm->sendCommandInvoked();
            break;

        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            throw cRuntimeError(tcpMain, "Error processing command SEND: connection closing");
    }

    if ((state->sendQueueLimit > 0) && (sendQueue->getBytesAvailable(state->snd_una) > state->sendQueueLimit))
        state->queueUpdate = false;

    // TCP_NOTSENT_LOWAT (Workstream H4): arm re-notification once the not-yet-
    // transmitted portion of the queue (from snd_nxt, not snd_una -- independent of
    // sendQueueLimit above) exceeds the low-water mark; sendSegment() disarms it and
    // signals the app again once transmission brings it back down to/below the mark.
    if (state->notsentLowat != (uint32_t)-1 && sendQueue->getBytesAvailable(state->snd_nxt) > state->notsentLowat)
        state->notsentLowatUpdate = false;
}

void TcpConnection::process_READ_REQUEST(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    if (autoRead)
        throw cRuntimeError("TCP READ arrived, but connection used in autoRead mode");
    //check whether we have data in the TCP queue. Store how much data the application wants. Check for pending read request.
    TcpReadCommand *readCmd = check_and_cast<TcpReadCommand *>(tcpCommand);
    if (readCmd->getMaxByteCount() <= 0)
        throw cRuntimeError("Illegal argument: numberOfBytes in TCP READ command is negative or zero.");
    if (maxByteCountRequested != 0)
        throw cRuntimeError("A second TCP READ command arrived before data for the previous READ was sent up");
    maxByteCountRequested = readCmd->getMaxByteCount();
    if (isToBeAccepted())
        throw cRuntimeError("READ without ACCEPT");

    if (receiveQueue->getQueueLength() > 0) {
        uint32_t endSeqNo = state->rcv_nxt;
        uint32_t requestedEndPos = receiveQueue->getFirstSeqNo() + maxByteCountRequested;
        if (seqLess(requestedEndPos, endSeqNo))
            endSeqNo = requestedEndPos;
        if (Packet *dataMsg = receiveQueue->extractBytesUpTo(endSeqNo)) {
            dataMsg->setKind(TCP_I_DATA);
            dataMsg->addTag<SocketInd>()->setSocketId(socketId);
            if (rxTimestampingEnabled)
                dataMsg->addTag<TcpRxTimestampInd>();
            sendToApp(dataMsg);
            maxByteCountRequested = 0;
        }
    }
    if (!peerClosedSentUp && fsm.getState() == TCP_S_CLOSE_WAIT && this->receiveQueue->getQueueLength() == 0) {
        sendIndicationToApp(TCP_I_PEER_CLOSED);
        peerClosedSentUp = true;
    }
    delete msg;
}

void TcpConnection::process_OPTIONS(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    ASSERT(event == TCP_E_SETOPTION);

    if (auto cmd = dynamic_cast<TcpSetTimeToLiveCommand *>(tcpCommand))
        ttl = cmd->getTtl();
    else if (auto cmd = dynamic_cast<TcpSetTosCommand *>(tcpCommand)) {
        tos = cmd->getTos();
    }
    else if (auto cmd = dynamic_cast<TcpSetDscpCommand *>(tcpCommand)) {
        dscp = cmd->getDscp();
    }
    else if (auto cmd = dynamic_cast<TcpSetTimestampingCommand *>(tcpCommand)) {
        rxTimestampingEnabled = cmd->getEnabled();
    }
    else if (auto cmd = dynamic_cast<TcpSetNotsentLowatCommand *>(tcpCommand)) {
        // Runtime TCP_NOTSENT_LOWAT: same field the notsentLowat module param
        // seeds at connection setup (configureStateVariables); -1 disables.
        // May legally arrive before OPEN creates state (like setTimestamping
        // above) -- keep the value on the connection and apply it now only if
        // state already exists; configureStateVariables() applies it otherwise.
        notsentLowatSockopt = cmd->getValue();
        if (state != nullptr)
            state->notsentLowat = (notsentLowatSockopt < 0) ? (uint32_t)-1 : (uint32_t)notsentLowatSockopt;
    }
    else
        throw cRuntimeError("Unknown subclass of TcpSetOptionCommand received from app: %s", tcpCommand->getClassName());
    delete tcpCommand;
    delete msg;
}

void TcpConnection::process_CLOSE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand;
    delete msg;

    switch (fsm.getState()) {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
            // Nothing to do here
            break;

        case TCP_S_SYN_SENT:
            // Delete the TCB and return "error:  closing" responses to any
            // queued SENDs, or RECEIVEs.
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
            //
            // SYN_RCVD processing (ESTABLISHED and CLOSE_WAIT are similar):
            //"
            // If no SENDs have been issued and there is no pending data to send,
            // then form a FIN segment and send it, and enter FIN-WAIT-1 state;
            // otherwise queue for processing after entering ESTABLISHED state.
            //"
            if (state->snd_max == sendQueue->getBufferEndSeq()) {
                EV_DETAIL << "No outstanding SENDs, sending FIN right away, advancing snd_nxt over the FIN\n";
                state->snd_nxt = state->snd_max;
                sendFin();
                tcpAlgorithm->restartRexmitTimer();
                state->snd_max = ++state->snd_nxt;

                emit(unackedSignal, state->snd_max - state->snd_una);

                // state transition will automatically take us to FIN_WAIT_1 (or LAST_ACK)
            }
            else {
                EV_DETAIL << "SEND of " << (sendQueue->getBufferEndSeq() - state->snd_max)
                          << " bytes pending, deferring sending of FIN\n";
                event = TCP_E_IGNORE;
            }
            state->send_fin = true;
            state->snd_fin_seq = sendQueue->getBufferEndSeq();
            break;

        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_LAST_ACK:
        case TCP_S_TIME_WAIT:
            // RFC 793 is not entirely clear on how to handle a duplicate close request.
            // Here we treat it as an error.
            throw cRuntimeError(tcpMain, "Duplicate CLOSE command: connection already closing");
    }
}

void TcpConnection::process_ABORT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand;
    delete msg;

    //
    // The ABORT event will automatically take the connection to the CLOSED
    // state, flush queues etc -- no need to do it here. Also, we don't need to
    // send notification to the user, they know what's going on.
    //
    switch (fsm.getState()) {
        case TCP_S_INIT:
            throw cRuntimeError("Error processing command ABORT: connection not open");

        case TCP_S_SYN_RCVD:
        case TCP_S_ESTABLISHED:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSE_WAIT:
            //"
            // Send a reset segment:
            //
            //   <SEQ=SND.NXT><CTL=RST>
            //"
            sendRst(state->snd_nxt);
            break;
    }
}

void TcpConnection::process_DESTROY(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand;
    delete msg;
    // TODO should we send a RST or not?
}

void TcpConnection::process_STATUS(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand; // but reuse msg for reply

    if (fsm.getState() == TCP_S_INIT)
        throw cRuntimeError("Error processing command STATUS: connection not open");

    TcpStatusInfo *statusInfo = new TcpStatusInfo();

    statusInfo->setState(fsm.getState());
    statusInfo->setStateName(stateName(fsm.getState()));

    statusInfo->setLocalAddr(localAddr);
    statusInfo->setRemoteAddr(remoteAddr);
    statusInfo->setLocalPort(localPort);
    statusInfo->setRemotePort(remotePort);
    statusInfo->setAutoRead(autoRead);

    statusInfo->setSnd_mss(state->snd_mss);
    statusInfo->setSnd_una(state->snd_una);
    statusInfo->setSnd_nxt(state->snd_nxt);
    statusInfo->setSnd_max(state->snd_max);
    statusInfo->setSnd_wnd(state->snd_wnd);
    statusInfo->setSnd_up(state->snd_up);
    statusInfo->setSnd_wl1(state->snd_wl1);
    statusInfo->setSnd_wl2(state->snd_wl2);
    statusInfo->setIss(state->iss);
    statusInfo->setRcv_nxt(state->rcv_nxt);
    statusInfo->setRcv_wnd(state->rcv_wnd);
    statusInfo->setRcv_up(state->rcv_up);
    statusInfo->setIrs(state->irs);
    statusInfo->setFin_ack_rcvd(state->fin_ack_rcvd);

    statusInfo->setReordering(state->reordering);
    statusInfo->setMinRtt(state->minRtt.dbl());
    statusInfo->setFlightSize(getFlightSize());
    statusInfo->setSackedBytes(state->sackedBytes);
    statusInfo->setDeliveredBytes(state->deliveredBytes);
    statusInfo->setTsEnabled(state->ts_enabled);
    statusInfo->setSackEnabled(state->sack_enabled);
    statusInfo->setWsEnabled(state->ws_enabled);
    statusInfo->setEctEnabled(state->ect);
    statusInfo->setSynDataAccepted(state->fastopenSynDataAccepted);
    statusInfo->setSndWndScale(state->snd_wnd_scale);
    statusInfo->setLastDataRecvTime(state->time_last_segment_received);

    // Congestion-window/RTO/RTT fields live on flavour-specific state variable
    // subclasses, one or two levels below the base TcpStateVariables* held as
    // `state` -- not every flavour (e.g. DumbTcp) has them, so guard with a
    // dynamic_cast and fall back to the UINT_MAX sentinel documented on
    // TcpStatusInfo.
    if (auto *baseAlgState = dynamic_cast<TcpBaseAlgStateVariables *>(state)) {
        statusInfo->setCwnd(baseAlgState->snd_cwnd);
        statusInfo->setSrtt(baseAlgState->srtt.dbl());
        statusInfo->setRexmitCount(baseAlgState->rexmit_count);
        statusInfo->setNumRtos(baseAlgState->numRtos);
    }
    else {
        statusInfo->setCwnd(UINT_MAX);
        statusInfo->setSrtt(-1);
        statusInfo->setRexmitCount(UINT_MAX);
        statusInfo->setNumRtos(UINT_MAX);
    }

    if (auto *tahoeRenoState = dynamic_cast<TcpTahoeRenoFamilyStateVariables *>(state))
        statusInfo->setSsthresh(tahoeRenoState->ssthresh);
    else
        statusInfo->setSsthresh(UINT_MAX);

    statusInfo->setCaState(deriveLinuxCaState());
    // rcv_nxt/irs are only meaningful once the 3WHS has fixed irs (peer's ISN); before
    // that (e.g. a STATUS query in SYN_SENT) both are still 0 and the subtraction
    // would underflow.
    statusInfo->setBytesReceived(seqGreater(state->rcv_nxt, state->irs) ? state->rcv_nxt - state->irs - 1 : 0);
    statusInfo->setDeliveredCePkts(state->deliveredCePkts);
    statusInfo->setDeliveredCeBytes(state->deliveredCeBytes);

    // TCP_INFO trio: report the accumulated total plus, if a period is still open
    // right now, the elapsed time since it started -- so a live query reflects the
    // up-to-the-moment total rather than only the last fully-closed period.
    statusInfo->setBusyTime((state->busyTimeAccumulated
        + (state->busyStartTime >= SIMTIME_ZERO ? simTime() - state->busyStartTime : SIMTIME_ZERO)).dbl());
    statusInfo->setRwndLimited((state->rwndLimitedAccumulated
        + (state->rwndLimitedStartTime >= SIMTIME_ZERO ? simTime() - state->rwndLimitedStartTime : SIMTIME_ZERO)).dbl());

    if (state->sack_enabled && rexmitQueue != nullptr && state->snd_mss > 0) {
        statusInfo->setLost(rexmitQueue->getTotalAmountOfLostBytes() / state->snd_mss);
        statusInfo->setRetrans(rexmitQueue->getTotalAmountOfRexmittedUnsackedBytes() / state->snd_mss);
    }
    else {
        statusInfo->setLost(UINT_MAX);
        statusInfo->setRetrans(UINT_MAX);
    }

    if (auto *baseAlgState = dynamic_cast<TcpBaseAlgStateVariables *>(state)) {
        statusInfo->setBackoff(baseAlgState->rexmit_count);
        statusInfo->setProbes(baseAlgState->zeroWindowProbesSent);
    }
    else {
        statusInfo->setBackoff(UINT_MAX);
        statusInfo->setProbes(UINT_MAX);
    }

    msg->setControlInfo(statusInfo);
    msg->setKind(TCP_I_STATUS);
    // Every other reply-sending path in this file tags its outgoing message
    // with SocketInd (see sendIndicationToApp() and friends in
    // TcpConnectionUtil.cc) so TcpSocket::belongsToSocket() can match it back
    // to the requesting app-side socket. This path reuses the incoming
    // request message, which only ever carried a SocketReq tag -- without
    // this, the STATUS reply is silently dropped by the app's socket
    // dispatch instead of reaching TcpSocket::ICallback::socketStatusArrived().
    check_and_cast<Message *>(msg)->addTag<SocketInd>()->setSocketId(socketId);
    sendToApp(msg);
}

void TcpConnection::process_QUEUE_BYTES_LIMIT(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    if (state == nullptr)
        throw cRuntimeError("Called process_QUEUE_BYTES_LIMIT on uninitialized TcpConnection!");

    state->sendQueueLimit = tcpCommand->getUserId(); // Set queue size limit
    EV << "state->sendQueueLimit set to " << state->sendQueueLimit << "\n";
    delete msg;
    delete tcpCommand;
}

} // namespace tcp
} // namespace inet

