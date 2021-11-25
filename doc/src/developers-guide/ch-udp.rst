:orphan:

.. _dg:cha:udp:

The UDP Model
=============

The UDP module
--------------

The state of the sockets are stored within the UDP module and the
application can configure the socket by sending command messages to the
UDP module. These command messages are distinguished by their kind and
the type of their control info. The control info identifies the socket
and holds the parameters of the command.

Applications don’t have to send messages directly to the UDP module, as
they can use the :cpp:`UdpSocket` utility class, which encapsulates the
messaging and provides a socket like interface to applications.

Sending UDP datagrams
~~~~~~~~~~~~~~~~~~~~~

If the application want to send datagrams, it optionally can connect to
the destination. It does this be sending a message with UDP_C_CONNECT
kind and :cpp:`UdpConnectCommand` control info containing the remote
address and port of the connection. The UDP protocol is in fact
connectionless, so it does not send any packets as a result of the
connect call. When the UDP module receives the connect request, it
simply remembers the destination address and port and use it as default
destination for later sends. The application can send several connect
commands to the same socket.

For sending an UDP packet, the application should attach an
:cpp:`UDPSendCommand` control info to the packet, and send it to
:ned:`Udp`. The control info may contain the destination address and
port. If the destination address or port is unspecified in the control
info then the packet is sent to the connected target.

The :ned:`Udp` module encapsulates the application’s packet into an
:msg:`UdpHeader`, creates an appropriate IP control info and send it
over ipOut or ipv6Out depending on the destination address.

The destination address can be the IPv4 local broadcast address
(255.255.255.255) or a multicast address. Before sending broadcast
messages, the socket must be configured for broadcasting. This is done
by sending an message to the UDP module. The message kind is
UDP_C_SETOPTION and its control info (an :cpp:`UdpSetBroadcastCommand`)
tells if the broadcast is enabled. You can limit the multicast to the
local network by setting the TTL of the IP packets to 1. The TTL can be
configured per socket, by sending a message to the UDP with an
:cpp:`UDPSetTimeToLive` control info containing the value. If the node
has multiple interfaces, the application can choose which is used for
multicast messages. This is also a socket option, the id of the
interface (as registered in the interface table) can be given in an
:cpp:`UdpSetMulticastInterfaceCommand` control info.



.. note::

   The :ned:`Udp` module supports only local broadcasts (using the special 255.255.255.255 address).
   Packages that are broadcasted to a remote subnet are handled as undeliverable messages.

If the UDP packet cannot be delivered because nobody listens on the
destination port, the application will receive a notification about the
failure. The notification is a message with UDP_I_ERROR kind having
attached an :cpp:`UdpErrorIndication` control info. The control info
contains the local and destination address/port, but not the original
packet.

After the application finished using a socket, it should close it by
sending a message UDP_C_CLOSE kind and :cpp:`UdpCloseCommand` control
info. The control info contains only the socket identifier. This command
frees the resources associated with the given socket, for example its
socket identifier or bound address/port.

Receiving UDP datagrams
~~~~~~~~~~~~~~~~~~~~~~~

Before receiving UDP datagrams applications should first “bind” to the
given UDP port. This can be done by sending a message with message kind
UDP_C_BIND attached with an :cpp:`UdpBindCommand` control info. The
control info contains the socket identifier and the local address and
port the application want to receive UDP packets. Both the address and
port is optional. If the address is unspecified, than the UDP packets
with any destination address is passed to the application. If the port
is -1, then an unused port is selected automatically by the UDP module.
The localAddress/localPort combination must be unique.

When a packet arrives from the network, first its error bit is checked.
Erronous messages are dropped by the UDP component. Otherwise the
application bound to the destination port is looked up, and the
decapsulated packet passed to it. If no application is bound to the
destination port, an ICMP error is sent to the source of the packet. If
the socket is connected, then only those packets are delivered to the
application, that received from the connected remote address and port.

The control info of the decapsulated packet is an
:cpp:`UDPDataIndication` and contains information about the source and
destination address/port, the TTL, and the identifier of the interface
card on which the packet was received.

The applications are bound to the unspecified local address, then they
receive any packets targeted to their port. UDP also supports multicast
and broadcast addresses; if they are used as destination address, all
nodes in the multicast group or subnet receives the packet. The socket
receives the broadcast packets only if it is configured for broadcast.
To receive multicast messages, the socket must join to the group of the
multicast address. This is done be sending the UDP module an
UDP_C_SETOPTION message with :cpp:`UdpJoinMulticastGroupsCommand`
control info. The control info specifies the multicast addresses and the
interface identifiers. If the interface identifier is given only those
multicast packets are received that arrived at that interface. The
socket can stop receiving multicast messages if it leaves the multicast
group. For this purpose the application should send the UDP another
UDP_C_SETOPTION message in their control info
(:cpp:`UdpLeaveMulticastGroupsCommand`) specifying the multicast
addresses of the groups.

Signals
~~~~~~~

The :ned:`Udp` module emits the following signals:

-  when an UDP packet sent to the IP, the packet

-  when an UDP packet received from the IP, the packet

-  when a packet passed up to the application, the packet

-  when an undeliverable UDP packet received, the packet

-  when an erronous UDP packet received, the packet
