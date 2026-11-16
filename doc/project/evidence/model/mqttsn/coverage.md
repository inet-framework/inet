# MQTT-SN implementation and verification ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [feature inventory](../../protocol/mqttsn/features.md), [implementation plan](../../../../../plan/pending/mqtt-sn.md)

## PR #1462 corrections, 2026-11-16

The correction series appends source-clause documentation, stable signal values, separate recovery/authentication fixes and this evidence reconciliation after reviewed head `a19a7a77481ac2fcceab934bf710183d1c0a3a23`. Source repair followed the user's “proceed”; subsequent committing, pushing and PR reconciliation were explicitly authorized. The remaining sections record the original implementation/publication campaign; their canceled-publication statements and older counts are historical.

The client now clears the attempted-gateway set when starting a fresh discovery round, after trying unattempted alternatives. A recovered same-id gateway can therefore be selected without canceling the only reset path. The gateway follows the existing MQTT client's credential-presence rule, sending a present zero-length password when a username is configured. Explicit Phase values preserve the existing numeric signal contract. Both endpoints now cite the normative transition clauses and identify local model policies.

Two added protocol fixtures run unchanged against the original implementation and the corrected implementation. `MqttSnGatewayRediscovery` fails on the original client's same-id recovery and passes after the attempt-round correction, including repeated announcements and a later second recovery. `MqttSnAuthentication` fails on original gateway admission and passes after the credential-presence correction; its wrong-password refusal remains a control. Both use debug mode, seed 0 and a 3s simulation limit. Their reviewer explanations are in [discovery and failures](../../../../../tests/protocol/mqttsn/discovery-and-failures.md#rediscovering-a-recovered-gateway) and [connection admission](../../../../../tests/protocol/mqttsn/connect.md#gateway-credentials-with-an-empty-password).

Fresh debug and release builds pass. The directly selected new cases plus DiscoveryBackoff and GatewayFailover pass 4/4. The complete corrected MQTT-SN protocol selection passes 81/81, and the module selection passes 9/9. There are no expected-failure declarations. Exact commands, exit statuses, generated artifacts and source hashes are retained in `audit/pull-request/pr-1462-corrections.md` and `pr-1462-corrected-manifest.txt` in this checkout. The first authentication fixture draft lacked its generated MQTT packet include and did not compile; that setup error is retained separately and is not counted as a behavioral comparison. The corrected identical fixtures then both failed at runtime before the source changes.

Publication also checks the fixes against their immediate parents. At `ac8d584233`, the identical rediscovery fixture fails before the recovery repair; the repaired checkpoint passes rediscovery, no-response backoff and alternative-gateway controls. At `08d1dc6bcc`, the identical authentication fixture fails before the credential repair; the repaired checkpoint passes authentication, ordinary connection and broker-refusal controls. Source increments compile in debug and release. Source bytes at the final implementation checkpoint match the previously verified correction candidate. Exact original P1–P6 intermediate commits were not rebuilt for this publication.

Scoped architecture, naming, source/document protection, Markdown links and whitespace checks pass. Project-wide architecture, naming and interface gates retain findings outside MQTT-SN. The new commits preserve the original implementation history; the audit's earlier commit-message-history finding remains recorded, and rewording drafts remain local. No baseline or shared MQTT production source is modified. External interoperability, feature-off profiles, example/document rendering and release behavioral tests were not rerun for this publication.

## Original implementation campaign

Work was authorized by the user's “Execute the plan” instruction on 2026-11-16. The starting tree is `73e8f28a285acc2305a8a963c3c6ecb95e59fc12`; the existing `AGENTS.md` modification is outside the change. Source protection, protocol ownership, packet/socket contracts and the plan's C1 contract were checked before the first write. Product code belongs under `applications/mqttsn`, with dependencies toward existing MQTT, socket and packet contracts; no protected packet-core edit is required.

The supported profile is MQTT-SN v1.2 over UDP (one message per datagram, maximum 65507 bytes), IPv4 unicast/local broadcast discovery and configured IPv6 unicast. Per-client broker connections terminate QoS independently; shared gateway publications carry connectionless traffic and Wills. Native sensor-link transport, forwarding encapsulation, IPv6 discovery, durable state, encryption, aggregation and full section 4.1 gateway transparency are excluded. The plan defines remaining policy choices and memory-loss limits.

| Step | Implementation | Direct verification |
| --- | --- | --- |
| C1: requirements/profile | Requirement catalog, feature inventory and this ledger written. | Source/plan inspection; no runtime claim. |
| C2: wire format | Packet schema, full codec, protocol identity, dissector/printer and 34 filler vectors. | Dedicated codec/filler module and real UDP wire case pass. Broader serializer suite has a compilation setup failure below. |
| C3: connection | Per-sensor MQTT TCP admission, refusal mapping, bounded setup, connection-scoped framing and endpoint replacement. | IPv4/IPv6, every MQTT refusal code, unavailable setup, duplicate CONNECT, changed UDP port and partial CONNACK cases pass. |
| C4: topics/routing | Distinct normal/predefined/short representations, wildcard routing, registration gates, subscription replay and retained semantics. | Both production directions, retained deletion, topic isolation, subscription retries/results, registration loss/refusal and full topic-table handler checks pass. |
| C5: QoS −1/1 | Shared connection, independent hop ids, duplicate/retry flags, congestion waits and QoS 0 feedback. | Connectionless delivery, both QoS 1 senders, congestion and six zero-id feedback/namespace/repair cases pass. |
| C6: QoS 2/recovery | Retained sender phases, Method B receiver suppression, clean/persistent reset, broker-session restoration queue. | Both senders and receivers, duplicate/id reuse, error PUBACK, clean discard/persistent resume, broker session loss and restoration identifier pressure pass. |
| C7: Will/liveness/sleep | Will matrix/updates, independently live upstreams, active/sleep supervision, suspended downlink exchanges and poll drains. | Negotiation/update retries, shared outage/queued Will, 60/61s boundaries, sleep/poll loss, late acknowledgements, multi-payload drain and expiry pass. |
| C8: discovery/failures/lifecycle | Candidate/suppression/expiry/backoff, live gateway failover, socket/event cleanup and retained restart state. | Discovery/backoff/relayed address/expiry, UDP-only partition with old gateway alive, malformed/stale/wrong-peer traffic, both endpoint lifecycle cases and three stop terminal paths pass. |
| C9: examples/docs/integration | Eight wired/wireless configurations, user-guide chapter, release note, feature dependencies and eight protocol reviewer documents, including the client publication unit. | Fixed-seed examples/counts, user-guide build and feature matrix pass; existing MQTT regression passes. Mechanical gate limitations are recorded below. |

The integrated implementation is divided into six publication commits using the plan's P1–P6 map. C1–C9 remain capability areas, not nine historical behavior increments. The user authorized committing/pushing and then canceled PR creation. No PR is created, no baseline changes apply, and the existing `AGENTS.md` edit is excluded. Exact parent-versus-feature runtime comparisons have not been run: the parent of each new wire/endpoint unit lacks the types needed by its new fixtures, as its commit body and reviewer document explain.

The handwritten MQTT-SN source manifest is `/tmp/inet-mqttsn-source-manifest.txt`, SHA-256 `ab73035a284c05b2a3df08de75d28851535180c0cc6ec851709c3cb5fdaeb5b8` (sorted `sha256sum` entries for package files excluding generated `_m.*`). The publication commits preserve those source bytes. Tests have only trailing blank-line cleanup, and reviewer/publication documents are reconciled separately.

## Publication commit checks

Each partial tree was assembled and built before its endpoint commit, with later endpoint files absent from the working tree. All commands run from this checkout after loading `.envrc`. For each source increment, `make makefiles`, `make MODE=debug -j8` and `make MODE=release -j8` pass. The tests use matching debug libraries and fixed seed 0. Raw prefixes below are under `/tmp/` and begin with `inet-mqttsn-`; `.out` and `.log` hold runner results, and `-debug.log`/`-release.log` hold builds.

| Unit | Commit | Executed checks and result | Artifact prefix |
| --- | --- | --- | --- |
| P1: plan/profile | `7981d042d7` | Documents only. No model behavior or runtime test applies. | Publication plan and inventories |
| P2: wire contract | `94c2a47789` | Codec module 1/1 and UDP wire protocol case 1/1 PASS. No session/client/gateway source present. | `inet-mqttsn-wire` |
| P3: session records | `3bea9d8d14` | Codec/allocation modules 2/2 and prior wire protocol case 1/1 PASS. No client/gateway source present. | `inet-mqttsn-session` |
| P4: sensor client | `eb246a1c6e` | All available modules 5/5 and wire/client protocol cases 23/23 PASS. No production gateway source present. | `inet-mqttsn-client` |
| P5: gateway | `237729f118` | All modules 9/9 and protocol cases 79/79 PASS. | `inet-mqttsn-gateway` |
| P6: examples/docs | Final publication commit | Production bytes unchanged. Both build modes pass after restoring the examples feature. Final mechanical checks and documentation below retain their explicit limits. | `inet-mqttsn-publish` |

The module command for each stage is `opp_run_module_tests --load . -p inet -m debug --no-build -f 'mqttsn_'`; P2 narrows that filter to `mqttsn_serializer\.test$`. The protocol command is `opp_run_protocol_tests --load . -p inet -m debug --no-build -w mqttsn`. Each invocation also sets `-l INFO -L INFO --log-file /tmp/ARTIFACT-PREFIX-module.log` or `-protocol.log`. All selected cases pass with exit 0. Source trees and tests, rather than only the final combined library, establish the staged buildability.

Publication checks use `check-pull-request.sh --base 73e8f28a285acc2305a8a963c3c6ecb95e59fc12` for the MQTT-SN increment and `--base upstream/master` for the entire branch, with no PR number or description argument. The five commit/classification/include/NED-parameter/release-note gates pass; the description gate is deliberately skipped. Logs are `/tmp/inet-mqttsn-publish-series.log` and `/tmp/inet-mqttsn-publish-full-series.log`. Whole-tree architecture/naming/interface checks were rerun and retain findings outside MQTT-SN, while scoped architecture/naming and source/document seals pass. The user-guide rebuild exits 0 with the same two existing cross-manual warnings (`/tmp/inet-mqttsn-publish-doc.log`). Local Markdown targets and Mermaid delimiters pass inspection; rendering is still unverified.

Commit-message classification was corrected before push, preserving each full tree exactly. The tested-to-published aliases are `2f379cf16e→7981d042d7`, `130e13b2f0→94c2a47789`, `a0ed9df79e→3bea9d8d14`, `574c0e02ed→eb246a1c6e`, and `dbef5935ea→237729f118`. Every old/new `^{tree}` comparison matches. The raw stage checks precede this message-only rewrite; no runtime rerun is claimed for a changed source tree.

## Implementation corrections found by direct checks

The initial architecture/ownership contract remained inside `applications/mqttsn`, existing MQTT/socket/packet contracts, new fixtures and documentation. No broker or packet-core refactor was needed. Runtime investigation corrected discovery's missing output-interface tag, kept TTL 1 confined to discovery, avoided gateway takeover reconnect loops, retained persistent downlink phases across setup, and restored accepted subscriptions before flushing new MQTT work under identifier pressure.

The final self-audit added per-session latest-QoS-0 representation context indexed by TopicId. Both endpoints now handle zero-id congestion/invalid-topic PUBACK without an ACK transaction, and a predefined-topic error cannot erase an equal-number normal binding. The plan and user guide record the remaining ambiguity after reuse of the same peer/TopicId with a different representation. This map clears with session exchanges and stores no payload/retry template. A malformed Will topic no longer cancels its setup timer before validation. Persistent first connections receive the configured predefined table.

Shutdown tests found two separate terminal-path defects: timeout failed to finish the active lifecycle operation, and old DISCONNECT transactions could survive restart. Acknowledgement, retry exhaustion and timeout now remove only the shutdown exchange, retain eligible persistent data phases, close/destroy the socket and complete the operation. Gateway callbacks reschedule the shared reconnect timer after closure; no per-sensor autonomous reconnection is introduced.

A fresh accepted subscription invalidates the older UNSUBSCRIBE reply cache. `MqttSnUnsubscribeReuse` reuses the completed unsubscribe id/filter after a new subscription and requires another actual unsubscribe, followed by a closed no-delivery window. The focused replay/subscription rerun passes 9/9 (`/tmp/inet-mqttsn-replay-verified.out`, `.log`).

## Executed checks

All commands below run from `/home/miguel/workspace/inet-mqtt-sn`, after `source .envrc >/dev/null`, unless otherwise stated. Protocol fixtures use `seed-set = 0`; their checked-in `.test` files pin finite limits and parameters. There are no expected-failure markers. The runner's two setup/summary tasks are not counted as tests.

| Check | Command / selection | Outcome | Raw artifacts |
| --- | --- | --- | --- |
| Final debug build | `make MODE=debug -j8` | PASS, exit 0 | `/tmp/inet-mqttsn-final-gateway-build.log` |
| Final release build | `make MODE=release -j6` | PASS, exit 0 | `/tmp/inet-mqttsn-final-gateway-release.log` |
| MQTT-SN production protocol suite | `opp_run_protocol_tests --load . -p inet -m debug --no-build -w mqttsn -l INFO -L INFO --log-file /tmp/inet-mqttsn-protocol-verdict.log` | PASS: 79/79, exit 0 | `/tmp/inet-mqttsn-protocol-verdict.out`, `.log` |
| MQTT-SN module suite | `opp_run_module_tests --load . -p inet -m debug --no-build -f 'mqttsn_' -l INFO -L INFO --log-file /tmp/inet-mqttsn-module-verdict.log` | PASS: 9/9, exit 0 | `/tmp/inet-mqttsn-module-verdict.out`, `.log` |
| Release protocol sample | `opp_run_protocol_tests --load . -p inet -m release --no-build -w mqttsn -f '(WireFormat\|Connect\|RoutingQos2\|.*QosZero.*)\.test$' -l INFO -L INFO --log-file /tmp/inet-mqttsn-release-verified.log` (remove the Markdown table escapes before use) | PASS: 10/10, exit 0; before the final stop-only cleanup | `/tmp/inet-mqttsn-release-verified.out`, `.log` |
| Release shutdown paths | `opp_run_module_tests --load . -p inet -m release --no-build -f 'mqttsn_stop_' -l INFO -L INFO --log-file /tmp/inet-mqttsn-stop-release.log` | PASS: 3/3, exit 0; before final subscription replay cleanup | `/tmp/inet-mqttsn-stop-release.out`, `.log` |
| Existing MQTT integration | `opp_run_protocol_tests --load . -p inet -m debug --no-build -w '^(tests/protocol/)?mqtt(sn)?$' -l INFO -L INFO --log-file /tmp/inet-mqttsn-protocol-final.log` | PASS: 53/53 at that stage: all 24 existing MQTT plus the then-existing 29 MQTT-SN cases, exit 0 | `/tmp/inet-mqttsn-protocol-final.out`, `.log` |

The existing MQTT sources were unchanged throughout. Subsequent changes were confined to MQTT-SN source/tests and documentation. The final protocol selection includes every `.test` in `tests/protocol/mqttsn/`. The module selection is `mqttsn_serializer`, `mqttsn_topic_capacity`, `mqttsn_gateway_capacity`, `mqttsn_restoration_capacity`, `mqttsn_lifecycle`, `mqttsn_gateway_lifecycle`, `mqttsn_stop_ack`, `mqttsn_stop_retry`, and `mqttsn_stop_timeout`. `mqttsn_serializer` executes all 34 filler variants through the registered packet serializer as well as direct golden bytes and malformed/length boundaries. Capacity fixtures are explicitly handler/helper evidence; lifecycle and protocol fixtures exercise real modules/sockets.

Earlier failures are not counted as passes. Initial source/fixture corrections and reruns are retained in `/tmp/inet-mqttsn-protocol-pass1.*`, `protocol-corrections.*`, `boundaries.*`, `discovery-backoff.*`, `count-check.*`, `plan-boundaries.*`, `modules-complete.*` and `modules-verified.*` with the `inet-mqttsn-` prefix. An accidental concurrent debug/release protocol invocation removed shared generated work files (`/tmp/inet-mqttsn-protocol-all.*`); it was a setup failure and was replaced by sequential runs. The count fixtures were corrected because `assertEvent` selects one occurrence; count guards now use filters and separate total counts where required. The stop failures led to the terminal-path fixes above.

## Examples and documentation

From an initialized INET shell, each example ran from `examples/mqttsn` with `inet --debug -u Cmdenv -c CONFIG -r 0 --cmdenv-express-mode=true`. Every run finished at 8s with seed 0 and exit 0. Logs are `/tmp/inet-mqttsn-example-CONFIG.log`; Qos1Loss was rerun with express mode false and app detail logging as `/tmp/inet-mqttsn-example-Qos1Loss-final.log`. Its first measurement establishes registration before the BER=1 burst at 1.15–1.35s; the later PUBLISH is retried with flags 160 (QoS 1 + DUP), TopicId 1 and MsgId 4.

| Configuration | Observer messages / bytes | Sensor messages / bytes |
| --- | ---: | ---: |
| General | 2 / 8 | 1 / 1 |
| Qos1 | 2 / 8 | 1 / 1 |
| Qos2 | 1 / 4 | 1 / 1 |
| Qos1Loss | 2 / 8 | 1 / 1 |
| SleepingSensor | 0 / 0 | 1 / 1 |
| Connectionless | 2 / 8 | 0 / 0 |
| Discovery | 2 / 8 | 1 / 1 |
| Wireless | 2 / 8 | 1 / 1 |

These values were read from each run's `results/CONFIG-#0.sca` application `deliveredMessages:count` and `deliveredBytes:sum`. Exact bytes are asserted in the protocol fixtures; example counts alone are not payload correctness evidence. [Example instructions](../../../../../examples/mqttsn/README.md) state the intended payloads and behavior.

The user guide builds with `source /tmp/inet-mqtt-doc-venv/bin/activate` followed by `PYTHONPATH=/tmp/inet-mqtt-doc-builder/extensions ENGINE=none doc/src/build --only users-guide --media /tmp/inet-mqtt-doc-cache/src/media --cache /tmp/inet-mqttsn-doc-cache --out /tmp/inet-mqttsn-doc-site`. Exit 0; raw log `/tmp/inet-mqttsn-doc-verified.log`, output `/tmp/inet-mqttsn-doc-site/docs/dev/users-guide/ch-mqtt-sn.html`. Two existing warnings refer to the omitted migration/developer guides; there are no MQTT-SN chapter warnings. An initial build failed fetching media, and another lacked `omnetpp_media`; the successful run explicitly uses the available local media and extension sources. Mermaid rendering remains unverified.

## Feature combinations

`opp_featuretool` generated the feature state; package declarations and dependencies remain in `.oppfeatures`. This matrix ran before the final QoS 0/stop fixes, which introduce no new feature dependency. It establishes the declared composition at that point; it is not a claim of rebuilding every matrix entry after each local state-machine correction.

| Profile | Preparation and check | Outcome / artifacts |
| --- | --- | --- |
| MQTT on, MQTT-SN off | `opp_featuretool disable -f MqttSn`, `make makefiles`, debug build, then `MqttConnect.test` in the existing MQTT suite | PASS build and 1/1 runtime; `/tmp/inet-mqttsn-featureoff-build.log`, `featureoff-runtime.out`, `.log` |
| Only MQTT-SN and required neighbors | `opp_featuretool disable -f all`, `opp_featuretool enable -r MqttSn`, `make makefiles`, debug build | PASS; `/tmp/inet-mqttsn-minimal-build.log`. Examples/tests and IPv4/TCP implementations are disabled in this compile-only profile. |
| Full examples/tests restored | `opp_featuretool enable -r MqttSnExamples MqttSnTests`, compare generated state with saved full profile, `make makefiles`, debug/release builds | PASS; state comparison `/tmp/inet-mqttsn-feature-restore.diff` is empty. Final builds and executions use this restored profile. |

## Mechanical checks and limits

Scoped architecture and naming, source-seal check for `src/inet/applications/mqttsn`, document seal/index check, release-note format and `git diff --check` pass. Raw logs are `/tmp/inet-mqttsn-scoped-architecture.log`, `scoped-naming.log`, `source-seals-final.log`, `document-seals-final.log` and `release-note-check.log`, all with the `inet-mqttsn-` prefix.

Repository-wide architecture, naming and interface checks exit 1 on unchanged files, including `common/socket/SocketMap.cc`, `common/IInterfaceRegistrationListener.h`, `routing/ospf_common` and `common/IPrintableObject.h`. They report no MQTT-SN paths. See `/tmp/inet-mqttsn-architecture-all.log`, `naming-all.log` and `interfaces.log` with the same prefix. These results do not make the repository-wide gates green. The C++ tidy gate exits 2 because `clang-tidy` is absent (`/tmp/inet-mqttsn-cpp.log`).

The broad `serializer_chunk_roundtrip.test` selection and serializer support build have compilation setup failures because their unconditional includes reference generated headers of disabled protocols (CsmaCaMac, DHCP, GPTP and IEEE 802.1AE). Raw logs are `/tmp/inet-mqttsn-serializer.out`, `.log` and `serializer-support.log`. The dedicated MQTT-SN codec/filler module passes; the broader suite is **NOT_VALIDATED**, not waived or labeled passing.

The skill-suite validator, packaging script and tests named in local `AGENTS.md` do not exist in this INET checkout. They remain unavailable rather than substituted from another repository. No skill package changes. Commit-series gates are run during publication preparation. The PR description gate and PR creation are NOT_RUN because the user canceled PR creation. Fingerprint, statistical and chart baselines were neither run nor modified. There is no external MQTT-SN implementation comparison or hardware/energy result.

These tests establish their selected production paths and helper invariants, not complete MQTT-SN conformance under every possible malformed message, timer race, memory failure or workload. The profile exclusions above and zero-id correlation limit remain part of the model contract.
