.. _mg:cha:migrationguide:

Migrating Code from INET 3.x
============================
Release: |release|

IEEE 802.11 TXOP and rate selection
----------------------------------

Management modules accept ``basicRates`` and ``operationalRates`` as lists
with units, for example ``"6Mbps 12Mbps"``. ``"auto"`` retains the automatic
policy. An empty string means a known empty set. Custom association frames
must advertise support for every BSS basic rate. The AP rejects incomplete
advertisements instead of silently assuming PHY support.

The MIB stores accepted rate sets. Detailed management learns peer rates from
accepted frames. Simplified stations use the AP's BSS policy and retain their
separate local rate limits.

Custom MAC implementations must implement ``IIeee80211ModeSetProvider``.
Management queries this read-only catalog contract after link-layer initialization.
The built-in ``Ieee80211Mac`` implements it through ``getModeSet()``.
Management now starts at ``INITSTAGE_NETWORK_CONFIGURATION`` so the MAC mode
set exists first. This moves beacon and association startup to a later stage.
The different order of random draws can change results outside the TXOP examples.

``IQosRateSelection::computeMode`` takes explicit ``startsTxop`` and
``previousModeForReceiver`` arguments. Its optional ``useFastestMode`` flag
requests the highest eligible rate for a permitted TXOP overrun.
Explicit configured rates retain precedence. Selectors no longer own holder
transmission history. Custom PHY modes must implement the non-HT reference
rate, modulation-class, and legacy-preamble queries.

The fastest-mode retry does not override a configured QoS rate during an overrun.
IEEE 802.11 recommends a high rate for this case; it does not require a fixed-rate override.

Custom ACK and RTS policies must provide the explicit response-mode timeout
queries. ``IFrameSequence`` steps expose prepared mode, length, interval,
and PPDU duration. Custom transmit callbacks must implement
``transmissionStarting``; false cancels the pending packet before transmission.
Custom contention implementations must support cancellation.

``SingleProtectionMechanism`` no longer has a rate-selection module parameter.
It reads the prepared plan. The former TXOP boundary stubs and selector
``frameTransmitted`` history hooks are removed. Use the context's active plan
and actual transmission history. Cancellation requires a sequence outcome.
Prepared ``ReceiveStep`` construction takes the response values directly;
it does not retain a pointer to the transmit step.

HCF stop retains packets whose normal ACK or Block Ack wait was interrupted.
Restart retries these packets with the Retry bit set. Administrative stop does
not consume a retry attempt or report a failed transmission.

HCF also retains Block Ack agreements and their absolute inactivity deadlines.
Restart restores the earliest finite deadline across originator and recipient
agreements. A deadline that expires during downtime triggers expiry at restart.
Expiry removes the local agreement before HCF queues its timeout DELBA.
The modeled stop and crash operations share this retention policy.

Block Ack handlers retain pending DELBA transactions across stop and restart.
Replacement ADDBA setup waits until the old DELBA receives an acknowledgment
or reaches a terminal drop. Individual transmission attempts do not remove
an agreement. Custom originator and recipient handlers must implement
``processDelbaFrameFinished()``. HCF calls it for final acknowledgment,
queue drop, or retry exhaustion. The existing management transaction tag
identifies the local teardown across packet copies. Late terminal callbacks
must not complete another teardown transaction.

Custom Block Ack agreement handlers must implement
``computeEarliestExpirationTime()``. It returns an absolute deadline, or
``SIMTIME_MAX`` when no finite deadline exists. The callback
``IBlockAckAgreementHandlerCallback::scheduleInactivityTimer()`` now takes no
argument. It requests a timer refresh from both agreement owners.

ADDBA response completion now only requests this timer refresh. A queued or
retried response can complete after its agreement expires or a new agreement
replaces it. The completion does not create or change an agreement.
``RecipientBlockAckAgreementHandler::updateAgreement()`` is removed without
a deprecation interval because its lookup can fail after expiry or select a
replacement agreement. Its only state update had no consumer.
``RecipientBlockAckAgreement::addbaResposneSent()`` and the protected
``isAddbaResponseSent`` flag are also removed. Custom handlers must use
``processTransmittedAddbaResp()`` for completion callbacks.

ADDBA request completion also leaves agreements unchanged. A request can still
retry after agreement removal. ``OriginatorBlockAckAgreement`` no longer exposes
``getIsAddbaRequestSent()``, ``setIsAddbaRequestSent()``, or the protected
``isAddbaRequestSent`` flag. This state also had no consumer. Its completion
update is unsafe for removed or replacement agreements, so these symbols are
removed without a deprecation interval. The
``OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq()`` callback
remains available for custom handlers.

Response-rate overrides must match the primary response mode. A conflicting
override now fails explicitly. Only DCF ``RateSelection`` provides
``useNonstandardResponseModes`` to permit nonstandard response overrides.
HCF ``QosRateSelection`` has no such option. For example, a conflicting
``**.hcf.rateSelection.responseAckFrameBitrate`` fails when the selector first
uses it. Remove conflicting overrides from QoS configurations.
Response selection uses the BSS basic rates or the applicable mandatory rates,
without the local operational-rate restriction used for data transmission.

Explicit DCF experimental ACK/CTS overrides also bypass known peer-rate restrictions.
Configure compatible experimental response modes at both peers.
Unspecified overrides retain primary response selection and its receive-mode requirements.

Custom recipient agreement handlers must implement ``blockAckRequestReceived()``
for Basic Block Ack Requests. This callback updates the matching inactivity deadline
before it requests the shared timer update. Block Ack data reception follows the same order.
The recipient stores the timeout it accepts in its ADDBA response.
A zero recipient policy disables expiry; a nonzero policy accepts the requested timeout.
Duplicate requests and response retries retain the accepted interval and current deadline.

``BlockAckRecord`` construction now requires the agreement's initial sequence number.
The record uses this cyclic boundary to distinguish missing frames from old frames.
Its missing-frame bitmap can cause retransmission where the old model silently removed data.

Basic Block Ack receive buffers now resume a repeated BAR at the next expected sequence
when its original start precedes data already delivered after retransmission.
The scan still stops at an incomplete or missing frame.
This progress rule is a model interpretation of the legacy receive-buffer procedure.
Buffer release and delivery also preserve cyclic sequence order across 4095 to 0.
``BlockAckReordering::ReorderBuffer`` is now a vector of sequence/fragment pairs in
delivery order. Custom consumers must iterate that order instead of using map lookup.
These corrections can change delivery counts and simulation fingerprints after packet loss.

HCF now reports ``STOPPED`` and a finish signal when a start listener cancels a grant.
Statistics based on starts minus finishes therefore return to zero after cancellation.
AP disassociation and acknowledged refusal now commit station status before rate removal signals.
The MIB already removes peer rate and HT state when it releases an association ID.

The Block Ack policy bypasses ``blockAckReqThreshold`` when no prepared data
candidate exists or the TXOP limit is zero. A Block Ack Request (BAR) then
takes priority over queued data if a group awaits a request.
With a zero limit, this can produce one BAR/Block Ack exchange per data frame.
This is an INET policy choice. A positive limit permits threshold-based groups
while data candidates remain available; a final BAR can still bypass the threshold.

A positive TXOP limit no longer permits an
arbitrarily oversized first exchange. Configure a legal fragment size or
increase the limit when no supported exception applies.
An unsupported oversized first exchange stops the simulation with a runtime error.
A Duration reservation above 32767 microseconds also stops the simulation.

These changes can alter EDCA reservations, response modes, and event timing.
Recheck experiment results and fingerprints before use.

IEEE 802.11 EDCA Management Recovery
-----------------------------------

The shared ``hcf.edca.mgmtAndNonQoSRecoveryProcedure`` module has moved to
``hcf.edca.edcaf[i].mgmtAndNonQoSRecoveryProcedure``. Each procedure now
updates its own EDCAF's contention window and management retry state.

Replace the old configuration path with
``hcf.edca.edcaf[*].mgmtAndNonQoSRecoveryProcedure`` to apply a setting to
all access categories, or select ``edcaf[3]`` for the current management
classification (voice). Update signal subscriptions and result paths in the
same way. Filter observations by source/AC; an ancestor subscription receives
independent recovery streams, including each instance's initialization sample.

C++ callers must use ``Edcaf::getMgmtAndNonQoSRecoveryProcedure()`` on the
affected EDCAF. The former ``Edca`` getter is removed. For internal collisions,
select the losing EDCAF, which need not be the channel owner.

IEEE 802.11 Beacon and Probe Response Fields
------------------------------------------

``Ieee80211BeaconFrame::channelNumber`` (also inherited by Probe Response) now
represents the DSSS Parameter Set's standard Current Channel value. Its default,
``-1``, means that the element is absent. Custom frame producers that previously
stored a radio's internal index must use
``band->getStandardChannelNumber(channelIndex)`` and account for the element's
three bytes in the body chunk length. Consumers convert a present value back
with ``receivedBand->getChannelIndex(currentChannel)``. These mapping functions
throw for unmappable values. Radio configuration and scan-result channel fields
continue to use internal indices.

The built-in AP emits this element for its modeled 2.4 GHz operation and omits
it for other bands or radios without IEEE channel information. Without the
element, discovery uses the receive channel when available. HT discovery still
uses HT Operation as its primary-channel authority.

As a modeling simplification, legacy APs using custom bands without a standard
channel-number mapping also omit the DSSS element. Supply an explicit mapping
to advertise a standard channel. HT operation still requires a valid mapping;
invalid internal channel indices remain errors in all modes.

AP ``beaconInterval`` values are rounded down to whole 1024-us TUs once, during
initialization, and must be between 1 and 65535 TUs. The effective value drives
both target scheduling and advertised content. Use ``102400us`` for exactly
100 TUs; the default ``100ms`` now schedules targets 97 TUs apart. Actual beacon
transmissions can be delayed by channel access. Custom producers should put
the same effective interval in Beacon and Probe Response bodies as they use
for target scheduling.
The serializers require an interval between 1 and 65535 TUs for both frame
types and throw for out-of-range values, including the default zero interval.
Custom producers must set a valid interval before serialization.

``RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION`` now has its standard value,
62. Code using the symbolic name needs only recompilation. Update external
numeric mappings that used 60 for this reason. Old stored value 60 cannot be
reinterpreted automatically: it also denoted invalid mesh security capability.

Implementing ``IArp``
---------------------

:cpp:`IArp` has a new pure virtual method, which :cpp:`DhcpClient` calls
before it takes a granted address:

.. code-block:: c++

   virtual void sendArpProbe(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address probedAddr) = 0;

An :cpp:`IArp` implementation outside INET no longer compiles until it
overrides this method. An implementation that exchanges ARP packets sends an
RFC 5227 ARP Probe and emits ``arpAddressConflictDetected`` if somebody
answers; :cpp:`Arp` shows how. An implementation that resolves addresses
without packets has nobody to ask. It overrides the method with an empty body,
as :cpp:`GlobalArp` does, and the client then takes the address.

Migrating ``FieldsChunkSerializer`` Subclasses
---------------------------------------------

:cpp:`FieldsChunkSerializer` has two protected hooks for each direction. The old
pair does not see the requested chunk type:

.. code-block:: c++

   void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
   const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

The new pair does:

.. code-block:: c++

   void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
   const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const override;

The old pair is deprecated, but it still works and it stays overridable. The new
hooks call the old hooks by default, so an existing serializer needs no source
change.

Override the new pair in new code. The ``typeInfo`` parameter names the concrete
chunk type that was requested from :cpp:`ChunkSerializerRegistry`. A serializer
that is registered for several chunk types must inspect ``typeInfo`` to build the
exact requested type; the old hook cannot do this and always builds one type. A
serializer that produces a single chunk type may leave the parameter unnamed.

One call between field serializers must use the new name as well, because the
default body of ``deserializeFields()`` is what forwards to the old hook:

.. code-block:: c++

   return OtherSerializer().deserializeFields(stream, typeid(OtherChunk));

.. _mg:sec:migrationguide:architecture:

Network Node Architecture
-------------------------

The internal structure of network nodes has been considerably changed. With the
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
ARP, ICMP, and IPv4 packets to the appropriate protocol modules.

.. _mg:sec:migrationguide:extendingprotocols:

Extending the Known Protocols
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Internally, protocols first must be added to the list of known protocols in the
Protocol class before they can be used. Some protocols (such as IP) have a mapping
between protocol-specific integer identifiers and actual protocols. These mappings
should be created as a :cpp:`ProtocolGroup`. Here are some examples of how to do this:

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
must also register by calling ``inet::registerInterface(...)`` for the corresponding
:cpp:`NetworkInterface` and gate in ``initialize()``. On the other hand, sockets are learned
by the :ned:`MessageDispatcher` automatically on the fly.

.. _mg:sec:migrationguide:attachingtags:

Attaching Tags for Dispatching
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a protocol sends a packet or command to another protocol or interface, it
must attach the appropriate tag for the :ned:`MessageDispatcher`. The dispatcher uses
the attached tags to look up the intended receiver in its registration list and
forwards the message on the appropriate gate. Here are some examples of how to do
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
the simplest case, this can be done by simply replacing the application vector
names. If the example uses more than one kind of application in a single network
node, then the submodule vector indexes must also be updated.

For example, the existing configuration:

.. code-block:: ini

   *.host1.numPingApps = 1
   *.host1.pingApp[0].destAddr = "host7"
   *.host1.numUdpApps = 1
   *.host1.udpApp[0].typename = "UdpSink"

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
- :cpp:`PacketProtcolTag` specifies the protocol of the packet

Tags come in three flavors:

- requests (called ``SomethingReq``) carry information from a higher layer to lower layer protocols
- indications (called ``SomethingInd``) carry information from a lower layer to higher layer protocols
- plain tags (called ``SomethingTag``) contain some meta information
- base classes (called ``SomethingTagBase``) must not be attached to packets

.. _mg:sec:migrationguide:controlinfo:

Splitting Control Infos
~~~~~~~~~~~~~~~~~~~~~~~

When migrating a protocol, the old control info data structures, which were
attached to packets, must be replaced with a set of tags. Implementors should
use already existing tags if possible; otherwise, they are free to create new
ones as they see fit.

Any code that sets, reads, or removes control info objects of packets must be
replaced with code that adds, reads, or removes the appropriate tags.

Setting control info on commands need not be changed but may be adapted for
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
echo application), most likely, all tags on the packet should be removed. The
reason is that the implementor can never be sure what kind of tags are attached
to a packet and what unintended effects those tags will have at a later stage
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
`dynamic_cast` operator with the desired type. The reason is that all packets are
instances of the :cpp:`Packet` class. In fact, this is quite understandable if one
views packets as a sequence of bytes. Any sequence of bytes, no matter how it
is represented by a :cpp:`Packet`, can be interpreted by any protocol, even if the
packet was not intended to be processed by that protocol. Therefore, before a
protocol is sending out a packet using any of its gates, it must attach a
:cpp:`PacketProtocolTag` to it. Here is an example of how to do this:

.. code-block:: c++

   packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4); // updates tag

.. _mg:sec:migrationguide:packetapi:

Packet API
----------

INET provides a new packet API that supports efficient construction, sharing,
duplication, encapsulation, aggregation, fragmentation, and serialization. The
data structure also supports dual representation by default. That is, data can
be accessed as raw bytes and also as field-based classes. Internally, packets
store their data in different kinds of chunks.

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

The most substantial change regarding protocols is that protocol-specific headers
(or messages) are no longer subclasses of :cpp:`cPacket`. Protocol headers subclass the
:cpp:`Chunk` class instead, and they are simply added to Packets during processing.
Variable references to :cpp:`Chunk` objects must use shared pointers (``Ptr<SomeChunk>``)
types. Here is an example of how to do this:

.. code-block:: c++

   auto ipv4Header = makeShared<IPv4Header>(); // creates mutable chunk
   ipv4Header->setSourceAddress(sourceAddress);
   packet->insertAtFront(ipv4Header);

   const auto& ipv4Header = packet->peekAtFront<IPv4Header>(); // return immutable chunk
   auto sourceAddress = ipv4Header->getSourceAddress();

Sometimes processing in a protocol module requires multiple utility functions
and classes. Some functions may need the packet and the protocol header at the
same time. Only passing the protocol header is not sufficient because due to
the shared nature of chunks, they don't have an owner packet. Only passing the
packet requires the called function to peek at the protocol header, which might
unnecessarily slow down execution. In such cases, it is a good idea to pass the
packet and the protocol header in separate parameters. Whether this is desirable
or not highly depends on the complexity of the protocol and the organization of
its implementation.

.. _mg:sec:migrationguide:immutability:

Immutability of Chunks
~~~~~~~~~~~~~~~~~~~~~~

Another important difficulty to note is that chunks can only be added to packets
if they are immutable. This requirement comes from the fact that packets support
peeking into their data regardless of how the data is represented. The result of
peek operations is required to stay consistent with the original content of the
packet. Moreover, the content of packets can be arbitrarily shared with other
packets that may be potentially present in different network nodes. Unfortunately,
these properties forbid arbitrary changes once the chunk has been added to the
packet. Of course, internally, packets do their best to reuse any chunk data
structure if possible.

When the need arises to change the contents of the packet, such as forwarding a
packet in a network protocol, the best thing to do is the following. Remove the
part that is to be updated, create a mutable copy, update it according to the
protocol, and add the updated part back to the packet. In fact, this is like
saying that forwarding a packet is the same as sending out another packet that
shares some structure with the received one. Here is an example of how to do this:

.. code-block:: c++

   auto ipv4Header = packet->removeAtFront<IPv4Header>(); // duplicate is necessary
   ipv4Header->setTimeToLive(ipv4Header->getTimeToLive() - 1); // mutable chunk
   packet->insertAtFront(ipv4Header);

.. _mg:sec:migrationguide:serializing:

Serializing Packets
~~~~~~~~~~~~~~~~~~~

The old packet serializer classes have been replaced with new classes subclassing
from the :cpp:`ChunkSerializer` class. The old serializers used to not only serialize
the packet they were responsible for but they also recursed into the encapsulated packet.
This is no longer the case as serializers are only responsible for the corresponding
chunk that they handle.

Actually transforming a packet to a sequence of bytes doesn't involve directly
calling the serialization API. In fact, calling the serialization API in most
cases is not needed. For example, retrieving the whole contents of a packet as
a sequence of bytes is as simple as follows:

.. code-block:: c++

   packet->peekAt<BytesChunk>(byte(0), packet->getPacketLength()); // generic peek
   packet->peekAllBytes(); // shorthand

This property of the API greatly simplifies code that serializes packets into
trace files, such as PCAP. Finally, the new API allows testing the protocol
implementations for proper emulation support. Configuring all network interfaces
to send out packets (in place of the original packets) which contain a single
:cpp:`BytesChunk` only is easy to do. At the receiver modules, there's no need to change
anything in the protocol implementations. The reason being that the packet API
transparently handles the dual representation, and it converts the sequence of
bytes to the requested chunk types as needed.

.. _mg:sec:migrationguide:checksums:

Handling Checksums
~~~~~~~~~~~~~~~~~~

The old serializer classes used to compute and verify checksums on the fly. This
caused some confusion, especially with the proper support of pseudo headers. With
the new API, this is no longer the case. The new serializers are only responsible
for transforming from one representation (sequence of bytes) to another (fields),
and vice versa.

Computing and verifying checksums is up to the protocol implementations, and it
is independent of the actual representation of the header. In general, protocols
should have parameters to declare the checksum correct/incorrect or to actually
compute and verify it. Of course, for emulation, one should enable computing and
verifying checksums.
