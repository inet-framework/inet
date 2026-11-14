# MAC feedback before neighbor expiry

`DsdvMacFailure.test` checks the optional link-layer failure path from Perkins/Bhagwat 1994 section 3. It runs three separated four-node lines: AckingMac with feedback enabled, the same routing setup with feedback disabled, and IEEE 802.11 ad hoc DCF with feedback enabled.

```mermaid
flowchart LR
    O[Observer] --- R[Router under test]
    R --- N[Moving next hop]
    N --- D[Destination]
```

Neighbors are initially 100 m apart with a 110 m radio range. At 2.5 s the router must have a two-hop route to the destination through the moving node. Ping traffic beginning at 1.5 s supplies real unicast data and populates the local ARP cache. At 3 s the next hop moves out of range while its interface stays up. The observer remains in range of the router.

At 3.6 s every line must have produced a real MAC failure. Both feedback-enabled routers must have removed routes to the lost neighbor and destination while preserving the unrelated observer route. The observer's UDP delivery signal must contain an actual received odd-sequence infinity advertisement for the destination. The timeout-only router must still retain its destination route at that point; by 12 s ordinary expiry must remove it. Its deliberately nonexistent `arpModule` path also proves that disabling feedback does not resolve the optional ARP dependency. Neighbor and destination timeouts are 8 s and 10 s, respectively.

The comparison uses owned IPv4 routes and common version-1 packet fields. When adapting this fixture to the parent, omit only the new `useMacFailureFeedback` assignments. The unchanged early-withdrawal assertions are expected to fail because the parent has no MAC-hint consumer. This is a source-grounded expectation until the exact comparison is executed.

Run from the repository root after a fresh debug build:

```sh
opp_run_protocol_tests --load . -p inet -m debug -w '(^|/)dsdv$' -f 'DsdvMacFailure\.test$'
```

The DCF case proves consumption of its existing retry-limit notification, not IEEE 802.11 conformance. A failure hint is not proof of physical disconnection under arbitrary interference. Delayed old-next-hop hints, unresolved identities, unrelated interfaces, repeated hints and lifecycle cancellation are covered by the associated module tests, not inferred from these three lines. Execution status belongs in the DSDV results record.
