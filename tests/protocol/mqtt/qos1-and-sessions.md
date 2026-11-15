# MQTT QoS 1 and sessions

> **Kind:** report · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [implementation plan](../../../plan/pending/mqtt.md)

C3 adds MQTT 3.1.1 sections 3.1.2.4, 3.4 and 4.3.2–4.6. C2 has only transient QoS 0 operation. Session-owned publication queues and outgoing acknowledgements now survive eligible transport disconnection, with independent identifier allocation per sender. A broker's subscriber delivery has its own acknowledgement; it never borrows the incoming publisher's identifier.

`MqttQos1.test` publishes binary `00 ff 41` through a production publisher, broker and subscriber, observes the exact payload handoff and its PUBACK, and requires no second delivery during a one-second fault-free window. This normal-path case does not impose exactly-once semantics on QoS 1 under failure.

`MqttSession.test` connects a scripted subscriber with Clean Session false, installs `factory/#` at QoS 1, then sends DISCONNECT and closes before publication. A production publisher sends `41` while it is offline. At 2s the subscriber reconnects with the same identifier; CONNACK must report Session Present and a queued QoS 1 PUBLISH must contain `41`. Each exchange has a one-second deadline; seed 0 and 1ms Ethernet links make the setup deterministic. C2 would not admit this persistent session or deliver its queued publication.

```mermaid
sequenceDiagram
    participant S as Persistent subscriber
    participant B as Broker
    participant P as Publisher
    S->>B: Subscribe and disconnect
    P->>B: QoS 1 publication
    Note over B: Queue publication in subscriber session
    S->>B: Reconnect with the same identifier
    B->>S: CONNACK with Session Present
    B->>S: Queued PUBLISH with recipient identifier
```

`MqttClientRecovery.test` and `MqttBrokerRecovery.test` interrupt both production sender roles before PUBACK. Each resumed PUBLISH must retain its original identifier and set DUP. The receiver may hand a QoS 1 retransmission onward again; these tests make no exactly-once claim for that QoS.

`MqttIdentifierExhaustion.test` leaves all 65,535 sender identifiers outstanding, then queues one more publication. A persistent reconnect with Session Present false must retry the old messages despite having no identifier available for subscription restoration. ACK of identifier 2 permits restoration SUBSCRIBE; its SUBACK releases the identifier again and must flush the queued publication. The test checks the full identifier set, absence of zero and original/retry counts. Workload publications are spaced by 100 microseconds to let TCP drain while MQTT acknowledgements remain withheld. A failure to flush after SUBACK was found and corrected by this probe. Execution status is in the [coverage ledger](../../../doc/project/evidence/model/mqtt/coverage.md). Rendering is unverified.
