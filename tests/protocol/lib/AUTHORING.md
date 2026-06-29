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
        .once(on("host1").sentToLower().layer(Layer::Transport)
                  .match("udp.destPort == 5000").within(0.2))
        .once(on("host2").receivedFromLower().layer(Layer::Transport)
                  .match("udp.destPort == 5000").within(0.1));
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

`on("node")` starts a selector; chain to narrow it:

| Clause | Meaning |
|---|---|
| `on("host1")` | the node that emitted the event |
| `.iface("eth0")` | restrict to an interface |
| `.sentToLower()` / `.receivedFromLower()` | direction + seam (the usual ones) |
| `.sentToUpper()` / `.receivedFromUpper()` | up-stack seams |
| `.dropped()` | a packet-drop event |
| `.inbound()` / `.outbound()` | direction only |
| `.layer(Layer::Link)` | `Physical` / `Link` / `Network` / `Transport` / `Application` |
| `.match("expr")` | content predicate (PacketFilter expression, §4) |
| `.match([](const MatchContext& c){ ... })` | content predicate as a C++ lambda |
| `.describe("phrase")` | human phrase used by `describe()` and diagnostics |
| `.capture("name", "proto.field")` | remember a field value for later steps (§4) |
| `.within(t)` | deadline: the step must be satisfied within `t` of its anchor |
| `.after(t)` / `.notBefore(t)` | earliest time (relative to the previous step's match) |

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
| `exactlyTimes(n, p)` | n | advance on the nth match |
| `oneOrMoreTimes(p)` | 1..∞ | greedy: consume the whole `within` window, need ≥1 |
| `anyNumberOfTimes(p)` | 0..∞ | greedy: consume the window |
| `atLeastTimes(n, p)` | n..∞ | greedy: consume the window, need ≥n |
| `atMostTimes(n, p)` | 0..n | greedy: fail on the (n+1)th |
| `betweenTimes(a, b, p)` | a..b | greedy: need the count in [a, b] |

**Fixed-count** kinds (`once`, `exactlyTimes`) advance the instant the count is reached, so
they don't disturb the timing of later steps. **Greedy** kinds (`oneOrMore`, `atLeast`,
`atMost`, `between`, `anyNumber`) consume *every* matching frame until their `within` window
closes, then check the range — size `within` so the window ends before the next expected
frame, or a greedy step will swallow it.

---

## 4. Content matching & captures

`.match("expr")` uses INET's `PacketFilter` expression engine over the dissected packet.
Protocol names are lowercase (`tcp`, `udp`, `ipv4`, `arp`, `ieee80211mac`), chunk class
names are as declared (`Ieee80211DataHeader`). Examples:

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
.once(on("host1").sentToLower().layer(Layer::Transport)
          .match("tcp.synBit == true")
          .capture("isn", "tcp.sequenceNo"))           // remember the ISN
.once(on("host1").receivedFromLower().layer(Layer::Transport)
          .match("tcp.ackNo == {isn} + 1"))            // refer back to it
```

For predicates the engine can't introspect, use a lambda plus `.describe("...")` so the
English rendering stays readable.

---

## 5. Combinators

| Builder | Meaning |
|---|---|
| `unordered({a, b, ...})` | all patterns must match, in any order (window = longest sub-`within`) |
| `anyOf({a, b, ...})` | the first alternative to match wins |
| `delivery(from, to, window)` | a sent packet is received as the **same packet** (correlated by `treeId`) within `window` |
| `strict()` | closed-world: a packet matching a step's selector *scope* but not its content fails that step |

---

## 6. Injection — craft and send packets

Inject a packet built in C++ into a module's gate, scheduled or reactively:

```cpp
.inject(inject("host2").into("eth[0]", "upperLayerOut").at(0.5)
          .describe("a UDP datagram to port 5000")
          .packet(buildInjectedUdpDatagram))      // a Packet *(const CaptureStore&) builder
```

- `.into(module, gate)` — the sink under the node to `pushPacket()` into.
- `.at(t)` absolute, or `.after(d)` relative to the previous step's match (reactive).
- `.packet(fn)` — the builder; it may read captures, so the injected packet can depend on an
  observed one (stimulus/response). The builder owns construction — any chunk/tag is possible.

Inject steps are ordered like any other step.

---

## 7. Interception (MITM) — drop / delay / mutate

A `PacketTap` module spliced onto a link can drop/delay/mutate frames in flight. Drive it
from the program with `intercept("tapModuleName")`:

```cpp
.intercept(intercept("tap")
             .match("tcp.destPort == 1000 && tcp.synBit == false")
             .minBytes(100)        // only the data-bearing segment
             .nth(1)               // the first match (1-based; 0 = every)
             .drop()               // or .delay(0.05) or .mutate([](Packet *p){ ... })
             .describe("the first data segment"))
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

```cpp
.reaches(state("node[0].eth[0].plca", "controlStateChanged")   // module path, signal name
           .is(EthernetPlca::CS_COMMIT)                         // the value (a public enum)
           .describe("node[0] commits in its transmit opportunity")
           .within(0.001))
.neverReaches(state("node[0].eth[0].plca", "controlStateChanged")
           .is(EthernetPlca::CS_ABORT).within(0.001))           // negative: must not enter this state
```

| Clause | Meaning |
|---|---|
| `state("path", "signalName")` | name the emitting module (relative to the network) and its scalar signal |
| `.is(value)` | require this exact value (typically a public enum, e.g. `EthernetPlca::CS_TRANSMIT`); omit to match any emission |
| `.within(t)` / `.notBefore(t)` / `.describe(...)` | as for packet steps |
| `reaches(p)` | the signal takes the value within the window (advance on match; fail on deadline) — the state analogue of `once` |
| `neverReaches(p)` | it must **not** take the value within the window (negative) |

`reaches`/`neverReaches` steps interleave with packet steps in one ordered program: a state
event only matches a state step, a packet event only a packet step.

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

Build the library and run a config:

```sh
cd tests/protocol/lib
./build.sh
./run-demo.sh -c Pass                       # run a config from omnetpp.ini
./run-demo.sh -c Trace                       # logEvents mode: print the event trace
./run-demo.sh -c Pass --*.tester.printDescription=true
```

A scenario is an ini config selecting a network and a `testName`. Share common settings via
[`protocoltest-base.ini`](protocoltest-base.ini) (`include protocoltest-base.ini`).

### Self-contained `.test` cases (program + network in one file)

A test does not need a `ProtocolTester` declared in its network. Define the program with
`Define_ProtocolTestProgram()` (one per build, no name/selection) and the framework attaches
a `ProtocolTester` to whatever network runs — so a test can target an **unmodified external
network** just by pointing `network =` at it. See the `opp_test` examples in [`../`](..):
`ProtocolTest_TcpHandshake.test`, `ProtocolTest_ViolationDetected.test`,
`ProtocolTest_TcpRetransmit.test`. Each carries its program in `%global`, its (tester-less)
network in `%file`, and asserts the verdict line with `%contains`.

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

---

## 11. Cookbook

All snippets are from [`ProtocolTests.cc`](ProtocolTests.cc); the config column is the
`omnetpp.ini` section that runs them.

### TCP three-way handshake — sequence/ack relations (`TcpHandshake`)
Observe SYN / SYN+ACK / ACK at the initiator, asserting the ack numbers follow seq+1 via
captures.
```cpp
.once(on("host1").sentToLower().layer(Layer::Transport)
          .match("tcp.synBit == true && tcp.ackBit == false")
          .capture("isn", "tcp.sequenceNo").within(0.2))
.once(on("host1").receivedFromLower().layer(Layer::Transport)
          .match("tcp.synBit == true && tcp.ackBit == true && tcp.ackNo == {isn} + 1")
          .capture("peerIsn", "tcp.sequenceNo").within(0.5))
.once(on("host1").sentToLower().layer(Layer::Transport)
          .match("tcp.ackBit == true && tcp.synBit == false && tcp.ackNo == {peerIsn} + 1").within(0.5));
```

### TCP retransmission via a dropped segment (`MitmRetransmit`)
A tap drops the first data segment; the test asserts host1 re-sends the same sequence number
after the RTO. Shows fault injection driving a behaviour, then asserting it.
```cpp
.intercept(intercept("tap").match("tcp.destPort == 1000 && tcp.synBit == false")
             .minBytes(100).nth(1).drop().describe("the first data segment"))
.once(... capture "dataSeq" = tcp.sequenceNo ...)
.once(... match "tcp.sequenceNo == {dataSeq} && tcp.synBit == false" .notBefore(0.3).within(5.0));
```
`MitmMutate` is the same outcome via `.mutate([](Packet *f){ f->setBitError(true); })`.

### Reactive injection — be the TCP peer (`TcpPeer`)
host1 opens to a phantom IP; the test observes the SYN, injects a crafted SYN+ACK acking
ISN+1, then observes host1's final ACK — a handshake driven entirely by injection.

### ARP resolution (`ArpResolution`)
```cpp
.once(on("host1").sentToLower().layer(Layer::Link).match("arp.opcode == 1")
          .describe("an ARP request").within(0.2))
.once(on("host1").receivedFromLower().layer(Layer::Link).match("arp.opcode == 2")
          .describe("host2's ARP reply").within(0.2));
```

### IPv4 fragmentation (`Fragmentation`)
A 4000-byte datagram over a 1500-byte MTU yields several fragments.
```cpp
.once(on("host1").sentToLower().layer(Layer::Link).match("ipv4.moreFragments == true")
          .describe("a fragment with the more-fragments flag set").within(0.2))
.once(on("host1").sentToLower().layer(Layer::Link).match("ipv4.fragmentOffset > 0")
          .describe("a later fragment at a non-zero offset").within(0.2));
```

### 802.11 Block Ack sequence (`WifiBlockAckFull`)
The full agreement: ADDBA handshake (each frame ACKed), then a block of 5 QoS data frames,
a Block Ack Request, and one Block Ack — using `exactlyTimes(5, ...)` for the block. See
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
(`PlcaMultidropDemo`): the controller sends the BEACON, node[0] synchronises, the transmit
opportunity rotates to node[0] (`curID == 1`), node[0] COMMITs and its data FSM transmits,
and the controller receives the frame — while the control FSM must never `CS_ABORT`.
```cpp
.reaches(state("controller.eth[0].plca", "controlStateChanged").is(EthernetPlca::CS_SEND_BEACON).within(0.001))
.reaches(state("node[0].eth[0].plca", "controlStateChanged").is(EthernetPlca::CS_SYNCING).within(0.001))
.reaches(state("node[0].eth[0].plca", "curID").is(1).within(0.001))
.reaches(state("node[0].eth[0].plca", "controlStateChanged").is(EthernetPlca::CS_COMMIT).within(0.001))
.reaches(state("node[0].eth[0].plca", "dataStateChanged").is(EthernetPlca::DS_TRANSMIT).within(0.001))
.neverReaches(state("node[0].eth[0].plca", "controlStateChanged").is(EthernetPlca::CS_ABORT).within(0.001));
```
Author it by first running `PlcaTrace` (`stateSignals = "controlStateChanged dataStateChanged
curID rxCmd txCmd"`) to read the real sequence. See `plca_beacon_cycle` in `ProtocolTests.cc`.

### Mobile IPv6 registration + route optimization (`Mipv6`, RFC 6275)
MIPv6 is a message-exchange protocol (no FSM-state signal), so this asserts the Mobility Header
sequence as packets. On a minimal MN/HA/CN handover network (`Mipv6Demo`), after the mobile node
roams to a foreign link it registers with its Home Agent (Binding Update → Binding Acknowledgement),
then route-optimizes with the correspondent node via the return-routability procedure (HoTI/CoTI →
HoT/CoT with the cookies echoed) and a direct Binding Update.
```cpp
// send  = on("MN[0]").receivedFromUpper().layer(Layer::Network)...   (a message the MN sends)
// receive = on("MN[0]").sentToUpper().layer(Layer::Network)...       (a message the MN receives)
.once(send("BindingUpdate.homeRegistrationFlag == true && BindingUpdate.ackFlag == true", ...))
.once(receive("BindingAcknowledgement.status == 0", ...))
.unordered({ send("HomeTestInit.homeInitCookie >= 0", ...).capture("hoCookie", "HomeTestInit.homeInitCookie"),
             send("CareOfTestInit.careOfInitCookie >= 0", ...).capture("coCookie", "CareOfTestInit.careOfInitCookie") })
.unordered({ receive("HomeTest.homeInitCookie == {hoCookie}", ...),     // cookie echoed back
             receive("CareOfTest.careOfInitCookie == {coCookie}", ...) })
.once(send("BindingUpdate.homeRegistrationFlag == false", ...));        // route-optimized BU direct to the CN
```
Two authoring notes: **(a)** observe at the MN's **IPv6 layer**, where the message is still the bare
Mobility Header (`mobileipv6` protocol) so PacketFilter can read its chunk fields (`BindingUpdate.*`,
`HomeTestInit.*`, `HomeTest.*`, …); on the wire it is buried inside the IPv6/802.11 frame and those
fields are not reachable. **(b)** From the IPv6 layer's perspective a message the MN *sends* is
"received from upper" and one it *receives* is "sent to upper" — hence the `send`/`receive` helpers.
Run `Mipv6Trace` first to read the real message sequence and timing. See `mipv6_registration_and_ro`.
