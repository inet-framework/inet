IEEE 802.11 Block Ack Agreement Lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
