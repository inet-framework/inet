# INET Framework — Requirements

Requirements from the perspective of the simulation user: a researcher or engineer
who builds, configures, runs, and analyzes network simulations with INET, without
necessarily writing C++ code.

## Modeling Scope (R-SCOPE)

- **R-SCOPE-INTERNET** — Provide simulation models for the standard Internet
  protocol suite across all layers, from applications down to the physical layer.
- **R-SCOPE-WIRELESS** — Support wireless networking with explicit modeling of the
  shared radio medium, signal propagation, and interference.
- **R-SCOPE-TSN** — Support time-sensitive and deterministic networking, including
  time synchronization, traffic shaping, and redundancy mechanisms.
- **R-SCOPE-CROSSCUT** — Model cross-cutting physical concerns — node mobility,
  power and energy, hardware clocks with drift, and the physical environment — as
  optional, freely combinable aspects of any network.
- **R-SCOPE-FIDELITY** — Offer multiple levels of modeling detail for the same
  concern, so users can trade accuracy against simulation performance.

## Composing Networks (R-COMPOSE)

- **R-COMPOSE-NOCODE** — Users can assemble, configure, and run simulations
  entirely declaratively; programming is required only to create new models.
- **R-COMPOSE-NODES** — Provide prebuilt network node types whose ingredients
  (applications, protocols, interfaces) can be replaced, extended, or omitted
  through configuration alone.
- **R-COMPOSE-AUTOCONF** — Network-level setup such as addressing and routing can
  be fully automatic, fully manual, or any mix of the two.
- **R-COMPOSE-DEFAULTS** — Model parameters have sensible defaults, so minimal
  configurations already produce meaningful simulations.

## Running Simulations (R-RUN)

- **R-RUN-SCENARIO** — Dynamic scenarios can be scripted: topology, parameters,
  and node states may change at predefined simulation times.
- **R-RUN-LIFECYCLE** — Nodes and protocols model startup, shutdown, crash, and
  recovery, so failure and transient behavior can be studied.
- **R-RUN-REPRO** — Simulation runs are deterministic and exactly reproducible.

## Results & Analysis (R-RESULT)

- **R-RESULT-BUILTIN** — Models declare their own statistics; useful results are
  recorded by default and are reconfigurable without code changes.
- **R-RESULT-FLOWS** — End-to-end measurements can be collected per logical
  traffic flow, across multiple nodes and protocol layers.
- **R-RESULT-EXPORT** — Traffic can be captured in standard trace formats
  consumable by external network analysis tools.
- **R-RESULT-AUTOMATION** — Simulation campaigns can be built, run, and analyzed
  programmatically, supporting large parameter studies and optimization.

## Visualization (R-VIS)

- **R-VIS-LAYERS** — Network activity can be visualized per protocol layer, in
  both 2D and 3D.
- **R-VIS-NEUTRAL** — Visualization is optional, independently configurable, and
  never affects simulation results.
- **R-VIS-LIVE** — Selected statistics can be displayed live on the network canvas
  while the simulation runs.

## Real-World Integration (R-EMU)

- **R-EMU-BRIDGE** — Simulated and real network components can be combined in both
  directions: real components in a simulated network, and simulated components in a
  real network.
- **R-EMU-REALTIME** — Simulations can execute synchronized to wall-clock time for
  hardware-in-the-loop use.

## Documentation & Learning (R-DOC)

- **R-DOC-LAYERED** — Documentation is layered by audience: a user's guide for
  simulation assembly, a developer's guide for model implementation, and generated
  per-module reference material.
- **R-DOC-RUNNABLE** — Every major feature area is accompanied by runnable
  examples; tutorials and focused showcase studies teach the framework
  progressively.

## Distribution & Compatibility (R-DIST)

- **R-DIST-FEATURES** — Users can enable exactly the feature subset they need;
  disabled features are fully excluded from the build.
- **R-DIST-PLATFORM** — Runs on every platform supported by the underlying
  simulation framework.
- **R-DIST-COMPAT** — Each release states its required simulation-framework
  version and documents breaking changes with migration guidance.
- **R-DIST-LICENSE** — Distributed as open source under a license that permits
  both research and commercial model development.
