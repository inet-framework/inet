# MQTT-SN features

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `MQTTSN-F-*` · **Stands on:** [requirement catalog](../../standard/mqtt-sn-v1.2/catalog.md)

This inventory describes protocol behavior independently of implementation class names. The [coverage ledger](../../model/mqttsn/coverage.md) distinguishes planned behavior from execution evidence.

| Feature | Behavior | Catalog rows |
| --- | --- | --- |
| MQTTSN-F-WIRE | Compact/extended messages, exact field encodings, binary payloads and malformed bounds. | WIRE, FIELDS, all WIRE rows |
| MQTTSN-F-CONNECT | Gateway admission/refusal, clean/persistent relationships and disconnect. | CONNECT, SESSION, DISCONNECT |
| MQTTSN-F-TOPICS | Registration, normal/predefined/short topics, wildcard subscriptions and retained delivery. | REGISTER, SUBSCRIBE, TOPIC-KINDS, GATEWAY-PUBLISH |
| MQTTSN-F-DELIVERY | QoS −1/0/1/2, identifiers, duplicate handling and phase-specific errors. | CLIENT-PUBLISH, CONNECTIONLESS, PUBLISH-WIRE |
| MQTTSN-F-RETRY | Timed unicast retransmission, congestion wait and retry exhaustion. | RETRY |
| MQTTSN-F-WILL | Negotiation, replacement, deletion and publication on sensor loss. | WILL-SETUP, WILL-UPDATE, SESSION |
| MQTTSN-F-SLEEP | Buffered downlink delivery, polling, interrupted exchanges and loss supervision. | SLEEP, PING-WIRE, LIVENESS |
| MQTTSN-F-DISCOVERY | Advertisement, search/response suppression, expiry and gateway change. | DISCOVERY-WIRE, DISCOVERY, RECOMMENDATIONS |
