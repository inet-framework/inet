:orphan:

.. _dg:cha:tcp:

The TCP Models
==============

The TCP Module
--------------

The :ned:`Tcp` model relies on sending and receiving
:cpp:`IPControlInfo` objects attached to TCP segment objects as control
info (see :fun:`cMessage::setControlInfo()`).

The :ned:`Tcp` module manages several :cpp:`TcpConnection` object each
holding the state of one connection. The connections are identified by a
connection identifier which is choosen by the application. If the
connection is established it can also be identified by the local and
remote addresses and ports. The TCP module simply dispatches the
incoming application commands and packets to the corresponding object.

.. _subsec:tcp_packets:

TCP packets
~~~~~~~~~~~

The INET framework models the TCP header with the :msg:`TcpHeader`
message class. This contains the fields of a TCP frame, except:

-  *Data Offset*: represented by :fun:`cMessage::length()`

-  *Reserved*

-  *Checksum*: modelled by :fun:`cMessage::hasBitError()`

-  *Options*: only EOL, NOP, MSS, WS, SACK_PERMITTED, SACK and TS are
   possible

-  *Padding*

The Data field can either be represented by (see
:cpp:`TcpDataTransferMode`):

-  encapsulated C++ packet objects,

-  raw bytes as a :cpp:`ByteArray` instance,

-  its byte count only,

corresponding to transfer modes OBJECT, BYTESTREAM, BYTECOUNT resp.

TCP commands
~~~~~~~~~~~~

The application and the TCP module communicates with each other by
sending :cpp:`cMessage` objects. These messages are specified in the
:file:`TCPCommand.msg` file.

The :cpp:`TCPCommandCode` enumeration defines the message kinds that are
sent by the application to the TCP:

-  TCP_C_OPEN_ACTIVE: active open

-  TCP_C_OPEN_PASSIVE: passive open

-  TCP_C_SEND: send data

-  TCP_C_CLOSE: no more data to send

-  TCP_C_ABORT: abort connection

-  TCP_C_STATUS: request status info from TCP

Each command message should have an attached control info of type
:cpp:`TcpCommand`. Some commands (TCP_C_OPEN_xxx, TCP_C_SEND) use
subclasses. The :cpp:`TcpCommand` object has a :var:`connId` field that
identifies the connection locally within the application. :var:`connId`
is to be chosen by the application in the open command.

When the application receives a message from the TCP, the message kind
is set to one of the :cpp:`TCPStatusInd` values:

-  TCP_I_ESTABLISHED: connection established

-  TCP_I_CONNECTION_REFUSED: connection refused

-  TCP_I_CONNECTION_RESET: connection reset

-  TCP_I_TIME_OUT: connection establish timer went off, or max
   retransmission count reached

-  TCP_I_DATA: data packet

-  TCP_I_URGENT_DATA: urgent data packet

-  TCP_I_PEER_CLOSED: FIN received from remote TCP

-  TCP_I_CLOSED: connection closed normally

-  TCP_I_STATUS: status info

These messages also have an attached control info with :cpp:`TcpCommand`
or derived type (TCPConnectInfo, TCPStatusInfo, TCPErrorInfo).

TCP parameters
~~~~~~~~~~~~~~

The :ned:`Tcp` module has the following parameters:

-  :par:`advertisedWindow` in bytes, corresponds with the maximal
   receiver buffer capacity (Note: normally, NIC queues should be at
   least this size, default is 14*mss)

-  :par:`delayedAcksEnabled` delayed ACK algorithm (RFC 1122)
   enabled/disabled

-  :par:`nagleEnabled` Nagle’s algorithm (RFC 896) enabled/disabled

-  :par:`limitedTransmitEnabled` Limited Transmit algorithm (RFC 3042)
   enabled/disabled (can be used for
   TCPReno/TCPTahoe/TCPNewReno/TCPNoCongestionControl)

-  :par:`increasedIWEnabled` Increased Initial Window (RFC 3390)
   enabled/disabled

-  :par:`sackSupport` Selective Acknowledgment (RFC 2018, 2883, 3517)
   support (header option) (SACK will be enabled for a connection if
   both endpoints support it)

-  :par:`windowScalingSupport` Window Scale (RFC 1323) support (header
   option) (WS will be enabled for a connection if both endpoints
   support it)

-  :par:`timestampSupport` Timestamps (RFC 1323) support (header option)
   (TS will be enabled for a connection if both endpoints support it)

-  :par:`mss` Maximum Segment Size (RFC 793) (header option, default is
   536)

-  :par:`tcpAlgorithmClass` the name of TCP flavour

   Possible values are “TCPReno” (default), “TCPNewReno”, “TCPTahoe”,
   “TCPNoCongestionControl” and “DumpTCP”. In the future, other classes
   can be written which implement Vegas, LinuxTCP or other variants. See
   section `1.3 <#sec:tcp_algorithms>`__ for detailed description of
   implemented flavours.

   Note that TCPOpenCommand allows tcpAlgorithmClass to be chosen
   per-connection.

-  :par:`recordStats` if set to false it disables writing excessive
   amount of output vectors

TCP connections
---------------

Most part of the TCP specification is implemented in the
:cpp:`TcpConnection` class: takes care of the state machine, stores the
state variables (TCB), sends/receives SYN, FIN, RST, ACKs, etc.
TCPConnection itself implements the basic TCP “machinery”, the details
of congestion control are factored out to :cpp:`TcpAlgorithm` classes.

There are two additional objects the :cpp:`TcpConnection` relies on
internally: instances of :cpp:`TcpSendQueue` and :cpp:`TcpReceiveQueue`.
These polymorph classes manage the actual data stream, so
:cpp:`TcpConnection` itself only works with sequence number variables.
This makes it possible to easily accomodate need for various types of
simulated data transfer: real byte stream, "virtual" bytes (byte counts
only), and sequence of :cpp:`cMessage` objects (where every message
object is mapped to a TCP sequence number range).

Data transfer modes
~~~~~~~~~~~~~~~~~~~

Different applications have different needs how to represent the
messages they communicate with. Sometimes it is enough to simulate the
amount of data transmitted (“200 MB”), contents does not matter. In
other scenarios contents matters a lot. The messages can be represented
as a stream of bytes, but sometimes it is easier for the applications to
pass message objects to each other (e.g. HTTP request represented by a
:msg:`HTTPRequest` message class).

The TCP modules in the INET framework support 3 data transfer modes:

-  ``TCP_TRANSFER_BYTECOUNT``: only byte counts are represented, no
   actual payload in :msg:`TcpHeader`’s. The TCP sends as many TCP
   segments as needed

-  ``TCP_TRANSFER_BYTESTREAM``: the application can pass byte arrays
   to the TCP. The sending TCP breaks down the bytes into MSS sized
   chunks and transmits them as the payload of the TCP segments. The
   receiving application can read the chunks of the data.

-  ``TCP_TRANSFER_OBJECT``: the application pass a :cpp:`cMessage`
   object to the TCP. The sending TCP sends as many TCP segments as
   needed according to the message length. The :cpp:`cMessage` object is
   also passed as the payload of the first segment. The receiving
   application receives the object only when its last byte is received.

These values are defined in :file:`TCPCommand.msg` as the
:cpp:`TcpDataTransferMode` enumeration. The application can set the data
transfer mode per connection when the connection is opened. The client
and the server application must specify the same data transfer mode.

Opening connections
~~~~~~~~~~~~~~~~~~~

Applications can open a local port for incoming connections by sending
the TCP a TCP_C_PASSIVE_OPEN message. The attached control info (an
:cpp:`TcpOpenCommand`) contains the local address and port. The
application can specify that it wants to handle only one connection at a
time, or multiple simultanous connections. If the :var:`fork` field is
true, it emulates the Unix accept(2) semantics: a new connection
structure is created for the connection (with a new :var:`connId`), and
the connection with the old connection id remains listening. If
:var:`fork` is false, then the first connection is accepted (with the
original :var:`connId`), and further incoming connections will be
refused by the TCP by sending an RST segment. The
:var:`dataTransferMode` field in :cpp:`TcpOpenCommand` specifies whether
the application data is transmitted as C++ objects, real bytes or byte
counts only. The congestion control algorithm can also be specified on a
per connection basis by setting :var:`tcpAlgorithmClass` field to the
name of the algorithm.

The application opens a connection to a remote server by sending the TCP
a TCP_C_OPEN_ACTIVE command. The TCP creates a :cpp:`TcpConnection`
object an sends a SYN segment. The initial sequence number selected
according to the simulation time: 0 at time 0, and increased by 1 in
each 4\ :math:`\mu`\ s. If there is no response to the SYN segment, it
retry after 3s, 9s, 21s and 45s. After 75s a connection establishment
timeout (TCP_I_TIMEOUT) reported to the application and the connection
is closed.

When the connection gets established, TCP sends a TCP_I_ESTABLISHED
notification to the application. The attached control info (a
:cpp:`TcpConnectInfo` instance) will contain the local and remote
addresses and ports of the connection. If the connection is refused by
the remote peer (e.g. the port is not open), then the application
receives a TCP_I_CONNECTION_REFUSED message.



.. note::

   If you do active OPEN, then send data and close before the connection
   has reached ESTABLISHED, the connection will go from SYN\_SENT to CLOSED
   without actually sending the buffered data. This is consistent with
   RFC 793 but may not be what you would expect.



.. note::

   Handling segments with SYN+FIN bits set (esp. with data too) is
   inconsistent across TCPs, so check this one if it is of importance.

Sending Data
~~~~~~~~~~~~

The application can write data into the connection by sending a message
with TCP_C_SEND kind to the TCP. The attached control info must be of
type :cpp:`TCPSendCommand`.

The TCP will add the message to the *send queue*. There are three type
of send queues corresponding to the three data transfer mode. If the
payload is transmitted as a message object, then
:cpp:`TCPMsgBasedSendQueue`; if the payload is a byte array then
:cpp:`TCPDataStreamSendQueue`; if only the message lengths are
represented then :cpp:`TCPVirtualDataSendQueue` are the classes of send
queues. The appropriate queue is created based on the value of the
:par:`dataTransferMode` parameter of the Open command, no further
configuration is needed.

The message is handed over to the IP when there is enough room in the
windows. If Nagle’s algorithm is enabled, the TCP will collect 1 SMSS
data and sends them toghether.



.. note::

   There is no way to set the PUSH and URGENT flags, when sending data.

Receiving Data
~~~~~~~~~~~~~~

The TCP connection stores the incoming segments in the *receive queue*.
The receive queue also has three flavours: :cpp:`TCPMsgBasedRcvQueue`,
:cpp:`TCPDataStreamRcvQueue` and :cpp:`TCPVirtualDataRcvQueue`. The
queue is created when the connection is opened according to the
:var:`dataTransferMode` of the connection.

Finite receive buffer size is modeled by the :par:`advertisedWindow`
parameter. If receive buffer is exhausted (by out-of-order segments) and
the payload length of a new received segment is higher than the free
receiver buffer, the new segment will be dropped. Such drops are
recorded in *tcpRcvQueueDrops* vector.

If the *Sequence Number* of the received segment is the next expected
one, then the data is passed to the application immediately. The
:fun:`recv()` call of Unix is not modeled.

The data of the segment, which can be either a :cpp:`cMessage` object, a
:cpp:`ByteArray` object, or a simply byte count, is passed to the
application in a message that has TCP_I_DATA kind.



.. note::

   The TCP module does not handle the segments with PUSH or URGENT
   flags specially. The data of the segment passed to the application
   as soon as possible, but the application can not find out if that
   data is urgent or pushed.

RESET handling
~~~~~~~~~~~~~~

When an error occures at the TCP level, an RST segment is sent to the
communication partner and the connection is aborted. Such error can be:

-  arrival of a segment in CLOSED state

-  an incoming segment acknowledges something not yet sent.

The receiver of the RST it will abort the connection. If the connection
is not yet established, then the passive end will go back to the LISTEN
state and waits for another incoming connection instead of aborting.

Closing connections
~~~~~~~~~~~~~~~~~~~

When the application does not have more data to send, it closes the
connection by sending a TCP_C_CLOSE command to the TCP. The TCP will
transmit all data from its buffer and in the last segment sets the FIN
flag. If the FIN is not acknowledged in time it will be retransmitted
with exponential backoff.

The TCP receiving a FIN segment will notify the application that there
is no more data from the communication partner. It sends a
TCP_I_PEER_CLOSED message to the application containing the connection
identifier in the control info.

When both parties have closed the connection, the applications receive a
TCP_I_CLOSED message and the connection object is deleted. (Actually one
of the TCPs waits for :math:`2 MSL` before deleting the connection, so
it is not possible to reconnect with the same addresses and port numbers
immediately.)

Aborting connections
~~~~~~~~~~~~~~~~~~~~

The application can also abort the connection. This means that it does
not wait for incoming data, but drops the data associated with the
connection immediately. For this purpose the application sends a
TCP_C_ABORT message specifying the connection identifier in the attached
control info. The TCP will send a RST to the communication partner and
deletes the connection object. The application should not reconnect with
the same local and remote addresses and ports within MSL (maximum
segment lifetime), because segments from the old connection might be
accepted in the new one.

Status Requests
~~~~~~~~~~~~~~~

Applications can get detailed status information about an existing
connection. For this purpose they send the TCP module a TCP_C_STATUS
message attaching an :cpp:`TcpCommand` info with the identifier of the
connection. The TCP will respond with a TCP_I_STATUS message with a
:cpp:`TcpStatusInfo` attachement. This control info contains the current
state, local and remote addresses and ports, the initial sequence
numbers, windows of the receiver and sender, etc.

.. _dg:sec:tcp_algorithms:

TCP algorithms
--------------

The :cpp:`TcpAlgorithm` object controls retransmissions, congestion
control and ACK sending: delayed acks, slow start, fast retransmit, etc.
They are all extends the :cpp:`TcpAlgorithm` class. This simplifies the
design of :cpp:`TcpConnection` and makes it a lot easier to implement
TCP variations such as Tahoe, NewReno, Vegas or LinuxTCP.

Currently implemented algorithm classes are :cpp:`TcpReno`,
:cpp:`TcpTahoe`, :cpp:`TcpNewReno`, :cpp:`TcpNoCongestionControl` and
:cpp:`DumbTcp`. It is also possible to add new TCP variations by
implementing :cpp:`TcpAlgorithm`.

.. graphviz:: figures/tcp_algorithms.dot
   :align: center

The concrete TCP algorithm class to use can be chosen per connection (in
OPEN) or in a module parameter.

DumbTcp
~~~~~~~

A very-very basic :cpp:`TcpAlgorithm` implementation, with hardcoded
retransmission timeout (2 seconds) and no other sophistication. It can
be used to demonstrate what happened if there was no adaptive timeout
calculation, delayed acks, silly window avoidance, congestion control,
etc. Because this algorithm does not send duplicate ACKs when receives
out-of-order segments, it does not work well together with other
algorithms.

TcpBaseAlg
~~~~~~~~~~

The :cpp:`TcpBaseAlg` is the base class of the INET implementation of
Tahoe, Reno and New Reno. It implements basic TCP algorithms for
adaptive retransmissions, persistence timers, delayed ACKs, Nagle’s
algorithm, Increased Initial Window – EXCLUDING congestion control.
Congestion control is implemented in subclasses.

Delayed ACK
^^^^^^^^^^^

When the :par:`delayedAcksEnabled` parameter is set to ,
:cpp:`TcpBaseAlg` applies a 200ms delay before sending ACKs.

Nagle’s algorithm
^^^^^^^^^^^^^^^^^

When the :par:`nagleEnabled` parameter is , then the algorithm does not
send small segments if there is outstanding data. See also
`[subsec:trans_policies] <#subsec:trans_policies>`__.

Persistence Timer
^^^^^^^^^^^^^^^^^

The algorithm implements *Persistence Timer* (see
`[subsec:flow_control] <#subsec:flow_control>`__). When a zero-sized
window is received it starts the timer with 5s timeout. If the timer
expires before the window is increased, a 1-byte probe is sent. Further
probes are sent after 5, 6, 12, 24, 48, 60, 60, 60, ... seconds until
the window becomes positive.

Initial Congestion Window
^^^^^^^^^^^^^^^^^^^^^^^^^

Congestion window is set to 1 SMSS when the connection is established.
If the :par:`increasedIWEnabled` parameter is true, then the initial
window is increased to 4380 bytes, but at least 2 SMSS and at most 4
SMSS. The congestion window is not updated afterwards; subclasses can
add congestion control by redefining virtual methods of the
:cpp:`TcpBaseAlg` class.

Duplicate ACKs
^^^^^^^^^^^^^^

The algorithm sends a duplicate ACK when an out-of-order segment is
received or when the incoming segment fills in all or part of a gap in
the sequence space.

RTO calculation
^^^^^^^^^^^^^^^

Retransmission timeout (:math:`RTO`) is calculated according to Jacobson
algorithm (with :math:`\alpha=7/8`), and Karn’s modification is also
applied. The initial value of the :math:`RTO` is 3s, its minimum is 1s,
maximum is 240s (2 MSL).

TCPNoCongestion
~~~~~~~~~~~~~~~

TCP with no congestion control (i.e. congestion window kept very large).
Can be used to demonstrate effect of lack of congestion control.

TcpTahoe
~~~~~~~~

The :cpp:`TcpTahoe` algorithm class extends :cpp:`TcpBaseAlg` with *Slow
Start*, *Congestion Avoidance* and *Fast Retransmit* congestion control
algorithms. This algorithm initiates a *Slow Start* when a packet loss
is detected.

Slow Start
^^^^^^^^^^

The congestion window is initially set to 1 SMSS or in case of
:par:`increasedIWEnabled` is to 4380 bytes (but no less than 2 SMSS and
no more than 4 SMSS). The window is increased on each incoming ACK by 1
SMSS, so it is approximately doubled in each RTT.

Congestion Avoidance
^^^^^^^^^^^^^^^^^^^^

When the congestion window exceeded :math:`ssthresh`, the window is
increased by :math:`SMSS^2/cwnd` on each incoming ACK event, so it is
approximately increased by 1 SMSS per RTT.

Fast Retransmit
^^^^^^^^^^^^^^^

When the 3rd duplicate ACK received, a packet loss is detected and the
packet is retransmitted immediately. Simultanously the :math:`ssthresh`
variable is set to half of the :math:`cwnd` (but at least 2 SMSS) and
:math:`cwnd` is set to 1 SMSS, so it enters slow start again.

Retransmission timeouts are handled the same way: :math:`ssthresh` will
be :math:`cwnd/2`, :math:`cwnd` will be 1 SMSS.

TcpReno
~~~~~~~

The :cpp:`TcpReno` algorithm extends the behaviour :cpp:`TcpTahoe` with
*Fast Recovery*. This algorithm can also use the information transmitted
in SACK options, which enables a much more accurate congestion control.

Fast Recovery
^^^^^^^^^^^^^

When a packet loss is detected by receiveing 3 duplicate ACKs,
:math:`ssthresh` set to half of the current window as in Tahoe. However
:math:`cwnd` is set to :math:`ssthresh + 3*SMSS` so it remains in
congestion avoidance mode. Then it will send one new segment for each
incoming duplicate ACK trying to keep the pipe full of data. This
requires the congestion window to be inflated on each incoming duplicate
ACK; it will be deflated to :math:`ssthresh` when new data gets
acknowledged.

However a hard packet loss (i.e. RTO events) cause a slow start by
setting :math:`cwnd` to 1 SMSS.

SACK congestion control
^^^^^^^^^^^^^^^^^^^^^^^

This algorithm can be used with the SACK extension. Set the
:par:`sackSupport` parameter to to enable sending and receiving *SACK*
options.

TcpNewReno
~~~~~~~~~~

This class implements the TCP variant known as New Reno. New Reno
recovers more efficiently from multiple packet losses within one RTT
than Reno does.

It does not exit fast-recovery phase until all data which was
out-standing at the time it entered fast-recovery is acknowledged. Thus
avoids reducing the :math:`cwnd` multiple times.
