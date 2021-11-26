.. _ug:cha:apps:

Applications
============

.. _ug:sec:apps:overview:

Overview
--------

This chapter describes application models and traffic generators. All applications
implement the :ned:`IApp` module interface to ease configuration. For example,
:ned:`StandardHost` contains an application submodule vector that can be filled
in with specific applications from the INI file.

INET applications fall into two categories. In the first category, applications
implement very specific behaviors, and generate corresponding traffic patterns
based on their specific parameters. These applications are implemented as simple
modules.

In the second category, applications are more generic. They separate traffic
generation from the usage of the protocol, :ned:`Udp` or :ned:`Tcp` for example.
These applications are implemented as compound modules. They contain separate
configurable traffic source, traffic sink, and protocol input/output submodules.
This approach allows building complex traffic patterns by composing queueing
model elements.

.. _ug:sec:apps:tcp-applications:

TCP applications
----------------

This sections describes the applications using the TCP protocol. These
applications use :msg:`GenericAppMsg` objects to represent the data sent
between the client and server. The client message contains the expected
reply length, the processing delay, and a flag indicating that the
connection should be closed after sending the reply. This way
intelligence (behaviour specific to the modelled application, e.g. HTTP,
SMB, database protocol) needs only to be present in the client, and the
server model can be kept simple and dumb.

.. _ug:sec:apps:tcpbasicclientapp:

TcpBasicClientApp
~~~~~~~~~~~~~~~~~

Client for a generic request-response style protocol over TCP. May be
used as a rough model of HTTP or FTP users.

The model communicates with the server in sessions. During a session,
the client opens a single TCP connection to the server, sends several
requests (always waiting for the complete reply to arrive before sending
a new request), and closes the connection.

The server app should be :ned:`TcpGenericServerApp`; the model sends
:msg:`GenericAppMsg` messages.

Example settings:

FTP:



.. code-block:: ini

   numRequestsPerSession = exponential(3)
   requestLength = truncnormal(20,5)
   replyLength = exponential(1000000)

HTTP:



.. code-block:: ini

   numRequestsPerSession = 1 # HTTP 1.0
   numRequestsPerSession = exponential(5)  # HTTP 1.1, with keepalive
   requestLength = truncnormal(350,20)
   replyLength = exponential(2000)

Note that since most web pages contain images and may contain frames,
applets etc, possibly from various servers, and browsers usually
download these items in parallel to the main HTML document, this module
cannot serve as a realistic web client.

Also, with HTTP 1.0 it is the server that closes the connection after
sending the response, while in this model it is the client.

.. _ug:sec:apps:tcpsinkapp:

TcpSinkApp
~~~~~~~~~~

Accepts any number of incoming TCP connections, and discards whatever
arrives on them.

.. _ug:sec:apps:tcpgenericserverapp:

TcpGenericServerApp
~~~~~~~~~~~~~~~~~~~

Generic server application for modelling TCP-based request-reply style
protocols or applications.

The module accepts any number of incoming TCP connections, and expects
to receive messages of class :msg:`GenericAppMsg` on them. A message
should contain how large the reply should be (number of bytes).
:ned:`TcpGenericServerApp` will just change the length of the received
message accordingly, and send back the same message object. The reply
can be delayed by a constant time (:par:`replyDelay` parameter).

.. _ug:sec:apps:tcpechoapp:

TcpEchoApp
~~~~~~~~~~

The :ned:`TcpEchoApp` application accepts any number of incoming TCP
connections, and sends back the data that arrives on them, The byte
counts are multiplied by :par:`echoFactor` before echoing. The reply can
also be delayed by a constant time (:par:`echoDelay` parameter).

.. _ug:sec:apps:tcpsessionapp:

TcpSessionApp
~~~~~~~~~~~~~

Single-connection TCP application: it opens a connection, sends the
given number of bytes, and closes. Sending may be one-off, or may be
controlled by a “script” which is a series of (time, number of bytes)
pairs. May act either as client or as server. Compatible with both IPv4
and IPv6.

Opening the connection
^^^^^^^^^^^^^^^^^^^^^^

Depending on the type of opening the connection (active/passive), the
application may be either a client or a server. In passive mode, the
application will listen on the given local local port, and wait for an
incoming connection. In active mode, the application will bind to given
local local address and local port, and connect to the given address and
port. It is possible to use an ephemeral port as local port.

Even when in server mode (passive open), the application will only serve
one incoming connection. Further connect attempts will be refused by TCP
(it will send RST) for lack of LISTENing connections.

The time of opening the connection is in the :par:`tOpen` parameter.

Sending data
^^^^^^^^^^^^

Regardless of the type of OPEN, the application can be made to send
data. One way of specifying sending is via the :par:`tSend`,
:par:`sendBytes` parameters, the other way is :par:`sendScript`. With
the former, :par:`sendBytes` bytes will be sent at :par:`tSend`. When
using :par:`sendScript`, the format of the script is:



::

   <time> <numBytes>; <time> <numBytes>;...

Closing the connection
^^^^^^^^^^^^^^^^^^^^^^

The application will issue a TCP CLOSE at time :par:`tClose`. If
:par:`tClose=-1`, no CLOSE will be issued.

.. _ug:sec:apps:telnetapp:

TelnetApp
~~~~~~~~~

Models Telnet sessions with a specific user behaviour. The server app
should be :ned:`TcpGenericServerApp`.

In this model the client repeats the following activity between
:par:`startTime` and :par:`stopTime`:

#. Opens a telnet connection

#. Sends :par:`numCommands` commands. The command is
   :par:`commandLength` bytes long. The command is transmitted as
   entered by the user character by character, there is
   :par:`keyPressDelay` time between the characters. The server echoes
   each character. When the last character of the command is sent (new
   line), the server responds with a :par:`commandOutputLength` bytes
   long message. The user waits :par:`thinkTime` interval between the
   commands.

#. Closes the connection and waits :par:`idleInterval` seconds

#. If the connection is broken, it is noticed after
   :par:`reconnectInterval` and the connection is reopened

Each parameter in the above description is “volatile”, so you can use
distributions to emulate random behaviour.



.. note::

   This module emulates a very specific user behaviour, and as such,
   it should be viewed as an example rather than a generic Telnet model.
   If you want to model realistic Telnet traffic, you are encouraged
   to gather statistics from packet traces on a real network, and
   write your model accordingly.

.. _ug:sec:apps:tcpserverhostapp:

TcpServerHostApp
~~~~~~~~~~~~~~~~

This module hosts TCP-based server applications. It dynamically creates
and launches a new “thread” object for each incoming connection.

Server threads can be implemented in C++. An example server thread class
is :cpp:`TcpGenericServerThread`.

Applications composing TCP traffic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following TCP modules are provided to allow composing applications with more
complex traffic without implementing new C++ modules:

-  :ned:`TcpClientApp`: generic TCP client application with composable traffic source and traffic sink
-  :ned:`TcpServerApp`: generic TCP server application with a TCP server listener to create TCP server connections
-  :ned:`TcpServerConnection`: generic TCP server connection with composable traffic source and traffic sink
-  :ned:`TcpServerListener`: generic TCP server listener for accepting/rejecting TCP connections and for creating TCP server connections
-  :ned:`TcpRequestResponseApp`: generic request-response based TCP server application with configurable pre-composed traffic source and traffic sink

There are some applications which model the traffic of the telnet protocol:

-  :ned:`TelnetClientApp`: telnet client application with configurable pre-composed telnet traffic source and traffic sink
-  :ned:`TelnetServerApp`: telnet server application with pre-configured TCP server listener to create telnet server connections
-  :ned:`TelnetServerConnection`: telnet server connection with configurable pre-composed telnet traffic source and traffic sink

.. _ug:sec:apps:udp-applications:

UDP applications
----------------

The following UDP-based applications are implemented in INET:

-  :ned:`UdpBasicApp` sends UDP packets to a given IP address at a given
   interval

-  :ned:`UdpBasicBurst` sends UDP packets to the given IP address(es) in
   bursts, or acts as a packet sink.

-  :ned:`UdpEchoApp` is similar to :ned:`UdpBasicApp`, but it sends back
   the packet after reception

-  :ned:`UdpSink` consumes and prints packets received from the
   :ned:`Udp` module

-  :ned:`UdpVideoStreamClient`,:ned:`UdpVideoStreamServer` simulates
   video streaming over UDP

The next sections describe these applications in details.

.. _ug:sec:apps:udpbasicapp:

UdpBasicApp
~~~~~~~~~~~

The :ned:`UdpBasicApp` sends UDP packets to a the IP addresses given in
the :par:`destAddresses` parameter. The application sends a message to
one of the targets in each :par:`sendInterval` interval. The interval
between message and the message length can be given as a random
variable. Before the packet is sent, it is emitted in the signal.

The application simply prints the received UDP datagrams. The signal can
be used to detect the received packets.

.. _ug:sec:apps:udpsink:

UdpSink
~~~~~~~

This module binds an UDP socket to a given local port, and prints the
source and destination and the length of each received packet.

.. _ug:sec:apps:udpechoapp:

UdpEchoApp
~~~~~~~~~~

Similar to :ned:`UdpBasicApp`, but it sends back the packet after
reception. It accepts only packets with :msg:`UdpHeader`, i.e.
packets that are generated by another :ned:`UdpEchoApp`.

When an echo response received, it emits an signal.

.. _ug:sec:apps:udpvideostreamclient:

UdpVideoStreamClient
~~~~~~~~~~~~~~~~~~~~

This module is a video streaming client. It send one “video streaming
request” to the server at time :par:`startTime` and receives stream from
:ned:`UdpVideoStreamServer`.

The received packets are emitted by the signal.

.. _ug:sec:apps:udpvideostreamserver:

UdpVideoStreamServer
~~~~~~~~~~~~~~~~~~~~

This is the video stream server to be used with
:ned:`UdpVideoStreamClient`.

The server will wait for incoming "video streaming requests". When a
request arrives, it draws a random video stream size using the
:par:`videoSize` parameter, and starts streaming to the client. During
streaming, it will send UDP packets of size :par:`packetLen` at every
:par:`sendInterval`, until :par:`videoSize` is reached. The parameters
:par:`packetLen` and :par:`sendInterval` can be set to constant values
to create CBR traffic, or to random values (e.g.
``sendInterval=uniform(1e-6, 1.01e-6)``) to accomodate jitter.

The server can serve several clients, and several streams per client.

.. _ug:sec:apps:udpbasicburst:

UdpBasicBurst
~~~~~~~~~~~~~

Sends UDP packets to the given IP address(es) in bursts, or acts as a
packet sink. Compatible with both IPv4 and IPv6.

Addressing
^^^^^^^^^^

The :par:`destAddresses` parameter can contain zero, one or more
destination addresses, separated by spaces. If there is no destination
address given, the module will act as packet sink. If there are more
than one addresses, one of them is randomly chosen, either for the whole
simulation run, or for each burst, or for each packet, depending on the
value of the :par:`chooseDestAddrMode` parameter. The :par:`destAddrRNG`
parameter controls which (local) RNG is used for randomized address
selection. The own addresses will be ignored.

An address may be given in the dotted decimal notation, or with the
module name. (The :cpp:`L3AddressResolver` class is used to resolve the
address.) You can use the "Broadcast" string as address for sending
broadcast messages.

INET also defines several NED functions that can be useful:

-  | ``moduleListByPath("pattern",...)``:
   | Returns a space-separated list of the modulenames. All modules
     whose full path matches one of the pattern parameters will be
     included. The patterns may contain wilcards in the same syntax as
     in ini files. Example:

-  | ``moduleListByNedType("fully.qualified.ned.type",...)``:
   | Returns a space-separated list of the modulenames with the given
     NED type(s). All modules whose NED type name occurs in the
     parameter list will be included. The NED type name is fully
     qualified. Example:

Examples:



.. code-block:: ini

   **.app[0].destAddresses = moduleListByPath("**.host[*]", "**.fixhost[*]")
   **.app[1].destAddresses = moduleListByNedType("inet.nodes.inet.StandardHost")

The peer can be UDPSink or another UDPBasicBurst.

Bursts
^^^^^^

The first burst starts at :par:`startTime`. Bursts start by immediately
sending a packet; subsequent packets are sent at :par:`sendInterval`
intervals. The :par:`sendInterval` parameter can be a random value, e.g.
``exponential(10ms)``. A constant interval with jitter can be
specified as ``1s+uniform(-0.01s,0.01s)`` or
``uniform(0.99s,1.01s)``. The length of the burst is controlled by
the :par:`burstDuration` parameter. (Note that if :par:`sendInterval` is
greater than :par:`burstDuration`, the burst will consist of one packet
only.) The time between burst is the :par:`sleepDuration` parameter;
this can be zero (zero is not allowed for :par:`sendInterval`.) The zero
:par:`burstDuration` is interpreted as infinity.

Operation as sink
^^^^^^^^^^^^^^^^^

When :par:`destAddresses` parameter is empty, the module receives
packets and makes statistics only.

Applications composing UDP traffic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following UDP modules are provided to allow composing applications with more
complex traffic without implementing new C++ modules:

-  :ned:`UdpApp`: generic UDP application with composable traffic source and traffic sink
-  :ned:`UdpSourceApp`: generic UDP application with a traffic source
-  :ned:`UdpSinkApp`: generic UDP application with a traffic sink
-  :ned:`UdpRequestResponseApp`: generic request-response based UDP server application with configurable pre-composed traffic source and traffic sink

.. -  :ned:`UdpClientApp`: generic UDP client application with composable traffic source and traffic sink
   -  :ned:`UdpServerApp`: generic UDP server application with a UDP session handler to create UDP server sessions
   -  :ned:`UdpServerSession`: generic UDP server session with composable traffic source and traffic sink

.. _ug:sec:apps:ipv4/ipv6-traffic-generators:

IPv4/IPv6 traffic generators
----------------------------

The applications described in this section use the services of the
network layer only, they do not need transport layer protocols. They can
be used with both IPv4 and IPv6.

:ned:`IIpvxTrafficGenerator` (prototype) sends IP or IPv6 datagrams to
the given address at the given :par:`sendInterval`. The
:par:`sendInterval` parameter can be a constant or a random value (e.g.
``exponential(1)``). If the :par:`destAddresses` parameter contains
more than one address, one of them is randomly for each packet. An
address may be given in the dotted decimal notation (or, for IPv6, in
the usual notation with colons), or with the module name. (The
:cpp:`L3AddressResolver` class is used to resolve the address.) To
disable the model, set :par:`destAddresses` to "".

The :ned:`IpvxTrafGen` sends messages with length :par:`packetLength`.
The sent packet is emitted in the signal. The length of the sent packets
can be recorded as scalars and vectors.

The :ned:`IpvxTrafSink` can be used as a receiver of the packets
generated by the traffic generator. This module emits the packet in the
signal and drops it. The ``rcvdPkBytes`` and ``endToEndDelay``
statistics are generated from this signal.

The :ned:`IpvxTrafGen` can also be the peer of the traffic generators;
it handles the received packets exactly like :ned:`IpvxTrafSink`.

.. _ug:sec:apps:the-pingapp-application:

The PingApp application
-----------------------

The :ned:`PingApp` application generates ping requests and calculates
the packet loss and round trip parameters of the replies.

Start/stop time, sendInterval etc. can be specified via parameters. An
address may be given in the dotted decimal notation (or, for IPv6, in
the usual notation with colons), or with the module name. (The
:cpp:`L3AddressResolver` class is used to resolve the address.) To
disable send, specify empty destAddr.

Every ping request is sent out with a sequence number, and replies are
expected to arrive in the same order. Whenever there’s a jump in the in
the received ping responses’ sequence number (e.g. 1, 2, 3, 5), then the
missing pings (number 4 in this example) is counted as lost. Then if it
still arrives later (that is, a reply with a sequence number smaller
than the largest one received so far) it will be counted as
out-of-sequence arrival, and at the same time the number of losses is
decremented. (It is assumed that the packet arrived was counted earlier
as a loss, which is true if there are no duplicate packets.)


.. _ug:sec:apps:ethernet-applications:

Ethernet applications
---------------------

The ``inet.applications.ethernet`` package contains modules for a
simple client-server application. The :ned:`EtherAppClient` is a simple
traffic generator that peridically sends :msg:`EtherAppReq` messages
whose length can be configured. destAddress, startTime,waitType,
reqLength, respLength

The server component of the model (:ned:`EtherAppServer`) responds with
a :msg:`EtherAppResp` message of the requested length. If the response
does not fit into one ethernet frame, the client receives the data in
multiple chunks.

Both applications have a :par:`registerSAP` boolean parameter. This
parameter should be set to ``true`` if the application is connected
to the :ned:`Ieee8022Llc` module which requires registration of the SAP
before sending frames.

Both applications collects the following statistics: sentPkBytes,
rcvdPkBytes, endToEndDelay.

The client and server application works with any model that accepts
Ieee802Ctrl control info on the packets (e.g. the 802.11 model). The
applications should be connected directly to the :ned:`Ieee8022Llc` or an
EthernetInterface NIC module.

The model also contains a host component that groups the applications
and the LLC and MAC components together (:ned:`EthernetHost`). This node
does not contain higher layer protocols, it generates Ethernet traffic
directly. By default it is configured to use half duplex MAC (CSMA/CD).
