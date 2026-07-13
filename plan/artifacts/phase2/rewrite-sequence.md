# Phase 2 rewrite — final sequence (authoritative)

Synthesized from `split-map.md` (agent A) + `fix-placement-map.md` (agent B) +
opus review. Pre-rewrite tip: `4861313719` = `topic/infrastructure-backup-20260714`
= `origin/topic/infrastructure`. Base: `47adf296fb`.

## Review deltas vs the agent maps (with reasons)

1. **todo1-F merged into todo1-E** (not fixup → 3e5bf3c8e2): `Tcp::setCallback`
   is born in `4c5522a4b0` (verified `git log -S`), so a `tcp->setCallback()`
   call site folded into `3e5bf3c8e2` would not compile for 12 commits.
   Compile-safety overrides line-blame.
2. **todo1-E is STANDALONE at todo1's position** (not fixup → 4c5522a4b0), and
   absorbs `fd329a752c` + `a8ce3fe580` whole (no 3–4-way split of a8ce3fe580):
   keeps 4c5522a4b0 from becoming a grab-bag, gives the SCTP accept-path fixes
   their natural home, and every hunk is compile-safe there (`Sctp::setCallback`
   from 4c5522a4b0 < todo1; `isToBeAccepted()` pre-branch, verified).
3. **Items 11–14 (f4f8c8f3b2, e409faa29f+cd8dfaa2ee, e441f2ef9d, 24d2d98e9d)
   stay STANDALONE in place** (agent B's client/server 2-way splits rejected):
   client hunks folded into `5a5ac65eea` would call server methods that only
   exist 6 commits later — non-compiling intermediate history. They are
   coherent per-protocol completion commits, in the branch's own idiom.
   Only change: `cd8dfaa2ee` fixup-C into adjacent `e409faa29f` (one Tcp commit).
4. **62d30b2a6f is SPLIT 2-way** (new, beyond the maps): coroutine machinery
   (CoroutineEventExecution.*, FunctionalEvent.cc, SimulationContinuation.*)
   vs 802.11 mgmt lookup fix (Ieee80211Mac.*, 6 mgmt .ned). Verified: the
   802.11 hunk is a plain pushPacket override, no coroutine reference. This
   makes the Phase-5 D9 strip decision a single-commit drop. todo6 folds into
   the coroutine piece → machinery born disabled (D9).
5. **4c5522a4b0 gets an honest reword** (message-only): its actual bulk is the
   server-side socketId method sweep + protocol-side callback fields (agent B
   headline finding), not Enter_Method/take.
6. Message hygiene rewords while we're here: `7f2fccbdff`, `f2aecee10e`,
   `ad606a05bb` (prefix only, content untouched).

## Pass A — in-place splits & rewords (no reordering; tree identical after)

Edit stops in series order (match by subject at the stop):

| Stop (orig SHA) | Action |
|---|---|
| `4c5522a4b0` | reword → msg-4c5522a4b0.txt |
| `85415fbf50` todo1 | split 9 pieces → spec-todo1.json |
| `7f2fccbdff` | reword → "all: Removed obsolete IProtocolRegistrationListener includes and registerProtocol/registerService calls." |
| `f2aecee10e` | reword → "EchoProtocol: Fixed missing initialize() declaration." |
| `ad606a05bb` | reword → "Sctp: Removed obsolete streamThroughputVectors code replaced by signal-based recording." |
| `d587f79440` todo2 | reword → msg-todo2.txt |
| `62d30b2a6f` | split 2 pieces → spec-62d.json |
| `f57702acd4` todo3 | split 2 pieces → spec-todo3.json |
| `8dee6f83f6` todo8 | split 3 pieces → spec-todo8.json |
| `8360ab4d9a` | split 2 pieces → spec-8360.json |

todo1 pieces (all whole-file; 25 files accounted):
- **todo1-A** standalone: NetPerfMeter.cc/.h, SctpServer.cc/.h — "NetPerfMeter, SctpServer: Converted socket communication to use callback interfaces and direct method calls."
- **todo1-B** fixup→3881df4100: PingApp.cc, UdpBasicApp.cc/.h, TunLoopbackApp.cc
- **todo1-C** standalone: Ieee80211Mac.cc/.h, Ieee80211MgmtSta.cc — "Ieee80211Mac, Ieee80211MgmtSta: Converted radio configuration commands to direct method calls."
- **todo1-D** fixup→8d539f3eac: BridgingLayer.cc
- **todo1-E** standalone (incl. old F): Ieee8022Llc.cc, Sctp.cc, SctpAssociationBase.cc, SctpAssociationUtil.cc, TcpConnectionEventProc.cc, TcpConnectionUtil.cc, contract/tcp/TcpSocket.h — "Ieee8022Llc, Sctp, Tcp: Converted the remaining protocol-side indication delivery to socket callback interfaces."
- **todo1-G** fixup→9fea7b31ba: NetworkInterface.cc
- **todo1-H** standalone: BgpRouter.cc/.h — "Bgpv4: Fixed established and available indication delivery to use the accepted socket's id."
- **todo1-I** standalone: Ipv4NodeConfigurator.cc, Ipv4RoutingTable.cc — "Ipv4: Guarded against missing Ipv4InterfaceData during interface configuration."
- **todo1-J** standalone: EthernetMacPhy.cc — "EthernetMacPhy: Skipped decapsulation for frames received with a bit error."

62d30b2a6f pieces:
- **62d-coro** standalone: CoroutineEventExecution.cc/.h, FunctionalEvent.cc, SimulationContinuation.cc/.h — "common: Added coroutine based event execution for the simulation continuation machinery." (body: disabled by default; receives todo6 → born disabled)
- **62d-mac** standalone: Ieee80211Mac.cc/.h/.ned, IIeee80211Mgmt.ned, Ieee80211MgmtAdhoc/Ap/ApSimplified/Sta/StaSimplified.ned — "Ieee80211Mac: Fixed the module interface lookup for management frames."

todo3 pieces: **todo3-a** fixup→08fd7785a4: MrpRelay.cc; **todo3-b** standalone:
the other 13 files — "Icmp, Icmpv6, Ipv4, Ipv6, Udp, Tcp: Replaced ICMP error
indication messages between network and transport layers with direct method calls."

todo8 pieces: **todo8-a** fixup→3881df4100 (chain after todo1-B): TunLoopbackApp.cc;
**todo8-b** fixup→todo1-J: EthernetMacPhy.cc; **todo8-c** standalone:
InterpacketGapInserter.cc — "InterpacketGapInserter: Documented timing edge cases
around zero-duration packets and signal ordering."

8360ab4d9a pieces: **8360-a** fixup→08fd7785a4 (after todo4): Ieee8022Llc.cc;
**8360-b** fixup→72b2d288bc: EthernetLayer.ned.

## Pass B — reorder + fixup + drop (single scripted rebase)

Fixup chains (chain order = original series order; all others stay in place):

| Target | Fixups (in order) |
|---|---|
| `2780ce0546` | 06df44b377 |
| `e0c99ee8b0` | 1dd0250e64 |
| `72b2d288bc` | todo5(c812c9e636), 8360-b, 8d111516b7 |
| `08fd7785a4` | todo3-a, todo4(8688c99866), 8360-a |
| `8d539f3eac` | todo1-D, 2f6c9f10cc |
| `3e5bf3c8e2` | 992b426b67 |
| `3881df4100` | todo1-B, todo8-a |
| `9fea7b31ba` | todo1-G |
| `62d-coro` | todo6(238566c312) |
| `todo1-E` | fd329a752c, a8ce3fe580 |
| `todo1-J` | todo8-b |
| `todo3-b` | todo7(bc26b8f6de), 075993e404, c03c0609db |
| `519027f347` | e6fba31362 |
| `3a9ec97538` | 57458b84b2 |
| `75f560badc` | 1da6a175e3 |
| `e409faa29f` | cd8dfaa2ee (fixup -C: keep cd8dfaa2ee's message) |
| plan commit (moved to tip) | 4861313719 + any later plan commits |

Drop: `7c0ff8e19f` (todo9, `__TODO` only).

Stay in place (standalone, no action): everything else, incl. 2737d114b6
(omnetpp-6.4 response, nothing in-branch to fix up into — agent B #4),
bde8051d8b (verbatim master goldens, rebase pre-resolution — must NOT squash),
5ae305f7ab, fe75bef16a (D1 decision commit, kept visible), 970383a539,
38e737a313, f4f8c8f3b2, e441f2ef9d, 24d2d98e9d, 65da1fc7a1.

## Verification (2.3)

1. `git diff topic/infrastructure-backup-20260714..HEAD` → exactly one delta:
   `__TODO` deleted. (Pass A midpoint: diff must be EMPTY.)
2. Commit count = 140 − squashed + pieces (computed at execution).
3. runtests.sh BUILD=1 + full module suite = 272/272.
4. Deferred to Phase 5 (content changes forbidden in Phase 2): scratch
   "…: PASS" comments from todo8 hunks (TunLoopbackApp.cc, EthernetMacPhy.cc);
   stale IEthernet.h include in BridgingLayer.cc left by fe75bef16a.
