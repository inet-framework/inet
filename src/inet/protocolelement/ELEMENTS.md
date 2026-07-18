# `protocolelement` — Full Element Catalog

Companion to **[README.md](README.md)**. This file enumerates *every* element in
`inet/src/inet/protocolelement`, grouped by subsystem, with its role, **C++**
base class, parameters, gates, and the header chunk / tag it manipulates.

Read README.md first for the design philosophy, the `inet.queueing` framework,
the construction patterns, and the composition mechanisms. This file is the
reference.

---

## Conventions used below

- **Real C++ base is authoritative.** Where a module's `.ned` declares
  `extends SimpleModule` + `@class(X)` (or names one base) while the C++ class in
  the `.h` extends a different/queueing base, the catalog reports the **C++**
  base, since that determines behavior. Mismatches are flagged inline.
- **`.msg` chunks** all `extend FieldsChunk` (fixed `chunkLength`, typed fields)
  and are what actually appears on the wire. **Tags** (`…Req`/`…Ind`, `…Tag`)
  are out-of-band metadata on the `Packet`, never serialized.
- Internal glue protocols (`AccessoryProtocol::hopLimit`, `…sequenceNumber`, …)
  are how an inserter and its matching checker agree on identity; see
  `common/AccessoryProtocol` below for the full list of 11 constants.

## Known quirks & inconsistencies (read before extending)

These were found while cataloging. They are real properties of the current
source on `master`, worth knowing before building on or copying these elements.

**NED-declared base ≠ real C++ base** (the `.ned` under-/mis-states the parent):

| Module | `.ned` says | C++ actually is |
|--------|-------------|-----------------|
| `fragmentation/FragmentNumberHeaderChecker` | `PacketFilterBase` | `PacketFlowBase` (never drops) |
| `redundancy/StreamSplitter` | `PacketPusherBase` | `PacketDuplicatorBase` |
| `transceiver/PacketReceiver` | `StreamingReceiverBase` | `PacketReceiverBase` (plainer) |
| `transceiver/DestreamingReceiver` | `PacketReceiverBase` | `StreamingReceiverBase` (richer) |
| `transceiver/base/PacketReceiverBase` | `PacketProcessorBase` | `OperationalMixin<PacketProcessorBase>` |
| `transceiver/base/PacketTransmitterBase` | `PacketProcessorBase` | `ClockUserModuleMixin<OperationalMixin<…>>` |
| `common/InterpacketGapInserter` | `PacketProcessorBase` | `ClockUserModuleMixin<PacketPusherBase>` |
| `common/PacketStreamer` | `PacketProcessorBase` | `ClockUserModuleMixin<PacketProcessorBase>` |

**Behavioral quirks / apparent bugs:**

- `service/DataService` instantiates `FragmentNumberHeaderBasedDefragmenter` for
  **both** its outbound `fragmenter` *and* inbound `defragmenter` submodules —
  the egress path has no real fragmenter. Looks like a copy-paste bug.
- `forwarding/ReceiveWithHopLimit::processPacket` pops `HopLimitHeader` **twice**
  in a row (second result discarded).
- `forwarding/Forwarding::findNextHop` is a hard-coded `// KLUDGE` table
  (recognizes only `10.0.0.10` from sources `10.0.0.1/.2/.7`) — a tutorial-grade
  router, not a general one.
- `selectivity/ReceiveAtMacAddress` re-dispatches unconditionally to
  `AccessoryProtocol::sequenceNumber` (hard-coded `// KLUDGE`) and sets the
  containing NIC's MAC address as an `initialize()` side effect.
- `fragmentation/policy/LengthBasedFragmenterPolicy` declares `minFragmentLength`
  and `roundingLength` params that are read but **never used** in
  `computeFragmentLengths()`.
- `trafficconditioner/TrafficConditionerLayer`'s submodule names
  (`ingressConditioner` / `egressConditioner`) are wired to the **opposite**
  direction their names imply.
- `socket/ISocketLayer` lacks the `@omittedTypename(...)` that its sibling layer
  interfaces all declare (so it cannot be silently omitted the same way).

**Strategy-pattern asymmetry** (`aggregation`, `fragmentation`): only the
*splitting/aggregating* end (`AggregatorBase`, `FragmenterBase`) owns a pluggable
policy submodule (`IAggregatorPolicy` / `IFragmenterPolicy`); the *reassembly*
end (`DeaggregatorBase`, `DefragmenterBase`) has no policy — reassembly is a
fixed state machine. `AggregatorBase`'s policy defaults to
`LengthBasedAggregatorPolicy`; `FragmenterBase`'s policy has **no default** and
must be set (e.g. `**.fragmenterPolicy.typename=…`). `checksum/` has no policy at
all — mode is a C++ `switch`.

---


All modules below are declared as `simple ... extends SimpleModule` (from `inet.common.SimpleModule`,
itself `extends ModuleMixin`) or directly `extends` one of the `inet.queueing.base` classes
(`PacketPusherBase`, `PacketFilterBase`, `PacketFlowBase`); the `@class(...)` NED property points to
the actual C++ implementation class. Every module here is a **simple module** (no compound modules or
module interfaces exist in these four subdirectories). Header/tag chunks live in `.msg` files and are
generated into `..._m.h`; all of them `extends FieldsChunk`. Cross-module signaling of which
"accessory protocol" a packet currently carries is done through `inet::AccessoryProtocol` constants
(`acknowledge`, `aggregation`, `checksum`, `destinationL3Address`, `destinationMacAddress`,
`destinationPort`, `forwarding`, `fragmentation`, `hopLimit`, `sequenceNumber`, `withAcknowledge`),
registered via `registerService`/`registerProtocol` (`IProtocolRegistrationListener`), and packets are
routed downstream between these elements using `PacketProtocolTag` / `DispatchProtocolReq`.

## acknowledgement
Implements a simple stop-and-wait acknowledgement/retransmission scheme: one element adds a sequence
number and waits for an ack (retransmitting on timeout via a companion resend element), and a peer
element consumes the sequence-numbered data and emits an acknowledgement packet back.

### `AcknowledgeHeader` (msg chunk, not a module)
- **Fields:** `sequenceNumber: int`; `chunkLength = B(4)`.
- Extends `FieldsChunk`. Carried by standalone "Ack" packets tagged with `AccessoryProtocol::acknowledge`.

### `ReceiveWithAcknowledge` (simple module)
- **Role/base:** header checker / packet splitter — C++ base is `PacketPusherBase` (`inet::queueing::base`).
- **Does:** On `pushPacket`, pops a `SequenceNumberHeader` (from `inet.protocolelement.ordering`, outside this catalog's scope) off the incoming data packet, forwards the now-header-less packet on `out`, then builds a brand-new `AcknowledgeHeader` packet (named `"Ack"`) carrying the same sequence number and sends it out `ackOut`, tagged with protocol `AccessoryProtocol::acknowledge`.
- **Parameters:** none (only `@class(ReceiveWithAcknowledge)`).
- **Gates:** `input in`, `output out`, `output ackOut`.
- **Header/tag:** removes `SequenceNumberHeader` (ordering module) from the data packet; creates/sends an `AcknowledgeHeader` (`sequenceNumber`, `B(4)`) on a separate packet via `ackOut`; sets `PacketProtocolTag` = `AccessoryProtocol::acknowledge` on the ack packet. Registers as service/protocol for `AccessoryProtocol::withAcknowledge` on `in`/`out`.

### `Resending` (simple module)
- **Role/base:** retransmitting pusher — `extends PacketPusherBase` directly in NED; C++ base `PacketPusherBase`.
- **Does:** Holds at most one in-flight packet (`canPushSomePacket`/`canPushPacket` return false while one is outstanding). On push, duplicates and forwards the packet downstream; `handlePushPacketProcessed` is the completion callback — if the push succeeded or the retry budget (`numRetries`) is exhausted, it reports completion to the producer and clears state; otherwise it re-duplicates and re-sends the same packet, incrementing `retry`.
- **Parameters:** `numRetries: int` (no default in NED; must be set in config) — maximum number of resend attempts before giving up.
- **Gates:** inherited from `PacketPusherBase`: `input in`, `output out`.
- **Header/tag:** none — operates purely on push/duplicate semantics, no chunk inserted/removed and no tags read/written.

### `SendWithAcknowledge` (simple module)
- **Role/base:** header inserter + ack-timeout tracker — C++ base `PacketFlowBase`.
- **Does:** `processPacket` branches on the packet's current `PacketProtocolTag`: if the packet carries `AccessoryProtocol::acknowledge` (i.e., it's an incoming ack), it pops the `AcknowledgeHeader`, looks up the pending timer by sequence number, cancels/deletes the timer, and reports the original data packet as successfully pushed to the producer. Otherwise (a fresh data packet), it inserts a `SequenceNumberHeader` (ordering module) with an incrementing `sequenceNumber`, tags the packet with `AccessoryProtocol::withAcknowledge`, sends it on `out`, and schedules a self-message timer for `acknowledgeTimeout` seconds keyed by sequence number (stored in a `timers` map). `handleMessage` fires on timeout: reports failure (`successful=false`) for that packet to the producer.
- **Parameters:** `acknowledgeTimeout: double @unit(s)` (no default) — how long to wait for an ack before treating the send as failed.
- **Gates:** `input in`, `input ackIn`, `output out`.
- **Header/tag:** inserts `SequenceNumberHeader` (ordering module, `insertAtFront`) on egress data packets; pops `AcknowledgeHeader` on packets arriving with protocol `AccessoryProtocol::acknowledge`; sets `PacketProtocolTag` to `AccessoryProtocol::withAcknowledge` on outgoing data and reads `PacketProtocolTag` to distinguish ack vs. data packets. Registers service for `AccessoryProtocol::withAcknowledge` on `in`/`out` and `AccessoryProtocol::acknowledge` on `ackIn`.

## dispatching
Adds/removes a lightweight numeric protocol-id header so a generic downstream module can be told which
INET `Protocol` a packet belongs to purely from packet content (rather than only from an out-of-band
tag), and dynamically forwards protocol (de)registration requests to the correct gate.

### `ProtocolHeader` (msg chunk, not a module)
- **Fields:** `protocolId: int`; `chunkLength = B(2)`.
- Extends `FieldsChunk`. Encodes an INET `Protocol`'s numeric id (`Protocol::getId()` / `Protocol::findProtocol(id)`).

### `ReceiveWithProtocol` (simple module)
- **Role/base:** header checker — C++ base `PacketPusherBase`.
- **Does:** On `pushPacket`, pops the `ProtocolHeader`, resolves the protocol object via `Protocol::findProtocol(protocolId)`, and tags the packet with both `PacketProtocolTag` and `DispatchProtocolReq` set to that resolved protocol (so a generic dispatcher downstream routes the packet correctly).
- **Parameters:** none (only `@class(ReceiveWithProtocol)`).
- **Gates:** `input in`, `output out`.
- **Header/tag:** removes `ProtocolHeader` (`protocolId`, `B(2)`); writes `PacketProtocolTag` and `DispatchProtocolReq` (both set to the decoded `Protocol*`). No explicit protocol registration call in this module's code.

### `SendWithProtocol` (simple module)
- **Role/base:** header inserter, and also a dynamic protocol-registration relay — C++ base `PacketFlowBase`, additionally implements `DefaultProtocolRegistrationListener`.
- **Does:** `processPacket` reads the packet's current `PacketProtocolTag` protocol, builds a `ProtocolHeader` with that protocol's numeric id, and prepends it. Separately, `handleRegisterService`/`handleRegisterProtocol` (invoked when something downstream registers interest in a protocol) forward that registration onto this module's own `in`/`out` gates, effectively making the module transparently proxy protocol registrations for whatever protocol arrives.
- **Parameters:** none (only `@class(SendWithProtocol)`).
- **Gates:** `input in`, `output out`.
- **Header/tag:** inserts `ProtocolHeader` (`protocolId`, `B(2)`, `insertAtFront`) built from the packet's `PacketProtocolTag`; does not itself set the tag. No fixed protocol registration (registrations are proxied dynamically per the protocol requested by peers).

## forwarding
Implements a minimal, test-oriented hop-by-hop L3 forwarding element (next-hop/interface lookup with a
hard-coded topology) together with a paired hop-limit header inserter/checker (TTL-like mechanism) that
decrements/validates the number of remaining hops a packet may travel.

### `Forwarding` (simple module)
- **Role/base:** next-hop lookup / router — C++ base `PacketPusherBase`.
- **Does:** On `pushPacket`, clears any existing `DispatchProtocolReq`, peeks the packet's `DestinationL3AddressHeader` to get the destination, and calls `findNextHop()` — a **hard-coded/KLUDGE lookup table** (only recognizes destination `10.0.0.10` from source addresses `10.0.0.1`/`10.0.0.2`/`10.0.0.7`, returning a fixed next hop and outgoing interface index; otherwise returns unspecified/-1). If no next hop is found (destination reached / unknown), it pops the `DestinationL3AddressHeader` and re-dispatches the packet as protocol `AccessoryProtocol::destinationL3Address`. If a next hop is found, it looks up the corresponding sibling `interface` submodule by index, sets `InterfaceReq` to that interface's id, constructs a synthetic destination MAC address by encoding the low byte of the next-hop IPv4 address, sets `MacAddressReq`, tags the packet `PacketProtocolTag = AccessoryProtocol::forwarding` and `DispatchProtocolReq = AccessoryProtocol::hopLimit`, trims the front (removes already-consumed regions), and pushes/sends it onward.
- **Parameters:** `address: string = default("")` — this node's own L3 address (resolved via `L3AddressResolver`), used to determine this hop's position in the hard-coded next-hop table.
- **Gates:** `input in`, `output out`.
- **Header/tag:** reads/pops `DestinationL3AddressHeader` (selectivity module, `destinationAddress`, `B(4)`) when at the final hop; reads/writes tags `DispatchProtocolReq`, `InterfaceReq`, `MacAddressReq`, `PacketProtocolTag`. Registers service+protocol for `AccessoryProtocol::forwarding` on `in`/`out` (both directions).

### `HopLimitHeader` (msg chunk, not a module)
- **Fields:** `hopLimit: int`; `chunkLength = B(2)`.
- Extends `FieldsChunk`. TTL-style remaining-hop counter.

### `ReceiveWithHopLimit` (simple module)
- **Role/base:** header checker / drop filter — C++ base `PacketFilterBase`.
- **Does:** `matchesPacket` peeks the `HopLimitHeader` and returns true only if `hopLimit > 0` (packets with hop limit 0 are dropped by the `PacketFilterBase` framework before `processPacket` runs). `processPacket` pops the `HopLimitHeader` off the front **twice** in a row (the header is popped once into a local variable, then popped again immediately — the second pop's result is discarded), sets `HopLimitInd` on the packet to the (first-popped) hop-limit value, and marks the packet for re-dispatch as `AccessoryProtocol::forwarding`.
- **Parameters:** none (only `@class(ReceiveWithHopLimit)`).
- **Gates:** `input in`, `output out`.
- **Header/tag:** pops `HopLimitHeader` (`hopLimit`, `B(2)`) — twice, per above; writes `HopLimitInd` tag with the hop-limit value; writes `DispatchProtocolReq = AccessoryProtocol::forwarding`. Registers service+protocol for `AccessoryProtocol::hopLimit` on `in`/`out`.

### `SendWithHopLimit` (simple module)
- **Role/base:** header inserter — C++ base `PacketFlowBase`.
- **Does:** `processPacket` clears any `DispatchProtocolReq`, builds a `HopLimitHeader` with the configured `hopLimit` value and prepends it, tags the packet `PacketProtocolTag = AccessoryProtocol::hopLimit`, and pushes/sends it onward. (This module always inserts the same configured hop-limit value; it does not decrement a hop count already on the packet.)
- **Parameters:** `hopLimit: int` (no default in NED) — the hop-limit value written into every outgoing packet's header.
- **Gates:** `input in`, `output out`.
- **Header/tag:** inserts `HopLimitHeader` (`hopLimit`, `B(2)`, `insertAtFront`); clears `DispatchProtocolReq`; sets `PacketProtocolTag = AccessoryProtocol::hopLimit`. Registers service+protocol for `AccessoryProtocol::hopLimit` on `in`/`out`.

## selectivity
A family of matched inserter/filter pairs — for L3 address, MAC address, and transport port — that let
a demultiplexing pipeline route/filter packets based on a small header field (destination L3 address,
destination MAC address, or destination port number) instead of needing full protocol-specific parsing;
each "SendTo..." module tags/labels a packet for a destination, and the matching "ReceiveAt..." module
filters for packets addressed to itself and strips the header again.

### `DestinationL3AddressHeader` (msg chunk, not a module)
- **Fields:** `destinationAddress: L3Address`; `chunkLength = B(4)`.
- Extends `FieldsChunk`.

### `DestinationMacAddressHeader` (msg chunk, not a module)
- **Fields:** `destinationAddress: MacAddress`; `chunkLength = B(6)`.
- Extends `FieldsChunk`.

### `DestinationPortHeader` (msg chunk, not a module)
- **Fields:** `destinationPort: int`; `chunkLength = B(2)`.
- Extends `FieldsChunk`.

### `ReceiveAtL3Address` (simple module)
- **Role/base:** header checker / drop filter — C++ base `PacketFilterBase`.
- **Does:** `matchesPacket` peeks `DestinationL3AddressHeader` and compares its `destinationAddress` against the configured `address` (resolved as an `Ipv4Address`); non-matching packets are dropped by the framework. `processPacket` (called only for matches) pops the header.
- **Parameters:** `address: string` (no default) — this receiver's own L3 address to match against.
- **Gates:** `input in`, `output out`.
- **Header/tag:** pops `DestinationL3AddressHeader` (`destinationAddress`, `B(4)`) on match; no tags written. Registers service+protocol for `AccessoryProtocol::destinationL3Address` on `in`/`out`.

### `ReceiveAtMacAddress` (simple module)
- **Role/base:** header checker / drop filter — C++ base `PacketFilterBase`.
- **Does:** `matchesPacket` peeks `DestinationMacAddressHeader` and compares against the configured `address`. `processPacket` pops the header and (marked `// KLUDGE` in source) unconditionally re-dispatches the packet as `AccessoryProtocol::sequenceNumber`. `initialize` also calls `getContainingNicModule(this)->setMacAddress(address)`, i.e. it configures the enclosing NIC module's MAC address as a side effect.
- **Parameters:** `address: string` (no default) — the MAC address to match against and to assign to the containing NIC.
- **Gates:** `input in`, `output out`.
- **Header/tag:** pops `DestinationMacAddressHeader` (`destinationAddress`, `B(6)`) on match; writes `DispatchProtocolReq = AccessoryProtocol::sequenceNumber` (hard-coded downstream target). Registers service+protocol for `AccessoryProtocol::destinationMacAddress` on `in`/`out`.

### `ReceiveAtPort` (simple module)
- **Role/base:** header checker / drop filter — C++ base `PacketFilterBase`.
- **Does:** `matchesPacket` peeks `DestinationPortHeader` and compares `destinationPort` against the configured `port`. `processPacket` pops the header on match.
- **Parameters:** `port: int` (no default) — the port number this receiver listens on.
- **Gates:** `input in`, `output out`.
- **Header/tag:** pops `DestinationPortHeader` (`destinationPort`, `B(2)`) on match; no tags written. Registers service+protocol for `AccessoryProtocol::destinationPort` on `in`/`out`.

### `SendToL3Address` (simple module)
- **Role/base:** header inserter — C++ base `PacketFlowBase`.
- **Does:** `processPacket` builds a `DestinationL3AddressHeader` with the configured `address` and prepends it, then marks the packet for re-dispatch as `AccessoryProtocol::forwarding` and tags it `PacketProtocolTag = AccessoryProtocol::destinationL3Address`.
- **Parameters:** `address: string` (no default) — the destination L3 address to stamp on every outgoing packet.
- **Gates:** `input in`, `output out`.
- **Header/tag:** inserts `DestinationL3AddressHeader` (`destinationAddress`, `B(4)`, `insertAtFront`); writes `DispatchProtocolReq = AccessoryProtocol::forwarding` and `PacketProtocolTag = AccessoryProtocol::destinationL3Address`. Registers service+protocol for `AccessoryProtocol::destinationL3Address` on `in`/`out`.

### `SendToMacAddress` (simple module)
- **Role/base:** header inserter, with custom push/pull wiring — C++ base `PacketFlowBase` (overrides `canPushSomePacket`/`canPushPacket`/`pushPacket`/`getConsumer`/`handleCanPushPacketChanged`/`handlePushPacketProcessed` to proxy push-capability checks straight through to its consumer and to report `getConsumer` as `nullptr`, i.e. it does not itself act as a pull-capable consumer/back-pressure point).
- **Does:** `pushPacket` immediately hands the packet to `handleMessage` → `processPacket`. `processPacket` picks the destination MAC address from the packet's `MacAddressReq` tag if present, else falls back to the configured `address`; builds a `DestinationMacAddressHeader` with that address and prepends it, and tags the packet `PacketProtocolTag = AccessoryProtocol::destinationMacAddress`.
- **Parameters:** `address: string = default("")` — fallback destination MAC address used when the packet carries no `MacAddressReq` tag.
- **Gates:** `input in`, `output out`.
- **Header/tag:** inserts `DestinationMacAddressHeader` (`destinationAddress`, `B(6)`, `insertAtFront`); reads `MacAddressReq` tag (if present) to choose the address; writes `PacketProtocolTag = AccessoryProtocol::destinationMacAddress`. Registers service+protocol for `AccessoryProtocol::destinationMacAddress` on `in`/`out`.

### `SendToPort` (simple module)
- **Role/base:** header inserter — C++ base `PacketFlowBase`. (NED comment: "Adds a destination port header to packets... used by the corresponding ReceiveAtPort module to filter packets.")
- **Does:** `processPacket` builds a `DestinationPortHeader` with the configured `port` and prepends it.
- **Parameters:** `port: int` (no default) — the destination port number stamped on every outgoing packet.
- **Gates:** `input in`, `output out`.
- **Header/tag:** inserts `DestinationPortHeader` (`destinationPort`, `B(2)`, `insertAtFront`); no tags read or written. Registers service+protocol for `AccessoryProtocol::destinationPort` on `in`/`out`.

---


Base dir: `/home/levy/workspace/inet-protocolelement/src/inet/protocolelement`.
All modules build on `inet.queueing`'s push/pull framework (`PacketPusherBase`, `PacketFlowBase`,
`PacketFilterBase`, `PacketProcessorBase`), each of which declares plain `input in` / `output out`
gates. Where a `.ned` file's `extends`/`like` clause and the C++ `.h` base class diverge, both are
reported below.

## aggregation
Combines several small packets into one larger packet (and the reverse split) to amortize
per-packet header overhead. Follows the Strategy pattern: `base/AggregatorBase` is an abstract
compound module that owns a pluggable `aggregatorPolicy` submodule typed by
`contract/IAggregatorPolicy`; `policy/LengthBasedAggregatorPolicy` is the concrete strategy shipped
by default. Deaggregation (`base/DeaggregatorBase`) has no such policy submodule — reassembly logic
is fixed and only the wire-format parsing differs by subclass. This variant is header-based: framing
information is carried on the wire as a `SubpacketLengthHeader` per subpacket (no tag-based
aggregation variant exists in this module set).

### `AggregatorBase` (abstract base)
- **Role/base:** aggregator base (`module AggregatorBase extends PacketPusherBase`; C++ `AggregatorBase : public PacketPusherBase`) / delegates the aggregation decision to a pluggable policy (`IAggregatorPolicy`)
- **Does:** On `pushPacket()`, if not currently aggregating, starts a new `aggregatedPacket` (copying tags from the first subpacket). Otherwise asks `aggregatorPolicy->isAggregatablePacket(aggregatedPacket, aggregatedSubpackets, subpacket)`; if the policy says no, the current aggregate is pushed downstream and a fresh one is started. `continueAggregation()` (base impl) appends the subpacket's name to the aggregate's name and updates counters/length stats; actual byte-level merging is left to subclasses. The consumed subpacket is deleted after folding in. Can self-delete (`deleteSelf`) once a cycle ends.
- **Parameters:** `bool deleteSelf = false` — delete this module once an aggregation cycle completes; `string aggregatorPolicyClass = ""` — C++ class to instantiate directly as the policy, bypassing the submodule; `string aggregatorPolicyModule = ".aggregatorPolicy"` — relative path used to find the policy submodule when `aggregatorPolicyClass` is empty
- **Gates:** in / out (inherited from `PacketPusherBase`)
- **Header/tag:** none itself (added by subclasses); owns submodule `aggregatorPolicy: <default("LengthBasedAggregatorPolicy")> like IAggregatorPolicy` — instantiated by default as `LengthBasedAggregatorPolicy`

### `DeaggregatorBase` (abstract base)
- **Role/base:** deaggregator base (`simple DeaggregatorBase extends PacketPusherBase`; C++ matches). No policy submodule — the pure-virtual `deaggregatePacket()` is the only extension point.
- **Does:** `pushPacket()` takes an aggregated packet, calls subclass-implemented `deaggregatePacket()` to split it into a vector of subpackets, pushes/sends each one downstream in order, updates counters, deletes the aggregate, and optionally self-deletes.
- **Parameters:** `bool deleteSelf = false` — self-delete after processing one aggregate
- **Gates:** in / out
- **Header/tag:** none (delegated to subclass)

### `IAggregatorPolicy` (moduleinterface)
- **Role/base:** contract for aggregation-decision policies
- **Does:** C++ interface (`contract/IAggregatorPolicy.h`) declares `virtual bool isAggregatablePacket(Packet *aggregatedPacket, std::vector<Packet*>& aggregatedSubpackets, Packet *newSubpacket) = 0`, which a policy module must implement.
- **Parameters:** none (NED only sets `@display`)
- **Gates:** none
- **Header/tag:** none

### `IPacketAggregator` / `IPacketDeaggregator` (moduleinterface)
- **Role/base:** marker interfaces used in the `like` clause of concrete aggregator/deaggregator modules (`SubpacketLengthHeaderBasedAggregator`/`Deaggregator`)
- **Does:** no behavior; purely typing/documentation
- **Parameters:** none
- **Gates:** none
- **Header/tag:** none

### `LengthBasedAggregatorPolicy` (simple)
- **Role/base:** policy (`IAggregatorPolicy`) — length/count-based aggregation strategy; C++ `class LengthBasedAggregatorPolicy : public SimpleModule, public IAggregatorPolicy`
- **Does:** `isAggregatablePacket()` returns `true` ("keep collecting, don't flush yet") while the aggregate is still below `minNumSubpackets`/`minAggregatedLength`, OR while adding the new subpacket would keep the aggregate within `maxNumSubpackets`/`maxAggregatedLength`. `AggregatorBase` flushes and starts a new aggregate exactly when this returns `false`.
- **Parameters:** `int minNumSubpackets: int = 0` — min subpacket count before a flush is allowed; `int maxNumSubpackets: int` (mandatory) — max subpackets per aggregate; `int minAggregatedLength: b = 0b` — min aggregate length before a flush is allowed; `int maxAggregatedLength: b` (mandatory) — max aggregate length
- **Gates:** none (policy modules sit outside the packet path)
- **Header/tag:** none

### `SubpacketLengthHeaderBasedAggregator` (compound)
- **Role/base:** aggregator, header-based variant (`module ... extends AggregatorBase like IPacketAggregator`; C++ `class SubpacketLengthHeaderBasedAggregator : public AggregatorBase`)
- **Does:** Overrides `continueAggregation()`: after the base bookkeeping, inserts a `SubpacketLengthHeader` (holding the subpacket's data length) plus the subpacket's payload at the back of the aggregate, and copies region tags across.
- **Parameters:** none of its own (inherits `AggregatorBase`'s)
- **Gates:** in / out
- **Header/tag:** inserts one `SubpacketLengthHeader` (2-byte `FieldsChunk`: `b lengthField`) immediately before each subpacket's payload

### `SubpacketLengthHeaderBasedDeaggregator` (simple)
- **Role/base:** deaggregator, header-based variant (`simple ... extends DeaggregatorBase like IPacketDeaggregator`)
- **Does:** Registers as service/protocol `AccessoryProtocol::aggregation`. `deaggregatePacket()` loops while data remains: pops a `SubpacketLengthHeader` from the front, pops `lengthField` bytes of payload, wraps it in a new `Packet` (named via `+`-tokenizing the aggregate's composite name), copies region tags, and repeats.
- **Parameters:** none of its own
- **Gates:** in / out
- **Header/tag:** pops one `SubpacketLengthHeader` per subpacket

## fragmentation
Splits one packet into multiple fragments (and reassembles them). Also follows the Strategy
pattern for the *split* side only: `base/FragmenterBase` is an abstract compound module owning a
pluggable `fragmenterPolicy` submodule (`contract/IFragmenterPolicy`); `policy/LengthBasedFragmenterPolicy`
is the concrete strategy. `base/DefragmenterBase` has no such policy submodule — reassembly is a
fixed state machine, and subclasses only differ in how they read fragment framing. **Two parallel
variants exist**: a header-based variant (`FragmentNumberHeaderBasedFragmenter`/`Defragmenter`,
plus separate `FragmentNumberHeaderChecker`/`Inserter` that convert between the wire header and a
`FragmentTag`) that puts a `FragmentNumberHeader` on the wire, and a tag-based variant
(`FragmentTagBasedFragmenter`/`Defragmenter`) that carries the same framing info purely as an
in-simulation `FragmentTag`, with no header bytes. `PreemptableStreamer` is a streaming/preemption
element that also produces and consumes `FragmentTag`s (802.1Qbu-style preemption) but is not itself
a `FragmenterBase`/`DefragmenterBase` subclass.

### `FragmenterBase` (abstract base)
- **Role/base:** fragmenter base (`module FragmenterBase extends PacketPusherBase`; C++ matches) / delegates fragment-length decisions to a pluggable policy (`IFragmenterPolicy`)
- **Does:** `pushPacket()` asks `fragmenterPolicy->computeFragmentLengths(packet)` for a list of fragment byte-lengths, then for each one slices the corresponding byte range via the virtual `createFragmentPacket()` hook (overridden by subclasses to add a header or tag) and pushes/sends the fragment. Updates counters; optionally self-deletes after fragmenting one packet.
- **Parameters:** `bool deleteSelf = false`; `string fragmenterPolicyClass = ""` — C++ class to instantiate directly; `string fragmenterPolicyModule = ".fragmenterPolicy"` — relative path to the policy submodule if the class param is empty
- **Gates:** in / out
- **Header/tag:** none itself; owns submodule `fragmenterPolicy: <> like IFragmenterPolicy` — **no default typename** (unlike `AggregatorBase`'s policy), so it must be configured explicitly (typically to `LengthBasedFragmenterPolicy`) via NED or ini

### `DefragmenterBase` (abstract base)
- **Role/base:** defragmenter base (`simple DefragmenterBase extends PacketPusherBase`). No policy submodule/Strategy pattern here.
- **Does:** Provides the shared reassembly state machine via `defragmentPacket(fragmentPacket, firstFragment, lastFragment, expectedFragment)`: if the fragment is unexpected, aborts/discards any in-progress reassembly (or just deletes the stray fragment); otherwise starts a new `defragmentedPacket` on the first fragment (name derived by stripping the `-frag...` suffix), appends the fragment's data and copies tags via `continueDefragmentation()`, and on the last fragment pushes/sends a duplicate of the fully reassembled packet and resets state (optionally self-deleting). Concrete subclasses are responsible for extracting `firstFragment`/`lastFragment`/`expectedFragment` (from header or tag) and calling this method.
- **Parameters:** `bool deleteSelf = false`
- **Gates:** in / out
- **Header/tag:** none itself

### `IFragmenterPolicy` (moduleinterface)
- **Role/base:** contract for fragment-length policies
- **Does:** C++ interface declares `virtual std::vector<b> computeFragmentLengths(Packet *packet) const = 0`
- **Parameters:** none
- **Gates:** none
- **Header/tag:** none

### `IPacketDefragmenter` / `IPacketFragmenter` (moduleinterface)
- **Role/base:** marker interfaces used by the concrete fragmenter/defragmenter modules
- **Does:** no behavior
- **Parameters:** none
- **Gates:** none
- **Header/tag:** none

### `LengthBasedFragmenterPolicy` (simple)
- **Role/base:** policy (`IFragmenterPolicy`) — length-based fragmentation strategy; C++ `class LengthBasedFragmenterPolicy : public SimpleModule, public IFragmenterPolicy`
- **Does:** `computeFragmentLengths()`: if the whole packet already fits in `maxFragmentLength`, returns one fragment covering it. Otherwise repeatedly peels off `(maxFragmentLength - fragmentHeaderLength)` bytes per fragment (reserving room for a header to be added later) until the remainder fits, then appends the remainder as the final fragment length.
- **Parameters:** `int minFragmentLength: b = 0b` — read into a field but not referenced by `computeFragmentLengths()` in this revision; `int maxFragmentLength: b` (mandatory) — max payload bytes per fragment; `int roundingLength: b = 1B` — read into a field but not referenced by `computeFragmentLengths()`; `int fragmentHeaderLength: b = 0b` — bytes reserved per fragment for a header, subtracted from `maxFragmentLength` when slicing
- **Gates:** none
- **Header/tag:** none

### `FragmentNumberHeaderBasedFragmenter` (compound)
- **Role/base:** fragmenter, header-based variant (`module ... extends FragmenterBase like IPacketFragmenter`)
- **Does:** Overrides `createFragmentPacket()`: calls the base slicer, then inserts a `FragmentNumberHeader` (`fragmentNumber`, `lastFragment` = is this the final fragment) at `headerPosition`, and tags the fragment with `PacketProtocolTag = AccessoryProtocol::fragmentation`.
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** inserts `FragmentNumberHeader` (1-byte `FieldsChunk`: `int fragmentNumber; bool lastFragment;`)

### `FragmentNumberHeaderBasedDefragmenter` (simple)
- **Role/base:** defragmenter, header-based variant (`simple ... extends DefragmenterBase like IPacketDefragmenter`)
- **Does:** Registers as service/protocol `AccessoryProtocol::fragmentation`. `pushPacket()` pops the `FragmentNumberHeader` (1 byte, at `headerPosition`), derives `firstFragment` (`fragmentNumber==0`), `lastFragment` (header flag), `expectedFragment` (`fragmentNumber == expectedFragmentNumber` counter), then calls the inherited `defragmentPacket()` state machine.
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** pops `FragmentNumberHeader`

### `FragmentNumberHeaderChecker` (simple)
- **Role/base:** header→tag converter, **not** a fragmenter/defragmenter subclass. NED declares `simple FragmentNumberHeaderChecker extends inet.queueing.base.PacketFilterBase`, but the real C++ base class (per `.h`) is `PacketFlowBase` (`class FragmentNumberHeaderChecker : public PacketFlowBase`) — a case where the NED `extends` and the C++ base class diverge.
- **Does:** Registers as service/protocol `AccessoryProtocol::fragmentation`. `processPacket()` adds/gets a `FragmentTag` on the packet, pops the `FragmentNumberHeader` at `headerPosition`, and populates the tag's `firstFragment`/`lastFragment`/`fragmentNumber` from the header (`numFragments` set to `-1`, i.e. unknown). Used to bridge a header-based fragment stream into tag-based downstream logic (e.g. before `PreemptableStreamer`).
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** pops `FragmentNumberHeader`; writes `FragmentTag` (`firstFragment`, `lastFragment`, `fragmentNumber`, `numFragments=-1`)

### `FragmentNumberHeaderInserter` (simple)
- **Role/base:** tag→header converter (`simple FragmentNumberHeaderInserter extends inet.queueing.base.PacketFlowBase`; C++ matches) — inverse of `FragmentNumberHeaderChecker`
- **Does:** `processPacket()` reads the packet's existing `FragmentTag`, builds a `FragmentNumberHeader` from its `fragmentNumber`/`lastFragment` fields, inserts it at `headerPosition`, and tags the packet with `PacketProtocolTag = AccessoryProtocol::fragmentation`.
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** reads `FragmentTag`; inserts `FragmentNumberHeader`

### `FragmentTagBasedFragmenter` (compound)
- **Role/base:** fragmenter, tag-based variant (`module ... extends FragmenterBase like IPacketFragmenter`)
- **Does:** Overrides `createFragmentPacket()`: slices via the base implementation, then instead of inserting a wire header, attaches a `FragmentTag` (`firstFragment`, `lastFragment`, `fragmentNumber`, `numFragments`) directly to the fragment. No header bytes are added to the packet's serialized length — framing travels out-of-band as a packet tag.
- **Parameters:** none
- **Gates:** in / out
- **Header/tag:** writes `FragmentTag` (no header chunk)

### `FragmentTagBasedDefragmenter` (simple)
- **Role/base:** defragmenter, tag-based variant (`simple ... extends DefragmenterBase like IPacketDefragmenter`)
- **Does:** `pushPacket()` reads the fragment's `FragmentTag` directly (no header popping), derives `firstFragment`/`lastFragment` from the tag, treats `fragmentNumber == -1` as "unspecified" (always accepted) in addition to matching the expected counter, then defers to the inherited `defragmentPacket()` state machine.
- **Parameters:** none
- **Gates:** in / out
- **Header/tag:** reads `FragmentTag` (no header)

### `PreemptableStreamer` (simple)
- **Role/base:** streaming/preemption element (`simple PreemptableStreamer extends inet.queueing.base.PacketProcessorBase like IPacketFlow`; C++ `class PreemptableStreamer : public ClockUserModuleMixin<PacketProcessorBase>, public virtual IPacketFlow`). Not a `FragmenterBase`/`DefragmenterBase` subclass, but grouped here because it manipulates `FragmentTag`s to implement 802.1Qbu-style frame preemption during pull-based streaming.
- **Does:** Supports push and pull, and both packet-passing and packet-streaming on its gates. On push, streams the whole packet at `datarate` (instantly if `datarate` is NaN) and ends streaming on a clock timer. On pull, `pullPacketStart()`/`pullPacketEnd()` can preempt an in-flight pull: it computes how much data was pulled during the elapsed time (rounded up to `roundingLength`, floored at `minPacketLength`); if enough data remains (`preemptedLength + minPacketLength <= packetLength`), it splits the packet — renames/retags the pulled part as a fragment (`lastFragment=false`, strips `PacketProtocolTag`) and stores the remainder as a new `Packet` (with a fresh `FragmentTag`: `firstFragment=false`, `lastFragment=true`, `fragmentNumber+1`) to return on the next pull. If an inbound pulled packet has no `FragmentTag` yet, one is added with defaults (`firstFragment=true, lastFragment=true, fragmentNumber=0, numFragments=-1`).
- **Parameters:** `string clockModule = ""` — optional relative path to an `IClock` module for timing; `double datarate: bps = NaN bps` — streaming rate (NaN = instantaneous); `int minPacketLength: b` (mandatory) — minimum length a preempted chunk or remainder must have; `int roundingLength: b = 1B` — granularity the preempted length is rounded up to
- **Gates:** `input in @labels(send,push,pull,pass)`; `output out @labels(send,push,pull,stream)`
- **Header/tag:** reads/writes `FragmentTag` (`firstFragment`, `lastFragment`, `fragmentNumber`, `numFragments`) on the packets it streams/splits; no wire header

### `FragmentNumberHeaderSerializer` (serializer)
- **Role/base:** serializer for `FragmentNumberHeader`; C++ `class FragmentNumberHeaderSerializer : public FieldsChunkSerializer`, registered via `Register_Serializer(FragmentNumberHeader, FragmentNumberHeaderSerializer)`
- **Does:** `serialize()` packs `fragmentNumber` (7 bits, `& 0x7F`) and `lastFragment` (bit 7, `0x80`) into a single byte. `deserialize()` unpacks that byte back into `fragmentNumber`/`lastFragment`.
- **Parameters:** none (not a NED module)
- **Gates:** none
- **Header/tag:** converts `FragmentNumberHeader` (`FieldsChunk`, `chunkLength = B(1)`, fields `int fragmentNumber; bool lastFragment;`) to/from its 1-byte wire format

## checksum
Adds/verifies a checksum trailer/header for data-integrity checks. **No Strategy-pattern
policy submodule exists here** — unlike aggregation/fragmentation, `base/ChecksumCheckerBase` and
`base/ChecksumInserterBase` implement checksum-mode dispatch (`disabled` / `declared correct` /
`declared incorrect` / `computed`) directly as a C++ `switch`, not via a pluggable submodule; the
`checksumType` (algorithm: internet, ethernet-fcs, crc32c, crc16-ibm, crc16-ccitt) is likewise a
plain string parameter. `ChecksumHeaderChecker`/`ChecksumHeaderInserter` add wire-format
(`ChecksumHeader`) handling on top of the base classes, and `EthernetFcsHeader*`/
`InternetChecksumHeader*` are further NED-only specializations that just pin `checksumType`.

### `ChecksumCheckerBase` (abstract base)
- **Role/base:** checksum-checker base (`simple ChecksumCheckerBase extends inet.queueing.base.PacketFilterBase`; C++ matches)
- **Does:** `checkChecksum(mode, type, checksum)` dispatches on `checksumMode`: `checkDisabledChecksum` — asserts the value is `0x0000` and always reports valid; `checkDeclaredCorrectChecksum` — asserts a magic `0xC00D`/`0xC00DC00D` marker, reports valid iff the underlying chunk deserialized correctly and the packet has no bit error; `checkDeclaredIncorrectChecksum` — asserts a magic `0xBAAD`/`0xBAADBAAD` marker, always reports invalid; `checkComputedChecksum` — recomputes the real checksum over the packet's bytes for `checksumType` and compares to the received value (also requiring correct deserialization / no bit error). Subclasses supply `matchesPacket()` (the `PacketFilterBase` pass/drop hook) using the header/value they parsed.
- **Parameters:** `string checksumType @enum("internet","ethernet-fcs","crc32c","crc16-ibm","crc16-ccitt") = "ethernet-fcs"`
- **Gates:** in / out
- **Header/tag:** none itself (subclass pops the `ChecksumHeader`)

### `ChecksumInserterBase` (abstract base)
- **Role/base:** checksum-inserter base (`simple ChecksumInserterBase extends inet.queueing.base.PacketFlowBase`; C++ matches)
- **Does:** `computeChecksum(mode, type)` dispatches on `checksumMode` to `computeDisabledChecksum` (returns `0x0000`), `computeDeclaredCorrectChecksum` (returns magic `0xC00DC00D`), `computeDeclaredIncorrectChecksum` (returns magic `0xBAADBAAD`), or `computeComputedChecksum` (calls `inet::computeChecksum()` over the packet's serialized bytes for `checksumType`). Subclasses build the wire header from this value.
- **Parameters:** `string checksumType @enum("internet","ethernet-fcs","crc32c","crc16-ibm","crc16-ccitt") = "ethernet-fcs"`; `string checksumMode @enum("disabled","declared","computed") = "declared"`
- **Gates:** in / out
- **Header/tag:** none itself

### `ChecksumHeaderChecker` (simple)
- **Role/base:** header-based checksum checker (`simple ChecksumHeaderChecker extends ChecksumCheckerBase`)
- **Does:** Registers as service/protocol `AccessoryProtocol::checksum`. `processPacket()` pops a `ChecksumHeader` of `getChecksumSizeInBytes(checksumType)` bytes at `headerPosition`. `matchesPacket()` peeks the same header and calls the inherited `checkChecksum()` with the header's stored `checksumMode`/`checksum` value.
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** pops `ChecksumHeader` (`checksum: uint64_t`, `checksumMode` enum; wire size 2 bytes for internet/crc16-ibm/crc16-ccitt, 4 bytes for ethernet-fcs/crc32c, `chunkLength` set programmatically per `checksumType`)

### `ChecksumHeaderInserter` (simple)
- **Role/base:** header-based checksum inserter (`simple ChecksumHeaderInserter extends ChecksumInserterBase`)
- **Does:** Registers as service/protocol `AccessoryProtocol::checksum`. `processPacket()` builds a `ChecksumHeader`, computes the value via the inherited `computeChecksum()`, sets the header's `chunkLength` to `getChecksumSizeInBytes(checksumType)`, stores `checksumMode` in the header, inserts it at `headerPosition`, and tags the packet with `PacketProtocolTag = AccessoryProtocol::checksum`.
- **Parameters:** `string headerPosition @enum("front","back") = "front"`
- **Gates:** in / out
- **Header/tag:** inserts `ChecksumHeader` (`checksum: uint64_t = 0`, `checksumMode` enum = `CHECKSUM_MODE_UNDEFINED` by default; wire size 2 or 4 bytes per `checksumType`, set programmatically)

### `EthernetFcsHeaderChecker` (simple)
- **Role/base:** checksum checker fixed to the Ethernet FCS algorithm (`simple EthernetFcsHeaderChecker extends ChecksumHeaderChecker`) — pure NED specialization, no own `.h`/`.cc`, inherits `@class` and all behavior from `ChecksumHeaderChecker`
- **Does:** identical to `ChecksumHeaderChecker`, with `checksumType` pinned
- **Parameters:** `checksumType = "ethernet-fcs"` (fixed); `headerPosition`, and all `ChecksumCheckerBase` parameters inherited/overridable
- **Gates:** in / out
- **Header/tag:** same `ChecksumHeader`, 4-byte wire size (CRC32-based Ethernet FCS)

### `EthernetFcsHeaderInserter` (simple)
- **Role/base:** checksum inserter fixed to the Ethernet FCS algorithm (`simple EthernetFcsHeaderInserter extends ChecksumHeaderInserter`) — pure NED specialization, no own `.h`/`.cc`
- **Does:** identical to `ChecksumHeaderInserter`, with `checksumType` pinned
- **Parameters:** `checksumType = "ethernet-fcs"` (fixed); `headerPosition`, `checksumMode` inherited/overridable
- **Gates:** in / out
- **Header/tag:** same `ChecksumHeader`, 4-byte wire size

### `InternetChecksumHeaderChecker` (simple)
- **Role/base:** checksum checker fixed to the Internet checksum algorithm (`simple InternetChecksumHeaderChecker extends ChecksumHeaderChecker`) — pure NED specialization, no own `.h`/`.cc`
- **Does:** identical to `ChecksumHeaderChecker`, with `checksumType` pinned
- **Parameters:** `checksumType = "internet"` (fixed); `headerPosition` inherited/overridable
- **Gates:** in / out
- **Header/tag:** same `ChecksumHeader`, 2-byte wire size

### `InternetChecksumHeaderInserter` (simple)
- **Role/base:** checksum inserter fixed to the Internet checksum algorithm (`simple InternetChecksumHeaderInserter extends ChecksumHeaderInserter`) — pure NED specialization, no own `.h`/`.cc`
- **Does:** identical to `ChecksumHeaderInserter`, with `checksumType` pinned
- **Parameters:** `checksumType = "internet"` (fixed); `headerPosition`, `checksumMode` inherited/overridable
- **Gates:** in / out
- **Header/tag:** same `ChecksumHeader`, 2-byte wire size

### `ChecksumHeaderSerializer` (serializer)
- **Role/base:** serializer for `ChecksumHeader`; C++ `class ChecksumHeaderSerializer : public FieldsChunkSerializer`, registered via `Register_Serializer(ChecksumHeader, ChecksumHeaderSerializer)`
- **Does:** `serialize()` requires `checksumMode` to be `CHECKSUM_DISABLED` or `CHECKSUM_COMPUTED` (throws otherwise — `declared-correct`/`declared-incorrect` checksums cannot be put on the wire), then writes the checksum value in network byte order, sized 1/2/4/8 bytes according to the header's `chunkLength`. `deserialize()` reads that many bytes back and infers `checksumMode` as `CHECKSUM_DISABLED` if the value is `0`, else `CHECKSUM_COMPUTED`.
- **Parameters:** none (not a module)
- **Gates:** none
- **Header/tag:** converts `ChecksumHeader` (`FieldsChunk`, fields `uint64_t checksum = 0; ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;`; wire size 1/2/4/8 bytes set programmatically, no mode byte on the wire) to/from bytes

---


Source root: `/home/levy/workspace/inet-protocolelement/src/inet/protocolelement`
All modules build on the `inet.queueing` push/pull packet-processing framework
(`PacketProcessorBase` → `PacketFlowBase` / `PacketFilterBase` / `PacketClassifierBase` /
`PacketMeterBase` / `PacketGateBase` / `PacketPusherBase` / `PacketDuplicatorBase` / `PacketQueueBase`).
Where the `.ned` inheritance and the real C++ base class in the `.h`/`.cc` diverge, both are called out.

---

## ordering

Generic sequence-numbering / reordering / duplicate-removal building blocks. A `SequenceNumberHeader`
is prepended to packets by `SequenceNumbering`, then consumed by `Reordering` and/or `DuplicateRemoval`
downstream. Not tied to a specific IEEE standard in this directory (it is generic infrastructure that
other protocols, e.g. redundancy/802.1CB, layer stream-specific sequencing on top of via a separate
`SequenceNumberReq`/`SequenceNumberInd` tag pair defined elsewhere in `inet.common`). A matching
`SequenceNumberHeaderSerializer` provides the wire format.

### `SequenceNumbering` (simple)
- **Role/base:** `SequenceNumbering : public PacketFlowBase` (matches `.ned extends PacketFlowBase`)
- **Does:** Maintains an internal `int sequenceNumber` counter starting at 0; on every packet, inserts a `SequenceNumberHeader` at the front with the current counter value and post-increments it; tags the packet's `PacketProtocolTag` with `AccessoryProtocol::sequenceNumber`.
- **Parameters:** none (only `@class`)
- **Gates:** `in` / `out` (inherited from `PacketFlowBase`)
- **Header/tag:** writes `SequenceNumberHeader` (`FieldsChunk`, `chunkLength = B(2)`, field `int sequenceNumber`); sets `PacketProtocolTag`.

### `Reordering` (simple)
- **Role/base:** `Reordering : public PacketPusherBase` (matches `.ned`)
- **Does:** Buffers pushed packets in a `std::map<int, Packet*>` keyed by the sequence number popped from `SequenceNumberHeader`. When the arriving sequence number equals `expectedSequenceNumber`, flushes all consecutively-buffered packets in order (incrementing `expectedSequenceNumber` each time) to the output. Registers itself as service/protocol for `AccessoryProtocol::sequenceNumber`. Per the `.ned` doc comment, there is no loss-recovery mechanism — a lost packet stalls delivery of everything after it indefinitely.
- **Parameters:** none (`@class`, `@display` only)
- **Gates:** `in` / `out`
- **Header/tag:** pops `SequenceNumberHeader` (`sequenceNumber` field) to determine ordering; does not re-attach it.

### `DuplicateRemoval` (simple)
- **Role/base:** `DuplicateRemoval : public PacketPusherBase` (matches `.ned`)
- **Does:** Tracks `lastSequenceNumber` (init `-1`). Pops `SequenceNumberHeader`; if its sequence number equals `lastSequenceNumber`, deletes the packet (duplicate); otherwise updates `lastSequenceNumber` and forwards. Per the `.ned` doc comment this strategy requires packets to already arrive in order.
- **Parameters:** none
- **Gates:** `in` / `out`
- **Header/tag:** pops `SequenceNumberHeader` (`sequenceNumber` field).

### `SequenceNumberPacketClassifierFunction` (C++ classifier function, not a `.ned` module)
- **Role/base:** `SequenceNumberPacketClassifierFunction : public cObject, public IPacketClassifierFunction`, registered via `Register_Class`. Meant to be plugged into a generic queueing classifier module (e.g. `PacketClassifier`) via a `classifierClass` string parameter, not instantiated on its own.
- **Does:** `classifyPacket()` peeks the `SequenceNumberHeader` at the front of the packet and returns its `sequenceNumber` as the classification index (e.g., to route packets to per-slot gates/buffers by sequence number).
- **Parameters:** n/a (plain C++ class)
- **Gates:** n/a
- **Header/tag:** reads `SequenceNumberHeader.sequenceNumber`; none written.

---

## redundancy

IEEE 802.1CB (Frame Replication and Elimination for Reliability, FRER)-style building blocks. Packets
are associated with a named "stream" (carried in `StreamReq`/`StreamInd` tags, not a header chunk), and
that stream identity drives duplication (splitting across paths), duplicate elimination (merging), and
VLAN/PCP encoding used to physically carry the stream tag on the wire. Three `*Layer.ned` compound
modules wire pairs of these into `IProtocolLayer` building blocks.

### `StreamTag.msg` (shared tag, not a module)
- `StreamTagBase extends TagBase` (abstract) — field `string streamName`
- `StreamReq extends StreamTagBase` — request: which stream to send a packet on (attached app → MAC direction)
- `StreamInd extends StreamTagBase` — indication: which stream a packet was received on (MAC → app direction)
- These are plain tags (no `chunkLength`, not wire-serialized); the actual on-wire indicator of stream membership is VLAN ID/PCP set by `StreamEncoder`/read by `StreamDecoder`.

### `StreamIdentifier` (simple, `like IPacketFlow`)
- **Role/base:** `StreamIdentifier : public PacketFlowBase, public TransparentProtocolRegistrationListener` (matches `.ned extends PacketFlowBase`)
- **Does:** If the packet already has a `StreamReq`, leaves it. Otherwise iterates the `mapping` array, evaluating each entry's `packetFilter` (via `PacketFilter`) against the packet; on first match, attaches `StreamReq` with that entry's `stream` name, and — if `hasSequenceNumbering` is true and the entry's `sequenceNumbering` flag is true — attaches a `SequenceNumberReq` (external `inet.common` tag) with a per-stream auto-incrementing counter (`incrementSequenceNumber`). Registration pass-through maps `in`↔`out`.
- **Parameters:** `hasSequenceNumbering: bool = true` — globally enables honoring the per-entry `sequenceNumbering` flag; `mapping: object = []` — array of `{stream, packetFilter, sequenceNumbering}` objects (mutable at runtime)
- **Gates:** `in` / `out`
- **Header/tag:** writes `StreamReq`; optionally writes `SequenceNumberReq` (defined outside this directory, in `inet.common.SequenceNumberTag` — distinct from `ordering`'s `SequenceNumberHeader` chunk).

### `StreamEncoder` (simple, `like IPacketFlow`)
- **Role/base:** `StreamEncoder : public PacketFlowBase, public TransparentProtocolRegistrationListener` (matches `.ned`)
- **Does:** Looks up the packet's `StreamReq` stream name in `mapping` (`{stream, vlan, pcp}`); on match sets `PcpReq`/`VlanReq` tags for any configured pcp/vlan; if the packet carries a `SequenceNumberReq`, ensures encapsulation of `Protocol::ieee8021rTag` (the 802.1CB R-TAG carrying the sequence number); if pcp or vlan was set, ensures encapsulation of `Protocol::ieee8021qCTag` (802.1Q C-TAG); calls `setDispatchProtocol`.
- **Parameters:** `mapping: object = []` — array of `{stream, vlan, pcp}` objects (mutable)
- **Gates:** `in` / `out`
- **Header/tag:** reads `StreamReq`, `SequenceNumberReq`; writes `PcpReq`, `VlanReq`; triggers R-TAG/C-TAG header encapsulation (headers themselves live in `linklayer`, not this directory).

### `StreamDecoder` (simple, `like IPacketFlow`)
- **Role/base:** `StreamDecoder : public PacketFlowBase, public TransparentProtocolRegistrationListener` (matches `.ned`)
- **Does:** For each `mapping` entry (`packetFilter`, `interface` name pattern, `source`/`destination` MAC, `vlan`, `pcp`, `stream`), checks the packet against `MacAddressInd`, `PcpInd`, `VlanInd`, `InterfaceInd` tags (interface name resolved via the referenced `IInterfaceTable`) and the packet filter; on the first fully-matching entry, attaches `StreamInd` with that stream name.
- **Parameters:** `interfaceTableModule: string` (required, no default) — path to the `IInterfaceTable`; `mapping: object = []` — array of `{stream, interface, source, destination, vlan, pcp, packetFilter}` (mutable)
- **Gates:** `in` / `out`
- **Header/tag:** reads `MacAddressInd`, `PcpInd`, `VlanInd`, `InterfaceInd`; writes `StreamInd`.

### `StreamClassifier` (simple, `like IPacketClassifier`)
- **Role/base:** `StreamClassifier : public queueing::PacketClassifierBase` (matches `.ned`)
- **Does:** Reads the stream name from `StreamReq` and/or `StreamInd` depending on `mode` (`"req"` reads only `StreamReq`; `"ind"` only `StreamInd`; `"both"` prefers `StreamReq`, falls back to `StreamInd`). Looks the name up in `mapping` (stream → gate index), adds `gateIndexOffset`; if the resulting gate's consumer can accept the packet, routes there, otherwise (or if no match) routes to `defaultGateIndex`.
- **Parameters:** `mode: string @enum("req","ind","both") = "both"`; `mapping: object = {}` — stream name → output gate index; `gateIndexOffset: int = 0`; `defaultGateIndex: int = 0`
- **Gates:** `in` / `out[]` (vector output, from `PacketClassifierBase`)
- **Header/tag:** reads `StreamReq` / `StreamInd`.

### `StreamFilter` (simple, `like IPacketFilter`)
- **Role/base:** `StreamFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener` (matches `.ned`)
- **Does:** Reads the stream name per `mode` (same `req`/`ind`/`both` logic as `StreamClassifier`) and matches it against a `cMatchExpression` built from `streamNameFilter`; packets with no resolvable stream name never match (filtered out).
- **Parameters:** `mode: string @enum("req","ind","both") = "both"`; `streamNameFilter: string = "*"` — match expression on the stream name
- **Gates:** `in` / `out`
- **Header/tag:** reads `StreamReq` / `StreamInd`; no tags written.

### `StreamMerger` (simple, `like IPacketFilter`)
- **Role/base:** `StreamMerger : public PacketFilterBase, public TransparentProtocolRegistrationListener` (matches `.ned`) — implements the 802.1CB "elimination"/duplicate-recovery function
- **Does:** For a packet carrying `StreamInd` whose name is a key in `mapping`, `matchesPacket` checks whether the packet's `SequenceNumberInd` value is already present in the sliding-window history (`sequenceNumbers[outputStreamName]`, capped at `bufferSize` entries) recorded for the mapped *output* stream — if so, the packet is filtered (dropped) as a duplicate. On pass, `processPacket` records the sequence number into that window and renames the packet's `StreamInd` to the mapped output stream name; if the mapping's output name is the empty string, `StreamInd`/`SequenceNumberInd` tags are stripped instead (stream membership cleared).
- **Parameters:** `mapping: object = {}` — input stream name → output stream name, `""` = drop stream tag (mutable); `bufferSize: int = 10` — per-output-stream sequence-number history window
- **Gates:** `in` / `out`
- **Header/tag:** reads/writes `StreamInd`; reads `SequenceNumberInd` (external `inet.common` tag).

### `StreamSplitter` (simple, `like IPacketPusher`)
- **Role/base — mismatch:** `.ned` declares `extends PacketPusherBase` (so its NED-level gates are `in`/`out` from `PacketPusherBase`), but the real C++ base is `StreamSplitter : public PacketDuplicatorBase, public TransparentProtocolRegistrationListener` — implements the 802.1CB "sequence generation"/replication function via packet duplication rather than flow-through processing.
- **Does:** On `pushPacket`, reads the packet's `StreamReq` stream name; if it is a key in `mapping`, duplicates the packet once per entry of the mapped output-stream-name array, tagging each duplicate's `StreamReq` with the corresponding split stream name and pushing it downstream, then deletes the original. If the stream isn't in `mapping` (or has no `StreamReq`), the original packet passes through unchanged. `getNumPacketDuplicates` reports `(list size − 1)` for the duplicator framework's accounting.
- **Parameters:** `mapping: object = {}` — input stream name → array of output stream names, `""` element = no stream (mutable)
- **Gates:** `in` / `out` (per `.ned`'s `PacketPusherBase`)
- **Header/tag:** reads `StreamReq`; writes `StreamReq` (renamed) on each duplicate.

### `StreamCoderLayer` (compound, `like IProtocolLayer`)
- **Role/base:** compound module
- **Does:** Combines a `decoder: StreamDecoder` (default typename) and `encoder: StreamEncoder` (default typename) into one `IProtocolLayer`. Upstream traffic (`upperLayerIn`) → `encoder` → `lowerLayerOut`; downstream traffic (`lowerLayerIn`) → `decoder` → `upperLayerOut`. Both submodules are optional (`if typename != ""`).
- **Parameters:** `interfaceTableModule: string`, propagated to submodules via `*.interfaceTableModule = default(this.interfaceTableModule)`
- **Gates:** `upperLayerIn`/`upperLayerOut`, `lowerLayerIn`/`lowerLayerOut`
- **Header/tag:** none directly (delegates to submodules).

### `StreamIdentifierLayer` (compound, `like IProtocolLayer`)
- **Role/base:** compound module
- **Does:** Wraps a single `identifier: StreamIdentifier` (default typename) as a protocol layer. `upperLayerIn` → `identifier.in`, `identifier.out` → `lowerLayerOut`; `lowerLayerIn` passes straight through to `upperLayerOut` (no processing on the receive path).
- **Parameters:** none of its own
- **Gates:** `upperLayerIn`/`upperLayerOut`, `lowerLayerIn`/`lowerLayerOut`
- **Header/tag:** none directly.

### `StreamRelayLayer` (compound, `like IProtocolLayer`)
- **Role/base:** compound module
- **Does:** Combines `merger: StreamMerger` (default typename) and `splitter: StreamSplitter` (default typename). Upstream traffic (`upperLayerIn`) → `splitter` → `lowerLayerOut` (replication going out); downstream traffic (`lowerLayerIn`) → `merger` → `upperLayerOut` (elimination coming in).
- **Parameters:** `interfaceTableModule: string`, propagated to submodules
- **Gates:** `upperLayerIn`/`upperLayerOut`, `lowerLayerIn`/`lowerLayerOut`
- **Header/tag:** none directly.

---

## shaper

IEEE 802.1Qcr Asynchronous Traffic Shaping (ATS) building blocks. Each packet is stamped with an
`EligibilityTimeTag` giving the simulation time after which it may be sent; a queue keeps packets
sorted by that time, and a gate holds back a packet until its eligibility time is reached. A
group-aware meter variant implements per-(port,priority)-group eligibility tracking as specified in
802.1Qcr §8.6.11.3.10 ("most recent value of the eligibilityTime variable from the previous frame").

### `EligibilityTimeTag.msg` (shared tag, not a module)
- `EligibilityTimeTag extends TagBase` — field `clocktime_t eligibilityTime` (the time after which the packet is eligible for transmission). No `chunkLength` (it's a tag, not a `Chunk`/header).

### `EligibilityTimeMeter` (simple, `like IPacketMeter`)
- **Role/base:** `EligibilityTimeMeter : public ClockUserModuleMixin<PacketMeterBase>` (`.ned extends PacketMeterBase` — matches modulo the clock-aware mixin)
- **Does:** Implements a token-bucket-style ATS scheduler per flow. On each packet, computes `lengthRecoveryDuration` from `(dataLength + packetOverheadLength) / committedInformationRate`, derives `schedulerEligibilityTime` and `bucketFullTime` from the running `bucketEmptyTime`, and sets `eligibilityTime = max(arrivalTime, groupEligibilityTime, schedulerEligibilityTime)`. If `maxResidenceTime` is unset (`-1`) or the computed eligibility time doesn't exceed `arrivalTime + maxResidenceTime`, updates `groupEligibilityTime`/`bucketEmptyTime` and attaches `EligibilityTimeTag` (otherwise the packet is left untagged, effectively causing it to be dropped downstream by `EligibilityTimeFilter`). Also emits a `tokensChanged` signal reflecting bucket fill level.
- **Parameters:** `clockModule: string = ""` — optional `IClock` reference; `packetOverheadLength: int @unit(b) = 0b` — extra length credited to lower-layer overhead; `committedInformationRate: double @unit(bps)` (required) — CIR of the flow; `committedBurstSize: int @unit(b)` (required) — token bucket depth; `maxResidenceTime: double @unit(s) = -1s` — optional cap on computed eligibility delay
- **Gates:** `in` / `out` (via `PacketFlowBase` → `PacketMeterBase`)
- **Header/tag:** writes `EligibilityTimeTag` (if within `maxResidenceTime`).

### `GroupEligibilityTimeMeter` (simple, extends `EligibilityTimeMeter`)
- **Role/base:** `GroupEligibilityTimeMeter : public EligibilityTimeMeter` (matches `.ned extends EligibilityTimeMeter`)
- **Does:** Overrides `meterPacket` to compute a per-group `groupEligibilityTime` instead of a purely per-instance one: builds a group key `"<interfaceId>-<pcp>"` (ingress `InterfaceInd`; PCP taken from `PcpInd` on inbound packets or `PcpReq` on outbound, selected via `DirectionTag`), reads the group's current eligibility time from the referenced `GroupEligibilityTimeTable`, uses it in the same `max(...)` formula as the base class, and — if accepted — writes the updated value back into the table (`updateGroupEligibilityTime`) so other flows/meters sharing the group observe it.
- **Parameters:** `groupEligibilityTimeTableModule: string` (required, no default) — path to the shared `GroupEligibilityTimeTable`; inherits all `EligibilityTimeMeter` parameters
- **Gates:** `in` / `out` (inherited)
- **Header/tag:** reads `DirectionTag`, `PcpInd`/`PcpReq`, `InterfaceInd`; writes `EligibilityTimeTag` (same as base).

### `GroupEligibilityTimeTable` (simple)
- **Role/base:** `GroupEligibilityTimeTable : public SimpleModule` (matches `.ned extends SimpleModule` — a bare, non-packet-processing module, not part of the `queueing` gate chain)
- **Does:** Maintains `std::map<std::string, clocktime_t>` from group key to eligibility time. `updateGroupEligibilityTime(group, newTime)` only overwrites if `newTime` is more recent than the stored value (monotonic-increase semantics per 802.1Qcr §8.6.11.3.10). `getGroupEligibilityTime(group)` returns (and lazily zero-initializes) the stored value. Referenced by `GroupEligibilityTimeMeter` instances via `ModuleRefByPar`, shared across flows within the same group.
- **Parameters:** none (`@class` only)
- **Gates:** none (plain side-table module, no packet gates)
- **Header/tag:** none; operates purely on the `(group key → clocktime_t)` map, not on packet contents.

### `EligibilityTimeFilter` (simple, `like IPacketFilter`)
- **Role/base:** `EligibilityTimeFilter : public ClockUserModuleMixin<PacketFilterBase>, public TransparentProtocolRegistrationListener` (matches `.ned extends PacketFilterBase` modulo clock mixin)
- **Does:** Drops any packet lacking an `EligibilityTimeTag`; if present and `maxResidenceTime != -1`, also drops packets whose `eligibilityTime` exceeds `arrivalTime + maxResidenceTime` (i.e., packets that would need to wait too long are dropped rather than shaped).
- **Parameters:** `clockModule: string = ""`; `maxResidenceTime: double @unit(s) = -1s`
- **Gates:** `in` / `out`
- **Header/tag:** reads `EligibilityTimeTag`; none written.

### `EligibilityTimeGate` (simple, `like IPacketGate`)
- **Role/base:** `EligibilityTimeGate : public ClockUserModuleMixin<PacketGateBase>` (matches `.ned extends PacketGateBase` modulo clock mixin)
- **Does:** A pull-side gate that inspects (without consuming) the next pullable packet's `EligibilityTimeTag`; opens if its eligibility time has passed (or there is no packet), closes and self-schedules a `ClockEvent` (`eligibilityTimer`) to re-check exactly at that eligibility time otherwise. Recomputes/opens on `pullPacket` and on `handleCanPullPacketChanged`. Emits a `remainingEligibilityTimeChanged` signal for monitoring.
- **Parameters:** `clockModule: string = ""` (own); inherits `bitrate`, `extraLength`, `extraDuration` from `PacketGateBase` (unused by the eligibility logic itself)
- **Gates:** `in` / `out` (via `PacketFlowBase` → `PacketGateBase`)
- **Header/tag:** reads `EligibilityTimeTag` on the head-of-line packet; none written.

### `EligibilityTimeQueue` (simple, extends `PacketQueue`)
- **Role/base:** no dedicated C++ class — plain `PacketQueue` (`queueing::PacketQueue`) with `comparatorClass` overridden
- **Does:** A standard `PacketQueue` (FIFO with optional capacity/dropper) reconfigured to keep packets sorted in ascending order by `EligibilityTimeTag.eligibilityTime` via the registered comparator function `inet::PacketEligibilityTimeComparator`.
- **Parameters:** `comparatorClass: string = "inet::PacketEligibilityTimeComparator"` (overridden default); all other `PacketQueue` parameters (`packetCapacity`, `dataCapacity`, `dropperClass`, `bufferModule`, ...) inherited unchanged
- **Gates:** `in` / `out` (from `PacketQueueBase`)
- **Header/tag:** none written; ordering key is `EligibilityTimeTag`.

### `EligibilityTimeComparatorFunction` (C++ comparator function, not a `.ned` module)
- **Role/base:** free function `comparePacketsByEligibilityTime`, registered as `PacketEligibilityTimeComparator` via `Register_Packet_Comparator_Function`
- **Does:** Compares two packets' `EligibilityTimeTag.eligibilityTime` values (returns -1/0/1), used by `EligibilityTimeQueue` to keep the queue sorted by eligibility time.
- **Parameters/Gates:** n/a (plain C++ function)
- **Header/tag:** reads `EligibilityTimeTag` from both compared packets.

---

## cutthrough

Cut-through switching support: lets a packet start being forwarded before it has fully arrived, by
splitting it at a configurable byte offset into a "head" that streams normally and a buffered "tail"
(carried via a `StreamBufferChunk`) that is spliced back on once it has arrived, all tracked via a
`CutthroughTag`.

### `CutthroughTag.msg` (shared tag, not a module)
- `CutthroughTag extends TagBase` — fields: `b cutthroughPosition` (byte/bit offset at which the packet was split), `ChunkPtr trailerChunk` (placeholder/reference for the not-yet-arrived tail region). No `chunkLength` (tag, not a `Chunk` header).

### `CutthroughSource` (simple, extends `PacketDestreamer`)
- **Role/base:** `CutthroughSource : public PacketDestreamer` (matches `.ned extends inet.protocolelement.common.PacketDestreamer`)
- **Does:** Overrides streaming push handlers. `pushPacketStart` starts the normal destreaming timing and schedules `cutthroughTimer` to fire after `cutthroughPosition / datarate` (i.e., as soon as enough of the packet has arrived to know its header up to the cut-through point). When the timer fires, it duplicates the in-flight `streamedPacket`, removes everything from `cutthroughPosition` onward into a `StreamBufferChunk` (buffering the not-yet-arrived tail, timestamped and rated), appends that buffer chunk at the back, copies region tags for the removed region, attaches a `CutthroughTag` recording `cutthroughPosition`, and immediately forwards this partial ("cut-through") packet downstream — before the original has finished arriving. `pushPacketEnd` (real end of arrival) then discards the original fully-streamed packet, extracts the true tail data from the incoming complete packet, and fills in `cutthroughBuffer`'s stream data (satisfying the buffer for whoever reads the cut-through packet's tail later).
- **Parameters:** `cutthroughPosition: int @unit(b)` (required) — byte offset at which to cut through
- **Gates:** `in` (streaming, labels `send,push,pull,stream`) / `out` (labels `send,push,pull,pass`) — from `PacketDestreamer`
- **Header/tag:** writes `CutthroughTag` (`cutthroughPosition`); manipulates a `StreamBufferChunk` appended at the packet's tail (buffer for not-yet-received bytes).

### `CutthroughSink` (simple, extends `PacketStreamer`)
- **Role/base:** `CutthroughSink : public PacketStreamer` (matches `.ned extends inet.protocolelement.common.PacketStreamer`)
- **Does:** Overrides `endStreaming()`: if the packet being finished carries a `CutthroughTag`, removes the `StreamBufferChunk` at `cutthroughPosition`, appends its (by-then fully available) stream data back onto the packet, and removes the `CutthroughTag` — reassembling the packet into a normal, complete chunk sequence — before delegating to the base `PacketStreamer::endStreaming()` to finish streaming it out.
- **Parameters:** none beyond `@class` (inherits `clockModule`, `datarate` from `PacketStreamer`)
- **Gates:** `in` (labels `send,push,pull,pass`) / `out` (streaming, labels `send,push,pull,stream`) — from `PacketStreamer`
- **Header/tag:** reads and removes `CutthroughTag`; consumes the `StreamBufferChunk` written by `CutthroughSource`.

---


Scope: `transceiver` (incl. `base`, `contract`), `common`, `lifetime`, `measurement`, `processing`, `socket`, `trafficconditioner`, `service`, `contract`, under
`/home/levy/workspace/inet-protocolelement/src/inet/protocolelement`.

All modules build on the `inet.queueing` push/pull packet-processing framework (`PacketProcessorBase`, `PacketFlowBase`, `PacketFilterBase`, `PacketPusherBase`, etc.) and, where noted, on `OperationalMixin<T>` (adds NIC start/stop/crash lifecycle via `ILifecycle`) and `ClockUserModuleMixin<T>` (adds optional external-clock support via a `clockModule` parameter). Several `.ned` files declare a NED-level `extends` that does **not** match the actual C++ base class in the `.h`; these are flagged explicitly below since they matter for a design document.

---

## transceiver

Implements the physical-transmission boundary: converting between whole-packet handling (upper layers) and time-extended streaming of signal start/progress/end onto/off of a wire (`cChannel`/`cGate` signal sends). Includes `transceiver/base` (abstract bases) and `transceiver/contract` (moduleinterfaces).

### `IPacketReceiver` (moduleinterface) — `transceiver/contract`
- **Role/base:** interface
- **Does:** Contract for anything that receives `Signal`s off the wire and emits `Packet`s on `out`.
- **Parameters:** none (display icon only)
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `IPacketTransmitter` (moduleinterface) — `transceiver/contract`
- **Role/base:** interface
- **Does:** Contract for anything that receives `Packet`s on `in` and sends `Signal`s onto the wire on `out`.
- **Parameters:** none
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `PacketReceiverBase` (abstract base) — `transceiver/base`
- **Role/base:** NED `extends PacketProcessorBase`. Real C++ base: `OperationalMixin<PacketProcessorBase>`, `virtual IActivePacketSource` — adds lifecycle (start/stop/crash) handling not visible in the NED `extends`.
- **Does:** Owns `in`/`out` gates and a `PassivePacketSinkRef consumer` on `out`; resolves the containing `NetworkInterface`. `decodePacket()` unwraps a `Signal` into a `Packet`, tags it `DirectionTag(INBOUND)` and `InterfaceInd`, copies the signal's bit-error flag. `handleMessageWhenUp` always throws (concrete subclasses override it); `handleMessageWhenDown` silently drops non-self messages while the interface is down. Declares statistics/signals `receptionStarted`/`receptionEnded`, `throughput`, `propagationTime`, `receptionTime`, `flowReceptionTime`.
- **Parameters:** `datarate: double @unit(bps)` (no default) — nominal rx bit rate used for streaming/statistics.
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `PacketTransmitterBase` (abstract base) — `transceiver/base`
- **Role/base:** NED `extends PacketProcessorBase`, `@class(PacketTransmitterBase)`. Real C++ base: `ClockUserModuleMixin<OperationalMixin<PacketProcessorBase>>`, `virtual IPassivePacketSink` — adds both clock support and lifecycle, neither visible in the NED `extends`.
- **Does:** Owns `in`/`out` gates, an `ActivePacketSourceRef producer`, and a `txEndTimer` (`ClockEvent`). `encodePacket()` computes clock-time duration from `datarate`, tags `PacketTransmittedEvent`/`TransmissionTimeTag`/`PropagationTimeTag`, and wraps the `Packet` in a `Signal`. `prepareSignal()` strips all tags except `PacketProtocolTag` before the signal is sent on the channel. `sendSignalStart/Progress/End()` are the shared primitives (`send()` + `SendOptions().duration()/updateTx()/finishTx()`) reused by streaming subclasses. At this level `pushPacketStart/End/Progress` all throw "Invalid operation" — only whole-packet `canPushSomePacket` (gated on `!txEndTimer->isScheduled()`) is supported.
- **Parameters:** `clockModule: string = ""` (optional path to an `IClock` module); `datarate: volatile double @unit(bps)` (no default) — tx bit rate.
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `StreamingReceiverBase` (abstract base) — `transceiver/base`
- **Role/base:** NED `extends PacketReceiverBase`, `@class(StreamingReceiverBase)`. C++ base: `PacketReceiverBase`, `cListener` (matches NED, adds channel-model-change listening).
- **Does:** Subscribes to `PRE_MODEL_CHANGE`/`POST_MODEL_CHANGE` on itself and on the input's transmission channel so it can react to topology/channel changes mid-reception; `receiveSignal()` is currently a stub (TODO: handle channel cut at the receiver).
- **Parameters:** none beyond base
- **Gates:** inherited `in`/`out`
- **Header/tag:** none

### `StreamingTransmitterBase` (abstract base) — `transceiver/base`
- **Role/base:** NED `extends PacketTransmitterBase`, `@class(StreamingTransmitterBase)`. C++ base: `PacketTransmitterBase`, `cListener` (matches NED).
- **Does:** Adds channel-aware streaming: overrides `scheduleAt()` to intercept the tx-end self-timer and instead emit a signal-progress update when the channel changes the scheduled end time; `canPushSomePacket()` additionally requires the transmission channel to exist and not be disabled; `receiveSignal()` reacts to path-cut/channel-disabled pre-notifications by invoking the pure-virtual `abortTx()`, and to path-create/cut/param-change post-notifications by re-resolving `transmissionChannel` and notifying the producer. `scheduleTxEndTimer()` reschedules the clock-based end timer.
- **Parameters:** none beyond base
- **Gates:** inherited
- **Header/tag:** none

### `PacketReceiver` (simple) — `transceiver/`
- **Role/base:** NED `extends StreamingReceiverBase like IPacketReceiver`. **Mismatch:** the actual C++ class is `class PacketReceiver : public PacketReceiverBase` — it bypasses `StreamingReceiverBase`'s cListener/channel-change handling entirely, even though NED names `StreamingReceiverBase` as its parent.
- **Does:** Whole-packet receive ("receives signals ... as a whole"): on any incoming `Signal`, emits `receptionEndedSignal`, calls `decodePacket()`, `handlePacketProcessed()`, then `pushOrSendPacket()` to the upper layer and deletes the `Signal`. No partial/progressive reception.
- **Parameters:** none of its own (inherits `datarate`)
- **Gates:** inherited `in`/`out`
- **Header/tag:** none

### `PacketTransmitter` (simple) — `transceiver/`
- **Role/base:** NED `extends PacketTransmitterBase like IPacketTransmitter`. C++ base matches (`PacketTransmitterBase`).
- **Does:** Whole-packet transmit ("receives packets ... as a whole"). `pushPacket()`→`startTx()`: encodes the packet into a `Signal`, sends it in one shot via plain `send()` (not `sendSignalStart`/`sendSignalEnd`), schedules `txEndTimer` for the full duration. `endTx()` (on timer) emits `transmissionEndedSignal`, decapsulates, notifies the producer. `pushPacketProgress` throws — no partial pushes accepted.
- **Parameters:** none of its own
- **Gates:** inherited
- **Header/tag:** none

### `StreamingTransmitter` (simple) — `transceiver/`
- **Role/base:** NED `extends StreamingTransmitterBase like IPacketTransmitter`. C++ base matches.
- **Does:** Sends the signal start immediately (`sendSignalStart`) then, on the tx-end timer, sends the signal end (`sendSignalEnd`) — packet is streamed onto the wire as start+end (not whole, not progressive). `abortTx()` truncates the in-flight packet at the current time position, sets `bitError`, and sends a shortened signal end early (used when the channel is cut/interface stops mid-transmission). `supportsPacketStreaming()` is true on the output gate.
- **Parameters:** none of its own
- **Gates:** inherited
- **Header/tag:** none

### `DestreamingReceiver` (simple) — `transceiver/`
- **Role/base:** NED `extends PacketReceiverBase like IPacketReceiver`. **Mismatch (opposite direction from `PacketReceiver`):** actual C++ class is `class DestreamingReceiver : public StreamingReceiverBase` — the C++ base is *more* derived than the NED parent, since it needs `StreamingReceiverBase`'s `cListener`/channel machinery to track tx-update signals from the streaming sender correctly.
- **Does:** Receives a stream (`isUpdate()`+`getRemainingDuration()` distinguish start / progress / end of the incoming `Signal`), buffering only the latest partial `Signal` (`rxSignal`), but forwards a *whole* decoded `Packet` to the upper layer exactly once, at signal end (`sendToUpperLayer` → `pushOrSendPacket`). Sets `setTxUpdateSupport(true)` and `deliverImmediately` on the input gate in `initialize()`.
- **Parameters:** none of its own
- **Gates:** inherited
- **Header/tag:** none

### `StreamThroughReceiver` (simple) — `transceiver/`
- **Role/base:** NED `extends StreamingReceiverBase like IPacketReceiver`. C++ base matches.
- **Does:** True stream-through: on signal start, immediately decodes and relays the packet start onward (`pushOrSendPacketStart`); on progress, updates the buffered `rxSignal`; on signal end, decodes and relays the packet end (`pushOrSendPacketEnd`). The input stream is relayed to the output stream essentially concurrently, unlike `DestreamingReceiver` which surfaces the packet only once, whole, at the end. `supportsPacketStreaming()` always true.
- **Parameters:** none of its own
- **Gates:** inherited
- **Header/tag:** none

### `StreamThroughTransmitter` (simple) — `transceiver/`
- **Role/base:** NED `extends StreamingTransmitterBase like IPacketTransmitter`. C++ base matches.
- **Does:** Transmit-side mirror of `StreamThroughReceiver`: implements `pushPacketStart/Progress/End` (`pushPacket` throws — only streaming pushes accepted) so an upstream stream can be relayed onward as a stream rather than buffered whole first. Tracks last input/tx progress position+time to skip redundant signal-progress sends when the data is unchanged, and to compute/schedule a `bufferUnderrunTimer` (throws `cRuntimeError("Buffer underrun during transmission")` if the outbound datarate is slower than the inbound rate and the buffer would run dry). `abortTx()` truncates and marks `bitError` like `StreamingTransmitter`.
- **Parameters:** none of its own
- **Gates:** inherited
- **Header/tag:** none

---

## common

Utility/atomic flow elements reused across protocol stacks: serialization, streaming/destreaming, padding, gap enforcement, protocol registration checking, an internal-protocol registry, and header-position helpers.

### `AccessoryProtocol` (utility class — not a module) — `common/AccessoryProtocol.{h,cc}`
- **Role/base:** plain C++ class of `static const Protocol` members; no NED, no `cModule`.
- **Does:** Central registry of `inet::Protocol` singleton constants for "accessory" (internal/auxiliary) concerns used to tag packets/dissection steps, distinct from real wire protocols.
- **Protocols defined (11):**
  - `acknowledge` — "Acknowledge"
  - `aggregation` — "Aggregation"
  - `checksum` — "Checksum"
  - `destinationL3Address` — "Destination L3 address"
  - `destinationMacAddress` — "Destination MAC address"
  - `destinationPort` — "Destination port"
  - `forwarding` — "Forwarding"
  - `fragmentation` — "Fragmentation"
  - `hopLimit` — "Hop limit"
  - `sequenceNumber` — "Sequence number"
  - `withAcknowledge` — "With acknowledge"
- **Parameters/Gates:** n/a (not a module)
- **Header/tag:** none (it's a `Protocol` constant provider, not a chunk/tag)

### `HeaderPosition` (utility — not a module) — `common/HeaderPosition.{h,cc}`
- **Role/base:** plain header/enum, no NED
- **Does:** Defines `enum HeaderPosition { HP_UNDEFINED, HP_NONE, HP_FRONT, HP_BACK }`, `parseHeaderPosition(const char*)` (parses `""`/`"none"`/`"front"`/`"back"`), and template helpers `peekHeader<T>`/`popHeader<T>`/`insertHeader<T>` that dispatch to `packet->peekAtFront/popAtFront/insertAtFront` vs the `...AtBack` equivalents based on the enum. Used by header-inserter/checker-style modules (e.g. `PaddingInserter`) to make front-vs-back placement a uniform parameter.
- **Parameters/Gates:** n/a
- **Header/tag:** none

### `InterpacketGapInserter` (simple) — `common/InterpacketGapInserter.ned/.h/.cc`
- **Role/base:** NED `extends PacketProcessorBase`, `@class(InterpacketGapInserter)`. **Mismatch:** actual C++ base is `ClockUserModuleMixin<PacketPusherBase>`, not `PacketProcessorBase` directly.
- **Does:** Enforces a minimum time gap between consecutive packets pushed through it (e.g. Ethernet IFG). Tracks `packetEndTime` of the last packet and delays the next push until `packetEndTime + duration` elapses, using a clock-aware `progress` `ClockEvent` when a delay is needed, otherwise passing through immediately. Also supports streaming pushes (`pushPacketStart/Progress/End` via `pushOrSendOrSchedulePacketProgress`), tagging deferred progress packets with an (external) `ProgressTag` so they resume correctly after the gap. `resolveDirective()` adds display-string directives `%g` (configured gap) and `%d` (last computed delay).
- **Parameters:** `clockModule: string = ""` — optional `IClock` path; `initialChannelBusy: bool = false` — assume the channel was busy before t=0; `duration: volatile double @unit(s)` (no default) — the gap length.
- **Gates:** input `in`; output `out`
- **Header/tag:** none (uses external `ProgressTag_m.h`)

### `OmittedProtocolLayer` (compound, empty passthrough) — `common/OmittedProtocolLayer.ned`
- **Role/base:** `module OmittedProtocolLayer extends Module like IProtocolLayer`, `@class(::inet::OmittedModule)` — a generic self-removing placeholder.
- **Does:** Wires `upperLayerIn --> lowerLayerOut` and `lowerLayerIn --> upperLayerOut` (pure passthrough splice); `OmittedModule` then deletes the module from the hierarchy during init. Used as the `@omittedTypename` default so an `IProtocolLayer` slot can be "turned off" by configuration.
- **Parameters:** none of its own (class fixed to `OmittedModule`)
- **Gates:** the 4-gate `IProtocolLayer` set (`upperLayerIn/Out`, `lowerLayerIn/Out`)
- **Header/tag:** none

### `PacketDeserializer` (simple) — `common/PacketDeserializer.ned/.h/.cc`
- **Role/base:** NED `extends PacketFlowBase`. C++ base matches (`PacketFlowBase`).
- **Does:** `processPacket()` reads the `PacketProtocolTag` (if any), runs a `PacketDissector` (backed by the global `ProtocolDissectorRegistry`) over the packet's raw bytes to rebuild a structured chunk tree, erases the packet's content and reinserts the dissected chunk — turns a byte-blob back into typed header/data chunks (inverse of `PacketSerializer`).
- **Parameters:** none of its own
- **Gates:** inherited `in`/`out` (from `PacketFlowBase`)
- **Header/tag:** none

### `PacketDestreamer` (simple) — `common/PacketDestreamer.ned/.h/.cc`
- **Role/base:** NED `extends PacketProcessorBase like IPacketFlow`. C++ base matches directly (`PacketProcessorBase`, `virtual IPacketFlow`) — no mixin.
- **Does:** Converts an inbound *stream* (push `Start`/`Progress`/`End` on `in`, which `supportsPacketStreaming`) into whole-packet delivery on `out` (`supportsPacketPassing` only on `out`): buffers the latest `streamedPacket` at each stage and only pushes/returns the finished packet at `pushPacketEnd` (or, pull-side, `pullPacket()` calls `provider.pullPacketStart()`+`pullPacketEnd()` back-to-back). Relays push/pull can-push/processed notifications between producer/provider and consumer/collector refs.
- **Parameters:** `datarate: double @unit(bps) = nan bps` — nominal rate recorded for the reconstructed packet's synthetic streaming calls.
- **Gates:** input `in @labels(send,push,pull,stream)`; output `out @labels(send,push,pull,pass)`
- **Header/tag:** none

### `PacketEmitter` (simple) — `common/PacketEmitter.ned/.h/.cc`
- **Role/base:** NED `extends PacketFlowBase like IPacketFlow`. C++ base matches.
- **Does:** Transparent pass-through `PacketFlow` that additionally emits a configurable signal (`signalName`, registered in `initialize()`) carrying a duplicate of each matching packet, for statistics/observation taps. Matching uses a `PacketFilter` built from the `packetFilter` NED object expression. If `direction` is set (`inbound`/`outbound`), it also stamps/validates a `DirectionTag` on every passing packet, throwing if it conflicts with an already-set tag.
- **Parameters:** `packetFilter: object = "*"` — filter expression selecting which packets trigger the signal; `direction: string @enum(undefined,inbound,outbound) = "undefined"`; `signalName: string` (no default, required) — name of the signal registered/emitted (matches the `.ned`-declared `packetSentToLower`/`packetReceivedFromLower` signal types used by callers).
- **Gates:** inherited `in`/`out` (from `PacketFlowBase`)
- **Header/tag:** none

### `PacketSerializer` (simple) — `common/PacketSerializer.ned/.h/.cc`
- **Role/base:** NED `extends PacketFlowBase`. C++ base matches.
- **Does:** `processPacket()` flattens the chunk tree into raw bytes (`peekAllAsBytes`), erases the structured content, reinserts it as a single byte blob — simulates on-wire serialization (inverse of `PacketDeserializer`).
- **Parameters:** none of its own
- **Gates:** inherited `in`/`out`
- **Header/tag:** none

### `PacketStreamer` (simple) — `common/PacketStreamer.ned/.h/.cc`
- **Role/base:** NED `extends PacketProcessorBase like IPacketFlow`, `@class(PacketStreamer)`. **Mismatch:** actual C++ base is `ClockUserModuleMixin<PacketProcessorBase>`, not `PacketProcessorBase` directly (adds clock support for `clockModule`).
- **Does:** Inverse of `PacketDestreamer` — accepts a whole packet on `in` (`supportsPacketPassing` on `in`) and streams it out (`supportsPacketStreaming` on `out`) using `datarate` to schedule an `endStreamingTimer` marking when the streamed packet's transmission time elapses (or ending immediately if `datarate` is NaN). Also implements the pull-side streaming (`pullPacketStart/Progress/End`) so it can sit between a passive packet source and an active streaming sink.
- **Parameters:** `clockModule: string = ""`; `datarate: double @unit(bps) = nan bps` — rate used to time the stream.
- **Gates:** input `in @labels(send,push,pull,pass)`; output `out @labels(send,push,pull,stream)`
- **Header/tag:** none

### `PaddingInserter` (simple) — `common/PaddingInserter.ned/.h/.cc`
- **Role/base:** NED `extends PacketFlowBase like IPacketFlow`. C++ base matches.
- **Does:** `processPacket()` computes bits needed to round the packet's current data length up to `roundingLength`, and/or up to `minLength` if larger; if padding is needed, inserts a `ByteCountChunk` (byte-aligned) or `BitCountChunk` (sub-byte) at `front`/`back` via the shared `HeaderPosition::insertHeader<T>()` helper.
- **Parameters:** `minLength: int @unit(b)` (no default, required) — minimum resulting length; `roundingLength: int @unit(b) = 1B` — length to round up to; `insertionPosition: string @enum(front,back) = "back"`.
- **Gates:** inherited `in`/`out`
- **Header/tag:** none (padding chunks are generic `ByteCountChunk`/`BitCountChunk` from `inet::common`, no `.msg` here)

### `ProtocolChecker` (simple) — `common/ProtocolChecker.ned/.h/.cc`
- **Role/base:** NED `extends PacketFilterBase`, `@class(ProtocolChecker)`. C++ base matches: `PacketFilterBase`, `DefaultProtocolRegistrationListener`.
- **Does:** Filters/gatekeeps: only lets through packets whose `PacketProtocolTag` names a protocol actually *registered* as an indication-service (`SP_INDICATION`) consumer on this module's output gate (tracked via `handleRegisterProtocol` populating a `protocols` set). `matchesPacket()` checks tag-protocol membership; unmatched/untagged packets are dropped via `dropPacket()` with reason `NO_PROTOCOL_FOUND` — catches "nobody registered to receive this protocol" misconfigurations at runtime.
- **Parameters:** none of its own
- **Gates:** inherited `in`/`out` (from `PacketFilterBase`)
- **Header/tag:** none

---

## lifetime

Packet-expiry/lifetime watchdogs — not queueing-pipeline flow elements, but signal-driven collection watchers implementing `IPacketLifeTimer`.

### `CarrierBasedLifeTimer` (simple) — `lifetime/CarrierBasedLifeTimer.ned/.h/.cc`
- **Role/base:** NED `extends SimpleModule like IPacketLifeTimer` (`@class(CarrierBasedLifeTimer)` declared twice in the `.ned`). C++ base matches: `SimpleModule`, `cListener`.
- **Does:** Not a pipeline element — a lifecycle watchdog on a packet collection. In `initialize()` resolves the containing `NetworkInterface`, subscribes to its `interfaceStateChangedSignal`, and resolves `collectionModule` (an `IPacketCollection`, e.g. a queue) via `ModuleRefByPar`, subscribing to that module's `packetPushedSignal`. Whenever the NIC's carrier is lost (`F_CARRIER` field change to no-carrier) or a packet is pushed into the collection while there is no carrier, `clearCollection()` drains the entire collection, emitting `packetDroppedSignal` (reason `INTERFACE_DOWN`) for every discarded packet — implements "packets waiting on a dead link expire immediately."
- **Parameters:** `collectionModule: string` (no default, required) — relative path to the `IPacketCollection` (queue) to watch/drain.
- **Gates:** none (pure signal listener, no packet gates)
- **Header/tag:** none

---

## measurement

`IProtocolLayer`-shaped compound layers that tap the upper/lower-layer data flow to start and record flow measurements (statistics), plus the empty-passthrough `Omitted*` variant.

### `IMeasurementLayer` (moduleinterface) — `measurement/IMeasurementLayer.ned`
- **Role/base:** interface
- **Does:** Contract for compound "layer" modules performing flow measurement (start/record) on passing packets. `@omittedTypename(OmittedMeasurementLayer)` lets a `like IMeasurementLayer` slot default to the no-op passthrough.
- **Parameters:** display icon only
- **Gates:** the 4-gate `IProtocolLayer`-shaped set: `upperLayerIn`, `upperLayerOut`, `lowerLayerIn`, `lowerLayerOut`
- **Header/tag:** none

### `MeasurementLayer` (compound) — `measurement/MeasurementLayer.ned`
- **Role/base:** compound, `like IMeasurementLayer`
- **Does:** Two submodules: **submodules** — `measurementStarter` (default type `FlowMeasurementStarter`, an `IPacketFlow`), `measurementRecorder` (default `FlowMeasurementRecorder`, an `IPacketFlow`). **wiring** — `upperLayerIn → measurementStarter.in`, `measurementStarter.out → lowerLayerOut` (downstream packets get a measurement "started" mark); `lowerLayerIn → measurementRecorder.in`, `measurementRecorder.out → upperLayerOut` (returning packets get the measurement recorded/finalized).
- **Parameters:** none beyond submodule type defaults
- **Gates:** the 4-gate set
- **Header/tag:** none

### `MultiMeasurementLayer` (compound) — `measurement/MultiMeasurementLayer.ned`
- **Role/base:** compound, `like IMeasurementLayer`
- **Does:** **submodules** — `measurementStarter[numMeasurementModules]`, `measurementMaker[numMeasurementModules]` (both `IPacketFlow`, default `FlowMeasurementStarter`/`FlowMeasurementRecorder`). **wiring** — starters chained in order from `upperLayerIn` down to `lowerLayerOut` (`measurementStarter[0]` ← `upperLayerIn`; `measurementStarter[i] → measurementStarter[i+1]`; last one → `lowerLayerOut`); makers chained in reverse from `lowerLayerIn` back up to `upperLayerOut` (`measurementMaker[numMeasurementModules-1]` ← `lowerLayerIn`; `measurementMaker[i+1] → measurementMaker[i]`; `measurementMaker[0] → upperLayerOut`). Allows stacking multiple independent measurement points on one interface.
- **Parameters:** `numMeasurementModules: int = 1`
- **Gates:** the 4-gate set
- **Header/tag:** none

### `OmittedMeasurementLayer` (compound, empty passthrough) — `measurement/OmittedMeasurementLayer.ned`
- **Role/base:** `extends Module like IMeasurementLayer`, `@class(::inet::OmittedModule)`
- **Does:** Same self-removing passthrough pattern as `OmittedProtocolLayer`: `upperLayerIn → lowerLayerOut`, `lowerLayerIn → upperLayerOut`, then deletes itself during init.
- **Parameters:** none
- **Gates:** the 4-gate set
- **Header/tag:** none

---

## processing

`IProtocolLayer`-shaped compound layer modeling device packet-processing delay, plus its empty-passthrough variant.

### `IProcessingDelayLayer` (moduleinterface) — `processing/IProcessingDelayLayer.ned`
- **Role/base:** interface
- **Does:** Contract for a compound layer modeling packet-processing time in both ingress and egress directions. `@omittedTypename(OmittedProcessingDelayLayer)`.
- **Parameters:** display icon only
- **Gates:** the 4-gate set
- **Header/tag:** none

### `ProcessingDelayLayer` (compound) — `processing/ProcessingDelayLayer.ned`
- **Role/base:** compound, `like IProcessingDelayLayer`
- **Does:** **submodules** — `ingress`, `egress` (both `IPacketDelayer`, default `PacketDelayer`). **wiring** — `upperLayerIn → egress.in`, `egress.out → lowerLayerOut` (the `egress` submodule handles the upper→lower/outgoing path); `lowerLayerIn → ingress.in`, `ingress.out → upperLayerOut` (the `ingress` submodule handles the lower→upper/incoming path). Delay amount itself is delegated to whichever `IPacketDelayer` implementation is plugged in (outside this catalog's scope).
- **Parameters:** none beyond submodule defaults
- **Gates:** the 4-gate set
- **Header/tag:** none

### `OmittedProcessingDelayLayer` (compound, empty passthrough) — `processing/OmittedProcessingDelayLayer.ned`
- **Role/base:** `extends Module like IProcessingDelayLayer`, `@class(::inet::OmittedModule)`
- **Does:** Same self-removing passthrough pattern.
- **Parameters:** none
- **Gates:** the 4-gate set
- **Header/tag:** none

---

## socket

`IProtocolLayer`-shaped socket-layer interface plus its empty-passthrough variant. No concrete (non-omitted) implementation exists in this directory.

### `ISocketLayer` (moduleinterface) — `socket/ISocketLayer.ned`
- **Role/base:** interface
- **Does:** Contract for a compound "socket layer" module. Notably lacks an `@omittedTypename(...)` annotation — the only one of the four layer interfaces in this catalog (`IMeasurementLayer`/`IProcessingDelayLayer`/`ITrafficConditionerLayer` all declare one) without an implicit omitted default wired into the interface itself.
- **Parameters:** display icon only
- **Gates:** the 4-gate set
- **Header/tag:** none

### `OmittedSocketLayer` (compound, empty passthrough) — `socket/OmittedSocketLayer.ned`
- **Role/base:** `extends Module like ISocketLayer`, `@class(::inet::OmittedModule)`
- **Does:** Same self-removing passthrough pattern (`upperLayerIn → lowerLayerOut`, `lowerLayerIn → upperLayerOut`).
- **Parameters:** none
- **Gates:** the 4-gate set
- **Header/tag:** none

---

## trafficconditioner

`IProtocolLayer`-shaped layer that plugs in ingress/egress `ITrafficConditioner` implementations (shaping/policing/dropping/reordering), plus its empty-passthrough variant.

### `ITrafficConditionerLayer` (moduleinterface) — `trafficconditioner/ITrafficConditionerLayer.ned`
- **Role/base:** interface
- **Does:** Contract for a compound traffic-conditioning layer that can shape, police, and condition traffic (dropping/delaying/reordering) in both ingress and egress directions. `@omittedTypename(OmittedTrafficConditionerLayer)`.
- **Parameters:** display icon only
- **Gates:** the 4-gate set
- **Header/tag:** none

### `OmittedTrafficConditionerLayer` (compound, empty passthrough) — `trafficconditioner/OmittedTrafficConditionerLayer.ned`
- **Role/base:** `extends Module like ITrafficConditionerLayer`, `@class(::inet::OmittedModule)`
- **Does:** Same self-removing passthrough pattern.
- **Parameters:** none
- **Gates:** the 4-gate set
- **Header/tag:** none

### `TrafficConditionerLayer` (compound) — `trafficconditioner/TrafficConditionerLayer.ned`
- **Role/base:** compound, `like ITrafficConditionerLayer`
- **Does:** **submodules** — `egressConditioner`, `ingressConditioner` (both `like ITrafficConditioner`, `<default("")>` — no default type, a concrete `ITrafficConditioner` must be supplied by the user). **wiring** — `upperLayerIn → ingressConditioner.in`, `ingressConditioner.out → lowerLayerOut` (i.e. the submodule literally named `ingressConditioner` conditions the upper→lower/outbound path); `lowerLayerIn → egressConditioner.in`, `egressConditioner.out → upperLayerOut` (the submodule literally named `egressConditioner` conditions the lower→upper/inbound path) — the submodule names are the reverse of the directional role implied by the wiring, reported here verbatim as written. Also sets `*.interfaceTableModule = default(absPath(this.interfaceTableModule))`, forwarding the interface-table reference into whichever conditioner is plugged in.
- **Parameters:** `interfaceTableModule: string` (no default, required) — path to the interface table, forwarded to submodules.
- **Gates:** the 4-gate set
- **Header/tag:** none

---

## service

Compound modules combining atomic `protocolelement` pieces (from sibling packages: `aggregation`, `fragmentation`, `ordering`, `forwarding`, `acknowledgement`, `checksum`, `dispatching`, `selectivity`, plus this catalog's `transceiver.PacketTransmitter`) into reusable per-hop / per-interface service layers. None of these modules declares a `like <interface>` clause (they are plain compounds, not typed to `IProtocolLayer` or any of the `contract/` interfaces), even though most share the 4-gate `upperLayerIn/Out`+`lowerLayerIn/Out` shape.

### `DataService` (compound) — `service/DataService.ned`
- **Role/base:** compound module (no `like` interface)
- **Does:** Full-duplex per-hop data pipeline. **submodules** — `aggregator` (`SubpacketLengthHeaderBasedAggregator`), `fragmenter` (`FragmentNumberHeaderBasedDefragmenter`), `sequenceNumbering` (`SequenceNumbering`), `queue` (`IPacketQueue`, default `DropTailQueue`), `server` (`IPacketServer`, default `PacketServer`), `deaggregator` (`SubpacketLengthHeaderBasedDeaggregator`), `defragmenter` (`FragmentNumberHeaderBasedDefragmenter`), `reordering` (`Reordering`). **wiring, outbound** — `upperLayerIn → aggregator.in → fragmenter.in → sequenceNumbering.in → queue.in → server.in → lowerLayerOut`. **wiring, inbound** — `lowerLayerIn → reordering.in → defragmenter.in → deaggregator.in → upperLayerOut`. Note: as literally written in the `.ned`, both the outbound submodule named `fragmenter` and the inbound submodule named `defragmenter` are instantiated with the same type, `FragmentNumberHeaderBasedDefragmenter`.
- **Parameters:** none of its own (all on submodules)
- **Gates:** input `upperLayerIn`; output `upperLayerOut`; input `lowerLayerIn`; output `lowerLayerOut`
- **Header/tag:** none

### `ForwardingService` (compound) — `service/ForwardingService.ned`
- **Role/base:** compound module (no `like` interface); `upperLayerIn`/`upperLayerOut` are `@loose`
- **Does:** Hop-limit-based forwarding built around an internal `MessageDispatcher` (`d1`) that routes traffic by dispatch tag. **submodules** — `d1` (`MessageDispatcher`), `forwarding` (`Forwarding`), `sendWithHopLimit` (`SendWithHopLimit`), `receiveWithHopLimit` (`ReceiveWithHopLimit`). **wiring** — `upperLayerIn → d1.in++`; `d1.out++ → forwarding.in`, `forwarding.out → d1.in++` (forwarding decisions loop back through the dispatcher); `d1.out++ → sendWithHopLimit.in → lowerLayerOut` (locally-originated traffic gets hop-limit stamped and sent down); `lowerLayerIn → receiveWithHopLimit.in → d1.in++`, `d1.out++ → upperLayerOut` (received traffic has its hop limit checked, then is dispatched either back into `forwarding` or up to `upperLayerOut` depending on tag).
- **Parameters:** none of its own
- **Gates:** input `upperLayerIn @loose`; output `upperLayerOut @loose`; input `lowerLayerIn`; output `lowerLayerOut`
- **Header/tag:** none

### `InterfaceService` (compound) — `service/InterfaceService.ned`
- **Role/base:** NED `extends NetworkInterface`, `@class(inet::NetworkInterface)` — it *is* a `NetworkInterface` module (not merely `like` one), reusing the plain `NetworkInterface` container C++ class (no bespoke logic of its own beyond the composition).
- **Does:** A complete MAC-level network interface assembled from `protocolelement` pieces plus one `inet.queueing` multiplexer. **submodules** — `sendToMacAddress`, `resending` (`Resending`, ARQ retransmission bookkeeping), `sendWithAcknowledge`, `m1` (`PacketMultiplexer`), `sendWithProtocol`, `crcInserter` (`EthernetFcsHeaderInserter`), `transmitter` (`transceiver.PacketTransmitter`, cataloged above), `receiveAtMacAddress`, `receiveWithAcknowledge`, `d1` (`MessageDispatcher`), `receiveWithProtocol`, `crcChecker` (`EthernetFcsHeaderChecker`). **wiring, outbound** — `upperLayerIn → sendToMacAddress.in → resending.in → sendWithAcknowledge.in → m1.in++ → sendWithProtocol.in → crcInserter.in → transmitter.in → phys$o`. **wiring, inbound** — `phys$i → crcChecker.in → receiveWithProtocol.in → d1.in++ → receiveWithAcknowledge.in → receiveAtMacAddress.in → upperLayerOut`; additionally `receiveWithAcknowledge.ackOut → m1.in++` (generated ACKs are injected back into the outbound multiplexed path) and `d1.out++ → sendWithAcknowledge.ackIn` (received ACKs are delivered to the sender-side ARQ logic). This is the one module in `service/` that directly instantiates a `transceiver/` module.
- **Parameters:** `interfaceTableModule: string` (no default, required); `protocol: string` (no default, required)
- **Gates:** input `upperLayerIn @loose`; output `upperLayerOut @loose`; inout `phys`
- **Header/tag:** none

### `PeerService` (compound, helper — defined alongside `MacService`) — `service/MacService.ned`
- **Role/base:** compound module (no `like` interface)
- **Does:** **submodules** — `multiplexer` (`PacketMultiplexer`), `defragmenter[numDefragmenter]` (`FragmentNumberHeaderBasedDefragmenter`, `deleteSelf = true` — dynamically created/self-deleting instances), `classifier` (`DynamicClassifier`). **wiring** — `in → classifier.in`; `classifier.out++ → multiplexer.in++` for each of `numDefragmenter` branches; `multiplexer.out → out`. Represents per-peer (e.g. per neighbor) reassembly state that can be spun up/torn down dynamically.
- **Parameters:** `numDefragmenter: int = 0`; `defragmenter[*].deleteSelf = true` (submodule parameter assignment)
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `MacService` (compound) — `service/MacService.ned`
- **Role/base:** compound module (no `like` interface)
- **Does:** Same classifier→fan-out→multiplexer pattern as `PeerService`, one level up. **submodules** — `multiplexer` (`PacketMultiplexer`), `peer[numPeers]` (`PeerService`, above), `classifier` (`DynamicClassifier`). **wiring** — `in → classifier.in`; `classifier.out++ → multiplexer.in++` for each of `numPeers` peer instances; `multiplexer.out → out`. Models a MAC layer maintaining independent per-peer processing/reassembly pipelines multiplexed onto one shared `in`/`out` pair.
- **Parameters:** `numPeers: int = 0`
- **Gates:** input `in`; output `out`
- **Header/tag:** none

### `SelectivityService` (compound) — `service/SelectivityService.ned`
- **Role/base:** compound module (no `like` interface); `connections allowunconnected` (permits submodule gates to remain unwired)
- **Does:** Address/port-based selective-delivery filter chain. **submodules** — `sendToPort` (`SendToPort`), `sendToL3Address` (`SendToL3Address`), `receiveAtPort` (`ReceiveAtPort`), `receiveAtL3Address` (`ReceiveAtL3Address`). **wiring, outbound** — `upperLayerIn → sendToPort.in → sendToL3Address.in → lowerLayerOut`. **wiring, inbound** — `lowerLayerIn → receiveAtL3Address.in → receiveAtPort.in → upperLayerOut`. Stamps outbound packets with destination L3-address/port selectors and filters inbound packets to those addressed to this L3 address/port, dropping the rest — socket-style address/port demultiplexing at the protocol-element level.
- **Parameters:** none of its own
- **Gates:** input `upperLayerIn`; output `upperLayerOut`; input `lowerLayerIn`; output `lowerLayerOut`
- **Header/tag:** none

---

## contract

The `protocolelement`-level moduleinterfaces: `IProtocolLayer` (the generic bidirectional-layer contract that `measurement`/`processing`/`trafficconditioner` conceptually mirror, though none of them formally `extends` it in NED — each redeclares the identical 4-gate set independently), `IProtocolHeaderInserter`/`IProtocolHeaderChecker` (queueing-flow/-filter contracts for header add/verify modules implemented in sibling `protocolelement` subpackages), and `IPacketLifeTimer` (the contract implemented by `lifetime/CarrierBasedLifeTimer`).

### `IProtocolLayer` (moduleinterface) — `contract/IProtocolLayer.ned`
- **Role/base:** interface
- **Does:** The canonical "optional bidirectional layer" contract: a module connects to an optional higher and an optional lower protocol layer. `@omittedTypename(OmittedProtocolLayer)` supplies the generic omit-default for any `like IProtocolLayer` slot.
- **Parameters:** `@omittedTypename(OmittedProtocolLayer)`; display icon
- **Gates:** the 4-gate IProtocolLayer set: input `upperLayerIn`; output `upperLayerOut`; input `lowerLayerIn`; output `lowerLayerOut`
- **Header/tag:** none

### `IProtocolHeaderChecker` (moduleinterface) — `contract/IProtocolHeaderChecker.ned`
- **Role/base:** interface, `extends IPacketFilter` (`inet.queueing.contract`)
- **Does:** Contract for modules that verify a protocol-specific header and typically remove it after successful validation, acting as a packet filter that can drop invalid packets based on header information. No concrete implementation within this catalog's directory set (implementations — e.g. `EthernetFcsHeaderChecker`, used by `service/InterfaceService` — live in sibling `protocolelement` subpackages).
- **Parameters:** display icon only (plus whatever `IPacketFilter` declares)
- **Gates:** inherited from `IPacketFilter` (packet-filter in/out shape)
- **Header/tag:** none

### `IProtocolHeaderInserter` (moduleinterface) — `contract/IProtocolHeaderInserter.ned`
- **Role/base:** interface, `extends IPacketFlow` (`inet.queueing.contract`)
- **Does:** Contract for modules that add protocol-specific headers to packets before transmission (addressing, control data, authentication codes), acting as an `IPacketFlow`. No concrete implementation within this catalog's directory set (e.g. `EthernetFcsHeaderInserter`, used by `service/InterfaceService`, is the sibling-package implementation).
- **Parameters:** display icon only
- **Gates:** inherited from `IPacketFlow` (flow in/out shape)
- **Header/tag:** none

### `IPacketLifeTimer` (moduleinterface) — `contract/IPacketLifeTimer.ned`
- **Role/base:** interface
- **Does:** Marks modules (e.g. `lifetime/CarrierBasedLifeTimer`) that implement a packet-expiry/lifetime policy attached to a packet collection (queue). Purely a display/typing marker — declares no gates, consistent with `CarrierBasedLifeTimer` being a pure signal-driven watchdog rather than a pipeline element.
- **Parameters:** display icon only
- **Gates:** none
- **Header/tag:** none

