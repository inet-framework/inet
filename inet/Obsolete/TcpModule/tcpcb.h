// -*- C++ -*-
//
//-----------------------------------------------------------------------------
//-- fileName: tcpcb.h
//--
//-- TCP Control Block
//--
//-- U. Kaage, Feb. 2001
//--
//-----------------------------------------------------------------------------
//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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


// Attention: This is a dummy. It is not used at the moment


#ifndef TCPCB_H
#define TCPCB_H

#include <omnetpp.h>

#include "pcb.h"
#include "tcp.h"

class TCPCB : public PCB
{
 private:

  // statistics
  cOutVector stat_seq_no;
  cOutVector stat_swnd;
  cOutVector stat_cwnd;
  cOutVector stat_ssthresh;

  //TCP finite state machine
  cFSM fsm;

  //TCP queues
  cQueue tcp_send_queue;             //tcp_send_queue: queue data not yet sent/queue send calls from appl.
  cQueue tcp_retrans_queue;          //tcp_retrans_queue: queue data which has been sent but not yet acknowledged
  cQueue tcp_data_receive_queue;     //to queue the data of incoming segments
  cQueue tcp_socket_queue;           //socket buffer
  cLinkedList tcp_rcv_rec_list;      //receive record list

  //application and connection identification information
  TcpConnId tb_conn_id;
  TcpState tb_state;
  TcpState tb_from_state;

  //event information
  TcpStEvent st_event;

  //status information
  TcpStatusInfo status_info;

  // FIXME: OBSOLETE
  //socket information
  //   int local_port; //local TCP-port
  //   int local_addr; //local IP-address
  //   int rem_port;   //TCP destination port
  //   int rem_addr;   //destination IP-address

  //timer information
  double timeout; //user timeout (not implemented)

  //number of bits requested by a client TCP
  unsigned long num_bit_req;

  //send sequence number varibales (RFC 793)
  unsigned long snd_una;     //send unacknowledged
  unsigned long snd_nxt;     //send next
  unsigned long savenext;    //save-variable for snd_nxt in fast rexmt
  unsigned long snd_up;      //send urgent pointer
  unsigned long snd_wnd;     //send window
  unsigned long snd_wl1;     //segment sequence number used for last window update
  unsigned long snd_wl2;     //segment ack. number used for last window update
  unsigned long iss;         //initial sequence number (ISS)

  unsigned long snd_fin_seq;   //last seq. no.
  int snd_fin_valid;           //FIN flag set?
  int snd_up_valid;            //urgent pointer valid/URG flag set?
  unsigned long snd_mss;       //maximum segment size

  // slow start and congestion avoidance variables (RFC 2001)
  unsigned long snd_cwnd;        //congestion window
  unsigned long ssthresh;    //slow start threshold
  int cwnd_cnt;

  //receive sequence number variables
  unsigned long rcv_nxt;      //receive next
  unsigned long rcv_wnd;      //receive window
  unsigned long rcv_wnd_last; //last receive window
  unsigned long rcv_up;       //receive urgent pointer;
  unsigned long irs;          //initial receive sequence number

  //receive variables
  unsigned long rcv_fin_seq;
  int rcv_fin_valid;
  int rcv_up_valid;
  unsigned long rcv_buf_seq;
  unsigned long rcv_buff;
  double  rcv_buf_usage_thresh;

  //retransmit variables
  unsigned long snd_max;         //highest sequence number sent; used to recognize retransmits
  unsigned long max_retrans_seq; //sequence number of a retransmitted segment

  //segment variables
  unsigned long seg_len;    //segment length
  unsigned long seg_seq;    //segment sequence number
  unsigned long seg_ack;    //segment acknoledgement number

  //timing information (round-trip)
  short t_rtt; //round-trip time
  unsigned long rtseq; //starting sequence number of timed data
  short rttmin;
  short srtt; //smoothed round-trip time
  short rttvar; //variance of round-trip time
  double last_timed_data; //timestamp for measurement

  // retransmission timeout
  short rxtcur;
  // backoff for rto
  short rxtshift;
  bool rexmt_sch;


  // duplicate ack counter
  short dupacks;

  // timer messages
  cMessage *timeout_rexmt_msg;
  cMessage *timeout_conn_estab_msg;
  cMessage *timeout_persist_msg;
  cMessage *timeout_keepalive_msg;
  cMessage *timeout_finwait2_msg;
  cMessage *timeout_delayed_ack_msg;

  //max. ack. delay
  double max_del_ack;
  //bool to handle delayed ACKs
  bool ack_sch;

  //number of bits requested by the application
  long num_pks_req;

  //last time a segment was send
  double last_snd_time;

  //application notification variable
  bool tcp_app_notified;

  //ACK times
  double ack_send_time;
  double ack_rcv_time;

  //bool to handle the time wait timer
  bool time_wait_sch;
  bool finwait2_sch;


  //variable to indicate if a SYN has been received
  int syn_rcvd;

  //connection establishment variable
  int conn_estab;

  // FIXME: OBSOLETE
  //variables to manage a passive open
  //   int passive;           //set, if the connection was initiated by a passive open
  //   int passive_rem_addr;  //copy of the remote address in case of a passive open
  //   int passive_rem_port;  //copy of the remote port in case of a passive open

  //variables for the management of dup_retrans_queue in retransQueueProcess(...)
  cQueue dup_retrans_queue;
  bool dup_retrans_queue_init;
  bool create_dup_retrans_queue;


  // private member functions
  void _init();
 public:

  TCPCB(const TCPCB& tcpcb);
  TCPCB(IPAddress laddr, PortNumber lport, IPAddress faddr, PortNumber fport);
  TCPCB();
  TCPCB(const char* name);
  virtual ~TCPCB();
  virtual cObject* dup() const {return new TCPCB(*this);}
  TCPCB& operator=(const TCPCB& tcpcb);
};


#endif
