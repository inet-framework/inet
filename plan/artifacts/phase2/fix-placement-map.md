# Fix-commit placement map (Phase 2 history rewrite)

Scope: the batch of fix/completion commits at the tip of `topic/infrastructure`
(merge-base `47adf296fb`, tip `4861313719`). For each, this determines whether it
should be squashed (fixup) into the earlier branch commit that introduced the
defect/incompleteness it addresses, or kept standalone. Method: `git show <sha>`
for the fix, then `git blame -w <sha>^ -- <file>` (occasionally `git log -L`) on
the modified lines to find the true introducer, cross-checked against topological
commit order (`git log --oneline | cat -n`), since author dates in this branch are
**not** reliable for ordering (many commits carry a uniform `2026-04-30`
committer-date stamp from an earlier rewrite pass, while author dates can even
run "backwards" relative to topological order).

## Headline correction: two sweep commits, not one

Several hypotheses assumed a single earlier "sweep" commit converted socket
classes to direct method calls. In fact **two complementary commits** did this,
and any later "completed the conversion" fix that adds a still-missing
method necessarily splits across both:

- **`5a5ac65eea`** "all: Replaced sending commands in socket classes with direct
  method calls to protocol modules." — **client side**: `XSocket.cc` call sites
  (`tcp->abort(connId)`, `udp->bind(...)`, `sctp->shutdown(...)`, `tun->open(...)`).
- **`4c5522a4b0`** "all: Added missing Enter_Method() and take() calls into
  pushPacket implementations." — **mislabeled**: its actual, much larger content
  (Tcp.cc/h, Sctp.cc/h, Udp.cc/h, Tun.cc/h, Ipv4.cc/h, Ipv6.cc/h, Ieee8022Llc.cc/h,
  BridgingLayer-adjacent files, ~30 files/536 lines) is the **server side**:
  the `X::method(int socketId, ...)` handler implementations the sockets call
  into. Confirmed by blame: `Tcp::abort/close/accept/setTimeToLive`,
  `Sctp::receive/streamReset/getSocketOptions/setCallback`,
  `Udp::setBroadcast/setMulticastLoop/joinMulticastGroups`, `Tun::open` are all
  first-introduced there — topologically **after** `5a5ac65eea` (Oct 4 → Oct 9
  in author-date terms, and confirmed by `git merge-base --is-ancestor`).

Any fix commit that completes the "socket command conversion" for a protocol
by adding a still-missing method pair (client call site + server handler) is a
**two-way split candidate**: the client-side hunk parallels `5a5ac65eea`, the
server-side hunk parallels `4c5522a4b0`. This affects items 2, 5, 11, 12, 13, 14
below.

## Table

| # | Fix SHA — subject | Target(s) — subject | Confidence | Evidence (blame SHAs) | Notes |
|---|---|---|---|---|---|
| 1 | `075993e404` Udp: Fixed ICMP module lookup to use the concrete module interface. | **`f57702acd4d`** (todo3) — ICMP-refactor sub-hunk | High | `git blame -w 075993e404^ -- src/inet/transportlayer/udp/Udp.cc` → all touched lines (`DispatchProtocolReq`+`findModuleInterface(...,IPassivePacketSink,...)` for icmpv4/icmpv6Module) are `f57702acd4d`, not any of 5a5ac65eea/d355db266b/08fd7785a4. | **Hypothesis wrong.** todo3 (`f57702acd4d`, message just "todo3") is the actual introducer — it rewrote Udp's ICMP lookup *and* Ipv4.cc/Ipv6.cc's ICMP-error dispatch (see #9) *and* MrpRelay.cc in one commit. Phase 0 already flagged todo3 as SPLIT: MrpRelay hunk → `62e378249c`; ICMP-refactor hunk → "standalone or `2c8e8ceb18`" (undecided). Items #1 and #9 are both fixups of that **same still-undecided ICMP-refactor group**, not of 5a5ac65eea/d355db266b/08fd7785a4. Icmp.ned/Icmpv6.ned `@interface[inet::Icmp]`/`[inet::Icmpv6]` additions travel with this same group (see rationale under #9 — they express a lookup style todo3 invented, so squashing them into the older `72b2d288bc` sweep would be anachronistic). |
| 2 | `8360ab4d9a` Ieee8022Llc: Fixed lower layer sink lookup to work over non-Ethernet MACs. | **SPLIT**: `Ieee8022Llc.cc` → **`8688c99866`** (todo4) → (already mapped by Phase 0 to fixup into **`08fd7785a4`**); `EthernetLayer.ned` (`pdu=ieee8022llc` attribute) → **`72b2d288bc`** | High | Ieee8022Llc.cc: `git blame -w 8360ab4d9a^ --` → the buggy `DispatchProtocolReq(ethernetMac)`-based `lowerLayerSink.reference()` is `8688c998662` (todo4), confirming Phase 0's squash-map entry ("todo4 → 08fd7785a4, high confidence"). EthernetLayer.ned: `upperLayerIn`'s `@interface[...](protocol=ethernetmac;...)` line (missing `pdu=`) is `72b2d288bcf`. | **Split confirmed** — two files, two generations. `pdu=` is an attribute of the annotation scheme `72b2d288bc` itself established (used on sibling gates in the same commit), so its omission here is that commit's own gap — same-generation retrofit, unlike #1/#9. |
| 3 | `8d111516b7` Ipv6: Added missing IPassivePacketSink gate interface declarations. | **`72b2d288bc`** | High | `git blame -w 8d111516b7^ -- src/inet/networklayer/ipv6/Ipv6.ned`: the sibling `transportIn`/`queueIn` gates on the same NED type got `@interface[...]` from `72b2d288bcf`; the four bare gates (`ndIn`, `upperTunnelingIn`, `lowerTunnelingIn`, `xMIPv6In`) are unchanged since 2008/2011 (`24a78f72f49`, `2869d59976f`) — i.e. `72b2d288bc`'s sweep walked past them without adding the same annotation. | Hypothesis confirmed as stated. Pure sweep-completion, no split. |
| 4 | `2737d114b6` Tests: Followed method call logging changes in queueing gate tests. | **STANDALONE** | High | Diff's own comment: `%# remove method call lines added in OMNeT++ 6.4`. `git log --oneline 47adf296fb..4861313719 -- src/inet/queueing/gate/PacketGate.cc src/inet/queueing/base/PacketGateBase.cc` → **empty** (no branch commit ever touched the gate module). `git blame` on `PacketFlowBase::pushPacketStart/pushPacketEnd`'s `EV_INFO << "Starting/Ending packet streaming"` lines → `990ae562282`/`bd9d824b65b` (2023, pre-merge-base). | **Hypothesis wrong.** Neither `5d5c8e68c9` nor `87923eb166` (both about packet *animation*, unrelated to EV logging) is the cause. This is purely a response to an **OMNeT++ kernel version bump to 6.4** (companion-repo behavior: `Enter_Method()` now emits an extra "Method call: ..." log line) — not any topic/infrastructure branch commit. Confirmed by the branch's own Phase-1 log (`plan/pending/complete-topic-infrastructure.md` line 233: "%subst for method-call lines, per existing convention"). Must stay standalone; there is nothing in-branch to fix up into. |
| 5 | `fd329a752c` Sctp: Fixed available indication delivery... + `a8ce3fe580` Sctp: Fixed callback delivery for accepted server side associations. | **SPLIT, 3–4 targets** (see notes) | High | See per-hunk breakdown below. | **Hypothesis wrong** (targets were guessed as 1fbca7833e/258db73048). They **partially share one target**: `85415fbf50` (todo1), but only for the `SctpAssociationUtil.cc` hunk. |
| 6 | `e6fba31362` Tests: Made protocol IDs ... robust against renumbering. | **`519027f347`** | High | `git show --stat 519027f347` created all 9 files `e6fba31362` touches, as brand-new files (all `ModuleInterfaceLookup_*.test`); no other commit in range touches them. | Hypothesis confirmed exactly. No split. |
| 7 | `bde8051d8b` Tests: Updated TCP algorithm test goldens to current scalar formatting. | **STANDALONE** | High | `git log --oneline 47adf296fb..4861313719 -- tests/module/tcp_algorithm_*.test` → only this one commit ever touches these 6 files. `git diff bde8051d8b master -- tests/module/tcp_algorithm_*.test` → **empty** (byte-identical to current master). | Hypothesis confirmed: this is a verbatim adoption of master's copy (rebase pre-resolution: master already reflects the `1000000 → 1e+06` scalar-formatting change), not a fix to anything the branch itself introduced. Must not be squashed anywhere in-branch. |
| 8 | `57458b84b2` Tests: Followed EV statement wording changes in UDPSocket_1 test. | **`3a9ec97538`** | High | `git show 3a9ec97538^:.../UdpSocket.cc` shows the source **already** used the current wording (`EV_FIELD(localAddr)`, `EV_FIELD(addr)`, `EV_FIELD(value)`) at that point — introduced earlier by `4daa804e71`/`5a5ac65eea` — yet `3a9ec97538`'s own added golden text used the **old** field names (`address`, `localAddress`, `multicastLoop`). Topological order (`git log --oneline | cat -n`, not author date) confirms `3a9ec97538` comes after both wording-introducing commits. | Hypothesis's target is confirmed, but the mechanism differs from what was guessed: `3a9ec97538` didn't merely lag behind a later `4daa804e71` rename — it authored an **inaccurate golden from the start**, already mismatched against the code as it stood when written. No other commit touches `UDPSocket_1.test` in between. No split. |
| 9 | `c03c0609db` Ipv4, Ipv6: Fixed ICMP error indication delivery to transport protocols. | **`f57702acd4d`** (todo3) — same ICMP-refactor group as #1 | High | `git blame -w c03c0609db^ -- src/inet/networklayer/ipv4/Ipv4.cc` (and Ipv6.cc, spot-checked) → the `DispatchProtocolReq`+`IPassivePacketSink` lookup in `handleIcmpErrorIndication` is `f57702acd4d` throughout. | Same introducer as #1 — todo3's ICMP refactor touched Udp.cc *and* Ipv4.cc/Ipv6.cc *and* MrpRelay.cc in one commit (Phase 0 already noted the split need for MrpRelay). The `Tcp.ned`/`Udp.ned` `@interface[inet::tcp::Tcp]`/`[inet::Udp]` additions on `ipIn` gates travel with this fix (not with `72b2d288bc`): they declare the concrete-type lookup that only exists because todo3 invented it — squashing them into the older, IPassivePacketSink-only-era `72b2d288bc` sweep would be anachronistic. **Recommend: items #1 and #9 plus todo3's ICMP-hunk be resolved as one group**, landing wherever Phase 0's "standalone or `2c8e8ceb18`" call is finally made. |
| 10 | `2f6c9f10cc` Mpls: Implemented IModuleInterfaceLookup to make the layer visible to lookups. | **`8d539f3eac`** | High | `git show --stat 8d539f3eac` touches 93 files implementing `IModuleInterfaceLookup`/`lookupModuleInterface()` for app- and routing-layer modules (Aodv, Bgp, Rip, DhcpClient/Server, NetPerfMeter, etc.) in the exact same style (`Enter_Method`+`EV_TRACE`+`findModuleInterface` pattern) but **never touches Mpls**. | Hypothesis confirmed: Mpls.cc/h are pure new-code additions (no prior `lookupModuleInterface` existed), so this is filling a gap `8d539f3eac`'s sweep left. No split. |
| 11 | `f4f8c8f3b2` Udp: Completed the socket command conversion to direct method calls. | **SPLIT**: bulk of `IUdp.h`+`Udp.cc`+`Udp.h` (new `setMulticastOutputInterface/setReuseAddress/block\|unblock\|leaveMulticastSources/joinMulticastSources/setMulticastSourceFilter`) → **`4c5522a4b0`**; `UdpSocket.cc` client call-site conversions → **`5a5ac65eea`**; `Udp::close()` null-callback guard → **`4c5522a4b0`** | High | Sibling `Udp::setBroadcast/setMulticastLoop/joinMulticastGroups/leaveMulticastGroups(int socketId,...)` handlers are all blamed to `4c5522a4b01` (server side); sibling `UdpSocket::bind/connect` client calls (`udp->bind(...)`) are `5a5ac65eea4` (client side). The unconditional `callback->handleClose()` in `Udp::close()` is `4c5522a4b01` verbatim (`// KLUDGE: this schedule call is here to keep the fingerprints`). | **Hypothesis was directionally right (5a5ac65eea) but incomplete** — it's actually a 3-hunk split across the two sweep commits per the headline finding. The 5 new IUdp methods have no precedent to "复用" other than being siblings of `4c5522a4b0`'s existing socketId-based Udp methods. |
| 12 | `e409faa29f` Tcp: Converted the socket destroy command... + `cd8dfaa2ee` Tcp: Converted remaining socket commands (read/setDscp/setTos)... | **SPLIT**: `ITcp.h`+`Tcp.cc`+`Tcp.h` (new `destroy`/`read`/`setDscp`/`setTos` handlers) → **`4c5522a4b0`**; `TcpSocket.cc` client call-site conversions → **`5a5ac65eea`** | High | Sibling `Tcp::abort/setTimeToLive/close/accept(int socketId,...)` handlers are all `4c5522a4b01`. Sibling `TcpSocket::abort()`'s `tcp->abort(connId)` / `setTimeToLive()`'s `tcp->setTimeToLive(connId,ttl)` client calls are `5a5ac65eea4`. `destroy/read/setDscp/setTos` themselves were untouched ancient code (2011/2018/2019/2023) before these two fixes — confirming neither sweep commit actually converted them, just left both sides (client+server) undone. | **Hypothesis wrong in scope** (assumed single target `5a5ac65eea`) — needs the same 2-way split as #11/#13/#14. |
| 13 | `e441f2ef9d` Sctp: Converted remaining socket commands to direct method calls. | **SPLIT**: `ISctp.h`+`Sctp.cc`+`Sctp.h` (new `setStreamPriority`/`setRtoInfo`/`destroy` handlers) → **`4c5522a4b0`**; `SctpSocket.cc` client call-site conversions → **`5a5ac65eea`** | High | Sibling `Sctp::receive/streamReset/getSocketOptions(int socketId,...)` handlers are `4c5522a4b01`. Sibling `SctpSocket::shutdown()`'s `sctp->shutdown(assocId,id)` / other calls' `sctp->getSocketOptions(assocId)` are `5a5ac65eea4`. `destroy/setStreamPriority/setRtoInfo` themselves were ancient (2016/2018/2020) before this fix. | Same pattern and same correction as #12. |
| 14 | `24d2d98e9d` Tun: Converted socket close and destroy commands to direct method calls. | **SPLIT**: `ITun.h`+`Tun.cc`+`Tun.h` (new `close`/`destroy` handlers) → **`4c5522a4b0`**; `TunSocket.cc` client call-site conversions → **`5a5ac65eea`** | High | `Tun::open(int socketId)` (the sibling already-converted handler) is `4c5522a4b01`. `TunSocket::open()`'s `tun->open(socketId)` client call is `5a5ac65eea4`. `TunSocket::close/destroy` themselves were ancient (2016/2018) before this fix. | Same pattern and same correction as #12/#13. |
| 15 | `5ae305f7ab` Quic: Converted the socket communication to the new intra-node architecture. | **STANDALONE** | High | `git log --oneline 47adf296fb..4861313719 -- src/inet/transportlayer/quic/` → only `5ae305f7ab` and an unrelated single-`#include`-removal commit (`7f2fccbdff`) ever touch quic/. `git show --stat 5a5ac65eea 4c5522a4b0 3881df4100 08fd7785a4 \| grep -i quic` → nothing. | Hypothesis confirmed: Quic was never converted by either sweep commit (or any other); this is genuinely new capability (new `IQuic` C++ interface + `QuicSocket` rewiring), not a fixup of an existing sweep. No split. |
| 16 | `fe75bef16a` BridgingLayer: Forward unclaimed module interface lookups to the layer below. | **`85415fbf50`** (todo1) | High | `git blame -w fe75bef16a^ -- .../BridgingLayer.cc`: the overall `lookupModuleInterface()` method is `8d539f3eac3` (2024-09-30), but the specific narrow `else if (type == typeid(IEthernet)) return findModuleInterface(...)` branch being broadened is `85415fbf507` (todo1, 2024-10-11) — todo1's own stat list includes `BridgingLayer.cc \| 5 +-`. | **Hypothesis wrong** (guessed `8d539f3eac`, the method's origin, rather than todo1, the line's actual origin). todo1 is itself an unresolved Phase-0 SPLIT candidate ("SPLIT across ~6 commits, low confidence, per-hunk mapping lost"); this is a further, previously-unidentified logical group within it — the fix should land wherever the todo1-split assigns the `BridgingLayer.cc` hunk. |
| 17 | `1dd0250e64` MessageDispatcher: Made the direct-claim precedence rule explicit in lookups. | **`e0c99ee8b0`** | High | `git blame -w 1dd0250e64^ -- src/inet/common/MessageDispatcher.cc`: the entire `forwardLookupModuleInterface()` function including the `// KLUDGE: to avoid ambiguity` block is `e0c99ee8b09`, unchanged since. | Hypothesis confirmed exactly. No split. |
| 18 | `06df44b377` IModuleInterfaceLookup: Documented the payload protocol lookup rationale. | **`2780ce0546`** | High | `git blame -w 06df44b377^ -- src/inet/common/IModuleInterfaceLookup.cc`: the whole file, including the literal `// TODO???` placeholder this commit replaces, is `2780ce05468` (2024-09-24), the file's origin. | Hypothesis confirmed exactly. Pure doc fixup, no split. |
| 19 | `992b426b67` EthernetSocket, Ieee8021qSocket: Made the missing socket support error actionable. | **`3e5bf3c8e2`** | High | `git blame -w 992b426b67^` on both `EthernetSocket.h` and `Ieee8021qSocket.h`: the mandatory `ethernet.reference(gate, true)` / `ieee8021q.reference(gate, true)` calls (whose generic failure this commit turns into an actionable error) are both `3e5bf3c8e2d` (2024-10-03) — identical in both files. | Hypothesis confirmed (candidate `3e5bf3c8e2` correct, not `3881df4100`). No split. |
| 20 | `970383a539` Tests: Added switch relaying test... + `38e737a313` Tests: Added protocol-path insertion tests. | **STANDALONE** (both) | High | `git show --stat` on both: 100% new files (`EthernetSwitch_UnknownEthertype.test`; `Insertion_MeasurementLayer_Host/Switch.test`, `Insertion_PcapRecorder.test`), zero modifications to existing files. | Hypothesis confirmed. These prove new requirements (D2 switch-relay property; protocol-path-insertion transparency), not regressions of branch code. |
| 21 | `1da6a175e3` "fixed checksumType parameter doesn't exist issue" | **`75f560badc`** | High | `git log --oneline 47adf296fb..4861313719 -- .../EthernetFcsRemover.ned` → only `75f560badc` (creates the file, extending `PacketFilterBase`, no `checksumType` param) and `1da6a175e3` (switches the base to `ChecksumCheckerBase` and sets `checksumType`). | Confirmed direct fixup of `75f560badc`'s own bug (wrong base class chosen when the module was created), all within the branch. **Proposed conforming message if kept standalone were ever preferred:** `EthernetFcsRemover: Extended ChecksumCheckerBase instead of PacketFilterBase to expose the checksumType parameter.` — but fixup into `75f560badc` is the clear recommendation; there's no reason to keep a bug-in-new-code fix standalone. |

### Item 5 detail — Sctp callback fixes, full hunk breakdown

`fd329a752c` (single hunk):
- `SctpAssociationUtil.cc::sendAvailableIndicationToApp` (fallback to listening
  association's callback) → **`85415fbf50`** (todo1). Blame: the naked
  `callback->handleAvailable(msg)` this replaces is `85415fbf507` (todo1),
  which converted `sctpMain->sendToApp(msg)` → `callback->handleXxx(msg)`
  throughout `SctpAssociationUtil.cc` without the accept-path fallback.

`a8ce3fe580` (three separable hunks):
1. `SctpAssociationUtil.cc::sendEstabIndicationToApp` (fork/oneToOne early
   return before adopting connection params) → **`85415fbf50`** (todo1), same
   introducer/reasoning as `fd329a752c`'s hunk — **these two fixes share this
   one target**, confirming the "check whether they share one target"
   question: yes, partially.
2. `SctpSocket.cc::handleEstablished` (the `oneToOne && socketId != assocId`
   fork short-circuit) → **`1fbca7833e`**. Blame: `handleEstablished()` itself
   (the whole function) is `1fbca7833e4` (2024-10-03), the commit that first
   implemented callback-interface delivery in `SctpSocket`.
3. `SctpSocket.cc::setCallback` (adds `sctp->setCallback(assocId, this)` for
   accepted sockets) + `Sctp.cc::setCallback` (tolerate a not-yet-existing
   association) — **finer split than originally scoped**:
   - `Sctp::setCallback(int,ICallback*)`'s null-tolerance → **`4c5522a4b0`**
     (blame: `getAssoc(socketId)->callback = callback;` is `4c5522a4b01`, the
     function's origin — same server-side sweep as items #11–14).
   - `SctpSocket::setCallback`'s new `sctp->setCallback(assocId, this)` call →
     **`5a5ac65eea`** by the same client-side-sweep-precedent logic as #11–14
     (Udp/Tcp's `setCallback` already register with the protocol module from
     `5a5ac65eea`; Sctp's never did — an `5a5ac65eea`-era gap, not a
     `4c5522a4b0` one). *Pragmatic note:* these last two are two one-line
     hunks in adjacent files serving one coherent purpose ("make
     `setCallback` robust for accepted sockets"); splitting them across two
     separate historical commits is defensible by blame but may not be worth
     the mechanical cost — if a simpler 3-way split is preferred over 4-way,
     folding hunk 3 entirely into `4c5522a4b0` is the reasonable compromise.

**Bottom line for item 5:** not a clean 1:1 fixup as hypothesized. `fd329a752c`
targets todo1 alone; `a8ce3fe580` needs a 3-to-4-way split across todo1,
`1fbca7833e`, and `4c5522a4b0` depending on how finely the last hunk is divided.

## Nonconforming commit-message sweep

`git log --oneline 47adf296fb..4861313719`, everything that is not `todoN` and
not already one of the 21 items above:

| SHA | Subject | Issue |
|---|---|---|
| `4861313719` | Updated the completion plan: Phase 1 complete. | No module/category prefix — "Updated the completion plan" is a full clause before the colon, not an INET-style prefix. **However** this commit only touches `plan/pending/complete-topic-infrastructure.md` (plan artifact, explicitly meant to be dropped before merge per `ef6b5e688b`'s own message) — message style is moot since the commit won't survive to the final history. |
| `ef6b5e688b` | Added the branch completion plan, decision log and test artifacts. | No prefix. Same as above: plan-artifact-only commit, to be dropped before merge; not a real code-history entry. |
| `62d30b2a6f` | Fixed 802.11 module interface lookup. | No module prefix (should read e.g. `Ieee80211Mac: Fixed module interface lookup.`). Real code commit — worth a message fix if it isn't squashed away as part of some other placement. |
| `1da6a175e3` | fixed checksumType parameter doesn't exist issue | No prefix, no period, not capitalized. **This is item #21 above** — recommended fixup into `75f560badc`, so the message issue disappears entirely once squashed. |
| `ad606a05bb` | Removed obsolete streamThroughputVectors code replaced by signal-based recording. | No module prefix (e.g. `Sctp: Removed obsolete...` — touches `SctpAssociationUtil.cc`/`.h`). |
| `f2aecee10e` | Fixed missing initialize() declaration in EchoProtocol.h. | No prefix (should be `EchoProtocol: Fixed missing initialize() declaration.`). |
| `7f2fccbdff` | Removed obsolete IProtocolRegistrationListener includes and registerProtocol/registerService calls. | No prefix (touches many files — `all:` would fit the branch's existing convention for cross-cutting commits). |

Minor/cosmetic, not flagged as a real problem: `65da1fc7a1 applications/ethernet:
Fixed module test EtherHost_lifecycle.test.` uses a slash-separated
`applications/ethernet:` prefix instead of a single module/category name —
unusual shape but still fits the "prefix: sentence." pattern the branch uses
elsewhere (cf. `ip:`, `sctpapp:`, `all:` lowercase category prefixes), so it
was not treated as nonconforming.

None of the above were analyzed for fixup placement (out of scope per the
task); they are listed for awareness only. `62d30b2a6f`, `ad606a05bb`,
`f2aecee10e`, `7f2fccbdff` are real code/behavior commits whose messages could
use a rename pass (not a squash) if message hygiene is pursued in this same
history rewrite.
