# S3b implementation state and verification

> **Kind:** report · **Status:** snapshot 2026-10-21 · **Seal:** none · **Owns:** — · **Stands on:** [S3b summary](ieee80211-compressed-blockack-s3b-summary.md), [lifecycle plan](../pending/ieee80211-blockack-agreement-lifecycle.md)

S3b implements safe Basic Block Ack agreement setup, teardown, and association reset. HCF rejects production Compressed selection during initialization. S4 BAR-only channel access, A5 transfer completion, and S5 Compressed exchanges remain outside this stage.

This record describes checkout `/home/user/omnetpp_ws/inet-compressed-blockack-s3b` at `5860acb229fcb412377b4eb804ecda4df216b015`. The production tree is `016f88787ba21e65bb7fe3c6d52a95ab287705cf`; the test tree is `fdff0d4f47022d97d3f1051385137a8dd7c8275c`. These identifiers describe the current tracked `src/` and `tests/` inputs.

## Implementation contract

[HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) coordinates agreement transitions, Action ownership, timers, DATA eligibility, and cleanup order. The [originator handler](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc) owns originator agreements and setup procedures. The [recipient handler](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc) owns recipient agreements and recipient teardown procedures. HCF reads procedure state and requests transitions through handler operations. The handlers own changes to their stored procedures.

The [recipient data service](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.cc) owns duplicate checks, reassembly, and delivery. The [originator data service](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) selects eligible DATA and assigns sequence numbers after extraction. Queue components own extraction effects. Management selects the old association context; the MIB owns capability and rate state.

### A1: accepted Actions and exchange interruption

The recipient data service returns only complete, nonduplicate management frames. HCF dispatches accepted ADDBA and DELBA Actions once. Duplicate receptions retain immediate ACK behavior. During a response wait, HCF processes the accepted Action before it interrupts the old exchange. A matching response can therefore resolve setup before the old request exchange reports failure.

The [frame-sequence handler](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.cc) owns the interruption outcome. It checks whether the outgoing DATA or management frame remains in `InProgressFrames`. A retired frame completes without obsolete failure feedback; an active frame retains its failure path. The handler destroys the exchange context before it deletes deferred frame copies.

### A2: accepted negotiation parameters

The originator accepts a successful response only when peer, TID, dialog token, immediate policy, and supported parameters match the pending setup. Request acknowledgment alone does not activate the agreement. The recipient copies the request token into its response. Both agreement owners use the response buffer size and timeout.

The built-in recipient policy selects `min(64, maximumAllowedBufferSize)` positions. The request buffer size does not cap that selection. A zero response timeout disables inactivity expiration. A-MSDU use under the agreement requires permission from both request and response flags.

Hypothetical example: the request suggests eight positions and the recipient permits 64. The response advertises 64. Both agreement owners use 64, so their buffer limits agree.

Basic admission requires no HT capability. Isolated policy checks cover Compressed admission with local and directional peer HT state. These checks do not enable production Compressed exchanges. The [HCF configuration](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.ned) retains `blockAckVariant = "basic"` as the default.

### A3: retirement, recovery, and association reset

HCF identifies an agreement by local role, peer, and TID. Received DELBA uses the transmitter address and Initiator field to select the local role. HCF detaches that agreement before DATA cleanup and deletion notification. Ordinary recipient teardown delivers complete buffered units in cyclic order and discards incomplete units. It preserves duplicate state. Association reset instead discards old buffered units.

Ordinary originator teardown discards transmitted, unacknowledged DATA and unsent fragments through the highest transmitted allocation index. The data service assigns this local index before fragmentation. All fragments retain it in [Ieee80211DataSequenceTag](../../src/inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag.msg). The agreement only advances its stored index. Older retransmissions cannot move the retirement boundary backward. Later intact DATA and another peer's DATA remain eligible.

The [fragment fixture](../../tests/module/Ieee80211BlockAckFragmentRetirement_1.test) includes sequence wrap and large allocation gaps. For example, fragment 0 of sequence 4095 transmits before teardown. Teardown removes its unsent siblings but preserves the intact successor with sequence 0. The fixture checks teardown before and after the first fragment receives its Normal Ack.

HCF uses separate ADDBA response and agreement inactivity timers. Request acknowledgment starts the response deadline. The inactivity timer uses the earliest absolute deadline across both agreement roles. HCF retires agreements with deadlines at or before the current time before it calculates the next deadline. A zero timeout supplies no finite deadline.

HCF tracks each generated Action by frame identity, peer, TID, and role. Intentional invalidation removes Action ownership before queue disposal can report failure. An accepted physical exchange retains its context until safe completion. Its callbacks cannot authorize retired DATA after CTS.

After a possible request transmission, setup failure requires DELBA recovery before DATA can resume. A failed originator DELBA discards the deferred batch and leaves recovery-needed state. New DATA triggers another DELBA attempt; no periodic retry timer exists. A failed recipient-role DELBA does not discard opposite-direction DATA. Valid Block Ack-policy DATA or a supported Basic BAR without an agreement triggers recipient-role DELBA. HCF rejects malformed input before that recovery path.

Management calls the typed [IIeee80211Mac interface](../../src/inet/linklayer/ieee80211/IIeee80211Mac.h) before association replacement or loss. Its `resetPeerAssociationContext(peer)` operation delegates through the concrete MAC to `Hcf::resetPeer(peer)`. This is a direct C++ call. It does not depend on a capability-change notification.

HCF detaches both agreement roles and their procedures before deletion callbacks. It invalidates old Actions and discards queued and in-progress QoS DATA for the peer. This disposal includes unsent Normal Ack DATA without an agreement when Block Ack support is active. The recipient data service clears reorder, duplicate, and reassembly state. HCF emits deletion notifications after cleanup and recalculates both timers. Management changes association and MIB state separately. Other peers and accepted physical exchange contexts remain intact.

### A4: setup deferral and queue service

HCF retains an eligible completed Normal Ack transmission as `WAITING_FOR_SUCCESSOR`, without an Action or timer. A later unexpired queued successor promotes that procedure to `SETUP_INTENT`. HCF defers new extraction while already numbered DATA completes or drops. It queries the next sequence number without consumption before it creates ADDBA Request. A lone frame therefore causes no empty setup request.

HCF uses filtered `canPullPacket()` queries to find unexpired DATA for a peer and TID. The originator data service uses the same contract to find eligible DATA. An eligible advertised head uses ordinary dequeue. Eligible DATA behind a deferred head uses filtered `pullPacket()` with an identity filter. Deferred DATA retains its sequence and lifetime state.

[CPacketFilterFunction](../../src/inet/queueing/function/PacketFilterFunction.h) accepts callables with captured values. Custom queues must support filtered queries; bypass of a deferred head also requires filtered pulls. Queue and filter errors propagate without an indexed-removal fallback. The [passive source contract](../../src/inet/queueing/contract/IPassivePacketSource.h) defines ownership, queue residence metadata, and pull effects.

Before A-MSDU construction, the data service checks source identity, uniqueness, membership, eligibility, and permission. An invalid source list causes individual extraction of the selected frame. HCF uses the same eligibility conditions for contention. First-DATA disposal retires its agreement before a successor can use it.

## Current verification

The source review confirms the production paths described above at the recorded revision. The debug build and all five selected protocol fixtures pass on 21 October 2026. All nine simulation runs report PASS. Both commands exit with status zero.

The focused protocol selection contains five files and nine simulation runs. All selected fixtures use configuration `General`, debug mode, and seed zero. These fixtures require PASS results; none declares an expected failure.

| Fixture guide | Runs | Required observation |
| --- | ---: | --- |
| [Duplicate ADDBA](../../doc/project/evidence/model/wifi/tests/blockack-duplicate-addba.md) | 1 | One dispatch for an original request and retry; both request ACKs complete. |
| [Dialog token](../../doc/project/evidence/model/wifi/tests/blockack-dialog-token.md) | 3 | Token echo, rejection of an altered token, and successful matching-token setup. |
| [BAR without agreement](../../doc/project/evidence/model/wifi/tests/blockack-bar-without-agreement.md) | 2 | Recipient-role DELBA for an orphan BAR; Basic Block Ack for an established agreement. |
| [Inactivity](../../doc/project/evidence/model/wifi/tests/blockack-inactivity.md) | 2 | Retirement at the absolute deadline; no expiration for timeout zero. |
| [Same-AP reassociation](../../doc/project/evidence/model/wifi/tests/blockack-same-ap-reassociation.md) | 1 | Four old roles disappear with equal peer parameters; fresh setup restores four roles and delivery. |

The build log is `/tmp/s3b-plan-current-build-debug.log`. The protocol output is `/tmp/s3b-plan-current-protocol.log`; its command log is `/tmp/s3b-plan-current-protocol-runner.log`. Generated per-run output resides in `tests/protocol/wifi/work/WifiBlockAck*/test.out` for the five selected fixtures. These local artifacts are outside Git. No test input, production source, or expected result changes for this verification.

The commands run from the checkout root after `source .envrc`:

```sh
make MODE=debug -j12
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAck(DuplicateAddba|DialogToken|BarWithoutAgreement|Inactivity|SameApReassociation)\.test$' \
  --no-concurrent --log-file /tmp/s3b-plan-current-protocol-runner.log
```

## Verification limits

The five protocol fixtures cover selected lifecycle conditions in the Basic compatibility profile. Their guides define the controlled peers, stimulus, assertions, and standards limits. The inactivity fixture covers one recipient deadline without DATA refresh. The token fixture includes a deliberately altered response. These observations cannot establish complete standards conformance or production Compressed support.

Module fixtures cover additional boundaries, but their presence alone does not establish an executed result. These include [Action disposal](../../tests/module/Ieee80211BlockAckActionDisposal_1.test), [DATA selection](../../tests/module/Ieee80211BlockAckDataSelection_1.test), and [fragment retirement](../../tests/module/Ieee80211BlockAckFragmentRetirement_1.test). The [filtered pull fixture](../../tests/module/PacketFilteredPullEffects_1.test) checks queue effects. This verification does not run those fixtures, fingerprints, release behavioral tests, sanitizers, or other platforms. CI and general MAC stop/start recovery remain outside this record.

The [migration guide](../../doc/src/migration-guide/index.rst) defines custom-module requirements. S4 and S5 require their own production evidence before any claim of final-transfer or Compressed support.
