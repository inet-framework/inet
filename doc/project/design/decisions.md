# Design decisions

> **Kind:** decision · **Status:** current · **Seal:** by decision · **Owns:** `D-*` · **Stands on:** [accepted-requirements.md](../requirement/accepted-requirements.md)

**How INET is built.** Not what it must do — that is
[accepted-requirements.md](../requirement/accepted-requirements.md) — but the choices the framework
rests on, what each one buys, and what each one costs.

A decision below could have gone another way and did not. Each one names the requirements it serves
and the price it charges, because a design that carries no cost has not been described honestly. The
option that lost is in [rejected-designs.md](rejected-designs.md), as a `REJ-*` entry.

A decision is a *choice*. The rule that a change is checked against is in
[rule/architecture.md](../rule/architecture.md), as an `AR-*` rule. Most decisions here have one or
more `AR-*` rules that keep them true; the decision says why, and the rule says what to check.

**Cite, do not repeat.** What each part *is*, in its settled form, is in the anatomy documents:
[node-anatomy.md](node-anatomy.md), [protocol-anatomy.md](protocol-anatomy.md),
[packet-anatomy.md](packet-anatomy.md) and [test-anatomy.md](test-anatomy.md). How a mechanism is
used is a chapter of the Developer's Guide under `doc/src/developers-guide/`.

## How to read a decision

1. A **bold lead sentence** stating the choice.
2. One or two sentences on what it buys.
3. *Serves* — the requirements and the user-visible capability behind it.
4. *Costs* — what the choice charges, and who pays.
5. *Kept true by* — the `AR-*` rules that stop it from eroding.

## Index

Every decision in document order. The identifier links to the decision; the statement is its lead sentence.

**The decisions**

| Rule | Statement |
| --- | --- |
| [D-KERNEL](#d-kernel) | Build on the OMNeT++ kernel |
| [D-NED-TRUTH](#d-ned-truth) | NED is the single source of truth for a module's external interface |
| [D-COMPOSE](#d-compose) | Behavior comes from small simple modules, structure from compound modules |
| [D-CONTRACTS](#d-contracts) | Every extensible role is a named contract |
| [D-CHUNKS](#d-chunks) | Packet content is a tree of typed, immutable, shared chunks |
| [D-DUAL](#d-dual) | Every header has both a field form and a raw-byte form |
| [D-TAGS](#d-tags) | Local metadata travels in tags, never in wire content |
| [D-SIGNAL](#d-signal) | A physical transmission is a Signal, distinct from the packet it carries |
| [D-REGISTRY](#d-registry) | Peers are addressed by protocol and service, not by wiring |
| [D-SOCKETS](#d-sockets) | Applications talk to transports through socket-style APIs |
| [D-DIRECT](#d-direct) | Same-instant, same-node coordination is a direct C++ call |
| [D-QUEUEING](#d-queueing) | A datapath is a chain of push and pull elements |
| [D-FIDELITY](#d-fidelity) | One concern is offered at several levels of detail, in one contractual slot |
| [D-OBSERVE](#d-observe) | Observation is one way: the model emits, the observer subscribes |
| [D-EXTEND-BY-ATTACH](#d-extend-by-attach) | A new protocol extends the core by attaching to it, never by editing it |
| [D-FEATURES](#d-features) | Optional functionality is partitioned into features that can be switched off |
| [D-FINGERPRINT](#d-fingerprint) | A behavioral regression is caught by a trajectory fingerprint |

## The decisions

### D-KERNEL

**Build on the OMNeT++ kernel**

**INET is a model library on the OMNeT++ simulation kernel, and it does not reimplement or patch the
kernel's facilities.** Event scheduling, the random number streams, NED, the result recording, the
user interfaces and the tooling belong to the kernel; INET builds models on them.

- *Serves* `R-DIST-PLATFORM`, `R-DIST-COMPAT`, `R-RUN-REPRO`. One kernel means one event trajectory,
  one result format and one debugging surface for every model in the ecosystem.
- *Costs* a dependency on the kernel's release cadence and its limits. A kernel defect is worked
  around here and repaired upstream, which is slower than a local patch. Each INET release must
  state the kernel version it needs.
- *Kept true by* [AR-ORG-KERNEL](../rule/architecture.md#ar-org-kernel).

### D-NED-TRUTH

**NED is the single source of truth for a module's external interface**

**What a module offers — its parameters, gates, signals, statistics and display — is declared in NED,
and everything else is derived from that declaration.** The reference documentation, the
configuration surface and the tooling read the same file.

- *Serves* `R-COMPOSE-NOCODE`, `R-RESULT-BUILTIN`, `R-DOC-LAYERED`.
- *Costs* a second language to learn beside C++, and a discipline: a value that belongs in NED must
  not be hardcoded in C++ for convenience.
- *Kept true by* [AR-OBS-NED-TRUTH](../rule/architecture.md#ar-obs-ned-truth),
  [AR-CFG-PARAMS](../rule/architecture.md#ar-cfg-params).

### D-COMPOSE

**Behavior comes from small simple modules, structure from compound modules**

**A capability is built by connecting small single-purpose modules, not by growing one module or one
inheritance chain.** A node, a protocol stack and a datapath are all compositions.

- *Serves* `R-COMPOSE-NODES`, `R-SCOPE-CROSSCUT`.
- *Costs* module count and NED verbosity. A behavior that a reader could hold in one file is spread
  over several, and a deep tree takes longer to walk.
- *Kept true by* [AR-MOD-COMPOSITION](../rule/architecture.md#ar-mod-composition),
  [AR-MOD-NODEBASE](../rule/architecture.md#ar-mod-nodebase).

### D-CONTRACTS

**Every extensible role is a named contract**

**A role that more than one implementation can fill is a C++ abstract class and a NED
`moduleinterface`, and a slot that holds it is interface-typed with a replaceable default.** A user
swaps an implementation in configuration; the core never learns the concrete type.

- *Serves* `R-COMPOSE-NODES`, `R-SCOPE-FIDELITY`.
- *Costs* an interface for every role, and the discipline to keep it minimal. A contract that grows
  a method for one implementation stops being a contract.
- *Kept true by* [AR-ORG-CONTRACTS](../rule/architecture.md#ar-org-contracts),
  [AR-MOD-PLUGGABLE](../rule/architecture.md#ar-mod-pluggable).

### D-CHUNKS

**Packet content is a tree of typed, immutable, shared chunks**

**A packet holds typed chunks that are manipulated as views: sharing instead of copying, and
immutability once a chunk is referenced.** A header is a value with fields, not an offset into a byte
array.

- *Serves* `R-SCOPE-INTERNET`, `R-RESULT-EXPORT`. A field a model set is the field an analysis tool
  reads.
- *Costs* an API that is unfamiliar to a reader who expects a byte buffer, and memory for the tree
  where a flat buffer would do. Peek, insert and update are three operations where a C++ struct would
  have one assignment.
- *Kept true by* [AR-PKT-CHUNKS](../rule/architecture.md#ar-pkt-chunks).

### D-DUAL

**Every header has both a field form and a raw-byte form**

**A registered serializer converts each header between its field representation and its wire bytes,
in both directions.** The two forms are the same information.

- *Serves* `R-RESULT-EXPORT`, `R-EMU-BRIDGE`. A packet can leave the simulation for a real network or
  a capture file and come back.
- *Costs* a serializer for every header, and a test that proves the two forms agree. A header added
  without its serializer is a gap that only shows up at the emulation or capture boundary.
- *Kept true by* [AR-PKT-DUAL](../rule/architecture.md#ar-pkt-dual).

### D-TAGS

**Local metadata travels in tags, never in wire content**

**Information that a node needs but a wire does not carry lives in tags attached to the packet, and
the tags stop at the transmission boundary.** A request travels down as a `*Req`, an indication
travels up as a `*Ind`.

- *Serves* `R-RESULT-FLOWS`, `R-SCOPE-INTERNET`.
- *Costs* a discipline that only review can hold: a field that belongs on the wire and is put in a
  tag produces a model that works in simulation and is wrong on real hardware.
- *Kept true by* [AR-PKT-TAGS](../rule/architecture.md#ar-pkt-tags).

### D-SIGNAL

**A physical transmission is a Signal, distinct from the packet it carries**

**The medium carries immutable Signals with an analog model; a packet is what a Signal conveys.**

Interference, reception and errors are computed on the Signal.

- *Serves* `R-SCOPE-WIRELESS`, `R-SCOPE-FIDELITY`.
- *Costs* a second object per transmission and a second vocabulary. A reader must know which of the
  two a piece of code holds.
- *Kept true by* [AR-PKT-SIGNAL](../rule/architecture.md#ar-pkt-signal).

### D-REGISTRY

**Peers are addressed by protocol and service, not by wiring**

**A module declares the protocols and services it provides in a global registry, and a dispatcher
routes by that declaration.** Adding a protocol to a node does not rewire the node.

- *Serves* `R-COMPOSE-NODES`, `R-COMPOSE-AUTOCONF`.
- *Costs* an indirection that a reader must follow to see where a packet goes, and a class of bug
  that moves from compile time to run time: a missing registration is a dispatch failure, not a
  build error.
- *Kept true by* [AR-COM-REGISTRY](../rule/architecture.md#ar-com-registry),
  [AR-COM-DISPATCH](../rule/architecture.md#ar-com-dispatch).

### D-SOCKETS

**Applications talk to transports through socket-style APIs**

**An application uses a socket object with callbacks, not hand-built command and indication
messages.** The socket owns the message shapes.

- *Serves* `R-COMPOSE-NOCODE`, `R-SCOPE-INTERNET`.
- *Costs* a socket implementation for every transport, and one more layer between the application and
  the protocol.
- *Kept true by* [AR-COM-SOCKETS](../rule/architecture.md#ar-com-sockets).

### D-DIRECT

**Same-instant, same-node coordination is a direct C++ call**

**Two submodules of one node that must agree at one instant call each other; they do not exchange a
zero-time message.** A message means an event, and an event means time passes or the medium is
crossed.

- *Serves* `R-RUN-REPRO`, and the quality of every event trace and fingerprint.
- *Costs* a compile-time coupling where a message would have been anonymous, and the loss of that
  interaction from the event log.
- *Kept true by* [AR-COM-DIRECT](../rule/architecture.md#ar-com-direct).

### D-QUEUEING

**A datapath is a chain of push and pull elements**

**Packet processing is built from elements with standard source and sink contracts, so queues,
shapers, filters, classifiers and schedulers chain in any order.** The contracts also carry
progressive transfer, which is what preemption and cut-through need.

- *Serves* `R-SCOPE-TSN`, `R-COMPOSE-NODES`, `R-SCOPE-FIDELITY`.
- *Costs* a vocabulary of contracts to learn before the first datapath, and an indirection for the
  simple case where one queue would have done.
- *Kept true by* [AR-QUEUE-ROLES](../rule/architecture.md#ar-queue-roles),
  [AR-QUEUE-STREAMING](../rule/architecture.md#ar-queue-streaming).

### D-FIDELITY

**One concern is offered at several levels of detail, in one contractual slot**

**A coarse model and a detailed model of the same concern fill the same interface-typed slot, and the
study chooses in configuration.** A large scenario buys abstraction; a focused study buys detail.

- *Serves* `R-SCOPE-FIDELITY`, `R-SCOPE-WIRELESS`.
- *Costs* several implementations of one concern, each with its own tests, and a documentation duty:
  a user must be able to tell which level a result came from.
- *Kept true by* [AR-MOD-FIDELITY](../rule/architecture.md#ar-mod-fidelity),
  [AR-PKT-ERRORS](../rule/architecture.md#ar-pkt-errors).

### D-OBSERVE

**Observation is one way: the model emits, the observer subscribes**

**A model declares signals and emits them; recording, visualization and analysis subscribe from
outside and never call back in.** Visualization and instrumentation live in their own packages.

- *Serves* `R-VIS-NEUTRAL`, `R-RESULT-BUILTIN`, `R-RUN-REPRO`. Turning observation on cannot change
  the result.
- *Costs* a signal declaration for everything worth watching, and a visualizer that must find its
  subject from the outside instead of being called by it.
- *Kept true by* [AR-OBS-SIGNALS](../rule/architecture.md#ar-obs-signals),
  [AR-ORG-VIS-SPLIT](../rule/architecture.md#ar-org-vis-split).

### D-EXTEND-BY-ATTACH

**A new protocol extends the core by attaching to it, never by editing it**

**A protocol is added through the existing contracts and registration points, and shared core
structures gain protocol-specific data by attachment.** No dispatcher switch and no `common/` edit.

- *Serves* `R-SCOPE-INTERNET`, and the maintenance cost of every future protocol.
- *Costs* an attachment mechanism that is more indirect than a field, and a core that must anticipate
  the shape of the attachment.
- *Kept true by* [AR-EXT-NOCORE](../rule/architecture.md#ar-ext-nocore),
  [AR-EXT-ATTACH](../rule/architecture.md#ar-ext-attach).

### D-FEATURES

**Optional functionality is partitioned into features that can be switched off**

**A feature descriptor names a subtree, its dependencies and its build flags, and a disabled feature
is fully excluded from the build.** The user compiles the subset they need.

- *Serves* `R-DIST-FEATURES`, `R-DIST-PLATFORM`.
- *Costs* a dependency graph to keep valid and a build matrix to test. A feature that is never built
  without its neighbours is a feature in name only.
- *Kept true by* [AR-EXT-FEATURES](../rule/architecture.md#ar-ext-features).

### D-FINGERPRINT

**A behavioral regression is caught by a trajectory fingerprint**

**A fingerprint hashes the event trajectory of a configuration, and a change to it is a reviewable
step with a reason.** It is the wide net that catches an unintended behavior change anywhere.

- *Serves* `R-RUN-REPRO`, and the ability to refactor at all.
- *Costs* a baseline that must be updated deliberately, and a strict limit on what it proves: a
  fingerprint says *that* behavior changed, never *whether the new behavior is correct*. That is why
  it never replaces a test of the claim.
- *Kept true by* [AR-QUAL-FINGERPRINT](../rule/architecture.md#ar-qual-fingerprint),
  [AR-QUAL-DETERMINISM](../rule/architecture.md#ar-qual-determinism),
  [AR-QUAL-TRACEABILITY](../rule/architecture.md#ar-qual-traceability), and
  [rule/testing.md](../rule/testing.md).

## What the rules produce together

The decisions above and the `AR-*` rules of [rule/architecture.md](../rule/architecture.md) are not
independent style preferences. They form a network of constraints that reinforce each other, and
their aggregate effect is larger than any rule alone. Seven compound properties are the real goal.
Each one names the rules that produce it together, which is also a review aid: a change that weakens
one rule usually attacks one of these properties, and the property says what else to check.

**A contract graph instead of a dependency tangle.** AR-ORG-DOMAINS, AR-ORG-CONTRACTS,
AR-MOD-COMPOSITION, AR-MOD-PLUGGABLE, AR-COM-REGISTRY, AR-COM-DISPATCH, AR-EXT-NOCORE, and
AR-EXT-ATTACH together produce *structural substitutability*: a component is replaceable not merely
because an interface exists, but because its slot is interface-typed, its service is discoverable
at runtime, its packet identity is explicit, and the core never learns its concrete type. This
combination is what makes adding a protocol an extension rather than a central-core edit.

**A truthful data path from model to wire to evidence.** AR-PKT-CHUNKS, AR-PKT-DUAL, AR-PKT-TAGS,
AR-PKT-ERRORS, AR-PKT-SIGNAL, AR-OBS-INTROSPECTION, and AR-OBS-FLOWS partition information by what
it *is* — typed content is what the packet carries, tags are local metadata, Signals are physical
transmissions, serializers are the wire boundary, region tags preserve identity through
transformation. The emergent property is *evidentiary continuity*: a field inspected in a packet,
serialized into a capture, processed by the PHY, and attributed to a flow is one representation
throughout, so analysis tooling cannot report something the model did not actually represent.

**Composable but causally explicit behavior.** AR-COM-DIRECT, AR-LIFE-STAGES, AR-LIFE-OPERATIONS,
AR-QUEUE-ROLES, and AR-QUEUE-STREAMING each put internal cooperation at its right semantic level:
direct calls for same-instant coordination, scheduled messages for genuine events, stages for
initialization order, queueing contracts for datapath transfer. The result is an event trajectory
that corresponds to modeled behavior rather than implementation plumbing — which is what makes
debugging, performance, and fingerprint signal quality good at the same time.

**Observability without observer effects.** AR-ORG-VIS-SPLIT, AR-OBS-SIGNALS, AR-OBS-NED-TRUTH, and
AR-OBS-INTROSPECTION establish a one-way path: model owner → declared signal → recorder, visualizer,
analyzer. Consumers subscribe from the outside and events are emitted once by their owner, so
recording is additive, never behavioral; NED remains the machine-readable statement of what exists.
Observability becomes an external capability instead of a hidden second implementation of the model.

**Fidelity as a controlled dimension.** AR-MOD-FIDELITY, AR-PKT-ERRORS, AR-PKT-SIGNAL,
AR-EXT-FEATURES, and AR-CFG-PARAMS let a study choose detail deliberately: a coarse error model and
a detailed analog model occupy the same contractual slot, and the choice is visible in
configuration. Large scenarios buy affordable abstraction, focused studies buy detail, and neither
requires replacing the surrounding architecture.

**Reproducible rather than anecdotal correctness.** AR-CFG-INFER, AR-CFG-PARAMS, AR-BUILD-OUTOFTREE,
AR-BUILD-DECLARATIVE, AR-QUAL-DETERMINISM, AR-QUAL-FINGERPRINT, AR-QUAL-TESTS, and
AR-QUAL-TRACEABILITY form a chain — unambiguous configuration → isolated, discoverable build →
deterministic execution → matching tests plus trajectory fingerprints → traceable baselines — in
which each link removes a different source of uncertainty. A fingerprint is meaningful only on a
deterministic model; a deterministic run is useful only when its inputs are known; a passing test is
persuasive only when its type matches the claim and its baseline has provenance.

**Complexity paid once, in infrastructure.** Registries, dispatchers, serializers, signals,
lifecycle protocols, queueing contracts, and feature descriptors are up-front structure whose
aggregate purpose is to make the *next* model cheaper to integrate: each new protocol reuses the
same extension, observation, testing, configuration, and build paths instead of carving a bespoke
path through the core. The architecture has a rising initial discipline cost and a falling marginal
integration cost — without it, every new feature looks locally simple while adding one more special
case to dispatch, inspection, build selection, and tests.

