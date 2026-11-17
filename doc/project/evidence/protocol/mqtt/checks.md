# MQTT checks

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features](features.md)

For each packet type compare the encoded fields and lengths against independently specified bytes. Split short control packets at every byte boundary; incomplete prefixes must produce no packet. Concatenate packets and require one decode per complete packet. Reject reserved flags, invalid lengths and prohibited UTF-8 encodings.

Establish a connection, subscribe to exact and wildcard filters, and publish distinct binary payloads. Observe the recipients and granted QoS, then unsubscribe and verify bounded absence of later delivery. Include empty topic levels, leading dollar namespaces and invalid filters.

Interrupt QoS 1/2 exchanges before their terminal acknowledgement. Resume eligible sessions and require the appropriate retry and identifier, distinguishing permissible QoS 1 duplicates from prohibited duplicate QoS 2 handoffs. Verify both delivery directions, identifier reuse/exhaustion and order on one topic.

Publish a retained value before subscription and require replay with the proper retain flag. Replace and delete the record; session deletion must not delete a retained value. Contrast graceful disconnect with abnormal closure, timeout and client identifier takeover. Verify Will payload, QoS and retain semantics only after failure detection. Measure keepalive from the defined send/receive boundaries, including zero and partial packet reception.
