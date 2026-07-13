# todoN squash map — per-hunk evidence (Phase 2 re-derivation)

Re-derived from scratch by `git show <sha>` + `git blame -w <sha>^ -- <file>` /
`git log -L<range>:<file> <sha>^`, per the Phase 2 instructions. Merge-base with
master is `47adf296fb`. All SHAs cited as "target" are reachable in
`47adf296fb..<todo-sha>^` (i.e. valid rebase targets) unless explicitly marked
**pre-branch** (an ancestor of `47adf296fb`, confirmed with
`git merge-base --is-ancestor`), in which case the piece must be a standalone
commit instead of a fixup.

Commit-date note: author dates in this history are not monotonic with topology
(e.g. some pre-branch master commits carry 2026 dates later than `47adf296fb`
itself, and some very recent standalone bugfix commits — `a8ce3fe580`,
`fd329a752c`, `e441f2ef9d`, `e409faa29f`, `cd8dfaa2ee` — were added near HEAD in
2026-07). Ancestry (`--is-ancestor`), not date, decides pre-branch vs in-range.

---

## todo1 = 85415fbf50 (25 files → 10 groups)

Phase 0 estimated "~6 distinct logical groups". Deeper hunk-level blame finds
**10** groups once same-file-different-concern splits are accounted for
(EthernetMacPhy.cc and TunLoopbackApp.cc each mix a real todo1 change with a
todo8 comment-only follow-up on the *same line*, see todo8 section).

### Group todo1-A — SCTP/TCP app socket-callback conversion (STANDALONE)

| File | Target | Action | Confidence |
|---|---|---|---|
| `applications/netperfmeter/NetPerfMeter.cc` | — | standalone | high |
| `applications/netperfmeter/NetPerfMeter.h` | — | standalone | high |
| `applications/sctpapp/SctpServer.cc` | — | standalone | high |
| `applications/sctpapp/SctpServer.h` | — | standalone | high |

Both apps are rewritten from raw `handleMessage()`/message-kind dispatch to
`TcpSocket::ICallback`/`SctpSocket::ICallback` + direct method calls
(`socket->send()`, `socket->setQueueLimits()`, `socket->receive()`,
`socket->shutdown()`). No single branch commit "owns" these files — `git log`
in range touches neither file except this one. This mirrors the *style* of
sibling standalone commits already in the branch (`5ae305f7ab Quic: Converted
the socket communication to the new intra-node architecture.`).

Evidence: `git log 47adf296fb..85415fbf50^ -- NetPerfMeter.cc/.h` empty;
same for `SctpServer.cc/.h`. Class-declaration base-list blame lands on
`2c8e8ceb18`/`8d539f3eac` only because those are the last edits of the *same
line* (adding unrelated base interfaces earlier) — not because they own this
conversion.

Note: both files retain live `// TODO` stubs (`Sctp.cc`'s
`SCTP_C_GETSOCKETOPTIONS` reply, `SctpServer`'s abort-notification path) that
were *disabled*, not converted — this is pre-existing incompleteness, carry
it forward as-is.

Proposed message: `NetPerfMeter, SctpServer: Converted socket communication to use callback interfaces and direct method calls.`

### Group todo1-B — sync-push reentrancy bookkeeping (FIXUP → 3881df4100)

Target: **3881df4100** "all: Replaced packet sends in socket classes with pushPacket() calls using the passive packet sink."

| File | Hunk | Evidence | Confidence |
|---|---|---|---|
| `applications/pingapp/PingApp.cc` | `sendPingRequest()`: store `sendTimeHistory`/`pongReceived` *before* `currentSocket->send()` | `Ipv4Socket::send`/`Ipv6Socket::send` converted from async `sendToOutput()` to synchronous `sink.pushPacket()` by 3881df4100 (verified diff) | medium-high |
| `applications/pingapp/PingApp.cc` | `socketClosed()`: finish `STOPPING_OPERATION` when `socketMap.size()==0` | same-day sibling `5a5ac65eea` made `close()` a direct call too | medium |
| `applications/udpapp/UdpBasicApp.cc/.h` | cache `sendInterval` in `sendPacket()`, consume cached value in `processSend()` | `UdpSocket::sendTo()` converted to synchronous `sink.pushPacket()` by 3881df4100 (verified diff) | medium-high |
| `applications/tunapp/TunLoopbackApp.cc` | re-add `PacketProtocolTag` after `packet->clearTags()` | `TunSocket::send()` converted to `sink.pushPacket()` by 3881df4100 (verified diff) — pushPacket dispatch needs the tag that `clearTags()` wiped | high |

Blame on the raw lines themselves lands pre-branch in all four cases (2016–2020
authors); the fixup target is justified causally: 3881df4100 made the
corresponding `Socket::send()`/`sendTo()` calls synchronous (direct
`pushPacket()` instead of an always-async `cGate::send()`), which is exactly
what each of these hunks is compensating for (reentrant callback ordering, or
a tag that's now required for the push-based dispatch to route correctly).

**todo8 also touches `TunLoopbackApp.cc`** at the identical line (adds a
trailing comment only, see todo8 section) — fold into this same group/commit.

### Group todo1-C — 802.11 radio configuration direct calls (STANDALONE)

| File | Target | Confidence |
|---|---|---|
| `linklayer/ieee80211/mac/Ieee80211Mac.cc` | — | high |
| `linklayer/ieee80211/mac/Ieee80211Mac.h` | — | high |
| `linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc` | — | high |

Replaces `RADIO_C_CONFIGURE` message commands (`Ieee80211ConfigureRadioCommand`,
`ConfigureRadioCommand`) with direct calls to `Ieee80211Radio::setChannelNumber()`
and `IRadio::setRadioMode()`, which are themselves ancient (2014) pre-existing
methods (`git blame` confirms). Two *later* branch commits, `696ee24636`
("Ieee80211ConfigureRadioCommand: Removed obsolete message class...") and
`014ef076df` ("ConfigureRadioCommand: Removed obsolete message class..."),
already assume this conversion happened — their diffs are pure `#include`
cleanups applied against the post-todo1 blob content, confirming todo1 is
their prerequisite, not the other way round.

Proposed message: `Ieee80211Mac, Ieee80211MgmtSta: Converted radio configuration commands to direct method calls.`

### Group todo1-D — BridgingLayer dormant IEthernet lookup (FIXUP → 8d539f3eac)

Target: **8d539f3eac** "all: Implemented new ILookupModuleInterface C++ interface for several modules."

| File | Evidence | Confidence |
|---|---|---|
| `linklayer/ethernet/common/BridgingLayer.cc` | `git blame 85415fbf50^` shows the commented-out `// else if (type == typeid(IEthernet)) …` block was introduced already-dormant by 8d539f3eac itself | high |

Same pattern as todo6/62d30b2a6f: dormant code introduced disabled by one
commit, enabled later by a todo commit. Per the "no enable→disable churn"
principle, squash directly into 8d539f3eac so it's born enabled (plus the
`#include "inet/linklayer/ethernet/contract/IEthernet.h"` todo1 adds).

### Group todo1-E — callback-based indication delivery completion (FIXUP → 4c5522a4b0)

Target: **4c5522a4b0** "all: Added missing Enter_Method() and take() calls into pushPacket implementations."

| File | What it completes | Confidence |
|---|---|---|
| `linklayer/ieee8022/Ieee8022Llc.cc` | `SocketCloseCommand` handling calls `callback->handleClosed()` instead of sending a `SOCKET_I_CLOSED` Indication | high |
| `transportlayer/sctp/Sctp.cc` | disables (marks `// TODO`) the leftover `send(cmsg, "appOut")` for `SCTP_C_GETSOCKETOPTIONS` | high |
| `transportlayer/sctp/SctpAssociationBase.cc` | reorders `stateEntered()` so `sendAvailableIndicationToApp()`/`sendEstabIndicationToApp()` run *after* vtagPair bookkeeping (needed once these calls are synchronous) | high |
| `transportlayer/sctp/SctpAssociationUtil.cc` | `sendIndicationToApp`/`sendAvailableIndicationToApp`/`sendEstabIndicationToApp`/`sendDataArrivedNotification`/`pushUlp`/`pathStatusIndication` switched from `sctpMain->sendToApp(msg)` to `callback->handleXxx(...)` | high |
| `transportlayer/tcp/TcpConnectionEventProc.cc` | `process_ABORT` on `TCP_S_INIT` tolerated instead of `throw` (small adjacent bugfix, same function region) | medium |
| `transportlayer/tcp/TcpConnectionUtil.cc` | `sendIndicationToApp`/`sendAvailableIndicationToApp`/`sendEstabIndicationToApp`/`initClonedConnection` switched to `callback->handleXxx(...)`, using `inet::scheduleAfter()` to preserve fingerprints | high |

Evidence: `git blame 85415fbf50^ -- SctpAssociation.h` / `TcpConnection.h` /
`Ieee8022Llc.h` all show the `callback`/`ICallback *callback` field was added
by **4c5522a4b0** in all three classes — the same commit added `open()`/
`close()`/`setCallback()` to `Ieee8022Llc` and `setCallback()` to
`Sctp`/`TcpConnection`, but left several call sites still using the old
message-based `sendToApp`/`send()` path. todo1 finishes that conversion.

### Group todo1-F — TcpSocket reconnect callback re-registration (FIXUP → 3e5bf3c8e2)

Target: **3e5bf3c8e2** "all: Added packet sink and protocol module interface fields to all socket classes."

| File | Evidence | Confidence |
|---|---|---|
| `transportlayer/contract/tcp/TcpSocket.h` | `setOutputGate()` body (which the hunk extends with `if (sockstate==CONNECTED) tcp->setCallback(connId, this);`) was written whole-cloth by 3e5bf3c8e2 | medium-high |

This is the TCP analogue of the SCTP fix later applied as a dedicated
standalone commit (`a8ce3fe580`): an accepted/forked socket calling
`setOutputGate()` on a fresh `TcpSocket` needs to re-register itself as the
connection's protocol-side callback since it never went through `connect()`/
`listen()`.

### Group todo1-G — NetworkInterface upperLayerOut null-consumer guard (FIXUP → 9fea7b31ba)

Target: **9fea7b31ba** "NetworkInterface: Changed module interface lookup to use a protocol indication argument for the upper layer out." (same author, same day, 4 minutes before todo1)

| File | Evidence | Confidence |
|---|---|---|
| `networklayer/common/NetworkInterface.cc` | 9fea7b31ba changed `upperLayerOutConsumer.reference(...)` to pass a protocol-specific `DispatchProtocolReq`; before, it referenced unconditionally. todo1 adds the `upperLayerOutConsumer != nullptr` guard (drop-with-warning instead of crashing) that this newly-possible lookup failure requires | high |

### Group todo1-H — Bgpv4 established/available indication socket-id fix (STANDALONE, alt. fixup → 074cf107c3)

| File | Evidence | Confidence |
|---|---|---|
| `routing/bgpv4/BgpRouter.cc` | `074cf107c3` mechanically added an `Indication *indication` parameter to `socketEstablished()` but left the body reading `socket->getSocketId()` (blamed to 2018, pre-branch) instead of the indication's `SocketInd` tag — exactly the bug class later fixed for SCTP as standalone commits `fd329a752c`/`a8ce3fe580` | medium |
| `routing/bgpv4/BgpRouter.h` | adds `ensureSocket()` helper, implements `socketAvailable()` (was `{ socket->accept(...); } // TODO`) | medium |

**Recommendation:** standalone commit, following the precedent set by the
*later* SCTP fixes (`fd329a752c`, `a8ce3fe580`) which the author wrote as
dedicated, well-documented commits rather than folding into the sweep commit
that exposed the bug. Alternative: fixup into `074cf107c3` if a tighter
history is preferred — both are defensible; blame favors neither strongly
since the crux logic predates the branch.

Proposed message (if standalone): `Bgpv4: Fixed established and available indication delivery to use the accepted socket's id.`

### Group todo1-I — Ipv4 defensive Ipv4InterfaceData lookups (STANDALONE)

| File | Evidence | Confidence |
|---|---|---|
| `networklayer/configurator/ipv4/Ipv4NodeConfigurator.cc` | calls `prepareInterface()` (pre-existing since 2013/2020) when `findProtocolData<Ipv4InterfaceData>() == nullptr`; blamed region pre-branch (`5f383373ca0`, 2024-02) | medium |
| `networklayer/ipv4/Ipv4RoutingTable.cc` | `isLocalBroadcastAddress()`: `getProtocolData` → `findProtocolData` guard; blamed pre-branch (`ef08685f1a1`, 2018) | medium |

Both are defensive-programming fixes for interfaces whose `Ipv4InterfaceData`
protocol data may not be attached yet. The `Ipv4RoutingTable.cc` hunk echoes
the *exact same* `findProtocolData` pattern that same-day, in-range commit
`43da82c930` ("Ipv4RoutingTable: Update netmask route for a specific interface
only for interface changed signals.") already applied elsewhere in the same
function file — that commit is evidence of the pattern's currency but doesn't
itself touch the modified lines, so this is presented as standalone rather
than a strict fixup.

Proposed message: `Ipv4: Guarded against missing Ipv4InterfaceData during interface (re)configuration.`

---

## todo3 = f57702acd4 (14 files → 2 pieces, MUST combine with todo7)

### (a) MrpRelay.cc — FIXUP, target **corrected**

Prior finding said target = `62e378249c` ("Mrp: Replaced send() calls with
pushPacket() calls using PassivePacketSinkRef."). **This is wrong**:
`62e378249c` only touches `Mrp.cc`/`Mrp.h` (a different class) — it never
touches `MrpRelay.cc`. Verified: `git show --stat 62e378249c` lists no
MrpRelay file; `git log 47adf296fb..f57702acd4^ -- MrpRelay.cc` lists only
`7f2fccbdff` (an unrelated `#include`/`registerService` cleanup) and todo3
itself.

**Corrected target: 08fd7785a4** "all: Added PassivePacketSinkRef fields for
output gates and replaced send() calls with pushPacket() calls." Evidence:
`lowerLayerSink` (the field the hunk's replacement `lowerLayerSink.pushPacket(packet)`
uses) is declared in `MacRelayUnitBase.h` (MrpRelay's grandparent base class
via `Ieee8021dRelay`), added by `08fd7785a4` (`git blame` confirms, line-exact
match). 08fd7785a4 did the mechanical `send()`→`pushPacket()` sweep everywhere
except this one `sendDelayed(packet, switchingDelay, "lowerLayerOut")` call,
which needed the more involved `inet::scheduleAfter(...) { lowerLayerSink.pushPacket(packet); }`
rewrite (nonzero delay) and was evidently skipped in the original mechanical
pass.

Confidence: **high** (corrected from the prior finding's medium).

### (b) ICMP/Ipv4/Ipv6/Udp/Sctp/Tcp direct-call refactor — STANDALONE, confirmed pre-branch

Files: `networklayer/ipv4/Icmp.cc/.h`, `networklayer/icmpv6/Icmpv6.cc/.h`,
`networklayer/ipv4/Ipv4.cc/.h`, `networklayer/ipv6/Ipv6.cc/.h`,
`transportlayer/udp/Udp.cc/.h`, `transportlayer/sctp/Sctp.cc` (1-line
`send_to_ip()` consistency tweak, folds into this group), `transportlayer/tcp/Tcp.cc/.h`.

`Icmp`/`Icmpv6` now hold a pointer to their `Ipv4`/`Ipv6` module (found via
`findModuleInterface`) and call `ipv4Module->handleIcmpErrorIndication()` /
`ipv6Module->handleIcmpErrorIndication()` directly instead of sending an
`Indication` via `"ipOut"`/`"ipv6Out"`. `Ipv4`/`Ipv6` in turn locate the
transport module via `findModuleInterface(gate("transportOut"), typeid(IPassivePacketSink), &req)`
and call `udp->processIcmpv4Error()` / `tcp->processIcmpv4Error()` directly.

Blame: `Ipv4::handleIcmpErrorIndication`'s pre-image body (the message-send
version being replaced) is entirely authored by **247f213d975** "Refactor
ICMP error indication propagation for proper layering" (2026-03-23).
Confirmed **pre-branch**: `git merge-base --is-ancestor 247f213d975 47adf296fb`
→ true. So per the stated rule this must be a standalone commit, not a fixup
into `2c8e8ceb18` as the prior finding tentatively suggested.

Proposed message: `Icmp, Icmpv6, Ipv4, Ipv6, Udp, Tcp: Replaced ICMP error indication messaging between network and transport modules with direct method calls.`

**Confidence: high** (both for standalone-ness and for the fact that it must
travel together with todo7 — see next section, which is not optional).

---

## todo7 = bc26b8f6de — confirmed Tcp.cc/Tcp.h only, and squash is MANDATORY not optional

`git show --stat bc26b8f6de` touches exactly `Tcp.cc` (-16) and `Tcp.h` (-2),
nothing else — confirmed.

**Critical finding beyond what was asked to verify: todo3 alone does not
compile.** `git show f57702acd4:Tcp.cc | grep -n "^void Tcp::processIcmpv4Error\|^void Tcp::processIcmpv6Error"`
shows **two** definitions of each function (lines 212/308 and 220/347) at
commit f57702acd4 itself:

- The pre-existing, substantial implementation (queries `findConnForSockPair`,
  calls `conn->processIcmpv4Error()`) was already in `Tcp.cc` before todo3 and
  is untouched by todo3's diff.
- todo3 *additively* inserts a second, trivial stub definition
  (`Enter_Method(...); take(indication); EV_DETAIL << "... discarding"; delete indication;`)
  right after `handleLowerCommand()`, without removing the old one.

Two definitions of the same non-overloaded member function in one translation
unit is a hard compile error. Symmetrically, `Tcp.h` gains a second, public
declaration of both methods (comment: "public for direct calls from Ipv4")
while the original protected declaration is left in place — also a duplicate
member declaration. **The tree at f57702acd4 does not build.** This is not a
style preference; todo7 removing the stub bodies (`Tcp.cc`) and the old
protected declarations (`Tcp.h`) is required to make any single resulting
commit compile.

Verified combined/net diff with the exact command from the task brief:

```
git diff f57702acd4^ bc26b8f6de -- src/inet/transportlayer/tcp/Tcp.h src/inet/transportlayer/tcp/Tcp.cc
```

Result: **`Tcp.cc` has zero net diff** (the stub add/remove cancels exactly,
leaving the original real implementation untouched). `Tcp.h` has exactly one
surviving change: `processIcmpv4Error`/`processIcmpv6Error` move from the
`protected:` section to the `public:` section (with the explanatory comment
"process ICMPv4/ICMPv6 error indications (public for direct calls from
Ipv4)"). This exactly matches the predicted net effect and is fully
consistent with `Icmp.cc` now calling `ipv4Module->handleIcmpErrorIndication()`
directly and `Ipv4::handleIcmpErrorIndication()` calling
`tcp->processIcmpv4Error()` directly — both need public access.

**Action: todo3's Icmp/Ipv4/Icmpv6/Ipv6/Udp/Sctp/Tcp piece and all of todo7
must be squashed into exactly one commit** (piece "todo3-b" below already
includes todo7). Do not place them non-adjacently or split further; any
intermediate commit containing todo3's Tcp.h/Tcp.cc hunks without todo7's
would fail to compile.

---

## todo8 = 8dee6f83f6 — 3-way split, corrected framing

`git show --stat`: `TunLoopbackApp.cc` (+1/-1), `EthernetMacPhy.cc` (+1/-1),
`InterpacketGapInserter.cc` (+3).

### (a) TunLoopbackApp.cc — folds into todo1-B, not a separate piece

The hunk only appends a trailing comment
(`// needed to run *** tun-echo.test: PASS`) to the *exact same line*
todo1 already introduces (`packet->addTag<PacketProtocolTag>()->setProtocol(&packetProtocol);`).
It is not an independent group — squash it into **todo1-B** (fixup target
`3881df4100`) as a comment addition to the same line. Consider dropping/
rewording the comment (it reads like a scratch test-run note, e.g.
`// needed for pushPacket-based dispatch to route correctly`) rather than
literally keeping "PASS" wording, but that's a style call, not a mapping one.

### (b) EthernetMacPhy.cc — corrected: not standalone-by-itself, merges with todo1's own hunk on the same file

Phase 0 called this "comment-only → standalone" in isolation, but **todo1
already modifies the exact same line** in the exact same function
(`processMsgFromNetwork()`): todo1 introduces the real behavior change
(`if (!hasBitError) decapsulate(packet);`, previously unconditional), and
todo8 merely appends a clarifying comment to that same `if` line
(`// needed to run examples/ospfv2/... CrashAndReboot ... : PASS`).

`git blame 85415fbf50^` on the pre-image line shows it dates to `b9ec6b12749`
(2017) / renamed by `941b815411f` (2025-04-12), both **pre-branch** — no
in-range commit owns it. So todo1's EthernetMacPhy.cc hunk is itself its own
standalone bugfix (call it **todo1-J**, not previously enumerated in the
groups above because it was found while cross-checking todo8): "only skip
decapsulation when the received signal has a bit error." todo8's comment
merges into that same new commit.

Proposed message: `EthernetMacPhy: Skipped decapsulation for frames received with a bit error.` (comment from todo8 folded in to document the regression test that motivated it.)

Confidence: high (file/line identity between todo1 and todo8 is unambiguous).

### (c) InterpacketGapInserter.cc — standalone, confirmed independent

Not touched by todo1 or anywhere else in `47adf296fb..8dee6f83f6^`
(`git log` for this path in that range is empty) — genuinely independent,
pure comments (`// TODO: duration may be 0 here` ×2, `// TODO: this maybe too
late, because the tx end emit signal is done earlier then the
handlePushPacketProcessed call`), no behavior change.

Proposed message: `InterpacketGapInserter: Documented timing edge cases around zero-duration packets and the handlePushPacketProcessed ordering.`

Confidence: high.

---

## todo4 = 8688c99866 — confirmed

Single file, `linklayer/ieee8022/Ieee8022Llc.cc`: fixes `lowerLayerSink.reference()`
to pass a `DispatchProtocolReq{ethernetMac, SP_REQUEST}` instead of a
`PacketProtocolTag{ieee8022llc}` (wrong tag type/protocol for the lookup).

`git blame 8688c99866^` on the replaced lines → **08fd7785a4** exactly
(the same commit that introduced `lowerLayerSink.reference(...)` in this file
in the first place). Target confirmed: **08fd7785a4**. Confidence: **high**
(matches prior finding exactly).

## todo5 = c812c9e636 — confirmed, with corrected evidence path

Single file, `linklayer/mrp/Mrp.ned`: adds
`@interface[inet::queueing::IPassivePacketSink](protocol=mrp; service=indication)`
to the `relayIn` gate.

Raw blame on that gate line lands on `c0850d265ae` (DJtime, 2024-02-10,
**pre-branch**) — `Mrp.ned` is *only* ever touched by todo5 itself within the
whole branch range (`git log 47adf296fb..4861313719 -- Mrp.ned` → one hit).
So the target isn't established by blame on this file, but by **mechanism
ownership**: `72b2d288bc` ("all: Added @interface properties to input gates
on many modules...") is a 79-file sweep that adds this *exact* annotation
convention to `relayIn` gates on sibling relay modules — e.g. every
`Ieee8021dRelay`-based `*.ned` gets
`@interface[inet::queueing::IPassivePacketSink](protocol=stp; service=indication)`
on `relayIn` (verified via `git show 72b2d288bc`) — but skips `Mrp.ned`
(protocol=mrp analog), presumably because `Mrp`/`MrpRelay` weren't wired into
that sweep's file list. Target confirmed: **72b2d288bc**, by pattern/mechanism
match rather than direct blame. Confidence: **high** (matches prior finding,
evidence path clarified/corrected).

---

## todo6 = 238566c312 — confirmed, single file, single target, no exceptions

Single file, `common/CoroutineEventExecution.cc`, exactly 2 lines changed:
both `cSimulation::getActiveSimulation()->setEventExecutor(...)` calls in
`installCoroutineEventExecution()`/`uninstallCoroutineEventExecution()` are
commented out.

`git blame 238566c312^` on both lines → **62d30b2a6f** "Fixed 802.11 module
interface lookup." exactly (that commit adds `CoroutineEventExecution.cc/.h`
as brand-new files, with both `setEventExecutor` calls already fully enabled).
No other hunk exists to blame elsewhere — this todo is as simple as it looks.

**Target: 62d30b2a6f. Action: fixup, no split. Confidence: high**, fully
matching the prior finding.

What 62d30b2a6f looks like after squashing todo6 in: identical file/commit
content in every other respect (the `CoroutineSlot`/`CoroutinePool` machinery,
the `SimulationContinuation.cc` rewrite from `ucontext_t`/`swapcontext` to
coroutine-slot yielding, `Ieee80211Mac::pushPacket()`, and the seven `.ned`
files' `@interface[...IPassivePacketSink]` annotations on mgmt/`mgmtIn` gates
all stay exactly as committed) — except `installCoroutineEventExecution()`
and `uninstallCoroutineEventExecution()` are born as no-ops (the
`setEventExecutor` calls commented out from the start). The dormant coroutine
event-execution machinery is introduced already-disabled in a single commit,
with no later enable→disable churn.

---

## Proposed pieces (for the interactive-rebase script)

| Label | Todo source(s) | Files | Target parent | Action |
|---|---|---|---|---|
| todo1-A | todo1 | NetPerfMeter.cc/.h, SctpServer.cc/.h | — | standalone |
| todo1-B | todo1 + todo8(a) | PingApp.cc, UdpBasicApp.cc/.h, TunLoopbackApp.cc | 3881df4100 | fixup |
| todo1-C | todo1 | Ieee80211Mac.cc/.h, Ieee80211MgmtSta.cc | — | standalone |
| todo1-D | todo1 | BridgingLayer.cc | 8d539f3eac | fixup |
| todo1-E | todo1 | Ieee8022Llc.cc, Sctp.cc, SctpAssociationBase.cc, SctpAssociationUtil.cc, TcpConnectionEventProc.cc, TcpConnectionUtil.cc | 4c5522a4b0 | fixup |
| todo1-F | todo1 | TcpSocket.h (contract) | 3e5bf3c8e2 | fixup |
| todo1-G | todo1 | NetworkInterface.cc | 9fea7b31ba | fixup |
| todo1-H | todo1 | BgpRouter.cc/.h | — (alt: 074cf107c3) | standalone (or fixup) |
| todo1-I | todo1 | Ipv4NodeConfigurator.cc, Ipv4RoutingTable.cc | — | standalone |
| todo1-J | todo1 + todo8(b) | EthernetMacPhy.cc | — | standalone |
| todo3-a | todo3 | MrpRelay.cc | 08fd7785a4 (corrected from 62e378249c) | fixup |
| todo3-b | todo3 + todo7 (mandatory) | Icmp.cc/.h, Icmpv6.cc/.h, Ipv4.cc/.h, Ipv6.cc/.h, Udp.cc/.h, Sctp.cc (1 line), Tcp.cc/.h | — | standalone |
| todo4 | todo4 | Ieee8022Llc.cc | 08fd7785a4 | fixup |
| todo5 | todo5 | Mrp.ned | 72b2d288bc | fixup |
| todo6 | todo6 | CoroutineEventExecution.cc | 62d30b2a6f | fixup |
| todo8-c | todo8 | InterpacketGapInserter.cc | — | standalone |
| todo9 | todo9 | (adds only `__TODO` file) | — | drop (unchanged from phase0) |

Total accounting for todo1: 4+4+3+1+6+1+1+2+2+1 = 25 files across
todo1-A..J. ✓
