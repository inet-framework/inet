# MQTT features

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards](standards.md)

| Capability | Governing MQTT 3.1.1 sections |
| --- | --- |
| Control packet representation and UTF-8 | 1.5, 2, 3 |
| Admission, client identifiers and credentials | 3.1, 3.2 |
| Publish and subscribe routing | 3.3, 3.8–3.11, 4.7 |
| QoS 0 | 4.3.1 |
| QoS 1 and its duplicate behavior | 3.4, 4.3.2 |
| QoS 2 and duplicate suppression | 3.5–3.7, 4.3.3 |
| Persistent sessions and delivery retry | 3.1.2.4, 3.2.2, 4.1, 4.4 |
| Ordered delivery and acknowledgement ordering | 4.6 |
| Retained values and subscription replay | 3.3.1.3, 3.8.4 |
| Keepalive, pings and Wills | 3.1.2.5–3.1.2.10, 3.12–3.14 |
| Malformed packet handling and network closure | 4.8 |

This feature map describes the protocol independently of a simulation implementation. Implementation status belongs in the model coverage ledger.
