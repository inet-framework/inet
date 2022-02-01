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
#include "inet/transportlayer/tcp/TcpSendQueue.h"
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

            if (remoteAddr.isUnspecified() || remotePort == -1)
                throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: remote address and port must be specified");

            if (localPort == -1) {
                localPort = tcpMain->getEphemeralPort();
                EV_DETAIL << "Assigned ephemeral port " << localPort << "\n";
            }

            EV_DETAIL << "OPEN: " << localAddr << ":" << localPort << " --> " << remoteAddr << ":" << remotePort << "\n";

            tcpMain->addSockPair(this, localAddr, remoteAddr, localPort, remotePort);

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
            sendQueue->enqueueAppData(packet); // queue up for later
            EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            EV_DETAIL << "Queueing up data for sending later.\n";
            sendQueue->enqueueAppData(packet); // queue up for later
            EV_DETAIL << sendQueue->getBytesAvailable(state->snd_una) << " bytes in queue\n";
            break;

        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
            sendQueue->enqueueAppData(packet);
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
    if (isToBeAccepted())
        throw cRuntimeError("READ without ACCEPT");
    delete msg;
    Packet *dataMsg;
    while ((dataMsg = receiveQueue->extractBytesUpTo(state->rcv_nxt)) != nullptr) {
        dataMsg->setKind(TCP_I_DATA);
        dataMsg->addTag<SocketInd>()->setSocketId(socketId);
        sendToApp(dataMsg);
    }
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

    msg->setControlInfo(statusInfo);
    msg->setKind(TCP_I_STATUS);
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

