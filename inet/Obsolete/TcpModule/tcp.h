//-----------------------------------------------------------------------------
//-- fileName: tcp.h
//--
//-- generated to test the TCP-FSM
//--
//-- V. Boehm, June 19 1999
//--
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


#ifndef _TCP_H_
#define _TCP_H_

#include <omnetpp.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

//2**31 = 2147483648, 32 bit -> 4 bytes -> 0 ... 4294967295
#define delta ((unsigned long) 1 << 31)


// message kind values
enum TcpCommand
{
  // commands (Application -> TCP)
  TCP_C_OPEN_ACTIVE = 1,
  TCP_C_OPEN_PASSIVE,
  TCP_C_SEND,
  TCP_C_RECEIVE,
  TCP_C_CLOSE,
  TCP_C_ABORT,
  TCP_C_STATUS,

  // TPDU types
  SYN_SEG,
  ACK_SEG,
  RST_SEG,
  FIN_SEG,
  TCP_SEG,
  SYN_DATA,
  ACK_DATA,
  RST_DATA,
  FIN_DATA,
  TCP_DATA,

  // self messages
  TIMEOUT_TIME_WAIT,
  TIMEOUT_REXMT,
  TIMEOUT_PERSIST,
  TIMEOUT_KEEPALIVE,
  TIMEOUT_CONN_ESTAB,
  TIMEOUT_FIN_WAIT_2,
  TIMEOUT_DELAYED_ACK
};


//TCP segment names
//enum TcpNames {SYN_SEG,
//               ACK_SEG,
//               RST_SEG,
//               FIN_SEG,
//               TCP_SEG};

//TCP data packet names
//enum TcpDataNames {SYN_DATA,
//                   ACK_DATA,
//                   RST_DATA,
//                   FIN_DATA,
//                   TCP_DATA};

//TCP connection status information (TCP -> Application)
enum TcpStatusInfo
{
  TCP_I_NONE = 0,
  TCP_I_SEG_FWD,
  TCP_I_RCVD_SYN,
  TCP_I_ESTAB,
  TCP_I_CLOSE_WAIT,
  TCP_I_CLOSED,
  TCP_I_ABORTED
};

//TCP port ID (does not work with gate("out")->...)
//typedef int Tcp_Port;

//TCP local connection ID
typedef int TcpConnId;


//TCP variables and defs
//TCP states, also used for FSM
enum TcpState
{
  TCP_S_INIT        = 0,
  TCP_S_CLOSED      = FSM_Steady(1),
  TCP_S_LISTEN      = FSM_Steady(2),
  TCP_S_SYN_SENT    = FSM_Steady(3),
  TCP_S_SYN_RCVD    = FSM_Steady(4),
  TCP_S_ESTABLISHED = FSM_Steady(5),
  TCP_S_CLOSE_WAIT  = FSM_Steady(6),
  TCP_S_LAST_ACK    = FSM_Steady(7),
  TCP_S_FIN_WAIT_1  = FSM_Steady(8),
  TCP_S_FIN_WAIT_2  = FSM_Steady(9),
  TCP_S_CLOSING     = FSM_Steady(10),
  TCP_S_TIME_WAIT   = FSM_Steady(11)
};



//TCP flags
enum TcpFlag {TCP_F_NSET = 0, TCP_F_SET = 1};
//tcp_flag TCP_FLAG_FIN, TCP_FLAG_SYN, TCP_FLAG_RST, TCP_FLAG_ACK;
//tcp_flag TCP_FLAG_PSH, TCP_FLAG_URG;

//These flags can be combined using OR (|), which means that all the flags
//combined with OR are set!
//#define TCP_FLAG_NONE   0x00
//#define TCP_FLAG_FIN    0x01
//#define TCP_FLAG_SYN    0x02
//#define TCP_FLAG_RST    0x04
//#define TCP_FLAG_PSH    0x08
//#define TCP_FLAG_ACK    0x10
//#define TCP_FLAG_URG    0x20

//events of the TCP_FSM
enum TcpEvent
{
  TCP_E_NONE = 0,
  TCP_E_OPEN_ACTIVE,
  TCP_E_OPEN_PASSIVE,
  TCP_E_SEND,
  TCP_E_RECEIVE,
  TCP_E_CLOSE,
  TCP_E_ABORT,
  TCP_E_STATUS,
  TCP_E_SEG_ARRIVAL,
  TCP_E_RCV_SYN,
  TCP_E_RCV_SYN_ACK,
  TCP_E_RCV_ACK_OF_SYN,
  TCP_E_RCV_FIN,
  TCP_E_RCV_ACK_OF_FIN,
  TCP_E_RCV_FIN_ACK_OF_FIN,
  TCP_E_TIMEOUT_TIME_WAIT,
  TCP_E_TIMEOUT_REXMT,
  TCP_E_TIMEOUT_PERSIST,
  TCP_E_TIMEOUT_KEEPALIVE,
  TCP_E_TIMEOUT_CONN_ESTAB,
  TCP_E_TIMEOUT_FIN_WAIT_2,
  TCP_E_TIMEOUT_DELAYED_ACK,
  TCP_E_PASSIVE_RESET,
  TCP_E_ABORT_NO_RST
};

//event structure
struct TcpStEvent
{
  TcpEvent  event;
  cMessage* pmsg;
  int       num_msg;
  TcpFlag   th_flag_urg;
  TcpFlag   th_flag_ack;
  TcpFlag   th_flag_psh;
  TcpFlag   th_flag_rst;
  TcpFlag   th_flag_syn;
  TcpFlag   th_flag_fin;
};


enum MsgSource {FROM_APPL, FROM_IP, FROM_TIMEOUT};


//TCP header definitions
struct TcpHeader
{
  //Source Port
  int th_src_port;

  //Destination Port
  int th_dest_port;

  //Sequence Number: first sequence number of the first data octet
  //in the respective segment (except if SYN is set; then the the
  //seq. number is the initial seq. number (ISS) and the first data
  //octet is ISS+1)
  unsigned int th_seq_no;

  //Acknowledgement Number: if ACK flag is set, this field contains
  //the next sequence number the sender of this segment is expecting
  //to receive
  unsigned int th_ack_no;

  //Data Offset: number of 32 bit words in the header
  //(no options are used, therefore the number of 32 bit words
  //in the header is 5 (= 20bytes))
  short th_data_offset;

  //Reserved: for future use, not included here

  //Control Bits
  TcpFlag th_flag_urg; //urgent pointer field significant if set
  TcpFlag th_flag_ack; //ack. number field significant if set
  TcpFlag th_flag_psh; //push function
  TcpFlag th_flag_rst; //reset the connection
  TcpFlag th_flag_syn; //synchronize seq. numbers
  TcpFlag th_flag_fin; //no more data from sender

  //Window: the number of data octets beginning with the one indicated
  //in the acknowledegment field which the sender of this segment is
  //willing to accept
  unsigned long th_window;

  //Checksum: for error checking purposes, not included here

  //Urgent Pointer (better urgent offset)
  unsigned long th_urg_pointer;

  //Options: no options included here (we assume that the MSS (max.
  //segment size) has the default value of 536 bytes)

  //Padding: not included here
};

//SegRecord: Record of a received segment
//that stores segments that have arrived out of
//order until the preceding data has arrived
struct SegRecord
{
  unsigned long seq;
  cMessage* pdata;
};

// Socket Pair
class SockPair {
public:
  int local_port;
  int local_address;
  int rem_port;
  int rem_address;

  void set(int l_port, int l_addr, int r_port, int r_addr)
  {local_port = l_port; local_address = l_addr;
  rem_port = r_port; rem_address = r_addr;};

  // Constructors
  SockPair(){set(-1,-1,-1,-1);};

  SockPair(int l_port, int l_addr, int r_port, int r_addr){set(l_port, l_addr, r_port, r_addr);};


  //used for key function in map<...>
  //bool operator<(const SockPair& comp_sock) const
  //{
  //  if (local_port < comp_sock.local_port)
  //    return true;
  //  else if (rem_port < comp_sock.rem_port)
  //    return true;
  //  else if (rem_address < comp_sock.rem_address)
  //    return true;
  //  else if (local_address < comp_sock.local_address)
  //    return true;
  //  else
  //    return false;
  //};

    //used for key function in map<...>
  bool operator<(const SockPair& comp_sock) const
  {
    if (local_port < comp_sock.local_port)
      return true;
    else if (local_port > comp_sock.local_port)
      return false;
    else if (local_address < comp_sock.local_address)
      return true;
    else if (local_address > comp_sock.local_address)
      return false;
    else if (rem_port < comp_sock.rem_port)
      return true;
    else if (rem_port > comp_sock.rem_port)
      return false;
    else if (rem_address < comp_sock.rem_address)
      return true;
    else
      return false;
  };

};

//TCP transmission control block (TCB)
struct TcpTcb
{
  //TCP finite state machine
  cFSM fsm;                          // stores a TcpState enum

  //TCP queues
  cQueue tcp_send_queue;             //tcp_send_queue: queue data not yet sent/queue send calls from appl.
  cQueue tcp_retrans_queue;          //tcp_retrans_queue: queue data which has been sent but not yet acknowledged
  cQueue tcp_data_receive_queue;     //to queue the data of incoming segments
  cQueue tcp_socket_queue;           //socket buffer
  cLinkedList tcp_rcv_rec_list;      //receive record list

  //application and connection identification information
  TcpConnId tb_conn_id;
  TcpState tb_prev_state;

  //event information
  TcpStEvent st_event;

  //status information
  TcpStatusInfo status_info;

  //socket information
  int local_port; //local TCP-port
  int local_addr; //local IP-address
  int rem_port;   //TCP destination port
  int rem_addr;   //destination IP-address

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

  //Rolf
  unsigned long recover;          //save-variable for snd_max in NewReno
  unsigned long new_ack;          //save-variable for the amount of new data acknowledged (NewReno)
  unsigned long partial_ack_no;   //save-variable for the partial Ack's number (NewReno)
  bool partial_ack;               //remember if we have just had a partial Ack during Fast Recovery in NewReno
  unsigned long savecwnd;         //save-variable for snd_cwnd in case of a partial acknowledgement (NewReno)
  //Rolf

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
  bool per_sch;          // if persist timer is scheduled or not

  //variable to indicate if a SYN has been received
  int syn_rcvd;

  //connection establishment variable
  int conn_estab;

  //variables to manage a passive open
  bool passive;          //set, if the connection was initiated by a passive open
  int passive_rem_addr;  //copy of the remote address in case of a passive open
  int passive_rem_port;  //copy of the remote port in case of a passive open

  //variables for the management of dup_retrans_queue in retransQueueProcess(...)
  cQueue dup_retrans_queue;
  bool dup_retrans_queue_init;
  bool create_dup_retrans_queue;
};

#endif // _TCP_H_

