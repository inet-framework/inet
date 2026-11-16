# MQTT-SN: Connection admission

> **Kind:** report · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [plan](../../../plan/pending/mqtt-sn.md), [requirements](../../../doc/project/evidence/standard/mqtt-sn-v1.2/catalog.md)

C3 implements the selected profile of MQTT-SN v1.2 sections 6.2, 6.3, 6.12 and 6.13. The publication gateway commit's immediate parent already has the wire/session helpers and production client, but no MqttSnGateway C++/NED type. Its gateway fixtures therefore cannot build or initialize on that parent. This is a source-based missing-capability comparison, not an observed parent runtime failure. Client-only cases are controls already introduced by the client commit.

The production sensor sends CONNECT over UDP. The gateway establishes a distinct TCP MQTT connection and waits for broker CONNACK before accepting the sensor. Unsupported ProtocolId remains structurally decodable and is refused with code 3. A broker refusal maps to a single refusing CONNACK rather than an unrelated DISCONNECT.

The shared Ethernet fixture has sensor, second sensor, gateway, alternative gateway, broker and MQTT observer hosts, 100Mbps links with 1ms delay, static addresses and seed 0. Unused hosts have no application. MqttSnConnect checks MQTT acceptance precedes MQTT-SN acceptance. MqttSnConnectRetry uses a scripted gateway to withhold CONNACK, checks the original request and exactly two retries, then closes a quiet window. MqttSnBrokerRefusal uses a scripted TCP broker to accept the shared gateway connection and refuse the sensor. MqttSnMalformedInput sends unsupported ProtocolId followed by a valid connection. MqttSnIpv6 assigns static IPv6 addresses and checks accepted CONNACK source family and exact binary delivery in both directions.

MqttSnBrokerRefusal1–4 and the original refusal case cover all five MQTT refusal codes; MqttSnUnavailableBroker exercises failed TCP setup. MqttSnPartialConnack closes the first sensor connection after three CONNACK bytes, then requires refusal followed by a healthy new acceptance. MqttSnChangedPort verifies that the obsolete UDP endpoint cannot register a topic after the ClientId moves. MqttSnDuplicateConnect repeats CONNECT both during setup and after acceptance and counts exactly one upstream sensor CONNECT.

Every positive event has a deadline. Scripted-peer expectations check message type and relevant fields in order; completion is asserted before simulation end. These are independent connection scenarios, not every possible simultaneous socket/UDP failure ordering.

```mermaid
sequenceDiagram
    participant S as Sensor
    participant G as Gateway
    participant B as MQTT broker
    S->>G: UDP CONNECT
    G->>B: TCP establishment, MQTT CONNECT
    B-->>G: MQTT CONNACK
    G-->>S: MQTT-SN CONNACK (mapped result)
```

Execution results, commands and remaining coverage limits belong to the [coverage ledger](../../../doc/project/evidence/model/mqttsn/coverage.md). Diagram rendering is unverified. The role-based publication trees and their directly executed checks are recorded in the ledger. The original C1–C9 capability labels are not separate historical commits.
