# MQTT verification coverage

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [model profile](conformance.md), [protocol checks](../../protocol/mqtt/checks.md)

The implemented MQTT 3.1.1 profile passes the 24-case protocol suite and five focused module cases. Debug/release compilation, an MQTT-disabled build, two existing TCP controls, five examples and the user-guide build also complete. The full repository serializer suite has an unrelated support-library setup failure; the same round-trip engine passes all 15 MQTT variants in isolation. Global architecture, naming and interface findings are identical to the pinned upstream baseline.

Execution took place on 2026-11-15–16 in `/home/miguel/workspace/inet-mqtt`, branch `feat/mqtt`. The source and verification revision is `224611ccfa88db2bfa240d87464d1ebbf42d247a`; the final documentation/examples commit adds no compiled MQTT source change. The series starts above upstream `91219b0fa7d2794ee67ac8a3dd45c3a09cfc45b0`; its original plan commit is `2d084b9f2b0ab8cb1ed154b3c92838a30225bba5`. The pre-existing `AGENTS.md` edit is excluded. Results describe the final integrated files, not separately rerun historical stages after fixture backports.

| Area | Test target | Status |
| --- | --- | --- |
| Packet representation and independent vectors | MqttWireFormat.test | PASS |
| Framing, UTF-8, length boundaries and packet inspection | mqtt_framing.test | PASS |
| Registered serializer, 15 variants with field/class coverage gates | mqtt_serializer.test | PASS |
| Global serializer suite | serializer_chunk_roundtrip.test | SETUP ERROR before execution |
| Default/configured connection and routing | MqttDefaults.test, MqttConnect.test, MqttRouting.test | PASS |
| Admission, wildcard and malformed-input matrices | MqttBoundaryCases.test, MqttInvalidServer.test, MqttFailureHandling.test | PASS |
| QoS 1, offline order and offline QoS 0 exclusion | MqttQos1.test, MqttSession.test | PASS |
| All 65,535 identifiers and restoration progress | MqttIdentifierExhaustion.test | PASS |
| Normal QoS 2, mixed subscriber QoS and completed identifier reuse | MqttQos2.test, MqttQos2Reconnect.test | PASS |
| Both production sender roles before PUBREC and after PUBREC | MqttQos2PublishRetry.test, MqttClientRecovery.test, MqttBrokerRecovery.test | PASS |
| Retained values, idle pings and abrupt-close Will | MqttRetained.test, MqttKeepAlive.test, MqttWill.test | PASS |
| Exact expiry/renewal, partial traffic, zero keepalive and missing response | MqttBrokerLiveness.test, MqttMissingPingResponse.test | PASS |
| Link partition and delayed failure detection | MqttPartitionWill.test | PASS |
| Stop/start, crash reset, broker persistence and retained-state loss | MqttLifecycle.test, MqttClientCrash.test, MqttBrokerPersistence.test | PASS |
| Identifier wrap/order, 17 invalid parameter configurations and timer ownership | mqtt_sessions.test, mqtt_parameters.test, mqtt_lifecycle.test | PASS |
| External reference | Mosquitto 2.0.22, 12 connections, 30 distinct wire vectors | CAPTURED and compared |
| Compilation | MQTT-on debug/release, MQTT-off debug | PASS |
| User-facing artifacts | Five examples and user-guide HTML | PASS, two inherited cross-manual warnings |

The [requirement map](requirements.md) assigns all 139 catalog identifiers, including the four WebSocket requirements outside this profile. [Reviewer documents](../../../../../tests/protocol/mqtt/wire-format.md) describe topology, stimulus, observations and limits. The [reference comparison](../../../../../tests/protocol/mqtt/reference-comparison.md) owns the external capture provenance and interpretation. The [self-review](review.md) records the semantic checklist.

## Reproduction and artifacts

The environment uses OMNeT++ `6.4.0aipre2`, Clang, and the existing INET launchers. Run these commands from the checkout root after `source .envrc`. All simulations use configuration General, run 0 and seed 0 unless an example configuration is named. The wired protocol topology uses 1ms Ethernet links; the partition case uses the shared tap topology.

The final feature-on profile enables `Ethernet`, `Ieee802`, `Ieee80211`, `Ieee8021d`, `Ieee8021q`, `Ieee8022`, `Ipv4`, `Ipv6`, `LinkLayerConfiguratorsCommon`, `Loopback`, `Mqtt`, `MqttExamples`, `MqttTests`, `PhysicalLayerCommon`, `PhysicalLayerWirelessCommon`, `ProtocolElement`, `Queueing`, `TcpCommon` and `TcpInet`. Product `Mqtt` depends only on `TcpCommon`. The test feature includes the extra dependencies of the repository's shared protocol/module support libraries, which otherwise fail on wireless, IPv6 or VLAN symbols.

```sh
opp_featuretool disable -f all
opp_featuretool enable -r MqttTests MqttExamples
make MODE=debug -j4
make MODE=release -j4

opp_run_protocol_tests --load . -p inet -m debug --no-build \
    -w '^tests/protocol/mqtt$' -f 'Mqtt.*[.]test$' --no-concurrent \
    -l INFO -L INFO --log-file /tmp/inet-mqtt-evidence/mqtt-complete-runner.log

opp_run_module_tests --load . -p inet -m debug --no-build \
    -f 'mqtt_.*[.]test$' --no-concurrent \
    -l INFO -L INFO --log-file /tmp/inet-mqtt-evidence/modules-final-pass-runner.log

opp_run_protocol_tests --load . -p inet -m debug --no-build \
    -w '^tests/protocol/(self|tcp)$' \
    -f '(TcpTestAppSockets|Rfc9293DataTransfer)[.]test$' --no-concurrent \
    -l INFO -L INFO --log-file /tmp/inet-mqtt-evidence/tcp-final-runner.log
```

Each final command selected exactly 24, 5 and 2 cases respectively; no case was skipped or expected to fail. Exit status is 0 for successful builds and runners. `--no-build` skips the INET library rebuild, so the matching fresh library is built explicitly; support libraries and fixtures are still built by the runners. The exhaustive identifier case is deliberately expensive and sends/retries 65,535 publications. Its first ACK releases identifier 2 for a restoration SUBSCRIBE; SUBACK then releases that identifier for the queued publication, proving progress through the original stall boundary.

The MQTT-off build ran in an isolated detached worktree, with the current feature descriptor and example NED files overlaid. Its profile contains `Ethernet`, `Ieee8021d`, `Ieee8022`, `Ipv4`, `LinkLayerConfiguratorsCommon`, `Loopback`, `PhysicalLayerCommon`, `ProtocolElement`, `Queueing`, `TcpCommon` and `TcpInet`. `Mqtt`, `MqttTests` and `MqttExamples` are disabled. The debug library builds and contains no `inet::mqtt` dynamic symbols. The active checkout's feature-on state was not changed for this check.

```sh
opp_featuretool disable -f all
opp_featuretool enable -r TcpInet Ipv4 Ethernet Loopback LinkLayerConfiguratorsCommon
make makefiles
make MODE=debug -j6
```

From `examples/mqtt`, each of General, Qos1, Qos2, Retained and PersistentReconnect completed to 8s with exit status 0:

```sh
inet --debug -u Cmdenv -c General --cmdenv-express-mode=true \
    --cmdenv-event-banners=false --'**.cmdenv-log-level=warn'
```

The subscriber recorded, respectively, 3, 2, 1, 1 and 2 handoffs, totaling 12, 8, 4, 4 and 8 payload bytes. These are example outputs; deterministic protocol assertions establish the behavioral claims.

The user guide builds using the repository launcher and an isolated documentation tool environment:

```sh
PATH=/tmp/inet-mqtt-doc-venv/bin:$PATH \
PYTHONPATH=/tmp/inet-mqtt-doc-builder/extensions ENGINE=none \
doc/src/build --only users-guide --no-fonts --jobs 2 \
    --out /tmp/inet-mqtt-doc-site --cache /tmp/inet-mqtt-doc-cache
```

This produces the MQTT HTML chapter. The two warnings concern existing links to the migration/developer manuals, which were not part of the selected user-guide build. No MQTT chapter warning remains.

## Gates and limits

Scoped MQTT architecture/naming checks, source-seal and document-seal checks pass. The series wrapper checks commit classification, includes, NED parameters and release notes. Repository-wide architecture, naming and interface checks still fail; running the same checks on clean upstream `91219b0fa7d2794ee67ac8a3dd45c3a09cfc45b0` produced exactly the same finding sets: 36 architecture violation/candidate lines, 24 naming violations and 16 interface-class violations. No new MQTT finding or exception-ledger change is introduced.

```sh
doc/project/enforcement/check-architecture.sh src/inet/applications/mqtt
doc/project/enforcement/check-naming.sh src/inet/applications/mqtt
doc/project/enforcement/check-source-seals.sh --base origin/master
doc/project/enforcement/check-seals.sh
doc/project/enforcement/check-pull-request.sh --base origin/master
```

The attempted global serializer command, from `tests/serializer`, is:

```sh
opp_run_opp_tests --load ../.. -p inet -m debug --no-build \
    -f 'serializer_chunk_roundtrip[.]test$' --no-concurrent
```

It exits 1 while compiling the pre-existing gPTP filler: with Clock disabled, `ClockEvent` is declared as both a class and a `cMessage` alias. No global serializer runtime result is claimed. The focused MQTT case reuses that suite's engine and actual MQTT fillers without compiling unrelated fillers; it passes 15 byte/length round trips, with zero unexpected/stale field gaps and zero unexpected/stale serializer-class skips.

Raw logs remain under `/tmp/inet-mqtt-evidence/`: `mqtt-complete-run.log`, `modules-final-pass-run.log`, `tcp-final-run.log`, `debug-final-build.log`, `release-final-confirm-build.log`, `feature-off-build.log`, `feature-off-confirm-build.log`, `user-guide-complete-build.log`, `example-*-final.log`, the gate/baseline logs, and `serializer-run.log`. Generated per-case outputs live in each test runner's ignored `work/` directories; example results live in its ignored `results/` directory. These paths are transient; this ledger and the committed reference vectors preserve the conclusions and reproduction commands.

Initial setup/fixture failures were corrected before the final passes: missing NED class declarations, test namespace qualification, shared-support feature dependencies, TCP delayed-ACK timing and first-occurrence versus count assertions. They are not declared expected failures. Mermaid sources were reviewed but rendering remains UNVERIFIED because no renderer is installed. The skill-package validator paths supplied in `AGENTS.md` do not exist in this checkout; no skill/package change was made. No existing fingerprint/statistical baseline was changed or refreshed. There is no live interoperability, TLS, durable-storage, performance or exhaustive packet-permutation claim.
