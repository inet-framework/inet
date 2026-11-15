# MQTT verification coverage

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [model profile](conformance.md), [protocol checks](../../protocol/mqtt/checks.md)

Implementation started from `2d084b9f2b0ab8cb1ed154b3c92838a30225bba5`. The user authorized execution of the whole plan in the conversation. The pre-existing `AGENTS.md` edit is excluded. This ledger records executed evidence separately from the plan's expectations.

| Area | Test target | Status |
| --- | --- | --- |
| Packet representation over TCP | MqttWireFormat.test | NOT_RUN |
| Framing and UTF-8 boundaries | mqtt_framing.test | NOT_RUN |
| Serializer round trips | serializer_chunk_roundtrip.test | NOT_RUN |
| Connection admission and QoS 0 routing | MqttConnect.test, MqttRouting.test | NOT_RUN |
| QoS 1 and sessions | MqttQos1.test, MqttSession.test | NOT_RUN |
| QoS 2 and recovery | MqttQos2.test, MqttQos2Reconnect.test | NOT_RUN |
| Retained values, keepalive and Wills | MqttRetained.test, MqttKeepAlive.test, MqttWill.test | NOT_RUN |
| Lifecycle and malformed exchanges | MqttLifecycle.test, MqttFailureHandling.test, mqtt_lifecycle.test | NOT_RUN |
| Reference exchanges and feature-off builds | C7 verification | NOT_RUN |

Raw build and runner logs are retained in `/tmp/inet-mqtt-evidence/` for this execution. This path is transient and must not substitute for summarized results, exact commands and limitations committed here. `clang++ -std=c++17 -fsyntax-only -Isrc -I$OMNETPP_ROOT/include -DOMNETPPLIBS_IMPORT -DINET_EXPORT src/inet/applications/mqtt/*.cc` passed after generated MSG compilation. This establishes source/API compatibility only. Initial default-feature compilation was interrupted in favor of `opp_featuretool disable -f all`, `opp_featuretool enable -r Mqtt MqttTests`, then `make MODE=debug -j4`; that focused build is in progress. No runtime pass is recorded yet.
