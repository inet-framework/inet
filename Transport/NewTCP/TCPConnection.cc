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
#include "TCPAlgorithm.h"


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

    // queues and algorithm will be created on active or passive open
    sendQueue = NULL;
    receiveQueue = NULL;
    tcpAlgorithm = NULL;
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
        case TCP_E_TIMEOUT_TIME_WAIT: process_TIMEOUT_TIME_WAIT(msg); break;
        case TCP_E_TIMEOUT_REXMT: process_TIMEOUT_REXMT(msg); break;
        case TCP_E_TIMEOUT_PERSIST: process_TIMEOUT_PERSIST(msg); break;
        case TCP_E_TIMEOUT_KEEPALIVE: process_TIMEOUT_KEEPALIVE(msg); break;
        case TCP_E_TIMEOUT_CONN_ESTAB: process_TIMEOUT_CONN_ESTAB(msg); break;
        case TCP_E_TIMEOUT_FIN_WAIT_2: process_TIMEOUT_FIN_WAIT_2(msg); break;
        case TCP_E_TIMEOUT_DELAYED_ACK: process_TIMEOUT_DELAYED_ACK(msg); break;
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
        case TCP_E_RCV_DATA: process_RCV_DATA( tcpseg); break;
        case TCP_E_RCV_SYN: process_RCV_SYN( tcpseg); break;
        case TCP_E_RCV_SYN_ACK: process_RCV_SYN_ACK( tcpseg); break;
        case TCP_E_RCV_FIN: process_RCV_FIN( tcpseg); break;
        case TCP_E_RCV_FIN_ACK: process_RCV_FIN_ACK( tcpseg); break;
        case TCP_E_RCV_RST: process_RCV_RST( tcpseg); break;
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
        case TCP_E_OPEN_ACTIVE: process_OPEN_ACTIVE( tcpIfPacket); break;
        case TCP_E_OPEN_PASSIVE: process_OPEN_PASSIVE( tcpIfPacket); break;
        case TCP_E_SEND: process_SEND( tcpIfPacket); break;
        case TCP_E_RECEIVE: process_RECEIVE( tcpIfPacket); break;
        case TCP_E_CLOSE: process_CLOSE( tcpIfPacket); break;
        case TCP_E_ABORT: process_ABORT( tcpIfPacket); break;
        case TCP_E_STATUS: process_STATUS( tcpIfPacket); break;
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
                case TCP_E_RCV_FIN:     FSM_Goto(fsm, TCP_S_CLOSE_WAIT); break;
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
/*
bool TCPConnection::segAccept(TCPSegment *tcpseg)
{
  //first octet outside the recive window
  unsigned long rcv_wnd_nxt = tcb_block->rcv_nxt + tcb_block->rcv_wnd;
  //seq. number of the last octet of the incoming segment
  unsigned long seg_end;

  //get the received segment
  cMessage*  seg        = tcb_block->st_event.pmsg;
  //get header of the segment
  TcpHeader* tcp_header = (TcpHeader*) (seg->par("tcpheader").pointerValue());
  //set sequence number
  tcb_block->seg_seq    = tcp_header->th_seq_no;
  //get segment length (counting SYN, FIN)
  tcb_block->seg_len    = seg->par("seg_len");

  //if segment has bit errors it is not acceptable.
  //TCP does this with checksum, we check the hasBitError() member
  if (seg->hasBitError() == true)
    {
      if (debug) ev << "Incoming segment has bit errors. Ignoring the segment.\n";
      return 0;
    }

  //if SEG.SEQ inside receive window ==> seg. acceptable
  if (seqNoLeq(tcb_block->rcv_nxt, tcb_block->seg_seq) && seqNoLt(tcb_block->seg_seq, rcv_wnd_nxt))
    {
      if (debug) ev << "Incoming segment acceptable.\n";
      return 1;
    }
  //if segment length equals 0
  if (tcb_block->seg_len == 0)
    {
      //if RCV.WND = 0: seg. acceptable <=> SEG.SEQ = RCV.NXT
      if (tcb_block->rcv_wnd == 0 && tcb_block->seg_seq == tcb_block->rcv_nxt)
        {
          if (debug) ev << "Incoming segment acceptable.\n";
          return 1;
        }
    }
  //if segment length not equal to 0
  else
    {
      seg_end = tcb_block->seg_seq + tcb_block->seg_len - 1;
      if (seqNoLeq(tcb_block->rcv_nxt, seg_end) && seqNoLt(seg_end, rcv_wnd_nxt))
        {
          if (debug) ev << "Incoming segment acceptable.\n";
          return 1;
        }
      //if only 1 octet in segment
      if (tcb_block->seg_len == 1 && tcb_block->rcv_nxt == tcb_block->seg_seq)
        {
          if (debug) ev << "Incoming segment acceptable.\n";
          return 1;
        }
    }

  //if segment is not acceptable, send an ACK in reply (unless RST is set),
  //drop the segment
  if (debug) ev << "Incoming segment not acceptable.\n";
  if (debug) ev << "The receive window is " << tcb_block->rcv_nxt << " to " << rcv_wnd_nxt << endl;
  if (debug) ev << "and the sequence number is " << tcb_block->seg_seq << endl;
  ackSchedule(tcb_block, true);

  if (tcb_block->rcv_wnd == 0 && tcb_block->seg_seq == tcb_block->rcv_nxt)
    {
      if (debug) ev << "The receive window is zero.\n";
      if (debug) ev << "Processing control information (RST, SYN, ACK) nevertheless.\n";
      if (checkRst(tcb_block))
        {
          if (checkSyn(tcb_block))
            {
              checkAck(tcb_block);
            }
        }
    }

  return 0;
}
*/

void TCPConnection::initConnection(TCPOpenCommand *openCmd)
{
    // create send/receive queues
    const char *sendQueueClass = openCmd->getSendQueueClass();
    if (!sendQueueClass || !sendQueueClass[0])
        sendQueueClass = module->par("sendQueueClass");
    sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));

    const char *receiveQueueClass = openCmd->getReceiveQueueClass();
    if (!receiveQueueClass || !receiveQueueClass[0])
        receiveQueueClass = module->par("receiveQueueClass");
    receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();
    if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
        tcpAlgorithmClass = module->par("tcpAlgorithmClass");
    tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
}

void TCPConnection::sendSyn()
{
    if (remoteAddr.isNull() || remotePort==-1)
        opp_error("Error processing command OPEN_ACTIVE: foreign socket unspecified");
    if (localPort==-1)
        opp_error("Error processing command OPEN_ACTIVE: local port unspecified");

    //...
}


//=========================================
// Processing events

void TCPConnection::process_OPEN_ACTIVE(TCPInterfacePacket *tcpIfPacket)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpIfPacket);
    switch(fsm.state())
    {
        case TCP_S_INIT:
            initConnection(openCmd);
            /*no break*/

        case TCP_S_LISTEN:
            // store local/remote socket
            state.active = true;
            remoteAddr = openCmd->getRemoteAddr();
            localAddr = openCmd->getLocalAddr();
            remotePort = openCmd->getRemotePort();
            localPort = openCmd->getLocalPort();

            // send initial SYN
            sendSyn();
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command OPEN_ACTIVE: connection already exists");
        case TCP_S_CLOSED: // FIXME remove?
            break;
    }
}

void TCPConnection::process_OPEN_PASSIVE(TCPInterfacePacket *tcpIfPacket)
{
    TCPOpenCommand *openCmd = check_and_cast<TCPOpenCommand *>(tcpIfPacket);
    switch(fsm.state())
    {
        case TCP_S_INIT:
            initConnection(openCmd);
            /*no break*/

        case TCP_S_LISTEN:
            // store local/remote socket
            state.active = false;
            remoteAddr = openCmd->getRemoteAddr();
            localAddr = openCmd->getLocalAddr();
            remotePort = openCmd->getRemotePort();
            localPort = openCmd->getLocalPort();
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command OPEN_PASSIVE: connection already exists");
        case TCP_S_CLOSED: // FIXME remove?
            break;
    }
}

void TCPConnection::process_SEND(TCPInterfacePacket *tcpIfPacket)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command SEND: OPEN command not issued yet");

        case TCP_S_LISTEN:
            state.active = true;

            // send initial SYN
            sendSyn();
            break;

        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            sendQueue->enqueueAppData(tcpIfPacket);
            break;

        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
            tcpAlgorithm->process_SEND(tcpIfPacket);
            break;

        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command SEND: connection closing");
            break;
        case TCP_S_CLOSED: // FIXME remove?
            break;
    }
}

void TCPConnection::process_RECEIVE(TCPInterfacePacket *tcpIfPacket)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command SEND: OPEN command not issued yet");

        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            // FIXME queue up request
            break;

        case TCP_S_ESTABLISHED:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
            tcpAlgorithm->process_RECEIVE(tcpIfPacket);

        case TCP_S_CLOSE_WAIT:
            // FIXME serve if there're data in buffer otherwise error "conn closing"

        case TCP_S_LAST_ACK:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command RECEIVE: connection closing");
            break;
        case TCP_S_CLOSED: // FIXME remove?
            break;
    }
}

void TCPConnection::process_CLOSE(TCPInterfacePacket *tcpIfPacket)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command CLOSE: OPEN command not issued yet");

        case TCP_S_LISTEN:
            // FIXME refuse outstanding RECEIVEs with error "conn closing"
            break;

        case TCP_S_SYN_RCVD:
            // FIXME refuse outstanding SENDs and RECEIVEs with error "conn closing"
            break;

        case TCP_S_SYN_SENT:
            // FIXME process...
            break;

        case TCP_S_ESTABLISHED:
            // FIXME process...
            break;

        case TCP_S_CLOSE_WAIT:
            // FIXME process...
            break;

        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
            // FIXME process...
            break;

        case TCP_S_LAST_ACK:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
            opp_error("Error processing command CLOSE: connection closing");
            break;
        case TCP_S_CLOSED: // FIXME remove?
            break;
    }
}

void TCPConnection::process_ABORT(TCPInterfacePacket *tcpIfPacket)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
            opp_error("Error processing command CLOSE: OPEN command not issued yet");

        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_STATUS(TCPInterfacePacket *tcpIfPacket)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}


void TCPConnection::process_RCV_DATA(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_RCV_SYN(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_RCV_SYN_ACK(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_RCV_FIN(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_RCV_FIN_ACK(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_RCV_RST(TCPSegment *tcpseg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}


void TCPConnection::process_TIMEOUT_TIME_WAIT(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_REXMT(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_PERSIST(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_KEEPALIVE(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_CONN_ESTAB(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_FIN_WAIT_2(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}

void TCPConnection::process_TIMEOUT_DELAYED_ACK(cMessage *msg)
{
    switch(fsm.state())
    {
        case TCP_S_INIT:
        case TCP_S_LISTEN:
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
        case TCP_S_ESTABLISHED:
        case TCP_S_CLOSE_WAIT:
        case TCP_S_LAST_ACK:
        case TCP_S_FIN_WAIT_1:
        case TCP_S_FIN_WAIT_2:
        case TCP_S_CLOSING:
        case TCP_S_TIME_WAIT:
        case TCP_S_CLOSED:
            break;
    }
}


