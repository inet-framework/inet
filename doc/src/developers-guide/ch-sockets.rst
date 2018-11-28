.. _dg:cha:sockets:

Using Sockets
=============

.. _dg:sec:sockets:overview:

Overview
--------

The INET Socket API provides special C++ abstractions on top of the
standard OMNeT++ message passing interface for several communication
protocols.

Sockets are most often used by applications and routing protocols to
acccess the corresponding protocol services. Sockets are capable of
communicating with the underlying protocol in a bidirectional way. They
can assemble and send service requests and packets, and they can also
receive service indications and packets.

Applications can simply call the socket class member functions (e.g.
:fun:`bind()`, :fun:`connect()`, :fun:`send()`, :fun:`close()`) to
create and configure sockets, and to send and receive packets. They may
also use several different sockets simulatenously.

The following sections first introduce the shared functionality of
sockets, and then list all INET sockets in detail, mostly by shedding
light on many common usages through examples.

.. note::

   Code fragments in this chapter have been somewhat simplified for brevity. For
   example, some ``virtual`` modifiers and ``override`` qualifiers have been
   omitted, and some algorithms have been simplified to ease understanding.

Socket Interfaces
~~~~~~~~~~~~~~~~~

Although sockets are always implemented as protocol specific C++
classes, INET also provides C++ socket interfaces. These interfaces
allow writing general C++ code which can handle many different kinds of
sockets all at once.

For example, the :cpp:`ISocket` interface is implemented by all sockets,
and the :cpp:`INetworkSocket` interface is implemented by all network
protocol sockets.

Identifying Sockets
~~~~~~~~~~~~~~~~~~~

All sockets have a socket identifier which is unique within the network
node. It is automatically assigned to the sockets when they are created.
The identifier can accessed with :fun:`getSocketId()` throughout the
lifetime of the socket.

The socket identifier is also passed along in :cpp:`SocketReq` and
:cpp:`SocketInd` packet tags. These tags allow applications and
protocols to identify the socket to which :cpp:`Packet`’s, service
:cpp:`Request`’s, and service :cpp:`Indication`’s belong.

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

Since all sockets work with message passing under the hoods, they must
be configured prior to use. In order to send packets and service
requests on the correct gate towards the underlying communication
protocol, the output gate must be configured:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketConfigureExample
   :end-before: !End
   :name: Socket configure example

In contrast, incoming messages such as service indications from the
underlying communication protocol can be received on any application
gate.

To ease application development, all sockets support storing a user
specified data object pointer. The pointer is accessible with the
:fun:`setUserData()`, :fun:`getUserData()` member functions.

Another mandatory configuration for all sockets is setting the socket
callback interface. The callback interface is covered in more detail in
the following section.

Other socket specific configuration options are also available, these
are discussed in the section of the corresponding socket.

Callback Interfaces
~~~~~~~~~~~~~~~~~~~

To ease centralized message processing, all sockets provide a callback
interface which must be implemented by applications. The callback
interface is usually called :cpp:`ICallback`, and it’s defined as an
inner class of the socket it belongs to. These interfaces often contain
some generic notification methods along with several socket specific
methods.

For example, the most common callback method is the one which processes
incoming packets:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketCallbackInterfaceExample
   :end-before: !End
   :name: Socket callback interface example

Processing Messages
~~~~~~~~~~~~~~~~~~~

In general, sockets can process all incoming messages which were sent by
the underlying protocol. The received messages must be processed by the
socket where they belong to.

For example, an application can simply go through each knonwn socket in
any order, and decide which one should process the received message as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketProcessExample
   :end-before: !End
   :name: Socket process example

Sockets usually deconstruct the received messages and update their state
accordingly if necessary. They also automatically dispatch received
packets and service indications for further processing to the
appropriate functions in the corresponding :cpp:`ICallback` interface.

Sending Data
~~~~~~~~~~~~

All sockets provide one or more :fun:`send()` functions which send
packets using the current configuration of the socket. The actual means
of packet delivery depends on the underlying communication protocol, but
in general the state of the socket is expected to affect it.

For example, after the socket is properly configured, the application
can start sending packets without attaching any tags, because the socket
takes care of the necessary technical details:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketSendExample
   :end-before: !End
   :name: Socket send example

Receiving Data
~~~~~~~~~~~~~~

For example, the application may directly implement the :cpp:`ICallback`
interface of the socket and print the received data as follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketReceiveExample
   :end-before: !End
   :name: Socket receive example

Closing Sockets
~~~~~~~~~~~~~~~

Sockets must be closed before deleting them. Closing a socket allows the
underlying communication protocol to release allocated resources. These
resources are often allocated on the local network node, the remote
nework node, or potentially somewhere else in the network.

For example, a socket for a connection oriented protocol must be closed
to release the allocated resources at the peer:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketCloseExample
   :end-before: !End
   :name: Socket close example

Using Multiple Sockets
~~~~~~~~~~~~~~~~~~~~~~

If the application needs to manage a large number of sockets, for
example in a server application which handles multiple incoming
connections, the generic :cpp:`SocketMap` class may be useful. This
class can manage all kinds of sockets which implement the :cpp:`ISocket`
interface simultaneously.

For example, processing an incoming packet or service indication can be
done as follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SocketFindExample
   :end-before: !End
   :name: Socket find example

In order for the :cpp:`SocketMap` to operate properly, sockets must be
added to and removed from it using the :fun:`addSocket()` and
:fun:`removeSocket()` methods respectively.

.. _dg:sec:sockets:udp-socket:

UDP Socket
----------

The :cpp:`UdpSocket` class provides an easy to use C++ interface to send
and receive :protocol:`UDP` datagrams. The underlying :protocol:`UDP`
protocol is implemented in the :ned:`Udp` module.

Callback Interface
~~~~~~~~~~~~~~~~~~

Processing packets and indications which are received from the
:ned:`Udp` module is pretty simple. The incoming message must be
processed by the socket where it belongs as shown in the general
section.

The :cpp:`UdpSocket` deconstructs the message and uses the
:cpp:`UdpSocket::ICallback` interface to notify the application about
received data and error indications:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketCallbackInterface
   :end-before: !End
   :name: UDP socket callback interface

.. _configuring-sockets-1:

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

For receiving :protocol:`UDP` datagrams on a socket, it must be bound to
an address and a port. Both the address and port is optional. If the
address is unspecified, than all :protocol:`UDP` datagrams with any
destination address are received. If the port is -1, then an unused port
is selected automatically by the :ned:`Udp` module. The address and port
pair must be unique within the same network node.

Here is how to bind to a specific local address and port to receive
:protocol:`UDP` datagrams:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketBindExample
   :end-before: !End
   :name: UDP socket bind example

For only receiving :protocol:`UDP` datagrams from a specific remote
address/port, the socket can be connected to the desired remote
address/port:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketConnectExample
   :end-before: !End
   :name: UDP socket connect example

There are several other socket options (e.g. receiving broadcasts,
managing multicast groups, setting type of service) which can also be
configured using the :cpp:`UdpSocket` class:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketConfigureExample
   :end-before: !End
   :name: UDP socket configuration example

.. _sending-data-1:

Sending Data
~~~~~~~~~~~~

After the socket has been configured, applications can send datagrams to
a remote address and port via a simple function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketSendToExample
   :end-before: !End
   :name: UDP socket sendTo example

If the application wants to send several datagrams, it can optionally
connect to the destination.

The :protocol:`UDP` protocol is in fact connectionless, so when the
:ned:`Udp` module receives the connect request, it simply remembers the
remote address and port, and use it as default destination for later
sends.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketSendExample
   :end-before: !End
   :name: UDP socket send example

The application can call connect several times on the same socket.

.. _receiving-data-1:

Receiving Data
~~~~~~~~~~~~~~

For example, the application may directly implement the
:cpp:`UdpSocket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !UdpSocketReceiveExample
   :end-before: !End
   :name: UDP socket receive example

.. _dg:sec:sockets:tcp-socket:

TCP Socket
----------

The :cpp:`TcpSocket` class provides an easy to use C++ interface to
manage :protocol:`TCP` connections, and to send and receive data. The
underlying :protocol:`TCP` protocol is implemented in the :ned:`Tcp`,
:ned:`TcpLwip`, and :ned:`TcpNsc` modules.

.. _callback-interface-1:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the various :ned:`Tcp` modules can be processed
by the :cpp:`TcpSocket` where they belong to. The :cpp:`TcpSocket`
deconstructs the message and uses the :cpp:`TcpSocket::ICallback`
interface to notify the application about the received data or service
indication:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketCallbackInterface
   :end-before: !End
   :name: TCP socket callback interface

Configuring Connections
~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`Tcp` module supports several :protocol:`TCP` different
congestion algorithms, which can also be configured using the
:cpp:`TcpSocket`:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketConfigureExample
   :end-before: !End
   :name: TCP socket configure example

Upon setting the individual parameters, the socket immediately sends
sevice requests to the underlying :ned:`Tcp` protocol module.

Setting up Connections
~~~~~~~~~~~~~~~~~~~~~~

Since :protocol:`TCP` is a connection oriented protocol, a connection
must be established before applications can exchange data. On the one
side, the application listens at a local address and port for incoming
:protocol:`TCP` connections:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketListenExample
   :end-before: !End
   :name: TCP socket listen example

On the other side, the application connects to a remote address and port
to establish a new connection:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketConnectExample
   :end-before: !End
   :name: TCP socket connect example

Accepting Connections
~~~~~~~~~~~~~~~~~~~~~

The :ned:`Tcp` module automatically notifies the :cpp:`TcpSocket` about
incoming connections. The socket in turn notifies the application using
the :fun:`ICallback::socketAvailable` method of the callback interface.
Finally, incoming :protocol:`TCP` connections must be accepted by the
application before they can be used:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketAcceptExample
   :end-before: !End
   :name: TCP socket accept example

After the connection is accepted, the :ned:`Tcp` module notifies the
application about the socket being established and ready to be used.

.. _sending-data-2:

Sending Data
~~~~~~~~~~~~

After the connection has been established, applications can send data to
the remote application via a simple function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketSendExample
   :end-before: !End
   :name: TCP socket send example

Packet data is enqueued by the local :ned:`Tcp` module and transmitted
over time according to the protocol logic.

.. _receiving-data-2:

Receiving Data
~~~~~~~~~~~~~~

Receiving data is as simple as implementing the corresponding method of
the :cpp:`TcpSocket::ICallback` interface. One caveat is that packet
data may arrive in different chunk sizes (but the same order) than they
were sent due to the nature of :protocol:`TCP` protocol.

For example, the application may directly implement the
:cpp:`TcpSocket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TcpSocketReceiveExample
   :end-before: !End
   :name: TCP socket receive example

.. _dg:sec:sockets:sctp-socket:

SCTP Socket
-----------

The :cpp:`SctpSocket` class provides an easy to use C++ interface to
manage :protocol:`SCTP` connections, and to send and receive data. The
underlying :protocol:`SCTP` protocol is implemented in the :ned:`Sctp`
module.

.. _callback-interface-2:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the :ned:`Sctp` module can be processed by the
:cpp:`SctpSocket` where they belong to. The :cpp:`SctpSocket`
deconstructs the message and uses the :cpp:`SctpSocket::ICallback`
interface to notify the application about the received data or service
indication:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketCallbackInterface
   :end-before: !End
   :name: SCTP socket callback interface

.. _configuring-connections-1:

Configuring Connections
~~~~~~~~~~~~~~~~~~~~~~~

The :cpp:`SctpSocket` class supports setting several :protocol:`SCTP`
specific connection parameters directly:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketConfigureExample
   :end-before: !End
   :name: SCTP socket configure example

Upon setting the individual parameters, the socket immediately sends
sevice requests to the underlying :ned:`Sctp` protocol module.

.. _setting-up-connections-1:

Setting up Connections
~~~~~~~~~~~~~~~~~~~~~~

Since :protocol:`SCTP` is a connection oriented protocol, a connection
must be established before applications can exchange data. On the one
side, the application listens at a local address and port for incoming
:protocol:`SCTP` connections:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketListenExample
   :end-before: !End
   :name: SCTP socket listen example

On the other side, the application connects to a remote address and port
to establish a new connection:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketConnectExample
   :end-before: !End
   :name: SCTP socket connect example

.. _accepting-connections-1:

Accepting Connections
~~~~~~~~~~~~~~~~~~~~~

The :ned:`Sctp` module automatically notifies the :cpp:`SctpSocket`
about incoming connections. The socket in turn notifies the application
using the :fun:`ICallback::socketAvailable` method of the callback
interface. Finally, incoming :protocol:`SCTP` connections must be
accepted by the application before they can be used:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketAcceptExample
   :end-before: !End
   :name: SCTP socket accept example

.. _sending-data-3:

Sending Data
~~~~~~~~~~~~

After the connection has been established, applications can send data to
the remote applica- tion via a simple function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketSendExample
   :end-before: !End
   :name: SCTP socket send example

Packet data is enqueued by the local :ned:`Sctp` module and transmitted
over time according to the protocol logic.

.. _receiving-data-3:

Receiving Data
~~~~~~~~~~~~~~

Receiving data is as simple as implementing the corresponding method of
the :cpp:`SctpSocket::ICallback` interface. One caveat is that packet
data may arrive in different chunk sizes (but the same order) than they
were sent due to the nature of :protocol:`SCTP` protocol.

For example, the application may directly implement the
:cpp:`SctpSocket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SctpSocketReceiveExample
   :end-before: !End
   :name: SCTP socket receive example

.. _dg:sec:sockets:ipv4-socket:

IPv4 Socket
-----------

The :cpp:`Ipv4Socket` class provides an easy to use C++ interface to
send and receive :protocol:`IPv4` datagrams. The underlying
:protocol:`IPv4` protocol is implemented in the :ned:`Ipv4` module.

.. _callback-interface-3:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the :ned:`Ipv4` module must be processed by the
socket where they belong as shown in the general section. The
:cpp:`Ipv4Socket` deconstructs the message and uses the
:cpp:`Ipv4Socket::ICallback` interface to notify the application about
the received data:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketCallbackInterface
   :end-before: !End
   :name: IPv4 socket callback interface

.. _configuring-sockets-2:

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

In order to only receive :protocol:`IPv4` datagrams which are sent to a
specific local address or contain a specific protocol, the socket can be
bound to the desired local address or protocol.

For example, the following code fragment shows how the INET
:ned:`PingApp` binds to the :protocol:`ICMPv4` protocol to receive all
incoming :protocol:`ICMPv4` Echo Reply messages:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketBindExample
   :end-before: !End
   :name: IPv4 socket bind example

For only receiving :protocol:`IPv4` datagrams from a specific remote
address, the socket can be connected to the desired remote address:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketConnectExample
   :end-before: !End
   :name: IPv4 socket connect example

.. _sending-data-4:

Sending Data
~~~~~~~~~~~~

After the socket has been configured, applications can immediately start
sending :protocol:`IPv4` datagrams to a remote address via a simple
function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketSendToExample
   :end-before: !End
   :name: IPv4 socket send to example

If the application wants to send several :protocol:`IPv4` datagrams to
the same destination address, it can optionally connect to the
destination:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketSendExample
   :end-before: !End
   :name: IPv4 socket send example

The :protocol:`IPv4` protocol is in fact connectionless, so when the
:ned:`Ipv4` module receives the connect request, it simply remembers the
remote address, and uses it as the default destination address for later
sends.

The application can call :fun:`connect()` several times on the same
socket.

.. _receiving-data-4:

Receiving Data
~~~~~~~~~~~~~~

For example, the application may directly implement the
:cpp:`Ipv4Socket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv4SocketReceiveExample
   :end-before: !End
   :name: IPv4 socket receive example

.. _dg:sec:sockets:ipv6-socket:

IPv6 Socket
-----------

The :cpp:`Ipv6Socket` class provides an easy to use C++ interface to
send and receive :protocol:`IPv6` datagrams. The underlying
:protocol:`IPv6` protocol is implemented in the :ned:`Ipv6` module.

.. _callback-interface-4:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the :ned:`Ipv6` module must be processed by the
socket where they belong as shown in the general section. The
:cpp:`Ipv6Socket` deconstructs the message and uses the
:cpp:`Ipv6Socket::ICallback` interface to notify the application about
the received data:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketCallbackInterface
   :end-before: !End
   :name: IPv6 socket callback interface

.. _configuring-sockets-3:

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

In order to only receive :protocol:`IPv6` datagrams which are sent to a
specific local address or contain a specific protocol, the socket can be
bound to the desired local address or protocol.

For example, the following code fragment shows how the INET
:ned:`PingApp` binds to the :protocol:`ICMPv6` protocol to receive all
incoming :protocol:`ICMPv6` Echo Reply messages:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketBindExample
   :end-before: !End
   :name: IPv6 socket bind example

For only receiving :protocol:`IPv6` datagrams from a specific remote
address, the socket can be connected to the desired remote address:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketConnectExample
   :end-before: !End
   :name: IPv6 socket connect example

.. _sending-data-5:

Sending Data
~~~~~~~~~~~~

After the socket has been configured, applications can immediately start
sending :protocol:`IPv6` datagrams to a remote address via a simple
function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketSendAtExample
   :end-before: !End
   :name: IPv6 socket send at example

If the application wants to send several :protocol:`IPv6` datagrams to
the same destination address, it can optionally connect to the
destination:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketSendExample
   :end-before: !End
   :name: IPv6 socket send example

The :protocol:`IPv6` protocol is in fact connectionless, so when the
:ned:`Ipv6` module receives the connect request, it simply remembers the
remote address, and uses it as the default destination address for later
sends.

The application can call :fun:`connect()` several times on the same
socket.

.. _receiving-data-5:

Receiving Data
~~~~~~~~~~~~~~

For example, the application may directly implement the
:cpp:`Ipv6Socket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !Ipv6SocketReceiveExample
   :end-before: !End
   :name: IPv6 socket receive example

.. _dg:sec:sockets:l3-socket:

L3 Socket
---------

The :cpp:`L3Socket` class provides an easy to use C++ interface to send
and receive datagrams using the conceptual network protocols. The
underlying network protocols are implemented in the
:ned:`NextHopForwarding`, :ned:`Flooding`,
:ned:`ProbabilisticBroadcast`, and :ned:`AdaptiveProbabilisticBroadcast`
modules.

.. _callback-interface-5:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the network protocol module must be processed by
the associated socket where as shown in the general section. The
:cpp:`L3Socket` deconstructs the message and uses the
:cpp:`L3Socket::ICallback` interface to notify the application about the
received data:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketCallbackInterface
   :end-before: !End
   :name: L3 socket callback interface

.. _configuring-sockets-4:

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

Since the :cpp:`L3Socket` class is network protocol agnostic, it must be
configured to connect to a desired network protocol:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketProtocolExample
   :end-before: !End
   :name: L3 socket protocol example

In order to only receive datagrams which are sent to a specific local
address or contain a specific protocol, the socket can be bound to the
desired local address or protocol. The conceptual network protocols can
work with the :cpp:`ModuleIdAddress` class which contains a
:var:`moduleId` of the desired network interface.

For example, the following code fragment shows how the INET
:ned:`PingApp` binds to the :protocol:`Echo` protocol to receive all
incoming :protocol:`Echo` Reply messages:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketBindExample
   :end-before: !End
   :name: L3 socket bind example

For only receiving datagrams from a specific remote address, the socket
can be connected to the desired remote address:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketConnectExample
   :end-before: !End
   :name: L3 socket connect example

.. _sending-data-6:

Sending Data
~~~~~~~~~~~~

After the socket has been configured, applications can immediately start
sending datagrams to a remote address via a simple function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketSendToExample
   :end-before: !End
   :name: L3 socket send to example

If the application wants to send several datagrams to the same
destination address, it can optionally connect to the destination:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketSendExample
   :end-before: !End
   :name: L3 socket send example

The network protocols are in fact connectionless, so when the protocol
module receives the connect request, it simply remembers the remote
address, and uses it as the default destination address for later sends.

The application can call :fun:`connect()` several times on the same
socket.

.. _receiving-data-6:

Receiving Data
~~~~~~~~~~~~~~

For example, the application may directly implement the
:cpp:`L3Socket::ICallback` interface and print the received data as
follows:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !L3SocketReceiveExample
   :end-before: !End
   :name: L3 socket receive example

.. _dg:sec:sockets:tun-socket:

TUN Socket
----------

The :cpp:`TunSocket` class provides an easy to use C++ interface to send
and receive datagrams using a :protocol:`TUN` interface. The underlying
:protocol:`TUN` interface is implemented in the :ned:`Tun` module.

A :protocol:`TUN` interface is basically a virtual network interface
which is usually connected to an application (from the outside) instead
of other network devices. It can be used for many networking tasks such
as tunneling, or virtual private networking.

.. _callback-interface-6:

Callback Interface
~~~~~~~~~~~~~~~~~~

Messages received from the :ned:`Tun` module must be processed by the
socket where they belong as shown in the general section. The
:cpp:`TunSocket` deconstructs the message and uses the
:cpp:`TunSocket::ICallback` interface to notify the application about
the received data:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TunSocketCallbackInterface
   :end-before: !End
   :name: TUN socket callback interface

.. _configuring-sockets-5:

Configuring Sockets
~~~~~~~~~~~~~~~~~~~

A :cpp:`TunSocket` must be associated with a :protocol:`TUN` interface
before it can be used:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TunSocketOpenExample
   :end-before: !End
   :name: TUN socket open example

Sending Packets
~~~~~~~~~~~~~~~

As soon as the :cpp:`TunSocket` is associated with a :protocol:`TUN`
interface, applications can immediately start sending datagrams via a
simple function call:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TunSocketSendExample
   :end-before: !End
   :name: TUN socket send example

When the application sends a datagram to a :cpp:`TunSocket`, the packet
appears for the protocol stack within the network node as if the packet
were received from the network.

Receiving Packets
~~~~~~~~~~~~~~~~~

Messages received from the :protocol:`TUN` interface must be processed
by the corresponding :cpp:`TunSocket`. The :cpp:`TunSocket` deconstructs
the message and uses the :cpp:`TunSocket::ICallback` interface to notify
the application about the received data:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !TunSocketReceiveExample
   :end-before: !End
   :name: TUN socket receive example

When the protocol stack within the network node sends a datagram to a
:protocol:`TUN` interface, the packet appears for the application which
uses a :cpp:`TunSocket` as if the packet were sent to the network.

.. IEEE 802.2 Socket
   -----------------

.. Ethernet Socket
   ---------------

.. IEEE 802.11 Socket
   ------------------

