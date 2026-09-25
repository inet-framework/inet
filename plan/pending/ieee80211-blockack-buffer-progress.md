# Block Ack receive-buffer progress

Status: implementation in progress.
Date: 2026-09-25.
Base: `08791304f2`.

The user authorized this follow-up to the completed TXOP and Block Ack plan.
The [original observation](../../audit/pull-request/branch-96008e2c7c-implementation.md) describes an acknowledged frame that remains in the receive buffer.
A Block Ack Request (BAR) can name a frame that the recipient already delivered after retransmission.
The current scan stops at that absent frame and leaves later complete frames in the buffer.

## Validated implementation contract

- Invariant and owner: `BlockAckReordering` must resume a BAR scan at the later of its start and `NextExpectedSequenceNumber`.
  The scan must stop at an actual missing or incomplete frame.
  `ReceiveBuffer` owns stored packets and the next expected sequence number.
- Entry and control path: HCF passes a Basic BAR to `RecipientQosMacDataService::controlFrameReceived()`.
  That method locates the agreement by transmitter and traffic identifier before it calls the reorder owner.
  The returned fragments pass through reassembly and MAC delivery.
- Affected consumers: `BlockAckReordering.h`, `BlockAckReordering.cc`, and `RecipientQosMacDataService.cc`.
  The result must preserve cyclic delivery order across 4095 to 0.
  The existing result map uses integer order and can delete a returned packet during buffer release across this boundary.
  An ordered vector of sequence/fragment pairs will preserve the collection order through both release and delivery.
  The receive-buffer map remains an indexed store.
- Sibling and terminal paths: data reception retains its current immediate-delivery rule.
  BAR collection retains preceding complete frames and stops at incomplete following frames.
  Duplicate and stale requests cannot deliver packets twice or move the expected sequence backward.
  DELBA and destruction retain their existing buffer cleanup.
  Peer and traffic-identifier buffers remain independent.
- Boundaries and units: use the existing 12-bit cyclic sequence type, within the negotiated receive window.
  Fragment completeness remains the existing `isComplete()` contract.
  Every returned packet leaves buffer ownership once before the consumer uses it.
  No wire format, serializer, timing unit, initialization stage, or NED parameter changes.
- Verification: add `Ieee80211BlockAckReordering_1.test` for the direct buffer contract.
  Extend `Ieee80211BlockAckLoss_1.test` through actual queues, radios, BAR dispatch, and delivery.
  The fixed seed is 0; finite cases vary loss, sequence wrap, fragments, and BAR position.
  Existing fixtures do not cover the reported two-frame loss; the new cases must fail before production edits.

The source paths are unsealed under the current registry.
The C++, simulator, packet ownership, and IEEE 802.11 preventive references apply.
The related result-order correction is required because the resumed scan can return multiple frames across sequence wrap.
No further reorder policy or agreement-lifetime change belongs to this patch.

## Standards scope

The local IEEE 802.11-2024 corpus is current but omits `NextExpectedSequenceNumber` and the former legacy receive-buffer procedure.
The existing model cites IEEE 802.11-2012, 9.21.4, for the Basic Block Ack receive buffer.
The [2012 standard](https://mrncciew.com/wp-content/uploads/2014/10/ieee-802-11-2012.pdf), printed page 908, describes ordered delivery and the next expected sequence state.
This fix treats delivery before that state as complete when a later BAR repeats an older start.
That progress rule is an explicit model interpretation; it does not introduce a modern compressed Block Ack algorithm.
The web index exposed the clause; full PDF retrieval through the web tool exceeded its size limit.

## Verification sequence

1. Run the new unit and radio cases against the unchanged debug library.
2. Confirm the failure occurs at the buffered-frame delivery assertion.
3. Apply the source fix and documentation changes.
4. Build with `make MODE=debug -j8`.
5. Run the focused unit and module fixtures in debug mode.
6. Run the five existing TXOP protocol fixtures.
7. Run the five scoped Block Ack and TXOP fingerprint cases.
8. Explain any changed fingerprint before a baseline update.
9. Complete the self-audit and scoped project checks.
10. Commit the fix with its direct tests and approved baselines, if any.

Commands run from the repository root, except the fingerprint command:

```sh
inet_run_unit_tests -m debug -f 'Ieee80211BlockAck(Reordering|Record)_1.test'
inet_run_module_tests -m debug -f 'Ieee80211BlockAck(Loss|Inactivity)_1.test'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/11n/N_Txop(BlockAck|Boundary|Burst|FragmentRetry|Fragments)\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-naming.sh --base 08791304f2 src/inet/linklayer/ieee80211
doc/project/enforcement/check-source-seals.sh --base 08791304f2
```

Run this command from `tests/fingerprint`:

```sh
./fingerprinttest -d -t 4 -m 'showcases/wireless/(blockack|txop)/|examples/wireless/qos/.*MacQosWithBlockAck' -f tplx -f '~tNl' -f '~tND'
```

The previous baseline approval covered B4 only. Any new baseline change requires its own exact approval.

## Reproduction before production edits

The new unit fixture fails when an older BAR must release two buffered frames.
The radio fixture passes scenarios 0–11 and fails the delivery count in scenario 12.
That scenario uses a 2 ms transmission opportunity and loses the first of two Block Ack data frames.
The sender queue is empty before the failed delivery assertion.
Both filtered commands exit 1 against the unchanged debug library.
The logs and assertion output reside under `/tmp/blockack-buffer-*-before.*`.
