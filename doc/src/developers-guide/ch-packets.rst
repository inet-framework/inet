.. _dg:cha:packet-api:

Working with Packets
====================

.. _dg:sec:packets:overviews:

Overview
--------

The INET Packet API is designed to ease the implementation of
communication protocols and applications by providing many useful C++
components. In the following sections, we introduce the Packet API in
detail, and we shed light on many common API usages through examples.

.. note::

   Code fragments in this chapter have been somewhat simplified for brevity. For
   example, some ``const`` modifiers and ``const`` casts have been omitted,
   setting fields have been omitted, and some algorithms have been simplified to
   ease understanding.

The representation of packets is essential for
communication network simulation. Applications and communication
protocols construct, deconstruct, encapsulate, fragment, aggregate, and
manipulate packets in many ways. In order to ease the implementation of
these behavioral patterns, INET provides a feature-rich general data structure,
the :cpp:`Packet` class.

The :cpp:`Packet` data structure is capable of representing application
packets, :protocol:`TCP` segments, :protocol:`IP` datagrams,
:protocol:`Ethernet` frames, :protocol:`IEEE 802.11` frames, and all
kinds of digital data. It is designed to provide efficient storage,
duplication, sharing, encapsulation, aggregation, fragmentation,
serialization, and data representation selection. Additional functionality,
such as support for enqueueing data for transmisson and buffering received
data for reassembly and/or for reordering, is provided as separate C++
data structures on top of :cpp:`Packet`.

.. _dg:sec:packets:representing-data:

Representing Data
-----------------

The :cpp:`Packet` data structure builds on top of another set of data
structures called chunks. Chunks provide several alternatives to represent
a piece of data.

INET provides the following built-in chunk C++ classes:

-  :cpp:`Chunk`, the base class for all chunk classes

-  repeated byte or bit chunk (:cpp:`ByteCountChunk`,
   :cpp:`BitCountChunk`)

-  raw bytes or bits chunk (:cpp:`BytesChunk`, :cpp:`BitsChunk`)

-  ordered sequence of chunks (:cpp:`SequenceChunk`)

-  slice of another chunk designated by offset and length
   (:cpp:`SliceChunk`)

-  many protocol specific field based chunks (e.g. :cpp:`Ipv4Header`
   subclass of :cpp:`FieldsChunk`)

In addition, communication protocols and applications often define
their own chunk types. User-defined chunks are normally defined in
``msg`` files as a subclass of :cpp:`FieldsChunk`, which the
OMNeT++ MSG compiler turns into C++ code.
It is also possible to write a user defined chunk from scratch.

Chunks usually represent application data and protocol headers.
The following examples demonstrate the construction of various
chunks.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ChunkConstructionExample
   :end-before: !End
   :name: Chunk construction example

In general, chunks must be constructed with a call to the :cpp:`makeShared`
function instead of the standard C++ :cpp:`new` operator, because chunks
are shared among packets using C++ shared pointers.

Packets most often contain several chunks, inserted by different
protocols, as they are passed through the protocol layers. The most
common way to represent packet contents is to form a compound chunk by
concatenation.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ChunkConcatenationExample
   :end-before: !End
   :name: Chunk concatenation example

Protocols often need to slice data, for example to provide
fragmentation, which is also directly supported by the chunk API.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ChunkSlicingExample
   :end-before: !End
   :name: Chunk slicing example

In order to avoid cluttered data representation due to slicing, the
chunk API provides automatic merging for consecutive chunk slices.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ChunkMergingExample
   :end-before: !End
   :name: Chunk merging example

Alternative representations can be easily converted into one another
using automatic serialization as a common ground.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ChunkConversionExample
   :end-before: !End
   :name: Chunk conversion example

The following MSG fragment is a more complete example which shows how a
UDP header could be defined:

.. literalinclude:: lib/Snippets.msg
   :language: msg
   :start-after: !UdpHeaderDefinitionExample
   :end-before: !End
   :name: UDP header definition example

It’s important to distinguish the two length related fields in the
:msg:`UdpHeader` chunk. One is the length of the chunk itself
(:var:`chunkLength`), the other is the value in the length field of the
header (:var:`lengthField`).

.. _dg:sec:packets:representing-packets:

Representing Packets
--------------------

The :cpp:`Packet` data structure uses a single chunk data structure to
represent its contents. The contents may be as simple as raw bytes
(:cpp:`BytesChunk`), but most likely it will be the concatenation
(:cpp:`SequenceChunk`) of various protocol specific headers (e.g.,
:cpp:`FieldsChunk` subclasses) and application data (e.g.,
:cpp:`ByteCountChunk`).

Packets can be created by both applications and communication protocols.
As packets are passed down through the protocol layers at the sender
node, new protocol specific headers and trailers are inserted during
processing.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketConstructionExample
   :end-before: !End
   :name: Packet construction example

In order to facilitate packet processing by communication protocols at
the receiver node, :cpp:`Packet` maintains two offsets into the packet data
that divide the data into three regions: front popped
part, data part, and back popped part. During packet processing, as the
packet is passed through the protocol layers, headers and trailers are
popped from the beginning and from the end of the packet, moving the
corresponding offsets. This effectively reduces the remaining
unprocessed part called the data part, but it doesn’t affect
the data stored in the packet.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketProcessingExample
   :end-before: !End
   :name: Packet processing example

.. _dg:sec:packets:representing-signals:

Representing Signals
--------------------

Protocols and applications use the :cpp:`Packet` data structure to
represent digital data during the processing within the network node. In
contrast, the wireless transmission medium uses a different data
structure called :cpp:`Signal` to represent the physical phenomena used
to transmit packets.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !SignalConstructionExample
   :end-before: !End
   :name: Signal construction example

Signals always encapsulate a packet and also contain a description of
the analog domain representation. The most important physical properties
of a signal are the signal duration and the signal power.

.. _dg:sec:packets:representing-transmission-errors:

Representing Transmission Errors
--------------------------------

An essential part of communication network simulation is the
understanding of protocol behavior in the presence of errors. The Packet
API provides several alternatives for representing errors. The
alternatives range from simple, but computationally cheap, to accurate,
but computationally expensive solutions.

-  mark erroneous packets (simple)

-  mark erroneous chunks (good compromise)

-  change bits in raw chunks (accurate)

The first example shows how to represent transmission erros on the
packet level. A packet is marked as erroneous based on its length and
the associated bit error rate. This representation doesn’t give too much
chance for a protocol to do anything else than discard an erroneous
packet.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !CorruptingPacketsExample
   :end-before: !End
   :name: Corrupting packets example

The second example shows how to represent transmission errors on the
chunk level. Similarly to the previous example, a chunk is also marked
as erroneous based on its length and the associated bit error rate. This
representation allows a protocol to discard only certain parts of the
packet. For example, an aggregated packet may be partially discarded and
processed.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !CorruptingChunksExample
   :end-before: !End
   :name: Corrupting chunks example

The last example shows how to actually represent transmission errors on
the byte level. In contrast with the previous examples, this time the
actual data of the packet is modified. This allows a protocol to discard
or correct any part based on checksums.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !CorruptingBytesExample
   :end-before: !End
   :name: Corrupting bytes example

The physical layer models support the above mentioned different error
representations via configurable parameters. Higher layer protocols
detect errors by chechking the error bit on packets and chunks, and by
standard CRC mechanisms.

.. _dg:sec:packets:packet-tagging:

Packet Tagging
--------------

Within network nodes, supplementary data often needs to be transmitted alongside
a packet. For instance, when an application-layer module intends to transfer data
using TCP, it must specify a connection identifier for TCP. Similarly, when TCP
transmits a segment via IP, IP requires a destination address, and when IP sends
a datagram to an Ethernet interface for transmission, a destination MAC address
must be specified. These additional details are attached to a packet as tags.

The following code fragment demonstrates how packet tags could be set in the
IPv4 protocol module:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketTaggingExample
   :end-before: !End
   :name: Packet tagging example

Packet tags are not transmitted from one network node to another. All physical
layer protocols delete all packet tags from a packet before sending it to the
connected peer or to the transmission medium.

For more details on what kind of tags are there see the

.. _dg:sec:packets:region-tagging:

Region Tagging
--------------

To gather certain statistics, it might be necessary to add metadata to various
regions of packet data. For instance, determining the end-to-end delay of a TCP
stream necessitates labeling data regions at the source with their creation
timestamp. Subsequently, as the data arrives, the receiver calculates the
end-to-end delay for each region.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !RegionTaggingSendExample
   :end-before: !End
   :name: Region tagging send example

Within a TCP stream, the data may be split, rearranged, and combined in various
ways by the underlying network. The packet data representation is responsible
for preserving the associated region tags as if they were individually attached
to each bit. To prevent a cluttered data representation resulting from the
aforementioned characteristics, the tag API offers automatic merging for
successive, equivalent tag regions.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !RegionTaggingReceiveExample
   :end-before: !End
   :name: Region tagging receive example

The above loop could execute once for the entirety of the data, or it could
execute multiple times, depending on the data's creation at the sender and the
operation of the underlying network.

.. _dg:sec:packets:dissecting-packets:

Dissecting Packets
------------------

Understanding what’s inside a packet is a very important and often used
functionality. Simply using the representation may be insufficient,
because the :cpp:`Packet` may be represented with a :cpp:`BytesChunk`,
for exmple. The Packet API provides a :cpp:`PacketDissector` class which
analyzes a packet solely based on the assigned packet protocol and the
actual data it contains.

The analysis is done according to the protocol logic as opposed to the
actual representation of the data. The :cpp:`PacketDissector` works
similarly to a parser. Basically, it walks through each part (such as
protocol headers) of a packet in order. For each part, it determines the
corresponding protocol and the most specific representation for that
protocol.

The :cpp:`PacketDissector` class relies on small registered
protocol-specific dissector classes (e.g. :cpp:`Ipv4ProtocolDissector`)
subclassing the required :cpp:`ProtocolDissector` base class.
Implementors are expected to use the :cpp:`PacketDissector::ICallback`
interface to notify the parser about the packet structure.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDissectorCallbackInterface
   :end-before: !End
   :name: Packet dissector callback interface

In order to use the :cpp:`PacketDissector`, the user is expected to
implement a :cpp:`PacketDissector::ICallback` interface. The callback
interface will be notified for each part of the packet as the
:cpp:`PacketDissector` goes through it.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDissectionExample
   :end-before: !End
   :name: Packet dissection example

.. _dg:sec:packets:filtering-packets:

Filtering Packets
-----------------

Filtering packets based on the actual data they contain is another
widely used and very important feature. With the help of the packet
dissector, it is very simple to create arbitrary custom packet filters.
Packet filters are generally used for recording packets and visualizing
various packet related information.

In order to simplify filtering, the Packet API provides a generic
expression based packet filter which is implemented in the
:cpp:`PacketFilter` class. The expression syntax is the same as other
OMNeT++ expressions, and the data filter is matched against individual
chunks of the packet as found by the packet dissector.

For example, the packet filter expression "ping*" matches all packets
having the name prefix ’ping’, and the packet chunk filter expression
"inet::Ipv4Header and srcAddress(10.0.0.*)" matches all packets that
contain an :protocol:`IPv4` header with a ’10.0.0’ source address
prefix.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketFilteringExample
   :end-before: !End
   :name: Packet filtering example

.. _dg:sec:packets:printing-packets:

Printing Packets
----------------

During model development, packets often need to be displayed in a human
readable form. The Packet API provides a :cpp:`PacketPrinter` class
which is capable of forming a human readable string representation of
:cpp:`Packet`’s. The :cpp:`PacketPrinter` class relies on small
registered protocol-specific printer classes (e.g.
:cpp:`Ipv4ProtocolPrinter` subclassing the required
:cpp:`ProtocolPrinter` base class.

The packet printer is automatically used by the OMNeT++ runtime user
interface to display packets in the packet log window. The packet
printer contributes several log window columns into the user interface:
’Source’, ’Destination’, ’Protocol’, ’Length’, and ’Info’. These columns
display packet data similarly to the well-known Wireshark protocol
analyzer.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketPrintingExample
   :end-before: !End
   :name: Packet printing example

The :cpp:`PacketPrinter` provides a few other functions which have
additional options to control the details of the resulting human
readable form.

.. _dg:sec:packets:recording-pcap:

Recording PCAP
--------------

Exporting the packets from a simulation into a PCAP file allows further
processing with 3rd party tools. The Packet API provides a
:cpp:`PcapDump` class for creating PCAP files. Packet filtering can be
used to reduce the file size and increase performance.

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PCAPRecoringExample
   :end-before: !End
   :name: PCAP recording example

.. _dg:sec:packets:encapsulating-packets:

Encapsulating Packets
---------------------

Many communication protocols work with simple packet encapsulation. They
encapsulate packets with their own protocol specific headers and trailers at
the sender node, and they decapsulate packets at the reciver node. The headers
and trailers carry the information that is required to provide the protocol
specific service.

For example, the Ethernet MAC protocol encapsulates an IP datagram by prepending
the packet with an Ethernet MAC header, and also by appending the packet with an
optional padding and an Ethernet FCS. The following example shows how a MAC
protocol could encapsulate a packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketEncapsulationExample
   :end-before: !End
   :name: Packet encapsulation example

When receiving a packet, the Ethernet MAC protocol removes an Ethernet MAC header
and an Ethernet FCS from the packet, and passes the resulting IP datagram along.
The following example shows how a MAC protocol could decapsulate a packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDecapsulationExample
   :end-before: !End
   :name: Packet decapsulation example

Although the :fun:`popAtFront` and :fun:`popAtBack` functions change the remaining
unprocessed part of the packet, they don’t have effect on the actual packet data.
That is when the packet reaches high level protocol, it still contains all the
received data but the remaining unprocessed part is smaller.

.. _dg:sec:packets:fragmenting-packets:

Fragmenting Packets
-------------------

Communication protocols often provide fragmentation to overcome various
physical limits (e.g. length limit, error rate). They split packets into
smaller pieces at the sender node, which send them one-by-one. They form
the original packet at the receiver node by combining the received
fragments.

For example, the IEEE 802.11 protocol fragments packets to overcome the
increasing probability of packet loss of large packets. The following
example shows how a MAC protocol could fragment a packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketFragmentationExample
   :end-before: !End
   :name: Packet fragmentation example

When receiving fragments, protocols need to collect the coherent
fragments of the same packet until all fragments becomes available. The
following example shows how a MAC protocol could form the original
packet from a set of coherent fragments:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDefragmentationExample
   :end-before: !End
   :name: Packet defragmentation example

.. _dg:sec:packets:aggregating-packets:

Aggregating Packets
-------------------

Communication protocols often provide aggregation to better utilize the
communication channel by reducing protocol overhead. They wait for
several packets to arrive at the sender node, then they form a large
aggregated packet which is in turn sent at once. At the receiver node
the aggregated packet is split into the original packets, and they are
passed along.

For example, the IEEE 802.11 protocol aggregates packets for better
channel utilization at both MSDU and MPDU levels. The following example
shows a version of how a MAC protocol could create an aggregate packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketAggregationExample
   :end-before: !End
   :name: Packet aggregation example

The following example shows a version of how a MAC protocol could
disaggregate a packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDisaggregationExample
   :end-before: !End
   :name: Packet disaggregation example

.. _dg:sec:packets:serializing-packets:

Serializing Packets
-------------------

In real communication systems packets are usually stored as a sequence
of bytes directly in network byte order. In contrast, INET usually
stores packets in small field based C++ classes (generated by the
OMNeT++ MSG compiler) to ease debugging. In order to calculate checksums
or to communicate with real hardware, all protocol specific parts must
be serializable to a sequence of bytes.

The protocol header serializers are separate classes from the actual
protocol headers. They must be registered in the
:cpp:`ChunkSerializerRegistry` in order to be used. The following
example shows how a MAC protocol header could be serialized to a
sequence of bytes:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketSerializationExample
   :end-before: !End
   :name: Packet serialization example

Deserialization is somewhat more complicated than serialization, because
it must be prepared to handle incomplete or even incorrect data due to
errors introduced by the network. The following example shows how a MAC
protocol header could be deserialized from a sequence of bytes:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketDeserializationExample
   :end-before: !End
   :name: Packet deserialization example

.. _dg:sec:packets:emulation-support:

Emulation Support
-----------------

In order to be able to communicate with real hardware, packets must be
converted to and from a sequence of bytes. The reason is that the
programming interface of operating systems and external libraries work
with sending and receiving raw data.

All protocol headers and data chunks which are present in a packet must
have a registered serializer to be able to create the raw sequence of
bytes. Protocol modules must also be configured to either disable or
compute checksums, because serializers cannot carry out the checksum
calculation.

The following example shows how a packet could be converted to a
sequence of bytes to send through an external interface:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !EmulationPacketSendingExample
   :end-before: !End
   :name: Emulation packet sending example

The following example shows how a packet could be converted from a
sequence of bytes when receiving from an external interface:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !EmulationPacketReceivingExample
   :end-before: !End
   :name: Emulation packet receiving example

In INET, all protocols automatically support hardware emulation due to
the dual representation of packets. The above example creates a packet
which contains a single chunk with a sequence of bytes. As the packet is
passed through the protocols, they can interpret the data (e.g. by
calling :fun:`peekAtFront`) as they see fit. The Packet API always
provides the requested representation, either because it’s already
available in the packet, or because it gets automatically deserialized.

.. _dg:sec:packets:queueing-packets:

Queueing Packets
----------------

Some protocols store packet data temporarily at the sender node before
actual processing can occur. For example, the TCP protocol must store
the outgoing data received from the application in order to be able to
provide transmission flow control.

The following example shows how a transport protocol could store the
received data temporarily until the data is actually used:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketQueueingExample
   :end-before: !End
   :name: Packet queueing example

The :cpp:`ChunkQueue` class acts similarly to a binary FIFO queue except
it works with chunks. Similarly to the :cpp:`Packet` it also
automatically merge consecutive data and selects the most appropriate
representation.

.. _dg:sec:packets:buffering-packets:

Buffering Packets
-----------------

Protocols at the receiver node often need to buffer incoming packet data
until the actual processing can occur. For example, packets may arrive
out of order, and the data they contain must be reassembled or reordered
before it can be passed along.

INET provides a few special purpose C++ classes to support data
buffering:

-  :cpp:`ChunkBuffer` provides automatic merging for large data chunks
   from out of order smaller data chunks.

-  :cpp:`ReassemblyBuffer` provides reassembling for out of order data
   according to an expected length.

-  :cpp:`ReorderBuffer` provides reordering for out of order data into a
   continuous data stream from an expected offset.

All buffers deal with only the data, represented by chunks, instead of
packets. They automatically merge consecutive data and select the most
appropriate representation. Protocols using these buffers automatically
support all data representation provided by INET, and any combination
thereof. For example, :cpp:`ByteCountChunk`, :cpp:`BytesChunk`,
:cpp:`FieldsChunk`, and :cpp:`SliceChunk` can be freely mixed in the
same buffer.

.. _dg:sec:packets:reassembling-packets:

Reassembling Packets
--------------------

Some protocols may use an unreliable service to transfer a large piece
of data over the network. The unreliable service requires the receiver
node to be prepared for receiving parts out of order and potentially
duplicated.

For example, the IP protocol must store incoming fragments at the
receiver node, because it must wait until the datagram becomes complete,
before it can be passed along. The IP protocol must also be prepared for
receiving the individual fragments out of order and potentially
duplicated.

The following example shows how a network protocol could store and
reassemble the data of the incoming packets into a whole packet:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketReassemblingExample
   :end-before: !End
   :name: Packet reassembling example

The :cpp:`ReassemblyBuffer` supports replacing the stored data at a
given offset, and it also provides the complete reassembled data with
the expected length if available.

.. _dg:sec:packets:reordering-packets:

Reordering Packets
------------------

Some protocols may use an unreliable service to transfer a long data
stream over the network. The unreliable service requires the sender node
to resend unacknowledged parts, and it also requires the receiver node
to be prepared for receiving parts out of order and potentially
duplicated.

For example, the TCP protocol must buffer the incoming data at the
receiver node, because the TCP segments may arrive out of order and
potentially duplicated or overlapping, and TCP is required to provide
the data to the application in the correct order and only once.

The following example shows how a transport protocol could store and
reorder the data of incoming packets, which may arrive out of order, and
also how such a protocol could pass along only the available data in the
correct order:

.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !PacketReorderingExample
   :end-before: !End
   :name: Packet reordering example

The :cpp:`ReorderBuffer` supports replacing the stored data at a given
offset, and it provides the available data from the expected offset if
any.
