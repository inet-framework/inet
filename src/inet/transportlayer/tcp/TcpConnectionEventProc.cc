//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <string.h>

#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBaseState_m.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

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

            // TCP Fast Open (RFC 7413 section 4.2): with a cached cookie for the
            // server, the SYN waits for the first SEND, and carries its data
            // (process_SEND()); without a cookie, the SYN goes out now and requests
            // one. After a suspected blackhole, the connection does not use Fast Open
            // (RFC 7413 section 4.1.3.1; Linux sends no Fast Open option then).
            if (openCmd->getFastOpen() && state->fastopenClientEnabled && tcpMain->isActiveFastOpenDisabled())
                EV_DETAIL << "Fast Open: stopped after a suspected blackhole\n";
            else if (openCmd->getFastOpen() && state->fastopenClientEnabled) {
                state->fastopenRequested = true;
                // without a cookie, as Linux can (fastopenClientNoCookieRequired), the SYN
                // waits for the first SEND too, and carries no option
                std::vector<uint8_t> cookie;
                if (tcpMain->getFastOpenCookie(remoteAddr, cookie) || state->fastopenClientNoCookieRequired) {
                    selectInitialSeqNum();
                    state->fastopenSynDeferred = true;
                    scheduleAfter(TCP_TIMEOUT_CONN_ESTAB, connEstabTimer);
                    EV_DETAIL << "Fast Open: the SYN waits for the first SEND\n";
                    break;
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
    // FIXME how to support PUSH? One option is to treat each SEND as a unit of data,
    // and set PSH at SEND boundaries
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
                // TCP Fast Open (RFC 7413 section 4.2.2): the server of a Fast Open
                // connection can send before the ACK of its SYN-ACK arrives
                EV_DETAIL << "Fast Open: sending data in SYN_RCVD\n";
                tcpAlgorithm->sendCommandInvoked();
            }
            else {
                EV_DETAIL << "Queueing up data for sending later.\n";
                EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            }
            break;

        case TCP_S_SYN_SENT:
            enqueueSendCommandData(packet); // queue up for later
            if (state->fastopenSynDeferred) {
                // TCP Fast Open (RFC 7413 section 4.2.2): the SEND that the SYN waits
                // for. The SYN carries as much of the data as the cached MSS of the
                // server allows, less the largest option space (as Linux does); a
                // SEND of zero bytes sends the SYN without data.
                uint32_t cachedMss = tcpMain->getFastOpenCachedMss(remoteAddr);
                uint32_t mss = cachedMss > 0 ? cachedMss : state->snd_mss;
                uint32_t maxSynData = mss > 40 ? mss - 40 : mss;
                state->fastopenSynDataLen = std::min(sendQueue->getBytesAvailable(state->iss + 1), maxSynData);
                sendSyn(); // with fastopenSynDeferred, writeHeaderOptions() writes the options of a first SYN
                state->fastopenSynDeferred = false;
                startSynRexmitTimer();
                break;
            }
            EV_DETAIL << "Queueing up data for sending later.\n";
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
        // as in sendAvailableDataToApp(): a received FIN occupies a sequence
        // number but puts no byte in the queue, so the read stops at the FIN
        if (state->fin_rcvd && seqLess(state->rcv_fin_seq, endSeqNo))
            endSeqNo = state->rcv_fin_seq;
        uint32_t requestedEndPos = receiveQueue->getFirstSeqNo() + maxByteCountRequested;
        if (seqLess(requestedEndPos, endSeqNo))
            endSeqNo = requestedEndPos;
        if (Packet *dataMsg = receiveQueue->extractBytesUpTo(endSeqNo)) {
            dataMsg->setKind(TCP_I_DATA);
            dataMsg->addTag<SocketInd>()->setSocketId(socketId);
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
    else
        throw cRuntimeError("Unknown subclass of TcpSetOptionCommand received from app: %s", tcpCommand->getClassName());
    delete tcpCommand;
    delete msg;
}

void TcpConnection::process_CLOSE(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg)
{
    // RFC 9293: a CLOSE means "I have no more to send", and the application may still
    // receive. With abortOnDataAfterClose, a CLOSE without halfClose is the full close
    // of Linux close(), which also shuts the receive side down (RFC 1122 section
    // 4.2.2.13 lets a host implement such a "half-duplex" close). state is still null
    // for a CLOSE that reaches a new connection in INIT.
    if (state != nullptr && tcpMain->par("abortOnDataAfterClose").boolValue()
            && (tcpCommand == nullptr || !tcpCommand->getHalfClose()))
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

    // Adaptive reordering: state->reordering grows past the static dupthresh as
    // checkSackReordering() observes SACKs arriving below the FACK (Linux
    // tp->reordering). Report the live degree, not the static dupthresh.
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

    if (auto *classicState = dynamic_cast<TcpClassicAlgorithmBaseStateVariables *>(state))
        statusInfo->setSsthresh(classicState->ssthresh);
    else
        statusInfo->setSsthresh(UINT_MAX);

    statusInfo->setCaState(deriveLinuxCaState());
    // rcv_nxt/irs are only meaningful once the 3WHS has fixed irs (peer's ISN); before
    // that (e.g. a STATUS query in SYN_SENT) both are still 0 and the subtraction
    // would underflow.
    statusInfo->setBytesReceived(seqGreater(state->rcv_nxt, state->irs) ? state->rcv_nxt - state->irs - 1 : 0);

    // TCP_INFO time counters: report the accumulated total plus, if a period is
    // still open right now, the elapsed time since it started -- so a live query
    // reflects the up-to-the-moment total rather than only the last closed period.
    statusInfo->setBusyTime((state->busyTimeAccumulated
        + (state->busyStartTime >= SIMTIME_ZERO ? simTime() - state->busyStartTime : SIMTIME_ZERO)).dbl());
    statusInfo->setRwndLimited((state->rwndLimitedAccumulated
        + (state->rwndLimitedStartTime >= SIMTIME_ZERO ? simTime() - state->rwndLimitedStartTime : SIMTIME_ZERO)).dbl());

    // Segment counts are approximated from byte totals by rounding UP: Linux
    // counts skbs, and a single retransmitted/lost sub-MSS segment must report 1,
    // not 0.
    if (state->sack_enabled && rexmitQueue != nullptr && state->snd_mss > 0)
        statusInfo->setLost((rexmitQueue->getLost() + state->snd_mss - 1) / state->snd_mss);
    else
        statusInfo->setLost(UINT_MAX);

    if (state->sack_enabled && rexmitQueue != nullptr && state->snd_mss > 0)
        statusInfo->setRetrans((rexmitQueue->getRetrans() + state->snd_mss - 1) / state->snd_mss);
    else
        statusInfo->setRetrans(UINT_MAX);

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
    // Every other reply-sending path tags its outgoing message with SocketInd
    // (see sendIndicationToApp() and friends in TcpConnectionUtil.cc) so
    // TcpSocket::belongsToSocket() can match it back to the requesting app-side
    // socket. This path reuses the incoming request message, which only carried
    // a SocketReq tag -- without SocketInd, the app's socket dispatch rejects the
    // STATUS reply instead of passing it to TcpSocket::ICallback::socketStatusArrived().
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

