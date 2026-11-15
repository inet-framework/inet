# MQTT model profile

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [protocol features](../../protocol/mqtt/features.md), [implementation plan](../../../../../plan/pending/mqtt.md)

The model provides MQTT 3.1.1 clients and broker over TCP, with actual packet bytes, QoS 0–2, topic routing, persistent sessions, retained publications, keepalive and Wills. Broker storage is in memory; crash loses its stored state. There is no CPU, disk, authentication-service, TLS, WebSocket, clustering or broker capacity model. The identifier policy accepts 1–23 ASCII alphanumeric bytes. Offline QoS 0 storage is disabled; live overlapping subscriptions produce one copy at the highest matching granted QoS capped by publication QoS. QoS 2 uses Method B.

`MqttClient` occupies an ordinary host application slot and executes an ordered XML workload. The broker optionally compares an exact username/password pair; this is a simulation admission policy over unencrypted TCP. Persistent client/broker state survives ordinary application stop/start; client crash resets local state and broker crash clears all broker state. The first accepted connection after client crash requests Clean Session true before configured persistent operation resumes.

Client keepalive measures handing a complete control packet to TCP. Broker deadlines renew only when the framer supplies a complete control packet. Keepalive zero disables this mechanism. Connection, ping-response and graceful-stop timeouts are positive configuration bounds, with five-second defaults. New accepted sockets and delayed transport indications are owned and cleaned by their application; stop timeout preserves eligible in-memory session state.

These are the implemented choices. The [coverage ledger](coverage.md) owns the executed verdicts and infrastructure limitations; this profile is not a claim of exhaustive external interoperability. No existing fingerprint/statistical baseline change is authorized or required.
