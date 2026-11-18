# AODV — behavioral checks

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features.md](features.md)

Observe control through UDP/IP, forwarding through actual output, and route state through the
public routing table. An arithmetic helper or a simulation fingerprint alone cannot demonstrate
message acceptance or packet release. Exact timer checks use deterministic delivery and test both
event orders at a shared deadline.

| Feature | Stimulus and required observation |
| --- | --- |
| AODV-F-FRESHNESS | Inject wrapping, equal, stale and unknown sequence information separately in requests, replies, Hellos and errors. Inspect the selected route and whether a reply was relayed. |
| AODV-F-LIFETIME | Inject a nonimproving request and a gratuitous reply late in simulation. Observe independent reverse expiry refresh and remaining whole-millisecond advertisements. |
| AODV-F-DISCOVERY | Exceed one request window with finite demand. Observe FIFO admission, full reply intervals, expanding rings, network-wide backoff, cancellation and terminal disposal. |
| AODV-F-LINK | Validate two neighbors concurrently, fail only one, and replace an in-flight path before reporting failure. Observe only attributable link effects. Incoming transit data refreshes its immediate sender's Hello deadline. |
| AODV-F-ERROR | Inject ordinary and N errors from matching and unrelated links. Observe invalidation versus preservation, prefix ambiguity suppression and complete recipient delivery for admitted partitions. |
| AODV-F-RECOVERY | Restart with pending work and learn a route during recovery. Observe held data, permitted ACK/errors, deadline extension and cleanup. |
| AODV-F-INTERFACE | Discover a secondary local address and return a reply on another selected link. Observe correct source, next hop and interface, with no state changes from unselected control ingress. |
| AODV-F-SUBNET | Combine aggregate and host records, independent counters and pending demand. Observe consistent eligibility for new/queued data and prefix-owned errors. |
| AODV-F-REPAIR | Enable repair for broken transit paths. Observe one bounded transaction, shared member queues, fresh matching-prefix completion, longer-path notification and single terminal cleanup. |

The implementation plan assigns concrete fixtures and case variants. Model-specific restrictions
must be stated alongside results rather than presented as additional RFC requirements.
