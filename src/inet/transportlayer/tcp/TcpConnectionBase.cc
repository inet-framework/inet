//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <assert.h>
#include <string.h>

#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

Define_Module(TcpConnection);

simsignal_t TcpConnection::stateSignal = registerSignal("state"); // FSM state
simsignal_t TcpConnection::sndWndSignal = registerSignal("sndWnd"); // snd_wnd
simsignal_t TcpConnection::rcvWndSignal = registerSignal("rcvWnd"); // rcv_wnd
simsignal_t TcpConnection::rcvAdvSignal = registerSignal("rcvAdv"); // current advertised window (=rcv_adv)
simsignal_t TcpConnection::sndNxtSignal = registerSignal("sndNxt"); // sent seqNo
simsignal_t TcpConnection::sndAckSignal = registerSignal("sndAck"); // sent ackNo
simsignal_t TcpConnection::rcvSeqSignal = registerSignal("rcvSeq"); // received seqNo
simsignal_t TcpConnection::rcvAckSignal = registerSignal("rcvAck"); // received ackNo (=snd_una)
simsignal_t TcpConnection::unackedSignal = registerSignal("unacked"); // number of bytes unacknowledged
simsignal_t TcpConnection::dupAcksSignal = registerSignal("dupAcks"); // current number of received dupAcks
simsignal_t TcpConnection::pipeSignal = registerSignal("pipe"); // current sender's estimate of bytes outstanding in the network
simsignal_t TcpConnection::sndSacksSignal = registerSignal("sndSacks"); // number of sent Sacks
simsignal_t TcpConnection::rcvSacksSignal = registerSignal("rcvSacks"); // number of received Sacks
simsignal_t TcpConnection::rcvOooSegSignal = registerSignal("rcvOooSeg"); // number of received out-of-order segments
simsignal_t TcpConnection::rcvNASegSignal = registerSignal("rcvNASeg"); // number of received not acceptable segments
simsignal_t TcpConnection::sackedBytesSignal = registerSignal("sackedBytes"); // current number of received sacked bytes
simsignal_t TcpConnection::tcpRcvQueueBytesSignal = registerSignal("tcpRcvQueueBytes"); // current amount of used bytes in tcp receive queue
simsignal_t TcpConnection::tcpRcvQueueDropsSignal = registerSignal("tcpRcvQueueDrops"); // number of drops in tcp receive queue
simsignal_t TcpConnection::tcpRcvPayloadBytesSignal = registerSignal("tcpRcvPayloadBytes"); // amount of payload bytes received (including duplicates, out of order etc) for TCP throughput

TcpStateVariables::~TcpStateVariables()
{
}

std::string TcpStateVariables::str() const
{
    std::stringstream out;
    out << "snd_una=" << snd_una;
    out << " snd_nxt=" << snd_nxt;
    out << " snd_max=" << snd_max;
    out << " snd_wnd=" << snd_wnd;
    out << " rcv_nxt=" << rcv_nxt;
    out << " rcv_wnd=" << rcv_wnd;
    return out.str();
}

std::string TcpStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << "active=" << active << "\n";
    out << "snd_mss=" << snd_mss << "\n";
    out << "snd_una=" << snd_una << "\n";
    out << "snd_nxt=" << snd_nxt << "\n";
    out << "snd_max=" << snd_max << "\n";
    out << "snd_wnd=" << snd_wnd << "\n";
    out << "snd_up=" << snd_up << "\n";
    out << "snd_wl1=" << snd_wl1 << "\n";
    out << "snd_wl2=" << snd_wl2 << "\n";
    out << "iss=" << iss << "\n";
    out << "rcv_nxt=" << rcv_nxt << "\n";
    out << "rcv_wnd=" << rcv_wnd << "\n";
    out << "rcv_up=" << rcv_up << "\n";
    out << "irs=" << irs << "\n";
    out << "rcv_adv=" << rcv_adv << "\n";
    out << "fin_ack_rcvd=" << fin_ack_rcvd << "\n";
    out << "nagle_enabled=" << nagle_enabled << "\n";
    out << "limited_transmit_enabled=" << limited_transmit_enabled << "\n";
    out << "increased_IW_enabled=" << increased_IW_enabled << "\n";
    out << "delayed_acks_enabled=" << delayed_acks_enabled << "\n";
    out << "ws_support=" << ws_support << "\n";
    out << "ws_enabled=" << ws_enabled << "\n";
    out << "ts_support=" << ts_support << "\n";
    out << "ts_enabled=" << ts_enabled << "\n";
    out << "sack_support=" << sack_support << "\n";
    out << "sack_enabled=" << sack_enabled << "\n";
    out << "snd_sack_perm=" << snd_sack_perm << "\n";
    out << "snd_sacks=" << snd_sacks << "\n";
    out << "rcv_sacks=" << rcv_sacks << "\n";
    out << "dupacks=" << dupacks << "\n";
    out << "rcv_oooseg=" << rcv_oooseg << "\n";
    out << "rcv_naseg=" << rcv_naseg << "\n";
    return out.str();
}

//
// FSM framework, TCP FSM
//

void TcpConnection::initConnection(Tcp *_mod, int _socketId)
{
    Enter_Method("initConnection");

    tcpMain = _mod;
    socketId = _socketId;

    fsm.setName(getName());
    fsm.setState(TCP_S_INIT);

    // queues and algorithm will be created on active or passive open

    the2MSLTimer = new cMessage("2MSL");
    connEstabTimer = new cMessage("CONN-ESTAB");
    finWait2Timer = new cMessage("FIN-WAIT-2");
    synRexmitTimer = new cMessage("SYN-REXMIT");

    the2MSLTimer->setContextPointer(this);
    connEstabTimer->setContextPointer(this);
    finWait2Timer->setContextPointer(this);
    synRexmitTimer->setContextPointer(this);
}

TcpConnection::~TcpConnection()
{
    delete sendQueue;
    delete rexmitQueue;
    delete receiveQueue;
    delete tcpAlgorithm;
    delete state;

    if (the2MSLTimer)
        delete cancelEvent(the2MSLTimer);
    if (connEstabTimer)
        delete cancelEvent(connEstabTimer);
    if (finWait2Timer)
        delete cancelEvent(finWait2Timer);
    if (synRexmitTimer)
        delete cancelEvent(synRexmitTimer);
}

void TcpConnection::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (!processTimer(msg))
            tcpMain->removeConnection(this);
    }
    else
        throw cRuntimeError("model error: TcpConnection allows only self messages");
}

bool TcpConnection::processTimer(cMessage *msg)
{
    printConnBrief();
    EV_DETAIL << msg->getName() << " timer expired\n";

    // first do actions
    TcpEventCode event;

    if (msg == the2MSLTimer) {
        event = TCP_E_TIMEOUT_2MSL;
        process_TIMEOUT_2MSL();
    }
    else if (msg == connEstabTimer) {
        event = TCP_E_TIMEOUT_CONN_ESTAB;
        process_TIMEOUT_CONN_ESTAB();
    }
    else if (msg == finWait2Timer) {
        event = TCP_E_TIMEOUT_FIN_WAIT_2;
        process_TIMEOUT_FIN_WAIT_2();
    }
    else if (msg == synRexmitTimer) {
        event = TCP_E_IGNORE;
        process_TIMEOUT_SYN_REXMIT(event);
    }
    else {
        event = TCP_E_IGNORE;
        tcpAlgorithm->processTimer(msg, event);
    }

    // then state transitions
    return performStateTransition(event);
}

bool TcpConnection::processTCPSegment(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address segSrcAddr, L3Address segDestAddr)
{
    Enter_Method("processTCPSegment");

    take(tcpSegment);
    printConnBrief();
    if (!localAddr.isUnspecified()) {
        ASSERT(localAddr == segDestAddr);
        ASSERT(localPort == tcpHeader->getDestPort());
    }

    if (!remoteAddr.isUnspecified()) {
        ASSERT(remoteAddr == segSrcAddr);
        ASSERT(remotePort == tcpHeader->getSrcPort());
    }

    if (tryFastRoute(tcpHeader))
        return true;

    // first do actions
    TcpEventCode event = process_RCV_SEGMENT(tcpSegment, tcpHeader, segSrcAddr, segDestAddr);

    // then state transitions
    return performStateTransition(event);
}

bool TcpConnection::processAppCommand(cMessage *msg)
{
    Enter_Method("processAppCommand");

    take(msg);
    printConnBrief();

    // first do actions
    TcpCommand *tcpCommand = check_and_cast_nullable<TcpCommand *>(msg->removeControlInfo());
    TcpEventCode event = preanalyseAppCommandEvent(msg->getKind());
    EV_INFO << "App command: " << eventName(event) << "\n";

    switch (event) {
        case TCP_E_OPEN_ACTIVE:
            process_OPEN_ACTIVE(event, tcpCommand, msg);
            break;

        case TCP_E_OPEN_PASSIVE:
            process_OPEN_PASSIVE(event, tcpCommand, msg);
            break;

        case TCP_E_ACCEPT:
            process_ACCEPT(event, tcpCommand, msg);
            break;

        case TCP_E_SEND:
            process_SEND(event, tcpCommand, msg);
            break;

        case TCP_E_CLOSE:
            process_CLOSE(event, tcpCommand, msg);
            break;

        case TCP_E_ABORT:
            process_ABORT(event, tcpCommand, msg);
            break;

        case TCP_E_DESTROY:
            process_DESTROY(event, tcpCommand, msg);
            break;

        case TCP_E_STATUS:
            process_STATUS(event, tcpCommand, msg);
            break;

        case TCP_E_QUEUE_BYTES_LIMIT:
            process_QUEUE_BYTES_LIMIT(event, tcpCommand, msg);
            break;

        case TCP_E_READ:
            process_READ_REQUEST(event, tcpCommand, msg);
            break;

        case TCP_E_SETOPTION:
            process_OPTIONS(event, tcpCommand, msg);
            break;

        default:
            throw cRuntimeError(tcpMain, "wrong event code");
    }

    // then state transitions
    return performStateTransition(event);
}

TcpEventCode TcpConnection::preanalyseAppCommandEvent(int commandCode)
{
    switch (commandCode) {
        case TCP_C_OPEN_ACTIVE:
            return TCP_E_OPEN_ACTIVE;

        case TCP_C_OPEN_PASSIVE:
            return TCP_E_OPEN_PASSIVE;

        case TCP_C_ACCEPT:
            return TCP_E_ACCEPT;

        case TCP_C_SEND:
            return TCP_E_SEND;

        case TCP_C_CLOSE:
            return TCP_E_CLOSE;

        case TCP_C_ABORT:
            return TCP_E_ABORT;

        case TCP_C_DESTROY:
            return TCP_E_DESTROY;

        case TCP_C_STATUS:
            return TCP_E_STATUS;

        case TCP_C_QUEUE_BYTES_LIMIT:
            return TCP_E_QUEUE_BYTES_LIMIT;

        case TCP_C_READ:
            return TCP_E_READ;

        case TCP_C_SETOPTION:
            return TCP_E_SETOPTION;

        default:
            throw cRuntimeError(tcpMain, "Unknown message kind in app command");
    }
}

bool TcpConnection::performStateTransition(const TcpEventCode& event)
{
    ASSERT(fsm.getState() != TCP_S_CLOSED); // closed connections should be deleted immediately

    if (event == TCP_E_IGNORE) { // e.g. discarded segment
        EV_DETAIL << "Staying in state: " << stateName(fsm.getState()) << " (no FSM event)\n";
        return true;
    }

    // state machine
    // TODO add handling of connection timeout event (KEEP-ALIVE), with transition to CLOSED
    // Note: empty "default:" lines are for gcc's benefit which would otherwise spit warnings
    int oldState = fsm.getState();

    switch (fsm.getState()) {
        case TCP_S_INIT:
            switch (event) {
                case TCP_E_OPEN_PASSIVE:
                    FSM_Goto(fsm, TCP_S_LISTEN);
                    break;

                case TCP_E_OPEN_ACTIVE:
                    FSM_Goto(fsm, TCP_S_SYN_SENT);
                    break;

                case TCP_E_DESTROY:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_LISTEN:
            switch (event) {
                case TCP_E_OPEN_ACTIVE:
                    FSM_Goto(fsm, TCP_S_SYN_SENT);
                    break;

                case TCP_E_SEND:
                    FSM_Goto(fsm, TCP_S_SYN_SENT);
                    break;

                case TCP_E_CLOSE:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_ABORT:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_DESTROY:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_SYN:
                    FSM_Goto(fsm, TCP_S_SYN_RCVD);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_SYN_RCVD:
            switch (event) {
                case TCP_E_CLOSE:
                    FSM_Goto(fsm, TCP_S_FIN_WAIT_1);
                    break;

                case TCP_E_ABORT:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_DESTROY:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_TIMEOUT_CONN_ESTAB:
                    FSM_Goto(fsm, state->active ? TCP_S_CLOSED : TCP_S_LISTEN);
                    break;

                case TCP_E_RCV_RST:
                    FSM_Goto(fsm, state->active ? TCP_S_CLOSED : TCP_S_LISTEN);
                    break;

                case TCP_E_RCV_ACK:
                    FSM_Goto(fsm, TCP_S_ESTABLISHED);
                    break;

                case TCP_E_RCV_FIN:
                    FSM_Goto(fsm, TCP_S_CLOSE_WAIT);
                    break;

                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_SYN_SENT:
            switch (event) {
                case TCP_E_CLOSE:
                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_TIMEOUT_CONN_ESTAB:
                case TCP_E_RCV_RST:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_SYN_ACK:
                    FSM_Goto(fsm, TCP_S_ESTABLISHED);
                    break;

                case TCP_E_RCV_SYN:
                    FSM_Goto(fsm, TCP_S_SYN_RCVD);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_ESTABLISHED:
            switch (event) {
                case TCP_E_CLOSE:
                    FSM_Goto(fsm, TCP_S_FIN_WAIT_1);
                    break;

                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_FIN:
                    FSM_Goto(fsm, TCP_S_CLOSE_WAIT);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_CLOSE_WAIT:
            switch (event) {
                case TCP_E_CLOSE:
                    FSM_Goto(fsm, TCP_S_LAST_ACK);
                    break;

                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_LAST_ACK:
            switch (event) {
                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_RCV_ACK:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_FIN_WAIT_1:
            switch (event) {
                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_FIN:
                    FSM_Goto(fsm, TCP_S_CLOSING);
                    break;

                case TCP_E_RCV_ACK:
                    FSM_Goto(fsm, TCP_S_FIN_WAIT_2);
                    break;

                case TCP_E_RCV_FIN_ACK:
                    FSM_Goto(fsm, TCP_S_TIME_WAIT);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_FIN_WAIT_2:
            switch (event) {
                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_TIMEOUT_FIN_WAIT_2:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_FIN:
                    FSM_Goto(fsm, TCP_S_TIME_WAIT);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_CLOSING:
            switch (event) {
                case TCP_E_ABORT:
                case TCP_E_DESTROY:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                case TCP_E_RCV_ACK:
                    FSM_Goto(fsm, TCP_S_TIME_WAIT);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_TIME_WAIT:
            switch (event) {
                case TCP_E_ABORT:
                case TCP_E_TIMEOUT_2MSL:
                case TCP_E_RCV_RST:
                case TCP_E_RCV_UNEXP_SYN:
                case TCP_E_DESTROY:
                    FSM_Goto(fsm, TCP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case TCP_S_CLOSED:
            break;
    }

    if (oldState != fsm.getState()) {
        EV_INFO << "Transition: " << stateName(oldState) << " --> " << stateName(fsm.getState()) << "  (event was: " << eventName(event) << ")\n";
        EV_DEBUG_C("testing") << tcpMain->getName() << ": " << stateName(oldState) << " --> " << stateName(fsm.getState()) << "  (on " << eventName(event) << ")\n";

        // cancel timers, etc.
        stateEntered(fsm.getState(), oldState, event);
    }
    else {
        EV_DETAIL << "Staying in state: " << stateName(fsm.getState()) << " (event was: " << eventName(event) << ")\n";
    }

    return fsm.getState() != TCP_S_CLOSED;
}

void TcpConnection::stateEntered(int state, int oldState, TcpEventCode event)
{
    // cancel timers
    switch (state) {
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
            connEstabTimer = synRexmitTimer = nullptr;
            // TCP_I_ESTAB notification moved inside event processing
            break;

        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
            if (state == TCP_S_CLOSE_WAIT)
                sendIndicationToApp(TCP_I_PEER_CLOSED);
            // whether connection setup succeeded (ESTABLISHED) or not (others),
            // cancel these timers
            if (connEstabTimer)
                cancelEvent(connEstabTimer);
            if (synRexmitTimer)
                cancelEvent(synRexmitTimer);
            break;

        case TCP_S_TIME_WAIT:
            sendIndicationToApp(TCP_I_CLOSED);
            break;

        case TCP_S_CLOSED:
            if (oldState != TCP_S_TIME_WAIT && event != TCP_E_ABORT)
                sendIndicationToApp(TCP_I_CLOSED);
            // all timers need to be cancelled
            if (the2MSLTimer)
                cancelEvent(the2MSLTimer);
            if (connEstabTimer)
                cancelEvent(connEstabTimer);
            if (finWait2Timer)
                cancelEvent(finWait2Timer);
            if (synRexmitTimer)
                cancelEvent(synRexmitTimer);
            tcpAlgorithm->connectionClosed();
            break;
    }
}

} // namespace tcp
} // namespace inet

