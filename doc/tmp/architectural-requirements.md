# INET Framework — Architectural Requirements

Requirements from the perspective of INET developers and model contributors: the
design rules the code base must follow so that models remain composable,
extensible, and maintainable.

Each requirement has a stable identifier of the form `AR-<AREA>-<NAME>` followed by a
one-line statement and one or two paragraphs of rationale. These architectural
requirements complement the user-facing requirements in
[requirements.md](requirements.md): the latter say *what INET must do for a simulation
user*, while these say *how the code base must be structured* to make that possible and
to keep it sustainable. They are distilled from the INET Developer's Guide and from
recurring lessons in day-to-day INET/OMNeT++ development.

## Code Organization (AR-ORG)

### AR-ORG-DOMAINS — Layered, domain-partitioned source tree with acyclic dependencies
Source code is organized by protocol layer and functional domain; cross-cutting
infrastructure lives in a shared `common` package, and dependencies point strictly from
protocols toward infrastructure, never the reverse.

The tree under `src/inet` mirrors the OSI-style layering — `physicallayer`, `linklayer`,
`networklayer`, `transportlayer`, `routing`, `applications` — plus orthogonal domains such
as `mobility`, `power`, `clock`, and `node`. A developer looking for a protocol can find it
from its layer and name alone (AODV under `routing/aodv`, TCP under `transportlayer/tcp`),
and a NED package name always matches its directory. This predictable partitioning is what
lets contributors add a protocol without reading the whole framework, and it is only
sustainable if the dependency direction is disciplined: shared abstractions (addresses,
packet chunks, tags, the queueing contracts) sit in `common` and know nothing about the
protocols that use them, so that an individual protocol can be built, understood, and
reasoned about without pulling in unrelated layers.

### AR-ORG-CONTRACTS — Every extensible role is a separate contract (C++ + NED interface)
Every role that is meant to be substitutable is defined first as a contract — a paired C++
interface and NED `moduleinterface` — kept separate from the reusable base implementations
and from the concrete models that realize them.

INET's flexibility rests on `like`-typed submodules, and the thing after `like` is always an
interface such as `IApp`, `IMacProtocol`, `IRadio`, `IMobility`, `IClock`, or `IPacketQueue`,
held in a `contract` subpackage. The interface fixes the gate/parameter shape and the C++ API
that all implementations must honor, while `*Base` modules and classes provide shared
machinery that concrete models opt into by extension. Keeping the three concerns — contract,
base, concrete — in distinct artifacts means an alternative implementation can be dropped in
with confidence that it satisfies the same contract, and that the contract can be documented
and reasoned about independently of any one implementation (`IInterfaceTable`, for example,
lets the interface table be replaced without recompiling the modules that use it).

### AR-ORG-VIS-SPLIT — Model logic, visualization, and instrumentation live in separate packages
Model logic, visualization, and infrastructure are separate packages; protocol code contains
no visualization or instrumentation logic.

A protocol module computes protocol behavior and emits signals; it never draws on the canvas,
never formats a figure, and never records a statistic itself. Visualizers are independent
modules that subscribe to the model from the outside, so that turning visualization on or off,
or swapping a 2D canvas visualizer for a 3D OSG one, cannot change a single simulated bit. This
separation keeps the protocol code small and focused, lets visualization evolve on its own
schedule, and guarantees the neutrality property that users depend on (see AR-OBS-SIGNALS):
observation is always additive and side-effect free.

### AR-ORG-KERNEL — Build on the OMNeT++ kernel; do not reimplement or patch its facilities
INET is a model library layered on the OMNeT++ simulation kernel and respects that boundary in
one direction: general discrete-event capabilities belong in the kernel and INET consumes them,
rather than growing private reimplementations or in-place patches of kernel internals.

Event scheduling and lifecycle hooks, coroutine suspend/resume, conditional breakpoints and
run-until control, RNG streams, and the signal mechanism are all facilities of general use to
any simulation model, not INET-specific concerns. When INET needs such a capability that the
kernel lacks, the architecturally correct response is to generalize it *into* the kernel — so
every model benefits and the kernel is not shaped around one library's specifics — not to bolt
a one-off mechanism onto INET or hand-patch kernel source. Keeping this boundary clean avoids a
whole class of bugs where an INET-local reimplementation silently drifts from kernel semantics,
and it keeps INET's own code focused on network modeling.

## Module Design (AR-MOD)

### AR-MOD-COMPOSITION — Build behavior from small single-purpose modules, structure from compound modules
Functionality is built by composing small, single-purpose modules: behavior lives in simple
modules, structure lives in compound modules, and composition is preferred over inheritance.

INET treats the module graph, not the class hierarchy, as the primary unit of reuse. A simple
module does one thing in C++ (`handleMessage`/`activity`); a compound module wires such pieces
into a larger capability without adding behavior of its own. Complex devices and protocol stacks
are therefore assembled from many cooperating small parts rather than from a few large
multi-purpose classes — a radio decomposes into antenna, transmitter, receiver, and energy
consumer; a network interface into queue, classifier, MAC, and PHY. This keeps each part
independently testable and replaceable, and it means new behavior is usually obtained by
re-wiring or substituting a submodule rather than by editing or subclassing an existing one.

### AR-MOD-PLUGGABLE — Submodules are interface-typed with replaceable defaults and can be omitted
Submodules are declared against module interfaces with replaceable default types, and optional
submodules can be omitted entirely through configuration.

The canonical NED idiom is `radio: <default("Ieee80211Radio")> like IRadio` — a slot typed by
contract, filled by a sensible default, overridable to any conforming type from the ini file
without touching NED or C++. Optionality is first-class too: submodules guarded by
`if typename != ""` (or realized by an `OmittedModule`) disappear from the model when
unconfigured, so a node carries a clock, an energy model, or a given protocol only when the
scenario asks for it. Substitutability and omittability together are what make the same node
type serve wired, wireless, and specialized scenarios by configuration alone.

### AR-MOD-FIDELITY — The same concern is offered at multiple, configuration-selectable levels of detail
A modeled concern is offered at several levels of fidelity — statistical vs. detailed, flat vs.
layered, scalar vs. multi-dimensional — selectable by configuration, so a study trades accuracy
against simulation performance without changing the surrounding model.

"Scalable level of detail" is an explicit, first-class design goal of INET, most visible in the
physical layer (a signal may be described in the packet, bit, symbol, sample, or analog domain,
and the analog domain ranges from a simple range-based model to a time- and frequency-varying
one) but present throughout: error representation (AR-PKT-ERRORS), the escalating statistics
tiers, and alternative protocol implementations behind one interface. The architectural
requirement is that fidelity be a *dimension the user chooses*, not a value hardcoded into the
model — a coarse model and a detailed model of the same thing present the same contract so they
are interchangeable, and no part of the framework forces worst-case fidelity on a study that
does not need it.

### AR-MOD-NODEBASE — Nodes assembled from shared per-layer bases; services found by lookup, not path
Network nodes are assembled incrementally from shared per-layer base modules, and node-scoped
services are located by lookup within the enclosing node rather than by hardcoded module paths.

Node types form a shallow extension chain — `NodeBase` (mobility, clock, energy) →
`ApplicationLayerNodeBase` (full stack) → `StandardHost` — so that a new node variant reuses the
common scaffolding instead of re-declaring it. Just as important, a module never assumes where
its collaborators sit: it finds the interface table, routing table, or clock through a
module-path parameter resolved with helpers like `absPath()` and `^`, defaulting relative to the
enclosing `@node`. Because lookup is relative and parameterized rather than absolute, the same
protocol module works unchanged whether the node has one interface or ten, and renaming or
restructuring a node does not silently break its internals.

## Packet Representation (AR-PKT)

### AR-PKT-CHUNKS — Packet content is typed, immutable, shared chunks manipulated as views
Packet content is represented as typed, immutable, shared chunks, and packet operations create
views rather than copying or destroying the underlying data.

A `Packet` carries a tree of `Chunk` objects (byte/bit counts, raw bytes, field-based headers,
slices, sequences); chunks are created via `makeShared` and shared among packets through
reference-counted pointers, so duplication, buffering, and encapsulation stay cheap. As a packet
moves through the layers, headers and trailers are *popped* by advancing independent front/back
offsets rather than by mutating stored data, so the full content remains available for logging,
PCAP export, or re-inspection. Immutability is enforced once a chunk is shared, so a packet
handed to several modules cannot be mutated out from under any of them. Modeling packet contents
as first-class typed objects — rather than opaque byte buffers or ad-hoc C++ structs — is the
foundation that the serializer, dissector, tagging, and generic buffer facilities all build on.

### AR-PKT-DUAL — Every header supports both field-based and raw-byte form via registered serializers
Every protocol header supports both a structured field-based representation and a raw-byte
representation, bridged by registered serializers, so models can interoperate with real network
data.

A header can exist as a convenient typed object during simulation and, on demand, be serialized
to the exact bytes that would appear on the wire (or reconstructed from captured bytes).
Serializers are separate classes from the headers themselves, registered in a
`ChunkSerializerRegistry`, so the packet API can convert between representations without the
protocol logic knowing how, and "how data is structured for debugging" stays independent of "how
it is encoded on the wire." This dual representation is what enables hardware emulation and PCAP
import/export, cross-checking against real implementations, and fingerprinting over actual packet
content (AR-QUAL-FINGERPRINT's `PACKET_DATA` ingredient), while still letting everyday model code
work with readable fields.

### AR-PKT-TAGS — Metadata travels in tags, never in wire content, and stops at the transmission boundary
Packet metadata travels in tags attached to packets or to byte regions, never encoded in wire
content, and tags do not cross the physical transmission boundary.

Protocols routinely need out-of-band information — which interface a packet came in on, what
transport protocol it belongs to, a requested transmission power, a socket identity, an
encapsulation or dispatch request — that is emphatically not part of the bytes on the wire. INET
carries this as typed `Tag`/`Req`/`Ind` objects on the packet (or on a region of it), with
directionality mirroring OSI service primitives: *requests* flow from higher layers to lower,
*indications* from lower to higher. Region tags attach to sub-ranges of the data and are
preserved through splitting, reordering, and recombination as if attached to each bit, with
equivalent adjacent regions merged automatically. Because tags model in-node state rather than
transmitted data, physical-layer protocols strip them at the point of transmission: what the
receiver gets is the wire content, from which it re-derives its own tags. This boundary keeps the
simulation honest about what information is actually available where.

### AR-PKT-ERRORS — Transmission errors are representable at multiple fidelity levels
Transmission errors are representable at several levels of fidelity, selectable per use case.

The same corrupted transmission can be modeled coarsely (a whole-packet error flag), at chunk
granularity (a good compromise), or by actually flipping bits in raw chunks (most accurate),
letting a study trade physical realism against simulation cost. A MAC that only needs to know
"was this frame received correctly?" pays nothing for bit-level fidelity, while a physical-layer
study that cares about capture, interference, and partial corruption can opt into it via
configurable receiver parameters. Making error fidelity a selectable dimension — rather than a
fixed assumption baked into the packet — is a specific instance of AR-MOD-FIDELITY applied to the
error model.

### AR-PKT-SIGNAL — Physical transmissions are modeled as immutable Signals distinct from packets
A physical transmission is represented by a `Signal` that encapsulates a `Packet` together with
an analog-domain description, and each stage of physical-layer processing produces an immutable
result.

The digital content a protocol manipulates (`Packet`) is architecturally distinct from the
physical phenomenon that carries it (`Signal` = packet + duration, power, and other analog
attributes, describable across the packet/bit/symbol/sample/analog domains). A single centralized
transmission-medium model owns all radios, transmissions, interference, and receptions — both
because reception depends on shared computations (path loss, interference across all concurrent
signals) that pairwise radio logic could not do consistently, and because centralization enables
optimistic parallel computation that can be invalidated and redone as the scenario changes. Every
step of the pipeline (transmitted signal → arrival → reception → interference → SNIR → reception
decision) yields a data structure that is not mutated afterward, so the physical facts of a
transmission remain auditable and cannot be corrupted by a later stage. The error decision itself
is a separate, optional, replaceable receiver component, since the literature offers many
competing error models and none should be architecturally privileged.

## Protocol Interaction (AR-COM)

### AR-COM-REGISTRY — Modules declare the protocols and services they provide in a global registry
At initialization, each module registers the protocols it handles and the services it provides for
them into a global registry, and message dispatch is driven by these registrations.

Rather than hardwiring "TCP output goes to gate 3," a module calls `registerProtocol` in its
`initialize` method to declare that it serves (or consumes) a given protocol, informing the
connected dispatchers of its presence; the dispatch machinery then consults these registrations to
route packets. Protocol identity is a first-class registered value, not a gate index or a magic
string buried in connection code, and the same protocol may register only once per dispatcher so
that dispatching is never ambiguous. This registration-driven approach decouples *what* a module
does for a protocol from *where* it sits in the node, and it is the mechanism that makes protocol
dispatch (AR-COM-DISPATCH) and no-core-change extensibility (AR-EXT-NOCORE) possible.

### AR-COM-DISPATCH — Address peers by protocol/service, not by wiring topology
Modules address their peers by protocol and service rather than by wiring topology, so the same
protocol module works unchanged under different node compositions.

A packet moving up or down the stack is steered by `MessageDispatcher` modules that read the
packet's protocol tags (`DispatchProtocolReq`, `EncapsulationProtocolReq`) and the service
registry, so a sender says "deliver this to the IPv4 layer," not "send out gate 2 to the module I
happen to be wired to." Dispatchers require no manual configuration — they discover registered
protocols and peek into packets — and they let protocol components be connected in one-to-many and
many-to-many arrangements, so a simple host can use a linear stack while a complex node (e.g. a
TSN device) freely interconnects protocols outside strict layering. Because routing between layers
is by identity, a node can insert a shaper, filter, or extra protocol into the path without every
neighbor being rewired, and one protocol implementation serves many different node structures.

### AR-COM-SOCKETS — Applications use socket-style callback APIs, not raw message exchange
Applications interact with protocols through socket-style APIs with callback interfaces rather
than through raw OMNeT++ message exchange.

Writing an application against bare `send`/`handleMessage` to a transport protocol means
re-encoding that protocol's command/indication conventions by hand, which is error-prone and
duplicated across every app. INET instead provides `UdpSocket`, `TcpSocket`, and peers that expose
a familiar bind/connect/send/close API and deliver events through a callback (`ICallback`)
interface; all sockets implement common `ISocket`/`INetworkSocket` interfaces so generic
infrastructure (e.g. `SocketMap`) can manage them uniformly, and a node-unique socket id is
round-tripped through `SocketReq`/`SocketInd` tags so any in-flight packet can be correlated back
to its socket. Sockets must be explicitly closed before deletion, because closing is a protocol
operation that releases resources on the local node, the peer, or elsewhere — not a
memory-management detail. The socket is a thin, uniform façade over message passing; it adds no
protocol behavior but makes the common case of *using* a protocol far simpler and more consistent.

### AR-COM-DIRECT — Same-instant, same-node coordination uses direct C++ calls, not zero-time messages
Coordination between submodules of the same node that happens at the same simulation instant is
expressed as direct, typed C++ calls, not as messages sent with zero delay.

Message passing models communication that takes simulation time or crosses the physical medium;
using `send()` or `scheduleAt(simTime())` for control flow that occurs at the same instant between
sibling submodules is a category error that inflates the event count, obscures causality, and
forces even trivial passthroughs to carry message classes and dispatch code they do not need. For
same-instant intra-node coordination INET uses direct calls through typed references and the
service/socket and queueing APIs, with `Enter_Method` establishing the correct module context,
reserving genuine events for things that actually advance simulation time. This keeps the event
trajectory meaningful — each event corresponds to something that "happens" in the network — so
that fingerprints reflect real behavior rather than internal plumbing, and it avoids the
boilerplate of turning a direct call into a round-trip through the scheduler.

## Initialization & Lifecycle (AR-LIFE)

### AR-LIFE-STAGES — A single global multi-stage initialization order that models slot into
Cross-module initialization follows one globally defined stage sequence; each stage has a
documented contract, and new models slot into existing stages rather than inventing their own
ordering.

Bringing up a network node has ordering constraints — interfaces must exist before addresses are
assigned, addresses before routes, routes before applications start — that no single module can
enforce alone. INET uses OMNeT++'s multi-stage `initialize(stage)` with a framework-wide set of
named stages (local, link-layer, network-address, routing, application, …) so every module knows
which stage to do which part of its setup in; network interfaces, for instance, self-register into
the node's `InterfaceTable` in the link-layer stage. A new protocol does not invent a private
bring-up protocol; it declares what it needs in each existing stage. This shared contract is what
lets independently authored modules initialize correctly in one another's presence.

### AR-LIFE-OPERATIONS — Shutdown/restart/crash via a common lifecycle protocol, scriptable
Modules that support shutdown, restart, or crash implement the common lifecycle-operation protocol
and are controllable from scripted scenarios.

Studying failures, reboots, and energy-driven sleep requires that "turning a node off" be a
well-defined, staged operation rather than an ad-hoc hack — and it does not come for free: each
affected component must explicitly program what happens (TCP forgets its connections, the routing
table clears its routes, interfaces go down and silently discard traffic). Modules opt in with
`@lifecycleSupport` and handle the shared start/shutdown/crash operations, with a `NodeStatus`
submodule tracking up/down state for others to query and display; a graceful shutdown differs from
a crash in that a crash performs no orderly teardown. Because these operations are triggered
through the scenario/lifecycle machinery (and can be driven by the power model when a battery
depletes), transient and failure behavior can be scripted to occur at chosen simulation times,
which is essential for reliability and resilience studies.

## Composable Packet Processing (AR-QUEUE)

### AR-QUEUE-ROLES — Standard push/pull source/sink contracts let processing elements chain arbitrarily
Packet-processing elements implement standard push/pull source and sink contracts, so queues,
filters, schedulers, shapers, classifiers, and similar elements compose into arbitrary chains.

The `queueing` package defines a small set of interfaces (packet source, packet sink, and their
active/passive push/pull variants) that every processing element speaks. Because a `PacketQueue`,
a `PacketFilter`, a `PacketScheduler`, a token-bucket shaper, and a classifier all present the same
connectors, they can be strung together in any order to express a device's datapath — an Ethernet
egress pipeline, a DiffServ block, a TSN gate array — declaratively in NED. This turns what would
otherwise be bespoke per-protocol buffering code into a reusable algebra of interchangeable parts,
and it is the same abstraction that lets an application talk to a queue through a direct reference
(AR-COM-DIRECT) rather than through messages.

### AR-QUEUE-STREAMING — Processing contracts support progressive transfer (preemption, cut-through)
The processing contracts support progressive packet transfer, so preemption and cut-through
behavior are expressible.

Modern link layers do not always treat a frame as an indivisible unit: frame preemption interrupts
a low-priority frame mid-transmission, and cut-through forwarding begins sending a frame before it
has been fully received. The queueing/streaming contracts model a packet being handed over
progressively rather than atomically, so these behaviors fall out of the same composable framework
instead of requiring special-case code. Supporting partial/streamed transfer at the contract level
is what lets INET model TSN and cut-through switches within the ordinary datapath abstraction.

## Observability (AR-OBS)

### AR-OBS-SIGNALS — Models expose behavior via declared signals; recording/visualization only subscribe
Models expose observable behavior by emitting declared signals, and statistics recording and
visualization subscribe to those signals and never participate in protocol logic.

A module publishes what happens (packet sent, queue length changed, throughput updated) by emitting
`@signal`-declared signals; separate result-recording and visualization machinery listens. The
producer of an event is decoupled from every consumer of it, so adding a new statistic or a new
visualizer requires no change to the protocol, and — crucially — the act of observing is guaranteed
not to perturb the simulation. This one-way, subscribe-from-outside discipline is the mechanism
behind both AR-ORG-VIS-SPLIT and the user-facing neutrality guarantee.

### AR-OBS-NED-TRUTH — A module's external interface is defined in NED, the single source of truth
A module's external interface — parameters, gates, signals, and statistics — is declared in its NED
definition, which is the single source of truth, and other documentation must not duplicate it.

Parameters (with units and defaults), gates (with labels), emitted `@signal`s, and recorded
`@statistic`s are all formally part of the NED type, from which the INET Reference is generated. The
Developer's Guide and prose docs deliberately do not restate them, both to avoid drift and because
the NED declaration is the artifact tools actually consume (the IDE, the documentation generator,
the statistics configuration). Treating NED as authoritative means a module's contract with the
outside world is machine-readable, always current, and defined in exactly one place.

### AR-OBS-INTROSPECTION — Each protocol registers dissection, printing, and filtering support
Each protocol registers support for the common introspection tooling — packet dissection, printing,
and filtering — alongside its implementation.

Beyond its own logic, a protocol contributes a registered `*ProtocolDissector`, `*ProtocolPrinter`,
and serializer so that generic tools can take any packet apart *according to protocol logic rather
than current representation*, render it readably, and filter on its fields without knowing the
protocol in advance; an authoritative `PacketProtocolTag` on every packet anchors this analysis,
since raw bytes alone cannot reveal the protocol. This is why the packet inspector, the textual
packet representation, and expression-based packet filters (`nodeFilter`/`interfaceFilter`/
`packetFilter`) work uniformly across the whole framework. The requirement is that introspection
support ships *with* each protocol as a registered capability, not as a central switch statement
that every new protocol would have to be threaded into.

### AR-OBS-FLOWS — End-to-end measurement is enabled by region tags that survive packet transformation
Cross-module, end-to-end measurements are supported by attaching flow membership to packet data on a
per-bit basis via region tags that survive fragmentation, aggregation, reordering, and
re-encapsulation.

A single protocol module can only measure what it locally handles, so quantities like end-to-end
latency or total queueing time along a path cannot be computed by any one module. INET builds
per-flow tracking on the region-tag mechanism (AR-PKT-TAGS): data is classified into a named flow on
entry and out of it on exit, and because region tags are preserved through every transformation of
the data as if attached to each individual bit — with equivalent adjacent regions merged
automatically — the flow identity follows the actual bytes across modules and nodes. This is the
architectural mechanism that makes whole-network, multi-layer measurements possible without every
protocol having to cooperate explicitly.

## Configuration & Parameterization (AR-CFG)

### AR-CFG-INFER — Structure is inferred, not repeated; configuration stays DRY
Facts that can be derived from the model are derived rather than restated, and configuration is
defined once and propagated, so the same fact is never stated in two places that can drift apart.

The number of wired interfaces on a host is inferred from the number of links attached to it;
addresses and routes are computed by configurators; per-submodule parameters are set once at the
enclosing scope and propagated with parameter patterns and `absPath()`/`^` references rather than
repeated at each site. Configuration composition follows the same discipline — abstract base configs
with `extends`, sensible NED `default()`s — so a scenario states only what makes it different, and a
configurator that optimizes a derived structure (e.g. routing tables) must preserve the invariant
that any packet routed by the unoptimized structure is still routed identically. INET's scale
(thousands of parameters across a node graph) makes redundant configuration a real source of silent
inconsistency; inference and single-point definition are the defenses.

### AR-CFG-PARAMS — Parameters are typed, unit-annotated, single-meaning, and default-provided
A module parameter carries its type and (for physical quantities) its unit, provides a sensible
default, and has exactly one meaning; a user-supplied override and a system-computed value are
distinct, unambiguously named fields.

Physical quantities are always declared with `@unit(s)`, `@unit(bps)`, `@unit(m)`, etc., so values
are dimensionally checked and never bare numbers, and every parameter supplies a `default()` so that
minimal configurations still run. Crucially, one field never means two things: an
"override-if-set-else-compute" value must be split into the override and the resolved value, so that
"what value will actually be used here?" is always answerable, and an API parameter has one explicit
shape rather than a meaning inferred from its runtime content. These rules keep INET's very large
configuration surface legible and tool-checkable, and they eliminate the ambiguity that otherwise
turns configuration into guesswork.

## Extensibility (AR-EXT)

### AR-EXT-NOCORE — New protocols are added purely through existing contracts and registration points
New protocols are added purely through the existing contract and registration points; adding one
requires no modification of core code.

The combination of interface-typed slots (AR-MOD-PLUGGABLE), the protocol/service registry
(AR-COM-REGISTRY), registered serializers/dissectors (AR-OBS-INTROSPECTION), and attachable core
data (AR-EXT-ATTACH) means a new protocol is *registered into* the framework rather than *patched
into* it. A contributor implements the relevant interfaces, registers the protocol and its helpers,
and the existing dispatch, packet, and tooling machinery picks it up. The core never grows a
dependency on the new protocol, which keeps the framework open for extension while closed for
modification.

### AR-EXT-ATTACH — Shared core structures are extended by attaching protocol-specific data
Shared core data structures are extended by attaching protocol-specific data to them, so the core
never depends on optional protocols.

When a protocol needs to hang extra state on a shared object — a per-interface data block for IPv4
or IPv6, extra routing state, protocol-specific packet metadata — it attaches its own data to the
core structure instead of the core structure being edited to know about that protocol. The generic
container (`NetworkInterface`, for example) defines an attachment mechanism; the protocol defines and
owns the attached type, using composition because it cannot add data by subclassing a shared module.
This inverts the usual dependency: `common` structures stay ignorant of the layers above them, so an
optional protocol can be compiled out entirely without leaving dangling references in the core.

### AR-EXT-FEATURES — Optional functionality is partitioned into independently disableable features
Optional functionality is partitioned into independently disableable features with declared
dependencies, and code touching an optional subsystem is guarded so everything else builds without
it.

INET ships a feature model: coherent chunks of functionality (a protocol family, emulation, a
visualizer set) are declared as features with explicit inter-feature dependencies and can be switched
off, in which case their sources are excluded from the build. For this to hold, code in other
subsystems must not hard-depend on an optional feature's symbols; interaction goes through the
contracts and registries so that a disabled feature leaves the rest compilable and runnable. This is
what lets a user build exactly the subset they need and keeps the framework from collapsing into one
monolithic must-build-everything blob.

## Build & Project Structure (AR-BUILD)

### AR-BUILD-OUTOFTREE — Build output is relocatable and out-of-tree
Compiling INET and generating derived sources must be able to place all artifacts outside the source
tree, in a relocatable location keyed by build mode and configuration, so that multiple build
variants and version combinations coexist without clobbering one another or polluting the checkout.

Generated message sources (`_m.cc`/`_m.h`), object files, shared libraries, and executables should
land in a per-configuration output directory, not scattered through `src/`. A build must be a pure
function of a pinned source state plus a build configuration — reproducible into a fresh output
directory, depending on no mutable state in the tree and leaving none behind. This is the property
that makes debug and release builds, several OMNeT++/INET version pairs, concurrent git worktrees,
and parallel CI jobs tractable instead of a source of clobbering, stale-artifact, and
wrong-commit-tested bugs.

### AR-BUILD-DECLARATIVE — Build configuration is declarative with a single source of truth
The information needed to build INET is described declaratively in one authoritative, introspectable
place, not duplicated across several build mechanisms and never hardcoded to a particular machine.

Include paths, compiler and linker flags, enabled features, the OMNeT++ toolchain location, and the
output layout should be discoverable from a single source that IDEs, CI, and external build drivers
can query, rather than reverse-engineered from a hand-written Makefile, generated makefiles, and
feature fragments that all have to agree. Build scripts must not bake in machine-specific values such
as absolute home paths or `-march=native`. When several mechanisms encode the same facts, any of them
can be silently wrong; a single introspectable description lets any tool ask "how is this built?" and
get a trustworthy answer. Feature selection (AR-EXT-FEATURES) is one dimension of this configuration
and is expressed the same declarative way.

## Quality & Conventions (AR-QUAL)

### AR-QUAL-FINGERPRINT — Behavioral regressions are guarded by trajectory fingerprints; baselines change in a reviewable step
Behavioral regressions are guarded by simulation-trajectory fingerprints, and changes that
intentionally alter behavior update the recorded expectations in a separate, reviewable step.

A fingerprint is a hash over the simulation's event trajectory (module paths, timing, packet data,
per configurable "ingredients"), so any unintended change in behavior shows up as a broken
fingerprint test on CI and in pull requests. INET extends the standard OMNeT++ ingredients with
network-aware ones (a cross-node communication filter, node/interface path, packet data) so a
fingerprint can be scoped to exactly the behavior under test. When a change legitimately alters
behavior, the workflow is to regenerate and commit the updated baseline CSVs as a distinct, reviewable
patch — never to quietly weaken the test — so that "the fingerprint changed" is always a conscious,
auditable decision. This regime is only meaningful because model behavior is deterministic
(AR-QUAL-DETERMINISM).

### AR-QUAL-TESTS — Contributions ship with tests in the category matching their nature
Contributions ship with tests in the categories matching their nature — unit, module behavior,
statistical, validation — in addition to fingerprint coverage.

The `tests` tree is stratified by kind: C++ unit tests for algorithmic pieces, module/behavior tests
for protocol interactions, statistical tests for stochastic properties, and validation tests against
external references or standards. A change is expected to bring the kind of test that actually
exercises what it introduces, rather than leaning on fingerprints alone (which detect *that* behavior
changed, not *whether it is correct*). Matching test type to the nature of the code is what gives the
suite both regression safety and genuine correctness evidence.

### AR-QUAL-DETERMINISM — Model code is deterministic and exactly reproducible
A simulation produces identical results from identical inputs, run after run, regardless of memory
layout, container iteration order, allocation addresses, or thread scheduling.

Model code must not let observable behavior depend on nondeterministic artifacts: no iterating a
pointer-keyed hash container in a way that affects the trajectory, no tie-breaking events by an
allocation or thread-assigned counter, no reliance on undocumented enumeration order. All
stochasticity flows through the OMNeT++ RNG framework with explicit stream assignment, and any
ordering assumption that affects the outcome (topology enumeration, per-event draw order, tie-breaking
keys) is made explicit and structurally derived from causal/model state rather than left emergent.
Determinism is not a nicety — it is the precondition for the fingerprint regime (AR-QUAL-FINGERPRINT)
to mean anything and for results to be scientifically reproducible (the user-facing R-RUN-REPRO); a
single nondeterministic tie-break can turn a green suite intermittently red for reasons unrelated to
any real change, and it is the first thing that must hold for parallel execution to be sound.

### AR-QUAL-NAMING — Framework-wide naming conventions make a component's role legible from its name
Modules, packages, signals, and packet types follow the framework-wide naming conventions, so a
component's role is recognizable from its name alone.

INET encodes a large amount of semantics into names: singular lowercase package names; module
suffixes/prefixes like `I*` (interface), `*Base`, `*Layer`, `*Inserter`/`*Checker`, `*Table`,
`*Configurator`, `Ext*`, `Ieee*`; class suffixes like `*Chunk`, `*Serializer`, `*ProtocolDissector`;
packet names ending `*Header`/`*Frame`; tags ending `*Tag`/`*Req`/`*Ind`; and signals like
`*SentSignal`/`*ChangedSignal`. Following these conventions is a hard requirement, not a style
preference, because both humans and tooling infer a component's category and how to compose it from
the name, and structurally similar entities must not drift onto different conventions. An
off-convention name is effectively mislabeled and undermines discoverability across a framework this
large.

### AR-QUAL-DISPLAY — Visual conventions are consistent and complete: one distinguishing icon per category
INET modules carry `@display` icons that convey their role at a glance, and this visual vocabulary is
as disciplined as the naming vocabulary: semantically distinct categories get distinct icons, size
variants simplify the glyph rather than merely rescaling it, and no module is left without an icon.

When a single generic icon stands for a hundred unrelated module types it carries no information, and
a module with no icon at all is invisible to the visual reasoning that the graphical editor and network
diagrams are meant to support. Consistent, complete iconography is the visual counterpart of
AR-QUAL-NAMING — part of how a framework this large stays navigable to the humans composing simulations
from it — and it must be maintained as a first-class convention, not left to accrete ad hoc as modules
are added.

### AR-QUAL-LOGGING — Common log-level semantics; programming errors are exceptions, never just logged
Log output follows the common level semantics and formatting rules, and programming errors raise
exceptions rather than being merely logged.

INET uses OMNeT++'s logging levels with agreed, audience-based meanings: `info` treats the module as a
black box (public inputs, outputs, and decisions, for people who know the protocol's interface),
`detail` treats it as a white box (internal state and timers, for maintainers who know the
implementation), with `error`/`warn`/`debug`/`trace` around them — mirroring the public-interface vs.
internal-implementation split found elsewhere in the architecture. A given verbosity therefore yields
comparable, filterable output across modules, and log statements never carry simulation-affecting side
effects. Crucially, a *programming error* — a violated invariant, an impossible state, a misused API —
must throw (via `check_and_cast`, `ASSERT`, or an explicit exception) so it fails loudly and stops the
run, instead of being downgraded to a log line a user might never see. Logging communicates what the
model is doing; exceptions enforce what the code requires.

### AR-QUAL-TRACEABILITY — Test coverage maps to model structure, and baseline changes carry provenance
It is possible to determine which tests exercise a given part of the model, and each accepted change to
a test baseline is traceable to the code change that justified it.

Because INET's correctness is guarded largely by simulation tests, developers need to compute the blast
radius of a change without rerunning the entire matrix — through the fingerprint suite's descriptive
tags and a dependable mapping from source files and NED packages to the configs that instantiate them.
Test-baseline artifacts (fingerprint and statistical stores) should be structured so that long-lived
parallel development branches do not inherently conflict over them, and every baseline update should be
attributable to the specific change that caused it. Traceability turns the test suite from an opaque
pass/fail gate into a tool that tells developers what to run and explains why a baseline moved.

### AR-QUAL-ENFORCED — Quality rules are machine-checked, not just documented
Every architectural requirement that *can* be mechanically verified is backed by an automated check —
a compiler constraint, a test, or a lint/architecture rule that runs in CI — and the project's
standing goal is to push each requirement up the enforcement ladder (from review-only, toward
automated, toward can't-even-build) rather than leaving it as prose.

A rule that lives only in a document is advisory: a contributor — human or, increasingly, an AI agent —
optimizes for whatever gate actually blocks the merge, not for the paragraph. The architecture is
therefore responsible for making the right thing the path of least resistance and the wrong thing
*fail loudly*: an illegal dependency should not compile, a behavioral regression should break a
fingerprint, an off-convention name should trip the linter. And "review" is itself a ladder: rules
that no static check can express but that remain *judgeable* — visualization logic leaking into a
protocol, a zero-time message standing in for a procedure call, prose that duplicates a NED
declaration — are enforceable by an **agent reviewer** run as a CI gate, leaving only genuine design
judgment (is a fidelity level worth adding?) to a human. The enforcement status of each requirement
is tracked in the map below.

## Quality attributes and enforcement

The requirements above are grouped by *architectural concern* — the axis a contributor uses to find
the rules that apply to the code in front of them. This section adds two orthogonal views: which
*quality attributes* (in the ISO/IEC 25010 and structural-characteristics vocabulary) each group
serves, and *how* each requirement is actually enforced.

### Quality-attribute coverage

| AR group | Quality attributes served |
|---|---|
| AR-ORG | Modularity, Analysability, Modifiability |
| AR-MOD | Modularity, Reusability, Modifiability; Fidelity = performance⇄accuracy trade-off |
| AR-PKT | Compatibility / Interoperability, Correctness, Performance efficiency |
| AR-COM | Modularity, Extensibility, Interoperability |
| AR-LIFE | Reliability (recoverability), Testability |
| AR-QUEUE | Modularity, Reusability, Extensibility |
| AR-OBS | Analysability, Testability |
| AR-CFG | Configurability, Maintainability (DRY) |
| AR-EXT | Extensibility, Modularity, Modifiability |
| AR-BUILD | Deployability, Portability, Reproducibility |
| AR-QUAL | Testability, Reliability, Analysability, plus the enforcement/governance dimension |

### Enforcement tiers

Each requirement is enforced at the strongest tier available to it; the goal of AR-QUAL-ENFORCED is to
move every requirement as far up this ladder as it can go.

- **T1 — won't build.** The compiler or NED toolchain rejects non-compliance.
- **T2 — fails a CI test.** A fingerprint, unit, module, statistical, or validation test breaks.
- **T3 — flagged by a deterministic check.** A linter, `featuretool`, build matrix, or a custom
  architecture script reports it in CI.
- **T4 — flagged by an agent reviewer.** An LLM reviewer, run as a CI gate, judges the diff against
  the requirement — catching *semantic* violations no static rule can express (visualization logic in
  a protocol, a zero-time message used as a procedure call, prose duplicating NED). This rung is what
  makes the "judgment" requirements enforceable rather than merely hoped-for, and it scales in a way
  human review does not.
- **T5 — human review.** Reserved for genuine design judgment and final sign-off (is a fidelity level
  worth adding? does this read as one system?).

*Proposed* checks below are achievable today with off-the-shelf tooling; starter artifacts live under
`doc/tmp/enforcement/`.

### Enforcement map

| Requirement | Tier | Enforced by (→ how to raise) |
|---|---|---|
| AR-ORG-DOMAINS | T1→T3 | NED package=directory (compiler) → include-graph layering script |
| AR-ORG-CONTRACTS | T1 | NED `like`/`moduleinterface` + C++ virtuals; contract-package purity → lint (T3) |
| AR-ORG-VIS-SPLIT | T3+T4 | `#include visualizer/` check (T3) + agent review for vis logic in protocol code |
| AR-ORG-KERNEL | T4 | agent review: "does this reimplement or patch a kernel facility?" |
| AR-MOD-COMPOSITION | T1+T4 | NED composition (compiler); agent review for composition-over-inheritance |
| AR-MOD-PLUGGABLE | T1 | NED `like` / default type / `if typename` (compiler) |
| AR-MOD-FIDELITY | T4→T5 | agent flags hardcoded fidelity; whether a level is worth adding is human judgment |
| AR-MOD-NODEBASE | T1/T2 | NED extension + `absPath`/`^` resolution (compiler/runtime) |
| AR-PKT-CHUNKS | T1/T2 | Chunk types (compiler) + runtime immutability asserts (debug) |
| AR-PKT-DUAL | T2 | serializer registry + fingerprint `D` ingredient → serializer-completeness test |
| AR-PKT-TAGS | T1/T2 | tag API (compiler); PHY strips tags (tested) |
| AR-PKT-ERRORS | T1 | chunk/error API (compiler) |
| AR-PKT-SIGNAL | T1/T2 | Signal API (compiler) + runtime asserts |
| AR-COM-REGISTRY | T1/T2 | registration macros (compiler) + runtime "register once" check |
| AR-COM-DISPATCH | T2 | MessageDispatcher + tags at runtime (tested) |
| AR-COM-SOCKETS | T1+T4 | socket API provided (compiler); agent review that apps use sockets, not raw messages |
| AR-COM-DIRECT | T3+T4 | lint for `scheduleAt(simTime())`/zero-delay send + agent review; runtime zero-delay hook (T2) |
| AR-LIFE-STAGES | T1/T2 | `INITSTAGE_*` / `Define_InitStage_Dependency` (compiler) + runtime ordering |
| AR-LIFE-OPERATIONS | T2 | `@lifecycleSupport` + module/lifecycle tests |
| AR-QUEUE-ROLES | T1 | queueing `I*` contracts (compiler) |
| AR-QUEUE-STREAMING | T1/T2 | streaming API (compiler) + tests |
| AR-OBS-SIGNALS | T1/T2 | NED `@signal`/`@statistic` (compiler) + fingerprint neutrality |
| AR-OBS-NED-TRUTH | T1+T4 | NED authoritative, reference generated from it; agent review for prose duplicating NED |
| AR-OBS-INTROSPECTION | T3+T4 | dissector/printer-completeness test + agent review |
| AR-OBS-FLOWS | T1/T2 | region-tag API (compiler) + flow tests |
| AR-CFG-INFER | T1+T4 | NED gate/size inference (compiler); agent review for DRY |
| AR-CFG-PARAMS | T1+T4 | **units library + `@unit` (compile-time dimensional analysis)**; agent review for single-meaning fields |
| AR-EXT-NOCORE | T3+T4 | feature-off build (core→optional deps) + agent review "no core edits for a new protocol" |
| AR-EXT-ATTACH | T1/T3 | attachment API (compiler) + feature-off build |
| AR-EXT-FEATURES | T3 | `inet_featuretool` dependency validation + feature-matrix build |
| AR-BUILD-OUTOFTREE | T2/T3 | CI clean/isolated build (opp_ci from a pinned commit) |
| AR-BUILD-DECLARATIVE | T1+T4 | build descriptors (`.oppfeatures`, opp descriptors); agent review for single-source-of-truth |
| AR-QUAL-FINGERPRINT | T2 ✔ | fingerprint tests on CI |
| AR-QUAL-TESTS | T2+T4 | test suites on CI; coverage/mutation gate + agent review that a change ships matching tests |
| AR-QUAL-DETERMINISM | T2 ✔ | fingerprint stability (same seed → same fingerprint; parallel-safe) |
| AR-QUAL-NAMING | T3+T4 | `clang-tidy` (C++) + agent review for NED/`.msg`/semantic names |
| AR-QUAL-DISPLAY | T3+T4 | "every module has an icon" coverage check + agent review |
| AR-QUAL-LOGGING | T3+T4 | `-Werror`/`clang-tidy` + agent review that programming errors throw, not log |
| AR-QUAL-TRACEABILITY | T3 | fingerprint tags + source→config mapping (partial) |
| AR-QUAL-ENFORCED | — | the CI gate set itself; measured by how many rows above reach T1–T4 (automated) |
