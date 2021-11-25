.. _mg:cha:migrationguide:

Migrating Code from INET 3.x
============================
Release: |release|

.. _mg:sec:migrationguide:architecture:

Network Node Architecture
-------------------------

The internal structure of network nodes has been changed considerably. With the
new architecture, applications can directly talk to any protocol down to the
link layer, and protocols don't have to deal with dispatching to other protocols.

The old :ned:`NodeBase` module has been split up into the following base modules:

- :ned:`NodeBase` contains mobility, status and energy related submodules
- :ned:`LinkLayerNodeBase` adds network interfaces to :ned:`NodeBase`
- :ned:`NetworkLayerNodeBase` adds network protocols to :ned:`LinkLayerNodeBase`
- :ned:`TransportLayerNodeBase` adds transport protocols to :ned:`NetworkLayerNodeBase`
- :ned:`ApplicationLayerNodeBase` adds applications to :ned:`TransportLayerNodeBase`

Protocol modules inside the network nodes are separated from each other by a
:ned:`MessageDispatcher` module. This module is responsible for dispatching packets
and commands to the intended receiver module based on various tags:

- :cpp:`SocketReq` specifies the sender socket
- :cpp:`SocketInd` specifies the receiver socket
- :cpp:`InterfaceReq` specifies the receiver interface
- :cpp:`DispatchProtocolReq` specifies the receiver protocol

The :ned:`MessageDispatcher` is also used inside network layer compound modules such
as the :ned:`Ipv4NetworkLayer`. This usage is not accidental, it solves dispatching
ARP, ICMP and IPv4 packets to the appropriate protocol modules.

.. _mg:sec:migrationguide:extendingprotocols:

Extending the Known Protocols
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Internally, protocols first must be added to the list of known protocols in the
Protocol class before they can be used. Some protocols (such as IP) have a mapping
between protocol specific integer identifiers and actual protocols. These mapping
should be created as a :cpp:`ProtocolGroup`. Here are some examples how to do this:

.. code-block:: c++

   const Protocol Protocol::ipv4("ipv4" , "IPv4);

   const ProtocolGroup ProtocolGroup::ethertype("ethertype", {
       { 0x0800, &Protocol::ipv4 },
       { 0x0806, &Protocol::arp },
       ...
   });

.. _mg:sec:migrationguide:registeringprotocols:

Registering Protocols for Dispatching
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Modules must register supported protocols with the :ned:`MessageDispatcher` to operate
properly. This is done by calling ``inet::registerProtocol(...)`` for each supported
protocol on each gate in ``initialize()``. Interfaces (usually MAC protocols modules)
must also register with calling ``inet::registerInterface(...)`` for the corresponding
:cpp:`NetworkInterface` and gate in ``initialize()``. On the other hand, sockets are learned
by the :ned:`MessageDispatcher` automatically on the fly.

.. _mg:sec:migrationguide:attachingtags:

Attaching Tags for Dispatching
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a protocol sends a packet or command to another protocol or interface, it
must attach the appropriate tag for the :ned:`MessageDispatcher`. The dispatcher uses
the attached tags to lookup the intended receiver in its registration list and
forwards the message on the appropriate gate. Here are some examples how to do
this:

.. code-block:: c++

   packet->addTag<DispatchProtocolReq>()->setProtocol(receiverProtocol); // send to protocol
   packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId); // send to interface
   packet->addTag<SocketInd>()->setSocketId(socketId); // send to socket

.. _mg:sec:migrationguide:configuringapps:

Configuring Applications
~~~~~~~~~~~~~~~~~~~~~~~~

The old application submodule vectors (``pingApp``, ``udpApp``, ``tcpApp``) have been merged
into a single application vector (``app``). The merged vector can contain all kinds
of applications, which are free to use any protocol they see fit.

This change requires updating the configuration of applications in INI files. In
the simplest case this can be done by simply replacing the application vector
names. If the example uses more than one kind of application in a single network
node then the submodule vector indexes must be also updated.

For example, the existing configuration:

.. code-block:: ini

   *.host1.numPingApps = 1
   *.host1.pingApp[0].destAddr = "host7"
   *.host1.numUdpApps = 1
   *.host1.udpApp[0].typename = "UDPSink"

The updated configuration:

.. code-block:: ini

   *.host1.numApps = 2
   *.host1.app[0].typename = "PingApp"
   *.host1.app[0].destAddr = "host7"
   *.host1.app[1].typename = "UdpSink"

.. _mg:sec:migrationguide:configuringprotocols:

Configuring Protocols
~~~~~~~~~~~~~~~~~~~~~

Transport layer and network layer protocols can be enabled/disabled with separate
boolean flags (:par:`hasTcp`, :par:`hasUdp`, :par:`hasSctp`, :par:`hasIpv4`, :par:`hasIpv6`). The number of network
interfaces can be set with separate parameters (:par:`numLoInterfaces`, :par:`numPppInterfaces`,
:par:`numEthInterfaces`, :par:`numWlanInterfaces`, :par:`numTunInterfaces`) or they
are set indirectly with the number of connected (pppg, ethg) gates.

.. _mg:sec:migrationguide:tagsapi:

Tags API
--------

Packets no longer carry control info data structures. They have a set of tags
attached instead. A tag is usually a very small data structure that focuses on
a single parameterization aspect of one or more protocols.

Some notable tag examples:

- :cpp:`SocketReq`, :cpp:`SocketInd` specifies the socket
- :cpp:`MacAddressReq`, :cpp:`MacAddressInd` specifies source and destination MAC addresses
- :cpp:`L3AddressReq`, :cpp:`L3AddressInd` specifies source and destination network addresses
- :cpp:`SignalPowerReq`, :cpp:`SignalPowerInd` specifies send and receive signal power
- :cpp:`DispatchProtocolReq`, :cpp:`DispatchProtocolInd` specifies intended receiver protocol
- :cpp:`PacketProtcolTag` specifies protocol of the packet

Tags come in three flavors:

- requests (are called ``SomethingReq``) carry information from higher layer to lower layer protocols
- indications (are called ``SomethingInd``) carry information from lower layer to higher layer protocols
- plain tags (are called ``SomethingTag``) contain some meta information
- base classes (are called ``SomethingTagBase``) must not be attached to packets

.. _mg:sec:migrationguide:controlinfo:

Splitting Control Infos
~~~~~~~~~~~~~~~~~~~~~~~

When migrating a protocol, the old control info data structures, which were
attached to packets, must be replaced with a set of tags. Implementors should
use already existing tags if possible, otherwise they are free to create new
ones as they see fit.

Any code that sets, reads or removes control info objects of packets must be
replaced with code that adds, reads or removes the appropriate tags.

Setting control info on commands need not be changed, but may be adapted for
consistency.

.. _mg:sec:migrationguide:communicating:

Communicating Through Protocol Layers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Tags can pass through protocol layers and reach far away from the originator
protocol in both the downward and upward direction. In general, tags are removed
where they are processed, usually turning into some data in a packet. Of course,
protocols are free to ignore any tag they wish based on their configuration and
state.

When a packet is reused for any purpose (e.g. forwarding, loopback interface,
echo application), most likely all tags on the packet should be removed. The
reason is that the implementor can never be sure what kind of tags are attached
to a packet, and what unintended effects those tags will have at a later stage
in some protocol.

Finally, it's important to note that tags are not transmitted from one network
node to another. All physical layer protocols are required to delete all tags
(except the :cpp:`PacketProtocolTag`) from a packet before sending it to the peer or
the medium. In other words, tags are only meant to be processed in the same
network node.

.. _mg:sec:migrationguide:determiningprotocol:

Determining the Protocol of Packets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With the new packet API, packets can no longer be differentiated using the C++
dynamic_cast operator with the desired type. The reason is that all packets are
instances of the :cpp:`Packet` class. In fact, this is quite understandable if one
views packets as a sequence of bytes. Any sequence of bytes, no matter how it
is represented by a :cpp:`Packet`, can be interpreted by any protocol, even if the
packet was not intended to be processed by that protocol. Therefore, before a
protocol is sending out a packet using any of its gates, it must attach a
:cpp:`PacketProtocolTag` to it. Here is an example how to do this:

.. code-block:: c++

   packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4); // updates tag

.. _mg:sec:migrationguide:packetapi:

Packet API
----------

INET provides a new packet API that supports efficient construction, sharing,
duplication, encapsulation, aggregation, fragmentation and serialization. The
data structure also supports dual representation by default. That is data can
be accessed as raw bytes and also as field based classes. Internally, packets
store their data in different kind of chunks.

The new API uses the following classes at the chunk level:

- :cpp:`Chunk`
- :cpp:`ByteCountChunk`, :cpp:`BytesChunk`, :cpp:`BitCountChunk`, :cpp:`BitsChunk`
- :cpp:`FieldsChunk`
- :cpp:`SliceChunk`
- :cpp:`SequenceChunk`
- :cpp:`cPacketChunk` (for backward compatibility)
- message compiler generated classes (subclassing :cpp:`FieldsChunk`)

The new API uses the following classes at the packet level:

- :cpp:`Packet`
- :cpp:`ReorderBuffer`
- :cpp:`ReassemblyBuffer`
- :cpp:`ChunkBuffer`
- :cpp:`ChunkQueue`

The new API uses the following classes for serialization:

- :cpp:`ChunkSerializer`
- one subclass of :cpp:`ChunkSerializer` for each :cpp:`Chunk` subclass listed above
- :cpp:`ChunkSerializerRegistry`

.. _mg:sec:migrationguide:headerclasses:

Protocol Header Classes
~~~~~~~~~~~~~~~~~~~~~~~

The most substantial change regarding protocols is that protocol specific headers
(or messages) are no longer subclasses of :cpp:`cPacket`. Protocol headers subclass the
:cpp:`Chunk` class instead, and they are simply added to Packets during processing.
Variable references to :cpp:`Chunk` objects must use shared pointers (``Ptr<SomeChunk>``)
types. Here is an example how to do this:

.. code-block:: c++

   auto ipv4Header = makeShared<IPv4Header>(); // creates mutable chunk
   ipv4Header->setSourceAddress(sourceAddress);
   packet->insertAtFront(ipv4Header);

   const auto& ipv4Header = packet->peekAtFront<IPv4Header>(); // return immutable chunk
   auto sourceAddress = ipv4Header->getSourceAddress();

Sometimes processing in a protocol module requires multiple utility functions
and classes. Some functions may need the packet and the protocol header at the
same time. Only passing the protocol header is not sufficient, because due to
the shared nature of chunks they don't have an owner packet. Only passing the
packet requires the called function to peek the protocol header which might
unnecessarily slow down execution. In such cases, it is a good idea to pass the
packet and the protocol header in separate parameters. Whether this is desirable
or not highly depends on the complexity of the protocol and the organization of
its implementation.

.. _mg:sec:migrationguide:immutability:

Immutability of Chunks
~~~~~~~~~~~~~~~~~~~~~~

Another important to note difficulty is that chunks can only be added to packets
if they are immutable. This requirement comes from the fact that packets support
peeking into their data regardless of how the data is represented. The result of
peek operations are required to stay consistent with the original content of the
packet. Moreover, the content of packets can be arbitrarily shared with other
packets which may be potentially present in different network nodes. Unfortunately
these properties forbid arbitrary changes once the chunk has been added to the
packet. Of course internally, packets do their best to reuse any chunk data
structure if possible.

When the need arises to change the contents of the packet such as forwarding a
packet in a network protocol, the best thing to do is the following. Remove the
part that is to be updated, create a mutable copy, update it according to the
protocol, and add the updated part back to the packet. In fact, this is like
saying that forwarding a packet is the same as sending out another packet that
shares some structure with the received one. Here is an example how to do this:

.. code-block:: c++

   auto ipv4Header = packet->removeAtFront<IPv4Header>(); // duplicate is necessary
   ipv4Header->setTimeToLive(ipv4Header->getTimeToLive() - 1); // mutable chunk
   packet->insertAtFront(ipv4Header);

.. _mg:sec:migrationguide:serializing:

Serializing Packets
~~~~~~~~~~~~~~~~~~~

The old packet serializer classes have been replaced with new classes subclassing
from the :cpp:`ChunkSerializer` class. The old serializers used to not only serialize
the packet they were responsible for but they recursed into the encapsulated packet.
This is no longer the case, serializers are only responsible for the corresponding
chunk that they handle.

Actually transforming a packet to a sequence of bytes doesn't involve directly
calling the serialization API. In fact, calling the serialization API in most
cases is not needed. For example, retrieving the whole contents of a packet as
a sequence of bytes is as simple as follows:

.. code-block:: c++

   packet->peekAt<BytesChunk>(byte(0), packet->getPacketLength()); // generic peek
   packet->peekAllBytes(); // shorthand

This property of the API greatly simplifies code that serializes packets into
trace files such as PCAP. Finally, the new API allows testing the protocol
implementations for proper emulation support. Configuring all network interfaces
to send out packets (in place of the original packets) which contain a single
:cpp:`BytesChunk` only, is easy to do. At the receiver modules, there's no need to change
anything in the protocol implementations. The reason being that the packet API
transparently handles the dual representation, and it converts the sequence of
bytes to the requested chunk types as needed.

.. _mg:sec:migrationguide:checksums:

Handling Checksums
~~~~~~~~~~~~~~~~~~

The old serializer classes used to compute and verify checksums on the fly. This
caused some confusion especially with the proper support of pseudo headers. With
the new API this is no longer the case. The new serializers are only responsible
for transforming from one representation (sequence of bytes) to another (fields),
and vice versa.

Computing and verifying checksums is up to the protocol implementations, and it
is independent of the actual representation of the header. In general, protocols
should have parameters to declare the checksum correct/incorrect or actually
compute and verify it. Of course, for emulation one should enable computing and
verifying checksums.
