.. _mg:cha:migrationguide:

Migrating Code from INET 3.x
============================
Release: |release|

IEEE 802.11 Prepared Exchanges and Tx Requests
----------------------------------------------

Tx is the component that transmits frames for medium access control (MAC). The hybrid coordination function (HCF) serves quality of service (QoS) traffic. Interframe space (IFS) is a required interval between specified frame transmissions. Short interframe space (SIFS) separates specified immediate responses and frames within an exchange. An acknowledgment (ACK) frame confirms reception when the selected policy requires it.

Each Tx request has an identity before any synchronous callback can occur. The identity contains a lifecycle epoch and a serial. A lifecycle epoch identifies one MAC lifecycle instance. A borrowed pointer gives access without ownership; its owner must keep the object alive during that access. A callback scope covers callback entry, nested callbacks, and return.

Restart changes the epoch so that old identities cannot identify new requests.

Staged frames are frames that the data service extracted and registered for transmission. The frame store owns those frames. A copied ACK snapshot reports their real ACK phase without protocol progress. For example, preparation reads a staged frame that has not transmitted. The real ACK state remains FRAME_NOT_YET_TRANSMITTED after that query.

A prepared exchange records its frames, physical layer (PHY) modes, responses, and required intervals before transmission. The current sequence tree supplies these choices. Prepared execution uses the recorded choices without another selector call. For example, the unit test's mode provider returns 24 Mbps first and 6 Mbps on another query. Preparation records 24 Mbps; prepared execution must use 24 Mbps without another query.

A transmission opportunity (TXOP) gives a QoS station time to start frame exchange sequences. TxopProcedure checks the complete prepared exchange cost. Each continuation includes SIFS before its first transmission. TXNAV is the medium reservation that this station transmits during its TXOP. A separate TXNAV check reads the reservation from actual transmissions.

Hypothetical: the complete continuation costs 100 µs and exactly fits the available TXOP time. Its cost without SIFS before the first transmission is 84 µs. An available TXNAV interval of 84 µs refuses it because that comparison requires strictly less time. The TXOP budget check accepts equality, while the TXNAV check rejects equality.

HCF uses prepared exchanges when ``isBlockAckSupported`` is false. The duration guarantee assumes zero propagation delay and responses after nominal SIFS. The actual response mode and complete frame length must match the prediction. HCF uses actual elapsed time for each continuation check. The exchange regression uses a 350 µs limit with three separate data frames. HCF admits two data/ACK exchanges and retains the third frame for another channel grant.

An oversized initial exchange without a supported exception raises a model-limit error before transmission. The model does not fragment a frame automatically to meet an airtime budget. ``TxopProcedure::getDuration()`` keeps its elapsed-time meaning.

External implementations require these changes:

* Implement ``IFrameSequence::planSequence()``. Implement ``startPlannedSequence()``. Return ``READY`` when a complete plan exists. Return ``EMPTY`` when no candidate exists. Return ``UNSUPPORTED`` when the selected sequence or required input has no supported representation. Duration refusal is a separate admission result.
* Implement ``ITransmitStep::getPreparedTransmit()``. Implement ``IReceiveStep::getPreparedReceive()``. Return a null record for a legacy step. Plans own prepared steps and generated controls. The frame store owns staged data and management frames.
* Add the prepared record argument to the handler's ``transmitFrame()`` callback. Implement ``frameSequenceStarted()`` to report the start before any synchronous transmission or cancellation. Implement ``setPendingTransmission()`` in custom handlers. Implement ``pendingTransmissionCanceled()``.

  Preserve borrowed objects until all synchronous callbacks return. Implement ``beginCallback()`` to defer disposal across Tx callbacks. Implement ``endCallback()`` to release that callback scope. Implement ``resetForLifecycle()``.
* Implement ``IAckHandler::snapshotFrameState()``. Implement ``dropFrame()``. The snapshot query must preserve the exact phase without insertion or protocol progress. A staged frame without an ACK registration is an error.
* Implement ``IOriginatorQoSAckPolicy::getAckTimeoutForMode()``. Implement ``IRtsPolicy::getCtsTimeoutForMode()``. These methods use the supplied response mode and preserve configured timeout overrides. They must not select another mode.
* Replace unidentified Tx calls with ``transmitFrame(id, packet, header, ifs, callback)``. The MAC allocates ``TxRequestId`` before the call. Implement ``cancelPendingTransmission()``. Implement ``resetForLifecycle()``.
* Implement the Tx callback ``isTransmissionPermitted()``. Implement ``transmissionStarted()``. Implement ``transmissionCanceled()``. Implement ``beginCallback()`` to protect borrowed sequence objects throughout each callback scope. Implement ``endCallback()`` to release that callback scope. Add the request identity to ``transmissionComplete()``.

The final permission check runs for zero IFS too. Tx must check identity again after a callback that can replace the request. Hypothetical: request A waits for IFS, and its permission callback replaces A with B. Tx detects the changed identity and cannot transmit A. The check leaves B under its own request identity.

Cancellation distinguishes ``CANCELED``, ``TOO_LATE``, and ``NOT_FOUND``. Only ``CANCELED`` removes the matching delayed copy. Explicit cancellation emits no completion callback. The caller reports successful explicit cancellation to the matching handler once. An on-air request keeps its normal completion path unless lifecycle cleanup aborts it.

``InProgressFrames`` calls its typed removal callback before it removes a referenced frame. HCF registers ``IInProgressFramesCallback`` through ``setRemovalCallback()``. Custom frame stores must preserve the ``frameWillBeRemoved()`` call before removal. Contexts retain frame references until their deferred disposal completes. ``clearDroppedFrames()`` preserves originals while a context still borrows them.

For example, a plan references a frame that the store must remove. HCF invalidates that plan before the store removes the frame.

The distributed coordination function (DCF) uses the legacy path with null prepared metadata. HCF configurations with Block Ack support retain the legacy path without the new duration guarantee. Rebuild external implementations after these interface changes. Preserve the accepted-request contract in `IEEE 802.11 Radio Command Deferral`_ below. That section defines ``ITx::hasTransmission()`` and supplies the ACK/SIFS example. The query also returns false after cancellation or lifecycle reset releases the accepted request.

IEEE 802.11 Block Ack Inactivity Deadlines
------------------------------------------

Block Ack reports reception status for multiple frames under an agreement. Add block acknowledgment (ADDBA) establishes the agreement. Delete block acknowledgment (DELBA) ends it. The traffic identifier (TID) identifies the agreement's traffic class or stream.

HCF retains Block Ack agreements across stop and crash. Downtime counts toward each absolute inactivity deadline. Restart retires overdue agreements before timeout DELBA or other traffic requests channel access. HCF restores the earliest deadline from the remaining agreements. An agreement with timeout zero has no inactivity deadline.

Hypothetical: an agreement expires at 5 s, and the node restarts at 6 s. HCF retires the agreement before channel access because downtime counts toward the deadline. With timeout zero, downtime causes no inactivity expiry.

Late Block Ack activity cannot renew an overdue agreement. HCF discards a data frame that reaches an overdue recipient agreement. HCF also discards Block Ack data after retirement removes the agreement.

The recipient handler sends UNKNOWN_BA DELBA when no agreement exists. Normal Ack and No Ack data retain their current receive paths.

Recipient retirement releases the corresponding reorder buffer. A replacement ADDBA therefore uses its own starting sequence number and buffer size. For example, the retirement test buffers sequence 20 under an agreement that starts at sequence 19. Expiry clears that buffer, and a replacement agreement starts at sequence 100. HCF can deliver sequence 100 without the old window. Other peers and TIDs retain their buffers.

HCF emits one deletion notification for each retired agreement. Delayed timeout DELBA completion preserves a replacement agreement without another deletion notification. Delayed UNKNOWN_BA DELBA completion also preserves a replacement agreement.

External implementations require these changes:

* Implement ``getEarliestExpirationTime() const`` in both agreement handlers. Return the earliest active absolute deadline, or ``SIMTIME_MAX`` if none exists.
* Add the agreement callback argument to ``blockAckAgreementExpired()``. Return timeout DELBA chunks. Remove all overdue entries before deletion callbacks. HCF queues the chunks after both roles complete retirement.
* Implement ``IBlockAckAgreementHandlerCallback::expireBlockAckAgreements()`` to coordinate both roles. Implement ``originatorBlockAckAgreementDeleted()``. Implement ``recipientBlockAckAgreementDeleted()``. The handler owns each borrowed agreement until the deletion callback returns. The callback must not retain or modify the agreement. Complete recipient buffer cleanup before the deletion notification.
* Add the agreement callback argument to ``processReceivedDelba()`` and ``processTransmittedDelba()``. Notify only when the handler removes an agreement.
* Add an ``IProcedureCallback *`` argument to ``qosFrameReceived()``. Return false for Block Ack data when the agreement is absent or overdue. Request expiry for overdue state before this return. For absent state, queue UNKNOWN_BA DELBA through the procedure callback. The procedure callback takes ownership of the new management Packet. The caller discards the data frame on a false result.

  DELBA completion must preserve a replacement after TIMEOUT or UNKNOWN_BA.
* Implement ``IRecipientQosMacDataService::clearReorderBuffer(originatorAddress, tid)``. Release only that peer and TID's buffer with its retained frames. Other peers and TIDs keep their buffers.

The timer callback uses ``IBlockAckAgreementHandlerCallback::scheduleInactivityTimer()`` without an argument. The callback reads both handlers and schedules their earliest absolute deadline. Rebuild external agreement handlers, callbacks, and recipient data services after these interface changes.

IEEE 802.11 Radio Command Deferral
---------------------------------

Tx is the component that transmits frames for medium access control (MAC).
The MAC keeps a radio command while Tx retains an accepted transmission.
This includes the short interframe space (SIFS) before a response.
The hybrid coordination function (HCF) and distributed coordination function (DCF)
release the pending command after the response completes.
This prevents radio reconfiguration before the accepted response completes.

Custom implementations of ``ITx`` need the new method to compile.
Implement the query with this signature:

.. code-block:: c++

   [[nodiscard]] bool hasTransmission() const override;

1. Return true after Tx accepts a frame, including any wait before transmission starts.
2. Keep the return value true while Tx transmits the frame.
3. Clear the accepted transmission state before Tx calls ``ICallback::transmissionComplete()``.
4. Return false when Tx holds no accepted transmission.

The completion callback can release a pending command, so it must observe the cleared state.
For example, Tx accepts an acknowledgment (ACK) frame with a SIFS delay.
A radio command arrives during that delay, when ``hasTransmission()`` returns true.
The MAC keeps the command until the ACK completes.
Tx clears its state before the callback, so the coordination function can release the command.

IEEE 802.11 PHY Mode Properties
-------------------------------

The physical layer (PHY) mode interface ``IIeee80211Mode`` gains three pure
virtual methods. Direct implementations outside INET must implement them:

.. code-block:: c++

   ModulationClass getModulationClass() const override;
   PreambleType getLegacyPreambleType() const override;
   bps getNonHtReferenceRate() const override;

``ModulationClass`` identifies the mode family. Its values are ``UNKNOWN``,
``DSSS_HRDSSS``, ``OFDM``, ``ERP_OFDM``, ``HT``, and ``VHT``.
HT means High Throughput; VHT means Very High Throughput.

``PreambleType`` describes the legacy preamble, which precedes the frame header.
Its values are ``UNKNOWN``, ``LONG``, ``SHORT``, and ``NOT_APPLICABLE``.
``LONG`` and ``SHORT`` apply only to legacy direct-sequence modes.
OFDM, HT, and VHT modes return ``NOT_APPLICABLE`` for this query.
OFDM means orthogonal frequency-division multiplexing.

``getNonHtReferenceRate()`` returns a bound for response-rate selection, in
bits per second. Supported legacy modes return their data rate. HT and VHT
modes derive the bound from the modulation and code rate of stream 1.
The bound does not depend on channel width, guard interval, or stream count.
An unsupported mapping returns ``bps(NaN)``, where NaN means not a number.
Check the value with ``std::isnan(rate.get())`` before rate comparisons.

Subclasses of ``Ieee80211ModeBase`` inherit defaults and need no new override
to compile. Those defaults return ``UNKNOWN`` for both enum queries and
``bps(NaN)`` for the reference rate.
Override each query that the custom mode supports.
Custom rate selectors can use these queries instead of concrete mode casts
or a second copy of the reference-rate formula.

IEEE 802.11 MIB Rate State and Listeners
----------------------------------------

``Ieee80211Mib`` adds rate state to the Management Information Base (MIB).
``Ieee80211RateSetState`` contains ``supported``, ``basic``, and ``operational``
rate sets. Each ``Ieee80211RateSet`` has a ``known`` flag, legacy rates in bits
per second, and HT modulation and coding scheme (MCS) indexes.
The default ``known=false`` means that the information is unknown.
A set with ``known=true`` and no rates is a known empty set.
Check ``known`` before you use an empty rate set.

Read the state with ``getLocalRateSet()``, ``getBssRateSet()``, and
``findPeerRateSet(address)``. BSS means Basic Service Set.
``findPeerRateSet()`` returns ``nullptr`` when no peer record exists.
The getters return views that remain valid until the corresponding update
or clear operation. Copy any state that you need after such an operation.
This includes an update that a synchronous signal listener makes.

Use ``setLocalRateSet()`` for local state.
Use ``setBssRateSet()`` for BSS state.
Use ``installBssAndPeerRateSets()`` to install BSS and peer state together.

These methods validate all input before they change the rate records.
Unknown sets must contain no rates. Known basic and operational sets must
respect the subset checks in ``validateIeee80211RateSetState()``.
An equal rate-set update emits no signal.

``clearBssRateSet()`` restores unknown BSS rate state.
``removePeerRateSet()`` and ``clearPeerRateSets()`` remove peer rate records.

Subscribe to ``Ieee80211Mib::rateStateChangedSignal`` for state notifications.
The signal name is ``rateStateChanged``. A notification carries boolean
``true`` and no details object. The MIB emits it after a committed rate,
HT capability, or channel-operation change. Query the MIB in the listener
to obtain current state.
``installBssAndPeerRateSets()`` publishes both records before one notification.

``releaseAssociationId()`` and ``clearAssociationIds()`` remove peer HT
capabilities and peer rate sets before notification. A teardown that removes
either kind of peer state emits one signal. Empty teardown emits no signal.
A listener can install replacement peer state from the callback.
The teardown leaves that replacement intact.
Standalone HT and rate removal methods retain their own notifications when
they remove state.

Existing configuration parameters and MIB method signatures need no change.

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
