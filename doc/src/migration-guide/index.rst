.. _mg:cha:migrationguide:

Migrating Code from INET 3.x
============================
Release: |release|

TCP Connection Setup
--------------------

The timer ``connEstabTimer`` of ``TcpConnection`` is now ``deferredSynTimer``, and the constant
``TCP_TIMEOUT_CONN_ESTAB`` is now ``TCP_TIMEOUT_DEFERRED_SYN``. The timer only waits for the SEND
of a deferred Fast Open SYN. It no longer limits a connection setup: the SYN and SYN-ACK
retransmissions do (``synRetries``). When they run out, the connection raises the event
``TCP_E_TIMEOUT_CONN_ESTAB``, as the timer did before, so a subclass that handles this event needs no
change.

IEEE 802.11 implicit Block Ack extension points
---------------------------------------------

The default MPDU policy remains disabled. ``HtMpduAggregationPolicy`` enables independent HT20 long-GI members with fixed DATA rates. Custom MPDU policies implement ``isEnabled()`` and ``selectFrames()``. The caller supplies negotiated receiver limits and one retained DATA mode. A policy returns borrowed member references without queue extraction or state changes.

Custom QoS data services implement batch preparation, validation, commitment, and construction through ``IOriginatorQosMacDataService``. Each new member owns one reversible preview. The batch borrows canonical members from the in-progress queue. Commitment validates every queued source before the first extraction. Failed preparation preserves sequence state and independent deadlines.

``IRx::lowerAggregateReceived()`` returns separately owned correct members and immutable delimiter topology. The caller retains the envelope. Custom recipient services pass ``BlockAckReceiveContext`` to the reorder owner. This context supplies physical interval and delimiter facts; it does not replace receipt state. Custom agreement handlers implement ``implicitBlockAckRequested()`` to refresh inactivity for valid implicit DATA.

Custom Tx implementations support ``transmitAggregate()``, ``cancelPendingAggregate()``, and ``finalizeMpdu()``. The aggregate callback validates and commits members at actual submission after the existing IFS timer. Cancellation removes a delayed request before exchange-context disposal. Aggregate completion supplies a null MAC header because the envelope has no outer MAC header or FCS. Every Tx callback implements ``frameTransmissionStarted()`` for an actual-submission notification. A concrete callback can use an empty override when submission requires no state change.

Custom QoS ACK policies accept probe eligibility through existing EDCA classification. They also supply the implicit Block Ack receive-start timeout from the actual response mode. Every custom rate selector implements ``computeCompressedBlockAckResponseMode()`` for implicit callers. A selector without this capability must throw a diagnostic from its concrete override. Explicit Compressed BAR and implicit DATA share this rate calculation without a fabricated BAR.

Custom recipient procedures construct a Compressed response directly from peer, TID, and current agreement state. ``FrameTransmissionDetails`` adds ``FRAME_TRANSMISSION_STATUS_UNACKNOWLEDGED`` for unsuccessful aggregate member attempts. A probe supplies no second DATA attempt outcome. Existing packet-drop signals retain terminal disposal observations.

IEEE 802.11 HT aggregate construction
------------------------------------

The MIB parameter ``htMinimumMpduStartSpacing`` supplies the encoded receiver limit from zero through seven. Zero preserves the default with no spacing restriction. The remaining values represent 0.25, 0.5, 1, 2, 4, 8, and 16 microseconds. Full and simplified management preserve the receiver's value in each negotiated direction. The negotiated transmit direction stores the peer receiver's value.

Custom ``IMpduAggregation`` implementations must supply two HT operations. ``computeAggregateLength()`` previews the PSDU length at the supplied mode and spacing. Zero means an empty batch, unsupported mode, invalid member length, or excessive HT PSDU length. The new ``aggregateFrames()`` overload accepts source references, the mode, encoded spacing, and the peer byte limit. It returns ``nullptr`` when construction exceeds a limit, including either HT duration limit. These operations retain all source Packets and their state.

The existing pointer overload retains its consuming contract. The supplied builder copies region tags from each source's data range to the member's position. It inserts alignment padding and zero-length delimiters between members as necessary. The final member has no padding. A singleton retains its delimiter, so HT-SIG byte conversion preserves its aggregate identity. These construction operations do not enable aggregate selection or implicit Block Ack responses.

IEEE 802.11 HT A-MPDU PHY reception
---------------------------------

HT receivers require a present error model to implement ``IIeee80211AmpduErrorModel``. This optional WLAN contract provides pure mode-support queries and probabilities for bounded PSDU ranges. ``Ieee80211ErrorModelBase`` supplies the implementation for NIST and Yans. Custom HT models outside this base must implement the C++ contract and its NED module interface. Non-HT models require no aggregate role.

An absent error model remains valid. The receiver retains its acquisition, sensitivity, attempt, and SNIR checks without error draws. This includes ordinary HT operation with ``Ieee80211UnitDiskRadio``. The aggregate profile covers scalar and dimensional HT20 reception; it adds no unit-disk aggregate support claim.

The receiver checks role presence at physical-layer initialization. Before the MIB update, the MAC supplies its exact mode set, channel widths, and spatial-stream limit for receive coverage validation. The model must support the modes that these inputs advertise, including HT20 short GI. Unsupported modes outside the effective stream limit cause no rejection. Runtime mode-set changes also check role presence. Any later capability-update caller must validate its exact inputs before the MIB update.

BerTable retains its current limit. Its NED type rejects the radio's HT operation string. The allowed ``n`` value reaches its initializer, which also rejects HT. Both failures precede the new role check. BerTable does not gain HT aggregate support. Supported HT configurations can use NIST or Yans.

The new role diagnostic applies to otherwise valid custom HT models without the contract. The current packet-level BerTable reception path also requires a transmission bit model. Scalar packet-level transmissions do not supply that model. This existing limitation remains outside the HT A-MPDU change.

The modeled HT-SIG header preserves all 48 bits through byte conversion. ``lengthField`` supplies HT Length. ``aggregation`` distinguishes an A-MPDU from a standalone MPDU, including singleton aggregates. The radio derives mode fields from the selected mode and calculates the CRC after field assignment. Decode preserves the received CRC. HEADER validation rejects invalid CRC and unsupported fields before delimiter or MPDU outcomes.

Separate-part reception retains its current abort; whole-signal reception supplies a failed container. DATA separately refuses modeled content shorter than a valid HT Length. This refusal retains the accepted header property but supplies no range outcomes. It represents malformed model input, not physical carrier loss. Decapsulation never uses an invalid header or insufficient content for length-based trailer removal. Ordinary valid HT duration inputs remain unchanged.

``Ieee80211AmpduInd`` carries copied PHY facts without reception or transmission pointers. Each range identifies a delimiter or complete MPDU by PSDU-relative byte offset, byte length, validity, and physical success. Spacing delimiters have four-byte ranges. Alignment padding has no range outcome or separate draw. MAC delimiter and FCS checks still control acceptance; the PHY indication alone grants no delivery or response permission.

The default ``Rx`` sends aggregate containers through a separate member-validation path before standalone FCS or MAC header checks. A failed common header or incomplete content prevents member acceptance. Custom receive modules must keep aggregate containers separate from standalone frame parsing until they implement member acceptance. MPDU deaggregation can extract bounded recovered members from an incomplete layout. Extraction alone grants no MAC acceptance.

The medium cache retains each aggregate decision. PREAMBLE calculates no HEADER or DATA outcome. HEADER, DATA, WHOLE, and direct success queries reuse retained prerequisites and samples. WHOLE summarizes all ranges. Partial DATA failure preserves the aggregate bytes for MAC member validation. The PHY indication alone does not authorize MAC receipt or an implicit Block Ack response.

With a present model, aggregate ``ErrorRateInd`` fields contain ``NaN`` for unknown probabilities. The receiver does not calculate the former whole-PSDU estimate for these fields. Member outcomes are separate from probability estimates. Without a model, the receiver preserves the common base's tag presence and values. Ordinary MPDUs retain their current metadata. Custom readers of aggregate error-rate fields must accept unknown values.

NIST and Yans calculate each range probability from its byte length and the configured scalar SNIR reduction. The receive profile includes HT20 long GI and short GI. Each range uses an independent draw when its success probability lies strictly between zero and one. This model does not represent correlated FEC errors or interference changes within one member. Alignment padding contributes duration but receives no separate outcome. The model checks establish no new error-accuracy or goodput claim.

Radio medium timing queries
--------------------------

Custom radio media must implement ``getTransmissionTimingSnapshot()`` and ``computePrecedingSignalGap()``. These queries supply copied physical endpoints from actual transmissions and cached arrivals. Unavailable endpoints produce an absent result. The snapshot must not retain transmission, arrival, or radio pointers. Communication caches must implement the const ``findCachedArrival()`` query without insertion or other mutation.

``SignalTimeInd`` now carries the medium's transmission identifier. The receiver supplies this local indication when it accepts a transmission. Consumers can associate later frame delivery with the corresponding physical timing evidence. The identifier is local metadata; it adds no transmitted field.

IEEE 802.11 signal extension
---------------------------

Custom IEEE 802.11 modes must implement ``getSignalExtensionTime()``. ``Ieee80211ModeBase`` supplies zero. ERP and 2.4 GHz HT modes supply their six-microsecond extension. HT duration now includes that extension, which can change airtime and simulation results. The transmission query derives the actual extension from its waveform, so consumers must not add the nominal value again.

IEEE 802.11 RIFS permission
--------------------------

HT Operation now preserves RIFS Mode through management conversion and equality checks. VHT operation clears that permission. The change preserves the received permission for a consumer that checks RIFS relations; it adds no RIFS transmission scheduler.

IEEE 802.11 Block Ack wire producers
-----------------------------------

The serializer decodes the complete four-bit Type field. Type 0 retains Basic compatibility; Type 2 selects Compressed. Other Types produce invalid input. Basic producers must keep the two low bits of their reserved field zero because those bits overlap Type. The serializer rejects inconsistent Basic objects. Compressed reserved fields write zero; received reserved values do not affect the decoded result.

Compressed producers must supply Fragment Number zero and a 64-bit bitmap. A Compressed Block Ack header chunk contains 28 bytes before FCS. Both BAR profiles occupy 24 bytes with FCS. Keep the corresponding header chunk and trailer lengths consistent. The serializer rejects malformed wire fields before runtime processing.

IEEE 802.11 bounded fragment preparation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IFragmentationPolicy`` implementations must implement ``computeBoundedFragmentSizes(frame, maxMpduBytes)``. The bound includes the MAC header and FCS. The result distinguishes an unfragmented frame, a legal fragment partition, and an impossible partition. Each nonfinal fragment must have the same even length. A partition must contain at most sixteen fragments. The existing unbounded operation remains available to non-QoS callers.

Custom ``IOriginatorQosMacDataService`` implementations must implement the fragment preparation, validation, and commitment operations. Preparation retains queued sources and owns all fragment previews. Commitment transfers those exact previews with one sequence identity, one allocation identity, and the source's absolute deadline. For a numbered source, the caller supplies a live source with no local DATA transmission. The caller replaces that source through ``InProgressFrames::replaceWithPreparedFragments()`` after commitment succeeds.

A fragment preparation can report an unfragmented or impossible partition without fragment previews. The caller must check its status before commitment.

Custom ``IAckHandler`` implementations must expose ``dropFrame()`` through the interface. Both built-in handlers already supply this operation. The committed-frame owner uses it to remove the old ACK entry before fragment replacement.

IEEE 802.11 reversible DATA preparation
-------------------------------------

Custom QoS data services must implement the reversible preparation operations in ``IOriginatorQosMacDataService``. ``prepareFrame()`` creates an owned preview and retains original MSDUs in their queue. ``isPreparationValid()`` checks current source identity, eligibility, lifetime, sequence state, and aggregation permission. ``commitPreparedFrame()`` transfers the same preview pointer only after successful validation. Failure must preserve the preview and all original queue frames. ``InProgressFrames::adoptPreparedFrame()`` accepts a committed frame without another extraction.

IEEE 802.11 Compressed Block Ack extension points
------------------------------------------------

HCF defaults to ``mac.hcf.blockAckVariant = "compressed"``. Both endpoints must select the same profile. ADDBA does not negotiate this selector. A valid addressed BAR with the other profile stops the simulation before agreement updates. The diagnostic identifies the peer, TID, local profile, and received profile. Configure the same selectors before a new run.

Compressed agreements require usable local HT support and directional peer HT state. An HT PHY mode alone does not supply that state. Without it, HCF retains Normal Ack DATA. Explicit ``blockAckVariant = "basic"`` retains historical Basic compatibility without that HT requirement. Compressed agreements suppress fragmentation for their peer and TID. Fragmentation outside those agreements retains its existing threshold.

Compressed supports immediate Block Ack only. Keep ``delayedAckPolicySupported = false`` in both agreement policies. HCF rejects either true value during initialization when Compressed support is enabled. Basic retains its existing delayed-policy configuration contract.

Custom QoS rate selectors must implement ``computeCompressedBlockAckReqMode(packet, header, startsTxop)``. HCF supplies the request's actual position in the TXOP. Select a legal non-HT mode from the applicable BSS or peer rates. A configured ``controlFrameBitrate`` must satisfy that rule. The existing Basic rate-selection methods remain available.

Custom recipient ACK policies must implement ``computeCompressedBlockAckDurationField()``. This operation subtracts SIFS and the actual Compressed response airtime from the request reservation. Compressed Block Ack contains 32 bytes with FCS; Basic contains 152 bytes. Retain the existing Basic operation.

Custom recipient data services must accept ``bool interframe = false`` in ``dataFrameReceived()``. The value states whether HCF established the required SIFS or RIFS relation. A false value supplies no interval evidence. Preserve that value through the recipient reorder path. Custom callers that omit it retain false.

Custom frame sequences must supply HCF's resolved profile to ``QoSContext``. Its constructor retains Basic for callers that omit the profile. Custom agreement creation must set the recipient agreement's variant before use. An agreement must retain that variant throughout its lifetime.

Custom frame sequences must preserve BAR arbitration before DATA preparation. The HCF preparation callback distinguishes the legacy path, accepted preparation, and admission refusal. Refusal must end the sequence without a DATA failure. Keep the selected DATA mode through RTS/CTS. Validate that decision without another adaptive rate query. Admit the complete exchange before source removal.

The active Compressed path reserves only the current exchange under single protection. A later BAR can require fresh contention. Before DATA commitment, failed RTS updates QSRC and the contention window without a provisional sequence counter. QSRC is the short retry counter for an access category. QSRC rollover does not discard queued DATA. Original lifetimes bound repeated RTS attempts.

Basic and outside-agreement fallback retain their existing preparation and recovery paths.

IEEE 802.11 Basic Block Ack extension points
------------------------------------------

HCF previews the next eligible DATA before BAR selection. Preview must preserve queue ownership and the next sequence number. Custom QoS data services must implement ``IOriginatorQosMacDataService``. This contract extends ``IOriginatorMacDataService`` with ``peekEligibleFrame()``, ``getNextSequenceNumber()``, and ``hasAMsduAggregation()``.

Basic single protection reserves only an already prepared successor. HCF ends the TXOP before further DATA extraction. A queued unit can change size or require agreement setup before transmission. Custom frame sequences must preserve this boundary when they use the same forecast.

Custom QoS ACK policies must add ``IOriginatorBlockAckAgreementHandler *agreements`` to ``isBlockAckReqNeeded()`` and ``computeBlockAckReqParameters()``. The argument supplies current agreement windows. Policies must keep peer and TID identities separate. The supplied policy can request BAR without a DATA candidate.

Callers must supply the agreement handler explicitly through both the interface and concrete policy types. An omitted argument fails compilation. Pass ``nullptr`` explicitly when no agreement handler is available; the supplied policy then requests no BAR. Custom overrides must also require the argument.

Calls to ``QosAckHandler::processTransmittedBlockAckReq()`` must supply the originator agreement explicitly. The agreement restricts processing to the negotiated window. An explicit ``nullptr`` retains bitmap-only coverage without that restriction. Production HCF always supplies the active agreement.

Custom ``InProgressFrames`` classes must add ``bool extract = true`` to their ``getPendingFrameFor()`` override. When ``extract`` is false, the method must return a borrowed candidate without extraction or sequence assignment. The default value preserves ordinary extraction. Custom candidate sources must also implement ``peekFrameToTransmit()`` without extraction.

TCP Algorithms
--------------

The algorithm of a TCP connection is a module now: each connection creates it as its submodule
``algorithm``, of the NED type that the ``tcpAlgorithmClass`` parameter names. An algorithm
implemented outside INET needs these changes:

- Register the class with ``Define_Module()`` instead of ``Register_Class()``.
- Add a simple module type that extends ``TcpAlgorithm``, with the C++ class in its ``@class``
  property. The value of ``tcpAlgorithmClass`` names this type: a name in the package
  ``inet.transportlayer.tcp.flavours``, or the qualified name of a type in another package. Without
  the type, the connection stops with an error.
- Rename the override of ``initialize()`` to ``initializeAlgorithm()``. The connection calls it in
  its own context, so the timers that it creates belong to the connection. ``initialize()`` is now
  the initialization of the module.
- Rename the calls of the ``cancelEvent()`` helper of ``TcpAlgorithmBase`` to ``cancelTimer()``.
  Delete a timer with ``conn->deleteTimer()``, because the connection owns it.
- Move the parameters of the algorithm from ``Tcp`` to the new type, and read them with ``par()``.

For example:

.. code-block:: ned

   import inet.transportlayer.tcp.flavours.TcpAlgorithm;

   simple MyTcpAlgorithm extends TcpAlgorithm
   {
       parameters:
           @class(MyTcpAlgorithm);
           double gain = default(0.5);
   }

The parameters of CUBIC, HyStart and DCTCP moved to ``TcpCubic`` and ``DcTcp``, so their ini keys
change. For example, ``**.tcp.cubicBeta`` is now ``**.tcp.conn-*.algorithm.beta``, and
``**.tcp.dctcpGamma`` is now ``**.tcp.conn-*.algorithm.g``. WHATSNEW lists all of them.

Queueing Classifiers
--------------------

``PacketClassifierBase::classifyPacket()`` is a query. The base class calls it from
``canPushPacket()``, ``canPullPacket()`` and ``handleCanPullPacketChanged()``, and again from the
operation that moves the packet. So it must not change the state of the module. Custom classifiers
must use this signature:

.. code-block:: c++

   int classifyPacket(Packet *packet) const override;

Move each state change of ``classifyPacket()`` into the new hook. The base class calls it one time
for each packet that goes through the module, and one time for each packet stream, before the
packet leaves the module:

.. code-block:: c++

   void handlePacketClassified(Packet *packet, int index) override;

For example, a classifier with a token bucket compares the token count in ``classifyPacket()``, and
it removes the tokens in ``handlePacketClassified()``. A classifier that overrides ``pushPacket()``
or ``pullPacket()`` must call ``handlePacketClassified()`` itself.

Queueing Schedulers
-------------------

``PacketSchedulerBase::schedulePacket()`` is a query. The base class calls it from
``canPushSomePacket()`` and ``handleCanPushPacketChanged()``, and again from the operation that
moves the packet. So it must not change the state of the module. Custom schedulers must use this
signature:

.. code-block:: c++

   int schedulePacket() const override;

Move each state change of ``schedulePacket()`` into the new hook. The base class calls it one time
for each packet that goes through the module, and one time for each packet stream, when the module
holds the packet and before the packet leaves the module:

.. code-block:: c++

   void handlePacketScheduled(Packet *packet, int index) override;

For example, a weighted round-robin scheduler selects the input in ``schedulePacket()``, and it
decrements the bucket of that input in ``handlePacketScheduled()``. The hook gets the packet, so a
scheduler that charges by length, such as deficit round-robin, can use its length. A scheduler that
overrides ``pushPacket()`` or ``pullPacket()`` must call ``handlePacketScheduled()`` itself.

Queueing Filters
----------------

``IPacketFilter::matchesPacket()`` is a query. ``PacketFilterBase`` calls it from
``canPushPacket()`` and ``canPullPacket()``, and again from the operation that moves the packet. So
it must not change the state of the module, and it must not draw a random number. Move each state
change into the new hook. The base class calls it one time for each packet that goes through the
filter or that the filter drops, and one time for each packet stream, when the module holds the
packet and before the packet leaves the module:

.. code-block:: c++

   void handlePacketFiltered(Packet *packet, bool matches) override;

``processPacket()`` changes each packet instance that passes, for example it removes a header, and
``dropPacket()`` disposes of each instance that the filter drops. In a packet stream the base class
calls them for each instance, so neither may change the state of the module. A filter that decides
with a random number draws the number in advance, and draws the next one in
``handlePacketFiltered()`` after a decision used it.

IPsec Sequence Numbers
----------------------

``SecurityAssociation::getAndIncSeqNum()`` returned the counter and then incremented it, so the
first packet of every SA carried the Sequence Number 0. RFC 4302 and RFC 4303 increment the counter
first, so that the first packet carries 1. The method is now
``SecurityAssociation::incrementAndGetSeqNum()``, and returns the incremented value. C++ code that
called the old method must use the new name, and expects one more than before.

IEEE 802.11 QoS DATA Lifetime
-----------------------------

HCF enforces a finite lifetime for every QoS DATA frame. Each access category uses ``mac.hcf.edca.edcaf[*].msduLifetime``, with a default of 0.512 seconds. This lifetime applies with Block Ack support enabled or disabled. HCF starts the timer at MAC acceptance, before queue insertion and sequence assignment. Queue residence can therefore cause disposal before the first transmission.

Set a positive, finite value for each required access category. Zero, negative, and nonfinite values fail at initialization. A value that rounds to zero at the simulation time resolution also fails. A long finite value permits compatibility studies without an unlimited mode:

.. code-block:: ini

   **.mac.hcf.edca.edcaf[*].msduLifetime = 100s

This value prevents lifetime expiration for newly accepted DATA during a ten-second run. Acknowledgment, queue loss, and retry-limit disposal can still resolve DATA earlier. Recovery that depends on expiration inherits the configured delay. The value does not guarantee unchanged fingerprints or delivery results.

A-MSDU construction restarts the aggregate timer when each MSDU joins the aggregate. The current builder adds all constituents at one simulation time. Fragments retain the source MSDU deadline. Retransmission and ACK-policy changes do not restart a deadline. An accepted exchange can complete after expiration. HCF retains its referenced DATA until safe completion, but permits no later attempt after expiration.

The concrete ``EdcaTransmitLifetimeHandler`` now uses local packet identities instead of sequence numbers. External C++ consumers must use its registration and replacement operations with packet identities. EDCA owns the handler. The handler cannot be copied. The DCF lifetime interface remains unchanged. This change does not add management lifetime or MAC stop/start recovery.

IEEE 802.11 Block Ack Recipient Delivery
----------------------------------------

Block Ack recipients return complete units in cyclic sequence order. External C++ callers of ``BlockAckReordering::processReceivedQoSFrame()`` and ``processReceivedBlockAckReq()`` must use ``BlockAckReordering::ReleasedFrames`` for their results. This type is a vector of sequence-number and fragment-group pairs. The indexed receive buffer retains its map storage.

Replace explicit ``BlockAckReordering::ReorderBuffer`` result declarations with ``BlockAckReordering::ReleasedFrames``. Iterate over the vector in its supplied order. Keep that order through reassembly, A-MSDU deaggregation, and delivery. Do not insert released units into a numeric sequence map. For example, a release across sequence wrap supplies 4095 before 0.

``ReceiveBuffer::getLength()`` counts occupied sequence positions. All Basic fragments of one MSDU count once. ``isFull()`` uses that same count. Custom diagnostics that require a fragment count must count the fragments in each stored position separately.

The protected ``getEarliestCompleteMsduOrAMsduIfExists()`` helper remains available for one release but is deprecated. It retains its lookup behavior for external subclasses and returns fragments owned by the receive buffer. The helper does not remove frames or transfer ownership. Its returned pointers remain valid only while the buffer retains those fragments. For delivery, consume the ordered ``ReleasedFrames`` results from ``processReceivedQoSFrame()`` or ``processReceivedBlockAckReq()``. These methods remove complete units from storage and transfer their ownership to the caller.

No configuration change is necessary. Gap release, window movement, and BAR before DATA can change delivery times and fingerprints in affected Basic Block Ack scenarios. These repairs do not enable Compressed Block Ack.

IEEE 802.11 Block Ack Agreement Lifecycle
------------------------------------------

HCF now defers new DATA extraction for a peer and TID during Block Ack setup or recovery. Already numbered DATA resolves before HCF creates ADDBA. The first DATA under an accepted agreement therefore retains the request sequence number. Both supported profiles use this lifecycle contract.

The recipient response now determines the buffer size and inactivity timeout. The built-in recipient selects the smaller of 64 and its configured maximum. A request size of zero supplies no size guidance. The response copies the request dialog token. Both negotiated A-MSDU flags must permit aggregation under an active agreement.

The originator starts ``mac.hcf.originatorBlockAckAgreementPolicy.addbaFailureTimeout`` after the request receives its ACK. Its default is 100 ms. The value must be positive and finite.

If the response does not arrive, HCF sends DELBA before it releases surviving unsent DATA. DATA lifetime separately limits retained DATA. Failed DELBA disposes of the current deferred batch. Later DATA starts one fresh recovery exchange.

At ordinary teardown, the originator discards every transmitted, unacknowledged MPDU for the selected agreement. This includes DATA whose lifetime remains valid when inactivity expires. It also discards unsent fragments through the highest transmitted allocation index, even if an earlier fragment received an ACK. This local index records allocation order before fragmentation. Later unsent DATA retains sequence continuity.

For example, fragment 0 of sequence 4095 enters transmission before teardown. Teardown discards its unsent fragments and preserves the intact successor with sequence 0. The recipient delivers complete buffered units in cyclic order before it releases resources. It discards incomplete units. For example, complete units 101 and 102 can reach the upper process after teardown across absent unit 100.

Custom originator code must call ``recordTransmittedData(allocationIndex)`` instead of ``markFirstDataTransmitted()`` or the former cyclic-sequence overload. The QoS data service assigns an increasing local index before fragmentation. All fragments retain this index in ``Ieee80211DataSequenceTag``. Custom QoS data services must supply the same metadata. The agreement exposes its highest transmitted index through ``getHighestTransmittedDataIndex()``, which replaces ``getHighestTransmittedSequence()``. HCF records this index when the transmitter accepts DATA.

The index remains available after ACK processing removes a frame. Teardown compares allocation indices to remove unsent fragments of a partially transmitted unit. This comparison remains valid across sequence gaps and wraparound. For example, transmission of sequences 0 and 2048 updates the boundary correctly when intervening DATA receives allocation indices but never transmits. Retransmission of older DATA cannot move the boundary backward.

Association reset instead discards all buffered units from the old association. Management calls ``IBlockAckAgreementLifecycle::resetPeerAgreements()`` with the old peer address before replacement. The MAC implements this contract and forwards the reset to HCF. Custom management implementations must supply this handoff for association replacement and loss. A rate-state notification alone cannot identify replacement with identical capabilities.

With Block Ack support enabled, peer reset also discards queued and in-progress QoS DATA addressed to the old peer. This includes unsent Normal Ack DATA without an agreement. Accepted physical exchanges retain their context until safe completion. Reset preserves other peers' queued DATA and lifetime records.

Custom MAC implementations that use these management modules must implement ``IBlockAckAgreementLifecycle``. The reset must complete both agreement roles' cleanup before it reports deletion. It must preserve an unrelated management exchange that already holds an accepted transmission.

``blockAckAgreementDeleted`` now supplies a nonnull agreement after removal from lookup and completion of the required cleanup. The payload identifies the selected role through its agreement class. The payload remains valid only during the synchronous callback. Listeners must not retain or modify it. Association reset removes both roles before the first callback. Ordinary directional teardown preserves the opposite role.

Custom QoS recipient services must return accepted, complete Actions from ``managementFrameReceived()``. HCF owns their agreement dispatch after duplicate checks. Services must implement directional retirement and peer cleanup through the recipient-service contract. Custom agreement policies must implement ``isProfileAllowed()`` with the supplied typed profile and usable capability inputs. Basic admission must preserve its historical non-HT compatibility.

Custom agreement handlers must implement the new procedure and retirement operations in their handler contracts. The originator handler owns setup intent, request identity, response deadlines, teardown, and recovery state. Its ``noteEligibleDataTransmission(peer, tid)`` operation records ``WAITING_FOR_SUCCESSOR`` after an eligible DATA transmission. This phase permits ordinary DATA access and has no request or deadline. A later queued successor promotes the phase to setup intent before extraction. Peer reset clears the phase.

The recipient handler owns its active agreements and pending teardown. Detachment removes an agreement from lookup and transfers it to the caller. The caller retains the object through its deletion callback. Const observations expose current state without a second agreement store.

Custom implementations of ``IBlockAckAgreementHandlerCallback`` must implement ``scheduleInactivityTimer()`` without a deadline argument. Agreement handlers request recomputation after their state changes. HCF queries both roles and schedules their earliest absolute deadline. It cancels the timer when neither role has a finite deadline. Custom callers must remove the old deadline argument.

Custom subclasses of ``OriginatorQosMacDataService`` must adapt the protected ``aMsduAggregateIfNeeded(queue, selected)`` helper. The selected packet is the first eligible packet in service order. An aggregate candidate list must start with that packet. The service validates every candidate before source removal and calls the aggregation policy once.

Custom queues need ``inet::queueing::ISelectivePacketCollection`` only when a deferred next frame hides eligible work. HCF retains ordinary dequeue when the advertised next frame is eligible. It waits when every frame is deferred. It reports the missing capability before mutation when eligible work requires a bypass.

The optional ``peekEligiblePacket(predicate)`` operation returns a borrowed packet in configured service order without mutation. ``extractPacket(packet)`` transfers that exact packet to its caller. Implementations must preserve their ordinary pull effects, including ownership, residence metadata, events, statistics, and applicable animation. Built-in simple queues, priority schedulers, and compound queues support this contract. Custom subclasses that change pull effects must adapt selected extraction. The optional contract supports complete packets only.

Providers that cannot perform a selected operation must throw ``inet::queueing::UnsupportedPacketSelection``. This exception permits ordinary service and collection inspection without the optional capability. Genuine queue and predicate errors must retain their original type.

External frame-sequence handlers must implement ``interruptFrameSequence()``. The handler owns the interruption outcome after recipient processing. It suppresses failure feedback when the transmitted DATA or management frame no longer belongs to the in-progress queue. Other exchanges retain ordinary failure feedback. Cleanup must destroy the exchange context before it deletes deferred frame copies. These repairs do not add MAC stop/start recovery or final-transfer teardown.

IEEE 802.11 Response Query Roles
--------------------------------

Response selectors now distinguish sender estimates from actual recipient responses. Previously, sender queries used the frame's transmitter address and could replace a valid HT response with a legacy mode.

External implementations of ``IRateSelection`` must provide these signatures:

.. code-block:: c++

   const IIeee80211Mode *computeResponseAckFrameMode(Packet *packet,
           const Ptr<const Ieee80211DataOrMgmtHeader>& header, ResponseQueryRole role) override;
   const IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet,
           const Ptr<const Ieee80211RtsFrame>& rts, ResponseQueryRole role) override;

``IQosRateSelection`` requires the same ACK and CTS changes. It also requires this signature:

.. code-block:: c++

   const IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *packet,
           const Ptr<const Ieee80211BlockAckReq>& bar, ResponseQueryRole role) override;

The enum is in ``inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h``. Update external callers and overrides as follows:

- Use ``ResponseQueryRole::PREDICT`` for sender reservations, response timeouts, and exchange admission estimates.
- Use ``ResponseQueryRole::RESPOND`` for actual ACK, CTS, or Basic Block Ack selection and recipient response duration calculations.
- Forward the role when an override calls another selector.

The frame argument identifies the frame that requires a response. Prediction uses its receiver address and ``localRxPeerTx``, the capability for peer transmission to the local receiver. A recipient response uses its transmitter address and ``localTxPeerRx``, the capability for local transmission to the peer receiver. Frame mode tags do not determine the role.

For example, compatible associated peers can now retain the same 13 Mbps HT response in both queries. The old sender query could select 12 Mbps legacy ERP-OFDM in ``n(mixed-2.4Ghz)``. Existing HT simulations can therefore change reservations, response timeouts, and TXOP admission. See :ref:`ug:sec:80211:response-prediction` for agreement conditions and unknown Tx limits.

IEEE 802.11 HT Support in ac Interfaces
---------------------------------------

``opMode = "ac"`` now enables existing HT support. HT management requires standard channel numbers for HT Operation elements. An ``ac`` AP on an arithmetic 5 GHz band therefore fails initialization, including simplified management. For standard channel 36 at 5180 MHz, use:

.. code-block:: ini

   **.wlan[*].opMode = "ac"
   **.wlan[*].radio.bandName = "5 GHz (mapped 20 MHz)"
   **.wlan[*].radio.channelNumber = 0
   **.wlan[*].radio.centerFrequency = 5.18GHz

Remove an earlier band override if it would take precedence over these settings. Preserve the required data rate, channel width, and spatial stream constraints. For centers outside the mapped subset, use a custom band with explicit standard channel conversion.

The supplied configurations now use index 0 at 5180 MHz:

.. list-table:: Effective initial center changes
   :header-rows: 1

   * - Configuration
     - Previous center
   * - ``tests/protocol/wifi/ini/_ac.ini``
     - 5040 MHz
   * - ``examples/wireless/lan80211ac/omnetpp.ini``
     - 5010 MHz
   * - ``tests/protocol/wifi/11ac/Ac_160mhz.test``
     - 5080 MHz

These are effective radio frequencies; the earlier ``centerFrequency = 5GHz`` setting did not make them 5000 MHz. The new frequencies can change propagation results and fingerprints. Added HT entries can also change ACK/CTS candidates and adaptive rate steps. See :ref:`ug:sec:80211:ac-ht` for the channel mapping, radio requirements, and current model limits.

IEEE 802.11 TXNAV and Sequence Callbacks
--------------------------------------

Custom ``IRx`` implementations must provide these operations:

.. code-block:: c++

   simtime_t getTxnavRemaining() const override;
   void successfulFrameTransmitted(simtime_t ppduEnd, simtime_t durationField) override;
   void setContentionBlocked(bool blocked) override;

Keep TXNAV separate from received NAV. Replace its endpoint with ``ppduEnd + durationField`` after successful holder transmission. Cancel TXNAV for zero or expired fields. Preserve TXNAV after TXOP end. Exclude contention while TXNAV remains active. Keep ``isMediumFree()`` independent of TXNAV for the CTS policy and radio configuration.

Honor HCF's separate contention permission through ``setContentionBlocked()``. HCF blocks contention after internal-collision handling and restores it after channel release and TXOP end. This permission prevents a second grant during an initial response wait, when successful TXNAV does not yet exist. Preserve the CTS medium query.

Custom single-protection admission must compare the complete exchange cost strictly against TXNAV at the decision before SIFS. Exclude the initial SIFS from that exchange cost. Include the initial SIFS in the separate positive TXOP limit check. For example, a 238 us DATA/ACK exchange passes with 246.67 us TXNAV and at least 248 us of positive TXOP time. This example uses 10 us initial SIFS. The timer sample instant follows INET's stated interpretation of IEEE Std 802.11-2024, Clause 10.23.2.8.

Custom sequence callbacks must provide these signatures:

.. code-block:: c++

   void frameSequenceStarted() override;
   bool transmitFrame(Packet *packet, simtime_t ifs) override;

Emit the start observation from ``frameSequenceStarted()``. The handler installs its context and initializes the sequence before this call. Return true after submission to Tx. Return false before submission to end the sequence normally. The handler destroys the refused step before finish observers inspect history. An empty preparation or immediate refusal therefore produces one start and one finish with valid contexts.

TxOpFs publishes the selected DATA ACK policy before execution. HCF consumes that policy without a second DATA-submission query. Custom ACK policies must not require effects from that removed query. Their method signatures remain unchanged.

IEEE 802.11 Local NAV Choice
---------------------------

Tx transmits MAC frames and can update the local network allocation vector (NAV). The NAV records a
reservation of the medium. Both ``ITx::transmitFrame()`` overloads require an explicit
``updateLocalNav`` argument. Custom implementations must use these signatures:

.. code-block:: c++

   void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
           bool updateLocalNav, ITx::ICallback *callback) override;
   void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
           simtime_t ifs, bool updateLocalNav, ITx::ICallback *callback) override;

Pass ``true`` for DCF holder frames. Pass ``false`` for HCF holder frames and recipient responses. For example, HCF publishes successful RTS and DATA fields through shared TXNAV. DCF retains the local NAV update. The transmitted duration field
remains in each response. The response no longer extends the recipient's local NAV. Received
reservations still prevent a recipient CTS when the medium is busy.

The immediate overload forwards the caller's choice with zero interframe space:

.. code-block:: c++

   transmitFrame(packet, header, SIMTIME_ZERO, updateLocalNav, callback);

1. Retain the choice with the accepted frame through its delay and transmission.
2. Save the completed frame's choice before the completion callback.
3. Clear the retained state before that callback.
4. Apply the local NAV update after the callback only when the saved choice is true.

.. code-block:: c++

   auto completedUpdateLocalNav = updateLocalNav;
   auto completedCallback = txCallback;
   auto completedFrame = frame;
   auto duration = completedFrame->peekAtFront<Ieee80211MacHeader>()->getDurationField();
   frame = nullptr;
   txCallback = nullptr;
   updateLocalNav = false;
   completedCallback->transmissionComplete(completedFrame,
       completedFrame->peekAtFront<Ieee80211MacHeader>());
   delete completedFrame;
   if (completedUpdateLocalNav)
       rx->frameTransmitted(duration);

The callback can accept another transmission with a different choice. For example, a completed
recipient CTS must retain ``false`` even if the callback accepts a holder frame with ``true``. Do
not use default arguments on these virtual methods.

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
