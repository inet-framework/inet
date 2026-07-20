# Architecture audit report — `src/inet/common/packet`

- **Scope:** `src/inet/common/packet` (recursive)
- **Date:** 2026-07-20
- **Command:** `doc/tmp/enforcement/check-architecture.sh src/inet/common/packet`
- **Result:** FAIL — 8 `AR-ORG-DOMAINS` couplings across 5 files; 0 `AR-ORG-VIS-SPLIT`.
- **Rules:** [architectural-requirements.md](../architectural-requirements.md) AR-ORG-DOMAINS,
  AR-ORG-VIS-SPLIT. Foundational value types (addresses, protocol ids) are allowlisted (AS-01).

## Findings

| # | File:line | Includes | Included type is | Ledger cluster |
|---|---|---|---|---|
| 1 | `packet/printer/PacketPrinter.h:16` | `physicallayer/common/Signal.h` | physical-layer signal | AV-ORG-02 |
| 2 | `packet/recorder/PcapngWriter.h:13` | `networklayer/common/NetworkInterface.h` | node-structure | AV-ORG-01 |
| 3 | `packet/recorder/IPcapWriter.h:13` | `networklayer/common/NetworkInterface.h` | node-structure | AV-ORG-01 |
| 4 | `packet/recorder/PcapRecorder.cc:20` | `linklayer/common/InterfaceTag_m.h` | interface tag | AV-ORG-01 |
| 5 | `packet/recorder/PcapRecorder.cc:21` | `networklayer/common/InterfaceTable.h` | node-structure | AV-ORG-01 |
| 6 | `packet/recorder/PcapRecorder.cc:24` | `physicallayer/common/Signal.h` | physical-layer signal | AV-ORG-02 |
| 7 | `packet/recorder/PcapRecorder.cc:25` | `.../packetlevel/IReception.h` | physical-layer reception | AV-ORG-02 |
| 8 | `packet/recorder/PcapRecorder.cc:26` | `.../packetlevel/ITransmission.h` | physical-layer transmission | AV-ORG-02 |

All 8 fall in two files-groups: the **PCAP recorder** (`recorder/`, findings 2–8) and the **packet
printer** (`printer/`, finding 1). No socket, application, or clock coupling exists in this subtree,
and there are no visualizer dependencies.

## Analysis

Every coupling here is **observation/recording infrastructure reaching downward**, which is inherent
to what these modules do — but it splits into two kinds with different fixability:

- **Node-structure, for labeling captures by interface** (findings 2–5: `NetworkInterface`,
  `InterfaceTable`, `InterfaceTag`). The recorder needs to know *which interface* a captured packet
  belongs to. This is the **more fixable** kind: interface identity can travel as a neutral value
  (an interface name/id already carried on the packet) rather than by including the interface-table
  module and the link-layer tag. Reducing it aligns with AR-EXT-ATTACH (depend on a neutral
  abstraction, not the concrete owner).
- **Physical-layer signal, to capture/print the on-air waveform** (findings 1, 6–8: `Signal`,
  `IReception`, `ITransmission`). Recording or printing a *wireless* transmission genuinely needs the
  physical signal. This is the **hard** kind: there is no neutral signal abstraction in `common/`
  today, so removing it would require introducing one (or accepting the dependency).

## Recommendation

Treat this subtree as **observation infrastructure** and pick one disposition per kind, recording it
in [architecture-exceptions.md](../architecture-exceptions.md):

1. **Physical-layer coupling (AV-ORG-02, findings 1/6/7/8):** *sanction* — either allowlist
   `physicallayer/common/Signal.h` + the reception/transmission contracts for `common/packet/`, or
   relocate `recorder/` and the signal-printing part of `printer/` into a dedicated observation
   package that is explicitly permitted to depend downward. Prefer relocation: it keeps the *rule*
   ("plain `common/` must not reach up") sharp instead of accreting allowlist entries.
2. **Node-structure coupling (AV-ORG-01, findings 2/3/4/5):** *fix* — carry the interface identity the
   recorder needs as a neutral tag (interface name/id) so `recorder/` no longer includes
   `InterfaceTable` / `NetworkInterface` / `InterfaceTag`. This is a small, self-contained change and
   the best first fix in this subtree.

Net: 4 findings are a genuine, contained fix (interface identity); 4 are a placement decision about
where observation infrastructure lives. Neither blocks; both are now on record.
