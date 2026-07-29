//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.common.L3Address;

namespace inet;

//
// TCP command codes, sent by the application to TCP. These constants
// should be set as message kind on a message sent to the TCP entity.
//
// @see ~TcpCommand, ~TcpOpenCommand, ~ITcp
//
enum TcpCommandCode
{
    TCP_C_OPEN_ACTIVE = 1;   // active open (must carry ~TcpOpenCommand)
    TCP_C_OPEN_PASSIVE = 2;  // passive open (must carry ~TcpOpenCommand)
    TCP_C_ACCEPT = 3;        // accept connection
    TCP_C_SEND = 4;          // send data (set on data packet)
    TCP_C_CLOSE = 5;         // "I have no more data to send"
    TCP_C_ABORT = 6;         // abort connection
    TCP_C_STATUS = 7;        // request status info (TCP_I_STATUS) from TCP
    TCP_C_QUEUE_BYTES_LIMIT = 8; // set send queue limit (in bytes)
    TCP_C_READ = 9;          // send request to TCP to deliver data
    TCP_C_DESTROY = 10;      // send request to TCP to destroy the socket descriptor
    TCP_C_SETOPTION = 11;    // setting TCP socket options
}

//
// TCP indications, sent by TCP to the application. TCP will set these
// constants as message kind on messages it sends to the application.
//
// @see ~TcpCommand, ~TcpStatusInfo, ~ITcp
//
enum TcpStatusInd
{
    TCP_I_DATA = 1;              // data packet (set on data packet)
    TCP_I_URGENT_DATA = 2;       // urgent data (set on data packet)
    TCP_I_AVAILABLE = 3;         // conncetion available
    TCP_I_ESTABLISHED = 4;       // connection established
    TCP_I_PEER_CLOSED = 5;       // FIN received from remote TCP
    TCP_I_CLOSED = 6;            // connection closed normally (via FIN exchange)
    TCP_I_CONNECTION_REFUSED = 7; // connection refused
    TCP_I_CONNECTION_RESET = 8;  // connection reset
    TCP_I_TIMED_OUT = 9;         // conn-estab timer went off, or max retransm. count reached
    TCP_I_STATUS = 10;           // status info (will carry ~TcpStatusInfo)
    TCP_I_SEND_MSG = 11;         // send queue abated, send more messages
    TCP_I_ICMPv4_ERROR = 12;       // ICMPv4 error received (carries ~Icmpv4ErrorInd tag)
    TCP_I_ICMPv6_ERROR = 13;     // ICMPv6 error received (carries ~Icmpv6ErrorInd tag)
    TCP_I_ZEROCOPY_COMPLETION = 14; // a zerocopy-marked SEND's data has been transmitted (MSG_ZEROCOPY; carries ~TcpZerocopyCompletionInfo)
}

//
// Currently not in use.
//
enum TcpErrorCode
{
}

//
// Control info for TCP connections. This class is to be set as control info
// (see cMessage::setControlInfo()) on all messages exchanged between TCP and
// application, in both directions. Some commands and indications
// (TCP_C_OPEN_xxx, TCP_I_STATUS) use subclasses.
//
// connId identifies the connection locally within the application (internally,
// TCP uses the (app gate index, connId) pair to identify the socket).
// connId is to be chosen by the application in the open command.
//
//# TODO explain userId
//
// @see ~TcpCommandCode, ~TcpStatusInd, ~TcpOpenCommand, ~TcpStatusInfo, ~ITcp
//
class TcpCommand extends cObject
{
    int userId = -1;   // id than can be freely used by the app
    bool halfClose = false; // only meaningful on TCP_C_CLOSE: true = shutdown(SHUT_WR) semantics
                            // (send FIN but the application keeps reading); false = full close()
                            // (Linux sk_shutdown gets RCV_SHUTDOWN too: new data arriving after a
                            // full close resets the connection, RFC 1122 4.2.2.13)
}

//
// Currently not in use.
//
class TcpErrorInfo extends TcpCommand
{
    TcpErrorCode errorCode;
    string messageText;
}

//
// Control info to be used for active or passive TCP open.
//
// localAddr, remoteAddr, localPort, remotePort should be self-explanatory.
// localAddr is optional because TCP can learn it from IP when a packet
// is received from the peer; localPort is optional because TCP supports
// ephemeral ports.
//
// The fork parameter is used with passive open, and controls what happens
// when an incoming connection is received. With fork=true, it emulates
// the Unix accept(2) syscall semantics: a new connection structure
// is created for the connection (with a new connId, see in ~TcpCommand),
// and the connection structure with the old connId remains listening.
// With fork=false, all the above does not happen: the first connection
// is accepted (with the original connId), and further incoming connections
// will be refused by TCP by sending an RST segment.
//
// The tcpAlgorithmClass field allows per-connection TCP configuration.
// The tcpAlgorithmClass field may contain name of class subclassed from
// TcpAlgorithm, respectively.
// If not set, module parameters with similar names are used.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpOpenCommand extends TcpCommand
{
    L3Address localAddr;   // may be left empty
    L3Address remoteAddr;  // required for active open
    int localPort = -1;    // required for passive open
    int remotePort = -1;   // required for active open
    bool fork = false;     // used only for passive open
    bool autoRead = true;  // true: TCPs sends up arrived data automatically. false: should use read command
    string tcpAlgorithmClass; // TCP congestion control algorithm; leave empty for default
    bool fastOpen = false; // active open only: attempt TCP Fast Open (RFC 7413) -- attach data to the SYN if a cookie is already cached for remoteAddr, else request one
}

//
// Control info that represents a READ request. ~Tcp only accepts READ requests
// from the user when the connection was opened with autoRead=false.
//
// ~Tcp will respond to the reception of a READ request with data; either
// from previously received and buffered data, or data received from the network.
// Only one READ request may be outstanding, ~Tcp will not accept another READ
// until the previous one has been satisfied.
//
// The maxByteCount field specifies the maximum number of bytes to be read.
// Note that ~Tcp will respond as soon as any data becomes available,
// so the response is often smaller than maxByteCount.
//
// @see ~TcpOpenCommand
//
class TcpReadCommand extends TcpCommand
{
    int32_t maxByteCount; // size limit for the data response
}

//
// Control info to be used to accept an available connection.
//
class TcpAcceptCommand extends TcpCommand
{
}

//
// Base class for TCP socket option control info classes.
//
class TcpSetOptionCommand extends TcpCommand
{
}

class TcpSetTimeToLiveCommand extends TcpSetOptionCommand
{
    int ttl;
}

class TcpSetDscpCommand extends TcpSetOptionCommand
{
    short dscp;   // dscp for Ipv4/Ipv6
}

class TcpSetTosCommand extends TcpSetOptionCommand
{
    short tos;   // type of service for Ipv4 / traffic class for Ipv6
}

//
// SO_TIMESTAMPING/SCM_TIMESTAMPING: enables/disables attaching a
// TcpRxTimestampInd tag (see TcpTimestampingTag.msg) to every TCP_I_DATA packet
// delivered to the app on this connection.
//
// @see ~TcpSocket::setTimestamping
//
class TcpSetTimestampingCommand extends TcpSetOptionCommand
{
    bool enabled = true;
}

//
// TCP_NOTSENT_LOWAT as a runtime socket option: sets
// the connection's not-yet-transmitted-bytes low-water mark at runtime, same
// semantics as the notsentLowat module parameter (which supplies the initial
// value at connection setup). -1 disables.
//
// @see ~TcpSocket::setNotsentLowat
//
class TcpSetNotsentLowatCommand extends TcpSetOptionCommand
{
    int value = -1;
}

//
// TCP_MAXSEG (setsockopt SOL_TCP): the application clamps the Maximum Segment
// Size for this connection. Like Linux's rx_opt.user_mss, it caps both the MSS
// this host advertises in its SYN/SYN-ACK and the effective sending MSS. -1
// (default) means "not set". May be sent before connect()/listen().
//
class TcpSetMaxSegCommand extends TcpSetOptionCommand
{
    int value = -1;
}

//
// The route's MTU changed under an open connection (Linux: a new dst_mtu, which
// tcp_current_mss picks up as icsk_pmtu_cookie). It raises the ceiling the RFC 4821
// MTU search may work up to; it does not by itself change the MSS in use, which only
// moves once a probe of the larger size has been acknowledged.
//
// @see ~TcpSocket::setPathMtu
//
class TcpSetPathMtuCommand extends TcpSetOptionCommand
{
    int value = 0;
}

//
// SO_RCVBUF as a runtime socket option: sets this connection's receive-BUFFER
// capacity (Linux sk_rcvbuf) and pins it, so the kernel's automatic growth under
// receive pressure no longer applies (SOCK_RCVBUF_LOCK). `value` is the final
// buffer size -- the socket layer has already applied Linux's doubling of the
// setsockopt argument. Shrinking it below what is already queued is legal and is
// how an application forces a memory squeeze: nothing is discarded, but the next
// arrival has nowhere to go.
//
// @see ~TcpSocket::setReceiveBufferSize
//
class TcpSetRcvBufCommand extends TcpSetOptionCommand
{
    int value = -1;
}

//
// TCP_NODELAY (setsockopt SOL_TCP): enable/disable Nagle's algorithm at runtime.
// nodelay==true disables Nagle and force-pushes any held partial segment (but does
// NOT clear TCP_CORK, which outranks TCP_NODELAY in Linux).
//
// Marks whether the connection is OWNED by an application socket (Linux
// sk->sk_socket): a listening-side connection is embryonic until the
// application accept()s it, and several kernel behaviors are gated on
// ownership -- e.g. tcp_data_queue_ofo's "do not grow rcvbuf for
// not-yet-accepted or orphaned sockets". Default is owned (active opens,
// forked-and-accepted connections); a harness driving accept() timing
// explicitly clears it at listen and sets it at the script's accept().
class TcpSetOwnedCommand extends TcpSetOptionCommand
{
    bool owned = true;
}

// The application writer's blocked-on-send-buffer state (Linux SOCK_NOSPACE
// while a blocking write waits in sk_stream_wait_memory): while set AND the
// send queue has run dry, the connection accumulates SNDBUF_LIMITED chrono
// time (tcpi_sndbuf_limited). Driven by the application layer, which is where
// Linux's signal originates too.
class TcpSetWriterBlockedCommand extends TcpSetOptionCommand
{
    bool blocked = false;
}

class TcpSetNoDelayCommand extends TcpSetOptionCommand
{
    bool nodelay = true;
}

//
// TCP_CORK (setsockopt SOL_TCP): hold sub-MSS partial segments (full segments still
// flow) until the cork is cleared, an incoming ACK opens the window, or the cork
// (RTO) timer fires. Clearing the cork (cork==false) flushes any held partial.
//
class TcpSetCorkCommand extends TcpSetOptionCommand
{
    bool cork = true;
}

//
// Sent with message kind TCP_I_ZEROCOPY_COMPLETION, in response to a
// zerocopy-marked SEND (MSG_ZEROCOPY -- see
// ~TcpSendZerocopyReq/~TcpSocket::sendZerocopy) once that SEND's data has
// been transmitted.
//
// @see ~TcpStatusInd, ~TcpCommandCode, ~ITcp
//
class TcpZerocopyCompletionInfo extends TcpCommand
{
    unsigned int zerocopyId;
}

//
// Sent with message kind TCP_I_AVAILABLE, to let the app know
// about the local and remote IP address and port.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpAvailableInfo extends TcpCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
    int newSocketId;
    bool autoRead;
}

//
// Sent with message kind TCP_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see ~TcpCommandCode, ~ITcp
//
class TcpConnectInfo extends TcpCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
    bool autoRead;   // true: TCP send up arrived data automatically. false: should use read command.
}

//
// Sent with message kind TCP_I_STATUS, in response to command TCP_C_STATUS.
// For explanation of variables, see RFC 9293 or TcpStateVariables in
// TcpConnection.h.
//
// @see ~TcpStatusInd, ~TcpCommandCode, ~ITcp
//
class TcpStatusInfo extends TcpCommand
{
    int state;
    string stateName;

    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
    bool autoRead;   // true: TCP send up arrived data automatically. false: should use read command.

    unsigned int snd_mss;
    unsigned int sndEffMss;  // effective send MSS after header options (Linux tcp_current_mss: e.g. 1448 with timestamps on a 1460 path) -- tcpi_snd_mss reports THIS
    unsigned int advmss;   // the MSS this host advertised in its SYN/SYN-ACK (state->advertisedMss); Linux tcp_info tcpi_advmss

    unsigned int snd_una;
    unsigned int snd_nxt;
    unsigned int snd_max;
    unsigned int snd_wnd;
    unsigned int snd_up;
    unsigned int snd_wl1;
    unsigned int snd_wl2;
    unsigned int iss;

    unsigned int rcv_nxt;
    unsigned int rcv_wnd;
    unsigned int rcv_up;
    unsigned int irs;

    bool fin_ack_rcvd;

    // Additional fields below bridge a Linux struct tcp_info-style status query
    // (e.g. for packetdrill's %{ }% assertion blocks) -- not part of RFC 793,
    // sourced from the congestion-control/loss-recovery state variables where
    // available. Fields with no equivalent for the connection's flavour (e.g.
    // DumbTcp has no congestion window) are set to UINT_MAX/-1 as a sentinel;
    // callers must check for that before using the value.
    unsigned int cwnd;
    unsigned int ssthresh;
    unsigned int reordering;
    double srtt;
    double minRtt;
    unsigned int flightSize;
    unsigned int sackedBytes;
    unsigned int deliveredBytes;
    unsigned int rexmitCount;
    unsigned int numRtos;
    bool tsEnabled;
    bool sackEnabled;
    bool wsEnabled;
    bool ectEnabled;
    bool synDataAccepted;   // server accepted TFO SYN data ahead of the 3WHS (-> TCPI_OPT_SYN_DATA)
    unsigned int sndWndScale;
    simtime_t lastDataRecvTime;

    // Linux tcp_ca_state ordinal (Open=0/Disorder=1/CWR=2/Recovery=3/Loss=4), derived
    // from INET's independent loss-recovery bools -- see
    // TcpConnection::deriveLinuxCaState(). INET never tracks Disorder(1) as a distinct
    // state; see that function's doc comment.
    int caState;
    // Backoff/retransmit counter: same underlying counter as rexmitCount above (INET
    // does not track Linux's tcpi_backoff and tcpi_retransmits as separate concepts).
    unsigned int backoff;
    // SACK/RACK-tracked lost bytes (TcpSackRexmitQueue::getTotalAmountOfLostBytes()),
    // converted to an approximate segment count via integer division by snd_mss -- a
    // lossy but honest approximation of Linux's tcpi_lost, which counts segments
    // directly. UINT_MAX if SACK is not enabled on this connection (no loss tracking).
    unsigned int lost;
    // Retransmitted-and-still-outstanding segments (TcpSackRexmitQueue::getRetrans()),
    // same segment approximation and UINT_MAX sentinel as `lost` (Linux tcpi_retrans /
    // retrans_out).
    unsigned int retrans;
    // Linux SK_MEMINFO_RCVBUF (sk->sk_rcvbuf): the live receive-buffer
    // capacity -- receiveBufferSize as configured, possibly GROWN by
    // tcp_clamp_window under out-of-order buffer pressure on an
    // application-owned socket. 0 when no buffer size is configured.
    unsigned int skRcvbuf;
    // Cumulative zero-window (persist) probes sent (TcpBaseAlgStateVariables::
    // zeroWindowProbesSent). UINT_MAX for flavours without persist-probe tracking.
    unsigned int probes;
    // Total bytes received in-sequence so far (rcv_nxt - irs - 1).
    unsigned int bytesReceived;
    // AccECN sender-side resolved CE-marked packet/byte counts
    // (state->deliveredCePkts/deliveredCeBytes); 0 (not a sentinel) when AccECN was
    // never negotiated on this connection, matching Linux's own tcp_info behavior for
    // an ECN-less connection.
    unsigned int deliveredCePkts;
    unsigned int deliveredCeBytes;
    // AccECN peer-reported ECT0/ECT1 delivered-byte counts (state->deliveredE0Bytes/E1Bytes),
    // reported as tcpi_delivered_e0_bytes / tcpi_delivered_e1_bytes.
    unsigned int deliveredE0Bytes;
    unsigned int deliveredE1Bytes;

    // Cumulative time (seconds) the connection was busy sending data, and time
    // specifically blocked by the peer's advertised window rather than the
    // congestion window -- see TcpConnection::enqueueSendCommandData()/
    // processAckInEstabEtc()/sendData() (INET-native bookkeeping, not a port of
    // Linux's tcpi_busy_time/tcpi_rwnd_limited, which are cumulative microseconds
    // from a kernel-internal chrono state machine this reimplements independently).
    // sndbuf_limited has no INET analog and is not surfaced (INET's send queue is
    // never actually blocking -- see notsentLowat/sendQueueLimit above).
    double busyTime;
    double rwndLimited;
    double sndbufLimited; // Linux tcpi_sndbuf_limited (TCP_CHRONO_SNDBUF_LIMITED), seconds -- see sndbufLimitedAccumulated
}
