# MQTT reference exchange comparison

> **Kind:** report · **Status:** snapshot 2026-11-15 · **Seal:** none · **Owns:** — · **Stands on:** [wire checks](wire-format.md), [coverage ledger](../../../doc/project/evidence/model/mqtt/coverage.md)

The reference is Mosquitto 2.0.22 with its own `mosquitto_pub` and `mosquitto_sub`, explicitly selecting MQTT 3.1.1 with `-V mqttv311`. Debian packages were downloaded and extracted under `/tmp/inet-mqtt-evidence/mosquitto-packages`; no system daemon or INET production dependency was installed. A local TCP proxy records complete control packets in each direction. [reference-exchanges.json](reference-exchanges.json) contains the version, exact client arguments and all 12 connections. [capture-reference.py](capture-reference.py) reproduces the capture and generates the 30 distinct packet vectors in [MqttReferenceVectors.h](MqttReferenceVectors.h).

```sh
LD_LIBRARY_PATH=/tmp/inet-mqtt-evidence/mosquitto-packages/extracted/usr/lib/x86_64-linux-gnu \
python3 tests/protocol/mqtt/capture-reference.py \
    --prefix /tmp/inet-mqtt-evidence/mosquitto-packages/extracted \
    --output tests/protocol/mqtt/reference-exchanges.json \
    --vectors tests/protocol/mqtt/MqttReferenceVectors.h
```

The broker listens only on the chosen loopback port with its default anonymous local policy. Ports are selected dynamically; the exact ports of this capture remain in the JSON commands. Per-direction packet order is preserved. The proxy does not record a total order between opposite directions or TCP headers, and this evidence makes no TCP timing claim.

| Reference connections, zero-based | Observation | Matching INET fixture |
| --- | --- | --- |
| 0–5 | CONNECT carries protocol name MQTT and level 4; accepted clean CONNACK has Session Present false. SUBACK grants 2. QoS 0 has no identifier, QoS 1 uses PUBACK, QoS 2 uses PUBREC, PUBREL byte `62`, PUBCOMP. Each publication carries byte `41`. | MqttWireFormat, MqttConnect, MqttQos1, MqttQos2 |
| 6–7 | A retained QoS 1 publication carries byte `52`; late subscription receives it with RETAIN true and acknowledges its recipient identifier. | MqttRetained, MqttBrokerPersistence |
| 8–11 | Clean Session false first creates a session, then reconnect reports Session Present true. Offline byte `51` is delivered at QoS 1. In connection 11 the queued PUBLISH appears after CONNACK and before SUBACK. | MqttSession, MqttClientRecovery, MqttBrokerRecovery |

The codec check decodes and re-encodes every captured vector byte for byte. The production endpoint fixtures independently assert these exchange rules, flags, acknowledgement identifiers and handoff counts. Identifiers are compared within an exchange: Mosquitto's choice of identifier 1 or 2 is not imposed on INET's allocator. Topic names, client identifiers, payload fixtures and configured keepalive differ between the scenarios; those are workload inputs. Mosquitto advertises keepalive 60 in this capture, while the INET normal-path fixtures use 0. Binary-zero and `ff` payload preservation is checked separately by INET's golden vectors and routing cases.

Both implementations permit an offline delivery immediately after accepted CONNACK, without waiting for a new subscription acknowledgement. The INET client accepts a valid unsolicited PUBLISH and acknowledges it independently of subscription reconciliation. This avoids imposing a SUBACK ordering that the reference does not promise.

This comparison covers normal connections, all three delivery QoS levels, retained replay and persistent offline delivery. It is an offline wire comparison; it does not establish live interoperability, malformed-input equivalence, broker performance or durable storage. The failure/recovery tests derive their expectations from MQTT 3.1.1, not from this implementation's behavior.
