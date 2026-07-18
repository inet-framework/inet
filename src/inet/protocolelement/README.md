# The `protocolelement` Module Library

*A study of `inet/src/inet/protocolelement` — INET's library of small, composable
protocol-processing elements.*

> Scope: this document describes the design philosophy, the underlying
> packet-processing framework, the recurring construction patterns, the
> composition mechanisms, and a full catalog of every element in the package.
> It is written as background for building **new** atomic elements and composing
> them into complete protocols.

---

## 1. What this package is

`protocolelement` is a library of **very small simple modules**, each performing
*one* protocol function — insert a header, check a header, number packets,
reorder them, fragment them, add a checksum, shape a stream, transmit bits on a
wire, and so on. None of them is a "protocol" by itself. A real protocol is
assembled by **wiring a chain of these elements together** (in NED), typically
inside a compound *layer* or *service* module.

The design intent is captured verbatim in the package's own `__TODO` manifesto:

```
considerations for generic modular packet processing elements:
 - modules implement the same set of interfaces
 - gates are either passive or active
   active output gate is connected to a passive input gate (push)
   passive output gate is connected to an active input gate (pull)
 - communication between elements is either synchronous (push/pull) or asynchronous (send)
 - processing granularity is either
   - passing a packet as a whole
   - streaming packet start, progress, and end
 - processing of a single element can be (input -> output):
   - passing:         passing in  -> passing out
   - streaming:       passing in  -> streaming out
   - destreaming:     streaming in -> passing out
   - streaming through: streaming in -> streaming out
```

Two design consequences matter for anyone extending this library:

1. **Uniform interfaces.** Every element speaks the same small vocabulary of
   gate contracts (from `inet.queueing`), so *any* element can be spliced into
   *any* point of a chain as long as the granularity (packet vs. stream) matches.
2. **Header space efficiency is deliberately ignored.** Each element that needs
   to carry state to its peer inserts its *own* independent header chunk
   (`HopLimitHeader`, `SequenceNumberHeader`, `FragmentNumberHeader`, …). A chain
   of *N* elements produces *N* stacked headers. This is verbose on the wire but
   keeps every element self-contained and orthogonal.

---

## 2. The foundation: the `inet.queueing` framework

`protocolelement` is a thin layer on top of **`inet.queueing`**, INET's generic
packet-flow framework. You cannot understand or extend `protocolelement` without
this model. Everything below lives in `src/inet/queueing`.

### 2.1 Active vs. passive gates — the four neighbor roles

Every packet-carrying connection joins an **active** gate to a **passive** gate.
The active side *initiates*; the passive side *responds*. There are therefore
four roles a module can play at a gate:

| Role | Gate | Contract | Meaning |
|------|------|----------|---------|
| **Producer** | active output | `IActivePacketSource` | pushes packets *out* into a consumer |
| **Consumer** | passive input | `IPassivePacketSink` | accepts pushed packets |
| **Collector** | active input | `IActivePacketSink` | pulls packets *in* from a provider |
| **Provider** | passive output | `IPassivePacketSource` | hands over a packet when pulled |

Gates are labelled `@labels(push)` / `@labels(pull)` so the NED type system can
check that only compatible gates are connected.

- **Push flow:** an active source calls `pushPacket()` on the downstream passive
  sink. Data-driven — the source decides when a packet exists.
- **Pull flow:** an active sink calls `pullPacket()` on the upstream passive
  source. Demand-driven — the sink decides when it wants a packet (used by
  queues/servers, schedulers, shapers).

A single module usually supports **both** directions (see the base classes
below), so the *same* element works whether the surrounding chain is push-driven
or pull-driven. Which mode is active is decided by what it is connected to.

### 2.2 Whole-packet vs. streaming granularity

A packet can be handed over either:

- **as a whole** — one `pushPacket`/`pullPacket` call, or
- **as a stream** — `pushPacketStart` → zero or more `pushPacketProgress` →
  `pushPacketEnd`, carrying a `datarate` and a `position`. Streaming models a
  packet being transmitted bit-by-bit over a wire over time, which is what
  enables **cut-through** switching and **preemption** (acting on a packet
  before it has fully arrived).

An element is classified by how it maps input granularity to output granularity:
*passing*, *streaming* (packetize→stream), *destreaming* (stream→packetize), or
*streaming-through*. The `transceiver/` subsystem is where this boundary lives.

### 2.3 Base classes

Concrete elements almost never implement the gate contracts directly; they
extend a base class from `inet/queueing/base` that handles all the push/pull/
stream plumbing and calls a single overridable hook:

| Base class | Override | Purpose |
|------------|----------|---------|
| `PacketProcessorBase` | — | common root: registration, display, logging |
| **`PacketFlowBase`** | `processPacket(Packet*)` | 1-in/1-out **transform** that never drops (header inserters, numbering, padding). Supports push, pull *and* streaming transparently. |
| **`PacketFilterBase`** | `matchesPacket()` + `processPacket()` | 1-in/1-out that **drops** packets failing the predicate (header checkers). |
| `PacketPusherBase` / `PacketPullerBase` | — | single-direction variants |
| `PacketQueueBase`, `PacketServerBase`, `PacketClassifierBase`, `PacketGateBase`, `PacketMeterBase`, `PacketSchedulerBase` | various | queues, servers, classifiers, gates, meters, schedulers used by the shaper/QoS elements |

> **NED-vs-C++ note.** Newer INET elements declare `simple X extends SimpleModule`
> with `@class(X)` in the `.ned`, while the C++ class extends `PacketFlowBase`/
> `PacketFilterBase`. The real base is in the `.h`. This catalog reports the
> **C++** base, because that is what determines an element's behavior.

---

## 3. Core construction patterns

These are the recurring recipes. A new atomic element almost always instantiates
one of them.

### 3.1 The header inserter / checker pair

The single most common pattern. A protocol feature that needs to carry one field
to its peer is built as **two** elements plus **one** header chunk:

- an **inserter** (`SendWith…` / `…HeaderInserter`, a `PacketFlowBase`) that
  builds a header chunk, fills its fields, and `insertAtFront`s it on egress;
- a **checker** (`ReceiveWith…` / `…HeaderChecker`, a `PacketFilterBase`) that
  `popAtFront`s the header on ingress, validates it, and copies fields into a
  *tag* for the rest of the stack; it can drop the packet.

Worked example — hop limit (`forwarding/`):

```cpp
// HopLimitHeader.msg
class HopLimitHeader extends FieldsChunk {
    chunkLength = B(2);
    int hopLimit;
}
```
```cpp
// SendWithHopLimit : PacketFlowBase
void processPacket(Packet *packet) {
    auto header = makeShared<HopLimitHeader>();
    header->setHopLimit(hopLimit);          // hopLimit is a NED parameter
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::hopLimit);
    pushOrSendPacket(packet, outputGate, consumer);
}
```
```cpp
// ReceiveWithHopLimit : PacketFilterBase
bool matchesPacket(const Packet *packet) const {          // drop if expired
    return packet->peekAtFront<HopLimitHeader>()->getHopLimit() > 0;
}
void processPacket(Packet *packet) {
    auto header = packet->popAtFront<HopLimitHeader>();
    packet->addTag<HopLimitInd>()->setHopLimit(header->getHopLimit());
}
```

This is the pattern to imitate for every new field-carrying element.

### 3.2 Header chunks (`.msg` → `FieldsChunk`)

Each header is a `class … extends FieldsChunk` in a `.msg` file, giving it a
fixed `chunkLength` and typed fields. The message compiler generates the
`_m.h/_m.cc`. Because chunks are self-describing and length-tagged, they stack
cleanly: element A's header sits in front of element B's header on the wire, and
each checker peels off exactly its own. This is *why* the "one header per
element" verbosity works correctly.

### 3.3 Tags — cross-layer signalling without headers

Tags are out-of-band metadata attached to a `Packet` (never serialized onto the
wire). Convention:

- **`…Req`** (request) tags flow **down** the egress chain: an upper element
  states intent (e.g. `MacAddressReq`, `HopLimitReq`), a lower element consumes
  it and turns it into a header field.
- **`…Ind`** (indication) tags flow **up** the ingress chain: a checker turns a
  header field back into an `…Ind` tag (e.g. `HopLimitInd`) for higher elements.

Tags are how elements communicate *within* a node; headers are how peer nodes
communicate *across* the wire. Designing a new element means deciding which
information is a tag (local) and which is a header (on-wire).

### 3.4 Protocol registration & `AccessoryProtocol`

Elements announce what they produce/consume via
`registerProtocol()` / `registerService()` (from
`IProtocolRegistrationListener`). This lets `MessageDispatcher` modules route
packets to the right chain by protocol. Internal "glue" protocols that exist only
between these elements (hop limit, sequence number, fragmentation, …) are defined
as constants in `common/AccessoryProtocol`, so an inserter and its matching
checker agree on an identifier without polluting the real protocol registry.

### 3.5 Base + contract + policy — the Strategy pattern

Where an element has a *decision* that varies independently of its *mechanism*,
the code splits into three pieces (seen in `aggregation/` and `fragmentation/`):

- `base/…Base` — the abstract mechanism (how to cut/merge packets);
- `contract/I…Policy` — a `moduleinterface` for the decision;
- `policy/…Policy` — a concrete strategy (e.g. `LengthBasedFragmenterPolicy`)
  plugged in as a submodule via `like I…Policy`.

This is the primary extension seam: swap the policy submodule to change *when/how
much* without touching the mechanism.

### 3.6 Serializers

An optional `serializer/` sub-package provides a `…Serializer` (a
`FieldsChunkSerializer`) that converts a header chunk to/from raw bytes for
byte-accurate, emulation-capable simulation. Only elements whose headers must be
wire-compatible with real hardware need one.

### 3.7 The transmission boundary (`transceiver/`)

Transmitter/receiver elements convert between the whole-packet world of the
protocol chain and the streamed, timed world of a physical channel. They are the
only elements that introduce or absorb transmission *time*, and they come in
whole-packet, streaming, and stream-through variants (see §2.2).

---

## 4. Composition mechanisms

### 4.1 From simple elements to compound layers

A working protocol is a compound module that instantiates several simple
elements and wires their `in`/`out` gates into an egress path and an ingress
path. The `service/`, `redundancy/*Layer`, `measurement/`, `processing/`,
`socket/`, and `trafficconditioner/` subsystems are exactly these **compound
combinators**.

### 4.2 `IProtocolLayer` — the stackable-layer contract

Compound layers implement `contract/IProtocolLayer`, a 4-gate interface:

```
input  upperLayerIn;   output upperLayerOut;
input  lowerLayerIn;    output lowerLayerOut;
```

Layers with this shape can be **stacked vertically**: `upperLayer*` of one
connects to `lowerLayer*` of the next. A whole protocol stack becomes a column
of `IProtocolLayer` modules chosen by type — and because the interface has an
`@omittedTypename(OmittedProtocolLayer)`, any layer can be replaced by a
zero-cost pass-through when a feature is disabled. This is the `Omitted…`
convention seen throughout the package (`OmittedProtocolLayer`,
`OmittedMeasurementLayer`, `OmittedSocketLayer`, …).

### 4.3 Worked example: `service/DataService`

`DataService` is a compound `IProtocolLayer`-shaped module that assembles a
reliable, ordered data path from atomic elements:

```
                egress (upper→lower)                 ingress (lower→upper)
 upperLayerIn → aggregator → fragmenter →           lowerLayerIn → reordering →
                sequenceNumbering → queue →                        defragmenter →
                server → lowerLayerOut                             deaggregator → upperLayerOut
```

Every submodule is one of the atomic elements catalogued in §5; the "service" is
nothing but their wiring.

### 4.4 Progressive composition: `tutorials/protocol`

`inet/tutorials/protocol` is the canonical teaching example of this whole idea:
`Network1..91` build a protocol up one element at a time, mirroring the tutorial
`__TODO` roadmap:

1. app talks directly to app (bare source → sink);
2. add channel delay + transmitter/receiver;
3. add lossy channel + **FCS inserter/checker**;
4. add **inter-packet gap**;
5. add **queue + server**;
6. add **acknowledgement / resending**;
7. add MAC addressing, **sequence numbering + reordering**, QoS priority queues,
   preemption, dispatch for multiple peers, interface table, ports, hop limit,
   **forwarding + routing**, **fragmentation**, **aggregation**, and finally a
   full **data service**.

Reading these networks in order is the fastest way to internalize how the atomic
elements snap together into real protocols.

---

## 5. Element catalog

Every element in the package — grouped by subsystem, with its role, C++ base
class, parameters, gates, and the header/tag it manipulates — is catalogued in
the companion file **[ELEMENTS.md](ELEMENTS.md)**. The table below is the index.

| Subsystem | Concern | Key elements |
|-----------|---------|--------------|
| `acknowledgement` | stop-and-wait ARQ | `SendWithAcknowledge`, `ReceiveWithAcknowledge`, `Resending` |
| `dispatching` | numeric protocol-id header | `SendWithProtocol`, `ReceiveWithProtocol` |
| `forwarding` | hop-by-hop forwarding + hop limit | `Forwarding`, `SendWithHopLimit`, `ReceiveWithHopLimit` |
| `selectivity` | address/port addressing | `SendTo{L3Address,MacAddress,Port}`, `ReceiveAt{…}` |
| `aggregation` | sub-packet aggregation (policy-driven) | `SubpacketLengthHeaderBasedAggregator`/`Deaggregator` |
| `fragmentation` | fragment/defragment (policy-driven) + preemption | `FragmentNumberHeaderBasedFragmenter`, `PreemptableStreamer` |
| `checksum` | FCS / Internet checksum insert+check | `ChecksumHeaderInserter`/`Checker`, `EthernetFcs…` |
| `ordering` | sequence numbering, reordering, dedup | `SequenceNumbering`, `Reordering`, `DuplicateRemoval` |
| `redundancy` | 802.1CB stream replication/elimination | `StreamIdentifier`, `StreamEncoder`/`Decoder`, `StreamSplitter`/`Merger` |
| `shaper` | 802.1Q eligibility-time shaping | `EligibilityTimeMeter`/`Filter`/`Gate`/`Queue` |
| `cutthrough` | cut-through switching | `CutthroughSource`, `CutthroughSink` |
| `transceiver` | packet⇄stream transmission boundary | `PacketTransmitter`/`Receiver`, `Streaming…`, `StreamThrough…` |
| `common` | utility flows + protocol glue | `PacketEmitter`, `Packet(De)Serializer/Streamer`, `AccessoryProtocol` |
| `lifetime` | packet lifetime expiry | `CarrierBasedLifeTimer` |
| `measurement` / `processing` / `socket` / `trafficconditioner` | compound layers | `MeasurementLayer`, `ProcessingDelayLayer`, … + `Omitted…` |
| `service` | assembled reliable-data layers | `DataService`, `MacService`, … |
| `contract` | the shared `moduleinterface`s | `IProtocolLayer`, `IProtocolHeader{Inserter,Checker}`, `IPacketLifeTimer` |

---

## 6. Notes for building new elements & composing protocols

This section connects the existing design to the goal of a library of tiny,
recombinable elements plus a later header-compression and generative step.

- **The library already embodies the "tiny orthogonal element" philosophy.** The
  cleanest template to copy is the inserter/checker pair (§3.1) plus a
  `FieldsChunk` header (§3.2); a decision that should vary independently goes
  behind a policy interface (§3.5).
- **The deliberate header verbosity (§1, point 2) is the natural target for the
  planned compression step.** Because each element stacks an independent,
  length-tagged chunk, a *known combination* of elements produces a *fixed
  sequence* of chunks whose constant fields are redundant. A downstream encoder
  can replace a recognized chunk-sequence with a single identifier (a "function
  call") and serialize only the degrees of freedom — exactly the design premise.
  The per-element `serializer/` seam (§3.6) is where such an alternative,
  combination-aware encoding would attach.
- **Composition is already declarative (NED).** A protocol is a wiring of typed
  elements with parameters — a structure that maps directly onto a genotype:
  element types + connection topology + parameter values. The uniform gate
  contracts (§2.1) guarantee that many wirings are type-valid, which is the
  property a generative/evolutionary search needs.
- **What to check before adding elements:** granularity compatibility
  (packet vs. stream, §2.2), push/pull neighbours, and whether a new on-wire
  field belongs in a header or a tag (§3.3).

---

*Generated as a study of the package on the `topic/protocolelement` branch.*
