# Architecture sweep — the dependency direction

> **Kind:** report · **Status:** snapshot 2026-08-31 · **Seal:** none · **Owns:** — · **Stands on:** [rule/architecture.md](../../../rule/architecture.md)

- **Scope:** `src/inet/common` for `AR-ORG-DOMAINS`, all of `src/inet` for `AR-ORG-VIS-SPLIT`
- **Date:** 2026-08-31, commit `e38566236b`
- **Command:** `doc/project/enforcement/check-architecture.sh`
- **Rules checked:** [AR-ORG-DOMAINS](../../../rule/architecture.md#ar-org-domains), [AR-ORG-VIS-SPLIT](../../../rule/architecture.md#ar-org-vis-split)
- **Result:** FAIL — 25 couplings across 15 files. **All 25 fall inside the existing ledger
  clusters**, so there is nothing new to record.

## Where they fall

| Cluster | Files | Ledger row |
| --- | --- | --- |
| observation and recording reaching into the physical layer | `packet/recorder/PcapRecorder.cc` (5), `packet/printer/PacketPrinter.h` (1) | `AV-ORG-02` |
| node-structure abstractions | `packet/recorder/PcapngWriter.h`, `IPcapWriter.h`, `MessageDispatcher.h/.cc`, `LifecycleController.cc` (2), `InterfaceOperations.cc`, `IInterfaceRegistrationListener.h`, `FingerprintCalculator.cc` | `AV-ORG-01` |
| result filters reaching into an application type | `ResultFilters.cc` (5) | `AV-ORG-02`, `AV-ORG-05` |
| socket map knowing concrete transports | `common/socket/SocketMap.cc` (2) | `AV-ORG-03` |
| clock knowing an application base | `common/clock/ClockUserModuleMixinImpl.cc` | `AV-ORG-04` |
| model code reaching the visualizer | `mobility/base/MobilityBase.cc`, `environment/ground/OsgEarthGround.cc` | `AV-VIS-01` |

`FingerprintCalculator.cc` was not named in the `AV-ORG-01` examples before this sweep; it includes
`networklayer/common/NetworkInterface.h`, which is the same node-structure coupling, so the row's
example list is extended rather than a new row opened.

## The two that are still undecided

`AV-ORG-01` and `AV-ORG-02` are marked `Open (decide)`, not `Open`. They are not obviously defects:
node-structure abstractions and observation infrastructure both have a real argument for reaching
across layers, and the resolution is a decision — sanction them and move the types, or invert them
through a `common/`-side interface.

**That decision now blocks something concrete.** `common/packet/` is sealed, and its seal rests on an
audit whose findings include both clusters. A seal over an unsanctioned violation is what
[SR-AUDIT-FIRST](../../../rule/sealing.md#sr-audit-first) forbids, so the seal is not valid until the
clusters are resolved. See [audit/seal-list.md](../../seal-list.md).

## What this sweep does not cover

The check reads the `#include` graph only. A behavioral violation of either rule — visualization
*logic* in a protocol that is not reached through an include — needs `T4` agent review.
