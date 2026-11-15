# MQTT retained values keepalive and Wills

> **Kind:** report · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [implementation plan](../../../plan/pending/mqtt.md)

C5 implements retained publication state, connection-owned Wills and liveness deadlines under MQTT 3.1.1 sections 3.1.2.5–3.1.2.10, 3.3.1.3 and 3.12–3.14. C4 has QoS exchanges but none of these features. Retained records live outside session maps, and Wills are removed on DISCONNECT or published once when the owning connection closes abnormally.

`MqttRetained.test` stores `41` before a subscriber connects at 1s. The subscriber must receive a retained PUBLISH. A zero-byte retained publication at about 2.1s must be forwarded live with RETAIN false; an observer subscribing at 3s must receive no stale replay during a two-second window. The test checks flags and payload lengths, not only message counts.

`MqttKeepAlive.test` starts an idle production client with keepalive 2s. It requires PINGREQ within 3s, PINGRESP within one second, and no disconnected transition over the following two seconds. Client and broker timers run in their production owners; the client send boundary is the TCP handoff, as documented in the model profile.

`MqttWill.test` connects a scripted publisher with Will topic `factory/status` and byte `ff`, then closes TCP without DISCONNECT. A production subscriber must receive exactly one live publication with that topic/payload and RETAIN false. One-second exchange deadlines and a final absence window bound the observation. All three fixtures use seed 0 and 1ms wired links.

```mermaid
sequenceDiagram
    participant C as Client
    participant B as Broker
    participant S as Subscriber
    C->>B: CONNECT with Will
    B->>C: CONNACK
    alt Valid DISCONNECT
        C->>B: DISCONNECT
        Note over B: Suppress Will
    else Abnormal closure
        Note over C,B: Detect failure or expiry
        B->>S: Publish Will once
    end
```

These initial cases do not establish all planned timeout boundaries, zero keepalive, missing PINGRESP, partition, takeover, retained QoS combinations or graceful Will suppression. Additional probes and their results must remain explicit in the coverage ledger. Diagram rendering is unverified.
