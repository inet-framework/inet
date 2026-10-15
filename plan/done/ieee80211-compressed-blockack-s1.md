# S1: repair Block Ack recipient capacity and delivery

> **Kind:** plan · **Status:** snapshot 2026-10-14 · **Seal:** none · **Owns:** — · **Stands on:** [contribution procedure](../../doc/project/guide/contribute-a-change.md), [test rules](../../doc/project/rule/testing.md)

S1 repairs recipient capacity and delivery through the retained Basic Block Ack path. The maintainer selected this stage from the Compressed Block Ack plan, revision 23. The implementation request authorizes S1. The later request authorizes the commit, push, and upstream PR. The maintainer's 14 October 2026 instruction, "Baseline updates approved", authorizes the four fingerprint updates below.

The implementation source parent is `c15860de93b3764c3e4ead69c6933fc5ae9ce957`. The publication parent is upstream `master` at `ac6344cc5bcac972012759281a11b3f8255b5d46`. That parent contains the merged VHT/HT prerequisite. Its IEEE 802.11 sources, release notes, migration guide, and selected fingerprint baselines equal the implementation parent's versions. Publication verification uses the final tree on that upstream parent.

The parent counts fragments as separate buffer positions. Two Basic fragments therefore cannot complete in a one-position buffer. DATA that closes a gap releases only one complete unit. Numeric map order reverses release across sequence wrap. A BAR before DATA creates no reorder record.

The repair counts sequence positions and returns released units in cyclic order. For example, buffered DATA 101 and 102 wait behind absent DATA 100. Receipt of DATA 100 releases all three units in the same operation. A BAR at 104 can also create an empty window from an accepted agreement that starts at 100. Subsequent DATA 104 is immediately eligible for delivery.

The three existing owners retain their responsibilities:

- `ReceiveBuffer` owns indexed fragment storage. All Basic fragments of one MSDU occupy one sequence position. Duplicate fragments do not consume another position.
- `BlockAckReordering` owns window movement and complete-unit release. Its ordered result preserves sequence wrap. Window movement releases older complete units and discards incomplete units outside the new window.
- `RecipientQosMacDataService` preserves released order through defragmentation, A-MSDU deaggregation, and delivery. It supplies the accepted agreement to the reorder owner.

The reorder owner creates a record on the first valid related DATA or BAR. It takes the initial window and capacity from the accepted agreement. Malformed Basic BAR fields, unsupported BAR input, and BAR without an agreement create no record. Older, equal, and half-space BAR sequence numbers leave an existing window unchanged.

The DATA and BAR window rules follow IEEE Std 802.11-2024, Clauses 10.25.6.6.2.1 and 10.25.6.6.3.1. Basic retention remains an INET compatibility choice. It supplies fragment reassembly without a current-standard conformance claim. S1 adds no Compressed exchange, lifetime, lifecycle, or rate-publication capability.

`Ieee80211BlockAckReordering_1.test` checks eight scenarios through the production recipient service with supplied accepted agreements. These scenarios cover fragment capacity, gap release, sequence wrap, BAR before DATA, window movement, incomplete-unit disposal, peer/TID isolation, and A-MSDU order. All eight scenarios fail on the implementation parent. All eight pass after the repair. `Ieee80211RecipientNav_1.test` supplies three existing recipient regressions, including a Basic exchange.

The initial debug fingerprint comparison selects 12 legacy configurations. All 12 pass on the implementation parent. Before the approved baseline update, eight pass after the repair. Four differ because DATA gap release now delivers additional complete units immediately:

| Configuration, run 0 | First changed recipient operation | Approved fingerprints |
| --- | --- | --- |
| `showcases/wireless/blockack:NoFragmentation` | DATA 14 releases 14–16 at 0.02971051219 s. The parent releases only 14. | `1082-899c/tplx;0068-9ae9/~tNl;e0b3-3193/~tND` |
| `showcases/wireless/blockack:MixedTraffic` | DATA 119 releases 119–122 at 0.33845991931 s. The parent releases only 119. | `8bfb-6ac9/tplx;5647-2cd1/~tNl;481a-ec9d/~tND` |
| `showcases/wireless/fragmentation:HCFfragblockack` | Completion of unit 147 also releases unit 148 at 0.446287872592 s. The parent releases only 147. | `4228-9743/tplx;8610-abcd/~tNl;68c2-19f0/~tND` |
| `examples/wireless/qos:MacQosWithBlockAck` | Unit 176 releases its four MSDUs and unit 177's two MSDUs at 1.518610305244 s. The parent releases four MSDUs. | `8dc8-a4a7/tplx;0826-0ca5/~tNl;0b4c-d9a2/~tND` |

The implementation commit includes these four approved fingerprint rows. The update preserves the other ingredients and configurations. Publication verification repeats the selected regressions against the final library. The commit also contains the direct fixture, release note, and migration instructions for external C++ callers of `ReleasedFrames`.
