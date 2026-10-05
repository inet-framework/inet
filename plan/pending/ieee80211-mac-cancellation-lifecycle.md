# IEEE 802.11 MAC cancellation and lifecycle

## 1. Problem and intended result

The MAC transmission module, Tx, can retain a frame during an interframe space (IFS) or transmission. A synchronous callback can cancel that request or replace its frame sequence. Old callbacks must not advance the replacement sequence. Stop and crash must release requests before coordination functions retire their exchanges.

Block Ack confirms reception of multiple frames under an agreement. The hybrid coordination function (HCF) retains agreements across downtime. Restart must retire overdue agreements before traffic requests channel access. Recipient retirement must clear only that agreement's reorder buffer.

This change applies to the distributed coordination function (DCF) and hybrid coordination function (HCF). It does not add exchange-duration admission. It preserves radio command deferral while Tx retains an accepted transmission.

## 2. Request identity and cancellation

The MAC allocates each request identity before submission to Tx. The identity contains a lifecycle epoch and a serial. Tx stores that identity with its accepted frame. Tx checks permission before transmission, including zero IFS. Tx checks identity again after each permission callback because that callback can replace the request.

Hypothetical: request A waits for IFS. Its permission callback cancels A and submits B. Tx detects the changed identity. Tx cannot transmit A or advance B through A's callback.

Cancellation removes only the matching delayed copy. It distinguishes successful cancellation, an absent request, and an on-air request. Explicit cancellation emits no completion callback. The caller reports successful cancellation to the matching sequence handler once. An on-air request follows its completion path unless lifecycle reset aborts it.

The sequence handler defers disposal until nested callbacks return. It protects borrowed contexts and steps during Tx callbacks. The contention module detaches its callback before channel-grant notification. A synchronous callback can therefore request channel access again safely.

## 3. Lifecycle reset and restart

The MAC resets Tx before it resets DCF or HCF. Reset changes the lifecycle epoch. Old request identities cannot identify requests after restart. Coordination functions stop response timers and retire the current exchange. Group, No Ack, and Block Ack transmissions do not enter Normal Ack failure processing during reset.

HCF retains absolute Block Ack inactivity deadlines. Downtime counts toward each finite deadline. Timeout zero disables inactivity expiry. Restart retires overdue agreements in both roles before new traffic requests channel access. The agreement handlers detach overdue state before deletion callbacks. HCF queues timeout DELBA frames after both roles finish retirement.

Hypothetical: an agreement expires at 5 s. The node restarts at 6 s. HCF retires the agreement before channel access. Late activity cannot renew it.

The recipient data service clears the retired peer and traffic identifier's reorder buffer. Actual retirement emits one deletion notification. Delayed TIMEOUT or UNKNOWN_BA DELBA completion preserves a replacement agreement. HCF discards Block Ack data without an active agreement and queues UNKNOWN_BA DELBA. Normal Ack and No Ack data retain their receive paths.

For example, the retirement test buffers sequence 20 under an agreement that starts at sequence 19. Retirement clears that buffer. Replacement ADDBA starts at sequence 100, which HCF can deliver without the old window. Other peers and traffic identifiers retain their buffers.

## 4. Recipient setup and response identity

An accepted ADDBA request creates pending recipient state. The inactivity deadline starts at the first response transmission completion. Pending state and timeout zero have no finite inactivity deadline. Response retries preserve the first deadline.

The recipient handler gives each setup a nonzero local identity. The existing management transaction tag carries that identity on its response Packet. HCF checks the peer, traffic identifier, setup identity, and deadline before transmission. HCF removes stale response ACK and frame state before normal cancellation ends the sequence. Completion ignores stale identities, including an old response after transmission starts.

Hypothetical: response A remains queued while its agreement ends. A new request creates agreement B for the same peer and traffic identifier. HCF rejects response A before transmission. Response B starts B's inactivity deadline. A late completion cannot activate or renew B.

## 5. Commit boundaries and verification

1. Record this plan and its verification scope.
2. Add request identities, exact cancellation, and callback disposal guards. Verify the cancellation module cases and existing DCF/HCF exchanges.
3. Add lifecycle reset, restart deadlines, and agreement retirement. Start recipient inactivity after the first matching ADDBA response transmission. Verify lifecycle and agreement tests against the production handlers. Verify pending setup, duplicate completion, stale completion, and replacement identities. Verify stop and crash before response transmission. Verify stale queued responses through HCF permission and cancellation.

Debug mode supplies the primary behavior evidence. Release mode supplies a separate compilation check. Each functional commit must build before its focused tests run. The final checks cover the union of those tests and directly related Wi-Fi protocol cases.

The focused unit selector is `Ieee80211BlockAckInactivity_1.test`. The module selectors cover request cancellation, group lifecycle, Block Ack lifecycle, restart, and retirement. Existing management recovery and RTS timeout tests cover adjacent HCF failure paths. Wi-Fi protocol cases cover DCF DATA/ACK, RTS/CTS, fragmentation, HCF bursts, and Block Ack.

The scope includes stop, crash, restart, zero timeout, both expiry event orders, buffer isolation, and replacement preservation. This change introduces no fingerprint baseline updates. Record exact commands, commit identifiers, results, and any coverage gap in the PR validation record.

## 6. Standards and limits

IEEE Std 802.11-2024, Clause 11.5.4 defines Block Ack inactivity expiry and late-data discard. Table 9-79 defines DELBA reason codes. Agreement handlers own the protocol state. Their deletion callbacks borrow agreements only until return.

External Tx modules, sequence handlers, ACK handlers, agreement handlers, callbacks, and recipient data services require the new interfaces. The migration guide explains each interface change. Custom implementations require source changes and a rebuild.
