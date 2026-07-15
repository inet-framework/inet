# INET Framework — Architectural Requirements

Requirements from the perspective of INET developers and model contributors: the
design rules the code base must follow so that models remain composable,
extensible, and maintainable.

## Code Organization (AR-ORG)

- **AR-ORG-DOMAINS** — Source code is organized by protocol layer and functional
  domain; cross-cutting infrastructure lives in a common package, and dependencies
  point strictly from protocols to infrastructure, never the reverse.
- **AR-ORG-CONTRACTS** — Every extensible role is defined first as a contract: a
  paired C++ interface and NED module interface, kept separate from reusable base
  implementations and from concrete models.
- **AR-ORG-VIS-SPLIT** — Model logic, visualization, and infrastructure are
  separate packages; protocol code contains no visualization or instrumentation
  logic.

## Module Design (AR-MOD)

- **AR-MOD-COMPOSITION** — Functionality is built by composing small,
  single-purpose modules: behavior lives in simple modules, structure in compound
  modules; composition is preferred over inheritance.
- **AR-MOD-PLUGGABLE** — Submodules are declared against module interfaces with
  replaceable default types; optional submodules can be omitted entirely through
  configuration.
- **AR-MOD-NODEBASE** — Network nodes are assembled incrementally from shared
  per-layer base modules; node-scoped services are found by lookup within the
  enclosing node, never by hardcoded module paths.

## Packet Representation (AR-PKT)

- **AR-PKT-CHUNKS** — Packet content is represented as typed, immutable, shared
  chunks; packet operations create views rather than copying or destroying data.
- **AR-PKT-DUAL** — Every protocol header supports both a field-based and a
  raw-byte representation, bridged by registered serializers, so models can
  interoperate with real network data.
- **AR-PKT-TAGS** — Packet metadata travels in tags attached to packets or to byte
  regions, never encoded in wire content; tags do not cross the physical
  transmission boundary.
- **AR-PKT-ERRORS** — Transmission errors are representable at multiple levels of
  fidelity, selectable per use case.

## Protocol Interaction (AR-COM)

- **AR-COM-REGISTRY** — Protocols, and the services modules provide for them, are
  declared in a global registry at initialization; message dispatch is driven by
  these registrations.
- **AR-COM-DISPATCH** — Modules address peers by protocol and service rather than
  by wiring topology; the same protocol module works unchanged under different
  node compositions.
- **AR-COM-SOCKETS** — Applications interact with protocols through socket-style
  APIs with callback interfaces, not through raw message exchange.

## Initialization & Lifecycle (AR-LIFE)

- **AR-LIFE-STAGES** — Cross-module initialization follows a single, globally
  defined stage sequence; each stage has a documented contract, and new models
  slot into existing stages rather than inventing their own ordering.
- **AR-LIFE-OPERATIONS** — Modules that support shutdown, restart, or crash
  implement the common lifecycle protocol and are controllable from scripted
  scenarios.

## Composable Packet Processing (AR-QUEUE)

- **AR-QUEUE-ROLES** — Packet-processing elements implement standard push/pull
  source/sink contracts, so queues, filters, schedulers, shapers, and similar
  elements compose into arbitrary chains.
- **AR-QUEUE-STREAMING** — The processing contracts support progressive packet
  transfer, so preemption and cut-through behavior are expressible.

## Observability (AR-OBS)

- **AR-OBS-SIGNALS** — Models expose observable behavior by emitting declared
  signals; statistics recording and visualization subscribe to signals and never
  participate in protocol logic.
- **AR-OBS-NED-TRUTH** — A module's external interface — parameters, gates,
  signals, statistics — is documented in its NED declaration, which is the single
  source of truth; other documentation must not duplicate it.
- **AR-OBS-INTROSPECTION** — Each protocol registers support for the common
  introspection tooling (dissection, printing, filtering) alongside its
  implementation.

## Extensibility (AR-EXT)

- **AR-EXT-NOCORE** — New protocols are added purely through the existing contract
  and registration points; no modification of core code is required.
- **AR-EXT-ATTACH** — Shared core data structures are extended by attaching
  protocol-specific data to them, so the core never depends on optional
  protocols.
- **AR-EXT-FEATURES** — Optional functionality is partitioned into independently
  disableable features with declared dependencies; code touching an optional
  subsystem is guarded so everything else builds without it.

## Quality & Conventions (AR-QUAL)

- **AR-QUAL-FINGERPRINT** — Behavioral regressions are guarded by simulation
  trajectory fingerprints; changes that intentionally alter behavior update the
  recorded expectations in a separate, reviewable step.
- **AR-QUAL-TESTS** — Contributions ship with tests in the categories matching
  their nature (unit, module behavior, statistical, validation) in addition to
  fingerprint coverage.
- **AR-QUAL-NAMING** — Modules, packages, signals, and packet types follow the
  framework-wide naming conventions, so a component's role is recognizable from
  its name alone.
- **AR-QUAL-LOGGING** — Log output follows the common level semantics and
  formatting rules; programming errors raise exceptions and are never merely
  logged.
