IEEE 802.11 Compressed Block Ack extension points
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Select ``mac.hcf.blockAckVariant = "compressed"`` for immediate Compressed Block Ack. Basic remains the default. Both endpoints must select the same profile. ADDBA does not negotiate this selector. A valid addressed BAR with the other profile stops the simulation before agreement updates. The diagnostic identifies the peer, TID, local profile, and received profile. Configure the same selectors before a new run.

Compressed agreements require usable local HT support and directional peer HT state. An HT PHY mode alone does not supply that state. Without it, HCF retains Normal Ack DATA. Explicit ``blockAckVariant = "basic"`` retains historical Basic compatibility without that HT requirement. Compressed agreements suppress fragmentation for their peer and TID. Fragmentation outside those agreements retains its existing threshold.

Compressed supports immediate Block Ack only. Keep ``delayedAckPolicySupported = false`` in both agreement policies. HCF rejects either true value during initialization when Compressed support is enabled. Basic retains its existing delayed-policy configuration contract.

Custom originator QoS data services must implement ``IOriginatorQosMacDataService::setFragmentationPermission()``. HCF supplies the permission callback through this interface. The interface change preserves fragmentation behavior.

The service retains the callback and checks it before fragmentation. A false result suppresses fragmentation for that packet. An empty callback permits the configured fragmentation policy. The callback receives a borrowed packet; the service retains packet ownership.

Custom QoS rate selectors must implement ``computeCompressedBlockAckReqMode(packet, header, startsTxop)``. HCF supplies the request's actual position in the TXOP. Select a legal non-HT mode from the applicable BSS or peer rates. A configured ``controlFrameBitrate`` must satisfy that rule. The existing Basic rate-selection methods remain available.

Custom recipient ACK policies must implement ``computeCompressedBlockAckDurationField()``. This operation subtracts SIFS and the actual Compressed response airtime from the request reservation. Compressed Block Ack contains 32 bytes with FCS; Basic contains 152 bytes. Retain the existing Basic operation.

Custom recipient data services must accept ``bool interframe = false`` in ``dataFrameReceived()``. Rx establishes the local SIFS or permitted RIFS relation. HCF passes the frame's ``Ieee80211InterframeInd`` result through this boolean argument. An absent indication supplies false. Preserve that value through the recipient reorder path. Custom callers that omit it retain false.

The detector supports coverage class zero. An outside local gap supplies no interval evidence. A valid frame and its positive receipt can remain behind unresolved sequence gaps until BAR processing or another recovery action permits delivery. Custom ``IRx`` implementations must implement the history invalidation operation described in :doc:`ieee80211-local-interframe`.

Custom frame sequences must supply HCF's resolved profile to ``QoSContext``. Its constructor retains Basic for callers that omit the profile. Custom agreement creation must set the recipient agreement's variant before use. An agreement must retain that variant throughout its lifetime.

Custom frame sequences must preserve BAR arbitration before DATA preparation. The HCF preparation callback distinguishes the legacy path, accepted preparation, and admission refusal. Refusal must end the sequence without a DATA failure. Keep the selected DATA mode through RTS/CTS. Validate that decision without another adaptive rate query. Admit the complete exchange before source removal.

The active Compressed path reserves only the current exchange under single protection. A later BAR can require fresh contention. Before DATA commitment, failed RTS updates QSRC and the contention window without a provisional sequence counter. QSRC is the short retry counter for an access category. QSRC rollover does not discard queued DATA. Original lifetimes bound repeated RTS attempts.

Basic and outside-agreement fallback retain their existing preparation and recovery paths.

Master supplies stage S4 agreement lifecycle, borrowed DATA preview, BAR-first arbitration, and shared window operations. Custom extensions must retain those contracts when they add Compressed preparation and feedback.
