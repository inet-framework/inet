//-----------------------------------------------------------------------------
// fileName: tcpmodule.cc
//
//  Implementation of the TCP-FSM (finite state machine) according to RFC 793;
//  slow start, congestion avoidance and fast retransmit like in 4.3BSD tahoe
//
//  Authors:
//    V. Boehm, June 15 1999
//    V. Kahmann, -> Sept 2000
//
//  Bugfixes, improvements (2003-):
//    Donald Liang (LYBD), Virginia Tech
//    Jeroen Idserda, University of Twente
//    Joung Woong Lee (zipizigi), University of Tsukuba
//    Andras Varga
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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

// Used Tags:
// FIXME: UNTESTED   Code/Function that has not been tested so far
// FIXME: MBI        Code/Function might be incorrect

#include <math.h>
#include <map>
#include <iostream>
#include <fstream>

using std::map;

// #define FSM_DEBUG       // To enable debugging in omnetpp.h

#include <omnetpp.h>
#include "tcp.h"


//duration of TIME-WAIT timer (twice maximum segment lifetime)
//set to 60 seconds (Stevens, Net/3)
//FIXME: MBI in some situations in our simulation fin could be delayed
const double TCP_2MSL = 60.0;

// maximum backoff shift for retransmissions
// if exceeded -> close connection
const int TCP_MAXRXTSHIFT = 12;

// Timeout values:
const double FINWAIT2_TIMEOUT_VAL = 600;
const double CONN_ESTAB_TIMEOUT_VAL = 75;


class TcpModule : public cSimpleModule
{
private:

  // TCP feature switches
  bool _feature_delayed_ack;   // delayed acknowledge
  bool _feature_fast_rt;       // fast retransmission
  bool _feature_fast_rc;       // fast recovery (Reno TCP)
  bool _feature_nr     ;       // NewReno

  // add more TCP features here

  bool debug; // initialized in initialize


  cOutVector *tcpdelay;
  cOutVector *cwnd_size;
  cOutVector *send_seq_no;
  cOutVector *rec_ack_no;
  cOutVector *rec_seq_no;

  // <Jeroen>
  int cnt_packet_delack;
  int seqnr_outorder_exp;
  // </Jeroen>

  //  cMessage *timeout_rexmt_msg;


  typedef map<SockPair, TcpTcb*> TcbList;
  TcbList tcb_list;
  typedef TcbList::const_iterator sockpair_iterator;

  //member functions
  MsgSource tcpArrivalMsg(cMessage* msg, SockPair& spair);
  void tcpTcpDelay(cMessage* msg);
  void snd_cwnd_size(unsigned long size);
  void seq_no_send(unsigned long sendnumber);
  void ack_no_rec(unsigned long recnumber);
  void seq_no_rec(unsigned long recnumber);

  TcpTcb* getTcb(cMessage* msg);
  TcpHeader* newTcpHeader(void);

  // function to analyze and manage the current event variables
  void anaEvent(TcpTcb* tcb_block, cMessage* msg, MsgSource source);

  int seqNoLt(unsigned long a, unsigned long b);
  int seqNoLeq(unsigned long a, unsigned long b);
  int seqNoGt(unsigned long a, unsigned long b);
  int seqNoGeq(unsigned long a, unsigned long b);

  // function to check the acceptance of a segment, returns 1 if acceptable,  0 otherwise
  int segAccept(TcpTcb* tcb_block);
  int checkRst(TcpTcb* tcb_block);
  int checkSyn(TcpTcb* tcb_block);
  int checkAck(TcpTcb* tcb_block);

  void calcRetransTimer(TcpTcb* tcb_block, short m);

  void timeoutPersistTimer(TcpTcb* tcb_block);
  void timeoutRetransTimer(TcpTcb* tcb_block);
  void timeoutDatalessAck(TcpTcb* tcb_block);

  void applCommandSend(cMessage* msg, TcpTcb* tcb_block);
  void applCommandReceive(cMessage* msg, TcpTcb* tcb_block);
  void segSend(cMessage* data, TcpTcb* tcb_block, unsigned long seq_no, TcpFlag fin, TcpFlag syn, TcpFlag rst,TcpFlag ack, TcpFlag urg, TcpFlag psh);
  void segReceive(cMessage* pseg, TcpHeader* pseg_tcp_header, TcpTcb* tcb_block, TcpFlag fin, TcpFlag syn, TcpFlag rst, TcpFlag ack, TcpFlag urg, TcpFlag psh);

  // function to decide from which queue to send data
  void sndDataProcess(TcpTcb* tcb_block);
  void retransQueueProcess(TcpTcb* tcb_block);
  void sndQueueProcess(TcpTcb* tcb_block);
  unsigned long sndDataSize(TcpTcb* tcb_block);
  unsigned long retransDataSize(TcpTcb* tcb_block, cQueue & retrans_queue);

  // function to process tcp_socket_queue/tcp_data_receive_queue
  void rcvQueueProcess(TcpTcb* tcb_block);
  void finSchedule(TcpTcb* tcb_block);
  void synSend(TcpTcb* tcb_block, TcpFlag fin, TcpFlag syn, TcpFlag rst, TcpFlag ack, TcpFlag urg, TcpFlag psh);
  void connAbort(cMessage* msg, TcpTcb* tcb_block);
  void connOpen(TcpTcb* tcb_block);
  void connCloseWait(TcpTcb* tcb_block);
  void connRcvdSyn(TcpTcb* tcb_block);
  void connClosed(TcpTcb* tcb_block);

  cMessage* removeFromDataQueue(cQueue & from_queue, unsigned long number_of_bits_to_remove);
  void transferQueues(cQueue &  from_queue, cQueue & to_queue, unsigned long number_of_octets_to_transfer);
  void copyQueues(cQueue &  from_queue, cQueue & to_queue, unsigned long number_of_octets_to_copy);
  void flushLabeledDataQueue(cQueue & queue_to_flush, unsigned long label, unsigned long number_of_bits_to_flush);
  void insertListElement(cLinkedList & ilist, SegRecord* ielement, unsigned long postion);
  void removeListElement(cLinkedList & list_to_inspect, unsigned long position);
  SegRecord* accessListElement(cLinkedList & list_to_inspect, unsigned long position);
  unsigned long numBitsInQueue(cQueue & queue_to_inspect);
  void insertAtQueueTail(cQueue & iqueue, cMessage* msg, unsigned long label);
  void flushQueue(cQueue & queue_to_flush, unsigned long number_of_octets_to_flush, bool tcp_seg_queue);

  void ackSchedule(TcpTcb* tcb_block, bool immediate);

  void outstRcv(TcpTcb* tcb_block);
  void outstSnd(TcpTcb* tcb_block);
  void flushTransRetransQ(TcpTcb* tcb_block);

  void printStatus(TcpTcb* tcb_block, const char *label);
  const char *stateName(TcpState state);
  const char *eventName(TcpEvent event);

  void procExInit(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExListen(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExSynRcvd(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExSynSent(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExEstablished(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExCloseWait(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExLastAck(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExFinWait1(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExFinWait2(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExClosing(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);
  void procExTimeWait(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header);

public:
  //stack size = 0 to indicate usage of handleMessage
  Module_Class_Members(TcpModule, cSimpleModule, 0);
  ~TcpModule();

  //virtual functions to be redefined
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  virtual void finish(); // added by LYBD
};

Define_Module(TcpModule);

void TcpModule::initialize()
{

   //fsm.setName("fsm");

  //parameters from ".ned"-file go here
  // ... = par("...");

  // set TCP features
  _feature_delayed_ack = par("TCPDelayedAck");
  _feature_fast_rt     = par("TCPFastRetrans");
  _feature_fast_rc     = par("TCPFastRecovery");
  _feature_nr          = par("TCPNewReno");

  //make sure that only one of the congestion control mechanisms is switched on
  if (_feature_nr)
    {
      _feature_fast_rc = false;
      _feature_fast_rt = false;
    }
  if (_feature_fast_rc)
    {
      _feature_fast_rt = false;
    }

  // add more TCP features here

  if (hasPar("debug")) {
          debug = par("debug");
  } else {
          debug = true; //-false;
  }

  // <Jeroen>
  cnt_packet_delack = 0;
  seqnr_outorder_exp = -1;
  // </Jeroen>

  tcpdelay = new cOutVector("TCP delay");
  cwnd_size = new cOutVector("Cwnd size");
  send_seq_no = new cOutVector("Send No");
  rec_ack_no = new cOutVector("Rec No");
  rec_seq_no = new cOutVector("Rec Seq No");

  // print out list of TCP features:
  if (debug) {
          ev << "TCP-Features of " << fullPath() << ":" << endl
             << "  Delayed Ack = " << (int)_feature_delayed_ack
             << "  Fast Retransmission = " << (int)_feature_fast_rt
             << "  Fast Recovery = " << (int)_feature_fast_rc
             << "  NewReno = " << (int)_feature_nr
            // add more TCP features here
             << endl
             << endl;
  }

  //int i = 0;
  //WATCH(i); //put all WATCHes in here
}

void TcpModule::finish()
{
}

TcpModule::~TcpModule()
{
// BCH LYBD
  // clear unused TCB or active connections
  TcbList::iterator iter = tcb_list.begin();
  while (iter != tcb_list.end())
  {
      TcpTcb *tcb_block =  (TcpTcb *) iter->second;
      while (tcb_block->tcp_rcv_rec_list.length() > 0) {
            SegRecord* seg_rec = (SegRecord *) tcb_block->tcp_rcv_rec_list.pop();
            delete seg_rec->pdata;
      }
      delete tcb_block;
      iter ++;
  }

  delete tcpdelay;
  delete cwnd_size;
  delete send_seq_no;
  delete rec_ack_no;
  delete rec_seq_no;
// ECH LYBD
}

void TcpModule::handleMessage(cMessage *msg)
{
  //definition of various variables
  //int istatus;
  //unsigned long rem_rcv_mss; (used for MSS option in procExListen(...), not implemented yet)
  //SegRecord* prcv_rec;
  //int ctr;

  //transmission control block (TCB)
  TcpTcb* tcb_block;
  bool delete_tcb = false; // flag to signal the deletion of tcb_block
  SockPair spair;

  TcpHeader* msg_tcp_header = NULL;

  //find out address and port info. and
  //if msg. is appl. call, arriving seg. or timeout
  MsgSource eventsource = tcpArrivalMsg(msg, spair);

  if (eventsource == FROM_IP)
    {
      msg_tcp_header = (TcpHeader*)(msg->par("tcpheader").pointerValue());

      //compute TCP-/TCP-delay
      tcpTcpDelay(msg);
    }

  //find TCB for current connection
  tcb_block = getTcb(msg);

  // LYBD: some timeout messages may arrive after TCB is deleted
  if (tcb_block == NULL) {
    delete msg;
    return;
  }

  //print TCB status information
  printStatus(tcb_block, "Connection state before event processing");

  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;


  //TCP finite state machine
  FSM_Switch(fsm)
    {
      //[FSM starting point] (= CLOSED EXIT)
    case FSM_Exit(TCP_S_INIT):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in INIT state
      procExInit(msg, tcb_block, msg_tcp_header);

      break;

      //[end point] TCP-FSM end point, starting point is INIT
    case FSM_Enter(TCP_S_CLOSED):

      if (debug) ev << "TCP in CLOSED.\n";

      //notify the application that TCP entered CLOSED
      connClosed(tcb_block);

      //print TCB status information
      printStatus(tcb_block, "Connection state after event processing");

      //connection closed: delete TCB from list
      if (debug) ev << "Deleting TCB from list.\n";
      tcb_list.erase(spair);

      // signal to delete tcb_block at end of handleMessage(). If it is done
      // now, the FSM macros will read freed memory
      delete_tcb = true;
      break;

      //[passive open]
    case FSM_Enter(TCP_S_LISTEN):

      if (debug) ev << "TCP in LISTEN.\n";

      //indicate a passive open
      tcb_block->passive = true;
      //copy address/port
      if (msg->arrivedOn("from_appl"))
        {
          tcb_block->passive_rem_addr = msg->par("dest_addr");
          tcb_block->passive_rem_port = msg->par("dest_port");
        }

      //handle ABORT
      if ((tcb_block->st_event.event == TCP_E_ABORT) || (tcb_block->st_event.event == TCP_E_ABORT_NO_RST))
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_LISTEN):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in LISTEN state
      procExListen(msg, tcb_block, msg_tcp_header);

      break;


    case FSM_Enter(TCP_S_SYN_RCVD):

      if (debug) ev << "TCP in SYN_RCVD.\n";

      if ((tcb_block->num_bit_req) == 0 && (eventsource != FROM_APPL))
        {
// BCH Andras -- code from UTS MPLS model
          if(msg->hasPar("num_bit_req"))
// ECH
              tcb_block->num_bit_req = msg->par("num_bit_req");
        }


      //notify the application that a SYN has been received
      // but only if we don't have a retransmission and we do not have a retransmitted
      // syn here
      // syn_rcvd is set to 0 on a rexmt
      if ((tcb_block->rxtshift <= 1) && (tcb_block->syn_rcvd == 1))
        connRcvdSyn(tcb_block);

      //handle ABORT
      if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          //create and send a data-less RST segment
          cMessage* rst_msg = new cMessage("RST_DATA", RST_DATA);
          rst_msg->setLength(0);
          TcpFlag rst = TCP_F_SET;
          segSend(rst_msg, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_SYN_RCVD):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in SYN_RCVD state
      procExSynRcvd(msg, tcb_block, msg_tcp_header);

      break;


      //[active open]
    case FSM_Enter(TCP_S_SYN_SENT):

      if (debug) ev << "TCP in SYN_SENT.\n";

      tcb_block->ack_send_time = simTime();

      //reset passive, since local and remote socket are specified
      tcb_block->passive = false;

      //check if sufficient socket information has been specified
      if ((tcb_block->local_port == -1) || (tcb_block->rem_port == -1) || (tcb_block->rem_addr == -1))
        {
          if (debug) ev << "Application issued ACTIVE OPEN or SEND with insufficient socket information. Aborting connection\n";
          tcb_block->st_event.event = TCP_E_ABORT;
        }

      if (tcb_block->st_event.event == TCP_E_ABORT || tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_SYN_SENT):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in SYN_SENT state
      procExSynSent(msg, tcb_block, msg_tcp_header);

      break;

      //[data transfer state]
    case FSM_Enter(TCP_S_ESTABLISHED):

      if (debug) ev << "TCP in ESTABLISHED.\n";

      //notify the application if the connection has just became established
      if (tcb_block->conn_estab == 0)
        {
          connOpen(tcb_block);
        }

      if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          //create and send a data-less RST segment
          cMessage* rst_msg = new cMessage("RST_DATA", RST_DATA);
          rst_msg->setLength(0);
          TcpFlag rst = TCP_F_SET;
          segSend(rst_msg, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else
        {
          //indicate that both ends of the connection are fully specified
          if (tcb_block->passive)
            {
              tcb_block->passive = false;
            }

          //process buffered and outgoing data
          rcvQueueProcess(tcb_block);
          sndDataProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_ESTABLISHED):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in ESTABLISHED state
      procExEstablished(msg, tcb_block, msg_tcp_header);

      break;

      //[passive close]
    case FSM_Enter(TCP_S_CLOSE_WAIT):

      if (debug) ev << "TCP in CLOSE_WAIT.\n";

      //notify the application that TCP entered CLOSE_WAIT
      connCloseWait(tcb_block);

      //notify the application if the connection has just became established
      if (tcb_block->conn_estab == 0)
        {
          connOpen(tcb_block);
        }

      if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          //create and send a data-less RST segment
          cMessage* rst_msg = new cMessage("RST_DATA", RST_DATA);
          rst_msg->setLength(0);
          TcpFlag rst = TCP_F_SET;
          segSend(rst_msg, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else
        {
          //process buffered and outgoing data
          rcvQueueProcess(tcb_block);
          sndDataProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_CLOSE_WAIT):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in CLOSE_WAIT state
      procExCloseWait(msg, tcb_block, msg_tcp_header);

      break;

      //[passive close]
    case FSM_Enter(TCP_S_LAST_ACK):

      if (debug) ev << "TCP in LAST_ACK.\n";

      if (tcb_block->st_event.event == TCP_E_ABORT || tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          connAbort(msg, tcb_block);
        }
      else
        {
          //process send queue
          sndDataProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_LAST_ACK):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in LAST_ACK state
      procExLastAck(msg, tcb_block, msg_tcp_header);

      break;

      //[active close]
    case FSM_Enter(TCP_S_FIN_WAIT_1):

      if (debug) ev << "TCP in FIN_WAIT_1.\n";

      if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          //create and send a data-less RST segment
          cMessage* rst_msg = new cMessage("RST_DATA", RST_DATA);
          rst_msg->setLength(0);
          TcpFlag rst = TCP_F_SET;
          segSend(rst_msg, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else
        {
          //indicate that both ends of the connection are fully specified
          if (tcb_block->passive)
            {
              tcb_block->passive = false;
            }

          //notify the application if the connection has just became established
          if (tcb_block->conn_estab == 0)
            {
              connOpen(tcb_block);
            }

          //process buffered and outgoing data
          rcvQueueProcess(tcb_block);
          sndDataProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_FIN_WAIT_1):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in FIN_WAIT_1 state
      procExFinWait1(msg, tcb_block, msg_tcp_header);

      break;

      //[active close]
    case FSM_Enter(TCP_S_FIN_WAIT_2):

      if (debug) ev << "TCP in FIN_WAIT_2.\n";

      // schedule the FIN_WAIT_2 timer.
      tcb_block->timeout_finwait2_msg = new cMessage("TIMEOUT_FIN_WAIT_2", TIMEOUT_FIN_WAIT_2);
      tcb_block->timeout_finwait2_msg->addPar("src_port")  = tcb_block->local_port;
      tcb_block->timeout_finwait2_msg->addPar("src_addr")  = tcb_block->local_addr;
      tcb_block->timeout_finwait2_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_finwait2_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + FINWAIT2_TIMEOUT_VAL, tcb_block->timeout_finwait2_msg);
      tcb_block->finwait2_sch = true;
      if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          //create and send a data-less RST segment
          cMessage* rst_msg = new cMessage("RST_DATA", RST_DATA);
          rst_msg->setLength(0);
          TcpFlag rst = TCP_F_SET;
          segSend(rst_msg, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else
        {
          //send received data to application layer
          rcvQueueProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_FIN_WAIT_2):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in ESTABLISHED state
      procExFinWait2(msg, tcb_block, msg_tcp_header);

      break;

      //[active close], [simultaneous close]
    case FSM_Enter(TCP_S_CLOSING):

      if (debug) ev << "TCP in CLOSING.\n";

      if (tcb_block->st_event.event == TCP_E_ABORT || tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          connAbort(msg, tcb_block);
        }
      else
        {
          //process send queue
          sndDataProcess(tcb_block);
        }

      break;

    case FSM_Exit(TCP_S_CLOSING):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in CLOSING sate
      procExClosing(msg, tcb_block, msg_tcp_header);

      break;

      //[active close]
    case FSM_Enter(TCP_S_TIME_WAIT):

      if (debug) ev << "TCP in TIME_WAIT.\n";

      //notify the application that the connection has been closed
      if (tcb_block->tcp_app_notified == false)
        {
          tcb_block->tcp_app_notified = true;

          if (tcb_block->st_event.event == TCP_E_ABORT || tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
            {
              //send ABORT to application process
              tcb_block->status_info = TCP_I_ABORTED;
              cMessage* abort_msg = new cMessage("TCP_I_ABORTED", TCP_I_ABORTED);

              //add connection ID parameter to abort_msg
              abort_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

              //add address and port information (note that they are switched for correct
              //address and port management at the application layer)
              abort_msg->addPar("src_port")  = tcb_block->rem_port;
              abort_msg->addPar("src_addr")  = tcb_block->rem_addr;
              abort_msg->addPar("dest_port") = tcb_block->local_port;
              abort_msg->addPar("dest_addr") = tcb_block->local_addr;

              if (debug) ev << "TCP notifies the application that the connection has been ABORTED.\n";
              send(abort_msg, "to_appl");
            }
          //else
          //{
          //  //send CLOSED notification to application process
          //  tcb_block->status_info = TCP_I_CLOSED;
          //  cMessage* closed_msg = new cMessage("TCP_I_CLOSED", TCP_I_CLOSED);
          //
          //  //add connection ID parameter to estab_msg
          //  closed_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;
          //
          //  //add address and port information (note that they are switched for correct
          //  //address and port management at the application layer)
          //  closed_msg->addPar("src_port") = tcb_block->rem_port;
          //  closed_msg->addPar("src_addr") = tcb_block->rem_addr;
          //  closed_msg->addPar("dest_port") = tcb_block->local_port;
          //  closed_msg->addPar("dest_addr") = tcb_block->local_addr;
          //
          //  if (debug) ev << "TCP notifies the application that the connection has been CLOSED.\n";
          //  send(closed_msg, "to_appl");
          //
          //  //reset connection establishment variable
          //  tcb_block->conn_estab = 0;
          //}
        }

      if (tcb_block->st_event.event == TCP_E_ABORT || tcb_block->st_event.event == TCP_E_ABORT_NO_RST)
        {
          //notify appl. of connection abort
          connAbort(msg, tcb_block);
        }
      else
        {
          // cancel rexmt, persist, keepalive and FIN-wait-2 timer
          //if (tcb_block->timeout_rexmt_msg->isScheduled())
          //  {
          //    if (debug) ev << "Cancel retransmission timer in TIME_WAIT state. \n";
          // delete cancelEvent(tcb_block->timeout_rexmt_msg);
          //  }
          //if (tcb_block->timeout_persist_msg->isScheduled())
          //  {
          //    if (debug) ev << "Cancel persist timer in TIME_WAIT state. \n";
          //delete cancelEvent(tcb_block->timeout_persist_msg);
          //  }
          //if (tcb_block->timeout_keepalive_msg->isScheduled())
          //  {
          //    if (debug) ev << "Cancel keepalive timer in TIME_WAIT state. \n";
          //delete cancelEvent(tcb_block->timeout_keepalive_msg);
          //  }
          if (tcb_block->finwait2_sch == true)
            {
              if (debug) ev << "Cancel FIN_WAIT_2 timer in TIME_WAIT state. \n";
              delete cancelEvent(tcb_block->timeout_finwait2_msg);
              tcb_block->finwait2_sch = false;
            }
          if (tcb_block->time_wait_sch == false)
            {
              //schedule the TIME-WAIT timer
              tcb_block->time_wait_sch = true;
              //create a timeout TIME-WAIT message
              cMessage* timeout_time_wait_msg = new cMessage("TIMEOUT_TIME_WAIT", TIMEOUT_TIME_WAIT);
              //add parameters used in function tcpArrivalMsg(...)
              //(no switching of addresses and ports here)
              timeout_time_wait_msg->addPar("src_port")  = tcb_block->local_port;
              timeout_time_wait_msg->addPar("src_addr")  = tcb_block->local_addr;
              timeout_time_wait_msg->addPar("dest_port") = tcb_block->rem_port;
              timeout_time_wait_msg->addPar("dest_addr") = tcb_block->rem_addr;
              scheduleAt(simTime() + TCP_2MSL, timeout_time_wait_msg);
            }
        }

      break;

    case FSM_Exit(TCP_S_TIME_WAIT):

      //set state, analyze event
      anaEvent(tcb_block, msg, eventsource);

      //processing in TIME_WAIT state
      procExTimeWait(msg, tcb_block, msg_tcp_header);

      break;
    } //end of FSM-switch

  if (delete_tcb == true)
    {
      if (debug) ev << "Deleting connection desctriptor.\n";
      delete tcb_block;
      tcb_block = NULL;
    }
  else
    {
      tcb_block->st_event.pmsg = NULL; // set pmsg to NULL because it points to msg

      //print TCB status information
      printStatus(tcb_block, "Connection state after event processing");
    }
  delete msg;
}


// function to determine whether incoming messages are coming from
// the application, ip module or if the are timeouts
// fills the AddPor data structure
MsgSource TcpModule::tcpArrivalMsg(cMessage* msg, SockPair& spair)
{
  MsgSource eventsource;
  //if origin of message is appl. layer use the following address format
  if (msg->arrivedOn("from_appl"))
    {
      //msg created and received at the same host => source and destination
      //parameters are not switched
      spair.local_port = msg->par("src_port");
      spair.rem_port   = msg->par("dest_port");
      spair.local_address = msg->par("src_addr");
      spair.rem_address = msg->par("dest_addr");
      eventsource      = FROM_APPL;
    }
  //if message was generated by TCP (segments etc.) use the following address format
  else if (msg->arrivedOn("from_ip"))
    {
      //note that source and destination parameters have to be switched
      TcpHeader* tcpheader = (TcpHeader*) (msg->par("tcpheader").pointerValue());
      spair.local_port    = tcpheader->th_dest_port;
      spair.rem_port      = tcpheader->th_src_port;
      spair.local_address = msg->par("dest_addr");
      spair.rem_address   = msg->par("src_addr");
      eventsource         = FROM_IP;
    }
  else if ((msg->kind() == TIMEOUT_TIME_WAIT) ||
           (msg->kind() == TIMEOUT_REXMT) ||
           (msg->kind() == TIMEOUT_PERSIST) ||
           (msg->kind() == TIMEOUT_KEEPALIVE) ||
           (msg->kind() == TIMEOUT_CONN_ESTAB) ||
           (msg->kind() == TIMEOUT_FIN_WAIT_2) ||
           (msg->kind() == TIMEOUT_DELAYED_ACK))  //add other timer msg kind values here
    {
      //msg created and received at the same host => source and destination
      //parameters are not switched
      spair.local_port = msg->par("src_port");
      spair.rem_port   = msg->par("dest_port");
      spair.local_address = msg->par("src_addr");
      spair.rem_address   = msg->par("dest_addr");
      eventsource      = FROM_TIMEOUT;
    }
  else
    error("Could not determine origin of message (forgot to add timeout?)\n");

  return eventsource;
}


// Function to compute the delay of a TCP segment between the
// two TCP modules of a connection
void TcpModule::tcpTcpDelay(cMessage* msg)
{
  double tcp_delay = simTime() - msg->timestamp();

  if (debug) {
          ev << "Received " << msg->name()
                 << " with a TCP-TCP-delay of " << tcp_delay << "sec.\n";
  }

  tcpdelay->record(tcp_delay);
}


//Function to save the current size of the congestion window to the output
//vector file after it has changed
void TcpModule::snd_cwnd_size(unsigned long size)
{
  double double_size = static_cast<double>(size);
  cwnd_size->record(double_size);
}


//Function to save the sequence number of a segment that has been sent
void TcpModule::seq_no_send(unsigned long sendnumber)
{
  double double_sendnumber = static_cast<double>(sendnumber);
  send_seq_no->record(double_sendnumber);
}


//Function to save the acknowledgement number of a segment that has been received
void TcpModule::ack_no_rec(unsigned long recnumber)
{
  double double_recnumber = static_cast<double>(recnumber);
  rec_ack_no->record(double_recnumber);
}

//Function to save the sequence number of a segment that has been received
void TcpModule::seq_no_rec(unsigned long recnumber)
{
  rec_seq_no->record(recnumber);
}

// Function to find TCB for current connection
// tries to find the TCB block specified by the socketpair of msg. There are
// three possible cases:
// 1. fully specified TCB-Block is found
// 2. unspecified TCB-Block is found -> substitute by fully specified
// 3. no one is found -> create one
TcpTcb* TcpModule::getTcb(cMessage* msg)
{
  //define TCB and socket pair
  TcpTcb*  tcb_block = NULL;
  SockPair spair;

  //address and port mangement function
  MsgSource eventsource = tcpArrivalMsg(msg, spair);

  //get or create a TCB holding connection state information
  sockpair_iterator search = tcb_list.find(spair);
  if (search != tcb_list.end())
    {
      //if fully specified socket pair is found in list
      tcb_block = search->second; //or tcb_list[spair];

      if (debug) ev << "Message/event belongs to connection tcp_conn_id="
                    << tcb_block->tb_conn_id << ".\n";
    }
  else
    {
      //if TCB not found when using fully specified socket pair, the
      //foreign socket might be unspecified due to a passive open

      //set socket pair with unspecified remote socket
      SockPair unspec_spair = spair;
      unspec_spair.rem_address = -1;     //remote addr. unspecified
      unspec_spair.rem_port = -1;        //remote port unspecified

      //search again for TCB
      sockpair_iterator search = tcb_list.find(unspec_spair);

      //if TCB not found now, create a new one
      if (search != tcb_list.end())
        {
          // if a TCB with unspecified remote socket has been found
          // we replace it with a fully specified TCB-Block.

          //get TCB found
          tcb_block = search->second;

          if (debug) ev << "Message/event belongs to connection tcp_conn_id="
                        << tcb_block->tb_conn_id << ", filling out remote addr/port in conn.\n";

          //erase old list element
          tcb_list.erase(unspec_spair);

          //fill in remote socket information
          tcb_block->rem_port = spair.rem_port;
          tcb_block->rem_addr = spair.rem_address;

          //insert TCB in tcb_list together with socket pair
          tcb_list.insert(TcbList::value_type(spair, tcb_block));

        }
      // LYBD: some timeout messages may arrive after TCB is deleted,
      // should not create new TCB for these timeout messages
      else if (eventsource != FROM_TIMEOUT)
        {
          if (debug) ev << "Creating new connection descriptor.\n";

          tcb_block = new TcpTcb;

          //assign names to TCP-queues
          tcb_block->tcp_send_queue.setName("TCP-Send-Queue");
          tcb_block->tcp_retrans_queue.setName("TCP-Retrans-Queue");
          tcb_block->tcp_data_receive_queue.setName("TCP-Data_Receive-Queue");
          tcb_block->tcp_socket_queue.setName("TCP-Socket-Queue");
          tcb_block->tcp_rcv_rec_list.setName("TCP-Rcv-Rec-List");

          //fill in tcb_block information;
          //-1 or 0 = information not yet available
          tcb_block->tb_conn_id    = -1;
          tcb_block->tb_prev_state = TCP_S_INIT;
          // no need to touch tcb_block->fsm -- it starts from TCP_S_INIT anyway

          tcb_block->st_event.event       = TCP_E_NONE;
          //pointer initialized in anaEvent(...)
          tcb_block->st_event.pmsg        = NULL;
          tcb_block->st_event.num_msg     = 0;
          tcb_block->st_event.th_flag_urg = TCP_F_NSET;
          tcb_block->st_event.th_flag_ack = TCP_F_NSET;
          tcb_block->st_event.th_flag_psh = TCP_F_NSET;
          tcb_block->st_event.th_flag_rst = TCP_F_NSET;
          tcb_block->st_event.th_flag_syn = TCP_F_NSET;
          tcb_block->st_event.th_flag_fin = TCP_F_NSET;

          tcb_block->status_info = TCP_I_NONE;

          tcb_block->local_port = -1;
          tcb_block->local_addr = -1;
          tcb_block->rem_port   = -1;
          tcb_block->rem_addr   = -1;

          tcb_block->timeout = -1; //msg->par("timeout");

          tcb_block->num_bit_req = 0;

          tcb_block->snd_una = 0;
          tcb_block->snd_nxt = 0;
          tcb_block->snd_up  = 0;
          tcb_block->snd_wnd = 0;
          tcb_block->snd_wl1 = 0;
          tcb_block->snd_wl2 = 0;
          tcb_block->iss     = 0;

          tcb_block->snd_cwnd = par("mss");     // 1 segment
          tcb_block->ssthresh = 65535;   // RFC 2001

          tcb_block->cwnd_cnt = 0;       // counter for setting cwnd when in congestion avoidance

          tcb_block->snd_fin_seq   = 0;
          tcb_block->snd_fin_valid = 0;
          tcb_block->snd_up_valid  = 0;
          tcb_block->snd_mss       = par("mss"); //maximum segment size (here set to the default value, no header options used to change it)

          tcb_block->rcv_nxt      = 0;
          //tcb_block->rcv_adv      = 0;
          //tcb_block->rcv_wnd    = 0; (see below (*))
          tcb_block->rcv_wnd_last = 0;
          tcb_block->rcv_up       = 0;
          tcb_block->irs          = 0;

          tcb_block->rcv_fin_seq   = 0;
          tcb_block->rcv_fin_valid = 0;
          tcb_block->rcv_up_valid  = 0;
          tcb_block->rcv_buf_seq   = 0;
          tcb_block->rcv_buff      = 65535; //540 //size of the receive queue in bytes
          tcb_block->rcv_wnd       = tcb_block->rcv_buff; //initial receive window is the full buffer size (*)
          tcb_block->rcv_buf_usage_thresh = 0.5;

          tcb_block->snd_max = 0;
          tcb_block->savenext = 0;
          tcb_block->dupacks = 0;

          tcb_block->max_retrans_seq = 0;

          tcb_block->seg_len = 0;
          tcb_block->seg_seq = 0;
          tcb_block->seg_ack = 0;

          tcb_block->srtt   = 0;   // smoothed rtt init 0 s
          tcb_block->rttvar = 24;  // smoothed mean dev. init 3 s
          tcb_block->rxtcur = 12;  // current rto init = 6 s
          tcb_block->rttmin = 2;   // min. value for rto = 1 s
          tcb_block->last_timed_data = -1; // starting time for last timed data used for rtt calculation
          tcb_block->rxtshift = 1;

          tcb_block->max_del_ack = 0.2; //max. ack. delay set to 0 (no ack. timer implemented yet)
          tcb_block->ack_sch     = false;

          tcb_block->per_sch     = false;

          tcb_block->num_pks_req = 0;

          tcb_block->last_snd_time = -1;

          tcb_block->tcp_app_notified = false;

          tcb_block->ack_send_time = -1;
          tcb_block->ack_rcv_time  = -1;

          tcb_block->finwait2_sch = false;
          tcb_block->time_wait_sch = false;
          tcb_block->rexmt_sch = false;

          tcb_block->syn_rcvd = 0;

          tcb_block->conn_estab = 0;

          tcb_block->passive = false;
          tcb_block->passive_rem_addr = -1;
          tcb_block->passive_rem_port = -1;

          tcb_block->dup_retrans_queue.setName("Dup-Retrans-Queue");
          tcb_block->dup_retrans_queue_init   = false;
          tcb_block->create_dup_retrans_queue = false;

          //insert TCB in tcb_list together with socket pair
          tcb_list.insert(TcbList::value_type(spair, tcb_block));
        }
        else
        {
          // LYBD: ignore timeout messages without TCB info,
          // the messsage will be deleted in handleMessage()
          if (debug) ev << "Timeout message arrive without associated TCB block.";
        }
    }

  return tcb_block;
}

//function to create a TCP header
TcpHeader* TcpModule::newTcpHeader(void)
{
  //create new TCP header
  TcpHeader* tcp_header = new TcpHeader;

  //fill in header information
  // -1 or 0  = information not yet available
  tcp_header->th_src_port    = -1;
  tcp_header->th_dest_port   = -1;
  tcp_header->th_seq_no      = 0;
  tcp_header->th_ack_no      = 0;
  tcp_header->th_data_offset = 5; //no options included
  tcp_header->th_flag_urg    = TCP_F_NSET;
  tcp_header->th_flag_ack    = TCP_F_NSET;
  tcp_header->th_flag_psh    = TCP_F_NSET;
  tcp_header->th_flag_rst    = TCP_F_NSET;
  tcp_header->th_flag_syn    = TCP_F_NSET;
  tcp_header->th_flag_fin    = TCP_F_NSET;
  tcp_header->th_window      = 0;
  tcp_header->th_urg_pointer = 0;

  return tcp_header;
}


//function to analyze and manage the current event variables
void TcpModule::anaEvent(TcpTcb* tcb_block, cMessage* msg, MsgSource source)
{
  //flags as ints due to casting problem
  int urg, psh;

  //save old state
  tcb_block->tb_prev_state = (TcpState) tcb_block->fsm.state();

  if (source == FROM_APPL)
    {
      //process the application call
      switch(msg->kind())
        {
        case TCP_C_OPEN_ACTIVE:
          tcb_block->st_event.event = TCP_E_OPEN_ACTIVE;
          break;

        case TCP_C_OPEN_PASSIVE:
          tcb_block->st_event.event = TCP_E_OPEN_PASSIVE;
          break;

        case TCP_C_SEND:
          tcb_block->st_event.event = TCP_E_SEND;
          urg  = msg->par("tcp_flag_urg");
          if (urg == 1)
            {
              tcb_block->st_event.th_flag_urg = TCP_F_SET;
            }
          psh = msg->par("tcp_flag_psh");
          if (psh == 1)
            {
              tcb_block->st_event.th_flag_psh = TCP_F_SET;
            }
          break;

        case TCP_C_RECEIVE:
          tcb_block->st_event.event = TCP_E_RECEIVE;
          tcb_block->st_event.num_msg = msg->par("rec_pks");
          break;

        case TCP_C_CLOSE:
          tcb_block->st_event.event = TCP_E_CLOSE;
          break;

        case TCP_C_ABORT:
          tcb_block->st_event.event = TCP_E_ABORT;
          break;

        case TCP_C_STATUS:
          tcb_block->st_event.event = TCP_E_STATUS;
          break;

        default:
          error("Unknown message kind in TcpModule::anaEvent()\n");
          break;
        } //end of switch
    } //end of if
  else if (source == FROM_TIMEOUT)
    {
      switch (msg->kind())
        {

        case TIMEOUT_TIME_WAIT:
          tcb_block->st_event.event = TCP_E_TIMEOUT_TIME_WAIT;
          break;
        case TIMEOUT_REXMT:
          tcb_block->st_event.event = TCP_E_TIMEOUT_REXMT;
          break;
        case TIMEOUT_PERSIST:
          tcb_block->st_event.event = TCP_E_TIMEOUT_PERSIST;
          break;
        case TIMEOUT_KEEPALIVE:
          tcb_block->st_event.event = TCP_E_TIMEOUT_KEEPALIVE;
          break;
        case TIMEOUT_FIN_WAIT_2:
          tcb_block->st_event.event = TCP_E_TIMEOUT_FIN_WAIT_2;
          break;
        case TIMEOUT_CONN_ESTAB:
          tcb_block->st_event.event = TCP_E_TIMEOUT_CONN_ESTAB;
          break;
        case TIMEOUT_DELAYED_ACK:
          tcb_block->st_event.event = TCP_E_TIMEOUT_DELAYED_ACK;
          break;
        default:
          opp_error("Unknown timeout in TcpModule::anaEvent()\n");
          break;
        }
    }
  else if (source == FROM_IP)
    {
      //get the pointer of the current TCP segment
      TcpHeader* msg_tcp_header = (TcpHeader*)(msg->par("tcpheader").pointerValue());

      //tcb_block->st_event.pmsg = new cMessage("st_event_pmsg");
      tcb_block->st_event.pmsg = msg;

      tcb_block->st_event.event = TCP_E_SEG_ARRIVAL;

      //flag processing
      tcb_block->st_event.th_flag_urg = msg_tcp_header->th_flag_urg;
      tcb_block->st_event.th_flag_ack = msg_tcp_header->th_flag_ack;
      tcb_block->st_event.th_flag_psh = msg_tcp_header->th_flag_psh;
      tcb_block->st_event.th_flag_rst = msg_tcp_header->th_flag_rst;
      tcb_block->st_event.th_flag_syn = msg_tcp_header->th_flag_syn;
      tcb_block->st_event.th_flag_fin = msg_tcp_header->th_flag_fin;
    }
  else
    {
      error ("Could not determine message source in TcpModule::anaEvent()\n");
    }

  // now print what we learned
  if (debug) ev << "TCP received event " << eventName(tcb_block->st_event.event)
                << " in state " << stateName((TcpState)tcb_block->fsm.state()) << endl;
}


//function "less than": a < b
//returns 1 if true, 0 otherwise
int TcpModule::seqNoLt(unsigned long a, unsigned long b)
{
  return ((0 < b - a) && (b - a < delta));
}


//function "less than or equal to" a <= b
//returns 1 if true, 0 otherwise
int TcpModule::seqNoLeq(unsigned long a, unsigned long b)
{
  return (b - a < delta);
}


//function "greater than" a > b
//returns 1 if true, 0 otherwise
int TcpModule::seqNoGt(unsigned long a, unsigned long b)
{
  return ((0 < a - b) && (a - b < delta));
}


//function "greater than or equal to" a >= b
//returns 1 if true, 0 otherwise
int TcpModule::seqNoGeq(unsigned long a, unsigned long b)
{
  return (a - b < delta);
}


//function to check the acceptance of a segment
//returns 1 if acceptable,  0 otherwise
int TcpModule::segAccept(TcpTcb* tcb_block)
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


//function to check if the RST flag of the incoming segment is set
//used in ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT
//return value: 0 if RST is set, 1 if not
int TcpModule::checkRst(TcpTcb* tcb_block)
{
  //get the received segment
  cMessage* seg = tcb_block->st_event.pmsg;
  //get header of the segment
  TcpHeader* tcp_header = (TcpHeader*) (seg->par("tcpheader").pointerValue());

  if (tcp_header->th_flag_rst == TCP_F_SET)
    {
      if (debug) ev << "TCP received a RST segment.\n";
      if (debug) ev << "The connection is aborted.\n";
      tcb_block->st_event.event = TCP_E_ABORT_NO_RST;
      return 0;
    }
  else
    {
      return 1;
    }
}


//function to check if the SYN flag of the incoming segment is set
//used in ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT, CLOSING,
//LAST_ACK, TIME_WAIT
//if the SYN is in the window it is an error ==> connection is aborted
//return value: 1 if no SYN (seg. is ok), 0 if not
int TcpModule::checkSyn(TcpTcb* tcb_block)
{
  //get the received segment
  cMessage* seg = tcb_block->st_event.pmsg;
  //get header of the segment
  TcpHeader* tcp_header = (TcpHeader*) (seg->par("tcpheader").pointerValue());

  if (tcp_header->th_flag_syn == TCP_F_SET)
    {
      if (debug) ev << "TCP received an unexpected SYN segment.\n";
      if (debug) ev << "The connection is aborted.\n";
      tcb_block->st_event.event = TCP_E_ABORT_NO_RST;
      return 0;
    }
  else
    {
      return 1;
    }
}


//function to check if the Ack flag of the incoming segment is set
//used in ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT, CLOSING,
//LAST_ACK, TIME_WAIT
//return value: 1 if ACK is set, 0 if not
int TcpModule::checkAck(TcpTcb* tcb_block)
{
  double mrtt;
  short sim_time_ticks, sent_time_ticks, mrtt_ticks;

  //get the received segment
  cMessage* seg = tcb_block->st_event.pmsg;
  //get header of the segment
  TcpHeader* tcp_header = (TcpHeader*) (seg->par("tcpheader").pointerValue());



  //if ACK is not set
  if (tcp_header->th_flag_ack == TCP_F_NSET)
    {
      if (debug) ev << "TCP received a segment without ACK.\n";
      if (debug) ev << "The connection is aborted.\n";
      return 0;
    }

  //ack sequence number of the incoming segment
  tcb_block->seg_ack = tcp_header->th_ack_no;
  ack_no_rec(tcp_header->th_ack_no);

  if (debug) ev << "The ack number of the incoming segment is " << tcb_block->seg_ack << endl;
  //Duplicate ACK?
  //FIXME: MBI we take dupacks for tcp without fast rexmt as normal acks
  if (seqNoLeq(tcb_block->seg_ack, tcb_block->snd_una))
    {
      //Rolf
      if (tcb_block->seg_len == 0)
      //Rolf
        {
          //Rolf
          if ((_feature_fast_rt) || ((_feature_fast_rc) || (_feature_nr)))
          //Rolf
            {
              if ((tcb_block->rexmt_sch == false) || (tcb_block->seg_ack != tcb_block->snd_una))
                {
                  if (debug) ev << "TCP has no outstanding data, or this is an older dupack.\n";
                  if (debug) ev << "Setting dupack counter to zero.\n";
                  //Rolf
                  //set snd_cwnd to ssthresh. Deflating the window after exiting
                  //Fast Retransmission Mode

                  if (((_feature_fast_rc) || (_feature_nr)) && (tcb_block->dupacks >=3))
                    {
                      tcb_block->snd_cwnd = tcb_block->ssthresh;
                      snd_cwnd_size(tcb_block->snd_cwnd);
                      if (_feature_nr)
                      //reset save-variables of NewReno

                        {
                          tcb_block->new_ack = 0;
                          tcb_block->partial_ack_no = 0;
                          tcb_block->partial_ack = false;
                        }
                    }
                  //Rolf
                  tcb_block->dupacks = 0;
                }
              else
                {
                  if (debug) ev << "TCP received a duplicate ACK.\n";
                  tcb_block->dupacks++;
                  if (debug) ev << "Duplicate ACK no. : " << tcb_block->dupacks << endl;
                  if (tcb_block->dupacks == 3)
                    // fast retransmit algo
                    {

                      //Rolf
                      if (_feature_nr)
                      //set the save-variables of New Reno to their initial values when entering
                      //Fast Retransmission Mode

                        {
                          tcb_block->partial_ack = false;
                          tcb_block->new_ack = 0;
                          tcb_block->partial_ack_no = 0;
                        }
                      //Rolf
                      tcb_block->savenext = tcb_block->snd_nxt;
                      // set ssthresh to 1/2 cwnd
                      tcb_block->ssthresh = MAX(MIN(tcb_block->snd_wnd,tcb_block->snd_cwnd) / 2, tcb_block->snd_mss*2);
                      //Rolf
                      //Record the highest sequence number transmitted plus 1 in the variable recover

                      if (_feature_nr)
                        {
                          tcb_block->recover = tcb_block->snd_max;
                        }
                      //Rolf
                      // turn retransmission timer and indication for timed data off
                      tcb_block->rexmt_sch = false;
                      //Rolf
                      delete cancelEvent(tcb_block->timeout_rexmt_msg);
                      //Rolf
                      tcb_block->last_timed_data = -1;
                      // set the condition for calling retransQueueProcess
                      tcb_block->snd_nxt = tcb_block->snd_una;
                      // set cwnd to one seg, so one seg will be rexmt
                      tcb_block->snd_cwnd = tcb_block->snd_mss;
                      if (_feature_fast_rt)
                        {
                          snd_cwnd_size(tcb_block->snd_cwnd);
                        }

                    }
                  //Rolf
                  //For each additional duplicate Ack received, increment snd_cwnd by snd_mss.
                  //This artificially inflates the congestion window in order to reflect the
                  //additional segment that has left the network

                  if ((tcb_block->dupacks > 3) && (_feature_fast_rc || _feature_nr))
                    {
                      tcb_block->snd_cwnd +=  tcb_block->snd_mss;
                      snd_cwnd_size(tcb_block->snd_cwnd);
                    }
                  //Rolf
                }
            } // of if (_feature_fast_rt)
          // do nothing on more dupacks, and perform slow start for next segs

          else
            // we have a duplicate ack, but no fast retransmit feature
            {
              if (debug) ev << "TCP received a duplicate ACK.\n";
              if (debug) ev << "Take it as normal ACK.\n";
              return 1;

            }
        }  // of if (seg_len == 0) ...
      else
        // this might be a data segment with a duplicate ack no. this is
        // not counted for duplicate acks.
        {
          //Rolf
          //set snd_cwnd to ssthresh. Deflating the window after exiting Fast Retransmission Mode

          //if (((_feature_fast_rc) || (_feature_nr)) && (tcb_block->dupacks >= 3))
            //{
              //tcb_block->snd_cwnd = tcb_block->ssthresh;
              //snd_cwnd_size(tcb_block->snd_cwnd);
              //if (_feature_nr)
              //reset save-variables of NewReno

                //{
                  //tcb_block->new_ack = 0;
                  //tcb_block->partial_ack_no = 0;
                  //tcb_block->partial_ack = false;
                //}
            //}
          //Rolf
          //tcb_block->dupacks = 0;
          return 1;
        }
    } // of if seqNoLeq
  else if (seqNoGt(tcb_block->seg_ack, tcb_block->snd_max))
    {
      if (debug) ev << "TCP received ACK of data not yet sent.\n";
      if (debug) ev << "Sending ACK segment to remote TCP.\n";
      ackSchedule(tcb_block, true);
      return 0;
    }

  //The incoming segment is acceptable:
  //number of octets the incoming segment acknowledges
  //unsigned long acked_octets = seg_ack - snd_una;

  if (seqNoGt(tcb_block->seg_ack, tcb_block->snd_una))
    {
      // BCH zipizigi1@hotmail.com
      unsigned long acked_octets;
      acked_octets = tcb_block->seg_ack - tcb_block->snd_una;
      // ECH
      tcb_block->rxtshift = 1;
      //Rolf
      bool deflat_window;
      if (_feature_fast_rc)
      //After the arrival of the next Ack that acknowledges new data, set snd_cwnd to ssthresh
      //(deflating the window). Remember this in order to avoid inflating snd_cwnd by snd_mss
      //in the snd_cwnd handling section at the end of the function

        {
          deflat_window = false;
          if (tcb_block->dupacks >= 3)
            {
              tcb_block->snd_cwnd = tcb_block->ssthresh;
              snd_cwnd_size(tcb_block->snd_cwnd);
              deflat_window = true;
            }
        }
      if (_feature_nr)
        {
          deflat_window = false;
          if (tcb_block->dupacks >=3)
          //if we are in Fast Retransmission Mode

            {
              if (seqNoGeq(tcb_block->seg_ack,tcb_block->recover))
              //Ack acknowledges all of the data up to and including recover (no partial ack)

                {
                  tcb_block->snd_cwnd = tcb_block->ssthresh;
                  snd_cwnd_size(tcb_block->snd_cwnd);
                  //avoid inflating snd_cwnd by snd_mss in the snd_cwnd handling section
                  //at the end of the function
                  deflat_window = true;
                  //reset save-variables of NewReno after exiting Fast Retransmission Mode
                  tcb_block->new_ack = 0;
                  tcb_block->partial_ack_no = 0;
                  tcb_block->partial_ack = false;
                }
              else if (seqNoLt(tcb_block->seg_ack,tcb_block->recover))
              //partial ack

                {
                  //remember snd_cwnd and snd_nxt for later use
                  tcb_block->savecwnd = tcb_block->snd_cwnd;
                  tcb_block->savenext = tcb_block->snd_nxt;
                  //set the condition for retransmitting one segment
                  tcb_block->snd_cwnd = tcb_block->snd_mss;
                  //avoid inflating snd_cwnd by snd_mss in the snd_cwnd handling section
                  //at the end of the function
                  deflat_window = true;
                  tcb_block->snd_nxt = tcb_block->seg_ack;
                  //remember the amount of new data acknowledged
                  if (tcb_block->seg_ack >= tcb_block->snd_una)
                    {
                      tcb_block->new_ack = tcb_block->seg_ack - tcb_block->snd_una;
                    }
                  else
                    {
                      tcb_block->new_ack = ~((unsigned long)0) - tcb_block->snd_una + tcb_block->seg_ack + 1;
                    }
                  //remember that we have just had a partial ack
                  tcb_block->partial_ack = true;
                  //increase the number of partial acks for later use with the retransmission timer
                  tcb_block->partial_ack_no += 1;
                }
            }
        }
      if (!((_feature_nr) && ((tcb_block->dupacks >= 3) && (tcb_block->partial_ack))))
      //Don´t exit Fast Retransmission Mode if we have had a partial ack

        {
          tcb_block->dupacks = 0;
        }
      //Rolf
      //check for ACK of FIN
      int snd_fin_valid = tcb_block->snd_fin_valid;
      unsigned long snd_fin_seq = tcb_block->snd_fin_seq;
      unsigned long seg_ack = tcb_block->seg_ack;

      if (snd_fin_valid && seqNoGt(seg_ack, snd_fin_seq))
        {
          //flush appropriate segments from retransmission queue
          if ((tcb_block->seg_ack - tcb_block->snd_una) > 1)
            {
              if (debug) ev << "Removing acknowledged octets from retransmission queue.\n";
              flushQueue(tcb_block->tcp_retrans_queue,  (tcb_block->seg_ack - tcb_block->snd_una - 1), false);
            }
          //set largest unacked seq. no. to ack. no.
          tcb_block->snd_una = tcb_block->seg_ack;
          if (debug) ev << "Setting event to TCP_E_RCV_ACK_OF_FIN.\n";
          tcb_block->st_event.event = TCP_E_RCV_ACK_OF_FIN;
        }

      //calculate rtt
      // we only collect one measurement (of the first segment sent)
      // per send-window
      // and none for retransmissions
      // we convert the double value into ticks (1 tick = 500 ms)
      if ((tcb_block->last_timed_data != -1) && (seqNoGeq(tcb_block->seg_ack, tcb_block->rtseq))) // fixed by LYBD
        {
          sim_time_ticks = (short) floor( (double) simTime()*2);
          sent_time_ticks = (short) floor( (double) tcb_block->last_timed_data*2);
          mrtt = simTime() - tcb_block->last_timed_data;
          if (debug) ev << "Measured rtt was " << mrtt << " seconds.\n";
          mrtt_ticks = sim_time_ticks - sent_time_ticks;
          if (debug) ev << "This is equivalent to " << mrtt_ticks << " ticks.\n";
          calcRetransTimer(tcb_block, mrtt_ticks);
        }
      //flush acked segments from retransmission queue
      if (tcb_block->seg_ack > tcb_block->snd_una)
        {
          unsigned long num_octets_in_rq = numBitsInQueue(tcb_block->tcp_retrans_queue) / 8;
          if (debug) ev << "Number of unacknowledged octets in retransmission queue: " << num_octets_in_rq << ".\n";
          if (debug) ev << "Removing " <<  (tcb_block->seg_ack - tcb_block->snd_una) <<" acknowledged octets from retransmission queue.\n";
          flushQueue(tcb_block->tcp_retrans_queue, (tcb_block->seg_ack - tcb_block->snd_una), false);
          num_octets_in_rq = numBitsInQueue(tcb_block->tcp_retrans_queue) / 8;
          if (debug) ev << "Number of unacknowledged octets in retransmission queue after removal: " << num_octets_in_rq << ".\n";
        }

      //set largest unacked seq. no. to ack. no. of incoming segment
      tcb_block->snd_una = tcb_block->seg_ack;

      // if snd_nxt is smaller than  snd_una then update it (rexmt case)
      if (seqNoLt(tcb_block->snd_nxt, tcb_block->snd_una))
        tcb_block->snd_nxt = tcb_block->snd_una;

      //If the seg. acked all data in the retransmission queue, reset the
      //sequence number of the next segment to be sent to be equal to the
      //maximum send sequence number.
      //stop retransmission timer, else restart it with rxtcur value
      if (seqNoLeq(tcb_block->snd_max, tcb_block->snd_una))
        {
          tcb_block->snd_nxt = tcb_block->snd_max;
          if (tcb_block->timeout_rexmt_msg->isScheduled() == true)
            {
              if (debug) ev << "Cancel retransmission timer \n" << endl;
              delete cancelEvent(tcb_block->timeout_rexmt_msg);
              tcb_block->rexmt_sch = false;
            }

        }
      else if (tcb_block->timeout_rexmt_msg->isScheduled() == true)
        {
          //Rolf
          if (!(((_feature_nr) && (tcb_block->dupacks >= 3)) && ((tcb_block->partial_ack) && (tcb_block->partial_ack_no > 1))))
          //only for the first partial ack that arrives during Fast Retransmission Mode, reset the retransmit timer

            {
              delete cancelEvent(tcb_block->timeout_rexmt_msg);
              tcb_block->timeout_rexmt_msg = new cMessage("TIMEOUT_REXMT",TIMEOUT_REXMT);
              //add parameters for function tcpArrivalMsg(...) here
              tcb_block->timeout_rexmt_msg->addPar("src_port") = tcb_block->local_port;
              tcb_block->timeout_rexmt_msg->addPar("src_addr") = tcb_block->local_addr;
              tcb_block->timeout_rexmt_msg->addPar("dest_port") = tcb_block->rem_port;
              tcb_block->timeout_rexmt_msg->addPar("dest_addr") = tcb_block->rem_addr;
              scheduleAt(simTime() + tcb_block->rxtcur / 2, tcb_block->timeout_rexmt_msg);
            }
          //Rolf
        }

      //UP processing (check if all urgent data to be sent has ben acknowledged)
      if (tcb_block->snd_up_valid)
        {
          if (seqNoGt(tcb_block->seg_ack, tcb_block->snd_up))
            {
              tcb_block->snd_up_valid = 0;
            }
        }

      //Rolf
      if (!((_feature_fast_rc || _feature_nr) && (deflat_window)))
      //Don´t inflate snd_cwnd by snd_mss if it has just been deflated

        {
          if (tcb_block->snd_cwnd <= tcb_block->ssthresh)
            {
              // BCH zipizigi1@hotmail.com
              tcb_block->snd_cwnd += acked_octets;
              // ECH
              snd_cwnd_size(tcb_block->snd_cwnd);
              if (debug) ev << "Incrementing the congestion window. New value: " << tcb_block->snd_cwnd << "\n";
            }
          else if (tcb_block->cwnd_cnt < (short) (tcb_block->snd_cwnd / tcb_block->snd_mss))
            {
              tcb_block->cwnd_cnt++;
              if (debug) ev << "Incrementing the Counter for congestion avoidance. New value: " << tcb_block->cwnd_cnt << "\n";
            }
          else
            {
              tcb_block->snd_cwnd += tcb_block->snd_mss;
              snd_cwnd_size(tcb_block->snd_cwnd);
              tcb_block->cwnd_cnt = 0;
              if (debug) ev << "Incrementing the congestion window. New value: " << tcb_block->snd_cwnd << "\n";
            }
        }
      //Rolf
      //put round trip time processing (Karn`s algorithm)here
    }

  //update remote receive window
  if (seqNoLt(tcb_block->snd_wl1, tcb_block->seg_seq) || (tcb_block->snd_wl1 == tcb_block->seg_seq && seqNoLeq(tcb_block->snd_wl2, tcb_block->seg_ack)))
    {
      tcb_block->snd_wl1 = tcb_block->seg_seq;
      tcb_block->snd_wl2 = tcb_block->seg_ack;
      tcb_block->snd_wnd = tcp_header->th_window;
      if (debug) ev << "Updated my snd_wnd to " << tcb_block->snd_wnd << "octets\n";
    }
  return 1;
}

//function to calculate the rtt values
void TcpModule::calcRetransTimer(TcpTcb* tcb_block, short m)
{
  if (m == 0)
    m = 1;
  if (tcb_block->srtt != 0)
    {
      m -= (tcb_block->srtt >> 3);
      tcb_block->srtt += m;
      if (m < 0)
        m = -m;
      m -= (tcb_block->rttvar >> 2);
      tcb_block->rttvar += m;
    }
  else
    {
      tcb_block->srtt = m << 3;
      tcb_block->rttvar = m << 2;
    }

  tcb_block->last_timed_data = -1;
  tcb_block->rxtshift = 0;
  tcb_block->rxtcur = (tcb_block->srtt >> 3) + tcb_block->rttvar;
  if (tcb_block->rxtcur < 2)
    tcb_block->rxtcur = 2;
  if (debug) ev << "Smoothed RTT (8*ticks): " << tcb_block->srtt << endl;
  if (debug) ev << "Smoothed mean deviation of RTT (4*ticks): " << tcb_block->rttvar << endl;
  if (debug) ev << "RTO value: " << tcb_block->rxtcur << endl;
}

void TcpModule::timeoutPersistTimer(TcpTcb* tcb_block) {
   if (debug) ev << simTime() << " Persist Time expired" << endl;
   tcb_block->snd_wnd = 1;
}

//function for retransmission timer expiry
void TcpModule::timeoutRetransTimer(TcpTcb* tcb_block)
{
      if (debug) ev << "Retransmission timer expired. \n";
      tcb_block->rexmt_sch = false;
      // set new rxtcur value with backoff shift
      // we have incremented the backoff shift before calling this function

      tcb_block->rxtcur = MIN(tcb_block->rxtcur*tcb_block->rxtshift,128);

      if (debug) ev << "Backoff shift: " << tcb_block->rxtshift <<endl;
      if (debug) ev << "New value for RTO: " << tcb_block->rxtcur << endl;
      // set new value for ssthresh, and enforce slow start
      tcb_block->ssthresh = MAX(MIN(tcb_block->snd_wnd, tcb_block->snd_cwnd)/2,tcb_block->snd_mss*2);
      tcb_block->snd_cwnd = tcb_block->snd_mss;
      snd_cwnd_size(tcb_block->snd_cwnd);
      if (debug) ev << "New Slow-Start threshold: " << tcb_block->ssthresh << endl;
      if (debug) ev << "New Congestion Window: " << tcb_block->snd_cwnd << endl;
      //Rolf
      //in case of a retransmission timeout, exit Fast Retransmission Mode

      if ((_feature_fast_rc) || (_feature_fast_rt))
        {
          tcb_block->dupacks = 0;
        }
      if (_feature_nr)
      //reset save-variables of NewReno to their initial values when exiting
      //Fast Retransmission Mode after a retransmission timeout

        {
          tcb_block->dupacks = 0;
          tcb_block->new_ack = 0;
          tcb_block->partial_ack_no = 0;
          tcb_block->partial_ack = false;
        }
      //Rolf
      // call retransmission processing
      tcb_block->snd_nxt = tcb_block->snd_una;
}

//function to send a data-less ACK to the remote TCP
void TcpModule::timeoutDatalessAck(TcpTcb* tcb_block)
{
  //create a data-less segment

  cMessage* msg = new cMessage("ACK_DATA", ACK_DATA) ;
  msg->setLength(0);

  //set flags
  TcpFlag fin = TCP_F_NSET;
  TcpFlag syn = TCP_F_NSET;
  TcpFlag rst = TCP_F_NSET;
  TcpFlag ack = TCP_F_SET;
  TcpFlag urg = TCP_F_NSET;
  TcpFlag psh = TCP_F_NSET;

  //send the data-less ACK segment to remote TCP
  segSend(msg, tcb_block, tcb_block->snd_nxt, fin, syn, rst, ack, urg, psh);
}


//function to process a SEND command issued by the application
void TcpModule::applCommandSend(cMessage* msg, TcpTcb* tcb_block)
{
  //unsent data buffer
  cQueue & tcp_send_queue = tcb_block->tcp_send_queue;


  //amount of data to be sent (in bits)
  long packet_size = msg->length();

  // store original packet size as parameter
  msg->addPar("original_length") = packet_size;

  if (packet_size == 0)
    {
      //exiting the simulation
      error( "No data to sent specified in SEND command issued by the application");
    }
  else
    //padding the received data to the nearest byte (packet_size modulo 8)
    if ((packet_size % 8) != 0)
      {
        packet_size += (8 - (packet_size % 8));
        msg->setLength(packet_size);
      }

  msg->setTimestamp();

  //put msg copy at tail of tcp_send_queue (msg will be deleted later)
  insertAtQueueTail(tcp_send_queue, (cMessage*) msg->dup(), 0);

  //urgent pointer processing (according to RFC-1122)
  if (tcb_block->st_event.th_flag_urg == TCP_F_SET)
    {
      tcb_block->snd_up_valid = 1;
      tcb_block->snd_up = tcb_block->snd_nxt + numBitsInQueue(tcp_send_queue) / 8 - 1;
    }
}


//function to process a RECEIVE command issued by the application
void TcpModule::applCommandReceive(cMessage* msg, TcpTcb* tcb_block)
{
  //amount of packets requested by the application
  int rec_pks = msg->par("rec_pks");
  tcb_block->num_pks_req += rec_pks;

  if (debug) ev << "The application is waiting for " << tcb_block->num_pks_req << " data packets.\n";
}


//function to send a segment to the remote TCP
void TcpModule::segSend(cMessage* data, TcpTcb* tcb_block, unsigned long seq_no, TcpFlag fin, TcpFlag syn, TcpFlag rst, TcpFlag ack, TcpFlag urg, TcpFlag psh)
{
  //create a new TCP segment
  cMessage* tcp_seg;

  if (data->kind() == SYN_DATA || syn == TCP_F_SET)
    {
      tcp_seg = new cMessage("SYN_SEG", SYN_SEG);
    }
  else if (data->kind() == ACK_DATA)
    {
      tcp_seg = new cMessage("ACK_SEG", ACK_SEG);
    }
  else if (data->kind() == RST_DATA)
    {
      tcp_seg = new cMessage("RST_SEG", RST_SEG);
    }
  else if (data->kind() == FIN_DATA || fin == TCP_F_SET)
    {
      tcp_seg = new cMessage("FIN_SEG", FIN_SEG);
    }
  else
    {
      data->setName("TCP_DATA");
      data->setKind(TCP_DATA);
      tcp_seg = new cMessage("TCP_SEG", TCP_SEG);
    }

  //create a new TCP header
  TcpHeader* tcp_header = newTcpHeader();

  //fill in source and destination port
  tcp_header->th_src_port  = tcb_block->local_port;
  tcp_header->th_dest_port = tcb_block->rem_port;

  //assign a sequence number to the segment
  tcp_header->th_seq_no = seq_no;

  //if this is an ACK segment, set the ACK flag and the ack number field
  if (ack == TCP_F_SET)
    {
      tcp_header->th_flag_ack = TCP_F_SET;
      tcp_header->th_ack_no   = tcb_block->rcv_nxt;

      //turn off timer for a data-less ack
      //tcb_block->ack_sch = false; (add bool ack_sch to tcb_block)
      //delete cancelEvent(del_ack_timer_msg);
    }

  //set size of the receive window
  tcp_header->th_window   = tcb_block->rcv_wnd;
  if (debug) ev << "Set the size of rcv_wnd (" << tcp_header->th_window << " octets) in hdr\n";
  tcb_block->rcv_wnd_last = tcb_block->rcv_wnd;

  //segment length in number of octets
  tcb_block->seg_len = data->length() / 8;

  //if this is a PSH segment, set the PSH flag
  if (psh == TCP_F_SET)
  {
      tcp_header->th_flag_psh = TCP_F_SET;
      // Set Push Parameter (if TCP_DATA)
      if(data->kind()==TCP_DATA)
        ((cMessage*)((cArray*)data->parList().get("msg_list"))->get(0))->
                par("tcp_flag_psh")=(long)1;
  }

  //if this is a RST segment, set the RST flag
  if (rst == TCP_F_SET)
    {
      tcp_header->th_flag_rst = TCP_F_SET;
    }

  //if this is a URG segment, set the URG flag and URG pointer
  if (urg == TCP_F_SET)
    {
      tcp_header->th_flag_urg = TCP_F_SET;
      tcp_header->th_urg_pointer = tcb_block->snd_up;
    }

  //Since SYN and FIN have to be acknowledged, seg_len is
  //increased by one octet (but not the actual length of
  //the TCP segment), RFC-793 p.26.
  //if this is a SYN segment, set the SYN flag
  if (syn == TCP_F_SET)
    {
      tcp_header->th_flag_syn = TCP_F_SET;
      tcb_block->seg_len++;

      //sequence number of a retransmitted segment
      tcb_block->max_retrans_seq = tcb_block->snd_una + tcb_block->seg_len - 1;

      //add MSS-processing here
    }

  //if this is a FIN segment, set the FIN flag
  if (fin == TCP_F_SET)
    {
      tcp_header->th_flag_fin = TCP_F_SET;
      tcb_block->seg_len++;
    }

  //add ICI for use in IP header
  tcp_seg->addPar("src_addr") = tcb_block->local_addr;
  tcp_seg->addPar("dest_addr") = tcb_block->rem_addr;

  //add seg_len as a parameter
  tcp_seg->addPar("seg_len") = tcb_block->seg_len;

  //add tcp_conn_id as parameter
  tcp_seg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

  //add num_bit_req as parameter
  tcp_seg->addPar("num_bit_req") = tcb_block->num_bit_req;

  //add TCP header
  tcp_seg->addPar("tcpheader") = (void*) tcp_header;
  tcp_seg->par("tcpheader").configPointer(NULL, NULL, sizeof(TcpHeader));

  //set length of the TCP header without options (in bits)
  tcp_seg->setLength(20 * 8);

  //set time stamp for TCP-/TCP-delay computation
  tcp_seg->setTimestamp();

  //encapsulate the data into the TCP segment
  tcp_seg->encapsulate(data);

  //start the retransmission timer
  //we do this for every packet except pure acks if the timer has not been started yet


  if ((tcb_block->rexmt_sch == false) && (tcp_seg->kind() != ACK_SEG) &&  (tcp_seg->kind() != RST_SEG))
    {
      tcb_block->timeout_rexmt_msg = new cMessage("TIMEOUT_REXMT",TIMEOUT_REXMT);
      //add parameters for function tcpArrivalMsg(...) here
      tcb_block->timeout_rexmt_msg->addPar("src_port") = tcb_block->local_port;
      tcb_block->timeout_rexmt_msg->addPar("src_addr") = tcb_block->local_addr;
      tcb_block->timeout_rexmt_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_rexmt_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + tcb_block->rxtcur / 2, tcb_block->timeout_rexmt_msg);
      if (debug) ev << "Started retransmission timer for " << simTime()+tcb_block->rxtcur / 2 << " seconds. \n";
      tcb_block->rexmt_sch = true;
    }

  //send segment
  if (debug) ev << "Sending segment of " << tcp_seg->length() / 8 << " bytes to remote TCP.\n";
  if (debug) ev << "Sequence number: " << tcp_header->th_seq_no << "\n";
  if (debug) ev << "RCV.NXT: " << tcb_block->rcv_nxt << "\n";
  if (debug) ev << "SYN = " << (int) tcp_header->th_flag_syn << ", FIN = " << (int) tcp_header->th_flag_fin << "\n";
  if (debug) ev << "RST = " << (int) tcp_header->th_flag_rst << ", ACK = " << (int) tcp_header->th_flag_ack << "\n";
  if (debug) ev << "URG = " << (int) tcp_header->th_flag_urg << ", PSH = " << (int) tcp_header->th_flag_psh << "\n";
  send(tcp_seg, "to_ip");
  seq_no_send(tcp_header->th_seq_no);
}

//function to process an incoming segment from the remote TCP
void TcpModule::segReceive(cMessage* pseg, TcpHeader* pseg_tcp_header, TcpTcb* tcb_block, TcpFlag fin, TcpFlag syn, TcpFlag rst, TcpFlag ack, TcpFlag urg, TcpFlag psh)
{
  cMessage*     pdata;
  unsigned long i, list_size;
  unsigned long rcv_nxt_old;
  unsigned long rcv_queue_end;
  unsigned long seg_up;
  unsigned long num_bytes;
  unsigned long seq_rec;
  SegRecord*    pseg_rec;
  SegRecord*    pnew_rec;

  //receive queue
  cQueue &      tcp_data_receive_queue = tcb_block->tcp_data_receive_queue;
  //socket buffer
  cQueue &      tcp_socket_queue       = tcb_block->tcp_socket_queue;
  //receive record list
  cLinkedList & tcp_rcv_rec_list       = tcb_block->tcp_rcv_rec_list;

  //list for segments of type SegRecord that arrived out of order
  // FIXME: MBI variable never used (something missing?)
  cLinkedList tcp_seg_rec_list;
  //re-segmentation queue to separate the usable parts of an incoming
  //segment from the ones that have to be dropped
  cQueue tcp_resegm_queue;

  //use variable "remainder" to compute the difference between the incoming segment
  //size and the size of the packet expected in the tcp_seg_receive_queue in case
  //of a retransmission
  int           remainder;
  int           pkt_size;
  unsigned long data_size;
  double        ddata_size;
  double        rcv_queue_usage   = 0;
  bool          segment_exists    = false;
  bool          complete_pkt_rcvd = false;

  // <Jeroen>
  cnt_packet_delack++; // to see if 2 packets have been received without sending an ack
  // </Jeroen>

  int ackno = pseg_tcp_header->th_ack_no;

  //starting value for RCV.NXT
  rcv_nxt_old = tcb_block->rcv_nxt;

  //check if URG is set
  if (urg == TCP_F_SET)
    {
      //get UP from incoming header
      seg_up = pseg_tcp_header->th_urg_pointer;
      if (seqNoGt(seg_up, tcb_block->rcv_up) || !(tcb_block->rcv_up_valid))
        {
          tcb_block->rcv_up = seg_up;
        }
      tcb_block->rcv_up_valid = 1;
    }

  //check if PSH is set
  if (psh == TCP_F_SET)
    {
      //if PSH is set the incoming segment completes a data package
      complete_pkt_rcvd = true;
    }

  //check if FIN is set
  if (fin == TCP_F_SET)
    {
      if (debug) ev << "Fin flag is set.\n";
     //fin takes up one octet in the sequence number space

      tcb_block->seg_len = pseg->par("seg_len");

      tcb_block->seg_len--;

      tcb_block->rcv_fin_valid = 1;

      tcb_block->seg_seq = pseg_tcp_header->th_seq_no;

      //sequence number of the FIN
      tcb_block->rcv_fin_seq = tcb_block->seg_seq + tcb_block->seg_len;
    }

  //if there is data in the incoming segment, process it
  pdata = pseg->decapsulate();
  if (pdata->length() == 0)
    {
      if (debug) ev << "Encapsulated data packet of the incoming segment contains no octets.\n";
      delete pdata; // LYBD: fix memory leaks!
    }
  else
    {
      remainder = 0;

      if (debug) ev << "The sequence number of the incoming segment is " << tcb_block->seg_seq << endl;

      //check if the data has arrived in order
      if (seqNoLeq(tcb_block->seg_seq, tcb_block->rcv_nxt))
        {
          //check if there are overlaps
          if (seqNoLt(tcb_block->seg_seq, tcb_block->rcv_nxt))
            {
              if (debug) ev << "TCP received a segment containing overlapping data.\n";

              //put the data of the segment which is expected next in the tcp_data_receive_queue
              insertAtQueueTail(tcp_data_receive_queue, pdata, 0);
              seq_rec = tcb_block->seg_seq;
              seq_no_rec(seq_rec);

              //flush the initial octets that have arrived before the current incoming segment
              flushQueue(tcb_block->tcp_data_receive_queue, (tcb_block->rcv_nxt - tcb_block->seg_seq), false);

              //update the sequence of the segment which is expected next
              tcb_block->rcv_nxt = tcb_block->rcv_buf_seq + numBitsInQueue(tcp_data_receive_queue) / 8 + numBitsInQueue(tcp_socket_queue) / 8;
            }
          //there is no overlapping data
          else // seg_seg == rcv_nxt
            {
              insertAtQueueTail(tcp_data_receive_queue, pdata, 0);
              seq_rec = tcb_block->seg_seq;
              seq_no_rec(seq_rec);

              //update the sequence of the segment which is expected next
              tcb_block->rcv_nxt = tcb_block->rcv_buf_seq + numBitsInQueue(tcp_data_receive_queue) / 8 + numBitsInQueue(tcp_socket_queue) / 8;

              //compare length of data having been retransmitted with the original expected sequence
              list_size = tcp_rcv_rec_list.length();
              if (debug) ev << "The list size of the out-of-order Q is: " << list_size << endl;
              if (list_size > 0)
                {
                  //access first out of order segment
                  pseg_rec = (SegRecord* ) tcp_rcv_rec_list.peekHead();
                  //amount of excess retransmission
                  remainder = tcb_block->seg_len - (pseg_rec->seq - tcb_block->seg_seq);

                  if (remainder > 0)
                    {
                      if (debug) ev << "Amount of excess retransmission: " << remainder << ", RCV.NXT: " << tcb_block->rcv_nxt << "\n";
                      if (debug) ev << "Retransmitted TCP segment contains more data than what the current socket process already contains.\n";
                    }
                  if (debug) ev << "Remainder: " << remainder << endl;


                  if (remainder >= 0)
                    {
                      //move out-of-order segments into tcp_data_receive_queue
                      // FIXME: MBI i=1..list_size

                      for (i = 1; i <= list_size; i++)
                        {
                          pseg_rec = accessListElement(tcp_rcv_rec_list, i);

                          if (pseg_rec == NULL)
                            {
                              if (debug) ev << "Warning: Not able to get segment record from tcp_rcv_rec_list. Moving to next record in list.\n";
                              continue;
                            }
                          if (debug) ev << "Seq No of segment in rcv_rec q: " << pseg_rec->seq << endl;
                          if (seqNoLeq(pseg_rec->seq, tcb_block->rcv_nxt))
                            {
                              //FIXME: MBI removing at other position
                              //remove record from list
                              //removeListElement(tcp_rcv_rec_list, i);
                              //list_size--;
                              //i--;

                              //get the data size of psegrec in bytes
                              cMessage* data = pseg_rec->pdata;
                              pkt_size = data->length() / 8;
                              if (debug) ev << "The packet size is " << pkt_size << endl;

                              // LYBD: there may be overlapping segments in the out-of-order queue
                              // calculate and take out the overlapping portions for each segment
                              remainder = tcb_block->rcv_nxt - pseg_rec->seq;

                              //if length of retransmission >= pkt_size ==> drop the duplicate segment
                              if (remainder >= pkt_size)
                                {
                                  delete pseg_rec->pdata;
                                }
                              else
                                {
                                  //put segment in tcp_data_receive_queue
                                  insertAtQueueTail(tcp_data_receive_queue, pseg_rec->pdata, pseg_rec->seq);
                                  // if psh flag is set, indicate that complete packet is rxd
                                  // FIXME: MBI
                                  // problems if a packet behind pshd packet
                                  // is in rcv-rec q ????
                                  if(((long) ((cMessage*)((cArray*)data->parList().get("msg_list"))->get(0))->
                                        par("tcp_flag_psh"))==1 )
                                  {
                                        //if PSH is set the incoming segment completes a data package
                                        complete_pkt_rcvd = true;
                                  }

                                  //drop the overlapping amount
                                  if (remainder > 0)
                                    {
                                      //flush the overlapping portions of data
                                      if (debug) ev << "Cannot fit one complete segment. Flushing " << remainder << " bytes.\n";
                                      flushLabeledDataQueue(tcp_data_receive_queue, pseg_rec->seq, remainder * 8);

                                      //reset the remainder variable
                                      remainder = 0;
                                    }

                                  //update the sequence number of the next expected segment
                                  tcb_block->rcv_nxt = tcb_block->rcv_buf_seq + numBitsInQueue(tcp_data_receive_queue) / 8 + numBitsInQueue(tcp_socket_queue) / 8;
                                  if (debug) ev << "Updating RCV.NXT to sequence number " << tcb_block->rcv_nxt << "\n";
                                }
                              removeListElement(tcp_rcv_rec_list, i);
                              list_size--;
                              i--;
                            } // of if seq_no
                        } // of for
                    } // of if remainder >= 0
                } // of if list_size > 0
            }  // of else


          //update receive buffer usage
          rcv_queue_usage = numBitsInQueue(tcp_data_receive_queue) / 8;
          if (debug) ev << "Number of bits in tcp-data-rx-q: " << rcv_queue_usage << endl;

          //rcv_buff is given size of tcp_receive_data_queue
          if ((rcv_queue_usage >= tcb_block->rcv_buf_usage_thresh * tcb_block->rcv_buff) || (complete_pkt_rcvd == true))
            {
              if (debug) ev << "Complete packet received: " << (int)complete_pkt_rcvd << endl;
              //size of data to be put in tcp_socket_queue (in octets)
              ddata_size = (complete_pkt_rcvd) ? (numBitsInQueue(tcp_data_receive_queue)) / 8 : (tcb_block->rcv_buf_usage_thresh * tcb_block->rcv_buff);

              data_size = (unsigned long) ceil(ddata_size);

              if (debug) ev << "Data size to be put into socket q: " << data_size << endl;
              //transfer data packets to tcp_socket_queue
              transferQueues(tcp_data_receive_queue, tcp_socket_queue, data_size);

              //update receive buffer usage
              rcv_queue_usage = numBitsInQueue(tcp_data_receive_queue) / 8;
            }
        }
      else
        {
          //this it not the next expected segment; put it in the
          //out-of-order list
          rcv_queue_end = tcb_block->rcv_buf_seq + tcb_block->rcv_buff;

          if (seqNoGt((tcb_block->seg_seq + tcb_block->seg_len), rcv_queue_end))
            {
              insertAtQueueTail(tcp_resegm_queue, pdata, 0);
              seq_rec = tcb_block->seg_seq;
              seq_no_rec(seq_rec);

              //clip off the part that does not fit in the queue
              num_bytes = MIN(tcb_block->seg_len, (rcv_queue_end - tcb_block->rcv_nxt));

              if (num_bytes > 0)
                {
                  pdata = removeFromDataQueue(tcp_resegm_queue, num_bytes * 8);
                }
              else
                {
                  pdata = NULL;
                }

              //flush the remaining data
              if (numBitsInQueue(tcp_resegm_queue) > 0)
                {
                  tcp_resegm_queue.clear();
                }
            }

          //if no data could be retained ==> discard segment, send an ACK
          if (pdata == NULL)
            {
              ackSchedule(tcb_block, true);
              if (debug) ev << "Returning from segReceive(...) function.\n";
              return;
            }

          //create new out-of-order segment record
          pnew_rec        = new SegRecord;
          pnew_rec->seq   = tcb_block->seg_seq;
          pnew_rec->pdata = pdata;

          //keep records sorted according to their sequence number
          list_size = tcp_rcv_rec_list.length();
// FIXME: MBI replacing processing from 0..length-1 with 1..length
// FIXME: UNTESTED for list_size > 0
          // loop gives position where segment should be inserted
          // and segment_exists if it's already in the out-of-order queue
          for (i = 1; i <= list_size; i++)
            {
              pseg_rec = accessListElement(tcp_rcv_rec_list, i);

              if (pseg_rec == NULL)
                {
                  if (debug) ev << "Warning: Unable to get segment record from tcp_rcv_rec_list. Moving to next record in list.\n";
                  continue;
              }

              if (seqNoLeq(tcb_block->seg_seq, pseg_rec->seq))
                {
                  //check if segment exists in list
                  if (tcb_block->seg_seq == pseg_rec->seq)
                    {
                      segment_exists = true;
                    }

                  break;
                }
            }

          if (debug) ev << "(required) position in list is " << i << endl;
          //if segment is not in list already, insert it
          //i can take values from 1..list_size+1
          if (!segment_exists)
            {
              insertListElement(tcp_rcv_rec_list, pnew_rec, i);

            } // of if
          ackSchedule(tcb_block, true);

          // <Jeroen>
          // we're sending an ack with the expected seq nr rcv_next
          // store this seqnr in case we receive this segment, so we can send an immediate ack
          seqnr_outorder_exp = tcb_block->rcv_nxt;
          // </Jeroen>

        } // of else (out of order processing)
    } //of else (length > 0)

  //update RCV.WND
  if (tcb_block->rcv_wnd > (tcb_block->rcv_nxt - rcv_nxt_old))
    {
      tcb_block->rcv_wnd = tcb_block->rcv_wnd - (tcb_block->rcv_nxt - rcv_nxt_old);
      if (debug) ev << "Updating rcv_wnd: decrementing for " << tcb_block->rcv_nxt - rcv_nxt_old << " octets. New value: "<< tcb_block->rcv_wnd << " octets. \n";
    }
  else
    {
      tcb_block->rcv_wnd = 0;
      if (debug) ev << "Setting rcv_wnd to 0 because it's smaller or equal to "<< tcb_block->rcv_nxt - rcv_nxt_old << " octets.\n";
    }

  // RdM: complete SWA according to RFC-1122, p.97
  if (tcb_block->rcv_buff - rcv_queue_usage - tcb_block->rcv_wnd >= MIN(tcb_block->rcv_buf_usage_thresh * tcb_block->rcv_buff, tcb_block->snd_mss))
    {
    tcb_block->rcv_wnd = tcb_block->rcv_buff - (unsigned long)rcv_queue_usage;
    if (debug) ev << "SWA: rcv_wnd = " << tcb_block->rcv_wnd << "\n";
  }

  //FIN processing, if FIN flag has been received
  if (debug) ev << "Sequence no. of fin " << tcb_block->rcv_fin_seq << endl;
  if (tcb_block->rcv_fin_valid && (tcb_block->rcv_fin_seq == tcb_block->rcv_nxt))
    {
      if ((tcb_block->st_event.event == TCP_E_RCV_ACK_OF_FIN) || (tcb_block->st_event.event == TCP_E_RCV_FIN_ACK_OF_FIN))
        {
          tcb_block->st_event.event = TCP_E_RCV_FIN_ACK_OF_FIN;
        }
      else
        {
          tcb_block->st_event.event = TCP_E_RCV_FIN;
        }
      tcb_block->rcv_nxt++;
    }

  if (!tcb_block->rcv_fin_valid)
    {
      //just in case anything new has been received acknowledge it
      if (seqNoGt(tcb_block->rcv_nxt, rcv_nxt_old))
        {
          if (debug) ev << "RCV.NXT: " << tcb_block->rcv_nxt << ", old RCV.NXT: " << rcv_nxt_old << "\n";
      if (_feature_delayed_ack && cnt_packet_delack < 2 && seqnr_outorder_exp != tcb_block->seg_seq)
            {
              ackSchedule(tcb_block, false);
            }
          else
            {
              // <Jeroen>
              if (debug) if (cnt_packet_delack == 2) ev << "Received 2 unacknowledged packets, sending Ack!" << endl;
              // </Jeroen>
              ackSchedule(tcb_block, true);
            }
        }
    }

  if (debug) ev << "Reached end of segReceive(...) function.\n";
}


//function to decide from which queue to send data
void TcpModule::sndDataProcess(TcpTcb* tcb_block)
{
  //decide if data should be sent from tcp_send_queue or tcp_retrans_queue
  //send data in tcp_retrans_queue
  if (seqNoLt(tcb_block->snd_nxt, tcb_block->snd_max))
    {
      if (debug) ev << "Retransmitting data.\n";
      retransQueueProcess(tcb_block);
    }

  //send data in tcp_send_queue
  if (seqNoGeq(tcb_block->snd_nxt, tcb_block->snd_max))
    {
      if (debug) ev << "Sending new data.\n";
      sndQueueProcess(tcb_block);
    }
}

//function to process the contents tcp_retrans_queue
void TcpModule::retransQueueProcess(TcpTcb* tcb_block)
{
  cQueue &      tcp_retrans_queue = tcb_block->tcp_retrans_queue;
  //duplicate retransmission queue created to actually resend the retransmitted data
  cQueue &      dup_retrans_queue = tcb_block->dup_retrans_queue;
  cQueue        resend_data_queue("resend_data_queue");
  //unsigned long retrans_data_sent;
  unsigned long retxn_queue_size;
  unsigned long bytes_to_be_resent;
  unsigned long retrans_queue_size;
  unsigned long segment_size;
  cMessage*     pseg;
  cMessage*     pto_be_sent_seg;

  //determine the size of the retransmission queue
  retrans_queue_size = numBitsInQueue(tcp_retrans_queue);
  if (debug) ev << "Number of bits in retransmission queue: " << retrans_queue_size << endl;
  //the following statement is true if a currently received
  //ACK segment frees the retransmission queue
  // LYBD: the FIN packet is not inserted until tcb_block->snd_nxt == tcb_block->snd_fin_seq
  if ((retrans_queue_size == 0) && !(tcb_block->snd_fin_valid
        && tcb_block->snd_nxt == tcb_block->snd_fin_seq))
    {
      if (debug) ev << "Returning from retransQueueProcess(...) function.\n";
      return;
    }
  //FIXME: MBI assuming we have only a fin to send if no bits in Q and fin
  //           scheduled. We have put that fin into rexmt Q in sendQProcess.
  //FIXME: MBI dup_retrans_queue insert something missing...
  //           and we should remove the msg from the retrans_queue when ACKed
  //           (maybe in the exit code of fin_wait_1 and last_ack)
  // LYBD: the FIN packet is not inserted until tcb_block->snd_nxt == tcb_block->snd_fin_seq
  else if ((retrans_queue_size == 0) && (tcb_block->snd_fin_valid)
      && tcb_block->snd_nxt == tcb_block->snd_fin_seq)
    {
        cMessage* packet = (cMessage *) tcp_retrans_queue.head();
        dup_retrans_queue.clear();
        insertAtQueueTail(dup_retrans_queue, (cMessage *) packet->dup(), 0);
        pseg = (cMessage *) dup_retrans_queue.remove(dup_retrans_queue.head());
        segSend(pseg, tcb_block, tcb_block->snd_nxt, TCP_F_SET, TCP_F_NSET, TCP_F_NSET, TCP_F_SET, TCP_F_NSET, TCP_F_NSET);
        return;
    }
  else
    {

      //FIXME: MBI code is never visited
      //clear dup_retrans_queue if necessary
      if (tcb_block->create_dup_retrans_queue == true)
        {
          if (debug) ev << "Create rexmt q set to false. \n";
          tcb_block->create_dup_retrans_queue = false;


          if (tcb_block->dup_retrans_queue_init == false)
            {
              if (debug) ev << "TCP duplicate retxn Q was not inited, initializing..\n";
              tcb_block->dup_retrans_queue_init = true;

            }
          else
            {
              if (debug) ev << "Clearing dup retxn Q. \n";
              dup_retrans_queue.clear();
            }
        }
      // FIXME: MBI just trying something
      //        dup_retrans_queue.clear();
      //copy data from tcp_retrans_queue to dup_retrans_queue
      //     transferQueues(tcp_retrans_queue, dup_retrans_queue, retrans_queue_size / 8);
      //copyQueues(tcp_retrans_queue, dup_retrans_queue, retrans_queue_size / 8);
      //retrans_queue_size = numBitsInQueue(tcp_retrans_queue);
    }

  //FIXME: MBI re-initialize dup_retrans_queue only if we are just beginning
  //with new window, if we are in the middle (ack just outstanding, but we
  //can send more) take the old dup_retrans_queue
  if (tcb_block->snd_una == tcb_block->snd_nxt)
    {
      if (debug) ev << "Re-Initializing Dup-Retxn Queue.\n";
      dup_retrans_queue.clear();
      copyQueues(tcp_retrans_queue, dup_retrans_queue, retrans_queue_size / 8);
    }
  else if (debug) ev << "Keeping dup-retxn Queue.\n";

  //size in octets of dup_retrans_queue
  retxn_queue_size = numBitsInQueue(dup_retrans_queue) / 8;

  //amount of data to be resent
  while (( bytes_to_be_resent = retransDataSize(tcb_block, dup_retrans_queue)) > 0)
    {
      //clear the re-send data queue
      resend_data_queue.clear();

      if (bytes_to_be_resent == 0)
        {
          if (debug) ev << "Returning from retransQueueProcess(...) function.\n";
          return;
        }

      //account for the possibility that tcp_retrans_queue contains
      //fewer octets than what can be sent
      //FIXME: MBI does TCP send anything from send queue then??
      //bytes_to_be_resent = (bytes_to_be_resent < retxn_queue_size) ? bytes_to_be_resent : retxn_queue_size;

      // LYBD: do not send any segment from tcp_send_queue
      // otherwise the sender may receive acks with seqno > SND.MAX
      // and the retransmission queue will be mixed with the send queue
      if (bytes_to_be_resent + tcb_block->snd_nxt > tcb_block->snd_max) {
          bytes_to_be_resent = tcb_block->snd_max - tcb_block->snd_nxt;
      }

      if (bytes_to_be_resent == 0)
        {
          if (debug) ev << "Returning from retransQueueProcess(...) function.\n";
          return;
        }

      // not necessary
      //retrans_data_sent = 0;

      //get the segment to be resent
      pto_be_sent_seg = removeFromDataQueue(dup_retrans_queue, bytes_to_be_resent * 8);

      //insert octets to be resent into resend_data_queue
      insertAtQueueTail(resend_data_queue, pto_be_sent_seg, 0);

      //send data using MSS (if possible) until there is no more data to be resent
      //while (retrans_data_sent < bytes_to_be_resent)
      // {
      //determine the segment size of the segment to be sent
      //if ((bytes_to_be_resent - retrans_data_sent) > tcb_block->snd_mss)
      //{
      //    segment_size = tcb_block->snd_mss;
      //  }
      //else
      //  {
      //    segment_size = bytes_to_be_resent - retrans_data_sent;
      //  }
      //
      // this code allows only one seg to be sent even if we have a bigger wnd..

      //set the segment size to MSS or bytes_to_be_resent
      if (bytes_to_be_resent > tcb_block-> snd_mss)
        {
          //FIXME: MBI should never be true
          segment_size = tcb_block->snd_mss;
        }
      else
        {
          segment_size = bytes_to_be_resent;
        }

      //get the segment
      pseg = removeFromDataQueue(resend_data_queue, segment_size * 8);
      if (pseg == NULL)
        {
          segment_size = 0;
        }

      //set ACK
      TcpFlag ack = TCP_F_SET;
      //stop the delayed-ack-timer if running
      if (tcb_block->ack_sch == true)
        {
          delete cancelEvent(tcb_block->timeout_delayed_ack_msg);
          tcb_block->ack_sch = false;
        }

      //check if PUSH and URG should be set here
      TcpFlag psh = TCP_F_NSET;
      if (segment_size < tcb_block->snd_mss)
        {
          psh = TCP_F_SET;
        }

      TcpFlag urg = TCP_F_NSET;
      if (tcb_block->snd_up_valid && seqNoGeq(tcb_block->snd_up, tcb_block->snd_una))
        {
          urg = TCP_F_SET;
        }

      //check if FIN should be set (if so increment
      //segment_size, see RFC-793 p.26)
      TcpFlag fin = TCP_F_NSET;
      if (tcb_block->snd_fin_valid && (tcb_block->snd_una + segment_size == tcb_block->snd_fin_seq))
        {
          fin = TCP_F_SET;
          segment_size++;
        }

      //send the segment
      segSend(pseg, tcb_block, tcb_block->snd_nxt, fin, TCP_F_NSET, TCP_F_NSET, ack, urg, psh);

      //update SND.NXT
      tcb_block->snd_nxt += segment_size;

      //put slow-start congestion avoidance here

      //update last send time
      tcb_block->last_snd_time = simTime();

      //set retransmission timeout here, if necessary

      //measure round trip time here, if applicable
      //rtt never measured in a rexmt! therefore
      tcb_block->last_timed_data = -1;

      //update send variable
      //retrans_data_sent += segment_size;

    } //end of while

  //clear the re-send data queue
  //resend_data_queue.clear();

  //if all the data has been sent, send a data-less FIN segment
  if (tcb_block->snd_fin_valid && tcb_block->snd_nxt == tcb_block->snd_fin_seq)
    {
      //create a data-less FIN segment
      cMessage* msg = new cMessage("FIN");
      msg->setLength(0);
      TcpFlag   fin = TCP_F_SET;
      TcpFlag   ack = TCP_F_SET;

      //only ACK and FIN are set                                                       (*)
      segSend(msg, tcb_block, tcb_block->snd_nxt++ , fin, TCP_F_NSET, TCP_F_NSET, ack, TCP_F_NSET, TCP_F_NSET);
    }

  //Rolf
  //set snd_cwnd to ssthresh plus 3 * snd_mss after retransmission of lost segment. This artificially inflates the
  //congestion window by the number of segments (three) that have left the network and which the receiver has buffered.
  //This occurs after entering Fast Retransmission Mode for the first time triggered by the third dupack received

  if ((_feature_fast_rc) || (((_feature_nr) && (tcb_block->partial_ack_no == 0)) && (!(tcb_block->partial_ack))))
  //Don´t set snd_cwnd to ssthresh + 3 * snd_mss in case of retransmission triggered by a partial Ack, because in
  //this case snd_cwnd is to be deflated by the amount of new data acknowledged and then to be added by one

    {
      if ((tcb_block->dupacks == 3) && (tcb_block->snd_cwnd == tcb_block->snd_mss))
        {
          tcb_block->snd_cwnd = tcb_block->ssthresh + 3 * tcb_block->snd_mss;
          snd_cwnd_size(tcb_block->snd_cwnd);
          tcb_block->snd_nxt = tcb_block->savenext;
          //send new data if possible after increase in the size of snd_cwnd
          sndDataProcess(tcb_block);
        }
    }
  if (((_feature_nr) && (tcb_block->dupacks >=3)) && ((tcb_block->snd_cwnd == tcb_block->snd_mss) && (tcb_block->partial_ack)))
  //if we have just retransmitted the first unacknowledged segment after a partial ack
    {
      //reset save-variable
      tcb_block->partial_ack = false;

      //Deflate the congestion window by the amount of new data acknowledged, then add back one mss
      //and send a new segment if permitted by the new value of snd_cwnd

      // BCH LYBD
      // tcb_block->snd_cwnd is unsigned integer
      if (tcb_block->savecwnd > tcb_block->new_ack) {
          tcb_block->snd_cwnd = tcb_block->savecwnd - tcb_block->new_ack;
      }
      else {
          tcb_block->snd_cwnd = 0;
      }
      // ECH LYBD

      tcb_block->snd_cwnd += tcb_block->snd_mss;
      snd_cwnd_size(tcb_block->snd_cwnd);
      //reset save-variable
      tcb_block->new_ack = 0;
      tcb_block->snd_nxt = tcb_block->savenext;
      sndDataProcess(tcb_block);
    }
  //Rolf
  if (_feature_fast_rt)
    {
      // if we are in fast retransmit we set snd_nxt to the send queue again
      if (tcb_block->dupacks >= 3)
        tcb_block->snd_nxt = tcb_block->savenext;
    }

  //retrans_queue_size = numBitsInQueue(tcp_retrans_queue);

  if (debug) ev << "End of retransQueueProcess(...) function reached.\n";
}

//function to process the contents of tcp_send_queue
void TcpModule::sndQueueProcess(TcpTcb* tcb_block)
{
  cQueue &  tcp_send_queue    = tcb_block->tcp_send_queue;
  cQueue &  tcp_retrans_queue = tcb_block->tcp_retrans_queue;
  cMessage* pdata_pkt;

  //send data until there is no data left in tcp_send_queue
  while ((tcb_block->seg_len = sndDataSize(tcb_block)) > 0)
    {
      //get data packet from tcp_send_queue
      pdata_pkt = removeFromDataQueue(tcp_send_queue, (tcb_block->seg_len * 8));

      if (pdata_pkt == NULL)
        {
          error( "Unable to get data from send queue");
        }

      //set ACK
      TcpFlag ack = TCP_F_SET;
      //stop the delayed-ack-timer if running
      if (tcb_block->ack_sch == true)
        {
          delete cancelEvent(tcb_block->timeout_delayed_ack_msg);
          tcb_block->ack_sch = false;
        }

      //set URG and/or PSH flags here (if applicable change statement (*) below)
      TcpFlag urg = TCP_F_NSET;
      if (tcb_block->snd_up_valid && seqNoGeq(tcb_block->snd_up, tcb_block->snd_nxt))
        {
          urg = TCP_F_SET;
        }

      TcpFlag psh = TCP_F_NSET;
      if(numBitsInQueue(tcp_send_queue) == 0)
        {
          psh = TCP_F_SET;
        }

      //put slow-start algorithm here

      //part of rtt-processing here
      // if we haven't sent data with timing info yet, we set the timing info
      // we use a kind of time stamp in the tcb here
      // instead of a counter
      if (tcb_block->last_timed_data == -1)
        {
          tcb_block->last_timed_data = simTime();
          tcb_block->rtseq = tcb_block->snd_nxt;
        }

      //update last time a segment was sent
      tcb_block->last_snd_time = simTime();

      //add segment to be sent to tcp_retrans_queue
      insertAtQueueTail(tcp_retrans_queue, (cMessage*) pdata_pkt->dup(), 0);
      unsigned long num_octets_in_rq = numBitsInQueue(tcb_block->tcp_retrans_queue) / 8;
      if (debug) ev << "Number of unacknowledged octets in retransmission queue: " << num_octets_in_rq << ".\n";

      //set retransmission timeout here, if necessary

      //send segment (only ACK flag set at the moment)                                            (*)
      segSend(pdata_pkt, tcb_block, tcb_block->snd_nxt, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, ack, urg, psh);

      //update SND.NXT
      tcb_block->snd_nxt = tcb_block->snd_nxt + tcb_block->seg_len;

      //update SND.MAX
      if (seqNoGeq(tcb_block->snd_nxt, tcb_block->snd_max))
        {
          tcb_block->snd_max = tcb_block->snd_max + tcb_block->seg_len;
        }
    }  // end of while

  //if everything up to the FIN flag has been sent , send a FIN segment as well
  if (tcb_block->snd_fin_valid && (tcb_block->snd_nxt == tcb_block->snd_fin_seq))
    {
      //update SND.MAX
      if (seqNoGeq(tcb_block->snd_nxt, tcb_block->snd_max))
        {
          tcb_block->snd_max++;
        }

      //create data-less FIN msg
      cMessage* msg = new cMessage("FIN");
      msg->setLength(0);
      TcpFlag   fin = TCP_F_SET;
      TcpFlag   ack = TCP_F_SET;

      if (debug) ev << "Inserting FIN segment into retransmission queue.\n";
      //put the FIN msg into the retransmission queue
      insertAtQueueTail(tcp_retrans_queue, (cMessage*) msg->dup(), 0);

      //only ACK and FIN are set                                                       (*)
      segSend(msg, tcb_block, tcb_block->snd_nxt++ , fin, TCP_F_NSET, TCP_F_NSET, ack, TCP_F_NSET, TCP_F_NSET);
    }
}


//function to determine the size of the next data segment to be sent
//(returns 0 if no data should be sent)
unsigned long TcpModule::sndDataSize(TcpTcb* tcb_block)
{
  cQueue &      tcp_send_queue = tcb_block->tcp_send_queue;
  unsigned long snd_queue_size;
  unsigned long total_wnd;
  unsigned long avail_wnd;

  //number of bytes that can be sent
  snd_queue_size = numBitsInQueue(tcp_send_queue) / 8;

  if (snd_queue_size == 0)
    {
      if (debug) ev << "Sendable number of bits (1): " << snd_queue_size  << ".\n";
      return 0;
    }

  //change this, if congestion avoidance is used
  //total_wnd = tcb_block->snd_wnd;
  //send only the minimum of send and congestion window for slow start
  total_wnd = MIN(tcb_block->snd_wnd,tcb_block->snd_cwnd);


  // BCH LYBD
  // FIXME Quick hack for the deadlock problem as follows: Ack arrives and
  // clear the retransmission queue when the advertised receiver window is
  // zero, resulting in no further reading from the send_queue.
  if (tcb_block->snd_wnd == 0 && tcb_block->snd_una == tcb_block->snd_max)
  {
      if (debug) ev << "Receiver window is zero, send one byte to avoid deadlock.\n";
      total_wnd = 1;
  }
  // ECH LYBD

  //available window = total window - amount of data already sent in this window
  if ((tcb_block->snd_una + total_wnd) > tcb_block->snd_nxt)
    {
      avail_wnd = tcb_block->snd_una + total_wnd - tcb_block->snd_nxt;
    }
  else
    {
      //do not use negative window size
      avail_wnd = 0;
    }

  if (debug) ev << "Available window: " << avail_wnd << "\n";
  if (debug) ev << "SND.UNA: " <<tcb_block->snd_una << "\n";
  if (debug) ev << "Total Window: " << total_wnd << "\n";
  if (debug) ev << "SND.NXT: " << tcb_block->snd_nxt << "\n";
  if (debug) ev << "Number of octets in send queue: " << snd_queue_size << "\n";
  if (debug) ev << "MSS: " << tcb_block->snd_mss << "\n";

  //determine segment size to be sent
  if ((avail_wnd >= tcb_block->snd_mss) && (snd_queue_size >= tcb_block->snd_mss))
    {
      if (debug) ev << "Sendable number of bits (2): " << tcb_block->snd_mss * 8 << ".\n";
      return (tcb_block->snd_mss);
    }

  //put Nagle algo. test here

  //if the above is not true, send the minimum
  if (debug) ev << "Sendable number of bits (3): " << (MIN(avail_wnd, snd_queue_size)) * 8  << ".\n";
  return (MIN(avail_wnd, snd_queue_size));
}

//function to determine the size of the next data segment to be sent when
// called from retransQueueProcess
//(returns 0 if no data should be sent)
unsigned long TcpModule::retransDataSize(TcpTcb* tcb_block, cQueue & retrans_queue)
{
  // cQueue &tcp_send_queue = tcb_block->tcp_send_queue;

  unsigned long retrans_queue_size;
  //unsigned long snd_queue_size;
  unsigned long total_wnd;
  unsigned long avail_wnd;

  //number of bytes that can be sent
  retrans_queue_size = numBitsInQueue(retrans_queue) / 8;

  if (retrans_queue_size == 0)
    {
      if (debug) ev << "Sendable number of bits (1): " << retrans_queue_size  << ".\n";
      return 0;
    }

  //change this, if congestion avoidance is used
  //total_wnd = tcb_block->snd_wnd;
  //send only the minimum of send and congestion window for slow start
  total_wnd = MIN(tcb_block->snd_wnd,tcb_block->snd_cwnd);

  //available window = total window - amount of data already sent in this window
  if ((tcb_block->snd_una + total_wnd) > tcb_block->snd_nxt)
    {
      avail_wnd = tcb_block->snd_una + total_wnd - tcb_block->snd_nxt;
    }
  else
    {
      //do not use negative window size
      avail_wnd = 0;
    }

  if (debug) ev << "Available window: " << avail_wnd << "\n";
  if (debug) ev << "SND.UNA: " <<tcb_block->snd_una << "\n";
  if (debug) ev << "Total Window: " << total_wnd << "\n";
  if (debug) ev << "SND.NXT: " << tcb_block->snd_nxt << "\n";
  if (debug) ev << "Number of octets to retransmit currently: " << retrans_queue_size << endl;
  if (debug) ev << "MSS: " << tcb_block->snd_mss << "\n";

  //determine segment size to be sent
  if ((avail_wnd >= tcb_block->snd_mss) && (retrans_queue_size >= tcb_block->snd_mss))
    {
      if (debug) ev << "Sendable number of bits (2): " << tcb_block->snd_mss * 8 << ".\n";
      return (tcb_block->snd_mss);
    }

  //put Nagle algo. test here

  //if the above is not true, send the minimum
  if (debug) ev << "Sendable number of bits (3): " << (MIN(avail_wnd, retrans_queue_size)) * 8  << ".\n";
  return (MIN(avail_wnd, retrans_queue_size));
}

//function to process tcp_socket_queue/tcp_data_receive_queue
void TcpModule::rcvQueueProcess(TcpTcb* tcb_block)
{
  cQueue & tcp_data_receive_queue = tcb_block->tcp_data_receive_queue;
  cQueue & tcp_socket_queue       = tcb_block->tcp_socket_queue;

  cMessage*     ppkt;
  //double pk_size;
  unsigned long num_pks_avail = tcp_socket_queue.length();
  unsigned long rcv_queue_free;
  unsigned long rcv_user;
  int           urgent        = 0;
  //double time_delay;

  //num_pks_avail = tcp_socket_queue.length();

  //keep forwarding data packets to the application until
  //all RECEIVE calls have been satisfied or the receive data
  //queue has been emptied
  if (debug) ev << "Enter rcvQueueProcess. \n";
  if (debug) ev << "num_pks_req in tcb_block: " << tcb_block->num_pks_req << endl;
  if (debug) ev << "TCP Socket Queue Length: " << num_pks_avail << endl;
  while ((tcb_block->num_pks_req > 0 || tcb_block->rcv_up_valid) && num_pks_avail > 0)    //!tcp_socket_queue.empty())
    {
      //remove data packet from the socket queue
      ppkt = (cMessage*) tcp_socket_queue.remove(tcp_socket_queue.head());
      // BCH LYBD
      num_pks_avail -= 1;
      // ECH  LYBD

      //put TCP-TCP-delay/statistics here

      if (debug) ev << "Sequence number: " << tcb_block->rcv_buf_seq << "\n";
      if (debug) ev << "Forwarding data packet to application.\n";

      if (tcb_block->rcv_up_valid && seqNoGeq(tcb_block->rcv_up, tcb_block->rcv_buf_seq))
        {
          urgent = 1;
        }

      //update the sequence number
      tcb_block->rcv_buf_seq = tcb_block->rcv_buf_seq + ppkt->length() / 8;

      //update the number of requests issued by the application
      tcb_block->num_pks_req--;
      //negative numbers are not possible
      if (tcb_block->num_pks_req < 0)
        {
          tcb_block->num_pks_req = 0;
        }

      //set msg kind of ppkt
      ppkt->setKind(TCP_I_SEG_FWD);
      ppkt->setName("TCP_I_SEG_FWD");
      ppkt->addPar("sequence") = tcb_block->rcv_buf_seq;

      if ((tcb_block->num_pks_req == 0 && !tcb_block->rcv_up_valid) || num_pks_avail == 1)
        {
          ppkt->addPar("urgent") = urgent;
          ppkt->addPar("tcp_conn_id") = tcb_block->tb_conn_id; // LYBD
          //forward data packet to the application
          send(ppkt, "to_appl");
        }
      else
        {
          //forward data packet to the application
          ppkt->addPar("tcp_conn_id") = tcb_block->tb_conn_id; // LYBD
          send(ppkt, "to_appl");
        }

      //if all urgent data has been sent to the application
      //the urgent pointer is no longer valid
      if (tcb_block->rcv_up_valid && seqNoGt(tcb_block->rcv_buf_seq, tcb_block->rcv_up))
        {
          tcb_block->rcv_up_valid = 0;
        }

    } //end of while

  //update the window
  //receiver side silly window avoidance (RFC-1122, p.97)
  rcv_user = numBitsInQueue(tcp_data_receive_queue) / 8;
  if (tcb_block->rcv_buff > rcv_user)
    {
      rcv_queue_free = tcb_block->rcv_buff - rcv_user;
      if (rcv_queue_free >= tcb_block->rcv_wnd + MIN((tcb_block->rcv_buff / 2), tcb_block->snd_mss))
        {
          // <Jeroen>
          // if receive window is 0, and there is space in the buffer now, send an ack (window update)
          if (tcb_block->rcv_wnd == 0 && rcv_queue_free != 0)
            {
          tcb_block->rcv_wnd = rcv_queue_free;
          if (debug) ev << "SWA: Set the rcv_wnd to the free space in buffer (" << rcv_queue_free << " octets).\n";
            ackSchedule(tcb_block, true);
            }
          else
            {
            tcb_block->rcv_wnd = rcv_queue_free;
            if (debug) ev << "SWA: Set the rcv_wnd to the free space in buffer (" << rcv_queue_free << " octets).\n";
          }
        }
      //  else if (tcp_socket_queue.empty())
      //  {
      //    tcb_block->rcv_wnd = tcb_block->snd_mss;
      //    if (debug) ev << "SWA: Set the rcv_wnd to maxsegsize.\n";
      //  }
      //    else
      //  {
      //    tcb_block->rcv_wnd = 0;
      //    if (debug) ev << "SWA: Set the rcv_wnd to 0.\n";
      //  }
    }

}


//function to schedule a FIN segment after all pending data has been sent
void TcpModule::finSchedule(TcpTcb* tcb_block)
{
  //a FIN should be sent after all outgoing data has been sent, indicate
  //that a FIN should be sent by setting the variables below

  cQueue & tcp_send_queue = tcb_block->tcp_send_queue;

  tcb_block->snd_fin_valid = 1;

  // LYBD: account for tcp_retrans_queue?
  //tcb_block->snd_fin_seq = tcb_block->snd_nxt + numBitsInQueue(tcp_send_queue) / 8;
  tcb_block->snd_fin_seq = tcb_block->snd_max + numBitsInQueue(tcp_send_queue) / 8;

  //put ACK timer management here
}


//function to send a SYN segment
void TcpModule::synSend(TcpTcb* tcb_block, TcpFlag fin, TcpFlag syn, TcpFlag rst, TcpFlag ack, TcpFlag urg, TcpFlag psh)
{
  //set the initial send sequence number and the send sequence variables
  // if we do a retransmission, we just use the one saved before
  // (if we ever want to simulate data going with SYNs we have to put the
  // syn + data in the retransmission queue...)

  if (tcb_block->rxtshift <= 1)
     tcb_block->iss = (unsigned long) (fmod (simTime() * 250000.0, 1.0 + (double)(unsigned)0xffffffff)) & 0xffffffff;


  tcb_block->snd_una = tcb_block->iss;
  tcb_block->snd_nxt = tcb_block->snd_una + 1;

  //update the maximum send sequence number
  tcb_block->snd_max = tcb_block->snd_una + 1;

  //create a data-less packet
  cMessage* msg = new cMessage("SYN_DATA", SYN_DATA);
  msg->setLength(0);

  msg->addPar("num_bit_req") = tcb_block->num_bit_req;

  //set the SYN flag
  syn = TCP_F_SET;

  //send the SYN segment to the remote TCP
  if (debug) ev << "Sending a SYN segment to remote TCP.\n";
  segSend(msg, tcb_block, tcb_block->iss, fin, syn, rst, ack, urg, psh);


  //schedule retransmission timeout here
}


//function to notify the application process that the TCP connection has been aborted
void TcpModule::connAbort(cMessage* msg, TcpTcb* tcb_block)
{
  //check if application has already been notified
  if (tcb_block->tcp_app_notified == false)
    {
      tcb_block->tcp_app_notified = true;

      //cancel the time wait timer, if one is set
      if ((tcb_block->time_wait_sch == true)) // && (tcb_block->st_event.event == TCP_E_TIMEOUT_TIME_WAIT))
        {

          if (debug) ev << "Cancelling Time-Wait timer due to a connection ABORT.\n";
          delete cancelEvent(msg);
          tcb_block->time_wait_sch = false;
        }

      //send ABORT to application process
      tcb_block->status_info = TCP_I_ABORTED;
      cMessage* abort_msg = new cMessage("TCP_I_ABORTED", TCP_I_ABORTED);

      //add connection ID parameter to abort_msg
      abort_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

      //add address and port information (note that they are switched for correct
      //address and port management at the application layer)
      abort_msg->addPar("src_port")  = tcb_block->rem_port;
      abort_msg->addPar("src_addr")  = tcb_block->rem_addr;
      abort_msg->addPar("dest_port") = tcb_block->local_port;
      abort_msg->addPar("dest_addr") = tcb_block->local_addr;

      if (debug) ev << "TCP notifies the application that the connection has been ABORTED.\n";
      send(abort_msg, "to_appl");
    }
}


//function to notify the application process that the TCP connection is open
void TcpModule::connOpen(TcpTcb* tcb_block)
{
  //send ESTABLISHED notification to application process
  tcb_block->status_info = TCP_I_ESTAB;
  cMessage* estab_msg = new cMessage("TCP_I_ESTAB", TCP_I_ESTAB);

  //add connection ID parameter to estab_msg
  estab_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

  //add MSS
  estab_msg->addPar("mss") = tcb_block->snd_mss;

  //add address and port information (note that they are switched for correct
  //address and port management at the application layer)
  estab_msg->addPar("src_port")  = tcb_block->rem_port;
  estab_msg->addPar("src_addr")  = tcb_block->rem_addr;
  estab_msg->addPar("dest_port") = tcb_block->local_port;
  estab_msg->addPar("dest_addr") = tcb_block->local_addr;

  if (debug) ev << "TCP notifies the application that the connection has been ESTABLISHED.\n";
  send(estab_msg, "to_appl");

  //set connection establishment variable
  tcb_block->conn_estab = 1;
}


//function to notify the application that TCP reached CLOSE_WAIT state
void TcpModule::connCloseWait(TcpTcb* tcb_block)
{
  //send CLOSE_WAIT notification to application process
  tcb_block->status_info = TCP_I_CLOSE_WAIT;
  cMessage* closewait_msg = new cMessage("TCP_I_CLOSE_WAIT", TCP_I_CLOSE_WAIT);

  //add connection ID parameter to estab_msg
  closewait_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

  //add address and port information (note that they are switched for correct
  //address and port management at the application layer)
  closewait_msg->addPar("src_port")  = tcb_block->rem_port;
  closewait_msg->addPar("src_addr")  = tcb_block->rem_addr;
  closewait_msg->addPar("dest_port") = tcb_block->local_port;
  closewait_msg->addPar("dest_addr") = tcb_block->local_addr;

  if (debug) ev << "TCP notifies the application that TCP entered CLOSE_WAIT.\n";
  send(closewait_msg, "to_appl");
}


//function to notify the application that TCP received a SYN-Segment
void TcpModule::connRcvdSyn(TcpTcb* tcb_block)
{
  //send ESTABLISHED notification to application process
  tcb_block->status_info = TCP_I_RCVD_SYN;
  cMessage* rcvd_syn_msg = new cMessage("TCP_I_RCVD_SYN", TCP_I_RCVD_SYN);

  //add connection ID parameter to rcvd_syn_msg
  rcvd_syn_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

  //add num_bit_req as parameter
  rcvd_syn_msg->addPar("num_bit_req") = tcb_block->num_bit_req;

  //add address and port information (note that they are switched for correct
  //address and port management at the application layer)
  rcvd_syn_msg->addPar("src_port")  = tcb_block->rem_port;
  rcvd_syn_msg->addPar("src_addr")  = tcb_block->rem_addr;
  rcvd_syn_msg->addPar("dest_port") = tcb_block->local_port;
  rcvd_syn_msg->addPar("dest_addr") = tcb_block->local_addr;

  if (debug) ev << "TCP notifies the application that a SYN has been received.\n";
  send(rcvd_syn_msg, "to_appl");
}


//function to notify the application that TCP entered the CLOSED state
void TcpModule::connClosed(TcpTcb* tcb_block)
{
  //if connection establishment variable is still 1, send notification to application,
  //if not send nothing (the application might have already been notified, see enter TIME_WAIT)
  if (tcb_block->conn_estab == 1 || tcb_block->syn_rcvd == 0)
    {
      //send CLOSED notification to application process
      tcb_block->status_info = TCP_I_CLOSED;
      cMessage* closed_msg = new cMessage("TCP_I_CLOSED", TCP_I_CLOSED);

      //add connection ID parameter to estab_msg
      closed_msg->addPar("tcp_conn_id") = tcb_block->tb_conn_id;

      //add address and port information (note that they are switched for correct
      //address and port management at the application layer)
      closed_msg->addPar("src_port")  = tcb_block->rem_port;
      closed_msg->addPar("src_addr")  = tcb_block->rem_addr;
      closed_msg->addPar("dest_port") = tcb_block->local_port;
      closed_msg->addPar("dest_addr") = tcb_block->local_addr;

      if (debug) ev << "TCP notifies the application that TCP entered CLOSED.\n";
      send(closed_msg, "to_appl");

      //reset the connection establishment variable
      tcb_block->conn_estab = 0;
    }
}


//function to remove a specified number of bits from a queue and put them into a data packet
cMessage* TcpModule::removeFromDataQueue(cQueue & from_queue,  unsigned long number_of_bits_to_remove)
{
  cMessage*     fdata_packet        = NULL; // LYBD: fix memory leaks
  cMessage*     rdata_packet        = new cMessage;
  unsigned long bit_size_from_queue = numBitsInQueue(from_queue);
  unsigned long fdp_len;
  unsigned long rdp_len             = 0;

  if (bit_size_from_queue == 0)
    {
      if (debug) ev << "No data available in the queue specified. Returning NULL-pointer.\n";
      delete rdata_packet;
      rdata_packet = NULL;

      return rdata_packet;
    }
  else if (number_of_bits_to_remove > bit_size_from_queue)
    {
      if (debug) ev << "Cannot remove more bits from queue than there are in the queue. Removing available bits only.\n";
      number_of_bits_to_remove = bit_size_from_queue;
    }

  rdata_packet->parList().add(new cArray("msg_list", 1));

  while (!from_queue.empty() && number_of_bits_to_remove > 0)
    {
        fdata_packet = (cMessage*) from_queue.remove(from_queue.head());
        fdp_len = fdata_packet->length();

        // code added by Sebastian Klapp
        // for packets coming from tcp-retrans-queue
        // check if fdata is already packed
        // BCH LYBD
        if(fdata_packet->hasPar("msg_list"))
        {
            // fdata_packet=(cMessage*) ((cArray*)
            // (fdata_packet->parList().get("msg_list")))->get(0);

            // rewritten by LYBD
            cMessage* pdata_packet = (cMessage*)
                ((cArray*)(fdata_packet->parList().get("msg_list")))->get(0);
            ((cArray*)(fdata_packet->parList().get("msg_list")))->remove(0);
            delete fdata_packet;
            fdata_packet = pdata_packet;
            fdata_packet->setLength(fdp_len);
        }
        ((cArray*) (rdata_packet->parList().get("msg_list")))->add(fdata_packet);
        // fdata_packet = (cMessage*) from_queue.remove(from_queue.head());
        //    ((cArray*) (rdata_packet->parList().get("msg_list")))->add(fdata_packet);
        //fdp_len = fdata_packet->length();

        // store original length in case it gets decremented due to partial packet
        // retrieval
        if (fdata_packet->findPar("orig_length") == -1)
            fdata_packet->addPar("orig_length") = fdp_len;

        //copy the parameters of fdata_packet to rdata_packet
        //(the important parameters should stay the same for all
        //fdata_packets in the queue, if not the parameters of the
        //last fdat_packet used will be valid)
        //rdata_packet = (cMessage*) fdata_packet->dup();

        if (fdp_len > number_of_bits_to_remove)
        {
            //duplicate packet and put one copy back into queue for next retrieval
            cMessage* res_data_packet = (cMessage*) fdata_packet->dup();
            fdata_packet->addPar("complete_pkt") = 0;
            fdata_packet->setLength(number_of_bits_to_remove);
            rdp_len += number_of_bits_to_remove;
            res_data_packet->addLength(-number_of_bits_to_remove);
            from_queue.insertHead(res_data_packet);
            number_of_bits_to_remove = 0;
        }
        // ECH LYBD
        else // (fdp_len <= number_of_bits_to_remove)
        {
            fdata_packet->addPar("complete_pkt") = 1;
            rdp_len += fdp_len;
            number_of_bits_to_remove -= fdp_len;
        }
    }

    rdata_packet->setLength(rdp_len);
    return rdata_packet;
}



//function to transfer a specified number of octets from one queue to another
void TcpModule::transferQueues(cQueue & from_queue, cQueue & to_queue, unsigned long number_of_octets_to_transfer)
{
  unsigned long octet_size_from_queue = numBitsInQueue(from_queue) /  8;
  cMessage*     fqdata_packet;
  cMessage*     tqdata_packet;
  unsigned long fqdata_packet_len_octets;


  if (octet_size_from_queue == 0)
    {
      error( "No data in source queue. Cannot transfer any octets");
    }
  else if (number_of_octets_to_transfer > octet_size_from_queue)
    {
      if (debug) ev << "Cannot transfer more octets than there are in the source queue. Transferring available octets only.\n";
      number_of_octets_to_transfer = octet_size_from_queue;
    }

  while (!from_queue.empty() && number_of_octets_to_transfer > 0)
    {
      fqdata_packet = (cMessage*) from_queue.remove(from_queue.head());
      fqdata_packet_len_octets = fqdata_packet->length() / 8;

      if (fqdata_packet_len_octets > number_of_octets_to_transfer)
        {
          tqdata_packet = (cMessage*) fqdata_packet->dup();
          tqdata_packet->setLength(number_of_octets_to_transfer * 8);
          fqdata_packet->addLength(- number_of_octets_to_transfer * 8);
          from_queue.insertHead(fqdata_packet);
          insertAtQueueTail(to_queue, tqdata_packet, 0);
          number_of_octets_to_transfer = 0;
        }
      else if (fqdata_packet_len_octets == number_of_octets_to_transfer)
        {
          insertAtQueueTail(to_queue, fqdata_packet, 0);
          number_of_octets_to_transfer = 0;
        }
      else
        {
          insertAtQueueTail(to_queue, fqdata_packet, 0);
          number_of_octets_to_transfer -= fqdata_packet_len_octets;
        }

    }
}

//function to copy a specified number of octets from one queue to another
void TcpModule::copyQueues(cQueue & from_queue, cQueue & to_queue, unsigned long number_of_octets_to_copy)
{

  unsigned long octet_size_from_queue = numBitsInQueue(from_queue) /  8;
  cMessage*     fqdata_packet;
  cMessage*     tqdata_packet;
  unsigned long fqdata_packet_len_octets;


  if (octet_size_from_queue == 0)
    {
      error( "No data in source queue. Cannot copy any octets");
    }
  else if (number_of_octets_to_copy > octet_size_from_queue)
    {
      if (debug) ev << "Cannot copy more octets than there are in the source queue. Copying available octets only.\n";
      number_of_octets_to_copy = octet_size_from_queue;
    }

  for (cQueueIterator iter(from_queue,1); !iter.end() && (number_of_octets_to_copy > 0); iter++ )
    {
      fqdata_packet = (cMessage*) iter();
      fqdata_packet_len_octets = fqdata_packet->length() / 8;

      if (fqdata_packet_len_octets > number_of_octets_to_copy)
        {
          tqdata_packet = (cMessage*) fqdata_packet->dup();
          tqdata_packet->setLength(number_of_octets_to_copy * 8);
          insertAtQueueTail(to_queue, tqdata_packet, 0);
          number_of_octets_to_copy = 0;
        }
      else if (fqdata_packet_len_octets == number_of_octets_to_copy)
        {
          tqdata_packet = (cMessage*) fqdata_packet->dup();
          insertAtQueueTail(to_queue, tqdata_packet, 0);
          number_of_octets_to_copy = 0;
        }
      else
        {
          tqdata_packet = (cMessage*) fqdata_packet->dup();
          insertAtQueueTail(to_queue, tqdata_packet, 0);
          number_of_octets_to_copy -= fqdata_packet_len_octets;
        }

    }
}

//function to flush a specified number of bits of the labeled data packet that is a part of the given queue
void TcpModule::flushLabeledDataQueue(cQueue & queue_to_flush, unsigned long label, unsigned long number_of_bits_to_flush)
{
  cMessage*     qdata = NULL;
  unsigned long qlabel;
  unsigned long qdata_length;

  for (cQueueIterator qiterator(queue_to_flush, 1); !qiterator.end(); qiterator++)
    {
      qdata = (cMessage*) qiterator();
      qlabel = qdata->par("label");
      if (qlabel == label)
        {
          break;
        }
    }

  if (qdata == NULL)
    {
      error("No data packet with specified label found in the queue searched");
    }
  else
    {
      qdata_length = qdata->length();
      if (qdata_length < number_of_bits_to_flush)
        {
          error("Cannot flush more bits than there are in the labeled data packet");
        }
      else
        {
          qdata->addLength(- number_of_bits_to_flush);
        }
    }
}


// FIXME: UNTESTED
// FIXME: MBI   Seems to me that argument position might be used in a wrong way
// FIXME: MBI   This is true also for removeListElement() accessListElement()
// FIXME: MBI   This function can be called now with position = 1..ilist_len+1
//              so we have covered all cases
//function to insert an element in a list at "position-th" position
void TcpModule::insertListElement(cLinkedList & ilist, SegRecord* ielement, unsigned long position)
{
  SegRecord* pos_element = NULL; // LYBD
  unsigned long ilist_len = ilist.length();

  if (position == 1) //FIXME: MBI but needed for length of list == 0
  {
      if (debug) ev << "Specified position is 1. Inserting element at head.\n";
      ilist.insertHead(ielement);
    }

  else if (position > ilist_len) // FIXME: MBI should never be true
    {
      if (debug) ev << "Specified position is greater than the length of the list."
        "Inserting element at the tail of the list.\n";
      pos_element = (SegRecord*) ilist.peekTail();
      ilist.insertAfter(pos_element, ielement);  //insertAfter() !!
    }
  // else if (position == 1)
  //{
  //    if (debug) ev << "Specified position is 1. Inserting element at head.\n";
  //    ilist.insertHead(ielement);
  //  }
  //else if (position == 0)
  //  {
  //    ilist.insertHead(ielement);
  //  }
  else if (position == (unsigned long) ilist_len) // FIXME: MBI insertAfter()?
    {
      pos_element = (SegRecord*) ilist.peekTail();
      ilist.insertBefore(pos_element, ielement); //insertBefore() !!
    }
  else
    {
      unsigned long counter = 1;
      cLinkedListIterator literator(ilist, 1);
      //move to the desired position in the list
      while(counter < position) // fixed by LYBD
        {
          literator++;
          counter++;
        }
      //access the desired list element
      pos_element = (SegRecord*) literator();
      ilist.insertBefore(pos_element, ielement); //insertBefore() !!
    }
}


// FIXME: UNTESTED
// FIXME: MBI
//function to remove the "position-th" element from the specified list
void TcpModule::removeListElement(cLinkedList & list_to_inspect, unsigned long position)
{
  SegRecord* list_element = NULL; // LYBD

  if (position > (unsigned long) list_to_inspect.length())
    {
      if (debug) ev << "Specified position is greater than the length of the inspected queue. Unable to remove any element.\n";
    }
  else if (position == 1) // FIXME: MBI should be 0
    {
      if (debug) ev << "Specified position is 1.\n";
      list_element = (SegRecord*) list_to_inspect.remove(list_to_inspect.head());

      delete list_element;
    }
  else if (position == (unsigned long) list_to_inspect.length()) // FIXME: MBI should be (list_to_inspect.length() - 1)
    {

      list_element = (SegRecord*)  list_to_inspect.getTail();
      delete list_element;
    }
  else
    {
      unsigned long counter = 1;
      cLinkedListIterator literator(list_to_inspect, 1);
      //move to the desired position in the list
      while(counter < position) // fixed by LYBD
        {
          literator++;
          counter++;
        }
      //get the desired list element
      list_element = (SegRecord*) list_to_inspect.remove(literator());
      delete list_element;
    }
}

// FIXME: UNTESTED
// FIXME: MBI position should start with 0
//function to access the "position-th" element of the list
SegRecord* TcpModule::accessListElement(cLinkedList & list_to_inspect, unsigned long position)
{
  SegRecord* list_element = NULL; // LYBD

  if (position > (unsigned long) list_to_inspect.length())
    {
      if (debug) ev << "Specified position is greater than the length of the inspected queue. Returning NULL-pointer.\n";
      list_element = NULL;
    }
  else if (position == 1)
    {
      list_element = (SegRecord*) list_to_inspect.peekHead();
    }
  else if (position == (unsigned long) list_to_inspect.length())
    {
      list_element = (SegRecord*) list_to_inspect.peekTail();
    }
  else
    {
      unsigned long counter = 1;
      cLinkedListIterator literator(list_to_inspect, 1);
      //move to the desired position in the list
      while(counter < position) // fixed by LYBD
        {
          literator++;
          counter++;
        }
      //access the desired list element
      list_element = (SegRecord*) literator();
    }
  return list_element;
}


//function to determine the size of a queue in bits
unsigned long TcpModule::numBitsInQueue(cQueue &  queue_to_inspect)
{
  cQueueIterator qiterator(queue_to_inspect, 1);
  cMessage*      queue_element;
  unsigned long  size_of_queue = 0;

  if (queue_to_inspect.length() > 0)
    {
      //take all elements into account except the last one
      while (!qiterator.end())
        {
          queue_element = (cMessage *) qiterator();
          size_of_queue += queue_element->length();
          qiterator++;
        }
      //get the last element in the queue
      //if (qiterator.end())
      //{
      //  queue_element = (cMessage *) qiterator();
      //  size_of_queue += queue_element->length();
      //}
    }

  return size_of_queue;
}


//function to insert a msg at the end of a queue
//"label" can be used to identify a specific msg
void TcpModule::insertAtQueueTail(cQueue & iqueue, cMessage *msg, unsigned long label)
{
  if (iqueue.empty())
    {
      msg->addPar("label") = label;
      iqueue.insertHead(msg);
    }
  else
    {
      cMessage* currentTail = (cMessage*) iqueue.peekTail();
      msg->addPar("label") = label;
      iqueue.insertAfter(currentTail, msg);
    }
}


//function to flush a queue:
//tcp_seg_queue = true if TCP segments are stored in the queue, false if TCP data (without TCP header)
void TcpModule::flushQueue(cQueue & queue_to_flush, unsigned long number_of_octets_to_flush, bool tcp_seg_queue)
{
  unsigned long num_octets_in_rq = numBitsInQueue(queue_to_flush) / 8;
  if (debug) ev << "Number of octets in queue to flush: " << num_octets_in_rq << "\n";
  unsigned long num_packets_in_rq = queue_to_flush.length();
  if (debug) ev << "Number of data packets in queue to flush: " << num_packets_in_rq << "\n";

  while (!queue_to_flush.empty() && number_of_octets_to_flush > 0) //!queue_to_flush.empty()
    {
      cMessage* qseg;
      cMessage* qdata;

      if (tcp_seg_queue == true)
        {
          qseg = (cMessage*) queue_to_flush.peekHead();
          //get pointer to encapsulated data
          qdata = qseg->encapsulatedMsg();
        }
      else
        {
          qdata = (cMessage*) queue_to_flush.peekHead();
        }

      //note: the length  of qdata is measured in bits
      if ((unsigned long) (qdata->length()) / 8 > number_of_octets_to_flush)
        {
          qdata->addLength(- number_of_octets_to_flush * 8);

          if (tcp_seg_queue == true)
            {
              qseg->par("seg_len") = (long unsigned int) qseg->par("seg_len") - number_of_octets_to_flush;
            }

          number_of_octets_to_flush = 0;
        }
      else if ((unsigned long) (qdata->length()) / 8 == number_of_octets_to_flush)
        {
          if (tcp_seg_queue == true)
            {
              qseg = (cMessage*) queue_to_flush.remove(queue_to_flush.head());
              delete qseg;
            }
          else
            {
              qdata = (cMessage*) queue_to_flush.remove(queue_to_flush.head());
              delete qdata;
            }

          number_of_octets_to_flush = 0;
        }
      else
        {
          //unsigned long qdata_octets1 = qdata->length() / 8;
          number_of_octets_to_flush -= qdata->length() / 8;

          if (tcp_seg_queue == true)
            {
              qseg = (cMessage*) queue_to_flush.remove(queue_to_flush.head());
              delete qseg;
            }
          else
            {
              qdata = (cMessage*) queue_to_flush.remove(queue_to_flush.head());
              //can be deleted after debugging
              //unsigned long qdata_octets2 = qdata->length() / 8;
              delete qdata;
            }
        }

      num_octets_in_rq = numBitsInQueue(queue_to_flush) / 8;
      if (debug) ev << "Number of octets in queue to flush: " << num_octets_in_rq << "\n";

    } //end of while
}


//function to send a data-less ACK to the remote TCP if the referring timer expires
void TcpModule::ackSchedule(TcpTcb* tcb_block, bool immediate)
{
  //time when the next delayed ACK segment is to be sent
  double next_del_ack_time   = 0;
  //current time
  double time                = 0;
  //maximum delay before a data-less ack is sent
  //max_del_ack is set to 0 == > only data-less ACKs are sent
  //since no timer has been implemented so far
  double max_del_ack         = tcb_block->max_del_ack; //= 0.0 at the moment, default is 0.2


  //if not already done, schedule a data-less ACK segment now
  //if (tcb_block->ack_sch != true)
  //  {
      time = simTime();

      if (immediate == true)
        {
      // <Jeroen>
          // reset delack packet counter
          cnt_packet_delack = 0;
      // </Jeroen>

          //create a data-less segment and send it at once
          cMessage* msg = new cMessage("ACK_DATA", ACK_DATA);
          msg->setLength(0);

          //set flags
          TcpFlag fin = TCP_F_NSET;
          TcpFlag syn = TCP_F_NSET;
          TcpFlag rst = TCP_F_NSET;
          TcpFlag ack = TCP_F_SET;
          TcpFlag urg = TCP_F_NSET;
          TcpFlag psh = TCP_F_NSET;

          // stop the delayed ack timer, if still running
          if (tcb_block->ack_sch == true)
            {
              delete cancelEvent(tcb_block->timeout_delayed_ack_msg);
              tcb_block->ack_sch = false;
            }

          //send the data-less ACK segment to remote TCP
          segSend(msg, tcb_block, tcb_block->snd_nxt, fin, syn, rst, ack, urg, psh);
        }
      else
        {
          //put code to schedule the ACK-timer here
          next_del_ack_time = (ceil(time / max_del_ack) + 1) * max_del_ack;

          //schedule ACK
          //scheduleAt ...
          if (tcb_block->ack_sch == false)
            {
              tcb_block->timeout_delayed_ack_msg = new cMessage("TIMEOUT_DELAYED_ACK",TIMEOUT_DELAYED_ACK);
              //add parameters for function tcpArrivalMsg(...) here
              tcb_block->timeout_delayed_ack_msg->addPar("src_port") = tcb_block->local_port;
              tcb_block->timeout_delayed_ack_msg->addPar("src_addr") = tcb_block->local_addr;
              tcb_block->timeout_delayed_ack_msg->addPar("dest_port") = tcb_block->rem_port;
              tcb_block->timeout_delayed_ack_msg->addPar("dest_addr") = tcb_block->rem_addr;

              scheduleAt(next_del_ack_time, tcb_block->timeout_delayed_ack_msg);
              tcb_block->ack_sch = true;
            }
          //timeoutDatalessAck(tcb_block);
        }
      // }

}


//function to give notification of outstanding requests (RECEIVE calls)
//issued by the appl. which have not been satisfied yet
void TcpModule::outstRcv(TcpTcb* tcb_block)
{
  //outstanding RECEIVES
  if (tcb_block->num_pks_req > 0)
    {
      if (debug) ev << "There are outstanding RECEIVE application calls.\n";
      if (debug) ev << "Still " << tcb_block->num_pks_req << " outstanding requests.\n";
      if (tcb_block->st_event.event == TCP_E_CLOSE)
        {
          if (debug) ev << "Error: Connection closing.\n";
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          if (debug) ev << "Error: Connection reset.\n";
        }
    }
}


//function to give notification of queued SEND calls
void TcpModule::outstSnd(TcpTcb* tcb_block)
{
  //queued SEND calls, that is, segments are still in tcp_send_queue
  if (!tcb_block->tcp_send_queue.empty())
    {
      if (debug) ev << "There are outstanding SEND application calls.\n";
      if (tcb_block->st_event.event == TCP_E_CLOSE)
        {
          if (debug) ev << "Error: Connection closing.\n";
        }
      else if (tcb_block->st_event.event == TCP_E_ABORT)
        {
          if (debug) ev << "Error: Connection reset.\n";
        }
    }
}

//function to flush the transmission and retransmission queues
void TcpModule::flushTransRetransQ(TcpTcb* tcb_block)
{
  if (!tcb_block->tcp_send_queue.empty())
    {
      if (debug) ev << "SEND commands from appl. are still in tcp_send_queue.\n";
      if (debug) ev << "Error: Connection reset.\n";
      if (debug) ev << "Flushing tcp_send_queue.\n";
      tcb_block->tcp_send_queue.clear();
    }
  if (!tcb_block->tcp_retrans_queue.empty())
    {
      if (debug) ev << "Segments are still in tcp_retrans_queue.\n";
      if (debug) ev << "Flushing tcp_retrans_queue.\n";
      tcb_block->tcp_retrans_queue.clear();
    }
}


//function to print out TCB status information
void TcpModule::printStatus(TcpTcb* tcb_block, const char *label)
{
  if (debug) ev << "=========== " << label << " ===========\n";
  if (debug) ev << "Connection ID:  " << (int) tcb_block->tb_conn_id << endl;
  if (debug) ev << "State:          " << (int) tcb_block->fsm.state()
                << " (" << stateName((TcpState)tcb_block->fsm.state()) << ")" << endl;
  if (debug) ev << "Local ip:port:  " << tcb_block->local_addr << ":" << tcb_block->local_port << endl;
  if (debug) ev << "Remote ip:port: " << tcb_block->rem_addr << ":" << tcb_block->rem_port << endl;
  if (debug) ev << "Previous state: " << (int) tcb_block->tb_prev_state
                << " (" << stateName(tcb_block->tb_prev_state) << ")" << endl;
  if (debug) ev << endl;
  if (debug) ev << "Number of bits to be transferred: " << (int) tcb_block->num_bit_req << endl;
  if (debug) ev << "Timeout: " << tcb_block->timeout << " (not implemented)" << endl;
  if (debug) ev << "Send unacknowledged: " << tcb_block->snd_una << endl;
  if (debug) ev << "Send next: " << tcb_block->snd_nxt << endl;
  if (debug) ev << "Send urgent pointer: " << tcb_block->snd_up << endl;
  if (debug) ev << "Send window: " << tcb_block->snd_wnd << endl;
  if (debug) ev << "Congestion window: " << tcb_block->snd_cwnd << endl;
  if (debug) ev << "Slow-start threshold: " << tcb_block->ssthresh << endl;
  if (debug) ev << "Congestion window counter: " << tcb_block->cwnd_cnt << endl;
  if (debug) ev << "Seg. seq. no. for last window update: " << tcb_block->snd_wl1 << endl;
  if (debug) ev << "Seg. ack. no. for last window update: " << tcb_block->snd_wl2 << endl;
  if (debug) ev << "ISS: " << tcb_block->iss << endl;
  if (debug) ev << "Receive next: " << tcb_block->rcv_nxt << endl;
  if (debug) ev << "Receive window: " << tcb_block->rcv_wnd << endl;
  if (debug) ev << "Receive urgent pointer: " << tcb_block->rcv_up << endl;
  if (debug) ev << "IRS: " << tcb_block->irs << endl;
  if (debug) ev << "Send maximum: " << tcb_block->snd_max << endl;
  if (debug) ev << "Smoothed RTT (ticks*8): " << tcb_block->srtt << endl;
  if (debug) ev << "Smoothed mdev of RTT (ticks*4): " << tcb_block->rttvar << endl;
  if (debug) ev << "RTO value : " << tcb_block->rxtcur << endl;
  if (debug) ev << "================\n";
}

// produce readable name
const char *TcpModule::stateName(TcpState state)
{
#define CASE(x) case x: s=#x; break
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

// produce readable name
const char *TcpModule::eventName(TcpEvent event)
{
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event)
    {
        CASE(TCP_E_NONE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
        CASE(TCP_E_SEND);
        CASE(TCP_E_RECEIVE);
        CASE(TCP_E_CLOSE);
        CASE(TCP_E_ABORT);
        CASE(TCP_E_STATUS);
        CASE(TCP_E_SEG_ARRIVAL);
        CASE(TCP_E_RCV_SYN);
        CASE(TCP_E_RCV_SYN_ACK);
        CASE(TCP_E_RCV_ACK_OF_SYN);
        CASE(TCP_E_RCV_FIN);
        CASE(TCP_E_RCV_ACK_OF_FIN);
        CASE(TCP_E_RCV_FIN_ACK_OF_FIN);
        CASE(TCP_E_TIMEOUT_TIME_WAIT);
        CASE(TCP_E_TIMEOUT_REXMT);
        CASE(TCP_E_TIMEOUT_PERSIST);
        CASE(TCP_E_TIMEOUT_KEEPALIVE);
        CASE(TCP_E_TIMEOUT_CONN_ESTAB);
        CASE(TCP_E_TIMEOUT_FIN_WAIT_2);
        CASE(TCP_E_TIMEOUT_DELAYED_ACK);
        CASE(TCP_E_PASSIVE_RESET);
        CASE(TCP_E_ABORT_NO_RST);
    }
    return s;
#undef CASE
}

//function to do exit INIT state processing
void TcpModule::procExInit(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch (tcb_block->st_event.event)
    {
      //if appl. passive open: set up TCB, go to LISTEN
    case TCP_E_OPEN_PASSIVE:
      if (debug) ev << "TCP received PASSIVE OPEN command from application.\n";

      tcb_block->local_port = amsg->par("src_port");
      tcb_block->local_addr = amsg->par("src_addr");

      tcb_block->tb_conn_id = amsg->par("tcp_conn_id");
      tcb_block->timeout    = amsg->par("timeout");

      if (amsg->findPar("num_bit_req") != -1)
        {
          tcb_block->num_bit_req = amsg->par("num_bit_req");
        }

      //print TCB status information
      //printStatus(tcb_block, "Connection state");

      break;

      //if appl. active open: set up TCB, send SYN, go to SYN_SENT
    case TCP_E_OPEN_ACTIVE:
      if (debug) ev << "TCP received ACTIVE OPEN command from application.\n";
      tcb_block->local_port = amsg->par("src_port");
      tcb_block->local_addr = amsg->par("src_addr");
      tcb_block->rem_port   = amsg->par("dest_port");
      tcb_block->rem_addr   = amsg->par("dest_addr");


      if (tcb_block->rem_port == -1 || tcb_block->rem_addr == -1)
        {
          error( "Error using ACTIVE OPEN: foreign socket unspecified");
        }

      tcb_block->timeout = amsg->par("timeout");

      if (amsg->findPar("num_bit_req") != -1)
        {
          tcb_block->num_bit_req = amsg->par("num_bit_req");
        }

      if (debug) ev << "Number of bits requested by the application: " << tcb_block->num_bit_req << ".\n";

      break;

    case TCP_E_SEND:
      error ("TCP received SEND command from appl. while in CLOSED/INIT.");
      break;

    case TCP_E_RECEIVE:
      error ("TCP received RECEIVE command from appl. while in CLOSED/INIT.\n");
      break;

    case TCP_E_CLOSE:
      error ("TCP received CLOSE command from appl. while in CLOSED/INIT.\n");
      break;

    case TCP_E_ABORT:
      error ("TCP received ABORT command from appl. while in CLOSED/INIT.\n");
      break;

    case TCP_E_STATUS:
      error ("TCP received STATUS command from appl. while in CLOSED/INIT.\n");
      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in INIT/CLOSED state.\n";

      //if incoming seg. does not contain a RST, a RST is sent in response
      //tcp_header is the header of the incoming segment amsg
      if (tcp_header->th_flag_rst == TCP_F_NSET)
        {
          //create a data-less RST packet
          cMessage* msg  = new cMessage("RST_DATA", RST_DATA);
          msg->setLength(0);

          //sequence number etc. if ACK bit is off
          if (tcp_header->th_flag_ack == TCP_F_NSET)
            {
              unsigned long seq_no = 0;
              //RCV.NXT is used in segSend(...) below to set the ACK number of the TCP header
              tcb_block->rcv_nxt = tcp_header->th_seq_no + (unsigned long) amsg->par("seg_len");
              //set ACK flag
              TcpFlag ack = TCP_F_SET;
              //set RST flag
              TcpFlag rst = TCP_F_SET;
              if (debug) ev << "Sending RST segment to remote TCP while in CLOSED/INIT state.\n";
              segSend(msg, tcb_block, seq_no, TCP_F_NSET, TCP_F_NSET, rst, ack, TCP_F_NSET, TCP_F_NSET);
            }
          //if ACK bit is on
          else if (tcp_header->th_flag_rst == TCP_F_SET)
            {
              unsigned long seq_no = tcp_header->th_ack_no;
              //set RST flag
              TcpFlag rst = TCP_F_SET;
              if (debug) ev << "Sending RST segment to remote TCP while in CLOSED/INIT state.\n";
              segSend(msg, tcb_block, seq_no, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
            }
        }

      break;

    default:
      //error( "Case not handled in switch statement (INIT)");
      error("Event %s not handled in INIT state switch statement", eventName(tcb_block->st_event.event));

      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_OPEN_PASSIVE:
      if (debug) ev << "TCP is going to LISTEN state.\n";
      FSM_Goto(fsm, TCP_S_LISTEN);

      break;

    case TCP_E_OPEN_ACTIVE:
      //send SYN segment
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);

      // set the connection-establishment timer to 75 seconds
      tcb_block->timeout_conn_estab_msg = new cMessage("TIMEOUT_CONN_ESTAB",TIMEOUT_CONN_ESTAB);
      //add parameters for function tcpArrivalMsg(...) here
      tcb_block->timeout_conn_estab_msg->addPar("src_port")  = tcb_block->local_port;
      tcb_block->timeout_conn_estab_msg->addPar("src_addr")  = tcb_block->local_addr;
      tcb_block->timeout_conn_estab_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_conn_estab_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + CONN_ESTAB_TIMEOUT_VAL, tcb_block->timeout_conn_estab_msg);

      if (debug) ev << "TCP is going to SYN_SENT state.\n";
      FSM_Goto(fsm, TCP_S_SYN_SENT);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit LISTEN state processing
void TcpModule::procExListen(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl active open: send SYN, go to SYN_SENT
    case TCP_E_OPEN_ACTIVE:
      if (debug) ev << "TCP received ACTIVE OPEN command from appl. while in LISTEN.\n";

      tcb_block->local_port = amsg->par("src_port");
      tcb_block->local_addr = amsg->par("src_addr");
      tcb_block->rem_port   = amsg->par("dest_port");
      tcb_block->rem_addr   = amsg->par("dest_addr");

      if (tcb_block->rem_port==-1 || tcb_block->rem_addr==-1)
        {
          error( "ACTIVE OPEN: foreign socket unspecified");
        }

      tcb_block->timeout = amsg->par("timeout");

      break;

      //if appl send: send SYN, go to SYN_SENT
    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in LISTEN.\n";

      tcb_block->local_port = amsg->par("src_port");
      tcb_block->local_addr = amsg->par("src_addr");
      tcb_block->rem_port   = amsg->par("dest_port");
      tcb_block->rem_addr   = amsg->par("dest_addr");

      if (tcb_block->rem_port==-1 || tcb_block->rem_addr==-1)
        {
          error( "ACTIVE OPEN: foreign socket unspecified");
        }

      tcb_block->timeout = amsg->par("timeout");

      //queue data for transmission after entering ESTABLISHED
      if (debug) ev << "Queuing data for transmission after entering ESTABLISHED.\n";
      applCommandSend(amsg, tcb_block);

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVED command from appl. while in LISTEN.\n";
      if (debug) ev << "Queuing request for processing after entering ESTABLISHED.\n";
      applCommandReceive(amsg, tcb_block);

      break;

      //if appl close: delete TCB (done in CLOSED), go to CLOSED
    case TCP_E_CLOSE:
      if (debug) ev << "TCP received CLOSE command from appl. while in LISTEN.\n";

      //notify, if there are outstanding requests
      outstRcv(tcb_block);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in LISTEN.\n";

      //notify, if there are outstanding requests
      outstRcv(tcb_block);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in LISTEN.\n";

      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in LISTEN state.\n";

      //check for an RST, an incoming RST is ignored
      if (tcb_block->st_event.th_flag_rst == TCP_F_SET)
        {
          if (debug) ev << "RST flag is set in the arriving segment. RST is ignored.\n";
        }

      //check for an ACK, send RST in response
      //(tcp_header is header of arriving msg. amsg)
      else if (tcb_block->st_event.th_flag_ack == TCP_F_SET)
        {
          if (debug) ev << "ACK flag is set in the arriving segment.\n";

          //create a data-less RST segment
          cMessage* msg = new cMessage("RST_DATA", RST_DATA) ;
          msg->setLength(0);

          //set RST flag
          TcpFlag rst = TCP_F_SET;

          //set sequence number of RST = ACK number of the incoming segment
          tcb_block->seg_ack = tcp_header->th_ack_no;

          //send the data-less ACK segment to remote TCP
          if (debug) ev << "Sending RST segment to remote TCP while in LISTEN state.\n";
          segSend(msg, tcb_block, tcb_block->seg_ack, TCP_F_NSET, TCP_F_NSET, rst, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);

        }

      //check for a SYN, check for security/compartment (not done here)
      else if (tcb_block->st_event.th_flag_syn == TCP_F_SET)
        {
          if (debug) ev << "SYN flag is set in the arriving segment.\n";

          //security/compartment would be checked here

          //set syn_rcvd to indicate that a SYN has been received
          tcb_block->syn_rcvd = 1;

          //set sequence number variables
          tcb_block->seg_seq     = tcp_header->th_seq_no;
          tcb_block->irs         = tcb_block->seg_seq;
          tcb_block->rcv_buf_seq = tcb_block->seg_seq + 1;
          tcb_block->rcv_nxt     = tcb_block->seg_seq + 1;

          //check MSS option here (not used in this implementation)
          //tcb_block->snd_mss = rem_rcv_mss; (rem_rcv_mss is defined in handleMessage(...))

          tcb_block->num_bit_req = amsg->par("num_bit_req");

          //set the size of the remote receive window
          tcb_block->snd_wl1 = tcb_block->seg_seq;
          tcb_block->snd_wl2 = tcb_block->seg_ack;
          tcb_block->snd_wnd = tcp_header->th_window;

          //if segment contains any data, queue it for later processing
          cMessage* data = amsg->encapsulatedMsg();
          if (data != NULL)
            {
              if (debug) ev << "Queuing data encapsulated in SYN segment (LISTEN).\n";
              segReceive(amsg, tcp_header, tcb_block, tcp_header->th_flag_fin, tcp_header->th_flag_syn, tcp_header->th_flag_rst, tcp_header->th_flag_ack, tcp_header->th_flag_urg, tcp_header->th_flag_psh);
            }
          else
            {
              delete data;
            }

          //print remote socket information provided by the incoming segment (set in getTcb(...))
          if (debug) ev << "Remote socket information provided by the incoming segment: \n";
          if (debug) ev << "Remote IP address: " << tcb_block->rem_addr << ", remote port: " << tcb_block->rem_port << "\n";
          if (debug) ev << "Initial Receive Sequence Number (IRS): " << tcb_block->seg_seq << "\n";

          //update event status: a SYN has been received
          tcb_block->st_event.event = TCP_E_RCV_SYN;
        }

      //any other text or control: segments are discarded
      else
        {
          if (debug) ev << "Arriving segment in LISTEN does not contain a RST, ACK or SYN. Segment is discarded.\n";
        }

      break;

    default:
      //error( "Case not handled in switch statement (LISTEN)");
      error("Event %s not handled in LISTEN state switch statement", eventName(tcb_block->st_event.event));

      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_OPEN_ACTIVE:
      //send SYN segment
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);


      // set the connection-establishment timer to 75 seconds
      tcb_block->timeout_conn_estab_msg = new cMessage("TIMEOUT_CONN_ESTAB",TIMEOUT_CONN_ESTAB);
      //add parameters for function tcpArrivalMsg(...) here
      tcb_block->timeout_conn_estab_msg->addPar("src_port")  = tcb_block->local_port;
      tcb_block->timeout_conn_estab_msg->addPar("src_addr")  = tcb_block->local_addr;
      tcb_block->timeout_conn_estab_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_conn_estab_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + CONN_ESTAB_TIMEOUT_VAL, tcb_block->timeout_conn_estab_msg);


      if (debug) ev << "TCP is going to SYN_SENT state.\n";
      FSM_Goto(fsm, TCP_S_SYN_SENT);

      break;

    case TCP_E_SEND:
      //send SYN segment
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
      // set the connection-establishment timer to 75 seconds
      tcb_block->timeout_conn_estab_msg = new cMessage("TIMEOUT_CONN_ESTAB",TIMEOUT_CONN_ESTAB);
      //add parameters for function tcpArrivalMsg(...) here
      tcb_block->timeout_conn_estab_msg->addPar("src_port")  = tcb_block->local_port;
      tcb_block->timeout_conn_estab_msg->addPar("src_addr")  = tcb_block->local_addr;
      tcb_block->timeout_conn_estab_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_conn_estab_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + CONN_ESTAB_TIMEOUT_VAL, tcb_block->timeout_conn_estab_msg);

      if (debug) ev << "TCP is going to SYN_SENT state.\n";
      FSM_Goto(fsm, TCP_S_SYN_SENT);

      break;

    case TCP_E_CLOSE:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_RCV_SYN:
      //create and send a SYN segment with the ACK flag set
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_SET, TCP_F_NSET, TCP_F_NSET);
      // set the connection-establishment timer to 75 seconds
      tcb_block->timeout_conn_estab_msg = new cMessage("TIMEOUT_CONN_ESTAB",TIMEOUT_CONN_ESTAB);
      //add parameters for function tcpArrivalMsg(...) here
      tcb_block->timeout_conn_estab_msg->addPar("src_port")  = tcb_block->local_port;
      tcb_block->timeout_conn_estab_msg->addPar("src_addr")  = tcb_block->local_addr;
      tcb_block->timeout_conn_estab_msg->addPar("dest_port") = tcb_block->rem_port;
      tcb_block->timeout_conn_estab_msg->addPar("dest_addr") = tcb_block->rem_addr;
      scheduleAt(simTime() + CONN_ESTAB_TIMEOUT_VAL, tcb_block->timeout_conn_estab_msg);


      //go to SYN_RCVD
      if (debug) ev << "TCP is going to state SYN_RCVD.\n";
      FSM_Goto(fsm, TCP_S_SYN_RCVD);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of swtich
}


//function to do exit SYN_RCVD processing
void TcpModule::procExSynRcvd(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  int acceptable;

  switch(tcb_block->st_event.event)
    {
      //if appl active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTICE OPEN command from appl. while in SYN_RCVD.\n");
      break;

      //if appl passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in SYN_RCVD.\n");
      break;

    case TCP_E_TIMEOUT_REXMT:
      if (debug) ev << "Retransmission timer expired.\n";
      tcb_block->rxtshift++;
      if (tcb_block->rxtshift > TCP_MAXRXTSHIFT)
        {
          tcb_block->rxtshift = TCP_MAXRXTSHIFT;
          if (debug) ev << "Backoff shift exceeds TCP_MAXRXTSHIFT ("<< TCP_MAXRXTSHIFT << ").\n";
          if (debug) ev << "Closing the connection.\n";
          tcb_block->st_event.event = TCP_E_ABORT;
          break;
        }


      tcb_block->ssthresh = MAX(MIN(tcb_block->snd_wnd, tcb_block->snd_cwnd)/2,tcb_block->snd_mss*2);
      if (debug) ev << "Resetting ssthresh. New value: " << tcb_block->ssthresh << " octets.\n";
      tcb_block->rexmt_sch = false;

      // set new rxtcur value with backoff shift

      tcb_block->rxtcur = MIN(tcb_block->rxtcur*tcb_block->rxtshift,128);

      if (debug) ev << "Backoff shift: " << tcb_block->rxtshift <<endl;
      if (debug) ev << "New value for RTO: " << tcb_block->rxtcur << endl;
      if (debug) ev << "Resending SYN ACK.\n";
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_SET, TCP_F_NSET, TCP_F_NSET);

      break;

    case TCP_E_TIMEOUT_CONN_ESTAB:
      if (debug) ev << "Connection establishment timer expired.\n";
      if (debug) ev << "Deleting rexmt timer.\n";
      if (tcb_block->rexmt_sch)
        {
          delete cancelEvent(tcb_block->timeout_rexmt_msg);
          tcb_block->rexmt_sch = false;
        }
      if (debug) ev << "Closing the connection.\n";

      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in SYN_RCVD.\n";
      if (debug) ev << "Queuing data for transmission after entering ESTABLISHED.\n";
      applCommandSend(amsg, tcb_block);

      break;

      //if appl send: queue data
    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in SYN_RCVD.\n";
      if (debug) ev << "Queuing request for processing after entering ESTABLISHED.\n";
      applCommandReceive(amsg, tcb_block);

      break;

      //if appl close, send FIN: go to FIN_WAIT_1
    case TCP_E_CLOSE:
      if (debug) ev << "TCP received CLOSE command from appl. while in SYN_RCVD.\n";

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in SYN_RCVD.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      //flush transmission/retransmission queues
      flushTransRetransQ(tcb_block);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in SYN_RCVD.\n";
      //printStatus(tcb_block, "Connection state");

      break;


    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in SYN_RCVD state.\n";

      //check sequence number (check if segment is acceptable)
      acceptable = segAccept(tcb_block);
      if (acceptable)
        {
          //check for a RST
          if (tcb_block->st_event.th_flag_rst == TCP_F_SET)
            {
              //check if connection was initiated by a passive open
              if (tcb_block->passive)
                {
                  if (debug) ev << "TCP received a RST segment while in SYN_RCVD after a PASSIVE OPEN. Returning to LISTEN.\n";

                  tcb_block->st_event.event = TCP_E_PASSIVE_RESET;

                  //restore the the initially specified remote socket information
                  tcb_block->rem_addr = tcb_block->passive_rem_addr;
                  tcb_block->rem_port = tcb_block->passive_rem_port;

                  //flush all queues
                  tcb_block->tcp_send_queue.clear();
                  tcb_block->tcp_retrans_queue.clear();
                  tcb_block->tcp_data_receive_queue.clear();
                  tcb_block->tcp_socket_queue.clear();
                  // BCH LYBD
                  //flush information about received packets from tcp_rcv_rec_list
                  // tcb_block->tcp_rcv_rec_list.clear();
                  while (tcb_block->tcp_rcv_rec_list.length() > 0) {
                      SegRecord* seg_rec = (SegRecord *) tcb_block->tcp_rcv_rec_list.pop();
                      delete seg_rec->pdata;
                  }
                  // ECH LYBD

                  //FIXME TODO: cancel all scheduled ACK and retransmission timers here
                  // BCH Andras
                  delete cancelEvent(tcb_block->timeout_conn_estab_msg);
                  if (debug) ev << "Cancelling connection establishment timer.\n";
                  // ECH Andras

                  //RFC-793: flush retransmission queue
                } //end of check passive
              else
                {
                  if (debug) ev << "Connection refused.\n";
                  tcb_block->st_event.event = TCP_E_ABORT;

                  //RFC-793: in the active open case go to CLOSED
                  //FSM_Goto(fsm, TCP_S_CLOSED);
                  //flush retransmission queue

                  if (debug) ev << "Returning from TCP_E_SEG_ARRIVAL in SYN_RCVD.\n";
                  break;
                }
            } //end of check RST

          //check for a SYN
          if (checkSyn(tcb_block))
            {
              //if incoming segment contains an ACK of the SYN sent by this process,
              //establish the connection
              if (tcb_block->st_event.th_flag_ack == TCP_F_SET)
                {
                  //if the incoming segment contains data, queue the data for later processing
                  tcb_block->seg_len = amsg->par("seg_len");
                  if (tcb_block->seg_len > 0)
                    {
                      TcpFlag fin = tcb_block->st_event.th_flag_fin;
                      TcpFlag syn = tcb_block->st_event.th_flag_syn;
                      TcpFlag rst = tcb_block->st_event.th_flag_rst;
                      TcpFlag ack = tcb_block->st_event.th_flag_ack;
                      TcpFlag urg = tcb_block->st_event.th_flag_urg;
                      TcpFlag psh = tcb_block->st_event.th_flag_psh;
                      if (debug) ev << "Queueing data encapsulated in SYN segment (SYN_RCVD).\n";
                      segReceive(amsg, tcp_header, tcb_block, fin, syn, rst, ack, urg, psh);
                    }

                  //get the ack. sequence number of the incoming segment
                  tcb_block->seg_ack = tcp_header->th_ack_no;

                  //update the remote receive window
                  tcb_block->snd_wl1 = tcb_block->seg_seq;
                  tcb_block->snd_wl2 = tcb_block->seg_ack;
                  tcb_block->snd_wnd = tcp_header->th_window;

                  if (seqNoLeq(tcb_block->snd_una, tcb_block->seg_ack) && seqNoLeq(tcb_block->seg_ack, tcb_block->snd_nxt))
                    {
                      //cancel retransmission timeout here
                      if (tcb_block->rexmt_sch == true)
                        {
                          delete cancelEvent(tcb_block->timeout_rexmt_msg);
                          if (debug) ev << "Retransmission timer cancelled. \n";
                          tcb_block->rexmt_sch = false;
                         }

                      //update SND.UNA
                      tcb_block->snd_una = tcb_block->seg_ack;

                      if (tcb_block->st_event.event != TCP_E_RCV_FIN)
                        {
                          tcb_block->st_event.event = TCP_E_RCV_ACK_OF_SYN;

                          //send nothing, go to ESTABLISHED
                          if (debug) ev << "Recived ACK of SYN. Going to ESTABLISHED.\n";
                        }
                    }
                  else
                    {
                      //ACK is not acceptable, send a data-less RST to remote TCP
                      if (debug) ev << "Incoming ACK is not acceptable. Sending a RST segment.\n";
                      cMessage* msg = new cMessage("RST_DATA", RST_DATA);
                      msg->setLength(0);

                      //set RST flag
                      TcpFlag rst = TCP_F_SET;

                      segSend(msg, tcb_block, tcb_block->seg_ack, TCP_F_NSET, TCP_F_NSET, rst,  TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
                    }

                } //end of check ACK
            } //end of checkSyn(...)
        } // end of if (acceptable)
      else
        {
          //probably the unacceptable segment is a retransmission
          // so we set syn_rcvd = 0 not to call connSynRcvd again.
          tcb_block->syn_rcvd = 0;
        }
      break;


    default:
      //error( "Case not handled in switch statement (SYN_RCVD)");
      error("Event %s not handled in SYN_RCVD state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_CLOSE:
      if (debug) ev << "Scheduling a FIN segment.\n";
      finSchedule(tcb_block);

      if (debug) ev << "TCP is going to FIN_WAIT_1 state.\n";
      FSM_Goto(fsm, TCP_S_FIN_WAIT_1);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_TIMEOUT_REXMT:

      if (debug) ev << "TCP is staying in SYN_RCVD state.\n";
      FSM_Goto(fsm, TCP_S_SYN_RCVD);

      break;

    case TCP_E_TIMEOUT_CONN_ESTAB:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_PASSIVE_RESET:
      //return to LISTEN
      FSM_Goto(fsm, TCP_S_LISTEN);

      break;

    case TCP_E_RCV_ACK_OF_SYN:
      // cancel the connection establishment timer (in a real implementation,
      // this is done slightly different but it's the same result...
      delete cancelEvent(tcb_block->timeout_conn_estab_msg);
      if (debug) ev << "Cancelling connection establishment timer.\n";

      FSM_Goto(fsm, TCP_S_ESTABLISHED);

      break;

    case TCP_E_RCV_FIN:
      if (debug) ev << "TCP received a FIN segment while in SYN_RCVD. Scheduling an ACK, going to CLOSE_WAIT.\n";

      //schedule an ACK
      ackSchedule(tcb_block, true);

      //going to CLOSE_WAIT
      FSM_Goto(fsm, TCP_S_CLOSE_WAIT);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit SYN_SENT processing
void TcpModule::procExSynSent(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in SYN_SENT.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in SYN_SENT.\n");
      break;

      // if retransmission timer expired -> double rto, re-send SYN
    case TCP_E_TIMEOUT_REXMT:
      if (debug) ev << "Retransmission timer expired.\n";
      tcb_block->rxtshift++;
      if (tcb_block->rxtshift > TCP_MAXRXTSHIFT)
        {
          tcb_block->rxtshift = TCP_MAXRXTSHIFT;
          if (debug) ev << "Backoff shift exceeds TCP_MAXRXTSHIFT ("<< TCP_MAXRXTSHIFT << ").\n";
          if (debug) ev << "Closing the connection.\n";
          tcb_block->st_event.event = TCP_E_ABORT;
          break;
        }

      tcb_block->rexmt_sch = false;
      if (debug) ev << "Resetting ssthresh. New value: " << tcb_block->ssthresh << " octets.\n";
      tcb_block->ssthresh = MAX(MIN(tcb_block->snd_wnd, tcb_block->snd_cwnd)/2,tcb_block->snd_mss*2);

      // set new rxtcur value with backoff shift

      tcb_block->rxtcur = MIN(tcb_block->rxtcur*tcb_block->rxtshift,128);

      if (debug) ev << "Backoff shift: " << tcb_block->rxtshift <<endl;
      if (debug) ev << "New value for RTO: " << tcb_block->rxtcur << endl;
      if (debug) ev << "Re-sending SYN.\n";
      synSend(tcb_block, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
      break;


      // if connection establishment timer expired -> close the connection
    case TCP_E_TIMEOUT_CONN_ESTAB:
      if (debug) ev << "Connection establishment timer expired.\n";
      if (debug) ev << "Deleting rexmt timer.\n";
      if (tcb_block->rexmt_sch)
        {
          delete cancelEvent(tcb_block->timeout_rexmt_msg);
          tcb_block->rexmt_sch = false;
        }
      if (debug) ev << "Closing the connection.\n";

      break;

      //if appl. send: queue data
    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in SYN_SENT.\n";
      if (debug) ev << "Queuing data for transmission after entering ESTABLISHED.\n";
      applCommandSend(amsg, tcb_block);

      break;

      //if appl. receive: queue request
    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in SYN_SENT.\n";
      if (debug) ev << "Queuing request for processing after entering ESTABLISHED.\n";
      applCommandReceive(amsg, tcb_block);

      break;

      //if appl. close: delete TCB (done in CLOSED), go to CLOSED
    case TCP_E_CLOSE:
      if (debug) ev << "TCP received CLOSE command from appl. while in SYN_SENT.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in SYN_SENT.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      break;


    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in SYN_SENT.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in SYN_SENT state.\n";

      //this is the first time a segment from the remote TCP can arrive, if so
      //set tb_conn_id
      if (tcb_block->tb_conn_id == -1)
        {
          tcb_block->tb_conn_id = amsg->par("tcp_conn_id");
        }

      //check for an ACK
      if (tcb_block->st_event.th_flag_ack == TCP_F_SET)
        {
          if (debug) ev << "ACK flag is set in the arriving segment.\n";

          //(tcp_header is the header of the incoming segment amsg)
          tcb_block->seg_ack = tcp_header->th_ack_no;

          //if SEG.ACK <= ISS or SEG.ACK > SND.NXT, send a RST (unless RST is  set in segment)
          if (seqNoLeq(tcb_block->seg_ack, tcb_block->iss) || seqNoGt(tcb_block->seg_ack, tcb_block->snd_nxt))
            {
              if (debug) ev << "Segment ACK unacceptable.\n";

              //if RST flag is set, simply drop the segment
              if (tcb_block->st_event.th_flag_rst == TCP_F_SET)
                {
                  if (debug) ev << "RST flag is set, discarding the segment.\n";
                }
              //else send data-less RST, discard the segment
              else
                {
                  if (debug) ev << "Sending data-less RST segment to remote TCP in response to the invalid ACK.\n";

                  //create a data-less RST
                  cMessage* msg = new cMessage("RST_DATA", RST_DATA);
                  msg->setLength(0);

                  //set RST flag
                  TcpFlag rst = TCP_F_SET;

                  segSend(msg, tcb_block, tcb_block->seg_ack, TCP_F_NSET, TCP_F_NSET, rst,  TCP_F_NSET, TCP_F_NSET, TCP_F_NSET);
                }

              if (debug) ev << "Returning from TCP_E_SEG_ARRIVAL in state SYN_SENT\n";
              break;
            }
          //check for a RST
          if (tcb_block->st_event.th_flag_rst == TCP_F_SET)
            {
              if (debug) ev << "ACK RST segment received.\n";
              if (debug) ev << "Error: Connection reset.\n";

              tcb_block->st_event.event = TCP_E_ABORT;

              //if (debug) ev << "TCP is going to CLOSED state.\n";
              //FSM_Goto(fsm, TCP_S_CLOSED);

              if (debug) ev << "Returning from TCP_E_SEG_ARRIVAL in state SYN_SENT\n";
              break;
            }
        } //end of check ACK
      //check for a RST
      else if (tcb_block->st_event.th_flag_rst == TCP_F_SET)
        {
          if (debug) ev << "TCP received RST segment without ACK. Discarding segment.\n";
          if (debug) ev << "Returning from TCP_E_SEG_ARRIVAL in state SYN_SENT\n";
          break;
        }

      //check security and precedence (not done here)

      //check for a SYN
      if (tcb_block->st_event.th_flag_syn == TCP_F_SET)
        {
          if (debug) ev << "SYN flag is set in the arriving segment.\n";

          //set the syn_rcvd variable
          tcb_block->syn_rcvd = 1;

          //update the sequence number
          tcb_block->seg_seq = tcp_header->th_seq_no;

          //cancel retransmission timeout here
          if (tcb_block->rexmt_sch)
           {
            delete cancelEvent(tcb_block->timeout_rexmt_msg);
            if (debug) ev << "Retransmission timer cancelled . \n";
            tcb_block->rexmt_sch = false;
           }

          //update the receive window information
          tcb_block->irs         = tcb_block->seg_seq;
          tcb_block->rcv_nxt     = tcb_block->seg_seq + 1;
          tcb_block->rcv_buf_seq = tcb_block->seg_seq + 1;

          //get size of the remote receive window
          tcb_block->snd_wl1 = tcb_block->seg_seq;
          tcb_block->snd_wl2 = tcb_block->seg_ack;
          tcb_block->snd_wnd = tcp_header->th_window;

          //check MSS option here (not yet implemented)

          //if the ACK flag is also set, update the ACK number
          if (tcb_block->st_event.th_flag_ack == TCP_F_SET)
            {
              tcb_block->seg_ack = tcp_header->th_ack_no;

              tcb_block->snd_una = tcb_block->seg_ack;

              //cancel retransmission timeout here
              if (tcb_block->rexmt_sch)
                {
                delete cancelEvent(tcb_block->timeout_rexmt_msg);
                if (debug) ev << "Retransmission timer cancelled. \n";
                tcb_block->rexmt_sch = false;
               }
            } //end of check ACK

          //chose correct transition (RCV_SYN ==> SYN_RCVD, RCV_SYN_ACK ==> ESTABLISHED)
          if (seqNoGt(tcb_block->snd_una, tcb_block->iss))
            {
              tcb_block->st_event.event = TCP_E_RCV_SYN_ACK;
            }
          else
            {
              tcb_block->st_event.event = TCP_E_RCV_SYN;

              if (debug) ev << "TCP received a SYN segment. Sending an ACK segment immediately.\n";

              //cancel ack timer here

              //send a data-less ACK segment immediately ======================================
              //create a data-less segment and send it at once
              tcb_block->ack_sch = true;

              //timeoutDatalessAck(tcb_block); (see //(**)// in transition switch statement)

              //maybe scheduleAt(simTime(), ack_seg); necessary instead of code between ===
              //==============================================================================
            }
        } //end of check SYN
      else
        {
          if (debug) ev << "TCP received non-SYN segment. Discarding segment.\n";
        }

      break;

    default:
      //error( "Case not handled in switch statement (SYN_SENT)");
      error("Event %s not handled in SYN_SENT state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_CLOSE:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_TIMEOUT_REXMT:
      if (debug) ev << "TCP is staying in SYN_SENT state.\n";
      FSM_Goto(fsm, TCP_S_SYN_SENT);
      break;

    case TCP_E_TIMEOUT_CONN_ESTAB:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_RCV_SYN_ACK:
      if (debug) ev << "TCP received a SYN/ACK segment. Scheduling an ACK segment.\n";
      ackSchedule(tcb_block, true);

      if (debug) ev << "TCP is going to ESTABLISHED state.\n";
      // cancel the connection establishment timer (in a real implementation,
      // this is done slightly different but it's the same result...
      delete cancelEvent(tcb_block->timeout_conn_estab_msg);
      if (debug) ev << "Cancelling connection establishment timer.\n";
      FSM_Goto(fsm, TCP_S_ESTABLISHED);

      break;

    case TCP_E_RCV_SYN:                                           //(**)//
      timeoutDatalessAck(tcb_block);

      if (debug) ev << "TCP is going to SYN_RCVD state.\n";
      FSM_Goto(fsm, TCP_S_SYN_RCVD);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of swtich
}


//function to do exit ESTABLISHED processing
void TcpModule::procExEstablished(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in ESTABLISHED.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in ESTABLISHED.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in ESTABLISHED.\n";
      applCommandSend(amsg, tcb_block);

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in ESTABLISHED.\n";
      applCommandReceive(amsg, tcb_block);

      break;

      //if appl. close: send FIN, go to FIN_WAIT_1
    case TCP_E_CLOSE:
      if (debug) ev << "TCP received CLOSE command from appl. while in ESTABLISHED.\n";

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in ESTABLISHED.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      //flush transmission/retransmission queues
      flushTransRetransQ(tcb_block);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in ESTABLISHED.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in ESTABLISHED state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) &&  checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          if (tcp_header->th_window != 0 && tcb_block->per_sch)
            {
             if (debug) ev << "Cancelling persist timer" << endl;
         delete cancelEvent(tcb_block->timeout_persist_msg);
             tcb_block->per_sch = false;
          }
          // activate persist timer
          if (tcp_header->th_window == 0)
            {
              ev << "Receive window receiver is ZERO" << endl;
              if (!tcb_block->per_sch)
                {
                 tcb_block->timeout_persist_msg = new cMessage("TIMEOUT_PERSIST",TIMEOUT_PERSIST);
                 //add parameters for function tcpArrivalMsg(...) here
                 tcb_block->timeout_persist_msg->addPar("src_port") = tcb_block->local_port;
                 tcb_block->timeout_persist_msg->addPar("src_addr") = tcb_block->local_addr;
                 tcb_block->timeout_persist_msg->addPar("dest_port") = tcb_block->rem_port;
                 tcb_block->timeout_persist_msg->addPar("dest_addr") = tcb_block->rem_addr;
         // <Jeroen>
         // FIXME schedule persist timer at now + 1.5 seconds.
         // this 1.5 sec. value should be calculated using the rtt and backoff shift!
                 scheduleAt(simTime() + 1.5, tcb_block->timeout_persist_msg);
                 tcb_block->per_sch = true;
              }
          }

          if (tcb_block->seg_len > 0)
            {
              TcpFlag fin = tcb_block->st_event.th_flag_fin;
              TcpFlag syn = tcb_block->st_event.th_flag_syn;
              TcpFlag rst = tcb_block->st_event.th_flag_rst;
              TcpFlag ack = tcb_block->st_event.th_flag_ack;
              TcpFlag urg = tcb_block->st_event.th_flag_urg;
              TcpFlag psh = tcb_block->st_event.th_flag_psh;
              if (debug) ev << "Queueing data encapsulated in TCP segment (ESTABLISHED).\n";
              //tcp_header is the header of the incoming amsg
              segReceive(amsg, tcp_header, tcb_block, fin, syn, rst, ack, urg, psh);
            }
        }
      break;

    // <Jeroen>
    case TCP_E_TIMEOUT_PERSIST:
        // we should send 1 byte of data now
        ev << "Timeout persist" << endl;
        timeoutPersistTimer(tcb_block);
        break;
    // </Jeroen>

    case TCP_E_TIMEOUT_REXMT:

      tcb_block->rxtshift++;
      if (tcb_block->rxtshift > TCP_MAXRXTSHIFT)
        {
          tcb_block->rxtshift = TCP_MAXRXTSHIFT;
          if (debug) ev << "Backoff shift exceeds TCP_MAXRXTSHIFT ("<< TCP_MAXRXTSHIFT << ").\n";
          if (debug) ev << "Closing the connection.\n";
          tcb_block->st_event.event = TCP_E_ABORT;
          break;
        }
      timeoutRetransTimer(tcb_block);
      // We don't call retransQueueProcess as it's called by sndDataProcess
      // when re-entering established state...
      //retransQueueProcess(tcb_block);
      break;

    case TCP_E_TIMEOUT_DELAYED_ACK:
      //schedule an immediate ack
      tcb_block->ack_sch = false;
      ackSchedule(tcb_block,true);
      break;

    default:
      //error( "Case not handled in switch statement (ESTABLISHED)");
      error("Event %s not handled in ESTABLISHED state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_CLOSE:
      if (debug) ev << "Scheduling a FIN segment.\n";
      finSchedule(tcb_block);

      if (debug) ev << "TCP is going to FIN_WAIT_1 state.\n";
      FSM_Goto(fsm, TCP_S_FIN_WAIT_1);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_RCV_FIN:
      if (debug) ev << "TCP received a FIN segment. Scheduling an ACK and going to CLOSE_WAIT.\n";
      ackSchedule(tcb_block, true);
      FSM_Goto(fsm, TCP_S_CLOSE_WAIT);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to process an appl. call in state exit CLOSE_WAIT
void TcpModule::procExCloseWait(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in CLOSE_WAIT.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in CLOSE_WAIT.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in CLOSE_WAIT.\n";
      applCommandSend(amsg, tcb_block);

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in CLOSE_WAIT.\n";
      if (debug) ev << "Ignoring command, since connection is closing.\n";

      break;

      //if appl. close: send FIN, go to FIN_WAIT_1 (last_ack???)
    case TCP_E_CLOSE:
      if (debug) ev << "TCP received CLOSE command from appl. while in CLOSE_WAIT.\n";

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in CLOSE_WAIT.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      //flush transmission/retransmission queues
      flushTransRetransQ(tcb_block);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in CLOSE_WAIT.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in CLOSE_WAIT state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) &&  checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          if (debug) ev << "Ignoring any segment text, since a FIN has already been received.\n";
        }
      break;

    default:
      //error( "Case not handled in switch statement (CLOSE_WAIT)");
      error("Event %s not handled in CLOSE_WAIT state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_CLOSE:
      if (debug) ev << "Scheduling a FIN segment.\n";
      finSchedule(tcb_block);

      if (debug) ev << "TCP is going to LAST_ACK state.\n";
      FSM_Goto(fsm, TCP_S_LAST_ACK);

      break;



    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of swtich
}


//function to do exit LAST_ACK processing
void TcpModule::procExLastAck(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //rexmt timer expiry: resend fin
    case TCP_E_TIMEOUT_REXMT:
      tcb_block->rxtshift++;
      if (tcb_block->rxtshift > TCP_MAXRXTSHIFT)
        {
          if (debug) ev << "I've tried enough now. I go to sleep.\n";
          tcb_block->st_event.event = TCP_E_ABORT;
          break;
        }
      timeoutRetransTimer(tcb_block);
      break;
      //if (debug) ev << "Retransmission timer expired.\n";
      //tcb_block->rexmt_sch = false;

      //tcb_block->ssthresh = MAX(MIN(tcb_block->snd_wnd, tcb_block->snd_cwnd)/2,tcb_block->snd_mss*2);
      //if (debug) ev << "Resetting ssthresh. New value: " << tcb_block->ssthresh << " octets.\n";
      // set new rxtcur value with backoff shift
      //tcb_block->rxtshift++;
      //tcb_block->rxtcur = MIN(tcb_block->rxtcur*tcb_block->rxtshift,128);

      //if (debug) ev << "Backoff shift: " << tcb_block->rxtshift <<endl;
      //if (debug) ev << "New value for RTO: " << tcb_block->rxtcur << endl;
      //if (debug) ev << "Re-sending FIN.\n";
      //finSchedule(tcb_block);

      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in LAST_ACK.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in LAST_ACK.\n");
      break;

    case TCP_E_SEND:
      error("TCP received SEND command from appl. while in LAST_ACK.\n");
      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in LAST_ACK.\n";
      if (debug) ev << "Ignoring RECEIVE command. Connection closing.\n";
      break;

    case TCP_E_CLOSE:
      error ("TCP received a CLOSE command from appl. while in LAST_ACK.\n");
      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in LAST_ACK.\n";
      if (debug) ev << "OK.\n";
      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in LAST_ACK.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in LAST_ACK state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) && checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          if (debug) ev << "Ignoring data in segment (if any), since all application data has already been received.\n";

          //---------------------------
          //if (tcb_block->snd_fin_valid)  //&& seqNoGt(tcb_block->seg_ack, tcb_block->snd_fin_seq))
          //{
          //  //flush appropriate segments from retransmission queue
          //  if ((tcb_block->seg_ack - tcb_block->snd_una) > 1)
          //    {
          //      if (debug) ev << "Removing acknowledged octets from retransmission queue.\n";
          //      flushQueue(tcb_block->tcp_retrans_queue,  (tcb_block->seg_ack - tcb_block->snd_una - 1), false);
          //    }
          //  //set largest unacked seq. no. to ack. no.
          //  tcb_block->snd_una = tcb_block->seg_ack;
          //  if (debug) ev << "Setting event to TCP_E_RCV_ACK_OF_FIN.\n";
          //  tcb_block->st_event.event = TCP_E_RCV_ACK_OF_FIN;
          //}
          //--------------------------
        }
      break;


    default:
      //error( "Case not handled in switch statement (LAST_ACK)");
      error("Event %s not handled in LAST_ACK state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_TIMEOUT_REXMT:
      if (debug) ev << "Staying in LAST ACK state.\n";
      FSM_Goto(fsm, TCP_S_LAST_ACK);

      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_RCV_ACK_OF_FIN:
      if (debug) ev << "TCP received ACK of FIN. Going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit FIN_WAIT_1 processing
void TcpModule::procExFinWait1(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
    case TCP_E_TIMEOUT_REXMT:
      // LYBD
      if (debug) ev << "Retransmission timer expired.\n";
      tcb_block->rxtshift++;
      // LYBD
      timeoutRetransTimer(tcb_block);
      // if (debug) ev << "Re-sending FIN.\n";
      //finSchedule(tcb_block);
      break;

      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in FIN_WAIT_1.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in FIN_WAIT_1.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in FIN_WAIT_1.\n";
      if (debug) ev << "TCP is not servicing this request.\n";
      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in FIN_WAIT_1.\n";
      //if insufficient incoming segments are queued to satisfy the request,
      //queue the request (p.58 RFC-793)
      applCommandReceive(amsg, tcb_block);

      break;

    case TCP_E_CLOSE:
      error ("TCP received a CLOSE command from appl. while in FIN_WAIT_1.\n");
      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in FIN_WAIT_1.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      //flush transmission/retransmission queues
      flushTransRetransQ(tcb_block);

      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in FIN_WAIT_1.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in FIN_WAIT_1 state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) &&  checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          if (tcb_block->seg_len > 0)
            {
              TcpFlag fin = tcb_block->st_event.th_flag_fin;
              TcpFlag syn = tcb_block->st_event.th_flag_syn;
              TcpFlag rst = tcb_block->st_event.th_flag_rst;
              TcpFlag ack = tcb_block->st_event.th_flag_ack;
              TcpFlag urg = tcb_block->st_event.th_flag_urg;
              TcpFlag psh = tcb_block->st_event.th_flag_psh;
              if (debug) ev << "Queueing data encapsulated in segment (FIN_WAIT_1).\n";
              //tcp_header is the header of the incoming amsg
              segReceive(amsg, tcp_header, tcb_block, fin, syn, rst, ack, urg, psh);
            }
        }
      break;

    default:
      //error( "Case not handled in switch statement (FIN_WAIT_1)");
      error("Event %s not handled in FIN_WAIT_1 state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_TIMEOUT_REXMT:
      if (debug) ev << "TCP is staying in FIN_WAIT_1.\n";
      FSM_Goto(fsm, TCP_S_FIN_WAIT_1);

      break;
    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

      //event = RCV_FIN => go to CLOSING
    case TCP_E_RCV_FIN:
      if (debug) ev << "TCP received a FIN segment. Scheduling an ACK and going to CLOSING.\n";
      ackSchedule(tcb_block, true);

      FSM_Goto(fsm, TCP_S_CLOSING);

      break;

      //event = RCV_ACK_OF_FIN => go to FIN_WAIT_2
    case TCP_E_RCV_ACK_OF_FIN:
      if (debug) ev << "TCP received an ACK of the FIN segment previously sent. Going to FIN_WAIT_2.\n";

      FSM_Goto(fsm, TCP_S_FIN_WAIT_2);

      break;

      //event = RCV_FIN_ACK_OF_FIN => go to TIME_WAIT
    case TCP_E_RCV_FIN_ACK_OF_FIN:
      if (debug) ev << "TCP received a FIN segment. Scheduling an ACK and going to TIME_WAIT.\n";
      ackSchedule(tcb_block, true);
      FSM_Goto(fsm, TCP_S_TIME_WAIT);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit FIN_WAIT_2 processing
void TcpModule::procExFinWait2(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in FIN_WAIT_2.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in FIN_WAIT_2.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in FIN_WAIT_2.\n";
      if (debug) ev << "TCP is not servicing this request.\n";
      error( "Connection closing");

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in FIN_WAIT_2.\n";
      //if insufficient incoming segments are queued to satisfy the request,
      //queue the requst (p.58 RFC-793)
      applCommandReceive(amsg, tcb_block);

      break;

    case TCP_E_CLOSE:
      error ("TCP received a CLOSE command from appl. while in FIN_WAIT_2.\n");
      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in FIN_WAIT_2.\n";

      //notify of queued SENDs and RECEIVEs from application
      outstRcv(tcb_block);
      outstSnd(tcb_block);

      //flush transmission/retransmission queues
      flushTransRetransQ(tcb_block);

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in FIN_WAIT_2.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in FIN_WAIT_2 state.\n";

      //------------------------------------------------------------
      //added for correct FIN processing
      //if (tcb_block->st_event.th_flag_fin == TCP_F_SET)
      //{
      //  tcb_block->st_event.event = TCP_E_RCV_FIN;
      //  if (debug) ev << "FIN is set, setting event to TCP_E_RCV_FIN.\n";
      //  break;
      //}
      //------------------------------------------------------------

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) &&  checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          if (tcb_block->seg_len > 0)
            {
              TcpFlag fin = tcb_block->st_event.th_flag_fin;
              TcpFlag syn = tcb_block->st_event.th_flag_syn;
              TcpFlag rst = tcb_block->st_event.th_flag_rst;
              TcpFlag ack = tcb_block->st_event.th_flag_ack;
              TcpFlag urg = tcb_block->st_event.th_flag_urg;
              TcpFlag psh = tcb_block->st_event.th_flag_psh;
              if (debug) ev << "Queueing data encapsulated in SYN segment (FIN_WAIT_2).\n";
              //tcp_header is the header of the incoming amsg
              segReceive(amsg, tcp_header, tcb_block, fin, syn, rst, ack, urg, psh);
            }
          // TODO: restart the finwait2 timer
        }
      break;

    case TCP_E_RCV_FIN:
      if (debug) ev << "FIN-Segment arrives while TCP is in FIN_WAIT_2 state.\n";
      break;

    case TCP_E_TIMEOUT_FIN_WAIT_2:
      if (debug) ev << "FIN_WAIT 2 timer expired. Closing the connection.\n";
      break;

   case TCP_E_TIMEOUT_DELAYED_ACK:
       tcb_block->ack_sch = false;
       ackSchedule(tcb_block, true);
      break;

    default:
      //error( "Case not handled in switch statement (FIN_WAIT_2)");
      error("Event %s not handled in FIN_WAIT_2 state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

      //event = RCV_FIN => go to TIME WAIT
    case TCP_E_RCV_FIN:
      if (debug) ev << "TCP received a FIN segment. Scheduling an ACK and going to TIME_WAIT.\n";
      ackSchedule(tcb_block, true);
      FSM_Goto(fsm, TCP_S_TIME_WAIT);

      break;

    case TCP_E_TIMEOUT_FIN_WAIT_2:
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit CLOSING processing
void TcpModule::procExClosing(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in CLOSING.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in CLOSING.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in CLOSING.\n";
      if (debug) ev << "TCP is not servicing this request.\n";
      error( "Connection closing");

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in CLOSING.\n";
      if (debug) ev << "Ignoring RECEIVE command. Connection closing.\n";

      break;

    case TCP_E_CLOSE:
      error ("TCP received a CLOSE command from appl. while in CLOSING.\n");
      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in CLOSING.\n";
      if (debug) ev << "OK.\n";

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in CLOSING.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in CLOSING state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) && checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          //ignore the segment, since all application data has been received
          if (debug) ev << "Ignoring segment.\n";
        }
      break;

    default:
      //error( "Case not handled in switch statement (CLOSING)");
      error("Event %s not handled in CLOSING state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_RCV_ACK_OF_FIN:
      if (debug) ev << "TCP received ACK of FIN. Going to TIME_WAIT state.\n";
      FSM_Goto(fsm, TCP_S_TIME_WAIT);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}


//function to do exit TIME_WAIT processing
void TcpModule::procExTimeWait(cMessage* amsg, TcpTcb* tcb_block, TcpHeader* tcp_header)
{
  //alias for TCP-FSM
  cFSM & fsm = tcb_block->fsm;

  switch(tcb_block->st_event.event)
    {
      //if appl. active open: error
    case TCP_E_OPEN_ACTIVE:
      error ("TCP received ACTIVE OPEN command from appl. while in TIME_WAIT.\n");
      break;

      //if appl. passive open: error
    case TCP_E_OPEN_PASSIVE:
      error ("TCP received PASSIVE OPEN command from appl. while in TIME_WAIT.\n");
      break;

    case TCP_E_SEND:
      if (debug) ev << "TCP received SEND command from appl. while in TIME_WAIT.\n";
      if (debug) ev << "TCP is not servicing this request.\n";
      error( "Connection closing");

      break;

    case TCP_E_RECEIVE:
      if (debug) ev << "TCP received RECEIVE command from appl. while in TIME_WAIT.\n";
      if (debug) ev << "Ignoring RECEIVE command. Connection closing.\n";

      break;

    case TCP_E_CLOSE:
      error ("TCP received a CLOSE command from appl. while in TIME_WAIT.\n");
      break;

    case TCP_E_ABORT:
      if (debug) ev << "TCP received ABORT command from appl. while in TIME_WAIT.\n";
      if (debug) ev << "OK.\n";

      break;

    case TCP_E_STATUS:
      if (debug) ev << "TCP received STATUS command from appl. while in TIME_WAIT.\n";
      //printStatus(tcb_block, "Connection state");

      break;

    case TCP_E_SEG_ARRIVAL:
      if (debug) ev << "Segment arrives while TCP is in TIME_WAIT state.\n";

      //check sequence number (check if segment is acceptable), check RST, SYN, ACK
      if (segAccept(tcb_block) && checkRst(tcb_block) && checkSyn(tcb_block) && checkAck(tcb_block))
        {
          //the only thing that can arrive in this state is a retransmission
          //of the remote FIN, acknowledge it
          ackSchedule(tcb_block,true);
        }
      break;

    case TCP_E_TIMEOUT_TIME_WAIT:
      if (debug) ev << "Time-Wait timeout expired.\n";
      break;

    default:
      //error( "Case not handled in switch statement (TIME_WAIT)");
      error("Event %s not handled in TIME_WAIT state switch statement", eventName(tcb_block->st_event.event));
      break;

    } //end of switch

  //transition switch statement
  switch (tcb_block->st_event.event)
    {
    case TCP_E_ABORT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    case TCP_E_TIMEOUT_TIME_WAIT:
      if (debug) ev << "TCP is going to CLOSED state.\n";
      FSM_Goto(fsm, TCP_S_CLOSED);

      break;

    default:
      if (debug) ev << "Staying in current state.\n";

      break;

    } //end of switch
}

