# Protocol Test Suite Framework for INET — Design & Implementation Plan

Status: **pending** (design agreed, not yet implemented)
Author: brainstormed with Claude, 2026-06-25

## 1. Purpose

A framework for writing **protocol conformance / behavioural tests** against INET
simulations. A test can do two things, in one unified program:

1. **Match the packet trace** of a running simulation against an expected pattern:
   send/receive **direction**, **which node/interface** sends or receives, **packet
   contents** (any chunk/field at any layer), and **timing constraints**.
2. **Inject arbitrary packets** into the network — crafted frames/segments at any
   layer, at scripted points, *reactively* in response to observed packets.

## 2. Design decisions (locked)

| Decision | Choice | Consequence |
|---|---|---|
| Authoring surface | **C++ builder API** | No DSL parser/grammar to build or maintain. Injection uses the real typed chunk classes; expectations use either INET match-expression strings or type-safe C++ lambda predicates. |
| Injection model | **Full stimulus/response (reactive)** | `inject` steps fire relative to the matcher's position, i.e. in response to observed packets — the packetdrill model. The engine is a live state machine, not an offline batch matcher. |
| Positioning | **Both, CI-first** | Build for the INET regression/conformance suite (opp_test/fingerprint CI) first, but keep the engine usable interactively (Qtenv) by protocol developers. |
| Injection point | **Generic tap/injector** | One `PacketTap` primitive that can act as a man-in-the-middle on any existing gate path *and* as a dedicated test peer node. It can observe, inject, and optionally intercept/drop/mutate. |

## 3. Reference models

- **packetdrill** (Google): a linear script alternating *inject* ("here is a packet
  from the peer") and *expect* ("you should now send this, within +Δ"). Drives a
  conversation. INET once had a port; it is gone from the current tree, but the model
  is the ancestor of our reactive engine.
- **Message-sequence-chart matching**: assert a whole trace contains the right packets,
  directions, endpoints and timing — without necessarily injecting.

**Unification thesis:** both are *one program of ordered steps walked against the live
event stream*. `expect` steps consume observed events under guards; `inject` steps
produce packets. Pure trace-assertion is a program with only `expect`s.

## 4. What INET already gives us (build on, don't reinvent)

Observation:
- `PcapRecorder` pattern — a `cListener` doing wildcard signal subscription
  (`src/inet/common/packet/recorder/PcapRecorder.cc`). Our monitor copies this.
- Packet signals: `packetSentToLowerSignal`, `packetReceivedFromLowerSignal`,
  `packetSentToUpperSignal`, `packetReceivedFromUpperSignal`, `packetDroppedSignal`,
  etc. (`src/inet/common/Simsignals.h`). Each callback yields the `Packet*`, the
  **source `cComponent`** (→ node/module/gate), the **signal id** (→ direction+layer),
  plus `DirectionTag`/`InterfaceTag` on the packet, and `PacketDropDetails` for drops.
- Content matching: `PacketFilter` (`src/inet/common/packet/PacketFilter.h`) already
  dissects a packet and exposes chunk fields as named expression variables via
  `cDynamicExpression`. Reused verbatim for the string-expression flavour of `match`.
- Inspection: `Packet::peekAtFront<T>()`, `peekAll<T>()`, `peekDataAt<T>()`,
  `findTag<T>()`; `PacketDissector` for layer walking.

Injection:
- Gate contract: `IPassivePacketSink::pushPacket(Packet*, const cGate*)` /
  `canPushPacket(...)` (`src/inet/queueing/contract/IPassivePacketSink.h`), driven via
  `PassivePacketSinkRef` (`src/inet/queueing/common/PassivePacketSinkRef.h`) and the
  `pushOrSendPacket(packet, gate, ref)` helper in
  `src/inet/queueing/base/PacketProcessorBase.cc` (falls back to `send()` for classic
  message gates). Pass-through reference: `PacketCloner`.
- Packet construction: normal INET API — `makeShared<TcpHeader>()` + setters,
  `insertAtFront/Back`, `BytesChunk` for truly-arbitrary/malformed payloads.
- Required tags to make an injected frame look "received from lower" at a MAC, etc.:
  `DirectionTag` (`DIRECTION_INBOUND`/`OUTBOUND`), `PacketProtocolTag`,
  `InterfaceReq`/`InterfaceInd`, plus dispatch tags as needed
  (`src/inet/common/DirectionTag.msg`, `ProtocolTag.msg`,
  `src/inet/linklayer/common/InterfaceTag.msg`). See `IpvxTrafGen::sendPacket()`.

Harness:
- `opp_test` `.test` files (`tests/unit/*.test`, `tests/packet/*.test`) with sections
  `%description %includes %global %activity %inifile %contains %not-contains %subst
  %extraargs`. C++ test bodies embed directly in `%activity`. Network-based tests use
  `%inifile` + a NED network from the test lib (`tests/unit/lib/`).

## 5. Architecture overview

```
        test author (C++ builder)                  CI: opp_test .test file
                 │                                            │
                 ▼                                            ▼
        ┌───────────────────┐    builds      ┌──────────────────────────┐
        │  ProtocolTest /   │ ─────────────▶ │   Program (Step graph)   │
        │  builder API      │                └──────────────────────────┘
        └───────────────────┘                           │ loaded by
                                                         ▼
   live signals  ┌──────────────────────────────────────────────────────┐
   ───────────▶  │              ProtocolTester (cListener)               │
   (observe)     │   ┌──────────────┐   advances   ┌───────────────────┐ │
                 │   │ Event stream │ ───────────▶ │  Matching engine  │ │
                 │   │ normaliser   │              │  (NFA + timing)   │ │
                 │   └──────────────┘              └───────────────────┘ │
                 │          ▲                               │ fire inject │
                 │          │ taps                          ▼             │
                 │   ┌──────┴───────┐               ┌───────────────┐     │
                 │   │  PacketTap[] │ ◀──inject──── │   Injector    │     │
                 │   │ (MITM/peer)  │               └───────────────┘     │
                 │   └──────────────┘                                     │
                 │                          verdict (PASS/FAIL + diag)    │
                 └──────────────────────────────────────────────────────┘
```

Four components:
- **`ProtocolTester`** module: the runtime. A `cListener` (observation) + scheduler
  (timing) + the matching engine + the injector. One per test (or per DUT group).
- **Matching engine**: compiles the `Program` into an NFA over normalised events;
  drives transitions with timing guards; fires `inject` actions; produces a verdict.
- **`PacketTap`** module: the generic injection/interception primitive (§9).
- **Builder API**: the C++ surface that constructs the `Program` (§7).

## 6. Core data model

```cpp
// A normalised observation, produced from a signal callback or a tap.
struct PacketEvent {
    Packet *packet;            // the (const) observed packet
    EventKind kind;            // SENT_TO_LOWER, RECEIVED_FROM_LOWER, SENT_TO_UPPER,
                               // RECEIVED_FROM_UPPER, DROPPED, PUSHED, PULLED, ...
    Direction direction;       // INBOUND / OUTBOUND (from DirectionTag or kind)
    cModule *node;             // containing node (resolved from source component)
    cModule *module;           // emitting module
    int interfaceId;           // from InterfaceTag, or -1
    int layer;                 // PHYSICAL/LINK/NETWORK/TRANSPORT/APP (from signal+module)
    simtime_t time;
    long treeId;               // packet identity for cross-node correlation
};

enum class Verdict { PASS, FAIL, INCONCLUSIVE };

// A Step is one node in the program graph (see §7/§8 for the combinators).
struct Step { /* selector + predicate + timing + action + structural links */ };

class Program {            // the compiled test
    std::vector<Step> steps;
    // + entry NFA state(s)
};
```

## 7. The C++ builder API

Goal: read like the protocol conversation. Fluent builder returning references so
steps chain. Two matcher flavours: **expression string** (reuse `PacketFilter`) and
**typed lambda** (type-safe, full power).

### 7.1 Selectors and a minimal example

```cpp
// A TCP 3-way handshake driven from the test acting as the remote peer.
auto t = ProtocolTest("tcp-handshake")
    .attach(tap("client").at("eth0"))                  // where we observe/inject

    // 1. DUT (client) opens the connection: expect a SYN out on eth0 within 1s.
    .expect(on("client").sentToLower().iface("eth0")
              .match("tcp.flags_SYN == true && tcp.flags_ACK == false")
              .within(1.0))

    // 2. Peer (us) replies SYN+ACK, reactively, 10ms after the observed SYN.
    .inject(to("client").iface("eth0").asInbound()
              .after(0.010)
              .frame(
                  EthernetMacHeader().src(peerMac).dest(clientMac),
                  Ipv4Header().src(peerIp).dest(clientIp),
                  TcpHeader().srcPort(80).destPort(lastObserved("tcp.srcPort"))
                             .flags(SYN|ACK).seq(0).ack(lastObserved("tcp.seq")+1)))

    // 3. Expect the final ACK, within 0.5s, with the right ack number.
    .expect(on("client").sentToLower().iface("eth0")
              .match([](const PacketEvent& e){
                  auto tcp = e.packet->peekAtFront<TcpHeader>();
                  return tcp->getAckBit() && !tcp->getSynBit();
              })
              .within(0.5))

    // 4. Negative: no RST should appear in the next 0.5s.
    .never(on("client").sentToLower().match("tcp.flags_RST == true").within(0.5));
```

### 7.2 Selector grammar (builder methods)

- Endpoint: `on(node)` (observe at) / `to(node)` (inject toward) / `from(node)`.
- Refine: `.iface("eth0")`, `.module("...relative.path")`, `.layer(NETWORK)`.
- Direction/kind: `.sentToLower()`, `.receivedFromLower()`, `.sentToUpper()`,
  `.receivedFromUpper()`, `.dropped()` (optionally `.reason(NO_ROUTE_FOUND)`),
  or generic `.direction(OUTBOUND)`.

### 7.3 Content predicates

- String: `.match("ipv4.destAddress == 10.0.0.1 && udp.destPort == 5000")` — compiled
  through `PacketFilter`'s `cDynamicExpression` resolver. Familiar to INET users.
- Lambda: `.match([](const PacketEvent& e){ ... })` — full typed access via
  `peekAtFront<T>()`, tags, region tags. Use for cross-field logic the expression
  language can't express.
- Captures: `lastObserved("tcp.seq")` and named captures `.capture("isn","tcp.seq")`
  let later inject/expect steps reference values seen earlier (essential for seq/ack
  numbers, ports, addresses negotiated at runtime).

### 7.4 Packet construction for `inject`

```cpp
.frame(chunkA, chunkB, ...)   // variadic; front→back order, real INET chunk objects
.bytes({0xde,0xad,...})       // raw BytesChunk for malformed/fuzz packets
.fromTemplate(pkt)            // clone & mutate a captured/observed packet
.tag<SomeTag>(...)            // override/add tags
.asInbound()/.asOutbound()   // sets DirectionTag (+ Interface{Ind,Req}) appropriately
```

The builder helper sets the mandatory tags (`DirectionTag`, `PacketProtocolTag`,
`InterfaceInd`/`InterfaceReq`) from the selector so the DUT processes the frame as a
genuine arrival at the chosen layer/interface.

### 7.5 Timing modifiers (per step)

- `.at(t)` absolute; `.after(Δ)` relative to the previous matched/fired step;
  `.within(Δ)` deadline (fail if not satisfied in time); `.notBefore(Δ)` earliest;
  `.tolerance(±ε)`. Negative windows for `never`.

### 7.6 Structural combinators (regex-over-packets)

```cpp
.sequence(a, b, c)          // ordered (default between consecutive steps)
.unordered({a, b})          // any interleaving; all must occur
.anyOf({a, b})              // alternation
.delivery(from("A"), to("B"), within)   // correlated send@A → receive@B by treeId

// Cardinality family (regex-quantifier style). Fixed-count kinds advance as soon as the
// count is reached; the greedy *Times kinds consume every match within the within()
// window, then check the range (an overflow past the upper bound fails immediately).
// Parameterized kinds take the count first.
.never(a)                   // 0      (must NOT occur in the window — negative)
.once(a)                    // 1      (the default between consecutive steps)
.atMostOnce(a)              // 0..1   (match-or-skip)
.oneOrMoreTimes(a)          // 1..*   (greedy)
.anyNumberOfTimes(a)        // 0..*   (greedy)
.exactlyTimes(n, a)         // n
.atLeastTimes(n, a)         // n..*   (greedy)
.atMostTimes(n, a)          // 0..n   (greedy)
.betweenTimes(a, b, sel)    // a..b   (greedy)
```

`.delivery(...)` is sugar over two primitive events correlated by `treeId` — expresses
end-to-end "A sends, B receives the same packet" cleanly.

### 7.7 Verdict

The program ends in `PASS` when all required steps are satisfied, `FAIL` on a mismatch,
deadline miss, or a triggered negative; `INCONCLUSIVE` if the simulation ends with
required steps still pending. The tester calls `endSimulation()` and prints a
structured verdict line (consumed by `%contains` in CI, §10).

## 8. Matching engine semantics

- Compile `Program` → **NFA**: combinators map to standard regex constructions
  (sequence=concat, anyOf=alternation, optional/repeat=ε-moves). Multiple states may be
  live at once (handles `unordered`, `optional`, concurrent flows).
- Each incoming `PacketEvent` is offered to all live transitions; a transition fires if
  selector + content predicate + timing guard all hold. Captures bind on fire.
- **Ordering**: default is a per-program sequence, but selectors implicitly scope a
  step to a flow (node+iface+direction), so independent flows interleave naturally;
  `unordered{}` relaxes ordering explicitly. This avoids the brittleness of a strict
  global total order on a concurrent trace.
- **Timing** is implemented with OMNeT++ self-messages: a `within(Δ)` arms a deadline
  timer when its predecessor fires; expiry → `FAIL`. `notBefore`/`after` gate the
  earliest fire time. Negative windows arm a timer that PASSES on expiry and FAILS on
  match.
- **Unmatched events**: by default ignored (open-world: the trace may contain unrelated
  packets). A `strict()` mode can flag any event matching a step's selector-scope but
  not its predicate as a failure (closed-world, for tight conformance).

## 9. The `PacketTap` injection/interception primitive

A reusable NED+C++ module implementing both queueing contracts
(`IPassivePacketSink` + `IActivePacketSource`, like `PacketCloner`) so it can be spliced
into any gate path. Modes:

- **Observe (pass-through)**: forwards every packet unchanged, emitting a normalised
  `PacketEvent` to the `ProtocolTester`. (For points already covered by signals, the
  tester can use signal subscription instead and skip the tap.)
- **Inject**: on command from the engine, `pushPacket()` a crafted packet downstream
  (or upstream), with the tags set per §7.4, using `PassivePacketSinkRef` /
  `pushOrSendPacket`.
- **Intercept (MITM)**: optionally drop, delay, duplicate, or mutate passing packets —
  enables fault-injection / adversarial tests.

Two deployment shapes, both supported by the same module:
- **MITM**: inserted between two existing modules' gates (e.g. between a host's MAC and
  PHY) via a small NED insertion, or attached to a node by module path at runtime.
- **Peer node**: a standalone node wired into the topology that the test drives as the
  remote endpoint (packetdrill style).

Resolution: the builder's `tap(node).at(iface)` / `attach(...)` records the desired
attachment; the `ProtocolTester` resolves module paths and either binds to existing
`PacketTap` instances declared in the NED or subscribes to the relevant signals.

## 10. Harness & CI integration (CI-first)

Each protocol test is a **self-contained simulation**: a NED network containing the DUT
node(s), any `PacketTap`s / peer nodes, and one `ProtocolTester`. The `Program` is built
in C++.

Two authoring routes, both run by `opp_test`:
1. **`.test` file** (preferred for CI): `%inifile` selects the network; the `Program`
   is built either in a `%global`/`%activity` body or returned by a registered builder
   function the `ProtocolTester` looks up by name (test name → `Program`). The verdict
   line is checked with `%contains: PROTOCOLTEST: PASS`. Lives under
   `tests/protocol/` (new dir) alongside the existing `tests/packet`, `tests/fingerprint`.
2. **Registered C++ test** (for developer/interactive use): a
   `Define_ProtocolTest(name, builderFn)` macro registers builders into a registry; a
   runner network/ini enumerates and runs them. Same engine, runnable in Qtenv for
   step-by-step inspection.

Fingerprint compatibility: because tests are deterministic simulations, they also
produce fingerprints, so they slot into the existing fingerprint CSV CI as a second
safety net.

## 11. Determinism

Protocol tests must be reproducible. Pin RNG seeds, use fixed timing, avoid wall-clock
dependence. This dovetails with the repeatable-tests / resolve-in-place work already in
opp_ci. Document a "deterministic test" checklist; provide a base ini fragment
(`protocoltest-base.ini`) that sets seeds and disables nondeterministic features.

## 12. Diagnostics (make failures debuggable)

On `FAIL`, emit a structured report: which step, its selector + predicate (pretty
form), the offending/observed event (node, iface, direction, time, `PacketPrinter`
dump of the packet), and for timing failures the deadline vs actual. Optionally dump a
PCAP of the window around the failure (reuse `PcapWriter`) and the live NFA state for
post-mortem. In Qtenv, annotate the failing packet.

## 13. Test description generation (English rendering)

Every `Program` can be **algorithmically translated to an English description** by a
deterministic walk of the step graph. This is cheap (no runtime, pure AST → text) and
pays off three ways: living documentation, a CI **golden file** (diff the generated
description on every change), and **Qtenv hover-text** on the running test.

### 13.1 Algorithm

Pre-order traversal of the `Program` graph; each node emits one clause from a fixed
template; field expressions pass through a small **phrasebook**. Because every clause
traces back to exactly one builder call, the mapping is 1:1 and stable.

```
render(Program p):  "Test \"p.name\"" + render-attach(taps) + numbered render(steps)

render(step):
  expect(on N .KIND .iface I .match M .within T)
      → "Within T, N must {KIND} on I a packet that {phrase(M)}{captures}."
  inject(to N .iface I .asInbound .after D .frame C...)
      → "{D} later, send to N on I (appearing as a received packet): {phrase(C...)}."
  never(sel .within T)
      → "For the next T, N must not {KIND} {phrase(M)}."
  delivery(from A, to B, within T)
      → "That packet must reach B within T."
  onMatch(sel, action)
      → "When {phrase(sel)}, {phrase(action)}."

KIND:    sentToLower→"send out"  receivedFromLower→"receive"  dropped→"drop" ...
capture: .capture("x", F)  → " (remember its F as x)"
use:     use("x")          → "the remembered x"
action:  drop()→"drop it"  delay(Δ)→"hold it for Δ"  duplicate()→"duplicate it"
```

Combinators become connectives:

| Combinator | Rendering |
|---|---|
| `sequence` | numbered list (default) |
| `unordered{a,b}` | "in any order: a; b" |
| `once(a)` | "a must …" (the default step) |
| `atMostOnce(a)` | "a may …" |
| `never(a)` | "a must not …" |
| `exactlyTimes(n,a)` | "n times: a" |
| `atLeastTimes(n,a)` / `oneOrMoreTimes` | "at least n times: a" / "one or more times: a" |
| `atMostTimes(n,a)` / `betweenTimes(a,b,…)` | "at most n times: …" / "between a and b times: …" |
| `anyNumberOfTimes(a)` | "any number of times: a" |
| `anyOf{a,b}` | "either a, or b" |
| `delivery` / `onMatch` | atom / "When …" side-rule |

### 13.2 Phrasebook (field-expression → English)

A table maps the expression namespace to prose, e.g. `tcp.flags_SYN` → "with SYN set",
`!tcp.flags_ACK` → "with ACK clear", `ipv4.destAddress == X` → "destined to X",
`icmpv4.type == ECHO_REQUEST` → "carrying an ICMPv4 Echo Request",
`arp.opcode == ARP_REQUEST` → "an ARP Request". The phrasebook is per-protocol and
extensible; an unknown term falls back to the raw expression in quotes (never fails).
**Lambda `match` predicates** can't be introspected, so a lambda step carries an
optional `.describe("...")` string the renderer uses (falling back to
"a packet matching a custom predicate").

### 13.3 Example output

For `tcp-handshake` the renderer produces:

> Test "tcp-handshake" (observing and injecting at client, eth0):
> 1. Within 1 s, client must send out on eth0 a TCP segment with SYN set and ACK clear
>    (remember its sequence number as `isn`, its source port as `cport`).
> 2. 10 ms later, send to client on eth0 (appearing as a received packet): Ethernet
>    peerMac→clientMac / IPv4 peerIp→clientIp / TCP 80→the remembered `cport`, SYN+ACK
>    set, seq 1000, acking the remembered `isn` + 1.
> 3. Within 0.5 s, client must send out on eth0 a TCP segment with ACK set, SYN clear,
>    ack number 1001.
> 4. For the next 0.5 s, client must not send a TCP segment with RST set.

### 13.4 Integration

- `ProtocolTest::describe()` returns the string; a `--describe` run mode prints it
  without simulating.
- CI: an optional `%contains`/golden-file check on the description catches unintended
  semantic changes to a test.
- Qtenv: show the description in an inspector panel, highlighting the clause whose step
  is currently live in the NFA.

## 14. Implementation roadmap

Each phase is a milestone with its own commit(s); work in a dedicated worktree.

- **Phase 0 — Skeleton & event normaliser. ✅ DONE.** `ProtocolTester` module subscribing
  to the packet signals (PcapRecorder-style), producing `PacketEvent`s; node/iface/
  direction/layer resolution; a trivial "log all events" mode. Validates observation
  end-to-end. *Exit:* run on an INET example, print a correct normalised trace.
  - Implemented in `tests/protocol/lib/` (`PacketEvent.h`, `ProtocolTester.{h,cc,ned}`,
    a two-host UDP demo, `build.sh`/`run-demo.sh`). Builds against a pre-built INET at
    the same commit (no full INET rebuild in the worktree).
  - Verified: clean 36-event trace over a UDP exchange; node/module/kind/layer correct;
    **treeId is stable across nodes** (confirms the `delivery()` correlation primitive).
  - Findings feeding later phases: (a) direction must come from the **signal kind**, not
    a `DirectionTag` (absent at these points) — fixed; (b) `InterfaceInd` is absent on
    `receivedFromLower` at the MAC — interface resolution must tolerate `-1`; (c) interface
    *name* resolution and precise (non-heuristic) layer classification are deferred to
    Phase 1 where selectors need them; (d) some modules emit a signal twice (e.g. `udp`
    `receivedFromLower`) — matching must be robust to duplicate emissions.

- **Phase 1 — Matching engine (expect-only, sequential). ✅ DONE.** `Program`/`Step`,
  builder for `expect` with selectors + string `match` + `.within/.notBefore`. Sequential
  timing-guarded matcher, timing via self-messages, verdict + diagnostics. *Exit:* a
  correct UDP pattern passes; a deliberately-wrong pattern fails with a clear report.
  - Implemented: `EventPattern` (fluent selector+timing, `PacketFilter` content
    expression), `ProtocolTest` program + name→builder registry + `Define_ProtocolTest`
    macro, and the engine in `ProtocolTester` (open-world: non-matching events ignored;
    `within` deadline via self-message; PASS when all steps match, FAIL on deadline miss
    or sim-end-with-pending; `endSimulation()` once decided). `udp_basic_pass` →
    `PASS`; `udp_basic_fail` → `FAIL` with `deadline missed for step 0 [on host1
    sentToLower match='udp.destPort == 9999' within=0.3]`.
  - The two-stage `delivery()` design is corroborated: treeId-stable UDP datagram
    matched first at `host1.udp sentToLower` then at `host2.udp receivedFromLower`.
  - Findings: (a) **context** — `receiveSignal` runs in the *emitting* module's context;
    the engine must `Enter_Method_Silent()` before `scheduleAt`/self-message ownership;
    (b) **expression robustness** — a content expression naming a protocol absent from a
    packet (`udp.*` on ARP) *throws*; caught and treated as non-match → **selectors should
    constrain layer/kind** so expressions see relevant packets. Caveat: this also means a
    *typo'd field name* silently never-matches; loud typo detection is a later refinement.
  - Deferred to Phase 7: wrapping these as formal `opp_test` `.test` files. Phase 1
    verification used `opp_run -c Pass/Fail/Trace` configs (equivalent).

- **Phase 2 — Lambda predicates & captures. ✅ DONE.** Typed `match` lambdas; `.capture`
  + read via context. *Exit:* assert seq/ack relationships in a TCP trace.
  - Implemented: `MatchContext{event, captures}`, a `match(MatchPredicate)` overload
    (lambda `bool(const MatchContext&)`) beside string `match`, and `capture(name, fn)`
    binding a `cValue` from the matched packet; the engine threads a `CaptureStore` and
    evaluates captures on match. Captures are **string-keyed** (read with `ctx.get("isn")`).
    `tcp_handshake_seq` → PASS (asserts `ackNo==isn+1`, `ackNo==peerIsn+1` across the two
    endpoints); `tcp_handshake_seq_bad` → FAIL (wrong `isn+2`).
  - **Major finding (signal coverage) — RESOLVED:** INET's `Tcp` emitted **no
    `packetSentToLower`** (it routed segments to IP via a raw `send()` in `sendToIp()`),
    so a node's outgoing TCP segments were not observable at the transport layer. Per the
    §15 policy this was fixed by **adding the signal to `Tcp`** (emit in `sendToIp` +
    `@signal` in `Tcp.ned`, mirroring `Udp`) — a minimal INET edit, approved. The handshake
    tests now observe the initiator's SYN/ACK naturally at `tcp sentToLower`. (Originally
    worked around by asserting from incoming segments.)
  - Robustness: lambda predicates are wrapped in try/catch like string expressions (a
    peek of an absent chunk is a non-match). Lambda bodies are opaque in diagnostics
    (the bad-handshake FAIL shows only the selector) — `.describe()` / Phase 8 will fix.
  - `use()` (reading captures inside *inject* field values) is deferred to Phase 3 where
    injection exists; in Phase 2 captures are consumed only inside predicates.
  - **Refinement (declarative matching, no lambdas):** to keep test *matching* free of C++
    lambdas, two facilities were added and all tests rewritten to use them: (a) string
    captures `capture("isn", "tcp.sequenceNo")` via `evalPacketField()` (dissect +
    class-descriptor read, same path as `PacketFilter`); (b) `{name}` placeholders in a
    `match` expression substituted with an earlier captured value, e.g.
    `match("tcp.ackNo == {isn} + 1")`. Lambda `match`/`capture` overloads remain for
    advanced use but are no longer used by the sample tests. Packet *construction* for
    injects still uses (named) builder functions — that is not "testing".

- **Phase 3 — Injection (time-scheduled). ✅ DONE.** Builder `inject(node).into(module,
  gate).at(t).packet(fn)`, tag setup, push path. *Exit:* inject a UDP packet that the DUT
  receives and responds to; assert the response.
  - Implemented: `Injection` model + `ProtocolTest::inject()`; the engine schedules
    injections as self-messages and delivers via the **queueing push contract**
    (`IPassivePacketSink::pushPacket`). `udp_inject_echo` crafts an IP/UDP datagram, pushes
    it up `host2.eth[0].upperLayerOut`; host2's `UdpEchoApp` receives it up the full stack
    (interface→IP→UDP→app) and echoes it — asserted and **PASS** (trace confirms the causal
    chain, single treeId throughout).
  - **Key findings (injection realism — the hard part the plan flagged):**
    (a) `sendDirect` only works to **unconnected** gates, so it can't target `udp.ipIn`;
    the robust seam is `pushPacket()` (a method call, no gate-connection constraint) on the
    interface's `upperLayerOut` — the same inbound path INET emulation uses.
    (b) Routing up requires the exact tags a decapsulated frame carries: `PacketProtocolTag`
    **plus** `DispatchProtocolReq(protocol, SP_INDICATION)` — without the `SP_INDICATION`
    service primitive the `MessageDispatcher` rejects it as "Unknown packet".
    (c) Header validity matters: `checksumMode = CHECKSUM_DECLARED_CORRECT` on IP+UDP avoids
    drops. → argues for a per-layer **inject-helper** library (Phase 6/7) so authors don't
    hand-assemble tags. `.frame(...)` chunk-sugar and `use()`-in-inject also deferred there.
  - **Scope note:** the inline-MITM `PacketTap` module is deferred to Phase 6 (where
    intercept/drop needs inline placement); Phase 3 injection is engine-driven via
    `pushPacket`, which needs no topology changes.

- **Phase 4 — Reactive injection (stimulus/response). ✅ DONE.** Wire `inject` into the
  engine so it fires relative to matcher position; full packetdrill-style conversation.
  *Exit:* drive a TCP 3-way handshake against INET's TCP, peer emulated by the test.
  - Implemented: `inject` unified into the step sequence (`Step` is `Expect` or `Inject`);
    the engine's `enterStep()` arms an expect deadline or schedules an inject. Inject timing
    `.at(t)` absolute or `.after(d)` relative to the previous match (reactive). The inject
    builder reads the `CaptureStore` → **`use()`-in-inject** now works.
  - `tcp_handshake_peer` → **PASS**: host1 actively opens to a **phantom IP** (10.0.0.99,
    ARP-blackholed so no real peer competes); the test observes the SYN at `ipv4.ip
    receivedFromUpper` (capturing ISN + ephemeral port), reactively injects a crafted
    SYN+ACK acking ISN+1, and observes host1's final ACK acking the peer ISN+1. Trace
    confirms SYN(dropped)→injSYNACK→TcpAck(dropped) — a real three-way handshake driven
    entirely by injection. Phase 3's time-scheduled inject is now an inject step with `.at()`.
  - Findings: (a) the **phantom-IP** pattern lets the test be the sole peer without fighting
    a real responder — the SYN/ACK leave the stack before ARP drops them, so they are
    observed regardless; (b) reactive injection needs the inject to fire as a *future*
    self-message (never re-entrantly in the matching callback) — `enterStep` schedules it,
    matching the plan's §14 caution; (c) `TcpSessionApp` requires `sendBytes>0` even when
    only the handshake is under test. (Originally observed host1's segments at the IP layer;
    once `Tcp` gained `packetSentToLower` (see Phase 2 finding), `tcp_handshake_peer` was
    switched to observe at `tcp sentToLower` like `tcp_handshake_seq`.)

- **Phase 5 — Combinators. ✅ DONE.** `unordered/anyOf/delivery`, the cardinality family
  (`never/once/atMostOnce/exactlyTimes` + greedy `oneOrMoreTimes/anyNumberOfTimes/
  atLeastTimes/atMostTimes/betweenTimes`), `strict()` mode. *Exit:* a multi-flow test with
  negative assertions — met.
  - Engine generalised from a single cursor to a group/stage-aware matcher: `enterStep`
    arms a window per kind; deadline expiry means fail / skip / pass by kind.
    - `never` — negative window (fail on match, else advance); `atMostOnce` — match-or-skip;
      `unordered({...})` — all patterns, any order (window = longest sub-`within`); `anyOf({...})`
      — first matching alternative wins; `exactlyTimes(n, p)` — advance on the nth match within
      the window; `delivery(from, to, window)` — send correlated to receive of the **same packet
      by treeId**, window measures send→receive latency (armed at the send); `strict()` —
      closed-world, an in-scope-but-wrong packet fails a `Once` step (`EventPattern::scopeMatches`
      splits scope from content). `finish()` treats trailing `Never`/`AtMostOnce`/range-satisfied
      `Count` as satisfied.
    - **Greedy `Count` kind** (added in the cardinality rename): models a count range
      `[cardMin, cardMax]` (`cardMax < 0` = unbounded). It consumes *every* match within the
      `within()` window, fails immediately on an overflow past `cardMax`, and at the window
      close passes iff `cardCount ≥ cardMin`. Fixed-count kinds (`once`, `exactlyTimes`) keep
      their advance-on-reach semantics so ordered sequences are unaffected.
  - Describer extended for every kind: "must/may/must not", "In any order: …", "One of: …",
    "N times: …", the greedy "at least/at most/between/any number of times: …", and the
    delivery sentence.
  - Verified (15 configs total): `multi_flow`, `udp_delivery`, `udp_anyof`, `udp_repeat`,
    `udp_count_pass`, `optional_skip` PASS; `multi_flow_bad` FAIL (`forbidden event occurred at
    t=0.1 …`), `udp_strict_bad` FAIL (`strict: unexpected in-scope packet …`),
    `udp_count_overflow` FAIL (`count step 0: more than 2 occurrence(s) …`).
  - **Naming note:** `expect/optional/expectNo/repeat(p,n)` were renamed to
    `once/atMostOnce/never/exactlyTimes(n,p)` (count-first) for a single, consistent
    cardinality vocabulary; no back-compat aliases (all call sites are in-repo).
  - **Deferred (not blocking):** explicit auto **per-flow scoping** (concurrent flow-cursors)
    — the most NFA-heavy piece; `unordered` already covers explicit interleaving, and
    specific selectors keep flows separated. `repeatUntil(cond)` also deferred.

- **Phase 6 — MITM/intercept & peer-node shapes.** `PacketTap` drop/delay/mutate;
  standalone peer node wiring; fault-injection example. *Exit:* a retransmission test
  driven by dropping a frame.

- **Phase 7 — Harness, CI, docs.** `tests/protocol/` dir, `Define_ProtocolTest` registry,
  runner network, fingerprint hookup, `protocoltest-base.ini`, authoring guide +
  cookbook (handshake, retransmission, ARP, DHCP, fragmentation examples).

- **Phase 8 — Description generator (§13). ✅ DONE (early, brought forward before Phase 5).**
  AST→English renderer, phrasebook, `describe()`. Qtenv panel / golden-file CI hook remain
  for Phase 7.
  - Implemented `ProtocolTestDescriber` (`describe(const ProtocolTest&)`): a phrasebook maps
    event kinds (sentToLower→"send a packet to the lower layer", …) and layers to prose;
    timing (`within`/`after`/`at`), captures ("remembering isn, clientPort"), and inject
    targets are rendered; an optional `.describe("…")` on `EventPattern`/`Injection` supplies
    readable text for otherwise-opaque lambda predicates and packet builders (fixes the
    lambda-opacity limitation). `ProtocolTester` prints it at startup (`printDescription`).
  - Sample output (tcp_handshake_peer): "1. Within 1s, host1 must receive a packet from the
    upper layer at the network layer -- host1's SYN (SYN set, ACK clear), remembering isn,
    clientPort. 2. 0.001s after the previous step, inject a SYN+ACK acknowledging host1's
    ISN+1 into host1's eth[0] (upperLayerOut). 3. …host1's final ACK…".
  - As each later phase adds constructs (combinators, intercept actions), it extends the
    renderer with their templates.

## 15. Risks & open questions

- **Signal coverage vs taps — RESOLVED (policy).** Not every interesting observation
  point emits a signal. Policy when a step needs an observation the model doesn't expose:
  **prefer adding/emitting a signal in the relevant module over inserting a tap.**
  Rationale: signals are INET's idiomatic, *passive* observation mechanism (no effect on
  packet flow or timing) and are reused by statistics/visualizers/other tests, whereas a
  `PacketTap` is *in-band* — it alters the NED topology away from the production network
  and can perturb behaviour. Mechanics:
  - At program-build/resolve time the framework checks that each `once`/`never`
    selector maps to an available signal; if not, it **fails fast with a concrete
    suggestion** — the exact module path and a recommended signal (name + emission site)
    to add — rather than silently substituting a tap.
  - **Caveat 1:** this applies to *observation* only. `inject` and `onMatch`/intercept
    still require a `PacketTap` regardless — a signal cannot inject or drop a packet.
  - **Caveat 2:** for modules that genuinely cannot be modified (third-party/closed),
    an **observe-only tap remains the fallback**, selected explicitly by the author
    (e.g. `tap(node).at(iface).observeOnly()`), never automatically.
  - Side benefit: this drives INET protocol modules toward consistent, reusable
    observation-signal coverage as a standing model-hardening effort.
- **Injection realism.** Getting all the tags right so an injected frame is processed
  identically to a real arrival is fiddly and protocol-specific; build a per-layer
  "inject helper" table (L2/L3/L4) and validate against captured real arrivals.
- **Reactive timing races.** "Inject 10ms after observed SYN" interacts with the
  scheduler; ensure injects are scheduled as future events, never re-entrantly inside a
  signal callback (`Enter_Method` discipline).
- **Open-world vs strict matching default.** Start open-world (ignore unrelated
  packets); make `strict()` opt-in.
- **Capture/expression coupling.** The `cDynamicExpression` resolver field names must be
  stable across protocols; document the available variable namespace per protocol.
- **Concurrency in the NFA.** Per-flow scoping needs a clear rule for which flow a step
  binds to; risk of ambiguous matches — define precedence (most-specific selector wins)
  and detect/report ambiguity.
```
