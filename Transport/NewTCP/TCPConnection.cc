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
#include "TCPConnection.h"
#include "TCPSegment_m.h"
#include "TCPInterfacePacket_m.h"
#include "IPInterfacePacket.h"
#include "TCPSendQueue.h"
#include "TCPReceiveQueue.h"


TCPConnection::TCPConnection(int _connId, cSimpleModule *_mod)
{
    connId = _connId;
    module = _mod;

    localPort = remotePort = -1;

    char fsmname[24];
    sprintf(fsmname, "fsm-%d", connId);
    fsm.setName(fsmname);
    fsm.setState(TCP_S_INIT);

    memset(&state,0,sizeof(state));

    // queues will be created on active or passive open
    sendQueue = NULL;
    receiveQueue = NULL;
}

TCPConnection::~TCPConnection()
{
    delete sendQueue;
    delete receiveQueue;
}

bool TCPConnection::processTimer(cMessage *msg)
{
    // first do actions
    TCPEvent event = analyseTimerEvent(msg);
    switch (event)
    {
        case TCP_E_TIMEOUT_TIME_WAIT: process_TIMEOUT_TIME_WAIT(event,msg); break;
        case TCP_E_TIMEOUT_REXMT: process_TIMEOUT_REXMT(event,msg); break;
        case TCP_E_TIMEOUT_PERSIST: process_TIMEOUT_PERSIST(event,msg); break;
        case TCP_E_TIMEOUT_KEEPALIVE: process_TIMEOUT_KEEPALIVE(event,msg); break;
        case TCP_E_TIMEOUT_CONN_ESTAB: process_TIMEOUT_CONN_ESTAB(event,msg); break;
        case TCP_E_TIMEOUT_FIN_WAIT_2: process_TIMEOUT_FIN_WAIT_2(event,msg); break;
        case TCP_E_TIMEOUT_DELAYED_ACK: process_TIMEOUT_DELAYED_ACK(event,msg); break;
        default: opp_error("wrong event code");
    }

    // then state transitions
    return performStateTransition(event);
}

bool TCPConnection::processTCPSegment(IPInterfacePacket *wrappedTcpSeg)
{
    // unwrap TCP segment
    TCPSegment *tcpseg = (TCPSegment *) wrappedTcpSeg->decapsulate();
    if (localAddr.isNull())
    {
        localAddr = wrappedTcpSeg->destAddr();
        remoteAddr = wrappedTcpSeg->srcAddr();
    }
    else
    {
        ASSERT(localAddr==wrappedTcpSeg->destAddr());
        ASSERT(remoteAddr==wrappedTcpSeg->srcAddr());
    }
    delete wrappedTcpSeg;

    // first do actions
    TCPEvent event = analyseTCPSegmentEvent(tcpseg);
    switch (event)
    {
        case TCP_E_RCV_DATA: process_RCV_DATA(event, tcpseg); break;
        case TCP_E_RCV_SYN: process_RCV_SYN(event, tcpseg); break;
        case TCP_E_RCV_SYN_ACK: process_RCV_SYN_ACK(event, tcpseg); break;
        case TCP_E_RCV_FIN: process_RCV_FIN(event, tcpseg); break;
        case TCP_E_RCV_FIN_ACK: process_RCV_FIN_ACK(event, tcpseg); break;
        case TCP_E_RCV_RST: process_RCV_RST(event, tcpseg); break;
        default: opp_error("wrong event code");
    }

    // then state transitions
    return performStateTransition(event);
}

bool TCPConnection::processAppCommand(TCPInterfacePacket *tcpIfPacket)
{
    // first do actions
    TCPEvent event = analyseAppCommandEvent(tcpIfPacket);
    switch (event)
    {
        case TCP_E_OPEN_ACTIVE: process_OPEN_ACTIVE(event, tcpIfPacket); break;
        case TCP_E_OPEN_PASSIVE: process_OPEN_PASSIVE(event, tcpIfPacket); break;
        case TCP_E_SEND: process_SEND(event, tcpIfPacket); break;
        case TCP_E_RECEIVE: process_RECEIVE(event, tcpIfPacket); break;
        case TCP_E_CLOSE: process_CLOSE(event, tcpIfPacket); break;
        case TCP_E_ABORT: process_ABORT(event, tcpIfPacket); break;
        case TCP_E_STATUS: process_STATUS(event, tcpIfPacket); break;
        default: opp_error("wrong event code");
    }

    // then state transitions
    return performStateTransition(event);
}

void TCPConnection::sendToIP(TCPSegment *tcpseg)
{
    IPInterfacePacket *ipIfPacket = new IPInterfacePacket();
    ipIfPacket->encapsulate(tcpseg);
    ipIfPacket->setProtocol(IP_PROT_TCP);
    ipIfPacket->setSrcAddr(localAddr);
    ipIfPacket->setDestAddr(remoteAddr);

    module->send(tcpseg,"to_ip");
}

void TCPConnection::sendToApp(TCPInterfacePacket *tcpIfPacket)
{
    module->send(tcpIfPacket,"to_appl");
}

//==========
// State transition processing

TCPEvent TCPConnection::analyseTCPSegmentEvent(TCPSegment *tcpseg)
{
    if (tcpseg->rstBit())
    {
        return TCP_E_RCV_RST;
    }
    else if (tcpseg->synBit())
    {
        if (!tcpseg->ackBit())
            return TCP_E_RCV_SYN;
        else
            return TCP_E_RCV_SYN_ACK;
    }
    else if (tcpseg->finBit())
    {
        if (!tcpseg->ackBit())
            return TCP_E_RCV_FIN;
        else
            return TCP_E_RCV_FIN_ACK;
    }
    else
    {
        if (!tcpseg->ackBit())
            return TCP_E_RCV_DATA;
        else
            return TCP_E_RCV_ACK;
    }
}

TCPEvent TCPConnection::analyseTimerEvent(cMessage *msg)
{
    return (TCPEvent) msg->kind();
}

TCPEvent TCPConnection::analyseAppCommandEvent(TCPInterfacePacket *tcpIfPacket)
{
    switch (tcpIfPacket->kind())
    {
        case TCP_C_OPEN_ACTIVE:  return TCP_E_OPEN_ACTIVE;
        case TCP_C_OPEN_PASSIVE: return TCP_E_OPEN_PASSIVE;
        case TCP_C_SEND:         return TCP_E_SEND;
        case TCP_C_RECEIVE:      return TCP_E_RECEIVE;
        case TCP_C_CLOSE:        return TCP_E_CLOSE;
        case TCP_C_ABORT:        return TCP_E_ABORT;
        case TCP_C_STATUS:       return TCP_E_STATUS;
        default: opp_error("Unknown message kind in app command (%s)%s",
                           tcpIfPacket->className(),tcpIfPacket->name());
                 return (TCPEvent)0; // to satisfy compiler
    }
}

bool TCPConnection::performStateTransition(const TCPEvent& event)
{
    switch (fsm.state())
    {
        case FSM_Exit(TCP_S_INIT):
            switch (event)
            {
                case TCP_E_OPEN_PASSIVE:FSM_Goto(fsm, TCP_S_LISTEN); break;
                case TCP_E_OPEN_ACTIVE: FSM_Goto(fsm, TCP_S_SYN_SENT); break;
            }
            break;

        case FSM_Exit(TCP_S_LISTEN):
            switch (event)
            {
                case TCP_E_OPEN_ACTIVE: FSM_Goto(fsm, TCP_S_SYN_SENT); break;
                case TCP_E_SEND:        FSM_Goto(fsm, TCP_S_SYN_SENT); break;
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_SYN:     FSM_Goto(fsm, TCP_S_SYN_RCVD);break;
            }
            break;

        case FSM_Exit(TCP_S_SYN_RCVD):
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_FIN_WAIT_1); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_CONN_ESTAB: FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_RST:     FSM_Goto(fsm, TCP_S_LISTEN); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_ESTABLISHED); break;
                // case TCP_E_RCV_FIN:  FSM_Goto(fsm, TCP_S_CLOSE_WAIT); break; -- not on diagrams
            }
            break;

        case FSM_Exit(TCP_S_SYN_SENT):
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_CONN_ESTAB: FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_SYN_ACK: FSM_Goto(fsm, TCP_S_ESTABLISHED); break;
                case TCP_E_RCV_SYN:     FSM_Goto(fsm, TCP_S_SYN_RCVD); break;
            }
            break;

        case FSM_Exit(TCP_S_ESTABLISHED):
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_FIN_WAIT_1); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSE_WAIT); break;
            }
            break;

        case FSM_Exit(TCP_S_CLOSE_WAIT):
            switch (event)
            {
                case TCP_E_CLOSE:       FSM_Goto(fsm, TCP_S_LAST_ACK); break;
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
            }
            break;

        case FSM_Exit(TCP_S_LAST_ACK):
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_CLOSED); break;
            }
            break;

        case FSM_Exit(TCP_S_FIN_WAIT_1):
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSING); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_FIN_WAIT_2); break;
                case TCP_E_RCV_FIN_ACK: FSM_Goto(fsm, TCP_S_TIME_WAIT);
            }
            break;

        case FSM_Exit(TCP_S_FIN_WAIT_2):
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_TIME_WAIT); break;
                case TCP_E_TIMEOUT_FIN_WAIT_2: FSM_Goto(fsm, TCP_S_CLOSED); break;
            }
            break;

        case FSM_Exit(TCP_S_CLOSING):
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_RCV_ACK:     FSM_Goto(fsm, TCP_S_TIME_WAIT); break;
            }
            break;

        case FSM_Exit(TCP_S_TIME_WAIT):
            switch (event)
            {
                case TCP_E_ABORT:       FSM_Goto(fsm, TCP_S_CLOSED); break;
                case TCP_E_TIMEOUT_TIME_WAIT: FSM_Goto(fsm, TCP_S_CLOSED); break;
            }
            break;

        case FSM_Exit(TCP_S_CLOSED):
            break;
    }
    return fsm.state() == TCP_S_CLOSED;
}

//=========================================
// Processing events

void TCPConnection::process_OPEN_ACTIVE(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpIfPacket);
    switch(fsm.state())
    {
        case TCP_S_INIT:             break;
        case TCP_S_LISTEN:           break;
        case TCP_S_SYN_RCVD:         break;
        case TCP_S_SYN_SENT:         break;
        case TCP_S_ESTABLISHED:      break;
        case TCP_S_CLOSE_WAIT:       break;
        case TCP_S_LAST_ACK:         break;
        case TCP_S_FIN_WAIT_1:       break;
        case TCP_S_FIN_WAIT_2:       break;
        case TCP_S_CLOSING:          break;
        case TCP_S_TIME_WAIT:        break;
        case TCP_S_CLOSED:           break;
    }
}

void TCPConnection::process_OPEN_PASSIVE(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpIfPacket);
    switch(fsm.state())
    {
        case TCP_S_INIT:
        {
            // store socket parameters
            remotePort = openCmd->getLocalPort();

            // create send/receive queues
            const char *sendQueueClass = openCmd->getSendQueueClass();
            if (!sendQueueClass || !sendQueueClass[0])
                sendQueueClass = module->par("defaultSendQueueClass");
            const char *receiveQueueClass = openCmd->getReceiveQueueClass();
            if (!receiveQueueClass || !receiveQueueClass[0])
                receiveQueueClass = module->par("defaultReceiveQueueClass");
            sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));
            receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));
            break;
        }
        case TCP_S_LISTEN:           break;
        case TCP_S_SYN_RCVD:         break;
        case TCP_S_SYN_SENT:         break;
        case TCP_S_ESTABLISHED:      break;
        case TCP_S_CLOSE_WAIT:       break;
        case TCP_S_LAST_ACK:         break;
        case TCP_S_FIN_WAIT_1:       break;
        case TCP_S_FIN_WAIT_2:       break;
        case TCP_S_CLOSING:          break;
        case TCP_S_TIME_WAIT:        break;
        case TCP_S_CLOSED:           break;
    }
}

void TCPConnection::process_SEND(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
}

void TCPConnection::process_RECEIVE(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
}

void TCPConnection::process_CLOSE(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
}

void TCPConnection::process_ABORT(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
}

void TCPConnection::process_STATUS(TCPEvent event, TCPInterfacePacket *tcpIfPacket)
{
}


void TCPConnection::process_RCV_DATA(TCPEvent event, TCPSegment *tcpseg)
{
}

void TCPConnection::process_RCV_SYN(TCPEvent event, TCPSegment *tcpseg)
{
}

void TCPConnection::process_RCV_SYN_ACK(TCPEvent event, TCPSegment *tcpseg)
{
}

void TCPConnection::process_RCV_FIN(TCPEvent event, TCPSegment *tcpseg)
{
}

void TCPConnection::process_RCV_FIN_ACK(TCPEvent event, TCPSegment *tcpseg)
{
}

void TCPConnection::process_RCV_RST(TCPEvent event, TCPSegment *tcpseg)
{
}


void TCPConnection::process_TIMEOUT_TIME_WAIT(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_REXMT(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_PERSIST(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_KEEPALIVE(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_CONN_ESTAB(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_FIN_WAIT_2(TCPEvent event, cMessage *msg)
{
}

void TCPConnection::process_TIMEOUT_DELAYED_ACK(TCPEvent event, cMessage *msg)
{
}


