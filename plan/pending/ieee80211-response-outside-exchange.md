# Implementation plan: an 802.11 response outside its frame exchange

Checkout and revision: `master` at `76736836de`; the work is on `topic/ieee80211-response-outside-exchange`.
Plan revision: 2026-10-22, first draft.
Approval: approved by the user on 2026-10-22 ("yes" to implementing all three forms in a worktree), for the first draft. Before that, the user asked to see the repair of the finding "Dcf stops on an unexpected ACK" of `plan/done/protocol-test-duplicate-and-corrupt-frames.md`.

## Problem and evidence

A station that gets a response frame while no frame exchange of its own waits for one stops the simulation. `Dcf::recipientProcessReceivedControlFrame` handles an RTS and throws "Unknown control frame" for every other control frame ([Dcf.cc:253-259](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.cc#L253-L259)). `Hcf::recipientProcessReceivedControlFrame` ignores a late Ack since 2017 or earlier, with the comment "it is an ACK frame that is received after the ACKTimeout", but it throws for a late CTS and a late BlockAck ([Hcf.cc:1020-1032](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L1020-L1032)).

The path: `processLowerFrame` sends a frame that is addressed to the station to the recipient side when no frame sequence runs ([Dcf.cc:174-175](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.cc#L174-L175)), and the recipient side sends a control frame to `recipientProcessReceivedControlFrame` ([Dcf.cc:240-244](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.cc#L240-L244), [Hcf.cc:1013-1016](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L1013-L1016)).

A response arrives outside its exchange in two ways:

- It starts after its timeout. The AckTimeout and the CTSTimeout are aSIFSTime + aSlotTime + aRxPHYStartDelay. When the propagation delay to the peer is larger than the margin of the slot, the response comes after the station has given up, and the station is then in its backoff with no sequence running.
- A frame repeats on the air. The peer answers each copy. `WifiDuplicateWithRetry.test` showed it: without its extra rule, the run stops at t=1.001426991758 s in `sta1.wlan[0].mac.dcf` with "Unknown control frame".

IEEE 802.11-2024 defines no effect for such a frame. 10.3.2.11 (Acknowledgment procedure): an Ack is a successful acknowledgment only when its reception starts within the AckTimeout interval after the MPDU. 10.3.2.9 (CTS and DMG CTS procedure): the same for a CTS within the CTSTimeout interval after an RTS; "anything else ... shall be interpreted as failure of the RTS frame transmission". A response that no exchange waits for therefore has no effect, and the station ignores it.

## Change and preserved behavior

`Dcf::recipientProcessReceivedControlFrame` ignores an Ack and a CTS, with a warning in the log, as `Hcf` does for an Ack. `Hcf::recipientProcessReceivedControlFrame` ignores a CTS and a BlockAck in the same branch as its Ack. The caller deletes the frame, as it does now. Nothing else changes: no signal, no counter, no change to the NAV or to the contention. A control frame of another kind still throws, because it is not a response, and its case is outside this plan.

A response that arrives while the station waits for another response goes to `processResponse` of the frame sequence, not to this function. That path does not change.

No fingerprint and no statistical result can move: a scenario that reached the throw stopped with an error, so no recorded run contains such a frame.

The commit is `ieee80211: fix: a response outside its frame exchange is ignored`, with a release note file in `doc/whatsnew/fixes/`. Its trailer is `Change: src.ieee80211 | behavior.change.fix | test whatsnew`.

## Verification

Tests in `tests/protocol/wifi/common/`:

- `WifiDuplicateWithRetry.test` and `WifiDuplicateWithoutRetry.test` lose their rule that keeps the ACK of the copy away from sta1, and the paragraph that explains it. Before the repair they stop with "Unknown control frame"; after it they pass. This covers the Ack in `Dcf`.
- New `WifiLateCts.test`, 802.11g with RTS/CTS for every frame (`rtsPolicy.rtsThreshold` below the frame size): the radio medium sends a copy of the CTS of the AP 1 ms after the original, when sta1 has finished its exchange. sta1 must receive the copy and go on: sta2 gets the datagram one time. This covers the CTS in `Dcf`.
- New `WifiLateCtsQos.test`, the same with QoS stations (802.11n). This covers the CTS in `Hcf`.
- New `WifiLateBlockAck.test`, 802.11n with a Block Ack agreement: the radio medium sends a copy of the BlockAck of the AP 1 ms after the original, after the application of sta1 has stopped, so that no exchange runs at sta1. sta1 must go on. This covers the BlockAck in `Hcf`.

Each new test fails before the repair with "Unknown control frame" and passes after it. Then the Wi-Fi suite and the fingerprint tests run.

The guide (`tests/protocol/lib/AUTHORING.md`, section 7) loses its sentence about the open problem with the ACK of a copy.

## Result (2026-10-22)

Done in `ieee80211: fix: a response outside its frame exchange is ignored`, as designed.

- Before the repair, each of the five tests stopped with "Unknown control frame": the two duplicate tests and `WifiLateCts` in `Dcf`, `WifiLateCtsQos` and `WifiLateBlockAck` in `Hcf`. After it, each passes.
- The timing of the copies needed care, because a copy that overlaps another frame at sta1 collides and is never received. In `WifiLateCts` and `WifiLateCtsQos` the copy comes 2 ms after the CTS: at 1 ms, the AP still forwards the datagram to sta2 with RTS/CTS, and the copy collides with the ACK of sta2. In `WifiLateBlockAck`, sta1 sends a burst of 2 ms (a shorter burst ends before the agreement exists), sta1 sends its last frame at about 1.007 s, and the air is busy until about 1.055 s, so the copy comes 60 ms after the first BlockAck of the AP.
- The Wi-Fi suite has 87 tests: on `master`, 52 PASS, 30 expected FAIL and the 5 tests of this plan as unexpected FAIL; with the repair, 57 PASS and 30 expected FAIL. No other verdict changes.
- The 753 local fingerprint runs, 178 of them wireless, give the same computed fingerprints on `master` and with the repair.
- `check-pull-request.sh`, `check-source-seals.sh`, and the architecture and naming checks of `coordinationfunction` pass.
