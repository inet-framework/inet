# MQTT model profile

> **Kind:** decision · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [protocol features](../../protocol/mqtt/features.md), [implementation plan](../../../../../plan/pending/mqtt.md)

The target is MQTT 3.1.1 clients and broker over TCP, with actual packet bytes, QoS 0–2, topic routing, persistent sessions, retained publications, keepalive and Wills. Broker storage is in memory; crash loses its stored state. There is no CPU, disk, authentication-service, TLS, WebSocket, clustering or broker capacity model. The initial identifier policy accepts 1–23 ASCII alphanumeric bytes. Offline QoS 0 storage is disabled; live overlapping subscriptions produce one copy at the highest matching granted QoS capped by publication QoS. QoS 2 uses Method B.

This is the final target, not a claim that unfinished stages conform. C1 adds only representation and framing. Later stages must update this profile and the coverage ledger to identify their actual supported behavior. No existing fingerprint/statistical baseline change is authorized.
