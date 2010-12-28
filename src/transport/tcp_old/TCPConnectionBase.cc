//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <string.h>
#include <assert.h>
#include "TCP_old.h"
#include "TCPConnection_old.h"
#include "TCPSegment.h"
#include "TCPCommand_m.h"
#include "TCPSendQueue_old.h"
#include "TCPReceiveQueue_old.h"
#include "TCPAlgorithm_old.h"

using namespace tcp_old;

TCPStateVariables::TCPStateVariables()
{
    // set everything to 0 -- real init values will be set manually
    active = false;
    fork = false;
    snd_mss = -1; // will be set from configureStateVariables()
    snd_una = 0;
    snd_nxt = 0;
    snd_max = 0;
    snd_wnd = 0;
    snd_up = 0;
    snd_wl1 = 0;
    snd_wl2 = 0;
    iss = 0;
    rcv_nxt = 0;
    rcv_wnd = -1; // will be set from configureStateVariables()
    rcv_up = 0;
    irs = 0;

    dupacks = 0;

    syn_rexmit_count = 0;
    syn_rexmit_timeout = 0;

    fin_ack_rcvd = false;
    send_fin = false;
    snd_fin_seq = 0;
    fin_rcvd = false;
    rcv_fin_seq = 0;
    afterRto = false;

    last_ack_sent = 0;
}

std::string TCPStateVariables::info() const
{
    std::stringstream out;
    out <<  "snd_una=" << snd_una;
    out << " snd_nxt=" << snd_nxt;
    out << " snd_max=" << snd_max;
    out << " snd_wnd=" << snd_wnd;
    out << " rcv_nxt=" << rcv_nxt;
    out << " rcv_wnd=" << rcv_wnd;
    return out.str();
}

std::string TCPStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << "active = " << active << "\n";
    out << "snd_mss = " << snd_mss << "\n";
    out << "snd_una = " << snd_una << "\n";
    out << "snd_nxt = " << snd_nxt << "\n";
    out << "snd_max = " << snd_max << "\n";
    out << "snd_wnd = " << snd_wnd << "\n";
    out << "snd_up = " << snd_up << "\n";
    out << "snd_wl1 = " << snd_wl1 << "\n";
    out << "snd_wl2 = " << snd_wl2 << "\n";
    out << "iss = " << iss << "\n";
    out << "rcv_nxt = " << rcv_nxt << "\n";
    out << "rcv_wnd = " << rcv_wnd << "\n";
    out << "rcv_up = " << rcv_up << "\n";
    out << "irs = " << irs << "\n";
    out << "fin_ack_rcvd = " << fin_ack_rcvd << "\n";
    return out.str();
}

TCPConnection::TCPConnection()
{
    // Note: this ctor is NOT used to create live connections, only
    // temporary ones to invoke segmentArrivalWhileClosed() on
    sendQueue = NULL;
    receiveQueue = NULL;
    tcpAlgorithm = NULL;
    state = NULL;
    the2MSLTimer = connEstabTimer = finWait2Timer = synRexmitTimer = NULL;
    sndWndVector = sndNxtVector = sndAckVector = rcvSeqVector = rcvAckVector = unackedVector = NULL;
}

//
// FSM framework, TCP FSM
//

TCPConnection::TCPConnection(TCP *_mod, int _appGateIndex, int _connId)
{
    tcpMain = _mod;
    appGateIndex = _appGateIndex;
    connId = _connId;

    localPort = remotePort = -1;

    char fsmname[24];
    sprintf(fsmname, "fsm-%d", connId);
    fsm.setName(fsmname);
    fsm.setState(TCP_S_INIT);


    // queues and algorithm will be created on active or passive open
    sendQueue = NULL;
    receiveQueue = NULL;
    tcpAlgorithm = NULL;
    state = NULL;

    the2MSLTimer = new cMessage("2MSL");
    connEstabTimer = new cMessage("CONN-ESTAB");
    finWait2Timer = new cMessage("FIN-WAIT-2");
    synRexmitTimer = new cMessage("SYN-REXMIT");

    the2MSLTimer->setContextPointer(this);
    connEstabTimer->setContextPointer(this);
    finWait2Timer->setContextPointer(this);
    synRexmitTimer->setContextPointer(this);

    // statistics
    sndWndVector = NULL;
    sndNxtVector = NULL;
    sndAckVector = NULL;
    rcvSeqVector = NULL;
    rcvAckVector = NULL;
    unackedVector = NULL;

    if (getTcpMain()->recordStatistics)
    {
        sndWndVector = new cOutVector("send window");
        sndNxtVector = new cOutVector("send seq");
        sndAckVector = new cOutVector("sent ack");
        rcvSeqVector = new cOutVector("rcvd seq");
        rcvAckVector = new cOutVector("rcvd ack");
        unackedVector = new cOutVector("unacked bytes");
    }
}

TCPConnection::~TCPConnection()
{
    delete sendQueue;
    delete receiveQueue;
    delete tcpAlgorithm;
    delete state;

    if (the2MSLTimer)   delete cancelEvent(the2MSLTimer);
    if (connEstabTimer) delete cancelEvent(connEstabTimer);
    if (finWait2Timer)  delete cancelEvent(finWait2Timer);
    if (synRexmitTimer) delete cancelEvent(synRexmitTimer);

    // statistics
    delete sndWndVector;
    delete sndNxtVector;
    delete sndAckVector;
    delete rcvSeqVector;
    delete rcvAckVector;
    delete unackedVector;
}

bool TCPConnection::processTimer(cMessage *msg)
{
    printConnBrief();
    tcpEV << msg->getName() << " timer expired\n";

    // first do actions
    TCPEventCode event;
    if (msg==the2MSLTimer)
    {
        event = TCP_E_TIMEOUT_2MSL;
        process_TIMEOUT_2MSL();
    }
    else if (msg==connEstabTimer)
    {
        event = TCP_E_TIMEOUT_CONN_ESTAB;
        process_TIMEOUT_CONN_ESTAB();
    }
    else if (msg==finWait2Timer)
    {
        event = TCP_E_TIMEOUT_FIN_WAIT_2;
        process_TIMEOUT_FIN_WAIT_2();
    }
    else if (msg==synRexmitTimer)
    {
        event = TCP_E_IGNORE;
        process_TIMEOUT_SYN_REXMIT(event);
    }
    else
    {
        event = TCP_E_IGNORE;
        tcpAlgorithm->processTimer(msg, event);
    }

    // then state transitions
    return performStateTransition(event);
}

bool TCPConnection::processTCPSegment(TCPSegment *tcpseg, IPvXAddress segSrcAddr, IPvXAddress segDestAddr)
{
    printConnBrief();
    if (!localAddr.isUnspecified())
    {
        ASSERT(localAddr==segDestAddr);
        ASSERT(localPort==tcpseg->getDestPort());
    }
    if (!remoteAddr.isUnspecified())
    {
        ASSERT(remoteAddr==segSrcAddr);
        ASSERT(remotePort==tcpseg->getSrcPort());
    }

    if (tryFastRoute(tcpseg))
        return true;

    // first do actions
    TCPEventCode event = process_RCV_SEGMENT(tcpseg, segSrcAddr, segDestAddr);

    // then state transitions
    return performStateTransition(event);
}

bool TCPConnection::processAppCommand(cMessage *msg)
{
    printConnBrief();

    // first do actions
    TCPCommand *tcpCommand = (TCPCommand *)(msg->removeControlInfo());
    TCPEventCode event = preanalyseAppCommandEvent(msg->getKind());
    tcpEV << "App command: " << eventName(event) << "\n";
    switch (event)
    {
        case TCP_E_OPEN_ACTIVE: process_OPEN_ACTIVE(event, tcpCommand, msg); break;
        case TCP_E_OPEN_PASSIVE: process_OPEN_PASSIVE(event, tcpCommand, msg); break;
        case TCP_E_SEND: process_SEND(event, tcpCommand, msg); break;
        case TCP_E_CLOSE: process_CLOSE(event, tcpCommand, msg); break;
        case TCP_E_ABORT: process_ABORT(event, tcpCommand, msg); break;
        case TCP_E_STATUS: process_STATUS(event, tcpCommand, msg); break;
        default: opp_error("wrong event code");
    }

    // then state transitions
    return performStateTransition(event);
}


TCPEventCode TCPConnection::preanalyseAppCommandEvent(int commandCode)
{
    switch (commandCode)
    {
        case TCP_C_OPEN_ACTIVE:  return TCP_E_OPEN_ACTIVE;
        case TCP_C_OPEN_PASSIVE: return TCP_E_OPEN_PASSIVE;
        case TCP_C_SEND:         return TCP_E_SEND;
        case TCP_C_CLOSE:        return TCP_E_CLOSE;
        case TCP_C_ABORT:        return TCP_E_ABORT;
        case TCP_C_STATUS:       return TCP_E_STATUS;
        default: opp_error("Unknown message kind in app command");
                 return (TCPEventCode)0; // to satisfy compiler
    }
}

bool TCPConnection::performStateTransition(const TCPEventCode& event)
{
    ASSERT(fsm.getState()!=TCP_S_CLOSED); // closed connections should be deleted immediately

    if (event==TCP_E_IGNORE)  // e.g. discarded segment
    {
        tcpEV << "Staying in state: " << stateName(fsm.getState()) << " (no FSM event)\n";
        return true;
    }

    // state machine
    // TBD add handling of connection timeout event (keepalive), with transition to CLOSED
    // Note: empty "default:" lines are for gcc's benefit which would otherwise spit warnings
    int oldState = fsm.getState();
    switch (fsm.getState())
    {
        case TCP_S_INIT:
            switch (event)
            {
                case TCP_E_OPEN_PASSIVE:FSM_Goto(fsm, TCP_S_LISTEN); break;
                case TCP_E_OPEN_ACTIVE: FSM_Goto(fsm, TCP_S_SYN_SENT); break;
                default:;
            }
            break;

        case TCP_S_LISTEN:
            switch (event)
            {
                case TCP_E_OPEN_ACTIVE: FSM_Goto(fsm, TCP_S_SYN_SENT); break;
                case TCP_E_SEND:        FSM_Goto(fsm, TCP_S_SYN_SENT); break;
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_SYN:     FSM_Goto(fsm, TCP_S_SYN_RCVD);break;
                default:;
            }
            break;

        case TCP_S_SYN_RCVD:
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_FIN_WAIT_1); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_CONN_ESTAB: FSM_Goto(fsm, state->active ? TCP_S_CLOSED : TCP_S_LISTEN); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, state->active ? TCP_S_CLOSED : TCP_S_LISTEN); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_ESTABLISHED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSE_WAIT); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_SYN_SENT:
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_CONN_ESTAB: FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_SYN_ACK: FSM_Goto(fsm, TCP_S_ESTABLISHED); break;
                case TCP_E_RCV_SYN:     FSM_Goto(fsm, TCP_S_SYN_RCVD); break;
                default:;
            }
            break;

        case TCP_S_ESTABLISHED:
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_FIN_WAIT_1); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSE_WAIT); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_CLOSE_WAIT:
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_LAST_ACK); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_LAST_ACK:
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_FIN_WAIT_1:
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSING); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_FIN_WAIT_2); break;
                case TCP_E_RCV_FIN_ACK: FSM_Goto(fsm, TCP_S_TIME_WAIT); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_FIN_WAIT_2:
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_TIME_WAIT); break;
                case TCP_E_TIMEOUT_FIN_WAIT_2: FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_CLOSING:
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_TIME_WAIT); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_TIME_WAIT:
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_2MSL: FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_UNEXP_SYN: FSM_Goto(fsm, TCP_S_CLOSED); break;
                default:;
            }
            break;

        case TCP_S_CLOSED:
            break;
    }

    if (oldState!=fsm.getState())
    {
        tcpEV << "Transition: " << stateName(oldState) << " --> " << stateName(fsm.getState()) << "  (event was: " << eventName(event) << ")\n";
        testingEV << tcpMain->getName() << ": " << stateName(oldState) << " --> " << stateName(fsm.getState()) << "  (on " << eventName(event) << ")\n";

        // cancel timers, etc.
        stateEntered(fsm.getState());
    }
    else
    {
        tcpEV << "Staying in state: " << stateName(fsm.getState()) << " (event was: " << eventName(event) << ")\n";
    }

    return fsm.getState()!=TCP_S_CLOSED;
}

void TCPConnection::stateEntered(int state)
{
    // cancel timers
    switch (state)
    {
        case TCP_S_INIT:
            // we'll never get back to INIT
            break;
        case TCP_S_LISTEN:
            // we may get back to LISTEN from SYN_RCVD
            ASSERT(connEstabTimer && synRexmitTimer);
            cancelEvent(connEstabTimer);
            cancelEvent(synRexmitTimer);
            break;
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            break;
        case TCP_S_ESTABLISHED:
            // we're in ESTABLISHED, these timers are no longer needed
            delete cancelEvent(connEstabTimer);
            delete cancelEvent(synRexmitTimer);
            connEstabTimer = synRexmitTimer = NULL;
            // TCP_I_ESTAB notification moved inside event processing
            break;
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            // whether connection setup succeeded (ESTABLISHED) or not (others),
            // cancel these timers
            if (connEstabTimer) cancelEvent(connEstabTimer);
            if (synRexmitTimer) cancelEvent(synRexmitTimer);
            break;
        case TCP_S_CLOSED:
            // all timers need to be cancelled
            if (the2MSLTimer)   cancelEvent(the2MSLTimer);
            if (connEstabTimer) cancelEvent(connEstabTimer);
            if (finWait2Timer)  cancelEvent(finWait2Timer);
            if (synRexmitTimer) cancelEvent(synRexmitTimer);
            tcpAlgorithm->connectionClosed();
            break;
    }
}


