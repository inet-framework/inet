# todoN squash map — summary (reconstructed)

Reconstructed from the Phase 0.4 agent's final report after the scratchpad copy
(358-line `squash-map.md` with per-hunk blame evidence) was lost. The per-hunk
evidence must be re-derived during Phase 2 execution where needed (esp. todo1).

| todo | SHA | Target squash | Confidence | Notes |
|------|-----|---------------|------------|-------|
| todo1 | 85415fbf50 | SPLIT across ~6 commits | **low** | 25 files, 6 distinct logical groups — requires manual `git add -p` split; per-hunk mapping lost, re-derive in Phase 2 |
| todo2 | d587f79440 | Standalone (new feature) | med | `yieldBeforePush()` mechanism (85-file sweep); best standalone after all pushPacket commits. **D9-coupled** |
| todo3 | f57702acd4 | SPLIT: MrpRelay → `62e378249c`; ICMP refactor → standalone or `2c8e8ceb18` | med | Tcp.cc/h handled by todo7 squash |
| todo4 | 8688c99866 | `08fd7785a4` (Added PassivePacketSinkRef fields) | **high** | Fixes wrong dispatch tag in Ieee8022Llc |
| todo5 | c812c9e636 | `72b2d288bc` (Added @interface properties to input gates) | **high** | Mrp.ned relayIn missed from sweep |
| todo6 | 238566c312 | `62d30b2a6f` (Fixed 802.11 module interface lookup) | **high** | Comments out `setEventExecutor` calls. **D9-coupled**; blame verified: `62d30b2a6f` is a mixed commit (802.11 fix + coroutine toggle) — if D9 = keep-dormant, introduce machinery already-disabled instead of enable→disable churn |
| todo7 | bc26b8f6de | Squash into todo3 (`f57702acd4`) | **high** | NOT a full cancel: Tcp.cc stubs removed (net zero), but Tcp.h declarations move protected→public. Net effect: `processIcmpv4Error`/`processIcmpv6Error` public, no stub bodies — Icmp calls Tcp directly |
| todo8 | 8dee6f83f6 | SPLIT: TunLoopbackApp → todo1 group; EthernetMacPhy → standalone (comment-only); InterpacketGapInserter → standalone | med | Comment/TODO annotations |
| todo9 | 7c0ff8e19f | **DROP** | **high** | Only adds `__TODO` file, no code |

Proposed rebase mechanics: scripted `GIT_SEQUENCE_EDITOR` todo-list; place
`fixup bc26b8f6de` immediately after `pick f57702acd4`.
