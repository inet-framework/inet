.. _ug:cha:transport-protocols:

Transport Protocols
===================

.. _ug:sec:transport:overview:

Overview
--------

In the OSI reference model, the protocols of the transport layer provide
host-to-host communication services for applications. They provide
services such as connection-oriented communication, reliability, flow
control, and multiplexing.

INET currently provides support for the :protocol:`TCP`, :protocol:`UDP`,
:protocol:`SCTP` and :protocol:`RTP` transport layer protocols.
INET nodes like :ned:`StandardHost` contain optional and replaceable
instances of these protocols, like this:

.. code-block:: ned

   tcp: <tcpType> like ITcp if hasTcp;
   udp: <udpType> like IUdp if hasUdp;
   sctp: <sctpType> like ISctp if hasSctp;

As RTP is more specialized that the other ones (multimedia streaming),
INET provides a separate node type, :ned:`RtpHost`, for modeling RTP
traffic.

.. _ug:sec:transport:tcp:

TCP
---

.. _ug:sec:transport:tcp-overview:

Overview
~~~~~~~~

TCP protocol is the most widely used protocol of the Internet. It
provides reliable, ordered delivery of stream of bytes from one
application on one computer to another application on another computer.
The baseline TCP protocol is described in RFC793, but other tens of RFCs
contains modifications and extensions to the TCP. As a result, TCP is a
complex protocol and sometimes it is hard to see how the different
requirements interact with each other.

INET contains three implementations of the TCP protocol:

-  :ned:`Tcp` is the primary implementation, designed for readability,
   extensibility, and experimentation.

-  :ned:`TcpLwip` is a wrapper around the lwIP (Lightweight IP) library,
   a widely used open source TCP/IP stack designed for embedded systems.

-  :ned:`TcpNsc` wraps Network Simulation Cradle (NSC), a library that
   allows real world TCP/IP network stacks to be used inside a network
   simulator.

All three module types implement the :ned:`ITcp` interface and
communicate with other layers through the same interface, so they can be
interchanged and also mixed in the same network.

.. _ug:sec:transport:tcpcore:

Tcp
~~~

The :ned:`Tcp` simple module is the main implementation of the TCP
protocol in the INET framework.

:ned:`Tcp` implements the following:

-  TCP state machine

-  initial sequence number selection according to the system clock.

-  window-based flow control

-  Window Scale option

-  Persistence timer

-  Keepalive timer

-  Transmission policies

-  RTT measurement for retransmission timeout (RTO) computation

-  Delayed ACK algorithm

-  Nagle’s algorithm

-  Silly window avoidance

-  Timestamp option

-  Congestion control schemes: Tahoe, Reno, New Reno, Westwood, Vegas,
   etc.

-  Slow Start and Congestion Avoidance

-  Fast Retransmit and Fast Recovery

-  Loss Recovery Using Limited Transmit

-  Selective Acknowledgments (SACK)

-  SACK based loss recovery

Several protocol features can be turned on/off with parameters like
:par:`delayedAcksEnabled`, :par:`nagleEnabled`,
:par:`limitedTransmitEnabled`, :par:`increasedIWEnabled`,
:par:`sackSupport`, :par:`windowScalingSupport`, or
:par:`timestampSupport`.

The congestion control algorithm can be selected with the
:par:`tcpAlgorithmClass` parameter. For example, the following ini file
fragment selects TCP Vegas:

.. code-block:: ini

   **.tcp.tcpAlgorithmClass = "TcpVegas"

Values like ``"TcpVegas"`` name C++ classes. Indeed, :ned:`Tcp` can
be extended with new congestion control schemes by implementing and
registering them in C++.

.. _ug:sec:transport:tcplwip:

TcpLwip
~~~~~~~

*lwIP* is a light-weight implementation of the TCP/IP protocol suite
that was originally written by Adam Dunkels of the Swedish Institute of
Computer Science. The current development homepage is
http://savannah.nongnu.org/projects/lwip/.

The implementation targets embedded devices: it has very limited
resource usage (it works “with tens of kilobytes of RAM and around 40
kilobytes of ROM”), and does not require an underlying OS.

The :ned:`TcpLwip` simple module is based on the 1.3.2 version of the
lwIP sources.

Features:

-  delayed ACK

-  Nagle’s algorithm

-  round trip time estimation

-  adaptive retransmission timeout

-  SWS avoidance

-  slow start threshold

-  fast retransmit

-  fast recovery

-  persist timer

-  keep-alive timer

Limitations
^^^^^^^^^^^

-  only MSS and TS TCP options are supported. The TS option is turned
   off by default, but can be enabled by defining LWIP_TCP_TIMESTAMPS to
   1 in :file:`lwipopts.h`.

-  :var:`fork` must be ``true`` in the passive open command

-  The status request command (TCP_C_STATUS) only reports the local and
   remote addresses/ports of the connection and the MSS, SND.NXT,
   SND.WND, SND.WL1, SND.WL2, RCV.NXT, RCV.WND variables.

.. _ug:sec:transport:tcpnsc:

TcpNsc
~~~~~~

Network Simulation Cradle (NSC) is a tool that allow real-world TCP/IP
network stacks to be used in simulated networks. The NSC project is
created by Sam Jansen and available on
http://research.wand.net.nz/software/nsc.php. NSC currently contains
Linux, FreeBSD, OpenBSD and lwIP network stacks, although on 64-bit
systems only Linux implementations can be built.

To use the :ned:`TcpNsc` module you should download the
:file:`nsc-0.5.2.tar.bz2` package and follow the instructions in the
:file:`<inet_root>/3rdparty/README` file to build it.



.. warning::

   Before generating the INET module, check that the ``opp_makemake`` call
   in the make file (:file:`<inet\_root>/Makefile`) includes the
   ``-DWITH_TCP_NSC`` argument. Without this option the :ned:`TcpNsc`
   module is not built. If you build the INET library from the IDE, it is enough
   to enable the *TCP (NSC)* project feature.

Parameters
^^^^^^^^^^

The module has the following parameters:

-  :par:`stackName`: the name of the TCP implementation to be used.
   Possible values are: ``liblinux2.6.10.so``,
   ``liblinux2.6.18.so``, ``liblinux2.6.26.so``,
   ``libopenbsd3.5.so``, ``libfreebsd5.3.so`` and
   ``liblwip.so``. (On the 64 bit systems, the
   ``liblinux2.6.26.so`` and ``liblinux2.6.16.so`` are available
   only).

-  :par:`stackBufferSize`: the size of the receive and send buffer of
   one connection for selected TCP implementation. The NSC sets the
   :var:`wmem_max`, :var:`rmem_max`, :var:`tcp_rmem`, :var:`tcp_wmem`
   parameters to this value on linux TCP implementations. For details,
   you can see the NSC documentation.

.. _limitations-1:

Limitations
^^^^^^^^^^^

-  Because the kernel code is not reentrant, NSC creates a record
   containing the global variables of the stack implementation. By
   default there is room for 50 instance in this table, so you can not
   create more then 50 instance of :ned:`TcpNsc`. You can increase the
   :var:`NUM_STACKS` constant in :file:`num_stacks.h` and recompile
   NSC to overcome this limitation.

-  The :ned:`TcpNsc` module does not supprt TCP_TRANSFER_OBJECT data
   transfer mode.

-  The MTU of the network stack fixed to 1500, therefore MSS is 1460.

-  TCP_C_STATUS command reports only local/remote addresses/ports and
   current window of the connection.

.. _ug:sec:transport:udp:

UDP
---

The UDP protocol is a very simple datagram transport protocol, which
basically makes the services of the network layer available to the
applications. It performs packet multiplexing and demultiplexing to
ports and some basic error detection only.

The :ned:`Udp` simple module implements the UDP protocol. There is a
module interface (:ned:`IUdp`) that defines the gates of the :ned:`Udp`
component. In the :ned:`StandardHost` node, the UDP component can be any
module implementing that interface.

.. _ug:sec:transport:sctp:

SCTP
----

The :ned:`Sctp` module implements the Stream Control Transmission
Protocol (SCTP). Like TCP, SCTP provides reliable ordered data delivery
over an ureliable network. The most prominent feature of SCTP is the
capability of transmitting multiple streams of data at the same time
between two end points that have established a connection.

.. _ug:sec:transport:rtp:

RTP
---

The Real-time Transport Protocol (RTP) is a transport layer protocol for
delivering audio and video over IP networks. RTP is used extensively in
communication and entertainment systems that involve streaming media,
such as telephony, video teleconference applications including WebRTC,
television services and web-based push-to-talk features.

The RTP Control Protocol (RTCP) is a sister protocol of the Real-time
Transport Protocol (RTP). RTCP provides out-of-band statistics and
control information for an RTP session.

INET provides the following modules:

-  :ned:`Rtp` implements the RTP protocol

-  :ned:`Rtcp` implements the RTCP protocol

TODO some details (how to use, etc.)

TODO //////////// TCP PROTOCOL DESCRIPTION /////////////////


Topics
~~~~~~

The TCP modules of the INET framework implements the following RFCs:

+------------+----------------------------------------------------------------------------------------+
| RFC 793    | Transmission Control Protocol                                                          |
+------------+----------------------------------------------------------------------------------------+
| RFC 896    | Congestion Control in IP/TCP Internetworks                                             |
+------------+----------------------------------------------------------------------------------------+
| RFC 1122   | Requirements for Internet Hosts – Communication Layers                                 |
+------------+----------------------------------------------------------------------------------------+
| RFC 1323   | TCP Extensions for High Performance                                                    |
+------------+----------------------------------------------------------------------------------------+
| RFC 2018   | TCP Selective Acknowledgment Options                                                   |
+------------+----------------------------------------------------------------------------------------+
| RFC 2581   | TCP Congestion Control                                                                 |
+------------+----------------------------------------------------------------------------------------+
| RFC 2883   | An Extension to the Selective Acknowledgement (SACK) Option for TCP                    |
+------------+----------------------------------------------------------------------------------------+
| RFC 3042   | Enhancing TCP’s Loss Recovery Using Limited Transmit                                   |
+------------+----------------------------------------------------------------------------------------+
| RFC 3390   | Increasing TCP’s Initial Window                                                        |
+------------+----------------------------------------------------------------------------------------+
| RFC 3517   | A Conservative Selective Acknowledgment (SACK)-based Loss Recovery Algorithm for TCP   |
+------------+----------------------------------------------------------------------------------------+
| RFC 3782   | The NewReno Modification to TCP’s Fast Recovery Algorithm                              |
+------------+----------------------------------------------------------------------------------------+

In this section we describe the features of the TCP protocol specified
by these RFCs, the following sections deal with the implementation of
the TCP in the INET framework.

TCP segments
^^^^^^^^^^^^

The TCP module transmits a stream of the data over the unreliable,
datagram service that the IP layer provides. When the application writes
a chunk of data into the socket, the TCP module breaks it down to
packets and hands it over the IP. On the receiver side, it collects the
recieved packets, order them, and acknowledges the reception. The
packets that are not acknowledged in time are retransmitted by the
sender.

The TCP procotol can address each byte of the data stream by *sequence
numbers*. The sequence number is a 32-bit unsigned integer, if the end
of its range is reached, it is wrapped around.

TCP connections
^^^^^^^^^^^^^^^

When two applications are communicating via TCP, one of the applications
is the client, the other is the server. The server usually starts a
socket with a well known local port and waits until a request comes from
clients. The client applications are issue connection requests to the
port and address of the service they want to use.

After the connection is established both the client and the server can
send and receive data. When no more data is to be sent, the application
closes the socket. The application can still receive data from the other
direction. The connection is closed when both communication partner
closed its socket.

...

When opening the connection an initial sequence number is choosen and
communicated to the other TCP in the SYN segment. This sequence number
can not be a constant value (e.g. 0), because then data segments from a
previous incarnation of the connection (i.e. a connection with same
addresses and ports) could be erronously accepted in this connection.
Therefore most TCP implementation choose the initial sequence number
according to the system clock.

Flow control
^^^^^^^^^^^^

The TCP module of the receiver buffers the data of incoming segments.
This buffer has a limited capacity, so it is desirable to notify the
sender about how much data the client can accept. The sender stops the
transmission if this space exhausted.

In TCP every ACK segment holds a Window field; this is the available
space in the receiver buffer. When the sender reads the Window, it can
send at most Window unacknowledged bytes.

Window Scale option
^^^^^^^^^^^^^^^^^^^

The TCP segment contains a 16-bit field for the Window, thus allowing at
most 65535 byte windows. If the network bandwidth and latency is large,
it is surely too small. The sender should be able to send
bandwitdh\*latency bytes without receiving ACKs.

For this purpose the Window Scale (WS) option had been introduced in
RFC1323. This option specifies a scale factor used to interpret the
value of the Window field. The format is the option is:

24 & &

[WARNING: Cannot translate LaTeX bitbox to HTML!]

If the TCP want to enable window sizes greater than 65535, it should
send a WS option in the SYN segment or SYN/ACK segment (if received a
SYN with WS option). Both sides must send the option in the SYN segment
to enable window scaling, but the scale in one direction might differ
from the scale in the other direction. The :math:`shift.cnt` field is
the 2-base logarithm of the window scale of the sender. Valid values of
:math:`shift.cnt` are in the :math:`[0,14]` range.

Persistence timer
^^^^^^^^^^^^^^^^^

When the reciever buffer is full, it sends a 0 length window in the ACK
segment to stop the sender. Later if the application reads the data, it
will repeat the last ACK with an updated window to resume data sending.
If this ACK segment is lost, then the sender is not notified, so a
deadlock happens.

To avoid this situation the sender starts a Persistence Timer when it
received a 0 size window. If the timer expires before the window is
increased it send a probe segment with 1 byte of data. It will receive
the current window of the receiver in the response to this segment.

Keepalive timer
^^^^^^^^^^^^^^^

TCP keepalive timer is used to detect dead connections.

Transmission policies
^^^^^^^^^^^^^^^^^^^^^

Retransmissions
^^^^^^^^^^^^^^^

When the sender TCP sends a TCP segment it starts a retransmission
timer. If the ACK arrives before the timer expires it is stopped,
otherwise it triggers a retransmission of the segment.

If the retransmission timeout (RTO) is too high, then lost segments
causes high delays, if it is too low, then the receiver gets too many
useless duplicated segments. For optimal behaviour, the timeout must be
dynamically determined.

Jacobson suggested to measure the RTT mean and deviation and apply the
timeout:

.. math:: RTO = RTT + 4 * D

Here RTT and D are the measured smoothed roundtrip time and its smoothed
mean deviation. They are initialized to 0 and updated each time an ACK
segment received according to the following formulas:

.. math:: RTT = \alpha*RTT + (1-\alpha) * M

.. math:: D = \alpha*D + (1-\alpha)*|RTT-M|

where :math:`M` is the time between the segments send and the
acknowledgment arrival. Here the :math:`\alpha` smoothing factor is
typically :math:`7/8`.

One problem may occur when computing the round trip: if the
retransmission timer timed out and the segment is sent again, then it is
unclear that the received ACK is a response to the first transmission or
to the second one. To avoid confusing the RTT calculation, the segments
that have been retransmitted do not update the RTT. This is known as
Karn’s modification. He also suggested to double the :math:`RTO` on each
failure until the segments gets through (“exponential backoff”).

Delayed ACK algorithm
^^^^^^^^^^^^^^^^^^^^^

A host that is receiving a stream of TCP data segments can increase
efficiency in both the Internet and the hosts by sending fewer than one
ACK (acknowledgment) segment per data segment received; this is known as
a “delayed ACK” [TCP:5].

Delay is max. 500ms.

A delayed ACK gives the application an opportunity to update the window
and perhaps to send an immediate response. In particular, in the case of
character-mode remote login, a delayed ACK can reduce the number of
segments sent by the server by a factor of 3 (ACK, window update, and
echo character all combined in one segment).

In addition, on some large multi-user hosts, a delayed ACK can
substantially reduce protocol processing overhead by reducing the total
number of packets to be processed [TCP:5]. However, excessive delays on
ACK’s can disturb the round-trip timing and packet “clocking” algorithms
[TCP:7].

a TCP receiver SHOULD send an immediate ACK when the incoming segment
fills in all or part of a gap in the sequence space.

Nagle’s algorithm
^^^^^^^^^^^^^^^^^

RFC896 describes the “small packet problem": when the application sends
single-byte messages to the TCP, and it transmitted immediatly in a 41
byte TCP/IP packet (20 bytes IP header, 20 bytes TCP header, 1 byte
payload), the result is a 4000% overhead that can cause congestion in
the network.

The solution to this problem is to delay the transmission until enough
data received from the application and send all collected data in one
packet. Nagle proposed that when a TCP connection has outstanding data
that has not yet been acknowledged, small segments should not be sent
until the outstanding data is acknowledged.

Silly window avoidance
^^^^^^^^^^^^^^^^^^^^^^

The Silly Window Syndrome (SWS) is described in RFC813. It occurs when a
TCP receiver advertises a small window and the TCP sender immediately
sends data to fill the window. Let’s take the example when the sender
process writes a file into the TCP stream in big chunks, while the
receiver process reads the bytes one by one. The first few bytes are
transmitted as whole segments until the receiver buffer becomes full.
Then the application reads one byte, and a window size 1 is offered to
the sender. The sender sends a segment with 1 byte payload immediately,
the receiver buffer becomes full, and after reading 1 byte, the offered
window is 1 byte again. Thus almost the whole file is transmitted in
very small segments.

In order to avoid SWS, both sender and receiver must try to avoid this
situation. The receiver must not advertise small windows and the sender
must not send small segments when only a small window is advertised.

In RFC813 it is offered that

#. the receiver should not advertise windows that is smaller than the
   maximum segment size of the connection

#. the sender should wait until the window is large enough for a maximum
   sized segment.

Timestamp option
^^^^^^^^^^^^^^^^

Efficient retransmissions depends on precious RTT measurements. Packet
losses can reduce the precision of these measurements radically. When a
segment lost, the ACKs received in that window can not be used; thus
reducing the sample rate to one RTT data per window. This is
unacceptable if the window is large.

The proposed solution to the problem is to use a separate timestamp
field to connect the request and the response on the sender side. The
timestamp is transmitted as a TCP option. The option contains two 32-bit
timestamps:

80 & & & &

[WARNING: Cannot translate LaTeX bitbox to HTML!]

Here the TS Value (TSVal) field is the current value of the timestamp
clock of the TCP sending the option, TS Echo Reply (TSecr) field is 0 or
echoes the timestamp value of that was sent by the remote TCP. The TSscr
field is valid only in ACK segments that acknowledges new data. Both
parties should send the TS option in their SYN segment in order to allow
the TS option in data segments.

The timestamp option can also be used for PAWS (protection against
wrapped sequence numbers).

Congestion control
^^^^^^^^^^^^^^^^^^

Flow control allows the sender to slow down the transmission when the
receiver can not accept them because of memory limitations. However
there are other situations when a slow down is desirable. If the sender
transmits a lot of data into the network it can overload the processing
capacities of the network nodes, so packets are lost in the network
layer.

For this purpose another window is maintained at the sender side, the
congestion window (CWND). The congestion window is a sender-side limit
on the amount of data the sender can transmit into the network before
receiving ACK. More precisely, the sender can send at most
:math:`max(CWND, WND)` bytes above SND.UNA, therefore
:math:`SND.NXT < SND.UNA + max(CWND, WND)` is guaranteed.

The size of the congestion window is dinamically determined by
monitoring the state of the network.

Slow Start and Congestion Avoidance
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two algorithm that updates the congestion window, “Slow Start”
and “Congestion Avoidance”. They are specified in RFC2581.

:math:`cwnd \gets 2*SMSS` :math:`ssthresh \gets` upper bound of the
window (e.g. :math:`65536`) whenever ACK received if
:math:`cwnd < ssthresh` :math:`cwnd \gets cwnd + SMSS` otherwise
:math:`cwnd \gets cwnd + SMSS*SMSS/cwnd` whenever packet loss detected
:math:`cwnd \gets SMSS` :math:`ssthresh \gets max(FlightSize/2, 2*SMSS)`

Slow Start means that when the connection opened the sender initially
sends the data with a low rate. This means that the initial window (IW)
is at most 2 MSS, but no more than 2 segments. If there was no packet
loss, then the congestion window is increased rapidly, it is doubled in
each flight. When a packet loss is detected, the congestion window is
reset to 1 MSS (loss window, LW) and the “Slow Start” is applied again.



 .. note::

    RFC3390 increased the IW to roughly 4K bytes: $min(4*MSS, max(2*MSS, 4380))$.

When the congestion window reaches a certain limit (slow start
threshold), the “Congestion Avoidance” algorithm is applied. During
“Congestion Avoidance” the window is incremented by 1 MSS per
round-trip-time (RTT). This is usually implemented by updating the
window according to the :math:` cwnd += SMSS*SMSS/cwnd ` formula on
every non-duplicate ACK.

The Slow Start Threshold is updated when a packet loss is detected. It
is set to :math:`max(FlightSize/2, 2*SMSS)`.

How the sender estimates the flight size? The data sent, but not yet
acknowledged.

How the sender detect packet loss? Retransmission timer expired.

Fast Retransmit and Fast Recovery
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

RFC2581 specifies two additional methods to increase the efficiency of
congestion control: “Fast Retransmit” and “Fast Recovery”.

“Fast Retransmit” requires that the receiver signal the event, when an
out-of-order segment arrives. It is achieved by sending an immediate
duplicate ACK. The receiver also sends an immediate ACK when the
incoming segment fills in a gap or part of a gap.

When the sender receives the duplicated ACK it knows that some segment
after that sequence number is received out-of-order or that the network
duplicated the ACK. If 3 duplicated ACK received then it is more likely
that a segment was dropped or delayed. In this case the sender starts to
retransmit the segments immediately.

“Fast Recovery” means that “Slow Start” is not applied when the loss is
detected as 3 duplicate ACKs. The arrival of the duplicate ACKs
indicates that the network is not fully congested, segments after the
lost segment arrived, as well the ACKs.

Loss Recovery Using Limited Transmit
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If there is not enough data to be send after a lost segment, then the
Fast Retransmit algorithm is not activated, but the costly retranmission
timeout used.

RFC3042 suggests that the sender TCP should send a new data segment in
response to each of the first two duplicate acknowledgement.
Transmitting these segments increases the probability that TCP can
recover from a single lost segment using the fast retransmit algorithm,
rather than using a costly retransmission timeout.

Selective Acknowledgments
^^^^^^^^^^^^^^^^^^^^^^^^^

With selective acknowledgments (SACK), the data receiver can inform the
sender about all segments that have arrived successfully, so the sender
need retransmit only the segments that have actually been lost.

With the help of this information the sender can detect

-  replication by the network

-  false retransmit due to reordering

-  retransmit timeout due to ACK loss

-  early retransmit timeout

In the congestion control algorithms described so far the sender has
only rudimentary information about which segments arrived at the
receiver. On the other hand the algorithms are implemented completely on
the sender side, they only require that the client sends immediate ACKs
on duplicate segments. Therefore they can work in a heterogenous
environment, e.g. a client with Tahoe TCP can communicate with a NewReno
server. On the other hand SACK must be supported by both endpoint of the
connection to be used.

If a TCP supports SACK it includes the *SACK-Permitted* option in the
SYN/SYN-ACK segment when initiating the connection. The SACK extension
enabled for the connection if the *SACK-Permitted* option was sent and
received by both ends. The option occupies 2 octets in the TCP header:

16 &

[WARNING: Cannot translate LaTeX bitbox to HTML!]

If the SACK is enabled then the data receiver adds SACK option to the
ACK segments. The SACK option informs the sender about non-contiguous
blocks of data that have been received and queued. The meaning of the
*Acknowledgement Number* is unchanged, it is still the cumulative
sequence number. Octets received before the *Acknowledgement Number* are
kept by the receiver, and can be deleted from the sender’s buffer.
However the receiver is allowed to drop the segments that was only
reported in the SACK option.

The *SACK* option contains the following fields:

| 32 & &

[WARNING: Cannot translate LaTeX bitbox to HTML!]

Each block represents received bytes of data that are contiguous and
isolated with one exception: if a segment received that was already
ACKed (i.e. below :math:`RCV.NXT`), it is included as the first block of
the *SACK* option. The purpose is to inform the sender about a spurious
retransmission.

Each block in the option occupies 8 octets. The TCP header allows 40
bytes for options, so at most 4 blocks can be reported in the *SACK*
option (or 3 if TS option is also used). The first block is used for
reporting the most recently received data, the following blocks repeats
the most recently reported SACK blocks. This way each segment is
reported at least 3 times, so the sender receives the information even
if some ACK segment is lost.

**SACK based loss recovery**

Now lets see how the sender can use the information in the *SACK*
option. First notice that it can give a better estimation of the amount
of data outstanding in the network (called :math:`pipe` in RFC3517). If
:math:`highACK` is the highest ACKed sequence number, and
:math:`highData` of the highest sequence number transmitted, then the
bytes between :math:`highACK` and :math:`highData` can be in the
network. However :math:` pipe \neq highData - highACK ` if there are
lost and retransmitted segments:

.. math:: pipe = highData - highACK - lostBytes + retransmittedBytes

A segment is supposed to be lost if it was not received but 3 segments
recevied that comes after this segment in the sequence number space.
This condition is detected by the sender by receiving either 3
discontiguous SACKed blocks, or at least :math:`3*SMSS` SACKed bytes
above the sequence numbers of the lost segment.

The transmission of data starts with a *Slow Start* phase. If the loss
is detected by 3 duplicate ACK, the sender goes into the recovery state:
it sets :math:`cwnd` and :math:`ssthresh` to :math:`FlightSize / 2`. It
also remembers the :math:`highData` variable, because the recovery state
is left when this sequence number is acknowledged.

In the recovery state it sends data until there is space in the
congestion window (i.e. :math:`cwnd-pipe >= 1 SMSS`) The data of the
segment is choosen by the following rules (first rule that applies):

#. send segments that is lost and not yet retransmitted

#. send segments that is not yet transmitted

#. send segments that is not yet retransmitted and possibly fills a gap
   (there is SACKed data above it)

If there is no data to send, then the sender waits for the next ACK,
updates its variables based on the data of the received ACK, and then
try to transmit according to the above rules.

If an RTO occurs, the sender drops the collected SACK information and
initiates a Slow Start. This is to avoid a deadlock when the receiver
dropped a previously SACKed segment.
