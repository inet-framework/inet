//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <string.h>
#include "TCPMain.h"
#include "TCPConnection.h"
#include "TCPSegment_m.h"
#include "TCPCommand_m.h"
#include "IPControlInfo_m.h"
#include "TCPSendQueue.h"
#include "TCPReceiveQueue.h"
#include "TCPAlgorithm.h"


//
// Event processing code
//

void TCPConnection::process_OPEN_ACTIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpCommand);

    switch(fsm.state())
    {
        case TCP_S_INIT:
            initConnection(openCmd);
            // no break: run on to TCP_S_LISTEN code
        case TCP_S_LISTEN:
            // store local/remote socket
            state->active = true;
            if (openCmd->getLocalAddr().isNull() || openCmd->getRemoteAddr().isNull() || openCmd->getLocalPort()==-1)
                opp_error("Error processing command OPEN_ACTIVE: at least remote addr+port and local port must be specified");

            tcpMain->updateSockPair(this, openCmd->getLocalAddr(), openCmd->getRemoteAddr(),
                                          openCmd->getLocalPort(), openCmd->getRemotePort());

            // send initial SYN
            selectInitialSeqNum();
            sendSyn();
            scheduleTimeout(connEstabTimer, TCP_TIMEOUT_CONN_ESTAB);
            break;

        default:
            opp_error("Error processing command OPEN_ACTIVE: connection already exists");
    }

    delete openCmd;
    delete msg;
}

void TCPConnection::process_OPEN_PASSIVE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpCommand);

    switch(fsm.state())
    {
        case TCP_S_INIT:
            initConnection(openCmd);
            // no break: run on to TCP_S_LISTEN code
        case TCP_S_LISTEN:
            // store local/remote socket
            state->active = false;
            if (openCmd->getLocalPort()==-1)
                opp_error("Error processing command OPEN_PASSIVE: at least local port must be specified");
            tcpMain->updateSockPair(this, openCmd->getLocalAddr(), openCmd->getRemoteAddr(),
                                          openCmd->getLocalPort(), openCmd->getRemotePort());
            break;

        default:
            opp_error("Error processing command OPEN_PASSIVE: connection already exists");
    }

    delete openCmd;
    delete msg;
}

void TCPConnection::process_SEND(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    TCPSendCommand *sendCommand = check_and_cast<TCPSendCommand *>(tcpCommand);

    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command SEND: connection not open");

        case TCP_S_LISTEN:
            // this turns passive open into active open, send initial SYN
            state->active = true;
            selectInitialSeqNum();
            sendSyn();
            sendQueue->enqueueAppData(msg);  // queue up for later
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            sendQueue->enqueueAppData(msg); // queue up for later
            break;

        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
            sendQueue->enqueueAppData(msg);
            tcpAlgorithm->sendCommandInvoked();
            break;

        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command SEND: connection closing");
    }

    delete sendCommand;
    // FIXME who deletes msg?
}

void TCPConnection::process_CLOSE(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand;
    delete msg;

    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command CLOSE: connection not open");

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
            if (state->snd_nxt==sendQueue->bufferEndSeq())
            {
                ev << "No outstanding SENDs, sending FIN right away, advancing snd_nxt over the FIN\n";
                sendFin();
                state->snd_nxt++;
                // state transition will automatically take us to FIN_WAIT_1 (or LAST_ACK)
            }
            else
            {
                ev << "SEND of " << (sendQueue->bufferEndSeq()-state->snd_nxt) <<
                      " bytes pending, deferring sending of FIN\n";
                // if we are in ESTABLISHED, go to FIN_WAIT_1 right away, no need
                // to defer that to the time we actually send the FIN. (Actually,
                // deferring the transition would cause endless complications on the code.)
                if (fsm.state()==TCP_S_CLOSE_WAIT)
                    event = TCP_E_IGNORE;  // this will defer transition to CLOSE_WAIT
                                           // FIXME even this might be unnecessary!!!
            }
            state->send_fin = true;
            state->snd_fin_seq = sendQueue->bufferEndSeq();
            break;

        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_LAST_ACK:
        case TCP_S_TIME_WAIT:
            // RFC 793 is not entirely clear on how to handle a duplicate close request.
            // Here we treat it as an error.
            opp_error("Duplicate CLOSE command: connection already closing");
    }
}

void TCPConnection::process_ABORT(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand;
    delete msg;

    //
    // The ABORT event will automatically take the connection to the CLOSED
    // state, flush queues etc -- no need to do it here.
    //
    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command ABORT: connection not open");

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

void TCPConnection::process_STATUS(TCPEventCode& event, TCPCommand *tcpCommand, cMessage *msg)
{
    delete tcpCommand; // but reuse msg for reply

    if (fsm.state()==TCP_S_INIT)
        opp_error("Error processing command STATUS: connection not open");

    TCPStatusInfo *statusInfo = new TCPStatusInfo();

    statusInfo->setState(fsm.state());
    statusInfo->setStateName(stateName(fsm.state()));

    statusInfo->setLocalAddr(localAddr);
    statusInfo->setRemoteAddr(remoteAddr);
    statusInfo->setLocalPort(localPort);
    statusInfo->setRemotePort(remotePort);

    statusInfo->setSnd_mss(state->snd_mss);
    statusInfo->setSnd_una(state->snd_una);
    statusInfo->setSnd_nxt(state->snd_nxt);
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
    sendToApp(msg);
}


