//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <climits>
#include <string.h>

#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/contract/tcp/TcpTimestampingTag_m.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBaseState_m.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

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
                // Cookie-less client mode (Linux tcp_fastopen bit 0x4,
                // TFO_CLIENT_NO_COOKIE): defer and attach data exactly like a
                // cache hit, but with no cookie to put in the SYN -- the FO
                // option is absent entirely (writeHeaderOptions has neither a
                // cached cookie nor a pending request to emit).
                if (!tcpMain->isActiveFastOpenDisabled()
                    && (tcpMain->getFastOpenCookie(remoteAddr, cachedCookie)
                        || tcpMain->par("fastopenClientNoCookieRequired").boolValue())) {
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
    // (MSG_EOR) rather than automatic on every SEND -- see
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
            if (state->fastopenAccelerated) {
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
                // A DATALESS SEND is legal here: sendto(..., 0, MSG_FASTOPEN)
                // with a cached cookie still releases the deferred SYN (Linux
                // sends the bare cookie-bearing SYN inside that syscall) --
                // nothing to queue, availableBytes stays 0 below.
                if (packet->getByteLength() > 0)
                    enqueueSendCommandData(packet);
                else
                    delete packet;
                uint32_t availableBytes = sendQueue->getBytesAvailable(state->iss + 1);
                // SYN-payload cap: the peer's MSS cached with the cookie minus
                // the maximum TCP option space (40) -- Linux sizes the SYN data
                // from the tcp_metrics-cached MSS, since nothing has been
                // negotiated yet on this connection (the corpus's over-mss test
                // pins 1420 = 1460-40, and its third-connection test pins
                // 900 = a cached 940 - 40).
                uint32_t capBytes = state->snd_mss > 0 ? state->snd_mss : 536;
                uint32_t cachedMss = tcpMain->getFastOpenCachedMss(remoteAddr);
                if (cachedMss > 40)
                    capBytes = cachedMss - 40;
                else if (capBytes > 40)
                    capBytes -= 40;
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

    // TCP_NOTSENT_LOWAT: arm re-notification once the not-yet-
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
    else if (auto cmd = dynamic_cast<TcpSetMaxSegCommand *>(tcpCommand)) {
        // Runtime TCP_MAXSEG: clamp the advertised and effective send MSS. Like
        // TCP_NOTSENT_LOWAT above it may arrive before OPEN creates state; keep it
        // on the connection and let configureStateVariables() apply it, or apply
        // now if state already exists (a mid-connection clamp).
        userMss = cmd->getValue();
        if (state != nullptr && userMss > 0) {
            state->advertisedMss = userMss;
            if (state->snd_mss == (uint32_t)-1 || (uint32_t)userMss < state->snd_mss)
                state->snd_mss = userMss;
            state->snd_effmss = calculateEffectiveMss();
        }
    }
    else if (auto cmd = dynamic_cast<TcpSetPathMtuCommand *>(tcpCommand)) {
        // A route change under an open connection. Linux notices it in
        // tcp_current_mss (dst_mtu != icsk_pmtu_cookie) -- but with MTU probing
        // armed the MSS still cannot exceed what the search has proven, so the new
        // ceiling only takes effect through a successful probe.
        pathMtuSockopt = cmd->getValue();
        if (state != nullptr && pathMtuSockopt > 0) {
            state->pathMtu = (uint32_t)pathMtuSockopt;
            if (state->mtupEnabled && state->mtupSearchHigh < state->pathMtu)
                state->mtupSearchHigh = state->pathMtu;
            EV_DETAIL << "Path MTU is now " << state->pathMtu << "\n";
        }
    }
    else if (auto cmd = dynamic_cast<TcpSetWriterBlockedCommand *>(tcpCommand)) {
        // The application's blocking write is (no longer) stalled on
        // send-buffer space -- drives the SNDBUF_LIMITED chrono
        // (tcp-info-sndbuf-limited).
        writerBlocked = cmd->getBlocked();
        EV_DETAIL << "Application writer is " << (writerBlocked ? "blocked on send-buffer space" : "no longer blocked") << "\n";
        updateSndbufLimitedChrono();
    }
    else if (auto cmd = dynamic_cast<TcpSetOwnedCommand *>(tcpCommand)) {
        // Application-ownership marker (Linux sk->sk_socket): gates the
        // kernel behaviors that skip embryonic (not-yet-accepted) sockets,
        // e.g. OOO-pressure rcvbuf growth (ooo-before-and-after-accept).
        appOwned = cmd->getOwned();
        EV_DETAIL << "Connection ownership set to " << (appOwned ? "owned" : "embryonic") << "\n";
    }
    else if (auto cmd = dynamic_cast<TcpSetNoDelayCommand *>(tcpCommand)) {
        // Runtime TCP_NODELAY: nagle_enabled is the runtime Nagle switch (nodelay
        // disables Nagle). Enabling nodelay force-flushes any withheld partial
        // (Linux __tcp_push_pending_frames on the nagle-off transition) but does NOT
        // clear tcp_cork -- CORK outranks NODELAY for future small writes. May arrive
        // before OPEN creates state; stash and let configureStateVariables() apply it.
        nodelaySockopt = cmd->getNodelay() ? 1 : 0;
        if (state != nullptr) {
            state->nagle_enabled = !cmd->getNodelay();
            if (cmd->getNodelay())
                flushCorkedData(false);
        }
    }
    else if (auto cmd = dynamic_cast<TcpSetCorkCommand *>(tcpCommand)) {
        // Runtime TCP_CORK: persistently hold the trailing sub-MSS partial. A
        // true->false transition (uncork) force-flushes the withheld partial
        // (Linux tcp_uncork tail). May arrive before OPEN creates state.
        corkSockopt = cmd->getCork() ? 1 : 0;
        if (state != nullptr) {
            bool wasCorked = state->tcp_cork;
            state->tcp_cork = cmd->getCork();
            if (wasCorked && !cmd->getCork())
                flushCorkedData(false);
        }
    }
    else
        throw cRuntimeError("Unknown subclass of TcpSetOptionCommand received from app: %s", tcpCommand->getClassName());
    delete tcpCommand;
    delete msg;
}

void TcpConnection::process_CLOSE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    // full close vs shutdown(SHUT_WR): a full close also shuts the receive side
    // down (see rcvShutdown's comment); a half close keeps reading possible.
    // state is still null for a CLOSE that reaches a freshly created (INIT)
    // connection -- e.g. an app closing an fd whose connect attempt failed.
    if (state != nullptr && (tcpCommand == nullptr || !tcpCommand->getHalfClose()))
        state->rcvShutdown = true;
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
                state->snd_max = ++state->snd_nxt;
                emit(sndMaxSignal, state->snd_max);
                // The FIN is new data on the wire: arm the retransmit timer if
                // it is not already running for outstanding data, and (re)arm
                // the loss probe for the new tail of the flight -- Linux
                // tcp_event_new_data_sent + tcp_schedule_loss_probe; a plain
                // rexmit-timer restart would disarm a pending probe (shared
                // timer slot) and the lost FIN would wait out the full RTO.
                // Must run AFTER snd_max advances over the FIN: schedulePto()
                // refuses to arm while snd_una == snd_max, which silently
                // skipped the probe for a FIN-only close (nothing else in
                // flight -- user_timeout pins TLP-then-backoff FIN rexmits).
                tcpAlgorithm->dataSent(state->snd_max - 1);

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
            // RFC 9293 is not entirely clear on how to handle a duplicate close request.
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
            // Send a reset segment. RFC 793 shows a bare <SEQ=SND.NXT><CTL=RST>,
            // but Linux's active reset (tcp_send_active_reset, also the
            // tcp_disconnect/AF_UNSPEC path) always sends RST|ACK with ack=rcv_nxt.
            sendRstAck(state->snd_nxt, state->rcv_nxt, localAddr, remoteAddr, localPort, remotePort);
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

    if (fsm.getState() == TCP_S_INIT) {
        // Linux parity: getsockopt(TCP_INFO) works on ANY socket fd, including
        // one whose connection attempt was refused/reset/timed out and whose
        // PCB is already gone -- the fd simply reports TCP_CLOSE. An app
        // STATUS landing here means Tcp created this fresh connection for the
        // command because the original was torn down: report a closed socket
        // (default/zeroed fields are the honest values) instead of crashing.
        TcpStatusInfo *closedInfo = new TcpStatusInfo();
        closedInfo->setState(TCP_S_CLOSED);
        closedInfo->setStateName(stateName(TCP_S_CLOSED));
        closedInfo->setLocalAddr(localAddr);
        closedInfo->setRemoteAddr(remoteAddr);
        closedInfo->setLocalPort(localPort);
        closedInfo->setRemotePort(remotePort);
        closedInfo->setCwnd(UINT_MAX);
        closedInfo->setSrtt(-1);
        closedInfo->setRexmitCount(UINT_MAX);
        closedInfo->setNumRtos(UINT_MAX);
        closedInfo->setSsthresh(UINT_MAX);
        closedInfo->setLost(UINT_MAX);
        closedInfo->setRetrans(UINT_MAX);
        closedInfo->setBackoff(UINT_MAX);
        closedInfo->setProbes(UINT_MAX);
        msg->setControlInfo(closedInfo);
        msg->setKind(TCP_I_STATUS);
        check_and_cast<Message *>(msg)->addTag<SocketInd>()->setSocketId(socketId);
        sendToApp(msg);
        return;
    }

    TcpStatusInfo *statusInfo = new TcpStatusInfo();

    statusInfo->setState(fsm.getState());
    statusInfo->setStateName(stateName(fsm.getState()));

    statusInfo->setLocalAddr(localAddr);
    statusInfo->setRemoteAddr(remoteAddr);
    statusInfo->setLocalPort(localPort);
    statusInfo->setRemotePort(remotePort);
    statusInfo->setAutoRead(autoRead);

    statusInfo->setSnd_mss(state->snd_mss);
    statusInfo->setSndEffMss(state->snd_effmss);
    statusInfo->setAdvmss(state->advertisedMss);
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

    // Adaptive reordering (RFC 4653-style dynamic DupThresh): state->reordering
    // grows past the static dupthresh as checkSackReordering() observes SACKs
    // arriving below the FACK (Linux tp->reordering). Report the live degree, not
    // the static dupthresh -- tcpi_reordering must track the adaptive value.
    statusInfo->setReordering(state->reordering);
    statusInfo->setMinRtt(state->minRtt.dbl());
    statusInfo->setFlightSize(tcpAlgorithm->getBytesInFlight());
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
    if (auto *baseAlgState = dynamic_cast<TcpAlgorithmBaseStateVariables *>(state)) {
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

    if (auto *tahoeRenoState = dynamic_cast<TcpClassicAlgorithmBaseStateVariables *>(state))
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
    statusInfo->setDeliveredE0Bytes(state->deliveredE0Bytes);
    statusInfo->setDeliveredE1Bytes(state->deliveredE1Bytes);

    // TCP_INFO trio: report the accumulated total plus, if a period is still open
    // right now, the elapsed time since it started -- so a live query reflects the
    // up-to-the-moment total rather than only the last fully-closed period.
    statusInfo->setBusyTime((state->busyTimeAccumulated
        + (state->busyStartTime >= SIMTIME_ZERO ? simTime() - state->busyStartTime : SIMTIME_ZERO)).dbl());
    statusInfo->setRwndLimited((state->rwndLimitedAccumulated
        + (state->rwndLimitedStartTime >= SIMTIME_ZERO ? simTime() - state->rwndLimitedStartTime : SIMTIME_ZERO)).dbl());
    statusInfo->setSndbufLimited((state->sndbufLimitedAccumulated
        + (state->sndbufLimitedStartTime >= SIMTIME_ZERO ? simTime() - state->sndbufLimitedStartTime : SIMTIME_ZERO)).dbl());

    // Segment counts are approximated from byte totals by rounding UP: Linux
    // counts skbs, and a single retransmitted/lost sub-MSS segment (e.g. the
    // 1420-byte TFO fallback rexmit against a 1460 MSS) must report 1, not 0
    // (syn-data-only-syn-acked asserts tcpi_retrans == 1).
    if (state->sack_enabled && rexmitQueue != nullptr && state->snd_mss > 0)
        statusInfo->setLost((rexmitQueue->getLost() + state->snd_mss - 1) / state->snd_mss);
    else
        statusInfo->setLost(UINT_MAX);

    if (state->sack_enabled && rexmitQueue != nullptr && state->snd_mss > 0)
        statusInfo->setRetrans((rexmitQueue->getRetrans() + state->snd_mss - 1) / state->snd_mss);
    else
        statusInfo->setRetrans(UINT_MAX);

    // Linux SK_MEMINFO_RCVBUF: the live sk_rcvbuf -- receiveBufferSize as
    // configured, possibly grown by tcp_clamp_window under OOO pressure
    // (ooo-before-and-after-accept asserts both the untouched embryonic value
    // and the post-accept growth). 0 when no buffer size is configured.
    statusInfo->setSkRcvbuf(state->rcvBufferSize);

    if (auto *baseAlgState = dynamic_cast<TcpAlgorithmBaseStateVariables *>(state)) {
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

