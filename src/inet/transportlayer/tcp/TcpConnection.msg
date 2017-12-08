//
// Copyright (C) 2016 OpenSim Ltd
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
// author: Zoltan Bojthe
//


import inet.common.INETDefs;
import inet.common.packet.ChunkBuffer;
import inet.common.packet.ChunkQueue;
import inet.common.packet.Packet;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
}}

namespace inet::tcp;

class TcpSackRexmitQueue { @existingClass; }
class TcpAlgorithm extends cObject { @existingClass; }

struct TcpStateVariables
{
    @existingClass;
    @descriptor(readonly);

    bool active;    // set if the connection was initiated by an active open
    bool fork;    // if passive and in LISTEN: whether to fork on an incoming connection

    uint32_t snd_mss;    // sender maximum segment size (without headers, i.e. only segment text); see RFC 2581, page 1.
                       // This will be set to the minimum of the local smss parameter and the value specified in the
                       // MSS option received during connection setup.

    // send sequence number variables (see RFC 793, "3.2. Terminology")
    uint32_t snd_una;    // send unacknowledged
    uint32_t snd_nxt;    // send next (drops back on retransmission)
    uint32_t snd_max;    // max seq number sent (needed because snd_nxt is re-set on retransmission)
    uint32_t snd_wnd;    // send window
    uint32_t snd_up;    // send urgent pointer
    uint32_t snd_wl1;    // segment sequence number used for last window update
    uint32_t snd_wl2;    // segment ack. number used for last window update
    uint32_t iss;    // initial sequence number (ISS)

    // receive sequence number variables
    uint32_t rcv_nxt;    // receive next
    uint32_t rcv_wnd;    // receive window
    uint32_t rcv_up;    // receive urgent pointer;
    uint32_t irs;    // initial receive sequence number
    uint32_t rcv_adv;    // advertised window

    // SYN, SYN+ACK retransmission variables (handled separately
    // because normal rexmit belongs to TcpAlgorithm)
    int syn_rexmit_count;    // number of SYN/SYN+ACK retransmissions (=1 after first rexmit)
    simtime_t syn_rexmit_timeout;    // current SYN/SYN+ACK retransmission timeout

    // whether ACK of our FIN has been received. Needed in FIN bit processing
    // to decide between transition to TIME-WAIT and CLOSING (set event code
    // TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK).
    bool fin_ack_rcvd;

    bool send_fin;    // true if a user CLOSE command has been "queued"
    uint32_t snd_fin_seq;    // if send_fin == true: FIN should be sent just before this sequence number

    bool fin_rcvd;    // whether FIN received or not
    uint32_t rcv_fin_seq;    // if fin_rcvd: sequence number of received FIN

    uint32_t sentBytes;    // amount of user data (in bytes) sent in last segment

    bool nagle_enabled;    // set if Nagle's algorithm (RFC 896) is enabled
    bool delayed_acks_enabled;    // set if delayed ACK algorithm (RFC 1122) is enabled
    bool limited_transmit_enabled;    // set if Limited Transmit algorithm (RFC 3042) is enabled
    bool increased_IW_enabled;    // set if Increased Initial Window (RFC 3390) is enabled

    uint32_t full_sized_segment_counter;    // this counter is needed for delayed ACK
    bool ack_now;    // send ACK immediately, needed if delayed_acks_enabled is set
                     // Based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861].
                     // ack_now should be set when:
                     //   - delayed ACK timer expires
                     //   - an out-of-order segment is received
                     //   - SYN is received during the three-way handshake
                     //   - a persist probe is received
                     //   - FIN is received

    bool afterRto;    // set at RTO, reset when snd_nxt == snd_max or snd_una == snd_max

    // WINDOW_SCALE related variables
    bool ws_support;    // set if the host supports Window Scale (header option) (RFC 1322)
    bool ws_enabled;    // set if the connection uses Window Scale (header option)
    int  ws_manual_scale; // the value of scale parameter if it was set manually (-1 otherwise)
    bool snd_ws;    // set if initial WINDOW_SCALE has been sent
    bool rcv_ws;    // set if initial WINDOW_SCALE has been received
    unsigned int rcv_wnd_scale;    // RFC 1323, page 31: "Receive window scale power"
    unsigned int snd_wnd_scale;    // RFC 1323, page 31: "Send window scale power"

    // TIMESTAMP related variables
    bool ts_support;    // set if the host supports Timestamps (header option) (RFC 1322)
    bool ts_enabled;    // set if the connection uses Window Scale (header option)
    bool snd_initial_ts;    // set if initial TIMESTAMP has been sent
    bool rcv_initial_ts;    // set if initial TIMESTAMP has been received
    uint32_t ts_recent;    // RFC 1323, page 31: "Latest received Timestamp"
    uint32_t last_ack_sent;    // RFC 1323, page 31: "Last ACK field sent"
    simtime_t time_last_data_sent;    // time at which the last data segment was sent (needed to compute the IDLE time for PAWS)

    // SACK related variables
    bool sack_support;    // set if the host supports selective acknowledgment (header option) (RFC 2018, 2883, 3517)
    bool sack_enabled;    // set if the connection uses selective acknowledgment (header option)
    bool snd_sack_perm;    // set if SACK_PERMITTED has been sent
    bool rcv_sack_perm;    // set if SACK_PERMITTED has been received
    uint32_t start_seqno;    // start sequence number of last received out-of-order segment
    uint32_t end_seqno;    // end sequence number of last received out-of-order segment
    bool snd_sack;    // set if received vaild out-of-order segment or rcv_nxt changed, but receivedQueue is not empty
    bool snd_dsack;    // set if received duplicated segment (sequenceNo+PLength < rcv_nxt) or (segment is not acceptable)
//FIXME show it    SackList sacks_array;    // MAX_SACK_BLOCKS is set to 60
    uint32_t highRxt;    // RFC 3517, page 3: ""HighRxt" is the highest sequence number which has been retransmitted during the current loss recovery phase."
    uint32_t pipe;    // RFC 3517, page 3: ""Pipe" is a sender's estimate of the number of bytes outstanding in the network."
    uint32_t recoveryPoint;    // RFC 3517
    uint32_t sackedBytes;    // number of sackedBytes
    uint32_t sackedBytes_old;    // old number of sackedBytes - needed for RFC 3042 to check if last dupAck contained new sack information
    bool lossRecovery;    // indicates if algorithm is in loss recovery phase

    // queue management
    uint32_t sendQueueLimit;
    bool queueUpdate;

    // those counters would logically belong to TcpAlgorithm, but it's a lot easier to manage them here
    uint32_t dupacks;    // current number of received consecutive duplicate ACKs
    uint32_t snd_sacks;    // number of sent sacks
    uint32_t rcv_sacks;    // number of received sacks
    uint32_t rcv_oooseg;    // number of received out-of-order segments
    uint32_t rcv_naseg;    // number of received not acceptable segments

    // receiver buffer / receiver queue related variables
    uint32_t maxRcvBuffer;    // maximal amount of bytes in tcp receive queue
    uint32_t usedRcvBuffer;    // current amount of used bytes in tcp receive queue
    uint32_t freeRcvBuffer;    // current amount of free bytes in tcp receive queue
    uint32_t tcpRcvQueueDrops;    // number of drops in tcp receive queue
};

class TcpSendQueue extends cObject
{
    @existingClass;
    @descriptor(readonly);
    ChunkQueue dataBuffer;
}

class TcpReceiveQueue extends cObject
{
    @existingClass;
    @descriptor(readonly);
    ReorderBuffer reorderBuffer;
}

class TcpConnection extends cObject
{
    @existingClass;
    @descriptor(readonly);

    int socketId;    // identifies connection within the app
    int listeningSocketId;    // identifies listening connection within the app

    // socket pair
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;

    TcpStateVariables *state;

    // TCP queues
    TcpSendQueue *sendQueue;
    TcpReceiveQueue *receiveQueue;
    TcpSackRexmitQueue *rexmitQueue;

    TcpAlgorithm *tcpAlgorithm;
    int fsmState;
};

