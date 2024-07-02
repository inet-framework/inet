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

   tcp: <default("Tcp")> like ITcp if hasTcp;
   udp: <default("Udp")> like IUdp if hasUdp;
   sctp: <default("Sctp")> like ISctp if hasSctp;

As RTP is more specialized than the other ones (multimedia streaming),
INET provides a separate node type, :ned:`RtpHost`, for modeling RTP
traffic.

.. _ug:sec:transport:tcp:

TCP
---

.. _ug:sec:transport:tcp-overview:

Overview
~~~~~~~~

The TCP protocol is the most widely used protocol of the Internet. It
provides reliable, ordered delivery of a stream of bytes from one
application on one computer to another application on another computer.
The baseline TCP protocol is described in RFC793, but other tens of RFCs
contain modifications and extensions to TCP. As a result, TCP is a
complex protocol and sometimes it is hard to see how the different
requirements interact with each other.

There are three implementations of the TCP protocol in INET:

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

It is important to note that in all three implementations, connections can
be opened in either "autoread" or "explicit-read" mode, usually controlled
by a boolean parameter called :par:`autoRead`. In "autoread" mode, all
incoming data in a TCP connection is immediately sent to the application,
whereas in "explicit-read" mode, the user must send a specific read request
similar to the Unix Socket API read() call, allowing TCP to respond with
either queued data or new incoming data based on the connection.

Although read mode appears to be an implementation detail, it has fundamental
implications on the behavior of the TCP protocol. "Autoread" mode always
advertises the full buffer size, while "explicit-read" mode manages the
advertised window based on read requests. Therefore, TCP flow control is
only effective in applications that open connections in "explicit-read" mode.
Most applications use "autoread" mode, which must be taken into account
when designing simulation experiments involving TCP connections.


.. _ug:sec:transport:tcpcore:

Tcp
~~~

The :ned:`Tcp` simple module is the main implementation of the TCP
protocol in the INET framework.

:ned:`Tcp` implements the following:

-  TCP state machine

-  Selection of the initial sequence number according to the system clock.

-  Window-based flow control

-  Window Scale option

-  Persistence timer

-  Keepalive timer

-  Transmission policies

-  RTT measurement for retransmission timeout (RTO) computation

-  Delayed ACK algorithm

-  Nagle's algorithm

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

lwIP is a light-weight implementation of the TCP/IP protocol suite
originally written by Adam Dunkels of the Swedish Institute of
Computer Science. The current development homepage is
http://savannah.nongnu.org/projects/lwip/.

The implementation targets embedded devices: it has very limited
resource usage (it works “with tens of kilobytes of RAM and around 40
kilobytes of ROM”) and does not require an underlying OS.

The :ned:`TcpLwip` simple module is based on the 1.3.2 version of the
lwIP sources.

Features:

-  delayed ACK

-  Nagle's algorithm

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
   off by default but can be enabled by defining LWIP_TCP_TIMESTAMPS to
   1 in :file:`lwipopts.h`.

-  :var:`fork` must be ``true`` in the passive open command

-  The status request command (TCP_C_STATUS) only reports the local and
   remote addresses/ports of the connection and the MSS, SND.NXT,
   SND.WND, SND.WL1, SND.WL2, RCV.NXT, RCV.WND variables.

.. _ug:sec:transport:tcpnsc:

TcpNsc
~~~~~~

Network Simulation Cradle (NSC) is a tool that allows real-world TCP/IP
network stacks to be used in simulated networks. The NSC project is
created by Sam Jansen and available on
http://research.wand.net.nz/software/nsc.php. NSC currently contains
Linux, FreeBSD, OpenBSD, and lwIP network stacks. However, on 64-bit
systems, only Linux implementations can be built.

To use the :ned:`TcpNsc` module, you should download the
:file:`nsc-0.5.2.tar.bz2` package and follow the instructions in the
:file:`<inet_root>/3rdparty/README` file to build it.



.. warning::

   Before generating the INET module, check that the ``opp_makemake`` call
   in the make file (:file:`<inet\_root>/Makefile`) includes the
   ``-DWITH_TCP_NSC`` argument. Without this option, the :ned:`TcpNsc`
   module is not built. If you build the INET library from the IDE, it is enough
   to enable the *TCP (NSC)* project feature.

Parameters
^^^^^^^^^^

The module has the following parameters:

-  :par:`stackName`: the name of the TCP implementation to be used.
   Possible values are: ``liblinux2.6.10.so``,
   ``liblinux2.6.18.so``, ``liblinux2.6.26.so``,
   ``libopenbsd3.5.so``, ``libfreebsd5.3.so``, and
   ``liblwip.so``. (On the 64 bit systems, the
   ``liblinux2.6.26.so`` and ``liblinux2.6.16.so`` are available
   only).

-  :par:`stackBufferSize`: the size of the receive and send buffer of
   one connection for the selected TCP implementation. The NSC sets the
   :var:`wmem_max`, :var:`rmem_max`, :var:`tcp_rmem`, :var:`tcp_wmem`
   parameters to this value on Linux TCP implementations. For details,
   you can see the NSC documentation.

.. _limitations-1:

Limitations
^^^^^^^^^^^

-  Because the kernel code is not reentrant, NSC creates a record
   containing the global variables of the stack implementation. By
   default, there is room for 50 instances in this table, so you cannot
   create more than 50 instances of :ned:`TcpNsc`. You can increase the
   :var:`NUM_STACKS` constant in :file:`num_stacks.h` and recompile
   NSC to overcome this limitation.

-  The :ned:`TcpNsc` module does not support TCP_TRANSFER_OBJECT data
   transfer mode.

-  The MTU of the network stack fixed to 1500, therefore MSS is 1460.

-  TCP_C_STATUS command reports only local/remote addresses/ports and
   the current window of the connection.

.. _ug:sec:transport:udp:

UDP
---

The UDP protocol is a very simple datagram transport protocol, which
basically provides the services of the network layer to the
applications. It performs packet multiplexing and demultiplexing to
ports and performs basic error detection only.

The :ned:`Udp` simple module implements the UDP protocol. There is a
module interface (:ned:`IUdp`) that defines the gates of the :ned:`Udp`
component. In the :ned:`StandardHost` node, the UDP component can be any
module implementing that interface.

.. _ug:sec:transport:sctp:

SCTP
----

The :ned:`Sctp` module implements the Stream Control Transmission
Protocol (SCTP). Like TCP, SCTP provides reliable ordered data delivery
over an unreliable network. The most prominent feature of SCTP is the
capability of transmitting multiple streams of data at the same time
between two end points that have established a connection.

.. _ug:sec:transport:rtp:

RTP
---

The Real-time Transport Protocol (RTP) is a transport layer protocol for
delivering audio and video over IP networks. RTP is used extensively in
communication and entertainment systems that involve streaming media,
such as telephony, video teleconference applications including WebRTC,
television services, and web-based push-to-talk features.

The RTP Control Protocol (RTCP) is a sister protocol of the Real-time
Transport Protocol (RTP). RTCP provides out-of-band statistics and
control information for an RTP session.

INET provides the following modules:

-  :ned:`Rtp` implements the RTP protocol

-  :ned:`Rtcp` implements the RTCP protocol