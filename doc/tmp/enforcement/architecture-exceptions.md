# Architecture Exceptions and Violations

Known places where the INET code base departs from the dependency-direction requirements in
[architectural-requirements.md](../architectural-requirements.md) — chiefly **AR-ORG-DOMAINS**
(dependencies point protocols → infrastructure, never the reverse) and **AR-ORG-VIS-SPLIT** (model
code must not depend on the visualizer). It is the architecture counterpart of
[naming-exceptions.md](../naming-exceptions.md), and it is seeded automatically from
[`check-architecture.sh`](check-architecture.sh) — re-running that script *is* the audit.

Each entry has a **disposition**:

- **Sanctioned** (`AS-*`) — a deliberate, accepted coupling. Sanctioned entries are added to the
  allowlist in `check-architecture.sh` so the check stays quiet about them.
- **Violation** (`AV-*`) — a real coupling to fix; the check keeps flagging it until it is inverted
  or the offending type is relocated. Each has a suggested fix and a status (`Open` / `Done`).

The underlying tension is that `common/` legitimately holds framework-wide *value types* and *node
abstractions*, but some of those historically live under a protocol layer, and some genuinely
protocol-specific code has drifted into `common/`. The two need to be told apart.

---

## Sanctioned exceptions

| Id | Coupling | Rule | Why it stays |
|---|---|---|---|
| AS-01 | `common/` → address & protocol-id value types: `MacAddress`, `Ipv4Address`, `Ipv6Address`, `L3Address`, `L3AddressResolver`, `EtherType`, `IpProtocolId` | AR-ORG-DOMAINS | Foundational value types used framework-wide; they *should* live in `common/`, but relocating them is a large, mechanical, high-churn change. Allowlisted in `check-architecture.sh`. **Real fix:** move them under `common/`. |

---

## Open violations

Grouped by cluster (the check reports the individual `file:line` hits). Counts are from the
2026-07-20 audit.

### Node-structure coupling — *decide*

| Id | Coupling | Where (examples) | Suggested resolution | Status |
|---|---|---|---|---|
| AV-ORG-01 | `common/` infra → `NetworkInterface` / `IInterfaceTable` / `InterfaceTable` / `InterfaceTag` (~11) | `MessageDispatcher`, `LifecycleController`, `InterfaceOperations`, `IInterfaceRegistrationListener`, `packet/recorder/*` | These are foundational *node-structure* abstractions, like AS-01 — either **sanction + allowlist** them (and ideally move them to `common/`), or invert via a `common/`-side interface. Pick one and record it. | Open (decide) |

### Observation/recording infra → physical layer — *decide*

| Id | Coupling | Where | Suggested resolution | Status |
|---|---|---|---|---|
| AV-ORG-02 | `common/` observation code → `physicallayer` `Signal` / `ReceptionBase` / `IReception` / `ITransmission` / `INarrowbandSignalAnalogModel` / `SignalTag` (~9) | `ResultFilters.cc`, `packet/printer/PacketPrinter.h`, `packet/recorder/PcapRecorder.cc` | Result filters and packet recorders inherently observe every layer. Either accept as observation infrastructure, or relocate the recorders/result-filters out of `common/` into a dedicated observation package that is *allowed* to depend downward. | Open (decide) |

### Genuine violations — *fix*

| Id | Coupling | Where | Suggested resolution | Status |
|---|---|---|---|---|
| AV-ORG-03 | `common/socket/SocketMap` → concrete transport sockets `TcpSocket`, `UdpSocket` | `common/socket/SocketMap.cc` | `SocketMap` should be generic over `ISocket`/`INetworkSocket`, not concrete transport types (AR-ORG-CONTRACTS). | Open |
| AV-ORG-04 | `common/clock` → `applications/base/ApplicationBase` | `common/clock/ClockUserModuleMixinImpl.cc` | Clock infrastructure must not know about an application base class; remove the include / depend on a neutral abstraction. | Open |
| AV-ORG-05 | `common/ResultFilters` → `applications/base/ApplicationPacket_m` | `common/ResultFilters.cc` | Move the application-specific result filter out of `common/`, or depend on a neutral packet tag. | Open |
| AV-VIS-01 | model code → `visualizer/` (AR-ORG-VIS-SPLIT) | `mobility/base/MobilityBase.cc`, `environment/ground/OsgEarthGround.cc` | Invert: the visualizer must subscribe to mobility/environment from outside, not the reverse; move any OSG rendering hooks into the visualizer package. | Open |

---

## Auditing

Re-running [`check-architecture.sh`](check-architecture.sh) from the INET repo root reproduces the
violation list. When it reports something new:

1. If the coupling is a deliberate, accepted framework-wide dependency, add an `AS-*` row **and** add
   the header to the allowlist in `check-architecture.sh`.
2. Otherwise add an `AV-*` row with a suggested fix and `Status = Open`; when the coupling is inverted
   or the type relocated, set `Status = Done` (keep the id).

### Audit coverage

| Area | Date | Findings |
|---|---|---|
| `check-architecture.sh` over `src/inet` (AR-ORG-DOMAINS, AR-ORG-VIS-SPLIT) | 2026-07-20 | AS-01 (allowlisted); AV-ORG-01…05 + AV-VIS-01. Include-graph only — behavioral violations of these ARs (e.g. vis *logic* not reached via an include) are not covered by this check and need T4 agent review. |
