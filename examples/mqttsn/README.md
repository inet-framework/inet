# MQTT-SN examples

These configurations run a UDP sensor, MQTT-SN gateway, TCP MQTT broker and MQTT observer. Start from an INET shell with the `MqttSnExamples` feature and its dependencies enabled. From this directory run, for example:

```sh
inet --debug -u Cmdenv -c Qos1Loss -r 0
```

Every configuration uses seed 0 and ends at 8 seconds. Application payloads are hexadecimal bytes; `32332e35` and `32342e30` represent the text measurements 23.5 and 24.0, while command `01` is binary.

| Configuration | What to observe | Observer deliveries | Sensor deliveries |
| --- | --- | ---: | ---: |
| General | Wired QoS 0 in both directions | 2 | 1 |
| Qos1 | Acknowledged delivery and concrete downlink registration | 2 | 1 |
| Qos2 | Independent four-message exchanges on each hop | 1 | 1 |
| Qos1Loss | Sensor link has BER 1 from 1.15s to 1.35s; the lost publication is retried after the burst | 2 | 1 |
| SleepingSensor | Command queues while asleep and arrives after the poll | 0 | 1 |
| Connectionless | Predefined topic, QoS −1, no sensor CONNECT | 2 | 0 |
| Discovery | Local IPv4 discovery before the QoS 1 exchange | 2 | 1 |
| Wireless | The QoS 1 applications over an existing stationary Wi-Fi/IP model | 2 | 1 |

The scalar `deliveredMessages:count` on each application records these counts. Protocol tests check exact binary payloads; the examples provide runnable configurations and count/byte evidence. The wireless example uses IP and UDP, and sleep is application buffering. Neither configuration models a native ZigBee binding or radio energy savings.

```mermaid
flowchart LR
    S[Sensor] <-->|UDP MQTT-SN| G[Gateway]
    G <-->|TCP MQTT| B[Broker]
    B <-->|TCP MQTT| O[Observer]
```

See the [user guide](../../doc/src/users-guide/ch-mqtt-sn.rst) and [verification ledger](../../doc/project/evidence/model/mqttsn/coverage.md) for profile limits and execution results. Mermaid rendering is unverified.
