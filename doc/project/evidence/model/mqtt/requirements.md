# MQTT requirement-to-check map

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standard catalog](../../standard/mqtt311/catalog.md), [model profile](conformance.md), [execution ledger](coverage.md)

Every one of the catalog's 139 normative identifiers is assigned below by its clause prefix. A row includes every identifier listed under that prefix in the catalog, not unlisted numbers in a numerical range. The four WebSocket requirements are outside the TCP profile. The other rows name the production owner and the checks that exercise its relevant normal, boundary or failure behavior. This is an assignment map; executed verdicts come from the coverage ledger. A passing test is evidence for its stated assertions, not a claim that every possible packet permutation was exercised.

Test names below omit `.test`. `framing`, `serializer`, `sessions` and `lifecycle` mean the respective `tests/module/mqtt_*.test` helpers. The remaining cases are under `tests/protocol/mqtt/`.

| Catalog clause prefix | Production owner | Direct checks |
| --- | --- | --- |
| MQTT-1.5.3 | MqttPacketSerializer UTF-8 validation and unchanged text bytes | framing, MqttBoundaryCases, MqttWireFormat |
| MQTT-2.2.2 | MqttPacketSerializer fixed-header flags | framing, MqttBoundaryCases, MqttInvalidServer |
| MQTT-2.3.1 | MqttSession allocation and endpoint acknowledgement handlers | sessions, MqttIdentifierExhaustion, MqttQos1, MqttQos2, MqttClientRecovery, MqttBrokerRecovery |
| MQTT-3.1.0 | Broker waiting/active phases | MqttConnect, MqttBoundaryCases |
| MQTT-3.1.2 | CONNECT codec, connection/session lifetime, Will and liveness owners | serializer, framing, MqttSession, MqttClientCrash, MqttBrokerPersistence, MqttBrokerLiveness, MqttMissingPingResponse |
| MQTT-3.1.3 | CONNECT codec and broker identifier/authentication policy | serializer, framing, MqttBoundaryCases |
| MQTT-3.1.4 | Broker admission, refusal and identifier takeover | MqttConnect, MqttBoundaryCases, MqttBrokerLiveness |
| MQTT-3.2.0 | Broker first response and client waiting phase | MqttConnect, MqttInvalidServer, MqttSession |
| MQTT-3.2.2 | CONNACK codec and session-present reconciliation | framing, MqttInvalidServer, MqttSession, MqttClientRecovery, MqttClientCrash |
| MQTT-3.3.1 | Publication codec, sender retry, retained records and routing QoS | serializer, framing, MqttQos2, MqttRetained, MqttBrokerPersistence, MqttClientRecovery, MqttBrokerRecovery |
| MQTT-3.3.2 | Topic validation and independent sender identifiers | MqttBoundaryCases, MqttQos1, MqttQos2, MqttClientRecovery |
| MQTT-3.3.4 | Broker matching and recipient handoff | MqttRouting, MqttBoundaryCases, MqttQos1, MqttQos2 |
| MQTT-3.3.5 | Endpoint application handoff and opaque payload treatment | MqttRouting, MqttQos1, MqttQos2, MqttClientRecovery |
| MQTT-3.6.1 | PUBREL fixed flags | framing, MqttClientRecovery, MqttBrokerRecovery, MqttQos2Reconnect |
| MQTT-3.8.1 | SUBSCRIBE fixed flags | framing, MqttBoundaryCases |
| MQTT-3.8.3 | Filter/QoS list decoding and validation | serializer, framing, MqttBoundaryCases |
| MQTT-3.8.4 | Broker subscription replacement, retained replay and grant selection | MqttBoundaryCases, MqttBrokerPersistence, MqttRetained, MqttQos2 |
| MQTT-3.9.3 | SUBACK result order, count and grant validation | MqttBoundaryCases, MqttInvalidServer |
| MQTT-3.10.1 | UNSUBSCRIBE fixed flags | serializer, framing |
| MQTT-3.10.3 | UNSUBSCRIBE list decoding | serializer, framing, MqttRouting |
| MQTT-3.10.4 | Exact subscription removal and UNSUBACK | MqttRouting, MqttBoundaryCases |
| MQTT-3.12.4 | Broker PINGREQ response | MqttKeepAlive, MqttBrokerLiveness |
| MQTT-3.14.1 | DISCONNECT fixed flags | serializer, framing |
| MQTT-3.14.4 | Client terminal send and broker Will suppression | MqttSession, MqttLifecycle, MqttBrokerLiveness |
| MQTT-4.1.0 | Session-owned exchanges and broker-owned retained records | MqttSession, MqttClientRecovery, MqttBrokerRecovery, MqttBrokerPersistence, MqttClientCrash |
| MQTT-4.3.1 | QoS 0 send/handoff without acknowledgement state | MqttRouting, MqttQos2 |
| MQTT-4.3.2 | QoS 1 sender retention, receiver acknowledgement and reuse | MqttQos1, MqttBoundaryCases, MqttClientRecovery, MqttBrokerRecovery |
| MQTT-4.3.3 | QoS 2 Method B receiver record and sender phase transition | MqttQos2, MqttQos2Reconnect, MqttQos2PublishRetry, MqttClientRecovery, MqttBrokerRecovery, sessions |
| MQTT-4.4.0 | Persistent sender retry on reconnect | MqttClientRecovery, MqttBrokerRecovery, MqttQos2PublishRetry, MqttIdentifierExhaustion |
| MQTT-4.5.0 | Endpoint validation and handling of publications independently of subscription reconciliation | MqttBoundaryCases, MqttInvalidServer, MqttClientRecovery |
| MQTT-4.6.0 | Session send-order queue and synchronous receive acknowledgement order | MqttSession, MqttQos1, MqttQos2, MqttClientRecovery, sessions |
| MQTT-4.7.1 | Topic/filter wildcard syntax | MqttBoundaryCases, MqttRouting |
| MQTT-4.7.2 | Leading dollar namespace rule | MqttBoundaryCases |
| MQTT-4.7.3 | Literal, case-sensitive UTF-8 topic levels and empty levels | MqttBoundaryCases, framing |
| MQTT-4.8.0 | Framer validation and endpoint protocol-error closure | MqttFailureHandling, MqttBoundaryCases, MqttInvalidServer, MqttBrokerLiveness |
| MQTT-6.0.0 | Outside profile: no WebSocket transport | Not applicable to these four conditional requirements |
| MQTT-7.0.0 | Separate client/server roles, standard wire format with no required extensions | MqttConnect, MqttWireFormat; bridge behavior is outside profile |
| MQTT-7.1.1 | Broker consumes the existing ordered TCP stream | MqttWireFormat and the two existing TCP controls in the execution ledger |
| MQTT-7.1.2 | Client consumes the existing ordered TCP stream | MqttWireFormat and the two existing TCP controls in the execution ledger |

Remaining Length is also checked explicitly despite having no catalog identifier: incomplete prefixes, concatenation, digit transitions and the maximum legal prefix are in `framing`. Ordinary stop/crash policy and timer ownership are additional declared model contracts, covered by the lifecycle protocol/module cases rather than attributed to a durable-storage requirement.
