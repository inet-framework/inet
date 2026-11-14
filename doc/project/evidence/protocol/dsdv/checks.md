# DSDV checks and test scenarios

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features](features.md), [source and wire profile](standards.md)

These checks describe the starting state, controlled events and expected answers used to test DSDV. A hand-worked oracle is an answer calculated independently of the implementation, such as a shortest-path distance or an exact byte sequence. This document explains the test scenarios and their observations; it is not a record of a new test run.

DSDV learns routes from neighbor advertisements. A destination sequence number describes how fresh the information is. Fresher information wins before distance is considered. For equal sequence numbers, a shorter route wins. A withdrawal marks a destination unreachable and retains a sequence-number floor so delayed old advertisements cannot revive it.

## Notation and test layers

A route written as **(sequence, metric)** describes one destination. For example, (12,2) means sequence 12 and distance two hops. Unless explicitly called a **wire metric**, metrics below include the receiving hop: a wire metric of 1 becomes a local metric of 2. Finite routes use even sequences; withdrawals use odd sequences and infinity, encoded as 0xffffffff.

A **successor** is the selected next-hop neighbor. The **forwarding view** is the route currently used to send packets. The **advertised view** is the information eligible to send to neighbors; it can retain an older route while a change settles. A **full dump** sends the complete advertised table, possibly in several datagrams. An **incremental update** sends selected differences from the last full dump.

| Test | Fixture and observations |
| --- | --- |
| DsdvFoundation | One routing table, synthetic updates from two neighbors, arithmetic assertions and an independent byte oracle. |
| DsdvDissemination | One routing table per scenario, explicit times and capacities, and inspection of prepared advertisements and deadlines. |
| DsdvGraph | Routing tables connected by an adjacency matrix, with controlled delivery, loop checks and independent shortest paths. |
| DsdvPaperTables | One table representing MH4, fed the neighbor advertisements needed to reconstruct the paper's table examples. |
| DsdvLifecycle | One simulated host with a DSDV probe, callback-level input, real interface signals and node lifecycle operations. |
| DsdvScheduling | One simulated host with a DSDV probe, controlled callbacks, route-table assertions and transmission counters. |
| DsdvForwarding | Real UDP control exchange and IPv4 ping forwarding in stationary wireless networks, including negative controls. |
| DsdvMobile | The four-router mobile example, with delivery checked from ping-reply timestamps. |

The first four are unit tests: their neighbors and edges are logical inputs, not radio modules. The next two are module tests: they use one real host but inject synthetic neighbor input directly into its receive callback. Only the last two exercise complete wireless paths between hosts. A table-level pass does not establish UDP delivery, and a callback injection does not establish reception over the air.

## Choice, arithmetic and recovery

### Logical topology for individual route decisions

The local node A is 10.0.0.1. Synthetic neighbors B and C are 10.0.0.2 and 10.0.0.3; destination D is 10.0.0.4. The test supplies advertisements from B and C about D without instantiating the paths beyond those neighbors.

~~~mermaid
flowchart LR
    B["B: synthetic neighbor<br/>10.0.0.2"] -->|Advertisement about D| A["A: table under test<br/>10.0.0.1"]
    C["C: synthetic neighbor<br/>10.0.0.3"] -->|Advertisement about D| A
    A -.-> D["D: destination identity<br/>10.0.0.4"]
~~~

The dashed arrow identifies the destination being looked up, not a direct physical link. The same arrangement supports the timing and correction scenarios below.

### Choice

DsdvFoundation feeds A the following sequence. The metrics shown already include A's receiving hop.

| Time | Input for D | Expected decision |
| --- | --- | --- |
| 0.1 s | B offers (10,2). | Install D through B. |
| 0.2 s | C offers (8,2). | Retain sequence 10 because sequence 8 is older. |
| 0.3 s | C offers (12,4). | Select C despite the longer distance, because sequence 12 is newer. |
| 0.4 s | B offers (12,2). | Select B because the sequence is equal and its path is shorter. |
| 0.5 s | C offers (12,2). | Retain B on the exact tie. |
| 0.6 s | B offers (12,3). | Ignore the worse route, including its attempted refresh. |

The final route must have metric 2, successor B and refresh time 0.4 s. An unusable worsening advertisement must not keep an old route alive. The general older-sequence rule also rejects older information that claims a shorter path.

### Arithmetic

The largest finite stored metric is 2147483647, the signed 32-bit maximum. An incoming metric at that value must be rejected before adding the receiving hop. An incoming metric of 2147483646 is accepted; a finite metric above the maximum is invalid even as an individual record.

Reject odd finite sequences and even infinite sequences. For unsigned 64-bit sequence arithmetic, adding two to UINT64_MAX − 2 is representable; adding two to UINT64_MAX − 1 must throw instead of wrapping. Parity validation is a separate rule: a representable arithmetic result is not automatically a valid finite advertisement.

### Recovery and retained withdrawal floors

After the choice trace, invalidate D at 0.7 s. Its state changes from (12,2) to (13,infinity). A second invalidation at 0.8 s must do nothing. A delayed finite sequence 12 at 0.9 s must leave sequence 13 in place. Finite sequence 14 at 1 s restores the route with metric 2.

DsdvDissemination also supplies a withdrawal about the local node itself. Self withdrawal sequence 19 must produce self sequence 20 and metric 0. Likewise, the illustrative self withdrawal 15 requires (16,0). A node refutes the withdrawal using newer information rather than installing an unreachable self route.

## Expiry and lifecycle

### Independent destination and neighbor expiry

In the unit scenario, A learns destinations D (10.0.0.4) and E (10.0.0.5) through B at time 0. Neighbor timeout is 5 s; destination timeout is 3 s. At exactly 3 s, B sends another update containing E but omitting D, then the test processes expiry.

D must expire, E must remain refreshed, and B must remain live. D advances from finite sequence 2 to withdrawal sequence 3. Repeating expiry at 3 s must not invalidate it again. Hearing from B refreshes B but does not refresh every destination previously learned through it.

The same unit scenario then prepares a full dump, suspends the table at 4 s and restarts it at 100 s. With one-entry capacity, the startup full dump spans three datagrams and repeats D's sequence-3 withdrawal. The self sequence is 2, showing that restart advanced it once rather than once per fragment. At 101 s, delayed finite sequence 2 cannot restore D, but finite sequence 4 can. Expiry at 106 s must report one newly invalidated destination.

If B itself expires, invalidate each selected route through B once. A complete refresh delivered at the expiry timestamp must win over the pending expiry callback. The module scenario tests that event ordering.

### Single-host module topology

Both module tests contain one stationary AdhocHost at 10.0.0.1, an IPv4 configurator and a unit-disk radio medium. The host's application slot contains a DSDV-derived probe. DsdvLifecycle additionally contains a scenario manager.

~~~mermaid
flowchart LR
    Input["Synthetic sender B<br/>10.0.0.2"] -.->|Complete datagram injected at receive callback| Probe
    subgraph host[One simulated host: 10.0.0.1]
        Probe[DSDV probe] --> Table[IPv4 routing table]
        Probe --> Timers[Periodic, advertisement and expiry timers]
        Probe --> Interface[Wireless interface and UDP attachment]
    end
    Scenario[Lifecycle scenario manager] -->|Shutdown, startup and crash| host
~~~

B and the advertised destinations are packet identities, not additional simulated hosts. The probe supplies ingress-interface, source-address and source-port tags and invokes the production receive callback. Outgoing packets use the host's attachment, but there is no remote receiver to confirm delivery.

### Lifecycle timeline

DsdvLifecycle uses a 1 s neighbor timeout, 1.5 s destination timeout and 100 s periodic interval. The long periodic interval keeps ordinary periodic updates outside the eight-second scenario.

| Time | Action | Required observation |
| --- | --- | --- |
| 0.1 s | Inject B's advertisement for D, sequence 10 and wire metric 1. | Establish the route for subsequent probes. |
| 1.1 s | Deliver another complete update exactly when neighbor expiry is due. | D remains finite with metric 2; next expiry is 2.1 s. Normal-priority input wins over the lower-priority expiry callback. |
| 1.2 s | Inject an update omitting D; add a manual route to 10.9.0.1. | Supply another component's route for later cleanup checks. |
| 2.5 s | Inject another update omitting D. | Supply neighbor traffic without refreshing D. |
| 2.600001 s | Inspect D. | D is withdrawn with sequence 11. |
| 3 s | Set the interface down. | DSDV is inactive, all three protocol timers are unscheduled, and its owned routes disappear. The manual route survives. |
| 3.5 s | Restore the interface and carrier, then inject an empty valid update. | Self sequence increases by two. The mandatory startup dump remains due immediately despite the injected update. |
| 3.500001 s | Inspect advertisement state. | The startup dump completed and advertised D's retained sequence-11 withdrawal. |
| 3.6 s | Take the interface down and up in the same callback. | At 3.600001 s DSDV is active and its UDP socket is open. |
| 4 s; inspect at 4.1 s | Shut down the node. | DSDV is inactive, its timers are cancelled and its owned routes are gone. |
| 5 s; inspect at 5.1 s | Restart the node. | DSDV is active, self sequence advanced by two, and D's sequence-11 floor remains. |
| 6 s; inspect at 6.1 s | Crash the node. | The same inactive, timer and owned-route cleanup checks hold. |
| 7 s; inspect at 7.1 s | Restart after the crash. | Self sequence advances by two and D's sequence-11 floor remains. |

The module test checks that D is withdrawn by 2.600001 s, but B can expire before D's own deadline. That observation alone does not isolate independent destination expiry; the unit scenario above does. Interface-down silence is a lifecycle requirement, but cancellation and route assertions are not a receiver-side capture of silence.

## Advertisements, corrections and settling

### Incremental updates and correction

DsdvDissemination inspects prepared advertisements directly. Their baseline is the last full dump, not the last incremental. Sending a changed destination in one incremental must not erase its difference from the full-dump baseline: a later explicitly prepared incremental can contain it again.

Required metric changes take space before optional sequence-only changes, and destinations must not appear twice in one datagram. A corrective obligation survives equality with the full-dump baseline. For example, with current finite (14,2), receiving stale withdrawal (13,infinity) requires immediate resubmission of the current finite route even if the last full dump already contained it. The illustrative pair finite (12,2) and withdrawal 11 follows the same rule.

A stale withdrawal also makes a settling correction urgent. The unit test supplies finite sequence 20 followed by withdrawal 19 and requires sequence 20 in the immediate advertisement. Several changes arriving before one urgent callback must coalesce to the latest selected record.

### Settling and fixed deadlines

Settling delays ordinary advertisements while forwarding can change immediately. Once established, an advertisement deadline must not move later as additional information arrives. A full dump during the wait must preserve the preceding advertised route.

The table-level scenario starts with D at (10,2), already included in a full dump, and a 1 s settling cap:

| Time | Action | Expected result |
| --- | --- | --- |
| 0.1 s | C offers newer sequence 12 with local metric 4. | Forwarding changes to metric 4; advertisement deadline is 0.35 s. |
| 0.2 s | B improves sequence 12 to metric 2. | Keep the established deadline. |
| 0.21 s | Prepare a full dump. | D's entry remains the preceding advertised (10,2). |
| 0.25 s | B offers sequence 14 with unchanged metric. | Deadline stays 0.35 s; updated estimate is 0.10625 s. |
| 0.35 s | Prepare the due incremental. | Advertise sequence 14 and clear the pending deadline. |

As a separate hand calculation, an old estimate of 0.5 s and an improvement sample spanning 0.1–0.2 s produce 0.75 × 0.1 + 0.25 × 0.5 = 0.2 s. With a 1 s cap and an initial wait ending at 1.1 s, further improvements cannot extend that deadline. Invalidation discards an open improvement sample. The smoothing law and delay bounds are engineering choices of this profile.

New reachability and withdrawals are immediate. Stable metric or successor changes after the cap are immediate too. Other changes wait at most one cap while the interface remains usable. Sequence-only changes schedule no trigger; they can fill spare incremental capacity or appear in a subsequent periodic dump.

### Minimum settling

Starting at sequence 2, feed 32 unchanged-metric updates at 1 ms intervals until the estimate reaches zero and sequence reaches 66. A full dump at 32 ms establishes the baseline. At 40 ms, C supplies sequence 68 with a worse metric. The default 10 ms minimum makes the update due at 50 ms.

A full dump at 41 ms must still carry sequence 66. An improvement from B at 42 ms must neither bypass nor postpone the deadline. At 50 ms, advertise sequence 68. Withdrawal 69 at 51 ms must be urgent immediately.

A configuration boundary check sets minimum and cap both to 20 ms: a change at 1 ms must be due at 21 ms. Reject zero or negative minima and a minimum above the cap.

### Production callback scheduling

DsdvScheduling repeats the key scenarios using the single-host module fixture. Neighbor and destination timeouts are 10 s and 12 s, settling cap is 1 s, DSDV payload budget is 48 bytes and ordinary periodic interval is 100 s. The probe forces selected periodic callbacks one microsecond after its current event.

| Time | Controlled event | Required observation |
| --- | --- | --- |
| 0.1 s; periodic callback after 0.11 s | Learn and advertise D at (10,2). | Establish the advertised baseline. |
| 0.2 s | Receive (12,4). | IPv4 forwarding changes immediately; advertisement timer is due at 0.45 s. |
| Periodic callback after 0.3 s | Send a full dump while settling. | At 0.31 s D's advertised entry remains (10,2). |
| 0.35 s and 0.4 s | Improve sequence 12 to metric 2, then supply sequence 14. | Deadline stays 0.45 s. Just afterward, advertised sequence is 14 and the timer is clear. |
| Periodic callback after 0.48 s; withdrawal at 0.5 s | Establish full baseline 14, then receive stale withdrawal 13. | Correction is due immediately and exactly one extra control packet is submitted. |
| 0.55 s and 0.56 s | Receive finite sequence 16 with local metric 4, then stale withdrawal 15. | Promote the deferred route to an immediate correction; advertised sequence becomes 16. |
| 0.6 s; periodic callback after 0.7 s | Receive same-metric sequence 18. | No trigger is scheduled, but the periodic dump advertises sequence 18. |
| 0.9 s | Learn three new destinations with two-entry capacity. | One full-dump event submits three packets and leaves no advertisement callback pending. |
| 1 s | Inject unknown-version input, then duplicate destination entries. | Both are rejected without changing accepted-update count, D's sequence or refresh time, or the expiry deadline. |
| 1.2 s | Receive newer withdrawal 19. | Exactly one remote-withdrawal indication occurs and D's IPv4 route disappears. |

These observations distinguish a stale withdrawal requiring correction from a newer withdrawal requiring route removal. Transmission counters observe local submission, not delivery to a remote receiver.

## Payload, periodic updates and encoding

### Capacity and overflow

Let K be the number of entries that fit in one DSDV datagram. The profile uses a 16-byte fixed header and 16 bytes per entry. A 48-byte DSDV payload budget therefore fits two entries. Interface MTU accounting must additionally reserve IPv4 and UDP headers.

K required entries fit one incremental; K+1 require escalation to a split full dump. Optional sequence-only changes must not force escalation. In the unit scenario, capacity 2 and three newly required destinations produce a five-entry full table in three fragments, of sizes 2, 2 and 1. Later, with capacity 1, three optional sequence changes and one required new destination produce one incremental containing the required destination only.

The module overflow scenario likewise counts three submitted fragments from one full-dump event. These assertions check the selected capacities and bounded submission, not every possible interface MTU.

### Periodic updates and startup loss

Advance the self sequence by two per update, not per fragment. The graph fixture explicitly advances each sender once before preparing its fragmented full dump.

The graph fixture discards every startup advertisement, then supplies later periodic full dumps. The real wireless fixtures also start routers together and configure randomized periodic phases. Later delivery checks exercise startup recovery, but do not independently assert which startup packets collided or verify a random-phase distribution.

### Exact bytes and malformed input

The foundation oracle uses sender 10.0.0.2, self sequence 0x0807060504030200, and one entry for 10.0.0.4 with sequence 0x0102030405060708 and wire metric 2. Its independent expected 32 bytes are:

~~~text
01 01 00 01 0a 00 00 02 08 07 06 05 04 03 02 00
0a 00 00 04 01 02 03 04 05 06 07 08 00 00 00 02
~~~

The first line contains version, kind, entry count, sender and self sequence; the second contains the destination entry. All multibyte values use network byte order. Serialization must match these bytes, and decoding must recover a valid header and the original entry.

Corrupt selected bytes to check rejection of unknown version or kind, inconsistent count, invalid sender and odd finite sequence. Lengths of 15, 31 and 33 bytes exercise short-header, short-entry and trailing-byte rejection. Separate assertions reject duplicate destinations, out-of-range metrics, invalid parity and invalid identities such as unspecified, loopback, multicast and limited broadcast addresses.

Validate the entire datagram before changing route state or clocks. The module's unknown-version and duplicate-entry probes check that rejection is atomic. This is an INET wire-profile check, not an external interoperability claim.

## Distributed graph topologies

DsdvGraph creates one routing table per vertex. Vertex 0 has address 10.0.0.1, vertex 1 has 10.0.0.2, and so on. An edge permits direct update delivery in both directions. No radio, UDP or IPv4 forwarding module participates.

~~~mermaid
flowchart LR
    subgraph single[Disconnected single vertex]
        S0[0]
    end
    subgraph pair[Pair]
        P0[0] --- P1[1]
    end
    subgraph line[Four-node line]
        L0[0] --- L1[1]
        L1 --- L2[2]
        L2 --- L3[3]
    end
    subgraph diamond[Diamond]
        D0[0] --- D1[1]
        D0 --- D2[2]
        D1 --- D3[3]
        D2 --- D3
    end
    subgraph cycle[Three-node cycle]
        C0[0] --- C1[1]
        C1 --- C2[2]
        C2 --- C0
    end
~~~

All tables use a 0.1 s settling cap, 2 s neighbor timeout, 3 s destination timeout and two-entry fragment capacity. Drop all startup dumps. Each later round advances time by 0.5 s. In vertex order, each sender advances its self sequence once, prepares a full dump, and delivers its fragments in reverse order to every adjacent receiver. Deliver each fragment twice, then process expiry after the round.

After every controlled delivery and expiry, follow each finite successor chain for every source/destination pair. Visiting a node twice is a loop and fails the check. Reaching a missing or withdrawn route ends the chain; absence of a loop does not by itself prove reachability.

After 20 rounds, independently calculate shortest paths using breadth-first search. Each reachable destination must have that metric; unreachable destinations must have no finite route. The line requires distances 1, 2 and 3 from vertex 0. The diamond requires a two-hop path from 0 to 3 without prescribing which equal-cost branch wins.

For every graph except the single vertex, remove all edges incident to vertex 0 and run 20 more rounds. Recheck distances, replay saved old fragments only across remaining edges, and recheck distances and loop freedom. Restore the original edges and run another 30 rounds before checking distances again. This covers isolation, stale replay and rejoin in the illustrated graphs, not arbitrary delivery schedules or every possible topology.

## Real forwarding topology

DsdvForwarding uses four independent stationary wireless groups. Neighbors are 100 m apart and radio range is 110 m, preventing two-hop shortcuts. Groups are separated far enough to prevent cross-group links.

~~~mermaid
flowchart LR
    subgraph enabled[DSDV enabled line]
        A["line[0]<br/>10.0.0.1"] --- B["line[1]<br/>10.0.0.2"]
        B --- C["line[2]<br/>10.0.0.3"]
        C --- D["line[3]<br/>10.0.0.4"]
    end
    subgraph disabled[Same line with DSDV replaced by UDP sinks]
        E["disabled[0]<br/>10.0.1.1"] --- F["disabled[1]<br/>10.0.1.2"]
        F --- G["disabled[2]<br/>10.0.1.3"]
        G --- H["disabled[3]<br/>10.0.1.4"]
    end
    subgraph pair[Direct neighbors with DSDV]
        I["pair[0]<br/>10.0.2.1"] --- J["pair[1]<br/>10.0.2.2"]
    end
    subgraph isolated[No neighbors]
        K["isolated<br/>10.0.3.1"]
    end
~~~

Use static addresses and ordinary local ARP. Disable configurator-generated static, default and subnet routes and clear connected netmask routes. This prevents configured routes from supplying the path DSDV must learn. Interfaces are AckingWirelessInterface with GenericUnitDiskRadio at 2 Mbps: a simplified routing fixture, not an IEEE 802.11 MAC/PHY conformance test.

Enabled routers use 500 ms full dumps, 50 ms jitter, 3 s neighbor and 4 s destination timeouts, a 100 ms settling cap and a 64-byte payload budget. DSDV occupies application slot 0; Ping occupies slot 1. Pings start at 6 s, repeat every 500 ms, and the simulation ends at 10 s.

The enabled line pings from its first to its last node. The pair pings from its first to its second node. The disabled line attempts the corresponding three-hop ping with the same physical connectivity but no DSDV application.

Required observations:

* At least one accepted control update across the network, at least one reply at the enabled line's source, and at least one reply at the pair's source.
* Both intermediate line nodes receive IPv4 ICMP traffic addressed to 10.0.0.4.
* The first line node sends at least one ARP request, and its observed requests target next hop 10.0.0.2 rather than remote destination 10.0.0.4.
* All 12 ordered non-self routes in the enabled line end as DSDV-owned /32 MANET routes. Metric is the number of line edges to the destination; next hop is the adjacent node in that direction.
* The isolated node has no DSDV-owned IPv4 route. The disabled source has no route to its far endpoint and receives no ping replies.

For example, the source's final route to 10.0.0.4 must use next hop 10.0.0.2 and metric 3. A reply exercises the return path too, although intermediate observations inspect destination-bound requests rather than every hop of each reply. The test requires some delivery, not every ping or correct routes throughout startup.

## Mobile partition and recovery topology

DsdvMobile runs four DsdvRouter hosts at x coordinates 100, 200, 300 and 400 m, initially all at y = 100 m. Addresses are 10.0.0.1 through 10.0.0.4. Each router has a separate DSDV module attached through the transport dispatcher. As in the fixed fixture, 110 m radios connect immediate neighbors, with no supplied multihop routes or global ARP.

~~~mermaid
flowchart LR
    subgraph joined[Before departure and after return]
        A["host[0]<br/>source"] --- B["host[1]<br/>moving relay"]
        B --- C["host[2]"]
        C --- D["host[3]<br/>destination"]
    end
    subgraph partition[While the relay is away]
        E["host[0]<br/>isolated source"]
        F["host[1]<br/>200 m, 500 m"]
        G["host[2]"] --- H["host[3]"]
    end
~~~

The relay waits until 8 s, moves from (200 m, 100 m) to (200 m, 500 m) during 8–12 s, waits until 20 s, and returns during 20–24 s. It crosses the range boundary while moving; these movement milestones are not asserted route-change instants.

Routers use the same 500 ms full dumps, 50 ms jitter, 100 ms settling cap and 64-byte budget as the fixed test, but neighbor and destination timeouts are 2 s and 3 s. The source pings the last node every 500 ms from 4 s until the simulation ends at 32 s. Run and seed are both 0.

Export the source Ping application's reply vector, require exactly one matching vector, then inspect its receipt timestamps:

| Window | Required result | Meaning |
| --- | --- | --- |
| 4 <= t < 8 s | At least one reply. | A usable initial path exists. |
| 12 <= t <= 22 s | No replies. | End-to-end delivery stops while the relay is out of range. |
| 26 <= t < 32 s | At least one reply. | A usable path returns after rejoining and a recovery allowance. |

The transition gaps allow movement, queued traffic and routing recovery without asserting one exact convergence instant. Silence alone does not prove stale-route removal: physical disconnection can prevent delivery even if stale routes remain. Recovery does not inspect withdrawal sequences or require every ping to succeed; those algorithm details need the separate checks above.

## Paper Tables 1–4 oracle

DsdvPaperTables reconstructs MH4's route and advertisement tables from the paper's section 4 examples. It models one table with synthetic input from MH2 and MH6, not the complete physical network. MHn uses address 10.0.0.n.

~~~mermaid
flowchart LR
    MH2["MH2 supplies routes<br/>initially including MH1"] --> MH4[MH4 table under test]
    MH6["MH6 supplies routes<br/>later including newer MH1"] --> MH4
~~~

Advance MH4's self sequence 355 times to reach 710. Neighbor updates then establish this initial full-dump oracle:

| Destination | Next hop | Metric | Sequence |
| --- | --- | ---: | ---: |
| MH1 | MH2 | 2 | 406 |
| MH2 | MH2 | 1 | 128 |
| MH3 | MH2 | 2 | 564 |
| MH4 | Self | 0 | 710 |
| MH5 | MH6 | 2 | 392 |
| MH6 | MH6 | 1 | 76 |
| MH7 | MH6 | 2 | 128 |
| MH8 | MH6 | 3 | 50 |

Capacity eight must produce one full datagram containing all eight metric/sequence pairs. MH4 advertises its local metric without adding another hop.

The initial full-dump assertions compare metrics and sequences. The next-hop column explains the supplied neighbor inputs; the later MH1 assertion explicitly checks its changed successor as well as its metric and sequence.

After 55 further self-sequence advances and updated neighbor input at 8 s, all listed sequences increase by 110. MH1 additionally changes to successor MH6 and metric 3 with sequence 516. This newer route replaces the older two-hop route despite being longer. A two-entry incremental must place MH1's required metric change first; optional unchanged-metric sequence updates may fill spare capacity. A subsequent full dump must retain the other metrics and contain all updated sequences.

Sender identity and self sequence live in this profile's datagram header. The check therefore does not depend on the paper example's sender-first entry convention.

## Interpreting the coverage

Unit checks establish controlled table decisions and encoding. Module checks add kernel ordering, route installation, rejection and lifecycle behavior. Protocol checks add real control exchange and application delivery. No single layer establishes all of these properties.

The scenarios include both source-derived DSDV behavior and engineering choices of the bounded IPv4 profile, including checked sequences, smoothing, retained withdrawal floors and encoding. The [source profile](standards.md) owns those distinctions. The [coverage ledger](../../model/dsdv/coverage.md) and [results record](../../model/dsdv/results.md) own mappings and executed evidence; they are needed to establish what was run, not to understand the scenarios here.
