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
#include "TCPSegment.h"
#include "TCPCommand_m.h"
#include "IPControlInfo_m.h"
#include "TCPSendQueue.h"
#include "TCPReceiveQueue.h"
#include "TCPAlgorithm.h"


//
// helper functions
//

const char *TCPConnection::stateName(int state)
{
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (state)
    {
        CASE(TCP_S_INIT);
        CASE(TCP_S_CLOSED);
        CASE(TCP_S_LISTEN);
        CASE(TCP_S_SYN_SENT);
        CASE(TCP_S_SYN_RCVD);
        CASE(TCP_S_ESTABLISHED);
        CASE(TCP_S_CLOSE_WAIT);
        CASE(TCP_S_LAST_ACK);
        CASE(TCP_S_FIN_WAIT_1);
        CASE(TCP_S_FIN_WAIT_2);
        CASE(TCP_S_CLOSING);
        CASE(TCP_S_TIME_WAIT);
    }
    return s;
#undef CASE
}

const char *TCPConnection::eventName(int event)
{
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (event)
    {
        CASE(TCP_E_IGNORE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
        CASE(TCP_E_SEND);
        CASE(TCP_E_CLOSE);
        CASE(TCP_E_ABORT);
        CASE(TCP_E_STATUS);
        CASE(TCP_E_RCV_DATA);
        CASE(TCP_E_RCV_ACK);
        CASE(TCP_E_RCV_SYN);
        CASE(TCP_E_RCV_SYN_ACK);
        CASE(TCP_E_RCV_FIN);
        CASE(TCP_E_RCV_FIN_ACK);
        CASE(TCP_E_RCV_RST);
        CASE(TCP_E_RCV_UNEXP_SYN);
        CASE(TCP_E_TIMEOUT_2MSL);
        CASE(TCP_E_TIMEOUT_CONN_ESTAB);
        CASE(TCP_E_TIMEOUT_FIN_WAIT_2);
    }
    return s;
#undef CASE
}

const char *TCPConnection::indicationName(int code)
{
#define CASE(x) case x: s=#x+6; break
    const char *s = "unknown";
    switch (code)
    {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
        CASE(TCP_I_ESTABLISHED);
        CASE(TCP_I_PEER_CLOSED);
        CASE(TCP_I_CLOSED);
        CASE(TCP_I_CONNECTION_REFUSED);
        CASE(TCP_I_CONNECTION_RESET);
        CASE(TCP_I_TIMED_OUT);
        CASE(TCP_I_STATUS);
    }
    return s;
#undef CASE
}

void TCPConnection::printConnBrief()
{
    tcpEV << "Connection " << this << " ";
    tcpEV << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort;
    tcpEV << "  on app[" << appGateIndex << "],connId=" << connId;
    tcpEV << "  in " << stateName(fsm.state()) << "\n";
}

void TCPConnection::printSegmentBrief(TCPSegment *tcpseg)
{
    tcpEV << "." << tcpseg->srcPort() << " > ";
    tcpEV << "." << tcpseg->destPort() << ": ";

    if (tcpseg->synBit())  tcpEV << (tcpseg->ackBit() ? "SYN+ACK " : "SYN ");
    if (tcpseg->finBit())  tcpEV << "FIN(+ACK) ";
    if (tcpseg->rstBit())  tcpEV << (tcpseg->ackBit() ? "RST+ACK " : "RST ");
    if (tcpseg->pshBit())  tcpEV << "PSH ";

    if (tcpseg->payloadLength()>0 || tcpseg->synBit())
    {
        tcpEV << tcpseg->sequenceNo() << ":" << tcpseg->sequenceNo()+tcpseg->payloadLength();
        tcpEV << "(" << tcpseg->payloadLength() << ") ";
    }
    if (tcpseg->ackBit())  tcpEV << "ack " << tcpseg->ackNo() << " ";
    tcpEV << "win " << tcpseg->window() << "\n";
    if (tcpseg->urgBit())  tcpEV << "urg " << tcpseg->urgentPointer() << " ";
}

TCPConnection *TCPConnection::cloneListeningConnection()
{
    TCPConnection *conn = new TCPConnection(tcpMain,appGateIndex,connId);

    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = sendQueue->className();
    conn->sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));

    const char *receiveQueueClass = receiveQueue->className();
    conn->receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));

    const char *tcpAlgorithmClass = tcpAlgorithm->className();
    conn->tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
    conn->tcpAlgorithm->setConnection(conn);
    conn->tcpAlgorithm->initialize();

    conn->state = conn->tcpAlgorithm->createStateVariables();

    // put it into LISTEN, with our localAddr/localPort
    conn->state->active = false;
    conn->state->fork = true;
    conn->localAddr = localAddr;
    conn->localPort = localPort;
    FSM_Goto(conn->fsm, TCP_S_LISTEN);

    return conn;
}

void TCPConnection::sendToIP(TCPSegment *tcpseg)
{
    // final touches on the segment before sending
    tcpseg->setSrcPort(localPort);
    tcpseg->setDestPort(remotePort);
    tcpseg->setLength(8*(TCP_HEADER_OCTETS+tcpseg->payloadLength()));
    // TBD account for Options (once they get implemented)

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(IP_PROT_TCP);
    controlInfo->setSrcAddr(localAddr);
    controlInfo->setDestAddr(remoteAddr);
    tcpseg->setControlInfo(controlInfo);

    tcpEV << "Send: ";
    printSegmentBrief(tcpseg);

    tcpMain->send(tcpseg,"to_ip");
}

void TCPConnection::sendToIP(TCPSegment *tcpseg, IPAddress src, IPAddress dest)
{
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(IP_PROT_TCP);
    controlInfo->setSrcAddr(src);
    controlInfo->setDestAddr(dest);
    tcpseg->setControlInfo(controlInfo);

    tcpEV << "Send: ";
    printSegmentBrief(tcpseg);

    check_and_cast<TCPMain *>(simulation.contextModule())->send(tcpseg,"to_ip");
}

void TCPConnection::signalConnectionTimeout()
{
    sendIndicationToApp(TCP_I_TIMED_OUT);
}

void TCPConnection::sendIndicationToApp(int code)
{
    tcpEV << "Notifying app: " << indicationName(code) << "\n";
    cMessage *msg = new cMessage(indicationName(code));
    msg->setKind(code);
    TCPCommand *ind = new TCPCommand();
    ind->setConnId(connId);
    msg->setControlInfo(ind);
    tcpMain->send(msg, "to_appl", appGateIndex);
}

void TCPConnection::sendEstabIndicationToApp()
{
    tcpEV << "Notifying app: " << indicationName(TCP_I_ESTABLISHED) << "\n";
    cMessage *msg = new cMessage(indicationName(TCP_I_ESTABLISHED));
    msg->setKind(TCP_I_ESTABLISHED);

    TCPConnectInfo *ind = new TCPConnectInfo();
    ind->setConnId(connId);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);

    msg->setControlInfo(ind);
    tcpMain->send(msg, "to_appl", appGateIndex);
}

void TCPConnection::sendToApp(cMessage *msg)
{
    tcpMain->send(msg, "to_appl", appGateIndex);
}

void TCPConnection::initConnection(TCPOpenCommand *openCmd)
{
    // create send/receive queues
    const char *sendQueueClass = openCmd->sendQueueClass();
    if (!sendQueueClass || !sendQueueClass[0])
        sendQueueClass = tcpMain->par("sendQueueClass");
    sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));

    const char *receiveQueueClass = openCmd->receiveQueueClass();
    if (!receiveQueueClass || !receiveQueueClass[0])
        receiveQueueClass = tcpMain->par("receiveQueueClass");
    receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->tcpAlgorithmClass();
    if (!tcpAlgorithmClass || !tcpAlgorithmClass[0])
        tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");
    tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
    tcpAlgorithm->setConnection(this);
    tcpAlgorithm->initialize();

    // create state block
    state = tcpAlgorithm->createStateVariables();
}

void TCPConnection::selectInitialSeqNum()
{
    // set the initial send sequence number
    state->iss = (unsigned long)(fmod(tcpMain->simTime()*250000.0, 1.0+(double)(unsigned)0xffffffffUL)) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;

    sendQueue->init(state->iss+1); // +1 is for SYN
}

bool TCPConnection::isSegmentAcceptable(TCPSegment *tcpseg)
{
    // segment entirely falls in receive window
    //FIXME probably not this simple, see old code segAccept() below...
    return seqGE(tcpseg->sequenceNo(),state->rcv_nxt) &&
           seqLE(tcpseg->sequenceNo()+tcpseg->payloadLength(),state->rcv_nxt+state->rcv_wnd);
}

void TCPConnection::sendSyn()
{
    if (remoteAddr.isNull() || remotePort==-1)
        opp_error("Error processing command OPEN_ACTIVE: foreign socket unspecified");
    if (localPort==-1)
        opp_error("Error processing command OPEN_ACTIVE: local port unspecified");

    // create segment
    TCPSegment *tcpseg = new TCPSegment("SYN");
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setSynBit(true);
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss+1;

    // send it
    sendToIP(tcpseg);
}

void TCPConnection::sendSynAck()
{
    // create segment
    TCPSegment *tcpseg = new TCPSegment("SYN+ACK");
    tcpseg->setSequenceNo(state->iss);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSynBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setWindow(state->rcv_wnd);

    state->snd_max = state->snd_nxt = state->iss+1;

    // send it
    sendToIP(tcpseg);
}

void TCPConnection::sendRst(uint32 seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TCPConnection::sendRst(uint32 seq, IPAddress src, IPAddress dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = new TCPSegment("RST");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setSequenceNo(seq);

    // send it
    sendToIP(tcpseg, src, dest);
}

void TCPConnection::sendRstAck(uint32 seq, uint32 ack, IPAddress src, IPAddress dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = new TCPSegment("RST+ACK");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(seq);
    tcpseg->setAckNo(ack);

    // send it
    sendToIP(tcpseg, src, dest);
}

void TCPConnection::sendAck()
{
    TCPSegment *tcpseg = new TCPSegment("ACK");

    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setWindow(state->rcv_wnd);

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

void TCPConnection::sendFin()
{
    TCPSegment *tcpseg = new TCPSegment("FIN");

    // Note: ACK bit *must* be set for both FIN and FIN+ACK. What makes
    // the difference for FIN+ACK is that its ackNo acks the remote TCP's FIN.
    tcpseg->setFinBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setSequenceNo(state->snd_nxt);
    tcpseg->setWindow(state->rcv_wnd);

    // send it
    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}

bool TCPConnection::sendData(bool fullSegments, int maxNumBytes)
{
    // start sending from snd_max
    state->snd_nxt = state->snd_max;

    // check how much we can sent
    ulong buffered = sendQueue->bytesAvailable(state->snd_nxt);
    if (buffered==0)
        return false;
    ulong win = state->snd_una + state->snd_wnd - state->snd_nxt;
    if (maxNumBytes!=-1 && (ulong)maxNumBytes<win)
        win = maxNumBytes;
    if (win>buffered)
        win = buffered;
    if (win==0)
    {
        tcpEV << (maxNumBytes==0 ? "Cannot send, congestion window closed\n" : "Cannot send, send window closed (snd_una+snd_wnd-snd_max=0)\n");
        return false;
    }
    if (fullSegments && win<state->snd_mss)
    {
        tcpEV << "Cannot send, don't have a full segment (snd_mss=" << state->snd_mss << ")\n";
        return false;
    }

    // start sending 'win' bytes
    uint32 old_snd_nxt = state->snd_nxt;
    ASSERT(win>0);
    while (win>0)
    {
        ulong bytes = Min(win, state->snd_mss);
        TCPSegment *tcpseg = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);
        tcpseg->setAckNo(state->rcv_nxt);
        tcpseg->setAckBit(true);
        tcpseg->setWindow(state->rcv_wnd);
        // TBD when to set PSH bit?
        // TBD set URG bit if needed
        ASSERT(bytes==(ulong)tcpseg->payloadLength());

        win -= bytes;
        state->snd_nxt += bytes;
        state->snd_wnd -= bytes;

        if (state->send_fin && state->snd_nxt==state->snd_fin_seq)
        {
            tcpEV << "Setting FIN on segment and advancing snd_nxt over FIN\n";
            tcpseg->setFinBit(true);
            state->snd_nxt = state->snd_fin_seq+1;
        }

        sendToIP(tcpseg);
    }

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    state->snd_max = state->snd_nxt;

    // notify (once is enough)
    tcpAlgorithm->ackSent();
    tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}


void TCPConnection::retransmitData()
{
    // retransmit one segment at snd_una, and set snd_nxt accordingly
    state->snd_nxt = state->snd_una;

    ulong bytes = Min(state->snd_mss, state->snd_max - state->snd_nxt);
    ASSERT(bytes!=0);

    TCPSegment *tcpseg = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);
    tcpseg->setAckNo(state->rcv_nxt);
    tcpseg->setAckBit(true);
    tcpseg->setWindow(state->rcv_wnd);
    // TBD when to set PSH bit?
    // TBD set URG bit if needed
    ASSERT(bytes==(ulong)tcpseg->payloadLength());

    state->snd_nxt += bytes;

    if (state->send_fin && state->snd_nxt==state->snd_fin_seq)
    {
        tcpEV << "Setting FIN on retransmitted segment\n";
        tcpseg->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq+1;
    }

    sendToIP(tcpseg);

    // notify
    tcpAlgorithm->ackSent();
}


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
  if (seqLE(tcb_block->rcv_nxt, tcb_block->seg_seq) && seqLess(tcb_block->seg_seq, rcv_wnd_nxt))
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
      if (seqLE(tcb_block->rcv_nxt, seg_end) && seqLess(seg_end, rcv_wnd_nxt))
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


