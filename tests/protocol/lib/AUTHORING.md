# Protocol Test Framework — Authoring Guide & Cookbook

A framework for writing **protocol conformance / behavioural tests** against INET
simulations. A test does two things in one program: it **matches the packet trace** of a
running simulation against an expected pattern (direction, node/interface, packet contents,
timing), and it can **inject** crafted packets or **intercept** (drop/delay/mutate) frames
in flight. Tests are plain C++ — no DSL, no parser.

This guide is the authoring reference. For the design rationale see
[`plan/pending/protocol-test-framework.md`](../../../plan/pending/protocol-test-framework.md).

---

## 1. Anatomy of a test

A test is a registered builder function returning a `ProtocolTest`:

```cpp
Define_ProtocolTest(udp_basic_pass)
{
    return ProtocolTest("udp_basic_pass")
        .once(on("host1.udp").signal("packetSentToLower")
                  .filterPacket("udp.destPort == 5000").within(0.2))
        .once(on("host2.udp").signal("packetReceivedFromLower")
                  .filterPacket("udp.destPort == 5000").within(0.1));
}
```

- `Define_ProtocolTest(id)` registers the program under the name `id`.
- A `ProtocolTester` module placed at the top of the network runs the program named by its
  `testName` parameter, walks it against the live event stream as an ordered, timing-guarded
  matcher, and emits a verdict line: `PROTOCOLTEST <name>: PASS` (or `FAIL` with a reason).
- With no `testName`, the tester just logs the normalised event trace (authoring aid).

Each step is built from a **selector** (`on(...)...`) plus a **cardinality** wrapper
(`once`, `never`, ...). Steps are ordered by default; the selector's `within(t)` bounds how
long the step waits.

---

## 2. Selectors — which event

`on("path")` names the module (sub)tree to observe; chain to narrow it. There is **one
functional per concern**:

| Clause | Concern | Meaning |
|---|---|---|
| `on("host1")` / `on("host1.wlan[0].mac")` | where (subscribe) | module path to observe — a node, or a specific submodule (matched as a path prefix on the emitter) |
| `.source("host1.eth[0].mac")` | where (emitter) | narrow to a specific emitting module within `on` |
| `.signal("packetSentToLower")` | which signal | packet signals, or scalar signals like `controlStateChanged` / `curID` (§8) |
| `.protocol("mobileipv6")` | packet protocol | the packet's `PacketProtocolTag` |
| `.dispatch("ipv4")` | dispatch protocol | the packet's `DispatchProtocolReq` (where it's headed) |
| `.iface("eth0")` | interface | restrict to an interface |
| `.filterPacket("expr")` | packet content | content predicate over the packet (PacketFilter, §4) |
| `.filterEvent([](const MatchContext& c){ ... })` | content (lambda) | typed content predicate |
| `filterValue(v)` | scalar value | a scalar signal's value, e.g. an FSM state index (§8) |
| `.attributeTo("host1.ipv6.mipv6")` | narration POV | description point of view only — never affects matching (§8b) |
| `.describe("phrase")` | narration | human phrase (rarely needed — `packet()` auto-translates) |
| `.capture("name", "proto.field")` | capture | remember a field value for later steps (§4) |
| `.within(t)` | timing | deadline: satisfy the step within `t` of its anchor |
| `.after(t)` / `.notBefore(t)` | timing | earliest time, relative to the previous step's match |

`on("host1")` matches `host1` and any descendant; `on("host1.udp")` pins the transport module.
Choose the path that scopes the observation (this is how you target a "layer" — by its module
path, e.g. `host1.udp`, `host1.eth[0].mac`, `host1.wlan[0].mac`).

The *anchor* of a step is when the previous step matched (the simulation start for the
first step). `within`/`after` are measured from it.

---

## 3. Cardinality — how many times

Every step wraps a selector in a cardinality (regex-quantifier vocabulary). Count argument
comes first for the parameterized ones.

| Builder | Count | Notes |
|---|---|---|
| `never(p)` | 0 | fail if a match occurs in the window, else advance |
| `once(p)` | 1 | advance on the first match (the common case) |
| `atMostOnce(p)` | 0..1 | match-or-skip |
| `nextTimes(n, p)` | the next n | advance on the nth match; **does not forbid an n+1th** |
| `oneOrMoreTimes(p)` | 1..∞ | greedy: consume the whole `within` window, need ≥1 |
| `anyNumberOfTimes(p)` | 0..∞ | greedy: consume the window |
| `atLeastTimes(n, p)` | n..∞ | greedy: consume the window, need ≥n |
| `atMostTimes(n, p)` | 0..n | greedy: fail on the (n+1)th |
| `betweenTimes(a, b, p)` | a..b | greedy: need the count in [a, b] |
| `exactlyTimes(n, p)` | n..n | greedy: fail at once on an n+1th, and on fewer than n when the window closes |

**Sequencing** kinds (`once`, `atMostOnce`, `nextTimes`) advance the instant the count is
reached, so they don't disturb the timing of later steps. **Cardinality** kinds
(`oneOrMore`, `atLeast`, `atMost`, `between`, `exactlyTimes`, `anyNumber`) consume *every*
matching frame until their `within` window closes, then check the range.

**Read `nextTimes` and `exactlyTimes` carefully; they are not the same rule.** `nextTimes(3,
p)` takes the next three matches and hands the fourth to the step after it.
`exactlyTimes(3, p)` says there are three in the window and no more, so a fourth fails. The
two shared the name `exactlyTimes` until 2026-09-14, and the cardinality was the one that did
not exist: a check that said "exactly three" got "the next three" and passed over everything
after them. `self/Repeat.test` and `self/NextTimes.test` cover the two.

A cardinality step is greedy, so size its `within` to end before the next expected frame, or
put it in `meanwhile(...)` and let it run beside the steps that follow.

---

## 4. Content matching & captures

### A filter picks the event, an assertion judges it

Every word that compares says which of the two it is. **A filter picks the event**: when
nothing matches, the step waits and finally misses its deadline. **An assertion must hold on
the event the filter picked**: when it does not, the step fails at once and the engine never
looks for a later event.

| | Filter — picks | Assertion — must hold |
| --- | --- | --- |
| expression over the packet | `filterPacket(e)` | `assertPacket(e)`, `assertNotPacket(e)` |
| predicate over the event | `filterEvent(f)` | `assertEvent(f)` |
| scalar equality | `filterValue(v)` | `assertValue(v)`, `assertNotValue(v)` |
| scalar bound | `filterValueAtLeast(v)`, `filterValueAtMost(v)` | `assertValueAtLeast(v)`, `assertValueAtMost(v)` |
| scalar range | `filterValueBetween(lo, hi)` | `assertValueBetween(lo, hi)` |

Position words compare nothing and stay bare: `first()`, and `nth(k)`. `first()` is `nth(1)`.

**Write a rule as an assertion, not as a filter.** This is the difference between a check
that can fail and one that cannot:

```cpp
// wrong: picks the first window that is small enough, and passes over one that is not
.once(on("host1.tcp").signal("cwnd").filterValueAtMost(IW_BOUND).within(0.5))

// right: the first publication is the subject, and the bound is the verdict
.once(on("host1.tcp").signal("cwnd").first().assertValueAtMost(IW_BOUND).within(0.5))
```

There is deliberately no `assertNotThat`: a lambda negates itself. `assertNotPacket` is not
redundant in the same way, because it differs from `assertPacket` of a negated expression when
the chunk is **absent**.

`assertValueAtLeast` does not collide with `atLeastTimes`: the cardinality family carries the
`Times` suffix.

### The expression engine

`filterPacket("expr")` uses INET's `PacketFilter` expression engine over the dissected packet
(it asserts the signal value is a packet; for a scalar signal use `filterValue(v)` or one
of the bounds instead).
Protocol names are lowercase (`tcp`, `udp`, `ipv4`, `arp`, `ieee80211mac`), chunk class
names are as declared (`BindingUpdate`, `Ieee80211DataHeader`). Examples:

```
udp.destPort == 5000
tcp.synBit == true && tcp.ackBit == false
ipv4.moreFragments == true
arp.opcode == 1
ieee80211mac.type == 24
```

An expression that doesn't apply to a frame (e.g. `tcp.*` on an ARP frame) is treated as a
non-match, never an error.

**Captures** remember a value when a step matches; later steps reference it with
`{name}` substitution:

```cpp
.once(on("host1.tcp").signal("packetSentToLower")
          .filterPacket("tcp.synBit == true")
          .capture("isn", "tcp.sequenceNo"))           // remember the ISN
.once(on("host1.tcp").signal("packetReceivedFromLower")
          .filterPacket("tcp.ackNo == {isn} + 1"))           // refer back to it
```

For predicates the engine can't introspect, use a lambda plus `.describe("...")` so the
English rendering stays readable.

A captured field may carry a unit — the SYN's header length is `24B`, not `24`. That works:
the capture keeps its quantity form and the expression engine compares it, so
`tcp.headerLength == {synHeaderLength} - 4B` is a valid step. A capture is converted only
where an expression names it, so a capture taken for one step cannot break another.

---

## 5. Combinators

| Builder | Meaning |
|---|---|
| `unordered({a, b, ...})` | all patterns must match, in any order (window = longest sub-`within`) |
| `anyOf({a, b, ...})` | the first alternative to match wins |
| `delivery(from, to, window)` | a sent packet is received as the **same packet** (correlated by `treeId`) within `window` |
| `strict()` | closed-world: a packet matching a step's selector *scope* but not its content fails that step |
| `meanwhile(step)` | start a step and do **not** wait for it; it runs beside the steps after it |

### A guard that does not block

Every step above holds the cursor for its whole window. So a `never` cannot cover the same
window as another `never`, and nothing can be observed while either is open.
`meanwhile(...)` starts a step and moves on in the same instant, and one event reaches every
running step rather than only the first:

```cpp
.meanwhile(never(on("host1.ipv4").signal("packetSentToUpper").within(0.5)))
.meanwhile(never(on("host1.eth[0].mac").signal("packetSentToLower")
                     .filterPacket("icmpv4.type == 3").within(0.5)))
.once(on("router.ipv4.ip").signal("packetDropped")
          .filterPacket("ipv4.identification == {id}").within(0.2))
```

`never`, `atMostTimes` and `atLeastTimes` exist as free builders for this, and they return a
step rather than adding one. A guard carries its own window and its own anchor, so a
`notBefore` inside it is measured from when the guard started.

---

## 6. Injection — craft and send packets

Inject a packet built in C++ into a module's gate, scheduled or reactively:

```cpp
.inject(at("host2").into("eth[0]", "upperLayerOut").at(0.5)
          .describe("a UDP datagram to port 5000")
          .filterPacket(buildInjectedUdpDatagram))      // a Packet *(const CaptureStore&) builder
```

- `.into(module, gate)` — the sink under the node to `pushPacket()` into.
- `.at(t)` absolute, or `.after(d)` relative to the previous step's match (reactive).
- `.filterPacket(fn)` — the builder; it may read captures, so the injected packet can depend on an
  observed one (stimulus/response). The builder owns construction — any chunk/tag is possible.

Inject steps are ordered like any other step.

---

## 7. Interception (MITM) — drop / delay / mutate

A `PacketTap` module spliced onto a link can drop/delay/mutate frames in flight. `tap(...)`
builds the clause and `.intercept(...)` adds it, so the two roles read differently:

```cpp
.intercept(tap("relay")
             .filterPacket("tcp.destPort == 1000 && tcp.synBit == false")
             .minBytes(100)        // only the data-bearing segment
             .nth(1)               // the first match (1-based; 0 = every)
             .drop()               // or .delay(0.05) or .mutate([](Packet *p){ ... })
             .describe("the first data segment"))
```

**A relay holds a list of rules.** Each clause adds one; a frame is offered to the rules in
order and the first that matches applies; a frame that matches none passes. Each rule counts
its own occurrences. `pass()` is an explicit no-op, and it earns its place here: it shadows a
later rule for the frames it names, so an exception is a clause of its own rather than a
condition inside the other rule's expression.

```cpp
.intercept(tap("relay").filterPacket("tcp.synBit == true").pass().describe("never touch a SYN"))
.intercept(tap("relay").filterPacket("tcp.destPort == 1000").drop().describe("drop the data"))
```

Interceptions are **standing** rules (armed for the whole run, not ordered steps). The
`ProtocolTester` resolves the named tap at startup and installs the rule.

- `.drop()` — discard the frame (force a retransmission).
- `.delay(t)` — forward after a hold time.
- `.mutate(fn)` — run `void(Packet*)` on the inner frame, then forward. `setBitError(true)`
  makes the receiving Ethernet MAC discard it (a corruption fault).

**Splicing a tap into a network** (the tap must carry `@networkNode()` so the IPv4
configurator merges both sides into one link):

```ned
host1.ethg++ <--> Eth100M <--> tap.a;
tap.b       <--> Eth100M <--> host2.ethg++;
```

The tap is a store-and-forward relay (it respects the channel's transmission time), so it is
transparent unless a rule fires. A bare tap (default `action = "pass"`) changes nothing.

---

## 8. State machines — assert FSM / scalar-signal state

Some behaviour lives in a module's **state machine or counters**, not in the packet trace
(e.g. Ethernet PLCA's beacon / transmit-opportunity cycle is invisible to packet observation).
The framework observes any scalar (`intval_t`) signal — an FSM state index, an ID, a counter —
as a second channel beside packets. INET's `Fsm` already emits its state on every transition
(`setStateChangedSignal`), so the state machine needs no modification.

A scalar signal is just another `signal()` — selected the same way as a packet signal, with
`filterValue(v)` for its value and the ordinary cardinality builders (`once`/`never`/…):

```cpp
.once(on("node[0].eth[0].plca").signal("controlStateChanged")   // module path, then the signal
          .filterValue(EthernetPlca::CS_COMMIT)                           // the value (a public enum)
          .within(0.001))
.never(on("node[0].eth[0].plca").signal("controlStateChanged")
          .filterValue(EthernetPlca::CS_ABORT).within(0.001))             // negative: must not enter this state
```

| Clause | Meaning |
|---|---|
| `on("path").signal("name")` | the emitting module and its scalar signal (e.g. `controlStateChanged`) |
| `filterValue(v)` | require this exact value (typically a public enum, e.g. `EthernetPlca::CS_TRANSMIT`); omit to match any emission |
| `once(p)` / `never(p)` / … | the same cardinality builders as packets — `once` = "reaches the value", `never` = "must not" |

A scalar signal flows through the **same engine** as packets: a scalar step matches a scalar
emission, a packet step a packet — they interleave freely in one ordered program.

**Discovering signals (the authoring step).** Set `stateSignals` to a space-separated list of
signal names; the tester then dumps every scalar emission (`SE t=… mod=… signal=… value=…`), so
you can read the real FSM sequence before writing assertions:

```
*.tester.stateSignals = "controlStateChanged dataStateChanged curID rxCmd txCmd"
```

The value is the raw index; map it with the module's public enum or its `@statistic[...] enum=`
declaration, and write assertions against the enum (`EthernetPlca::CS_*` / `DS_*`), not the bare
number.

---

## 9. Self-description

Set `*.tester.printDescription = true` to print the program as English at startup, e.g.:

```
ProtocolTest "wifi_block_ack_full":
  1. Within 0.1s, host1 must send a packet to the lower layer at the link layer -- ADDBA Request.
  ...
  5. 5 times: Within 0.5s, host1 must send a packet to the lower layer at the link layer -- a QoS data frame with Block Ack policy.
```

Interception rules render as `* Fault injection: on tap 'tap', drop the first data segment (≥ 100 bytes), occurrence 1.`

---

## 10. Running tests

Every test is a self-contained `opp_test` `.test` file. The suites live next to this
library, one folder per subject:

| Folder | Subject |
| --- | --- |
| [`../self/`](../self) | the framework itself: the matching engine, the combinators, injection, interception |
| [`../tcp/`](../tcp) | TCP |
| [`../arp/`](../arp), [`../ipv4/`](../ipv4), [`../ipv6/`](../ipv6), [`../ethernet/`](../ethernet) | one folder per protocol |
| [`../wifi/`](../wifi) | the IEEE 802.11 conformance suite (its own runner) |

Before running, follow the
[INET library freshness and build-mode guidance](../../../doc/project/guide/run-the-gates.md#keep-the-tested-library-current).
The runner builds the applicable test support library and generated executables automatically.
Run from the repository root, selecting the suite and test files for focused development:

```sh
inet_run_protocol_tests -p inet -m debug -w '<suite-regex>' -f '<test-path-regex>'
inet_run_protocol_tests -p inet -m debug -w self  # one suite (the folder name)
inet_run_protocol_tests -p inet -m debug -w ipv   # every suite whose folder matches
inet_run_protocol_tests -p inet -m debug          # every suite
```

The runner finds the suites itself: every direct subfolder of `tests/protocol` that holds
`.test` files is one suite. A new protocol folder needs no registration.

To run one test and read its output, use `opp_test` directly in the suite folder. Share
common ini settings via [`protocoltest-base.ini`](protocoltest-base.ini)
(`include protocoltest-base.ini`).

### Self-contained `.test` cases (program + network in one file)

A test does not need a `ProtocolTester` declared in its network. Define the program with
`Define_ProtocolTestProgram()` (one per build, no name/selection) and the framework attaches
a `ProtocolTester` to whatever network runs — so a test can target an **unmodified external
network** just by pointing `network =` at it. Every test in [`../self/`](../self) is such
an example: `Basic.test` is the smallest one, `ViolationDetected.test` asserts that the
framework reports a violation, and `InterceptMutate.test` drives a fault into the wire.
Each carries its program in `%global`, its (tester-less) network in `%file`, and asserts
the verdict line with `%contains`.

How the attach works: defining a `Define_ProtocolTestProgram()` registers it as the default
program; a simulation lifecycle listener (`ProtocolTestAttach.cc`) creates a `ProtocolTester`
under the network at `LF_POST_NETWORK_INITIALIZE` (a real, Qtenv-inspectable module, just not
in the NED) and runs the default program. If the network already has its own tester, the
listener leaves it alone. The framework's NED types live in package `inet.protocoltest` with
an explicit `@namespace`, which keeps them immune to a consumer `.test`'s root `@namespace`.

> Note on `%global`: opp_test only compiles the embedded C++ when an `%activity` is also
> present, so each example carries a one-line `%activity` that never runs (the network is the
> one in `%inifile`). And avoid writing a literal `%word` in a `%description` — opp_test reads
> it as a section directive.

### Declaring an expected result (`%# expected-result`)

A test's verdict (pass/fail) is one dimension; whether that verdict was *expected* is a
separate one. Always assert the honest, spec-conformant line with `%contains`
(`PROTOCOLTEST <name>: PASS`). If the faithful assertion currently *fails* for one of two
reasons, declare that up front instead of faking it:

- a feature is known to be unimplemented, or
- a defect is known, and a known limitation blocks its repair, maybe for a long time.

```
%# expected-result: FAIL
```

The second case needs its reason in the test file: the `%description` names the defect, the
limitation that blocks the repair, and the results file that records both. A defect that is to be
fixed soon gets no declaration; it fails the run until somebody fixes it. See
[the class of a failure](../../../doc/project/guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected).

opp_test ignores `%#` comment lines, so this is metadata for the opp_repl test wrapper
(`opp_run_opp_tests`), which reads it and reports the pair: a matching failure shows
**`FAIL (expected)`** (green — a real failure, honestly marked, not a fake PASS); a regression
shows **`FAIL (unexpected)`**; and a gap that *closes* shows **`PASS (unexpected)`**. The last
two fail the run. Allowed values: `PASS` (the default when absent), `FAIL`, `ERROR`.

> Never invert `%contains` to expect the failure line (`… : FAIL`) — that lies: a genuinely
> failing test reports PASS, and a gap closing looks like a regression.

---

## 11. Cookbook

All snippets come from the `.test` files in the suite folders; the name in brackets is
the test file that runs them.

### TCP three-way handshake — sequence/ack relations
Observe SYN / SYN+ACK / ACK at the initiator, asserting the ack numbers follow seq+1 via
captures.
```cpp
.once(on("host1.tcp").signal("packetSentToLower")
          .filterPacket("tcp.synBit == true && tcp.ackBit == false")
          .capture("isn", "tcp.sequenceNo").within(0.2))
.once(on("host1.tcp").signal("packetReceivedFromLower")
          .filterPacket("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 1")
          .capture("peerIsn", "tcp.sequenceNo").within(0.5))
.once(on("host1.tcp").signal("packetSentToLower")
          .filterPacket("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == {peerIsn} + 1").within(0.5));
```

### TCP retransmission via a dropped segment (`MitmRetransmit`)
A tap drops the first data segment; the test asserts host1 re-sends the same sequence number
after the RTO. Shows fault injection driving a behaviour, then asserting it.
```cpp
.intercept(tap("tap").filterPacket("tcp.destPort == 1000 && tcp.synBit == false")
             .minBytes(100).nth(1).drop().describe("the first data segment"))
.once(... capture "dataSeq" = tcp.sequenceNo ...)
.once(... match "tcp.sequenceNo == {dataSeq} && tcp.synBit == false" .notBefore(0.3).within(5.0));
```
`MitmMutate` is the same outcome via `.mutate([](Packet *f){ f->setBitError(true); })`.

### Reactive injection — be the TCP peer (`TcpPeer`)
host1 opens to a phantom IP; the test observes the SYN, injects a crafted SYN+ACK acking
ISN+1, then observes host1's final ACK — a handshake driven entirely by injection.

### ARP resolution
```cpp
.once(on("host1.eth[0].mac").signal("packetSentToLower").filterPacket("arp.opcode == 1")
          .describe("an ARP request").within(0.2))
.once(on("host1.eth[0].mac").signal("packetReceivedFromLower").filterPacket("arp.opcode == 2")
          .describe("host2's ARP reply").within(0.2));
```

### IPv4 fragmentation (`Fragmentation`)
A 4000-byte datagram over a 1500-byte MTU yields several fragments.
```cpp
.once(on("host1.eth[0].mac").signal("packetSentToLower").filterPacket("ipv4.moreFragments == true")
          .describe("a fragment with the more-fragments flag set").within(0.2))
.once(on("host1.eth[0].mac").signal("packetSentToLower").filterPacket("ipv4.fragmentOffset > 0")
          .describe("a later fragment at a non-zero offset").within(0.2));
```

### 802.11 Block Ack sequence (`WifiBlockAckFull`)
The full agreement: ADDBA handshake (each frame ACKed), then a block of 5 QoS data frames,
a Block Ack Request, and one Block Ack — using `nextTimes(5, ...)` for the block. See
`wifi_block_ack_full` for the complete sequence.

### DHCP (pattern)
Not bundled (needs a DHCP server/client scenario), but the shape is the standard four-way
exchange as a sequence of `once` steps matching the message type at the application layer:
DISCOVER (client → broadcast) · OFFER (server → client) · REQUEST (client → broadcast) ·
ACK (server → client). Add a scenario with `DhcpClient`/`DhcpServer` apps and assert each
`bootp`/`dhcp` message in order, the same way the handshake example asserts TCP flags.

### Ethernet PLCA state machine (`Plca`, 10BASE-T1S) — state channel
PLCA's beacon / transmit-opportunity cycle lives in two state machines, not the packet
trace, so this asserts the FSM signals (§8) instead. On a controller + 2-node multidrop bus
(`../ethernet/PlcaBeaconCycle.test`): the controller sends the BEACON, node[0] synchronises, the transmit
opportunity rotates to node[0] (`curID == 1`), node[0] COMMITs and its data FSM transmits,
and the controller receives the frame — while the control FSM must never `CS_ABORT`.
```cpp
.once(on("controller.eth[0].plca").signal("controlStateChanged").filterValue(EthernetPlca::CS_SEND_BEACON).within(0.001))
.once(on("node[0].eth[0].plca").signal("controlStateChanged").filterValue(EthernetPlca::CS_SYNCING).within(0.001))
.once(on("node[0].eth[0].plca").signal("curID").filterValue(1).within(0.001))
.once(on("node[0].eth[0].plca").signal("controlStateChanged").filterValue(EthernetPlca::CS_COMMIT).within(0.001))
.once(on("node[0].eth[0].plca").signal("dataStateChanged").filterValue(EthernetPlca::DS_TRANSMIT).within(0.001))
.never(on("node[0].eth[0].plca").signal("controlStateChanged").filterValue(EthernetPlca::CS_ABORT).within(0.001));
```
Author it by first setting `stateSignals = "controlStateChanged dataStateChanged
curID rxCmd txCmd"` on the tester to read the real sequence. See
[`../ethernet/PlcaBeaconCycle.test`](../ethernet/PlcaBeaconCycle.test).

### Mobile IPv6 registration + route optimization (RFC 6275)
MIPv6 is a message-exchange protocol (no FSM-state signal), so this asserts the Mobility Header
sequence as packets. On a minimal MN/HA/CN handover network, after the mobile node
roams to a foreign link it registers with its Home Agent (Binding Update → Binding Acknowledgement),
then route-optimizes with the correspondent node via the return-routability procedure (HoTI/CoTI →
HoT/CoT with the cookies echoed) and a direct Binding Update.
```cpp
// send  = on("MN[0]").signal("packetReceivedFromUpper").protocol("mobileipv6").attributeTo("MN[0].ipv6.mipv6")...
// receive = on("MN[0]").signal("packetSentToUpper").protocol("mobileipv6").attributeTo("MN[0].ipv6.mipv6")...
.once(send("BindingUpdate.homeRegistrationFlag == true && BindingUpdate.ackFlag == true", ...))
.once(receive("BindingAcknowledgement.status == 0", ...))
.unordered({ send("HomeTestInit.homeInitCookie >= 0", ...).capture("hoCookie", "HomeTestInit.homeInitCookie"),
             send("CareOfTestInit.careOfInitCookie >= 0", ...).capture("coCookie", "CareOfTestInit.careOfInitCookie") })
.unordered({ receive("HomeTest.homeInitCookie == {hoCookie}", ...),     // cookie echoed back
             receive("CareOfTest.careOfInitCookie == {coCookie}", ...) })
.once(send("BindingUpdate.homeRegistrationFlag == false", ...));        // route-optimized BU direct to the CN
```
Notes: **(a)** observe at the MN's **IPv6 boundary**, where the message is still the bare Mobility
Header (`protocol("mobileipv6")`), and `attributeTo("MN[0].ipv6.mipv6")` narrates it from the mipv6
module's point of view (so `packetReceivedFromUpper` reads as the mipv6 module *sending*). **(b)** the
**same case at the network-interface level** (`mipv6_registration_and_ro_interface`) just swaps the
observation point to `on("MN[0].wlan[0]").signal("packetSentToLower"/"packetReceivedFromLower")` — the
field/cookie assertions are identical (PacketFilter's PacketDissector reaches the Mobility Header even
through the IPv6/802.11 encapsulation). Run `Mipv6Trace` first to read the real sequence. See
`mipv6_registration_and_ro` and `…_interface`.
