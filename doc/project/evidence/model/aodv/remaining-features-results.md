# AODV remaining-feature implementation evidence

> **Kind:** report · **Status:** snapshot 2026-11-14 · **Seal:** none · **Owns:** — · **Stands on:** [implementation plan](../../../../../plan/pending/aodv-remaining-features.md), [model coverage](coverage.md)

The implementation starts at `8c331c8ec7`. This record separates observed behavior from static checks and unexecuted comparisons. It supplements the earlier correction-series results; neither source inspection nor fingerprints establish complete RFC conformance.

## Scope

The implementation adds per-neighbor Hello intervals, native errors after originating discovery exhaustion, selected IPv6 links and received aggregates, live secondary-link reconciliation, primary-identity recovery, attached subnet authority, bounded candidate cooperation and independently routed external exports. Shared changes precede the AODV callers: queued-datagram failure reporting, forwarded ARP-exhaustion errors, deleted-interface UDP membership cleanup, pinned IPv6 next-hop resolution and a generic native-expiry query.

Authoritative subnet operation is an opt-in INET profile. Its mandatory scenario-local extension preserves exact-prefix counter domains; its candidate announcements assume trusted finite candidates, a common connected coordination network, reliable bounded-delay delivery and fail-stop failures. It does not provide single authority across arbitrary partitions. The custom IPv6 AODV encoding is an INET adaptation.

## Execution records

All new native protocol fixtures use General, run 0, seed 0. Review documents own their exact selectors, commands, observations and limitations:

| Contract | Direct evidence |
| --- | --- |
| Hello wire extension, timing, malformed input and lifecycle | [Hello interval](../../../../../tests/protocol/aodv/hello-interval-extension.md) |
| IP-owned queue failure and native error suppression | [IPv4](../../../../../tests/protocol/ipv4/queued-datagram-failure.md), [IPv6](../../../../../tests/protocol/ipv6/queued-datagram-failure.md) |
| Originating exhaustion versus silent cleanup/repair | [Discovery failure](../../../../../tests/protocol/aodv/discovery-failure-notification.md) |
| Selected IPv6 links and independent control state | [IPv6 interfaces](../../../../../tests/protocol/aodv/ipv6-interfaces.md) |
| Received IPv6 prefixes and exact-prefix freshness | [IPv6 subnets](../../../../../tests/protocol/aodv/ipv6-subnet-routes.md) |
| Explicit unicast next hop after hook acceptance/reinjection | [IPv6 provider](../../../../../tests/protocol/ipv6/pinned-unicast-next-hop.md) |
| Secondary loss, removed history, delayed output and repair | [Live interfaces](../../../../../tests/protocol/aodv/interface-changes.md) |
| Primary suspension, return and recovery hold | [Primary identity](../../../../../tests/protocol/aodv/primary-identity-changes.md) |
| Deleted-interface multicast membership cleanup | [UDP provider](../../../../../tests/protocol/udp/multicast-interface-removal.md) |
| Attached authority, real relay discovery, repeated failure recovery and members | [Subnet authority](../../../../../tests/protocol/aodv/subnet-authority.md) |
| Native forwarded ARP-exhaustion errors | [IPv4 provider](../../../../../tests/protocol/ipv4/arp-failure-unreachable.md) |
| Candidate eligibility, counter transfer and inspection | [Subnet cooperation](../../../../../tests/protocol/aodv/subnet-election.md) |
| External gateways, return traffic, route mutation and expiry | [Infrastructure gateway](../../../../../tests/protocol/aodv/infrastructure-gateway.md) |

Final matching-library verification passes **174 distinct focused cases: 127 protocol, 44 module and three unit cases**. Negative startup cases pass by producing their expected diagnostics. Targeted reruns establish the final strengthened assertions; repeated execution of the same file is not counted as another case.

| Selection | Protocol | Module | Unit | Result and record |
| --- | ---: | ---: | ---: | --- |
| Hello and structural/provider units | 11 | 5 | 3 | PASS; `/tmp/aodv-hello-owned-{protocol,module,unit}-run.log` |
| Queued failure and ARP providers | 18 | 2 | 0 | PASS; `audit/aodv-remaining-features/netfilter-owned-protocol-final/verification.json` |
| Discovery, IPv6 interfaces/prefixes and pinned-next-hop provider | 36 | 11 | 0 | PASS; `/tmp/aodv-c345-owned-protocol.log`, `/tmp/aodv-c345-module-owned-protocol.log`, `/tmp/aodv-pinned-owned-protocol.log` |
| Live interfaces, primary identity and UDP membership | 25 | 4 | 0 | PASS; `/home/miguel/workspace/ai-logs/executions/aodv-live-owned-protocol-bcb9f5d13fa8/record.json` |
| Subnet authority/cooperation and external gateways | 37 | 12 | 0 | PASS; `/tmp/aodv-subnet-owned-protocol-record.json` |
| Coordination configuration boundaries | 0 | 10 | 0 | PASS; `/tmp/aodv-hello-owned-module-run.log` (the shared 15-case selection also contains the five Hello module cases above) |

The native recovery assertions additionally passed three targeted cases after explicitly measuring at least three counter advances from their pre-failure baseline. The crash test passed with a requested floor of `fffffffe` after previously known zero, distinguishing lost validity from preserved counter state. Records are `/tmp/aodv-subnet-debug8-native-delta-record.json` and `/tmp/aodv-subnet-debug8-crash-floor-record.json`.

Earlier epochs retain their own hashes and results; they are not promoted to final evidence. A shared file lock serializes test runners and debug-library replacement, and generated outputs are archived before release. Work-directory interference and fixture compile/setup failures are excluded from protocol verdicts. In particular, native Router Solicitation correctly refreshed a Hello deadline, and AckingMac could wake a radio set OFF. The final isolation fixture establishes both positive Hellos and then sets only the peer's primary interface DOWN; the DUT's interfaces remain UP.

## Builds and feature gates

Final debug and release compilation passed after the published audit's protocol-ownership correction, both exit 0: `/tmp/aodv-owned-protocol-debug-build.log` and `/tmp/aodv-owned-protocol-release-build.log`. All 174 cases were repeated against the resulting debug library. No production source changed after these builds.

| Library | SHA-256 |
| --- | --- |
| `src/libINET_dbg.so` | `bcb9f5d13fa867ddca91b87c45121fe280c38c2a6042d8fc4b830e7d777e1011` |
| `src/libINET.so` | `65ec7c99e5f506c55f57a514787de5a87e9041d5bd1287d4df0c41aaf70d8981` |

All focused behavior checks use the debug library. Release behavior is not separately claimed. Ten affected translation units also compiled with an isolated `features.h` overlay disabling `INET_WITH_IPv6`, including the new AODV-owned registration file; the current default build enables IPv6. The overlay and exact object commands are in `audit/aodv-remaining-artifacts/ipv4-only-compile/compile-owned-protocol.log`. This checks the changed-source feature gates; it is not an IPv4-only whole-library link or runtime campaign.

The ownership correction was built while pending against published parent `a16aaac0ba`; it was then folded into its owning commits. The tested `src/` Git tree is `1f024452c9f0b0d605c38e16849ee39b35ca56e6`. `audit/aodv-remaining-features/owned-protocol-source-manifest.json` records that tree and both library hashes, allowing final commit reconstruction to be checked without treating the old parent label in runner metadata as the tested source.

## Fingerprint comparison

Dependency mapping uses `tests/dependency.json`: `src/inet/routing/aodv` maps to `inet.routing.aodv`, feature `Aodv`, and the selected `examples/aodv` configurations. The project description was copied with only its INI discovery restricted to that directory; model paths, feature exclusions and fingerprint store were unchanged. Selection is `-w '^examples/aodv$' -c '^(Static|SimpleRREQ|SimpleLifecycle|StaticExternal)$' -r 0`, debug, ingredients `tplx`, `~tNl`, `~tND`. Stored limits are 100 s except Static at 50 s.

All four simulations completed. Against the stored baseline, **one of 12 ingredient comparisons matches and 11 differ**, so the fingerprint runner exits 1. The post-audit rerun is `/tmp/aodv-fingerprint-owned-protocol.log` and `-result.json`; it reproduces every earlier calculated fingerprint value. No baseline was updated.

| Configuration | Current `tplx` | Current `~tNl` | Current `~tND` |
| --- | --- | --- | --- |
| SimpleLifecycle, 100 s | `282a-c041` | `4ac4-ccff` | `d743-9a36` |
| SimpleRREQ, 100 s | `f4b3-91b9` | `d2e6-a947` (matches) | `3256-af4d` |
| Static, 50 s | `0dc8-7176` | `96a3-1d0b` | `bc4e-929d` |
| StaticExternal, 100 s | `d013-b695` | `5b32-2ffa` | `82e6-3c79` |

Two explicit counterfactuals identify the additional feature-series effects without claiming an exact parent binary. Both restore `Aodv.{h,cc}`, its control serializer and IPv4 ARP-exhaustion behavior from `8c331c8ec7`, while retaining that experiment's generic/message infrastructure. The second additionally reproduces the new unused coordination socket's unique-number allocations. The source overlays, compile/link commands and libraries are in `audit/aodv-remaining-artifacts/fingerprint-ablation/`; results are `/tmp/aodv-fingerprint-{ablation,normalized}.log` and corresponding JSON. These counterfactuals belong to the earlier debug-8 epoch; the post-audit rerun independently confirms that the ownership correction changes none of the 12 calculated values.

| Observation relative to the counterfactual | Evidence |
| --- | --- |
| All four packet-length/time fingerprints agree. | `~tNl` values are identical. |
| Three packet-data differences come from global identifier allocation. | Reproducing the extra UDP socket allocation reproduces all four current `~tND` values. `UdpSocket` and `PingApp::pid` both use `getUniqueNumber()`. |
| Static, SimpleRREQ and StaticExternal event fingerprints agree. | Their `tplx` values are identical. |
| SimpleLifecycle contains one additional native socket-close indication. | At 12.3 s, both traces process the shutdown and UDP close request; current code then processes `closed` at AODV before the TCP lifecycle timer. The old behavior has already finished without that acknowledgment. This matches explicit pending-close tracking. |

The 13-second first-divergence traces and commands are `candidate-lifecycle-info-trace.log` and `normalized-lifecycle-info-trace.log` in the ablation directory. This explains the observed feature-series differences in the selected configurations; it does not convert the older stored mismatches into correctness evidence or prove every later trajectory against an exact parent build.

## Static review and comparisons

The prepublication review found and corrected retained-host blocking after interface loss, deferred repair through a lost interface, stale prepared output after down/up, native IPv6 export expiry, suspended-primary error fallback, implicit IPv6 identity validation and missing announcement introspection. Focused cases exercise those boundaries. The native-expiry query also uses a distinct name so it cannot accidentally override DSDV's unrelated metadata expiry. The first published-head audit found one architecture issue: the coordination identity belonged in AODV-owned support. It now uses the existing public `Protocol` constructor there, with no change to the core protocol registry. The repeated introspection unit checks name lookup and the same registered singleton used by sender, dissector and printer. Published-head audit reports are kept separately under `audit/pull-request/`.

Repository-wide architecture, naming and interface checks report existing candidates. A detached upstream-base checkout at `980f5c3f9e` produces the same candidate sets: zero new candidates after normalizing source line numbers. The source-seal, document-seal/index and complete-PR whitespace checks pass. All six series/description gates are recorded in `/tmp/aodv-final-owned-protocol-pr-gates.log`; three source-size notes concern inherited prerequisite commits. The optional clang-tidy gate is NOT_RUN because that executable is absent.

All 39 Mermaid diagrams in the AODV reviewer set, shared-provider reviewers, cooperation design and PR description rendered successfully using Mermaid 11 in a headless Firefox session. SVGs and `results.json` are retained at `/home/miguel/workspace/ai-logs/executions/aodv-diagrams-20261114/`. Two inherited sequence-diagram semicolons were corrected before the successful rendering pass.

Exact parent runtime comparisons are not claimed where the parent lacks the new API, configuration or wire contract. Source-based before/after mechanisms are identified as such in each reviewer document. Stored fingerprint baselines remain unchanged.
