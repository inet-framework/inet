# Implement DSDV from scratch

Status: **implemented and validated**, 2026-11-08. Authorized by the request to implement and push.

Completed: independent catalog and paper-table oracle; versioned wire tooling;
state owner; UDP/IPv4 adapter; expiry, recovery and lifecycle; incrementals and
settling; deterministic mobile example; eight passing correctness tests and a
passing new fingerprint. Debug/release, feature-off and isolated-feature builds
passed. Same-seed and recorder/logging-neutrality checks matched. Current evidence,
commands and the pre-existing global-gate/missing-tool gaps are recorded in
`doc/project/evidence/model/dsdv/results.md`. Nonvisual expectations travel with
their causal controller and timing changes; visual references remain unchanged.

Implementation discovery: the state owner is a `DsdvTable` member of the routing
module. Selection, clocks and dissemination are implemented and tested together
as one state foundation so that the UDP adapter cannot introduce a second owner.
The commit series consolidates the state mechanisms from steps 1, 3 and 4 into
that foundation; production forwarding, lifecycle/kernel evidence, dissemination
edge evidence and final integration remain separate review concerns. This changes
the proposed intermediate commit boundaries, not the acceptance criteria.

The UDP-controller commit includes `tests/protocol/dsdv/DsdvForwarding.test` and its reviewer explanation `tests/protocol/dsdv/DsdvForwarding.md`. The mobile-example commit includes `tests/protocol/dsdv/DsdvMobile.test` and `tests/protocol/dsdv/DsdvMobile.md`. Each explanation belongs beside the behavior commit and describes its topology, controlled input, required observations and limits; neither is deferred to the final documentation index.

## Basis and scope

Build a new DSDV model in the INET checkout at `/home/miguel/workspace/inet-dsdv`.
The existing DSDV implementation, its tests, and its configuration are excluded as design inputs.
This plan was prepared without reading them. Use the original publication and generic INET contracts.
Do not use another simulator's DSDV implementation as a substitute specification.

The source is the original paper by Charles E. Perkins and Pravin Bhagwat,
*Highly Dynamic Destination-Sequenced Distance-Vector Routing (DSDV) for Mobile Computers*,
SIGCOMM 1994, pp. 234–244, [DOI](https://doi.org/10.1145/190314.190336).
An accessible [copy of the paper](https://www.cse.iitb.ac.in/~mythili/teaching/cs653_spring2014/references/dsdv.pdf)
was read for this plan.

The paper's algorithmic basis (§3 and §4): prefer newer destination sequence numbers, then smaller
hop counts. Destinations originate even sequence numbers; link failures produce odd sequence numbers
and infinite metrics. Maintain periodic and triggered broadcasts, full dumps and incremental updates,
and distinct forwarding and advertised state. Estimate settling time using a weighted history;
unreachable routes bypass damping. Prioritize metric changes in incrementals, using remaining room
for sequence-only changes. A newer finite route can rebut a stale unreachable advertisement.

The first deliverable is network-layer IPv4 host routing over one configured ad hoc interface per
node, with hop-count metrics and bidirectional connectivity assumed. IPv6, multiple active routing
interfaces, prefix/default-route dissemination, layer-2 routing, proxy ARP, security, and alternate
metrics are outside this first scope. Reject unsupported configurations explicitly. These are proposed
scope choices, not limitations imposed by the paper. Broadcast reception alone does not prove a
bidirectional link; asymmetric-link operation is not claimed.

## 1. Freeze a specification and test oracle before coding

Create a short requirements catalog with paper section and table references. Separate statements
from the paper from engineering choices. Use the
[specification-first procedure](../../doc/project/guide/derive-tests-from-a-standard.md):

- `doc/project/evidence/standard/perkins1994/catalog.md`: paper statements and ambiguities.
- `doc/project/evidence/protocol/dsdv/{standards,features,checks}.md`: source scope, features, and
  English test procedures, without INET names or run results.
- `doc/project/evidence/model/dsdv/{coverage,conformance,categories,results}.md`: proposed model
  support and, later, actual test outcomes. Initially state that nothing has run.

Resolve the paper's inconsistent paragraph in §4, “Damping Fluctuations”: it describes adopting a
worse metric at the same sequence number, while the preceding selection rules prefer a lower metric.
Proposed interpretation: follow the repeated freshness/lower-metric selection rules; never adopt a
worse same-sequence candidate merely because it arrived later. A detected failed successor is handled
as invalidation. Record this interpretation and its rationale before implementation; retain a test
distinguishing the alternatives. Do not silently present the choice as unambiguous source text.

Write hand-worked traces, including §4's tables, for route choice, loss, and restoration. These are
the correctness oracle. Do not compare against the excluded implementation.

## 2. Declare the INET boundary

Follow [add-a-protocol](../../doc/project/guide/add-a-protocol.md) and
[protocol anatomy](../../doc/project/design/protocol-anatomy.md). Proposed source owner:
`src/inet/routing/dsdv/`, namespace `inet::dsdv`, stem `Dsdv`.
Names in this plan are targets, not assertions about files already present.

Verified generic contracts:

| Boundary | Contract checked | Proposed use |
| --- | --- | --- |
| Lifecycle | `RoutingProtocolBase` derives from `OperationalBase` and selects routing stages | Own startup, timers, stop, crash and restart |
| Node attachment | `ApplicationLayerNodeBase.app[]` uses `IApp` socket gates | Attach a UDP-based routing control module through the existing dispatcher |
| Transport | `UdpSocket` supports bind, broadcast, TTL and callbacks | One-hop control exchange, with output interface selected explicitly |
| Forwarding | `IRoutingTable` creates/adds/removes/deletes routes; `IRoute` sets destination, next hop, interface and source | Publish selected `/32` host routes and withdraw only this module's routes |

Using an application attachment slot describes transport plumbing; the module remains a routing
control protocol. Keep ordinary data forwarding in IPv4. Confirm interface selection, socket-close
outcomes, route-source classification, protocol registration, and route deletion notifications against
their generic providers before writing the adapter. Do not invent contract methods or edit core
registries to make integration work.

Declare `Dsdv.ned` first: interface selection, control port, update intervals, jitter, failure timeout,
settling parameters, payload budget, signals and statistics. Then write `DsdvHeader.msg`, its registered
serializer, dissector and printer together. Keep transport/interface metadata in existing tags.
Do not add custom tags, a configurable strategy interface, or separate submodules without a caller
that needs them. A small protocol module with private route-state and scheduling helpers is sufficient.

## 3. Define state ownership and transitions

One protocol owner holds destination records: successor, interface, metric, sequence, reachability,
destination refresh time, last metric/successor change time, settling sample and estimate, a pending
deadline, and last advertised values.
Keep one full-dump baseline for incremental comparisons and a set of destinations that must be sent
at the next advertisement callback. This set records send obligations, including corrective resends;
it does not duplicate route values. The IPv4 route is a published projection,
not a second owner of DSDV freshness. No revision counters or deferred packet snapshots are needed:
each send callback constructs and submits its packets synchronously.

Start with a self record at metric zero and even sequence zero. Advance the local sequence by two
at each periodic update, once per update rather than once per datagram. Self records never acquire
a successor. For all other destinations, use this proposed transition table after input validation:

| Stored record | Received record | Action |
| --- | --- | --- |
| Unknown | Finite, even sequence | Install the received sequence and advertised metric plus one; mark newly reachable |
| Unknown | Infinite, odd sequence | Retain an invalid sequence floor; advertise the withdrawal |
| Reachable or invalid | Older sequence | Ignore; if it is an infinite report and the stored route is finite, urgently advertise the stored route as a correction |
| Reachable or invalid | Newer finite, even sequence | Install the candidate, including when its metric is worse; mark forwarding changed |
| Reachable or invalid | Newer infinite, odd sequence | Store invalid state and the received sequence; remove any owned IPv4 route; urgently advertise |
| Reachable | Equal sequence, smaller finite candidate metric | Install the improvement and its successor |
| Reachable | Equal sequence, equal finite candidate metric | Keep the successor; refresh the destination only if the sender is that successor |
| Reachable | Equal sequence, larger finite candidate metric | Ignore without refreshing the destination |
| Invalid | Equal sequence, infinite metric | Keep invalid state; do not retrigger merely for a duplicate |

Here “candidate metric” includes the receiving hop exactly once. Finite records have even sequences
and infinite records have odd sequences; mixed-parity cases at equal sequence are invalid input.
For a report about the local destination, ignore finite records. If an infinite report has a sequence
greater than the local sequence, set the local sequence to the smallest even value greater than it
and urgently advertise metric zero; otherwise do not advance it. Never install a route to self.

Use a 32-bit unsigned wire metric with `UINT32_MAX` reserved for infinity, but limit finite metrics
to `INT_MAX` to match `IRoute::setMetric(int)`. Reject finite wire values above that limit and reject
a received finite entry if adding the receiving hop would exceed `INT_MAX`. Check before addition
or signed conversion; do not convert an oversized finite entry into an even-sequence withdrawal.
This finite-run profile does not claim networks whose shortest paths exceed that finite limit.

Neighbor liveness and destination freshness have separate clocks. Any fully validated control
datagram on the configured interface refreshes its sender's neighbor clock. A destination clock is
refreshed only by installation of an accepted finite route or an equal-sequence/equal-metric record
from its selected successor. A sender header alone does not refresh all routes through that sender.
Omission from a full dump or incremental is not an implicit withdrawal.

At neighbor timeout, immediately invalidate every selected route through that neighbor. At destination
timeout, invalidate that destination even if its successor remains live. Interface-down invalidates
all selected routes on that interface. Invalidation removes the owned IPv4 route, changes an even
sequence `s` to odd `s + 1`, and urgently advertises infinity. Repeated expiry of invalid state does
nothing. Recovery requires a newer finite sequence; never restore an older cached alternative.

Configure neighbor timeout `N` and destination timeout `R` separately. For loss-free fixtures, choose
`N > P + J + L` and `R > P + J + S + L`, where `P` is the full-dump period, `J` its maximum jitter,
`S` the settling cap, and `L` the fixture's control-delivery bound. These inequalities prevent the
fixture's own schedule from causing expiry; they are not loss or convergence guarantees. Under loss,
timeouts deliberately trade false invalidations against detection delay.
Require `P > 0`, `S > 0`, `J >= 0`, `N > 0` and `R > 0` at initialization.

Use 64-bit unsigned sequence numbers and checked increments initially. Stop with an explicit error
before exhaustion; do not introduce unreviewed wraparound comparisons. This is a finite-run modeling
choice.

Retain invalid destination records and their sequence floor until the run ends in this first model.
This prevents garbage collection from allowing a delayed old route to reappear. Memory consequently
scales with the number of distinct destinations observed. Every full dump also repeats those
withdrawals, so periodic control traffic scales with retained destinations, including departed nodes.
Bounded garbage collection requires a separate, justified message-lifetime and restart contract.

## 4. Implement dissemination and settling

Use one periodic timer, one earliest-advertisement timer, and one expiry timer covering both neighbor
and destination deadlines. Use simulation time and destination-address order. Give timers lower
scheduling priority than incoming control delivery; at a tie, process expiry before sending, and
re-read current records when constructing packets. Tests must establish that a refresh delivered at
the expiry timestamp wins. A packet still queued below UDP at that timestamp is not yet a refresh.

Use one configurable settling cap `S`; do not add a quiet-window timer or configurable smoothing
strategies. For each destination's current finite sequence, record its first acceptance time and the
time of its last equal-sequence metric improvement, initially the same time. On acceptance of a newer
finite sequence, close any existing finite sample as `min(S, lastImprovement - firstAcceptance)` and
update
`estimate = 0.75 * sample + 0.25 * estimate`. Initialize `estimate = S / 2`. Invalidation discards an
unfinished sample without updating the estimate, including when caused by a newer odd-sequence
withdrawal. Recovery from invalid state starts a new finite sample without closing an old one.
These are explicit model choices rather than a
claim that the paper defines this equation. A sequence without improvements contributes zero; each
sequence owns only one sample, so overlapping quiet windows cannot arise.

Forwarding changes apply immediately. A newly reachable route, withdrawal, corrective response, or
finite metric/successor change after at least `S` of unchanged metric and successor is urgent.
Other finite metric/successor changes open one deadline at
`now + min(S, 2 * estimate)`. Later updates replace the pending values but never move that deadline
later; urgency can move it earlier. Sequence-only updates do not create a trigger. If one arrives
while a deadline exists, it replaces the pending sequence without moving the deadline. Thus an open
finite-change deadline is serviced within `S` of its creation, irrespective of later updates, while
the module remains operational and its interface remains usable.

When a finite-change deadline becomes due, promote its current record and add the destination to
the must-send set. An urgent change makes its record eligible immediately, adds its destination to
that set, and schedules the advertisement callback at the current simulation time, with no settling
or trigger holdoff. A corrective response does this even when the record equals the last full-dump
baseline; if it was settling, the correction makes its current forwarding record eligible now.
Multiple changes before that callback coalesce by destination, and construction uses the current
record rather than the value when the obligation was created. This bounds protocol
submission delay while the interface is usable, not wireless access or reception time. Periodic
callbacks advance by `P + U(0, J)`; use only the module's assigned OMNeT++ RNG. After each startup
dump, randomize the phase of the first periodic update in `[0, P]`. The startup dump itself is
immediate: simultaneous node starts can collide. Test convergence with that initial collision rather
than assuming the randomized periodic phase prevents it.

Both full dumps and incrementals use the same advertised view. At any send callback, promote eligible
pending records to that view. For a route still settling, keep its last advertised values; omit it
if it has never been advertised. Withdrawals and newly reachable routes are eligible immediately.
Sequence-only changes may be promoted at any send when no finite-change deadline blocks them.
A periodic full dump must not promote a still-settling forwarding record merely because it is full.

For an incremental, form the required entries as the union of the must-send set and advertised
entries whose reachability or metric differs from the last full-dump baseline. Include each
destination once. If these required entries fit one datagram, include all of them, then fill spare
capacity with sequence-only changes relative to that baseline in destination-address order.
Sequence-only changes left over wait for another send or the next periodic full dump; they never
cause overflow escalation. If the required entries alone exceed capacity, submit a full dump of the
advertised view in that callback, split into independently processable MTU-sized datagrams. This
rule needs no fair queue or separate urgent batches. Include sender identity and current self
sequence in every datagram header, so neighbor discovery does not depend on chunk contents.

Discharge must-send obligations only after their entries have been submitted to UDP, whether in an
incremental or a full dump. Baseline equality never cancels an obligation. An incremental does not
replace the full-dump baseline; significant changes since that dump can therefore be repeated in
later incrementals. No send callback interleaves with another state transition.

Construct and submit every full-dump datagram synchronously in one callback; retain no unsent snapshot.
Then replace the baseline with the advertised view actually submitted. The loop has at most
`ceil(A / K)` datagrams, for `A` advertised entries and capacity `K >= 1`; it never reschedules itself
at zero time. No receive or expiry callback can interleave during its construction. Already submitted
packets can still be delayed or reordered, which sequence comparisons must tolerate.

Sequence-only changes that create no trigger appear in the next periodic full dump after they
become eligible, within `P + J`. Periodic dumps also repeat withdrawals and repair broadcast loss
when a later dump is received. This periodic bound applies only while the module and interface remain
operational. Neither periodic transmission nor a finite send queue guarantees
delivery under arbitrary packet loss.

## 5. Specify the simulation wire profile

The paper does not supply a complete interoperable byte format. Define and version an INET model
profile explicitly: IPv4 UDP broadcast, TTL 1, configurable unprivileged port, network byte order,
sender identity, message kind, entry count, and entries containing destination, sequence and metric.
Declare field widths, infinity encoding, length accounting, and payload budget before implementing
the serializer. Do not claim interoperability with an external DSDV implementation.

Reject malformed lengths/counts, unsupported versions, duplicate destinations, invalid address forms,
and inconsistent metric/sequence parity before mutating state. Sender identity and self sequence are
mandatory header fields, with an even self sequence. Pin exact policy in encoding tests. Every
datagram must fit the configured interface MTU after IPv4/UDP overhead; diagnose a budget too small
for one entry. Never rely on IP fragmentation for incrementals.

## 6. Handle lifecycle and observability

Start only after addresses and interfaces are usable, covering all inherited initialization stages.
When the configured interface goes down during operation, invalidate its selected routes, then cancel
all three protocol timers and suspend control transmission and reception. Retain sequence floors;
do not queue control packets for a down interface. Interface-up resumes the existing module without
requiring a module restart: once the interface and its address are usable, run the restart/recovery
procedure below and send its immediate full dump. Down-time invalidations are disclosed by that dump.
No deadline or neighbor clock runs across the outage.

On stop or crash, cancel timers, terminate socket activity through its supported API, and discard
pending advertisements. Remove only routes owned by this module; avoid borrowed route pointers that
can outlive routing-table cleanup. A crash sends no control packets.

Use one startup procedure for initial start, restart, and interface recovery of the same module
instance. Clear all
pending advertisement deadlines, the must-send set, neighbor clocks, advertised values and the
full-dump baseline. Reset settling samples and estimates. On initial start create the self record
at sequence zero. On restart or interface recovery retain the self sequence and advance it by two;
retain invalid learned
sequence floors and turn every retained finite learned record into an odd-sequence invalid record
using the invalidation rule. Relearn paths only from newer finite records.

Seed the advertised view with the self record and every retained withdrawal. Once the socket and
interface are usable, schedule an immediate full dump of that view, even if every withdrawal was
advertised before the stop. This startup dump establishes a new baseline and adds no further self
sequence increment. Schedule the first periodic update using the randomized phase from section 4.
Module destruction/recreation with the same address is outside this first restart contract unless
persistent sequence state is supplied. Test ordinary shutdown, crash and restart separately.

Declare signals for accepted/rejected updates, route installation/withdrawal, invalidation reason,
control bytes/packets, triggered/full/incremental sends and settling delay. Emit at the owning
transition; external recorders derive statistics. Logging and observers must not change decisions.

## 7. Deliver in buildable commits with direct evidence

| Commit | Deliverable | Required evidence in that commit |
| --- | --- | --- |
| 1 | Source catalog, English checks, NED interface first, then wire/state foundation and feature/build plumbing | Hand-worked trace review; comparison, arithmetic and serializer unit tests including signed metric boundaries; feature-on/off build |
| 2 | UDP adapter, startup, periodic full dumps and forwarding projection | Two-node and line-topology protocol tests proving control exchange and real data forwarding |
| 3 | Expiry, invalidation, corrective updates and lifecycle | Partition/rejoin, lost-successor cascade, stale replay, stop/crash/restart tests including startup repetition of previously advertised withdrawals; interface-down silence and interface-up recovery without a module restart |
| 4 | Incrementals, settling, jitter and payload scheduling | Controlled timer tests; full-dump settling rules, corrective resend at baseline equality, sequence-only overflow exclusion, required-entry overflow, submission deadlines and convergence after colliding startup dumps |
| 5 | Example, final documentation and integration verification | Feature isolation; deterministic mobile example; focused fingerprint regression plus all correctness checks |

Implement each commit's behavior and tests together. The early module must not claim features delivered
only by later commits. Each implementing commit carries `Plan: plan/pending/dsdv-from-scratch.md` and
the current classification trailer required by [pull-request rules](../../doc/project/rule/pull-request.md).

The decisive scenarios are: a disconnected node, two nodes, a four-node line, a diamond with a newer
longer route followed by an equal-sequence shorter route, equal ties, route loss in a cycle, and
partition/rejoin under delayed/duplicated/reordered control messages. Test that self metric remains
zero, sequence floors never regress, and the graph of usable successors has no cycle after each
controlled transition. Check packet paths as well as table state; successful delivery alone does not
exclude transient loops.

In forwarding fixtures, assign unique static IPv4 addresses, enable forwarding at intermediate nodes,
and disable configurator-generated multihop routes and other routing protocols. Inspect the effective
table: only DSDV-owned `/32` routes may provide a multihop path. Connected routes must not bypass the
selected next hop; ARP resolves that next hop rather than the remote destination. In the line fixture,
prove endpoint delivery fails with DSDV disabled under otherwise identical addressing, connectivity
and traffic. With DSDV enabled, prove the owned routes and intermediate packet traversal before
counting delivery as evidence. Do not use global ARP or injected host routes to hide integration gaps.

Use module tests for scheduling and lifecycle, protocol tests for production exchanges and forwarding,
unit tests for pure comparisons and encoding, and fingerprints only for trajectory regression.
For ideal loss-free static fixtures, compare converged host-route distances to independent graph
shortest paths. Use published examples as external algorithm checks. Performance claims require
separate repeated-seed validation and justified uncertainty; they are not acceptance shortcuts.

## Completion and remaining discovery

Acceptance requires every claimed feature to have direct passing evidence; no unexplained loops,
stale resurrection, surviving timers, foreign-route deletion, or payload overflow; and documented
model choices and limits. Build fresh libraries in the tested mode, run explicitly selected new cases,
and record commands, configurations, seeds, statuses and artifact locations. Verify same-seed
repeatability and observer neutrality. Follow the focused and before-push sequences in
[run-the-gates](../../doc/project/guide/run-the-gates.md), including debug/release compilation,
architecture/naming checks and feature-off compilation.

At planning freeze no algorithm tests, build, or simulation had run. Concrete new test filenames,
runner filters and numerical timer defaults were subsequently established with the specification
and fixtures; current execution evidence is in the model results. The project map and `doc/project/guide/` search expose
no dedicated implementation-plan procedure or mandatory plan approval template; this draft uses
`plan/pending/` and the current contribution/protocol/test routes.

The existing DSDV model remains uninspected. Before integration, inventory only filename and type-name
collisions and externally visible references, without using existing DSDV behavior as evidence.
Develop the new files in an isolated source staging area if names collide; choose replacement and
migration handling as a separate integration decision. Do not ship two definitions of the same module.
The source seal registry currently covers `common/packet/`; use its public API without changing it.
Any later recorded-expectation changes need their exact scope and reason reviewed under
[baseline rules](../../doc/project/rule/testing.md#tr-baseline-deliberate).

The planning-only authorization has been superseded by the explicit implementation
request. The implemented profile and its measured support are maintained in the
model evidence, not inferred from the intent recorded in this plan. Existing
scenario trajectories are intentionally not claimed compatible with the new model.
