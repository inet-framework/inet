# Plan: Path MTU Discovery showcase

Status: **blocked on implementation work**. Nothing built yet.
Written 2026-09-02. Split out of the IPv6-in-IPv6 tunneling showcase planning.

## Claim

An IPv6 path can carry a smaller packet than the sender's own link allows. This
page shows what happens when a sender does not know that, what Path MTU
Discovery does about it, and what it costs when the discovery fails.

A tunnel is used only as the *cause* of the mismatch — it is the most natural
way to make a path narrower than its links. The page is about the MTU
behaviour, not about tunneling.

## Why this is a separate page from the tunneling showcase

The two were originally planned as one page. Reasons for splitting:

- Combined, the page would have carried seven configs.
- More importantly: once Path MTU Discovery is implemented, these configs are
  not about tunneling at all. A reader looking for IPv6 MTU behaviour would
  never think to look inside a tunneling page.

The tunneling page (`showcases/ipv6/tunneling`, branch
`topic/gy/ipv6-tunneling-showcase`) mentions the 40-byte encapsulation overhead
and that `tun[0].mtu` defaults to 1500, and points here. Nothing more.

## Prerequisites — this page cannot be built until these exist

### 1. Path MTU Discovery in `Ipv6` (RFC 8201) — **DONE 2026-09-02**

Implemented and committed on this branch:

```
5718113c06  Icmpv6: refactor: extract quoteAndSendErrorMessage() from sendErrorMessage()
909a3e0dec  Icmpv6: add: sendPtbMessage() for reporting the next-hop MTU
5d79674b15  Ipv6: fix: report the real next-hop MTU in Packet Too Big
3b1b656787  Ipv6RoutingTable: add: path MTU estimate in the destination cache
2e77bf5fb0  Ipv6: add: Path MTU Discovery (RFC 8201)
```

The cache lives in `DestCacheEntry` (where the marker comment invited it), with
`reducePathMtu()` / `getPathMtu()`; RFC policy (1280 floor, when to consult)
stays in `Ipv6`. Two new NED parameters on `Ipv6`: `pathMtuDiscovery` (bool,
**default true**) and `pathMtuAgingTime` (default 10 min, lazy expiry).

Verified: all 324 module tests pass (debug), five new tests added, TCP still
receives the indication and reduces `snd_mss`, and the fingerprint sweep over
516 tests showed zero mismatches.

**Module tests must be run in debug.** In release, `-DNDEBUG` compiles out
`EV_DEBUG`, so `IPv6_packet_too_big` and `MIPv6_tcp_handover` fail on master
too. Pre-existing mode artifact, not a regression.

**TOPOLOGY CONSTRAINT for this showcase — do not ignore.** The MTU bottleneck
must be at or before the tunnel entry point, i.e. set `tun[0].mtu` so the entry
point itself generates the Packet Too Big. If the bottleneck is placed *beyond*
the entry point (e.g. on the transit link), the entry point receives a Packet
Too Big about the *outer* datagram, double-unwraps it and dispatches to a
transport protocol it does not have, and the simulation errors out. See "Bugs
found" below.

### 1b. Bugs found while implementing (reported, NOT fixed)

1. **`Ipv6::handleIcmpErrorIndication()` fails on a pure router that is a tunnel
   entry point.** A Packet Too Big arriving about the outer datagram is unwrapped
   twice and dispatched to UDP, which a router has no instance of:
   `Unknown protocol: protocolId = 69, protocolName = udp, servicePrimitive =
   INDICATION ... in (inet::MessageDispatcher) ...ipv6.lp`. Pre-existing —
   reproduces with `pathMtuDiscovery=false`. This is exactly the hole RFC 2473
   §7.2 (tunnel error relaying) is meant to fill. **Bug-report candidate.**
2. The same double-unwrap caches an inner path MTU without subtracting the 40-byte
   tunnel overhead (1400 stored where 1360 is correct). Benign today, but wrong,
   and the same §7.2 gap.
3. **Fragments carry the wrong Payload Length.** `Ipv6::fragmentAndSend()` builds each
   fragment's base header with
   `staticPtrCast<Ipv6Header>(ipv6Header->dupShared())` and changes only
   `protocolId`, so `payloadLength` keeps the *original* datagram's value. Observed on
   the Path MTU Discovery showcase: a 126-byte last fragment whose outer header
   reports `payloadLength = 1500 B`; the correct value is 60 (8-byte fragment header
   plus 52 bytes of data). RFC 8200 Section 4.5 requires each fragment packet's
   Payload Length to describe that fragment. Harmless inside INET, because reassembly
   uses chunk lengths rather than the field, but wrong on the wire and wrong in any
   serialized capture. **NOT FIXED** — the one-line fix changes packet content, which
   could move fingerprints, and that sweep was too expensive to run here. Bug-report
   candidate; decide with the user.
4. The agent saw master storm on that scenario (~750k messages in the FES, 4.7 GB
   resident) while this branch runs clean, and could not attribute it.
   **Attribution is now known: that is the `isWireless` phantom-link bug fixed by
   commit 0a93193944** — the probe's `<wireless>` workaround had been removed, so
   master's library hits the recursive-routing loop. Not a separate defect.

### 1c. Original gap analysis (kept for reference)

Verified state of master (commit 817c84b4e6). Both ends of the wire are already
done; only the middle is missing.

Already present:
- `src/inet/networklayer/icmpv6/Icmpv6Header.msg` ~line 51-56 —
  `Icmpv6PacketTooBigMsg` already has `int MTU; //MTU of next-hop link`.
- `src/inet/networklayer/icmpv6/Icmpv6.cc` ~line 339 —
  `createPacketTooBigMsg(int mtu)` already sets the field.
- `src/inet/networklayer/icmpv6/Icmpv6.cc` ~line 122 — the receive path already
  extracts it: `errorInd->setMtu(ptb->getMTU())` into an `Icmpv6ErrorInd`.

Missing:
- `src/inet/networklayer/icmpv6/Icmpv6.cc` ~line 271 —
  `// TODO implement MTU support.` then `createPacketTooBigMsg(0)`. The MTU is
  never threaded down from the caller.
- `src/inet/networklayer/ipv6/Ipv6.cc` ~line 1070 —
  `sendIcmpError(packet, ICMPv6_PACKET_TOO_BIG, 0); // TODO set MTU`. The value
  is in the local variable `mtu` at that exact call site.
- No per-destination path MTU cache exists anywhere in
  `src/inet/networklayer/ipv6/`.

Work estimate:
1. Thread the MTU through `sendIcmpError` -> `sendErrorMessage` ->
   `createPacketTooBigMsg`. ~10 lines across three call sites.
2. Per-destination path MTU cache in `Ipv6`: store the value, honour the
   1280-byte IPv6 floor, age entries out and retry a larger size per RFC 8201.
   ~60-100 lines.
3. Consume the error Indication (IPv6 already receives it) and update the cache.
   ~20 lines.
4. When sending, fragment to `min(interface MTU, cached path MTU)`. ~10 lines.

A few hundred lines with tests. Needs its own branch, issue and pull request.
Fingerprint impact should be small — only simulations with an actual MTU
mismatch change — but check.

### 2. A way to drop ICMPv6 from configuration (for config 2)

**RESOLVED — no C++ needed, but it constrains the topology.**

There is no firewall or packet-filter module in the network layer. The
`inet.queueing.filter.*` modules exist but have no NED slot in
`Ipv6NetworkLayer.ned`, so they cannot be dropped into the IP path from ini.

What does work: the **IPsec module's `DISCARD` action**.
- `inet.networklayer.ipsec.IPsec`, present in `Ipv6NetworkLayer.ned` under
  `hasIpsec`, with `networkProtocol = "ipv6"`.
- Enable with `**.hasIpsec = true` and `**.spdConfig = xmldoc(...)`.
- SPD entries take `<Action>DISCARD</Action>` with a `<Selector>`.
- Registers on all five netfilter hooks including `datagramForwardHook`, so it
  works on a **router**, not just a host.
- Working IPv6 example already in the tree: `examples/inet/ipsec/ipv6.ini`.

**Constraint:** the ICMP type/code selector is IPv4-only —
`PacketSelector::matches()` gates it on `protocol == IP_PROT_ICMP`
(`src/inet/networklayer/ipsec/PacketSelector.cc:41-46`), and `IPsec.cc:396`
/`:484` note "ICMPv6 and any other protocol are matched on protocol number
only". So from XML you can drop **all** ICMPv6 (`<Protocol>58</Protocol>`) but
not selectively Packet Too Big.

Dropping all ICMPv6 next to hostA would also kill Neighbour Discovery. **The way
round it is a topology change** — put a firewall router between hostA and the
tunnel entry point:

```
hostA --- firewall --- routerA(tunnel entry) === transit === routerB --- hostB
```

Drop all *forwarded* ICMPv6 at the firewall. Neighbour Discovery is link-local
and never forwarded, so it is unaffected; only end-to-end messages such as
Packet Too Big die. This needs no C++, and it is more realistic than filtering
at an endpoint — a firewall between the local network and the WAN edge eating
ICMP is exactly the production case.

**VERIFIED 2026-09-02 — it works, zero C++ changes.** Probe files kept at
`/tmp/.../scratchpad/icmpfw/` (session-local; the recipe below is the durable
part).

Evidence: ping across the firewall went 50/51 delivered -> 0/51, while the UDP
flow stayed at 50/50 and SLAAC still completed on every host
(`inUnprotectedDrop:count = 0`). Confirmed on the real scenario too: ICMPv6
error indications reaching hostA went 50 -> 0.

Working ini:

```ini
*.firewall.ipv6.hasIpsec = true
*.firewall.ipv6.ipsec.spdConfig = xmldoc("spd.xml", "ipsecConfig/Devices/Device[@id='firewall']/")
```

Working SPD, in document order (first match wins,
`SecurityPolicyDatabase.cc:43-50`): one DISCARD entry selecting
`<Protocol>58</Protocol>` with LocalAddress = the far-side prefix range and
RemoteAddress = hostA's LAN prefix range, `<Direction>OUT</Direction>`; then a
catch-all `<Selector/>` BYPASS for OUT; then the same for IN. No
SecurityAssociation or SPI is needed for DISCARD/BYPASS.

**Three traps — all cost real time if rediscovered:**

1. **IPsec defaults to DROP when no SPD entry matches**
   (`IPsec.cc:434-437`, `:950-954`). Omit either catch-all BYPASS and the whole
   router goes dark — verified: 102 inbound drops, zero UDP delivered.
2. **`datagramForwardHook()` is a stub returning ACCEPT** (`IPsec.cc:354`), as
   are the pre-routing and local-out hooks. Only `datagramPostRoutingHook()`
   and `datagramLocalInHook()` do anything. Forwarded traffic is therefore
   caught by a `<Direction>OUT</Direction>` entry, because
   `Ipv6::fragmentPostRouting()` runs the POST_ROUTING hook for forwarded
   datagrams too (`Ipv6.cc:1038`). An `IN` DISCARD does **not** catch forwarded
   traffic.
3. **Do not scope the selector by address scope.** INET's Neighbour Discovery
   sends unicast NS/NA from the interface's *global* preferred address, so a
   "global source and global destination" rule kills address resolution
   (verified: the firewall's own Neighbour Advertisement was dropped, UDP fell
   to 8/51). Scope by concrete prefixes that exclude the filtering router's own
   addresses. Multicast ND is short-circuited to BYPASS before the SPD is
   consulted (`IPsec.cc:412-417`), so RA/DAD/solicited-node NS are safe
   regardless — and equally, cannot be filtered this way at all.

**Known limit:** `<ICMPType>` is IPv4-only (`PacketSelector.cc:40-46`), so
`<Protocol>58</Protocol>` drops *all* ICMPv6 matching the selector, not just
Packet Too Big. Scope by address instead, and say so on the page. Extending it
would be ~12 lines in `IPsec.cc` and `PacketSelector.cc`; not needed.

**Side effects for the page:** `hasIpsec = true` adds three visible submodules
under the node's `ipv6` compound — `ipsec`, `spd`, `sad`
(`Ipv6NetworkLayer.ned:125-138`) — and the `ipsec` icon shows a live
ACCEPT/DROP/BYPASS counter, which is a good visual. New statistics appear on
that node (`outDrop`, `outBypass`, `inUnprotectedDrop`, ...). No timing side
effects: BYPASS and DISCARD are instantaneous, and the UDP flow was
bit-identical filtered vs unfiltered.

## The configs

All four use the same network: a host behind a border router, a tunnel across a
transit, a host behind the far border router. The tunnel exists only to make the
path narrower than the links.

Sizes are chosen so the *IPv6 datagram* lands on round numbers, not the
application payload:

```
app payload 1452 B  + 8 (UDP) + 40 (IPv6)  = 1500 B inner datagram
app payload 1412 B  + 8 (UDP) + 40 (IPv6)  = 1460 B inner datagram
tunnel MTU 1460 = 1500 (link) - 40 (outer IPv6 header)
```

| # | app payload | tunnel MTU | ICMPv6 | outcome |
|---|---|---|---|---|
| 1 | 1452 B | 1500 | — | inner fits the tunnel; outer is 1540 and overflows the link; **routerA fragments the outer packet**, far end reassembles |
| 2 | 1452 B | 1460 | filtered | inner exceeds the tunnel; routerA is forwarding so may not fragment; Packet Too Big is sent but never arrives; **black hole** — large packets vanish silently, small ones work |
| 3 | 1452 B | 1460 | delivered | one loss, hostA caches 1460, then **hostA fragments at the source** — the cost moves from the router to the endpoint |
| 4 | 1412 B | 1460 | — | sender already fits; **nothing fragments anywhere** |

Each config changes exactly one thing from the previous.

### The point of the sequence

Config 3 does **not** eliminate fragmentation. With a UDP application still
writing 1452-byte messages, the source has no way to write less, so it fragments
its own datagram to fit 1460. What discovery buys is *placement*: the source
does the work instead of a router in the forwarding path, which is exactly
IPv6's design principle (RFC 8200 forbids routers fragmenting in transit).

Config 4 is therefore essential, not optional — it is the only config with no
fragmentation anywhere, and it is the real destination of the arc.

For TCP the story differs: discovery makes TCP shrink its segments, so no
fragmentation occurs at all. This is why Maximum Segment Size clamping exists
and why it only helps TCP. Worth one sentence in the prose, **not** a config —
INET's TCP is weak on master (see the `feedback_inet_tcp_weak` memory).

## Measurement — what to record

Verified fragment split for config 1 (from `Ipv6::fragmentAndSend`, master):

```
outer datagram 1540 B, link MTU 1500
fragmentLength = ((1500 - 40 - 8) / 8) * 8 = 1448

fragment 1:  40 (IPv6) + 8 (Fragment header) + 1448 data = 1496 B
fragment 2:  40        + 8                   +   52 data =  100 B
                                               total 1596 B  vs 1540 unfragmented

bytes on the wire:   +3.6 %
packets on the wire: +100 %
```

**Packet count is the headline, not bytes.** The second fragment is a runt
carrying 52 bytes of data in a 100-byte frame, and every per-packet cost is paid
in full for it.

Primary evidence: packets on the wire per application packet, config 1 against
config 4. Secondary: goodput against raw throughput (~3.6% header tax).

### Rejected measurements — do not revisit without a new reason

- **Loss amplification.** Losing any fragment destroys the whole datagram, so
  datagram loss is roughly double the frame loss rate. Rejected: this is a wired
  Ethernet scenario, and injecting a percent-level error rate onto Ethernet is
  exactly the contrived parameter the showcase rules forbid.
- **Delay.** Real but a few microseconds on an idle 100 Mbps link. Would need a
  loaded link to be visible; not worth the added complexity.
- **Per-packet processing cost.** This is the cost operators actually care about
  — real routers punt fragmented and reassembled traffic from hardware to
  software forwarding. INET has no per-packet processing budget, so it **cannot
  be demonstrated**. State it as background; do not imply the measured overhead
  is the whole story.

## Verified facts (do not re-derive)

Run against master's build with a probe network
(hostA - routerA - routerT - routerB - hostB, tunnel routerA<->routerB):

- 1440 B payload, tunnel MTU 1500: log shows `Breaking datagram into 2
  fragments`, all 4 packets delivered. Fragmentation at the tunnel entry
  confirmed.
- 1400 B payload, tunnel MTU 1300: 0 packets delivered. The drop path confirmed.
- `Ipv6::fragmentAndSend` logic (Ipv6.cc ~1059):
  `fits -> send` / `!fromHL -> Packet Too Big, drop` / `fromHL -> fragment`.
  `fromHL` is true when the packet has no `InterfaceInd` tag.
- The tunnel entry point is a **router** for the inner packet (may not fragment)
  and the **source** of the outer packet (may fragment). Same node, opposite
  rules, decided by which size limit is exceeded first.
- A host on Ethernet never emits an inner datagram above 1500, and
  `Ipv6TunnelInterface` defaults `mtu` to 1500 — so with defaults the inner
  packet always fits and the drop case is unreachable. The tunnel MTU **must**
  be set for configs 2-4.

## Prior art search — done 2026-09-02, nothing found

IPv6 RFC 8201 Path MTU Discovery exists **nowhere**: not on master, not on any
of ~120 local or ~100 remote branches, not in any worktree or stash, and it has
never been raised as an issue or pull request on `inet-framework/inet`. No
duplication risk.

Searched: GitHub issues and pull requests (open and closed) for path MTU, PMTU,
MTU discovery, Packet Too Big, ICMPv6 MTU, RFC 8201/1981/1191; `git log --all
--grep` across every ref; `git status` in all 19 worktrees plus 20 stashes;
`git grep` for `pathMtu|pmtu|MtuCache` over `src/`.

Adjacent things that do exist:
- **PR #574** "Report MTU when sending an ICMPv4 PTB" — closed **unmerged**
  2020-11-26. Its substance later landed on master by other means.
- **IPv4 send side is complete**: `Ipv4.cc:977-981` calls
  `icmp->sendPtbMessage(packet, mtu)` with the real MTU; `Icmp.cc:161-176`
  builds it, `Icmp.cc:281` extracts it on receipt. **Copy this for the IPv6
  send half.**
- **Neither IPv4 nor IPv6 has an IP-layer per-destination MTU cache.** The cache
  half is genuinely new; there is nothing to mirror.
- `IPV6_MIN_MTU` (1280) already defined at `Ipv6InterfaceData.h:30`.
- Transport-level path MTU discovery already exists and would immediately
  benefit: TCP at `TcpConnectionUtil.cc:398-410` (IPv4) and `:462-472` (IPv6
  Packet Too Big, reduces `snd_mss`), gated by `pmtudEnabled` (default false,
  `Tcp.ned:208`), example `examples/inet/tcp_pmtud/`. QUIC has a full RFC 8899
  implementation at `src/inet/transportlayer/quic/dplpmtud/`, example
  `examples/quic/dplpmtud/`.
- Existing test on master: `tests/module/IPv6_packet_too_big.test` — asserts
  only that the router *emits* the message.
- `origin/topic/av/protocol-test-framework` marks IPv6 path MTU conformance
  tests `EXPECTEDFAIL "NOT-MODELED"`. The gap is known upstream, just unfilled.

## Open questions

1. ~~Category~~ DECIDED: `showcases/ipv6/`, a new subcategory, for both pages.
2. ~~Detail levels~~ DECIDED (applies to BOTH pages): **some** protocol/standards
   background, **more** INET configuration detail (parameters, XML, how to set
   the scenario up), and **no** C++ implementation detail at all. So the pages
   explain the protocol enough to follow, spend their weight on configuration,
   and never describe module internals.
3. Whether the Path MTU Discovery implementation gets its own branch and pull
   request, or is carried on `topic/gy/ipv6-tunneling-showcase` alongside the
   `isWireless` fix (commit 0a93193944) and split at pull-request time. The user
   chose to keep both showcases on this branch; the implementation split is not
   yet decided.

## House rules that apply

- No "In one minute" box, no Details/Fine-print admonitions, no bold figure
  takeaways, no 70-word paragraph budget. Match the pages on master; mechanism
  before results. (`feedback_showcase_house_style`)
- Showcase `omnetpp.ini` stays nearly comment-free; the prose explains. Never
  literalinclude a comment — anchor ranges on config lines.
  (`feedback_showcase_ini_comment_minimalism`)
- Spell out every abbreviation in full on use, repeatedly, not just at first
  use. (`feedback_spell_out_abbreviations`)
- Add a VIDEO/FIGURE RECIPE comment after every figure or video so the capture
  can be redone. (`project_showcase_capture_recipes`)
- No Co-Authored-By or Claude attribution in commits.
  (`feedback_no_coauthored_trailers`)
