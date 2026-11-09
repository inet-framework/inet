# Correct AODV against RFC 3561

> **Kind:** decision · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [contribute-a-change.md](../../doc/project/guide/contribute-a-change.md), [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md)

Status: **implementation in progress**. Written 2026-11-08 for
`/home/miguel/workspace/inet-dsdv`, with source inspected at
`6d1604c54b4837107eb4ab1e86422807e46fc17c`.

Execution tracking, 2026-11-09: preparation, freshness, lifetimes and discovery ownership are
implemented with recorded parent comparisons. The ownership stage includes basic TTL/backoff
progression because it replaces the old timer/retry stores together. The next stage retains
timing assertions, parameter arithmetic and defaults. Source-derived eligibility refinement:
connected routes on the selected MANET interface define address scope, not multihop reachability;
explicit independent routes remain eligible. See model results and discovery-ownership.md.

This plan covers route freshness and lifetimes, discovery progress, neighbor validation, transit
failure, reboot recovery, selected interfaces, received subnet routes and opt-in local repair.
The acceptance matrix in §7 defines the correction and compatibility scope without relying on a
separate review. Source-derived defect hypotheses require runtime reproductions before they become
executed before/after evidence. Implementation was authorized by the user on 2026-11-09.

## 1. Scope, sources and decisions

The behavioral source is [RFC 3561](https://www.rfc-editor.org/rfc/rfc3561.html), July 2003.
Recheck its errata when deriving the tests; verified erratum 7762 corrects wording in RFC §6.7 and
does not resolve the RFC §6.1/§6.6.1 sequence ambiguity. RFC 5148 remains the existing jitter companion;
changing jitter policy is outside this repair unless required to make a corrected deadline valid.

Follow the current [project map](../../doc/project/README.md),
[contribution procedure](../../doc/project/guide/contribute-a-change.md) and
[test derivation procedure](../../doc/project/guide/derive-tests-from-a-standard.md).

Proposed decisions:

- Repair the existing `inet::aodv::Aodv` module through `UdpSocket`, `INetfilter`, `IRoutingTable`,
  existing packet tags, and lifecycle contracts. Keep the protocol-specific mechanisms in
  `src/inet/routing/aodv/`; avoid a general routing framework or new public extension API.
- Preserve the current single-interface configuration and control-message wire formats. Add explicit
  opt-in interface selection for multiple interfaces. Receive nonzero-prefix RREPs correctly;
  automatic subnet-router election/advertisement and cross-prefix sequence reconciliation are separate
  features. Receiving aggregate routes is supported with §6's retained-host limitation and §5's
  unambiguous RERR profile; no static sequence-owner configuration is added.
- Implement local repair in the dedicated final stage in §6, keeping the existing NED default
  `useLocalRepair=false`. With the flag off, retain ordinary invalidation/RERR handling; with it on,
  use the bounded repair transaction and tests specified below. N-flagged RERR reception remains
  supported independently of whether this node initiates local repair.
- In the IPv4 subnet profile, reserve RREQ destination-sequence floors for exact host records. Keep
  aggregate counter history local and apply §6's conservative RREQ/RREP rules. Record this
  restriction and its repair/interoperability limits as model decisions, not new RFC conformance
  assertions. No prefix-provenance field, counter-owner registry or configuration parameter is added.
- Interpret destination RREP generation using RFC §6.1's freshness rule: adopt a fresher requested
  destination sequence when the RREQ marks it known, before replying, without an additional
  unconditional increment. An unknown requested sequence cannot advance local freshness. This matches
  RFC §6.6.1's result for an RREQ exactly one ahead and handles RREQs further ahead. Record the
  contradiction and chosen interpretation in model conformance documentation and in distinguishing
  tests. This is a proposed engineering interpretation, not an assertion that the RFC is unambiguous.
- Limit new RFC conformance assertions to IPv4. Add the bounded IPv6 compatibility cases in §7 for
  shared routing/discovery logic and existing encodings, including existing Hello behavior. Do not
  reject an existing IPv6 configuration merely because the new tests target IPv4. Newly introduced
  multi-interface and aggregate-route features are implemented/tested for IPv4 first; do not silently
  disable existing IPv6 receive behavior or claim new IPv6 feature parity. A concrete incompatibility
  discovered during implementation requires its own documented decision and reproduction.

The source surface is `Aodv.{h,cc,ned}`, `AodvRouteData.{h,cc}`, and, only if internal timer
payloads need changes, `AodvControlPackets.msg` plus regenerated outputs. Multi-interface attachment
may touch `src/inet/node/aodv/AodvRouter.ned`. Serializer tests must cover any representation change;
no wire-format change is planned. Use `WHATSNEW` and the owning user guide for changed observable
behavior/configuration, with migration instructions if configuration compatibility changes.

The current seal registry protects `common/packet/`, not these AODV paths. No edit to that sealed
subtree is planned. Recheck seals and both exception ledgers before implementation. DSDV work and
agent-guidance changes remain outside this task. Use a dedicated AODV worktree for implementation
and preserve any unrelated local edits present when that work begins.

## 2. Correct route selection and lifetime handling

Owner: `Aodv` packet handlers; persistent route facts remain in `AodvRouteData` attached to an
AODV-owned `IRoute`. All destination sequence values use an explicit 32-bit unsigned type.

### Sequence ordering and route identity

Introduce one small protocol-local serial comparison using unsigned subtraction and the signed
32-bit interpretation specified by RFC §6.1, without signed-overflow expressions. Replace ordinary
ordering and `std::max` over sequences in RREQ, RREP, Hello and RERR processing. Equality is distinct
from freshness. Document the exactly-half-range boundary and reject an ambiguous candidate rather
than treating both directions as newer; test that modeling choice explicitly.

Separate finding an exact AODV route record from finding the best forwarding route. A default route
or an unrelated protocol's route must not be rewritten as a host or neighbor record merely because
longest-prefix matching returned it. Retain longest-prefix matching where the RFC specifies subnet
forwarding behavior. This distinction is necessary for both subnet support and the existing gateway
extension; test it with a default route installed.

This table applies only to the **originator route learned from an RREQ** and the **advertised
destination route learned from an RREP**, before committing the candidate path:

| Input/state | Required transition |
| --- | --- |
| No owned destination record | Create a record from applicable packet information |
| Stored sequence unknown | Accept applicable known sequence information; the RREQ U bit is never the validity flag of its originator route |
| Incoming sequence older | Preserve sequence, next hop and metric; do not forward a rejected RREP |
| Incoming sequence newer | Replace the route information using that packet's metric, next hop and interface |
| Equal sequence, shorter candidate | Replace according to the message's route-selection rule |
| Equal sequence, inactive destination route, RREP | Reactivate using the RREP, including an equal or longer metric as allowed by RFC §6.7 |
| Equal sequence, active route, equal or longer metric, RREP | Preserve route and suppress forwarding of that RREP |
| RREQ with no route-selection improvement | Preserve selected path, but apply the independent reverse-lifetime refresh |

The requested destination sequence in a forwarded RREQ can update the outgoing RREQ, never the
stored destination route. For these RREQ-floor operations, local destination knowledge means an
exact owned host record for the requested address; a covering aggregate is excluded under §6.
Treat U and that host record's sequence-valid flag before applying serial ordering; an unknown
numeric field is never an ordering operand.

| Requested destination sequence | Valid exact-host sequence | Forwarded RREQ |
| --- | --- | --- |
| Unknown (`U=1`) | None | Preserve U; ignore the numeric placeholder |
| Unknown (`U=1`) | Known S | Advertise S and clear U, without comparing S to the placeholder |
| Known Q (`U=0`) | None | Preserve Q and U=0; ignore any invalid local numeric value |
| Known Q (`U=0`) | Known S | Keep the serial-newer known value, retaining Q on equality; clear U. At the exactly-half-range boundary, retain Q rather than substituting ambiguous S |

Host-route intermediate RREPs additionally require an active, unexpired, usable exact host route,
a valid sequence and D clear. With U set, there is no requested sequence floor; with U clear, that
host sequence must be equal to or serial-newer than Q. A half-range comparison does not satisfy that
condition. Aggregate intermediate RREPs use only §6's unknown-floor branch; never compare an
aggregate counter with Q. This applies independently of the RREQ originator transition above.
Test both U values against absent/invalid/valid exact-host knowledge, including U=1 with placeholder 0
and S=0xfffffffe, both known wrap directions, equality and half-range.
Observe the forwarded fields, intermediate-RREP decision and unchanged stored destination sequence.

Hello processing uses the exact IP sender's host record; the Hello destination must name that sender.
For a positive-lifetime Hello, create a known active one-hop record if absent. Accept its sequence
without comparison when stored validity is unknown; otherwise accept only equal/newer information.
Older or half-range-ambiguous Hello information cannot change route sequence, path, active state
or expiry; link observation remains independent. An accepted Hello
uses its sender and ingress interface as the direct path. Set its expiry to
`now + max(advertisedDuration, allowedHelloLoss*helloInterval)`, retaining a later expiry only if
the old record was active. An inactive record's deletion deadline is not a usable-route lifetime.
Zero-lifetime Hellos cannot install/reactivate a route or arm a new loss deadline.
RERR has no route-creation or path-replacement transition. First resolve each wire destination to
one owned prefix under §5; only then apply sequence/active-state checks. An ordinary RERR from the
selected next hop invalidates that active record, copies the applicable nonstale sequence and sets
deletion time. RERRs from another next hop or with stale information leave it unchanged. Ambiguous
entries and N-flagged RERRs follow §5. Test these message-specific distinctions independently.

Keep message-specific rules explicit: do not make a universal update helper that silently applies
RREP reactivation exceptions to RREQs. The RREQ reverse-route unknown condition reads the stored
originator route's sequence-valid flag. A previous-hop reachability refresh preserves its known
sequence and sequence-valid flag; a forwarded RREQ/RREP supplies no unrelated sequence for that
neighbor. If the exact previous-hop record is also the RREQ originator or RREP destination record,
the message-specific transition owns its creation, sequence, active flag, metric, next hop and
interface. Evaluate acceptance against that record before any neighbor refresh can change those
fields; apply an already-due expiry transition first. Do not pre-create an aliased unknown neighbor
record or reactivate an aliased inactive record as a side effect of reachability observation.
Rejected stale information may refresh separate link observations, but cannot activate that route.
This needs only handler-local ordering, not another persistent freshness store.

Test sender=originator and sender=destination cases for the path fields as well as sequence
preservation. In particular, an inactive known route to D followed by an equal-sequence RREP sent
directly by D must take the RREP reactivation branch and be relayed when appropriate; it must not
become an active equal-hop route before the acceptance decision. A stale direct RREP leaves it
inactive and cannot enable the data fast path or complete discovery.

`handleRREP()` records whether destination information was created/updated. Only an accepted update
is forwarded. At the originating node, remove the unconditional second installation. The owned route
record remains the sole authority for sequence freshness; do not copy a minimum sequence into the
discovery. Retain its known sequence through expiry/invalidation while discovery depends on it, as
the existing `expungeRoutes()` already does for pending host routes. Extend that retention to a
covering aggregate record when necessary. Discovery records refer to route keys, not borrowed route
pointers. Each cause's sequence action below changes or preserves that one record; later updates
are checked against its resulting value without synchronizing a second freshness store.

A matching route completes discovery only after the applicable selection rule has accepted it and
it is active, unexpired, with a usable next hop/interface. A new route with no prior sequence knowledge
may satisfy discovery under RFC §6.2; do not invent a known-sequence prerequisite for an otherwise usable
one-hop route. Do not expunge or replace a retained record in a way that loses freshness while a
discovery depends on it. Stop/crash clears both route and discovery state together.

After committing a control handler's route changes, check affected searches for every changed route,
including an unaliased previous-hop record created/refreshed from an RREQ or RREP. A usable one-hop
record can complete its neighbor's search even with unknown sequence validity. Check only after
the message-specific alias ordering above, including on duplicate/rejected-message return paths
that already committed an unrelated neighbor change. A rejected advertised destination update
cannot satisfy that destination's search; an independent neighbor update may satisfy its own.
Find affected owners through both discovery targets and the actual-destination FIFO mapping, so a
member update can reach a shared gateway search. A subnet update checks all covered members under §6.
During `RECOVERY_WAIT`, record updates but retain data and the recovery wake-up. Outside recovery,
evaluate eligibility without changing packet tags. On terminal completion of an originating search
or prefix repair, cancel timers/remove
admissions and detach the record and its dependent destination FIFOs into callback-local work.
If only some members become forwardable through independent eligible routes, detach just those
FIFOs and their owner references. Cancel an originating search when no dependent FIFOs remain;
a prefix repair retains its notification obligation even with no members (§6).
For each detached destination FIFO, select its currently eligible forwarding route under §6;
process FIFOs in stable address order and retain insertion order within each FIFO:

1. Recheck the member destination's prefix eligibility and the packet's original caller-specified
   output constraints against the selected route without modifying it. If incompatible, dispose of
   that packet through `dropQueuedDatagram()` and continue;
   no extra per-interface discovery is introduced for this unsupported constrained output.
2. Immediately before releasing an eligible packet, invoke the shared forwarding-preparation function
   to set required output tags and refresh the selected route lifetime, then call
   `reinjectQueuedDatagram()` on that packet. If revalidation finds no eligible output, dispose of
   that packet instead of sending using stale tags; never reattach a completed transaction.

A queued LOCALOUT/PREROUTING packet resumes after its hook and will not revisit the hook automatically.
Each packet is either disposed of or prepared/reinjected once. An incompatible packet cannot block
the rest; remaining eligible packets preserve FIFO order. No queued packet is pre-mutated while a
different packet's eligibility is being decided, so no rollback, reservation or AODV-tag ownership
marker is needed. Document the constrained-output disposal policy and test it with the incompatible
packet first, in the middle and last. RREP-ACK processing remains independent of acceptance (§4 below).

### Lifetimes

Refresh the reverse route on every eligible nonduplicate RREQ using
`max(existingLifetime, now + 2*NET_TRAVERSAL_TIME - 2*hopCount*NODE_TRAVERSAL_TIME)`, independently
of path replacement. Do not shorten an existing expiration when merely refreshing a neighbor.
Make lifetime changes visible to the expiry scheduler.

Store route expiration/deletion as absolute simulation times. For a route-derived RREP, including
`createGratuitousRREP()`, compute `remaining = expiry - now`, then truncate a positive remaining
duration downward to whole milliseconds, matching the existing serializer. Check that the resulting
millisecond count fits `uint32_t` before creating the advertisement. Suppress the advertisement if
the route is expired or the count is zero: 0.999 ms remaining produces no RREP; 1.999 ms advertises
1 ms. If this prevents an intermediate RREP, continue normal RREQ forwarding when its TTL permits.
Apply the same quantization to configured destination/Hello advertisement durations and reject those
configurations if their duration rounds to zero. No rounding parameter is needed.

On ordinary RREP receipt, expiry is `now + advertisedDuration`; Hello uses the rule above. A
zero-lifetime advertisement supplies no usable destination route and cannot complete that
destination's discovery or repair. An unrelated usable previous-hop record remains independent,
as does any required RREP-ACK.
Test late-simulation values, submillisecond boundaries and the largest representable duration.

Use the existing route-state/expiry machinery with these cause-specific sequence actions. For local
expiry/failure actions, an unknown sequence remains unknown; never increment its numeric placeholder.

| Cause | Sequence action | Route-state action |
| --- | --- | --- |
| Ordinary idle expiry of an active route | Preserve the stored value and sequence-valid flag | Mark only that record inactive, set deletion time to `now+deletePeriod`; no link-loss fan-out or RERR solely for idle expiry |
| Confirmed next-hop loss, including an attributed Hello-loss timeout | Increment each newly affected active record's known sequence once, modulo 2^32 | Invalidate the current routes using that link and set deletion time; repair admission follows §6 |
| Undeliverable transit data outside repair, RFC §6.11 case (ii) | Increment the applicable retained record's known sequence once for this failed-data action; no record means no sequence mutation | Keep it inactive and refresh deletion time, then apply ordinary RERR/drop handling |
| Accepted ordinary received RERR | Copy the applicable advertised sequence; no local increment | Invalidate only the uniquely resolved active record and set deletion time |
| N-flagged RERR | Preserve value and validity | Preserve route fields and apply only §5's permitted relay |
| Repeated loss notification for an already invalidated record, or repair timeout | No additional increment | Merge the loss or finish repair under §6; do not restart its deadline |
| Retention refresh while inactive | Preserve value and validity | Change only the deletion deadline under the existing retention rules |

Apply a failure action's sequence mutation before deciding whether its RERR fits the rate budget.
Partitioning, interface copies and precursor fan-out do not repeat that mutation. A later failed
transit packet outside repair is a new case-(ii) action; packets queued for an ongoing repair are not.

Ordinary idle expiry is not evidence that the next-hop link expired. Preserving its sequence is the
chosen interpretation of that distinction in RFC §§6.1/6.11, consistent with the inspected
`expungeRoutes()`; record it in model conformance. Apply the same transition from the expiry timer
and lazily before route selection/update. Thus an idle-expired known route at S can be reactivated
by an otherwise acceptable RREP at S, while a confirmed loss that advanced S to S+1 rejects S.
Test both expiry-timer/RREP orders at the exact deadline, wraparound, unknown sequence validity,
and inactivity while discovery retains the record. Pending discovery delays deletion, not expiry.

## 3. Make discovery progress through rate limits and timeouts

Owner: `Aodv`, with one map of search records and one FIFO of ready search keys. A key contains an
address family, normalized address and prefix length. An `ORIGINATING` search uses a full-width host
key for its discovery target, after any external-gateway translation. A `LOCAL_REPAIR` search uses
the exact retained route prefix; host repair therefore shares a key with originating discovery to
that host. A record also holds its actual RREQ target address (`wireTarget`), purpose, phase, next
TTL, network-wide attempt count and retained route key where applicable. An aggregate repair's
first triggering member supplies `wireTarget`; its key remains the aggregate prefix. The synthetic
external-gateway /0 adapter is ineligible for local repair; it retains ordinary failure handling and
P1's originating gateway discovery. At actual send, construct the RREQ for `wireTarget` from its
current exact host sequence or U, applying §6's aggregate-search overrides. A retained aggregate is
still the authority for checking updates to its own prefix; its sequence is never copied into an
unlabelled RREQ floor. The synthetic /0 adapter supplies no RREQ floor. The search stores no
second sequence value.

Each actual data-destination FIFO has exactly one owning search key, using the same dependency
mapping needed for gateway queues in P1. A search may serve several FIFOs. Joining traffic changes
neither purpose nor `wireTarget`, retry progress or deadlines; §6 defines originating/repair
admission. Use the existing counter timer for the ready FIFO. Every live record has a scheduled
phase timer or a FIFO entry served by the next counter tick; local repair also has its total
deadline in §6. Success, final failure, stop and crash remove all those obligations and every
dependent FIFO reference. No second scheduler or per-packet transaction framework is needed.

| Phase/event | Action and next phase |
| --- | --- |
| New attempt or RREP timeout with attempts left | Sample bounded jitter once; schedule the search timer for `now+jitter`, phase `SEND_DELAY` |
| `SEND_DELAY` timer fires | Append search key once to ready FIFO, phase `READY`; drain available budget |
| Budget exhausted | Keep `READY` FIFO entry; existing one-second counter timer will drain it |
| Ready FIFO head admitted | Construct RREQ and increment ID/sequence now, consume one budget slot, send immediately; schedule full RREP interval, phase `WAIT_REPLY` |
| Route update satisfies the search's terminal condition outside `RECOVERY_WAIT` | Cancel search timer/remove FIFO entry, detach record and complete its dependent FIFOs once |
| Independent eligible route releases some destination FIFOs | Detach/release those FIFOs only; cancel an empty originating search, but preserve a prefix repair's phase and deadlines (§6) |
| Qualifying route update during `RECOVERY_WAIT` | Retain learned route, data and recovery timer; no reinjection or send |
| Final RREP timeout | Detach record and fail all its dependent FIFOs once; local-repair notifications follow §6 |
| Reboot hold applies | Phase `RECOVERY_WAIT`, search timer at recovery deadline; when extended, reschedule it |
| Recovery timer fires | Recheck the current recovery deadline first; if still held, reschedule; otherwise recheck route usability and complete if possible, entering `SEND_DELAY` only when discovery is still needed |

Before each search wake-up, ready admission or terminal timeout, recheck current route eligibility
under §2 before sending another RREQ or dropping data. This also observes legitimate non-AODV routes
added while data was queued; no new routing-table subscription or scheduler is needed. Apply recovery
hold before release/send, but recompute the freshness reference under §6 even while held, before
returning to `RECOVERY_WAIT`. Retain prefix-repair obligations after independent member release.

`startTime` is this operational interval's start time, set on every start/restart together with zero
counters and window index zero; it is not a new NED parameter. Counter ticks and window boundaries
are `startTime + k*1s`. Before every FIFO drain or RERR budget check, reset counters if the computed
window index advanced, even when its counter event has not run yet. Both paths use the same index,
so a late counter event cannot erase sends already counted in the new window.
Budget is consumed only by actual logical transmission, after jitter, so crossing a window requires
no reservations. FIFO order is readiness-event order, including OMNeT++ event order for exact ties;
newly ready search keys cannot overtake entries already waiting. A late counter tick drains the
same FIFO. Joining another packet or destination FIFO creates no additional admission entry.

- Honor a first-attempt TTL supplied from an invalid route's last hop count plus `ttlIncrement`.
  Use `ttlStart` only without retained information. Clamp to `netDiameter`. A retained-hop start
  above `ttlThreshold` but below `netDiameter` is one initial ring at that supplied TTL; after its
  timeout go directly to `netDiameter`. Otherwise increment rings through the threshold when reached
  exactly, then go to the network diameter once the next increment exceeds it. An initial TTL equal
  to the diameter is the first network-wide attempt, not a ring with a separate waiting interval.
- Use `2*nodeTraversalTime*(TTL+timeoutBuffer)` for expanding rings. The first network-wide attempt
  waits `netTraversalTime`; subsequent network-wide attempts double that duration. Count
  `rreqRetries` as additional network-wide sends after the first. A rate-limited deferral is not
  a transmitted retry and consumes neither a retry nor a new RREQ ID/sequence increment.
- RREP waiting starts at the actual send in the table above. Rate-limit queueing and jitter neither
  consume a retry nor shorten that waiting interval.
- On success detach the discovery state before reinjecting its packets, so reinjection cannot
  observe a completed transaction as still pending. On final failure remove admissions/timers and
  dispose of all dependent buffered packets through `INetfilter::dropQueuedDatagram()` exactly once.
  The inspected IPv4 implementation deletes the queued packet without application notification.
  This repair deliberately retains that disposal-only outcome: no new socket/ICMP callback or
  synthetic application error is introduced. Record the unmet RFC §6.3 recommendation for an
  application Destination Unreachable indication in model conformance and user-facing limitations.
  Test final exhaustion for packet destruction, no reinjection and complete discovery cleanup;
  application-level timeout/error behavior remains owned by the existing transport/application.
- Apply the explicit parameter constraints below; reject arithmetic that cannot fit simulation time
  or the relevant wire field before scheduling or serialization.

### Parameter bounds and blacklist duration

| Parameters | Constraint for this repair |
| --- | --- |
| `ttlStart`, `ttlThreshold`, `netDiameter`, `ttlIncrement` | `1 <= ttlStart <= ttlThreshold <= netDiameter <= 255`; `1 <= ttlIncrement <= 255`; clamp retained-hop initial TTL before use |
| `rreqRetries`, `timeoutBuffer` | Nonnegative integers; zero retries is valid and still permits the first network-wide attempt |
| `rreqRatelimit`, `rerrRatelimit`, `allowedHelloLoss` | Positive integers |
| Traversal, active-route, Hello, delete, discovery, RREP-ACK and blacklist durations | Finite and positive; evaluate all sums/backoff within simulation-time range |
| `activeRouteTimeout` with Hello enabled | Strictly greater than `allowedHelloLoss*helloInterval` |
| `deletePeriod` | At least `activeRouteTimeout` when link feedback is used, and at least `allowedHelloLoss*helloInterval` when Hello loss is used; enforce both when both mechanisms are enabled. Retain the default `5*max(activeRouteTimeout,helloInterval)` |
| `myRouteTimeout` | At least `2*pathDiscoveryTime`; adjust the default to `max(2*activeRouteTimeout, 2*pathDiscoveryTime)` and document the observable correction |
| Broadcast jitter | `maxJitter >= 0`; each sampled `jitter` in `[0,maxJitter]`; reject overrides violating the declared bound |
| Periodic jitter | `0 <= maxPeriodicJitter <= helloInterval/2`; each sampled value in `[0,maxPeriodicJitter]` |
| RREP advertisement durations | Truncate downward to whole milliseconds as in §2; outgoing counts must be in `1..UINT32_MAX`. Validate configured destination/Hello durations before use; suppress route-derived advertisements that round to zero |

Use the conservative default blacklist duration
`B = max(2*nodeTraversalTime*(netDiameter+timeoutBuffer),
         R*2*nodeTraversalTime*(ttlThreshold+timeoutBuffer))
     + netTraversalTime*(2^(rreqRetries+1)-1)
     + (R+rreqRetries+1)*maxJitter`,
where `R = 1 + floor((ttlThreshold-1)/ttlIncrement)` bounds the count of expanding rings for any
start at or below the threshold. The first term bounds either one above-threshold initial ring or
the entire sequence of at/below-threshold rings, since those cases cannot occur in the same search.
Overcounting a threshold equal to the diameter is deliberate and safe.
Compute using checked arithmetic; the retry count is supported only when this bound is representable.
Use this as the NED default expression; keep positive explicit overrides as user-selected durations.
It covers the configured unthrottled search, not arbitrary rate-limit queue delay. Tests cover the
default, zero retries, explicit override and overflow rejection. Include the retained-hop counterexample:
`ttlStart=ttlThreshold=ttlIncrement=1`, `netDiameter=255`, retained hops 200, `rreqRetries=0`,
`nodeTraversalTime=40ms`, `timeoutBuffer=2`, zero jitter. The TTL-201 ring plus diameter wait totals
36.64 s; the revised bound is 40.96 s. No finite discovery-latency guarantee is made under unbounded
offered load. The formula is a bound for this plan's schedule, not an RFC-prescribed equality.

Preserve FIFO order within each destination queue. Completion by a subnet route must satisfy all
covered pending destinations whose freshness requirements are met, not only the address in the RREP.
External-gateway queue handling is the separately assigned prerequisite P1 in §8.

## 4. Scope RREP-ACKs and neighbor reachability correctly

RREP-ACK handling belongs to link validation, independent of whether an RREP improves a route or will be
forwarded. A valid received A-flagged RREP produces a one-hop RREP-ACK to its sender, including at
the RREQ originator. Clear the received A bit before any forwarding; decide the outgoing link's
RREP-ACK requirement separately.

Replace the single `failedNextHop` with pending RREP-ACK state keyed by neighbor and interface. An RREP-ACK
clears only its matching pending validation. Use one map of link keys to deadlines and one timer at
the earliest deadline. Multiple RREPs to one neighbor share the first outstanding deadline; later
sends do not postpone it. Set the deadline at actual send time and reschedule the shared timer after
every map change. At expiry, remove all due entries and blacklist only those links. An unsolicited
or late RREP-ACK with no pending entry is ignored and does not reactivate routes or clear a blacklist.
For an RREP-ACK and timeout at exactly the same time, OMNeT++ event order decides: RREP-ACK processed first
clears validation; timeout processed first blacklists and the following RREP-ACK is late. Test both orders.

Confirmed failure of a transmitted unicast RREP immediately adds/extends that link's blacklist expiry to
`now + blacklistTimeout`, whether or not an RREP-ACK was requested, then removes any matching pending RREP-ACK
entry. A confirmed link loss from another cause resolves an outstanding RREP-ACK validation on that link
as failed before removing it; without a pending validation it only performs ordinary link-loss
handling. Neither path waits for, or leaves behind, an RREP-ACK timer that can repeat the outcome. An RREP-ACK
received after this failure cannot undo the blacklist. Blacklist expiry removes only the expired
blacklist entry; it does not cancel a newer pending validation. Stop/crash clears both stores. Test
failed unicast RREPs with and without A, another link's independent validation, and a late RREP-ACK after
failure. A broadcast failure alone does not identify a peer to blacklist.

Learn interface-qualified IP/MAC bindings from received one-hop AODV messages regardless of
`useHelloMessages`. Require `InterfaceInd`; when `MacAddressInd` identifies the immediate IP-hop
sender, bind `(address family, ingress interface, IP source)` to its source MAC. These bindings are
local observation state, independent of route sequence validity and Hello deadlines. Retain them
through route expiry until stop/crash, so an in-flight transmission remains attributable after a
route changes. Do not learn a transit packet's IP source as its previous hop or use the
interface-ambiguous `IArp::getL3AddressFor(MacAddress)` reverse lookup.

Use the inverse binding only when a MAC identifies one IP neighbor on that interface and in that
address family. If a learned IP changes MAC, or a MAC is claimed by a different IP, mark all affected
MAC bindings unusable until restart, retaining that mark even if the route expires. This prevents a
late failure for an old binding from being assigned to its replacement. Missing metadata learns no
binding; it does not authorize a guess. Stop/crash clears bindings and unusable marks.

Attribute `linkBrokenSignal` using the failed packet's retained `InterfaceReq` and destination
`MacAddressReq`, then resolve that interface/MAC through the unique usable binding. IPv4 consumes
`NextHopAddressReq` before MAC delivery; the current best route to the packet's destination is not
evidence of the attempted next hop. AckingMac retains these tags on its original transmit packet;
nonaggregating IEEE 802.11 DCF retains them across encapsulation/retries. Establish retention and
immediate-peer meaning in those production-path test configurations; aggregation and other profiles
need their own evidence. If tags or a unique binding are unavailable, or the target MAC is
broadcast/multicast, skip peer-specific failure handling and diagnose the coverage gap once per
interface per start/stop interval. Leave pending RREP-ACK validation to its existing deadline. This path
works with Hello disabled and requires no new packet metadata or transmission registry.

An attributed failure affects only this module's current route records whose next hop and interface
still match the failed link. If data for D was sent via N, then D acquired a route via M before N's
failure report, preserve D's replacement route and M's RREP-ACK/blacklist state; other routes still using N
remain affected. Start optional repair for D only if its current owned record still uses the failed
link and satisfies §6 admission. Never start repair from superseded path information.

For `useHelloMessages=true`, keep last-heard neighbor deadlines separately from route-use lifetimes.
Otherwise outgoing traffic could keep an absent neighbor alive indefinitely. Apply this observation
contract to both IPv4 and IPv6 prerouting through the shared AODV netfilter hook:

- An attributable positive-lifetime Hello may arm its neighbor's loss deadline while the interface's
  observation contract remains satisfied, using the bindings learned above.
- In prerouting for either address family, match `(address family, ingress interface, source MAC)`
  to those learned link records and refresh their deadlines. Never use a transit datagram's IP
  source as its previous hop. This handles `S -> N -> us`: data sourced by S refreshes N, not S.
- The required contract is an ingress interface plus a source MAC identifying the immediate IP-hop
  sender, consistent between its one-hop AODV packets and its data. Direct AckingMac, independent
  IEEE 802.11 and Ethernet are test configurations for that contract, not an allowed-type whitelist.
  Custom interfaces providing the same contract need no AODV dependency on their concrete type.
  These are proposed test configurations; their production paths must establish the contract before
  the model claims Hello-loss coverage for them.
- Require `InterfaceInd` on received AODV messages; use `MacAddressInd` when available. Without
  reliable previous-hop metadata, retain existing Hello exchange and link-feedback behavior; do not
  enable the new data-refreshed loss deadline or falsely expire a live neighbor from Hello silence
  alone. Emit a diagnostic for the attribution coverage gap, not a configuration error. An IP source
  from a one-hop AODV packet can still refresh its own known neighbor; a transit IP source cannot.
  Do not inspect Wi-Fi internals or infer a previous hop from a reverse route. Infrastructure/WDS
  attribution requires its own evidence before claiming this loss-detection coverage.

The supported profile has stable sender attribution for each selected interface throughout one
start/stop interval. Keep optional per-neighbor deadlines and one disable flag per interface, reset
on start. An attributable positive-lifetime Hello arms only its sender at
`now + allowedHelloLoss*helloInterval`.
Attributed data or other one-hop control refreshes an already armed neighbor; data alone never arms
one. Valid metadata from a sender with no learned binding changes no deadline.

If a received IPv4 or IPv6 packet on a selected interface lacks usable sender metadata, disable this
loss detector for that interface until restart, cancel its armed deadlines and emit one diagnostic.
A changed or ambiguous binding as defined above likewise leaves the supported stable-attribution
profile and disables this detector until restart. A packet without sender metadata cannot identify
which neighbor may have been heard, so retaining those deadlines could falsely expire a live
neighbor. Hello exchange and route updates continue; MAC feedback remains available for uniquely
attributable links. Later data or Hellos do not automatically re-enable this detector; restoring its
observation contract requires restart.

Use one earliest-neighbor-deadline timer and recompute it after refresh, cancellation or expiry.
Metadata failure/refresh at the same time as expiry follows event order: processed first it
cancels/postpones the deadline; processed after expiry it cannot undo the emitted link-loss action.
Test both orders and the restart reset. Tag presence alone does not establish correct attribution in
arbitrary bridging/WDS topologies; those profiles require separate evidence.

Read indication tags without altering borrowed packets; learn bindings only from one-hop control
senders. On expiry, run the same dependent-route invalidation/RERR path as confirmed link breakage.
A Hello-created neighbor route with no dependent precursors must not cause an unnecessary RERR flood. Keep MAC
feedback working with Hello disabled. No changes to Wi-Fi MAC/PHY behavior are required.

## 5. Restore transit failure and reboot semantics

### Transit packets and RERR

Classify recognized AODV control datagrams before data admission in both LOCALOUT and PREROUTING,
using their UDP port, address family and control-packet header. Share that recognition with the
link-failure path in §6. Control datagrams never enter a data FIFO, `RECOVERY_WAIT`, originating
discovery or local repair. Received control follows the IP delivery/control-handler path; invalid
or misdirected control is not repaired as transit data. Enforce reboot restrictions in the control
generation/forwarding path itself. Permitted locally generated RERRs and one-hop RREP-ACKs bypass
the data gates, with their selected outgoing interface/next hop supplied through existing output
tags; their send must not require an active destination route. The RERR budget still applies.

Keep local-out discovery separate from prerouting transit decisions. Prerouting is needed because
IPv4 can fail route lookup before invoking the forward hook. In prerouting, pass local delivery and
broadcast/multicast normally; for unicast transit without a usable route, buffer only for an eligible
or ongoing local repair when enabled (§6). Otherwise send the applicable RERR, update invalid-route
retention and drop the packet through the hook contract. Do not start ordinary originating discovery
for transit data or forward an inactive AODV route. Preserve forwarding using legitimate routes
owned by another protocol, particularly at an external gateway. Keep active-route lifetime refresh
in the appropriate forwarding/local-in paths without double counting.

Keep an outgoing RERR's address and counter attached to the same record. If a retained aggregate
itself is unusable, report its normalized prefix and that record's sequence, including for later
failed member data outside repair. For a host-specific failure, use its exact host record's sequence
or zero when unknown/absent; never label an aggregate counter with a member's host address or mutate
the aggregate for an isolated host failure. Prefix-scoped case-(ii) reporting is an explicit subnet
profile policy extending RFC §6.11's host wording. Apply the same ownership to precursor selection.

N-flagged RERR reception preserves route validity, sequence, metric, next hop and expiry. Relay the
notification to affected precursors only when it came from the selected next hop, preserving N.
Both ordinary and N-flagged entries must first pass this prefix-resolution rule:

- For each wire destination, find distinct owned route keys with that normalized address, address
  family, stored next hop equal to the sender, and stored interface equal to ingress. Exclude the
  synthetic gateway /0 adapter. Include active and retained inactive records in this candidate set.
- With no candidate, ignore the entry. With exactly one, apply that record's ordinary or N-flagged
  rules. With several distinct prefix keys, skip that entry: no sequence, validity, expiry, repair
  or queue-state changes, and no relay to any candidate's precursors. Process other entries normally;
  emit one diagnostic for ambiguous entries in that received packet.
- Resolve identity before filtering on active state or sequence freshness. An inactive record still
  witnesses ambiguity. Never choose a prefix by longest-prefix lookup or by comparing independent
  counters, and never infer a prefix from the sender's private repair key.

Full RERR coverage in this subnet profile requires one fixed prefix identity for each
`(address family, peer, interface, normalized wire address)` throughout the run. For example,
`10.0.0.0/24` and `10.0.0.0/25` through the same peer/interface fall outside that profile. This is a
topology/profile assumption matching RFC §6.11's compatible-prefix premise, not something current
route lookup can prove. Reassigning the tuple to another prefix after deleting an old record is
also unsupported: a delayed RERR still carries no prefix length. Document the limitation; the
ambiguity guard supplies conservative handling for observable violations, without a history registry
or wire extension. Other peers/interfaces can distinguish otherwise equal wire addresses.

Ordinary RERR processing then uses §2's cause-specific sequence action, freshness checks and rate
budget. RERRs and link failures use precursor sets to select destinations and interfaces.
Invalidation must still happen when an RERR send is rate limited. RERR list partitioning is the separately assigned
adjacent fix P2 in §8; it does not expand the route-transition rules.

For RERR admission, a partition of at most 255 unreachable records is one logical control message.
At its actual send time, after any jitter, check/reset the window under §3 and charge one RERR budget
slot for that partition. Then deliver all its required precursor/interface copies without further
budget charges, including iterative unicast as described in RFC §6.11. If no slot is available,
suppress the whole partition; do not send to only the first precursors that fit. Local invalidation
covers the entire affected set regardless of admission. Apply the same unit to N notifications and
relayed RERRs. Use the existing tracked delayed-send mechanism; no deferred retry queue is added.

### Reboot

Represent initial startup separately from restart after sequence loss, including a restart at time
zero. On restart, hold a recovery deadline of `now + deletePeriod`. Accept control information into
state as appropriate but prohibit forwarding control packets and originating discovery during the
hold. Suppress RREP/Hello advertisements that would publish reset freshness prematurely.
Transit data triggers the permitted RERR and moves the deadline to `now + deletePeriod`. Receiving
an A-flagged RREP may still produce its one-hop RREP-ACK; these two permitted control outputs use the
bypass above and do not extend recovery or create data discovery state merely by being sent.
Keep locally generated data in a deferred discovery state that wakes when recovery permits it,
including packets whose destination already has a usable learned route. Recovery admission precedes
the ordinary active-route fast path and any discovery-completion callback. On hold expiry, recheck
the retained route before originating an RREQ; a qualifying update received during the hold must
allow direct completion then, without an unnecessary RREQ. If the deadline is extended, no pending
wake-up may release data based on its old value. For data arriving exactly at expiry, use the current
deadline and normal OMNeT++ event order, testing both extension/wake-up orders.
Fresh RREQs addressed to this node can restore its sequence under the documented interpretation.

Stop/crash cancels discovery, neighbor, RREP-ACK, blacklist, expiry and delayed-send events. Track delayed
packet holders so no control packet from before restart escapes afterward. Dispose of queued
datagrams through their real owner, respecting lifecycle ordering if IPv4 already removed them;
never use stale borrowed packet pointers. Remove only this module's routes and clear gateway-route
references, neighbor bindings and attribution-disable state. Validate down-at-initialization,
shutdown, crash and restart separately.

## 6. Support selected interfaces and received subnet routes

### Interfaces

Retain `interface` as the primary-address/legacy selection parameter. Add an optional `interfaces`
selection parameter, empty by default, meaning the legacy interface alone. For a nonempty selection,
resolve the explicit configured set, require that it contains the primary interface, and reject
unsupported address-family mixtures. Use one node originator identity, RREQ-ID stream and sequence
stream; do not create a separate routing process per interface.

Use the primary selected address only for originating a discovery. Treat every address on a selected
interface as a local destination: an RREQ for a secondary address produces an RREP naming that
requested address, using the shared node sequence after applying the destination-sequence rule.
Perform local-address matching before intermediate-RREP logic. Reject duplicate local IPv4 addresses
across selected interfaces in this profile. RREQs whose originator is any selected local address
are self-originated and are suppressed, independently of the return interface and duplicate cache.

Send one Hello per selected interface when that link needs a Hello: its IP source and RREP destination
are that interface's address, its hop count is zero and it advertises the shared sequence. Do not send
a primary-address Hello on a secondary link and install an unreachable primary address as a direct
neighbor there. Keep last-broadcast suppression per interface. RREPs for primary-originated
discoveries terminate at the primary identity even if received on a secondary interface.

Carry ingress `InterfaceInd` into control handlers. Store each learned next hop's interface and use
it for unicast control/data forwarding. Ignore AODV control received on an interface outside the
configured ad-hoc selection before learning bindings or changing routes, RREP-ACKs, blacklists or search
state. Missing ingress metadata cannot select a default interface for such control. This boundary
does not prohibit legitimate data forwarding to/from an external gateway interface.
For broadcast discovery, duplicate the logical RREQ across
selected interfaces with their proper source addresses; preserve its originator identity and ID.
Duplicate suppression remains by originator and RREQ ID. The rate budget counts a logical originating
RREQ; all its interface copies are one attempt. RERR propagation selects interfaces with
affected precursors. Use existing interface/source-address tags rather than new metadata types.
Qualify pending RREP-ACK, blacklist, precursor delivery and neighbor-liveness state by link identity.
Gate this profile on IPv4, with at least one enabled nonloopback interface and one usable unicast
address per selected interface. Define `interfaces` as a whitespace-separated list of exact names;
reject unknown/duplicate names, missing primary membership and down or unaddressed selections at
startup. Interface removal/address reassignment during operation is unsupported in this repair;
stop/reconfigure/start is the supported transition. Link loss on a retained interface remains supported.

### Subnet RREPs

Read `prefixSize` when installing an advertised route. Zero retains host-route semantics; nonzero
installs the normalized network prefix with the advertised length and stores sequence/precursors
against that prefix. Do not reinterpret zero as a default route. Use exact prefix ownership for
updates and longest-prefix matching for forwarding, including coexistence with a more-specific
host route. A subnet RREP to one host must allow a second host in that subnet to use the route.
The supported prefix combinations must also meet §5's stable, unambiguous RERR-identity assumption;
the forwarding rules below do not claim full RERR propagation for colliding wire addresses.

Retained inactive records remain protocol freshness state and may still reside in the IPv4 table;
IPv4's native lookup does not filter their AODV active flag. The AODV adapter must therefore make
one forwarding decision used by local-out admission, transit admission and discovery completion:

| Overlapping records for a target | Forwarding/completion decision |
| --- | --- |
| Active, unexpired more-specific route | It wins over the aggregate; do not redirect its packets to the aggregate |
| More-specific AODV route has expired | First perform its ordinary invalidation/retention transition, then handle it as inactive |
| Inactive more-specific AODV record with a known sequence | Keep the record; the aggregate cannot satisfy this target while that record is retained. Do not compare counters across prefixes |
| Inactive more-specific AODV record with unknown sequence | Keep the record; it imposes no sequence floor on the covering active aggregate |
| Target blocked by retained known state | Retain that record and construct discovery under the RREQ policy below; an acceptable update can satisfy it under this table. Other covered targets may use their own eligible routes |
| No eligible aggregate or host route | Use ordinary missing/inactive-route discovery or transit failure handling |

Serial ordering requires a shared counter. RFC 3561 §7 requires a sequence number for an entire
subnet; it does not establish that this counter is the router's host counter or another subnet's
counter. The RREP wire format provides no declaration that counters for different prefixes are
shared. A common advertising router, next hop or numerical value is insufficient evidence. This
repair therefore checks stored route history only within an exact normalized prefix and adds no
owner map. Same-prefix operation assumes consistent subnet-router semantics under
RFC 3561 §§7–8; live handover between independent subnet counters remains outside this profile.

RREQs carry no prefix identifying the destination-sequence counter. The supported subnet profile
therefore uses this explicit convention: a known RREQ floor belongs to the requested host, while
aggregate history stays in its exact local route record. Only exact host knowledge can fill an
unknown floor or raise a known floor under §2. A covering aggregate neither clears U nor compares
its counter with Q. This convention is a conservative model restriction on RFC §§6.3–6.6 aggregate
discovery, not a claim that the RFC specifies host-only floors. Interoperation with peers exporting
aggregate counters in RREQs is outside this profile: their provenance cannot be recovered from the
wire. Document that limit in model conformance and the user guide; no runtime inference is attempted.

| Search/history at actual send | Originated RREQ fields |
| --- | --- |
| Originating search, known exact host record | U=0, that host's current sequence, configured D |
| Originating search, neither a known host nor a retained known aggregate | U=1, numeric placeholder 0, configured D |
| Originating search, retained known aggregate and no known host | U=1, numeric placeholder 0, D=1 |
| Host local repair | Known sequence: U=0 and the host value after invalidation increment; unknown: U=1 and placeholder 0; configured D |
| Aggregate local repair | U=1, numeric placeholder 0, D=1; retain the incremented aggregate sequence only locally |

For an originating search, select its most-specific retained known nondefault record as local
freshness authority; use it to derive the table above without storing another floor or flag. D=1
bypasses cached RREPs when aggregate history cannot be expressed as an RREQ floor. Relays
preserve D and apply only exact-host floor promotion. An authoritative subnet router's ability to
answer for its members is outside this implementation; the subnet test peer must supply it explicitly.
If no authoritative RREP is available, the existing bounded search fails normally.

An intermediate may answer from its selected eligible aggregate only with U=1 and D clear, a valid
aggregate sequence and a positive representable remaining lifetime. With U=0, forward the RREQ
without comparing its floor to the aggregate or replying from that aggregate. Host-RREPs
continue to use §2. For an allowed cached aggregate RREP, keep the requested destination address
in the RREP, set `prefixSize` to the stored prefix length, and take sequence, metric and remaining
lifetime from that same prefix record. Add precursors to that record. Never emit `prefixSize=0`
with an aggregate counter. Host RREPs and the gratuitous RREP about the exact RREQ-originator
host explicitly use zero; forwarding a received subnet RREP preserves its nonzero prefix size.
This bounded cached response does not implement authoritative subnet-router advertisement/election.

RREQ construction does not pin completion to the retained record's prefix. Accept each update
against the existing record for its own exact prefix, then apply the forwarding table above to the
target. The same eligibility predicate governs new data, originating-search completion and release
of a repair member FIFO. A newly accepted active /32 can therefore complete an originating search
that retained a /24, just as it can release a member of aggregate repair; its sequence is checked
only against its own /32 history. Conversely, an active /24 cannot bypass a retained known inactive
/32. No sequence comparison is made between the two prefixes.

After each relevant route change and before a search callback, recompute the originating target's
most-specific retained known nondefault record and update its retained route key before completion
or recovery deferral. This includes a known aggregate replacing an unknown host as freshness
authority, as well as a known /32 replacing a /24. Keep the selected record while depended on;
the old record follows its normal lifetime unless another search retains it. Search key, purpose,
`wireTarget`, retry progress and deadlines remain unchanged; no second sequence store is added.
Test queued and new data together: an accepted /32 releases originating data exactly once and
cancels later RREQs; the repair-member case releases only that member. During recovery, keep the
selected /32 or aggregate's sequence history past its otherwise-due deletion and defer release.
An older same-prefix RREP still fails after that deadline. A known inactive /32 remains the
authority and cannot be bypassed by a /24.

Document and test this conservative limitation: router R's Hello creates a /32 record; a link loss
invalidates it; a later /24 RREP cannot substitute for the retained known /32, even if R advertises
both and the /24 number is larger. A sufficiently fresh /32 RREP can recover immediately; otherwise
the /32 must reach its deletion deadline after discovery terminates before the aggregate can serve
R. Other unblocked members may use the /24 immediately. Substitution of a less-specific route across
retained known more-specific state, including a profile that deliberately shares a host/subnet
counter, needs a separate design and is not claimed here.
Respect active non-AODV route precedence.

Deletion is not an eventual-progress guarantee: traffic for an invalid route refreshes retention,
and repeated discovery retains its freshness record. With continuing demand, the /32 may therefore
block the aggregate indefinitely. Test sustained demand through multiple search/deletion intervals,
then quiescence long enough for ordinary deletion and subsequent aggregate use. Keep the RFC's
retention rules; this plan adds no forced-deletion timer to escape that limitation.

When the selected eligible aggregate would otherwise be masked by an inactive AODV record, set the
existing `InterfaceReq` and `NextHopAddressReq` on the admitted IPv4 data packet to the selected
aggregate's interface/next hop. `Ipv4::routeUnicastPacket()` honors this explicit decision and avoids
its native lookup; no change to IPv4 or a new routing-table class is required. Invoke the same
preparation function before returning ACCEPT from local-out/prerouting and, independently, from
discovery completion immediately before each reinjection. Preserve caller-specified output decisions;
do not overwrite an independent explicit output request. Forwarding lifetime/precursor updates must
use the selected aggregate key, not redo a lookup that selects the inactive host record. Keep the
host record until its normal deletion deadline (extended while depended on by discovery), so using
the aggregate does not erase freshness. If the aggregate later fails or expires, re-evaluate using
that retained state. Test actual emitted next hop/interface, not merely successful discovery.

For ICMP Host Unreachable feedback about one member, preserve the aggregate route as required by
RFC §7. Keep error delivery in the existing IP/transport path; do not translate that feedback into
an aggregate RERR or link-loss action. Inject the error with a quoted member datagram through IP
delivery and assert that the aggregate remains usable; no new AODV ICMP observer is required.

This adds receiving/using subnet advertisements and the constrained cached/forwarded RREPs above;
authoritative subnet-router election and automatic advertisements remain explicitly unsupported.
The existing configured external gateway/default-route extension is an INET behavior, not an
RFC-defined default-route algorithm. Keep it functional and prevent its /0 record from masquerading
as an exact neighbor/subnet record.

### Dedicated stage: implement local repair

Implement RFC 3561 §6.12 after the core correction stages; keep `useLocalRepair=false` as the default.
Read the flag in `Aodv::initialize()` and remove the NED claim that the enabled mode is unimplemented.
Use the existing route repair flags and discovery/queue infrastructure, adding a discovery purpose
(`ORIGINATING` or `LOCAL_REPAIR`) and the old route hop count. A repair belongs to the exact retained
route prefix, including for an aggregate, and owns that record's repair flags and deadlines. Eligible
member destinations join its dependent FIFOs; they do not create more repairs or sequence increments
for that prefix. Use the keys and queue-dependency mapping in §3. Validate the enabled mode through
IPv4 production paths and IPv6 compatibility cases for success/failure.

Apply these admission rules before creating a search; normal event order decides simultaneous arrivals:

| Existing state and arriving data | Admission and search ownership |
| --- | --- |
| `ORIGINATING` already owns this destination FIFO; local data arrives | Join that FIFO; preserve its purpose, retry schedule and deadlines |
| An `ORIGINATING` search already depends on the retained prefix; transit data for that prefix becomes undeliverable | Decline optional repair and perform ordinary RERR/drop for that transit packet. Do not queue it, convert a search or start a competing repair for another member. Preserve all originating FIFOs and searches |
| `LOCAL_REPAIR` owns the eligible retained prefix; local or transit data for an unowned member arrives | Attach that member FIFO to the existing repair; preserve its original TTL, `wireTarget`, retry progress and deadlines. Apply the retained-prefix/output rules before release |
| No search owns the destination and no eligible repair covers it; local data arrives | Start `ORIGINATING`, including when an inactive route is marked repairable |
| No originating search depends on the retained prefix; eligible undeliverable transit data arrives | Join the existing prefix repair, or start its single `LOCAL_REPAIR` record if absent |

An originating search that already depends on a prefix takes precedence over starting repair for
any member of that prefix. This preserves its existing retry budget and avoids overlapping search
purposes for the same retained route. A search for a separately retained more-specific record does
not block repair of other eligible aggregate members, and its FIFO remains independent. Only the
repair owner clears its prefix's repair flags or generates repair notifications. On repair failure,
dispose of all FIFOs attached to that repair, including locally generated packets, without automatic
conversion to originating discovery. A later local packet may start a new originating search after
cleanup. Test local-first and transit-first arrival orders for host and aggregate routes.

If an independent eligible route makes a member forwardable, release just that FIFO under §2.
This includes a host route during aggregate repair and an aggregate during host repair when the
retained inactive host sequence is unknown. A known inactive host still blocks the aggregate.
Only an update to the owned repair prefix can finish the repair itself. Keep its flags, remaining
members and original deadlines, even with no remaining FIFOs: the bounded precursor-notification
obligation survives. Release uses the selected route's own freshness/precedence; released packets
cannot be disposed of again by prefix timeout. No purpose conversion or deadline extension occurs.

Add `maxRepairTTL` with default `floor(0.3*netDiameter)` and range `0..netDiameter`; zero makes all
destinations ineligible. Keep `localAddTTL` with its existing default 2 and require it nonnegative.
`MIN_REPAIR_TTL` is the retained pre-break destination hop count, not a second user setting.
For an eligible transit destination with old distance at most `maxRepairTTL`, use
`max(oldDestinationHopCount, ceil(hopsToDataSource/2)) + localAddTTL` as the initial RREQ TTL.
Obtain `hopsToDataSource` from the active reverse route to the triggering data source. If the
required old route/reverse distance is unavailable, or the computed TTL exceeds `netDiameter`/255,
decline optional repair and perform ordinary failure handling; do not invent a distance or silently
truncate the formula. Local data never initiates repair; it follows the admission table above.

Classify the failed packet before repair eligibility, reusing the control recognition in §5 rather
than a packet name. `linkBrokenSignal` reports MAC failures for both data and control traffic; a
network destination alone does not make the packet repairable data. Only undeliverable transit data
from a link failure or a deferred repairable route can initiate `LOCAL_REPAIR`. A failed
RREQ/RREP/RERR/RREP-ACK must not initiate local repair or originating discovery for the control
destination. Dispatch it to the existing control-link validation/error path: confirmed unicast RREP
send failure immediately blacklists its link and retires pending RREP-ACK state as specified in §4.
Other confirmed link losses resolve any pending validation before cleanup and run
ordinary invalidation without creating a data-repair transaction. Unknown or non-data failures
likewise cannot opt into repair merely because they contain a network header.
Control failures and declined repair do not create new deferred-repair flags; notify their affected
ordinary precursors instead. Existing repairs retain their obligations.

| Trigger/outcome | Required local-repair action |
| --- | --- |
| Link failure affecting transit destination, flag off or destination ineligible | Existing ordinary invalidation and RERR path; no repair RREQ |
| Eligible link failure with flag on and admission permits repair | Invalidate affected route records, increment each known sequence once, retain old route metric/precursors, and start one `LOCAL_REPAIR` transaction for the triggering route prefix |
| Other route records sharing the link after an admitted data repair | Mark inactive immediately; mark eligible records repairable until `now+activeRouteTimeout`, but do not proactively discover them; notify ordinary precursors for records not held for deferred repair |
| Transit data for a member of the currently repairing prefix | Apply admission above and queue eligible unowned/joined members once through prerouting; no repeated sequence increment, RREQ transaction or premature ordinary RERR |
| Transit data for another eligible repairable route prefix before its repairable deadline | Subject to admission, start its one local transaction on demand; otherwise use ordinary RERR/drop handling |
| Accepted sufficiently fresh update to the owned repair prefix | Compare new hop count with saved old count, then run the success cleanup below before releasing remaining member FIFOs through the existing preparation/reinjection path |
| Repaired path is longer than saved path | Send one precursor notification entry for the repaired prefix with N set and that record's sequence, keeping the repaired route active; equal/shorter paths send no N notification |
| Repair wait expires without usable route | Clear the owned prefix's repairing/repairable state, issue ordinary RERR for that prefix and dispose of all its attached FIFOs once; do not increment the sequence a second time solely for this timeout |
| Repairable deadline expires before another route is repaired | Clear its repairable flag and expose ordinary failure handling to subsequent data |
| Stop, crash or reboot | Cancel repair/admission/deferred-send obligations and dispose of queued packets through the existing lifecycle path; recovery hold takes precedence over new repairs |

Use one successful-repair terminal path for an accepted update to the owned route prefix that
satisfies its freshness/forwarding checks. Committed one-hop, RREQ-originator and Hello updates can
complete host repair; a host-specific update cannot complete aggregate repair or clear its flags.
Check the owned prefix record directly, even if `wireTarget` now selects an independent host route
or every member has left.
Capture any required longer-path N notification using the saved pre-break hop count and precursors.
Clear the repaired record's `beingRepaired` and `repairable` flags, cancel its RREP/total repair
deadline and any deferred repairable deadline, remove its queued rate admission/delayed send, and
discard its saved repair context. Detach the transaction before releasing packets or sending the
captured N notification.
Keep the repaired route active; other still-invalid route prefixes sharing the original broken link
retain their independent repairable flags/deadlines. A later expiry callback for the completed repair
must have no remaining obligation to invalidate the route or emit an ordinary RERR. A second real
failure after success starts a new eligible repair with a newly saved hop count and deadline.

For an aggregate, both N and ordinary repair-failure RERR entries carry the normalized prefix address
and its stored sequence, not `wireTarget` or each queued member address. Receivers resolve that
address under §5; the sender's private prefix key is not on the wire. Use the captured route key for
cleanup/notification rather than a new longest-prefix lookup on `wireTarget`; an independently
learned host route must survive aggregate repair failure. Test two members sharing one repair: one
RREQ attempt, both FIFOs released once on matching-prefix success, or both disposed once on timeout,
with one prefix notification entry and no surviving member deadline. Apply the existing RERR budget
and precursor delivery rules; a prefix entry may still need delivery on several precursor links.
Also accept an independent host update for one member while the prefix remains under repair: release
that FIFO once, retain the original prefix deadline, and on failure drop only the remaining FIFO
while preserving the host route.

Choose one local RREQ attempt at the computed TTL, with wait
`2*nodeTraversalTime*(repairTTL+timeoutBuffer)` from actual send. Reuse jitter and FIFO rate admission;
the repair has a total deadline of creation time plus `pathDiscoveryTime` to bound budget starvation.
If that deadline arrives before transmission or before a usable RREP, fail the repair. This finite
deadline and single attempt are the chosen policy for the RFC's optional local search; they do not
replace the originating discovery retry/backoff schedule. At an exact RREP/deadline tie use normal
event order and ensure the losing event cannot complete/dispose the transaction again.

Aggregate repair uses the U=1, D=1 RREQ above and cannot demand advancement of the subnet counter
on the wire. A matching-prefix RREP must still satisfy the retained sequence after invalidation;
an unchanged older counter is rejected and may cause this repair to time out. Keep that retained
sequence and the ordinary bounded failure path. Document this aggregate-repair limitation and test
both a genuinely fresher prefix RREP that succeeds and an unchanged-counter RREP that ends in
one ordinary prefix RERR. Host repair still carries its known host floor.

The packet in `linkBrokenSignal` is borrowed and may already be committed to lower-layer disposal.
Inspect its headers and existing tags read-only during the callback to classify data/control and
identify source/destination; resolve the attempted link under §4's binding and request-tag contract,
independent of Hello. Copy only needed scalar/key values into owned state. Never retain the borrowed
packet or references into it, mutate it, reinject it or duplicate it as a new data transmission.
Buffer subsequent undeliverable transit packets through the IP QUEUE contract. Tests must distinguish
loss of that original failed transmission from successful release of packets queued during repair.
When a confirmed link failure arrives while a repair already exists, merge affected invalidation
without restarting its search or extending its deadline. A received stale RREP cannot complete it.
Repeat §4's in-flight N-to-M route replacement case with repair enabled: no repair is started for D,
and subsequent data uses its replacement route through M.

N notifications share the existing RERR budget/precursor/interface selection. Suppressing a notification
because the rate budget is exhausted must not undo successful repair or invalidate its route.
Retain ordinary N reception semantics from §5; an upstream node need not initiate rediscovery merely
because it receives N. Document enabled behavior, the default-off profile and the finite-search policy.

## 7. Tests and evidence

Extend the existing [AODV standards map](../../doc/project/evidence/protocol/aodv/standards.md) for
the selected clauses. Add `standard/rfc3561/catalog.md` and `protocol/aodv/{features,checks}.md`
under `doc/project/evidence/`, deriving those artifacts from the RFC only. Put model-specific
categories, run results and interpretations in `model/aodv/`; update its existing coverage and
conformance records without rewriting their historical pass evidence. Do not claim level 5 merely
because this repair touches clauses assigned to that level: broader features remain out of scope.

Add a small production-path fixture under `tests/protocol/aodv/`, following
[AUTHORING.md](../../tests/protocol/lib/AUTHORING.md). Inject control packets through UDP/IP, capture
outgoing control packets, assert route state via the public routing table, and test data forwarding.
Use typed packet predicates if a filter lacks AODV field access. Keep support protocol-specific;
add shared harness capability only after demonstrating a real missing generic contract. Such a
prerequisite is a separate commit. No tests may substitute a copy of AODV dispatch logic.

Use module tests for lifecycle and netfilter ownership that need a scriptable node; helper unit
tests for serial arithmetic supplement, but never replace, protocol-path checks. All proposed names
below are new artifacts, not claims that these tests already exist. Each row names the behavior
and its principal distinguishing observation. The referenced sections define all required case
variants; the matrix does not replace those policies.

| Behavior | Proposed test artifact | Required distinguishing observation |
| --- | --- | --- |
| Serial ordering | `tests/unit/AodvSequence.test`; `tests/protocol/aodv/Rfc3561SequenceFreshness.test` | Accept 0 after 0xffffffff, reject the reverse; cross 0x7fffffff/0x80000000, check equality and half-range; real RREQ/RREP/RERR paths use the result |
| Reverse validity | `Rfc3561ReverseRoute.test` | Cross both RREQ U values with stored known/unknown originator sequences; stale originator information never replaces a fresh path |
| Requested-destination validity | `Rfc3561SequenceFreshness.test` | All §2 U/exact-host-validity combinations preserve or replace forwarded fields correctly; unknown placeholders are excluded from ordering; aggregate-only knowledge never fills or compares an RREQ floor |
| RREP acceptance | `Rfc3561ReplyAcceptance.test` | Rejected advertised information is neither relayed nor installed and cannot complete that destination's search; accepted information releases eligible data once; incompatible caller output first/middle/last is disposed of without blocking eligible packets or pre-mutating their tags |
| Previous-hop freshness and alias ordering | `Rfc3561NeighborSequence.test` | Preserve known neighbor sequence and path fields until message-specific acceptance when sender=destination/originator; equal-sequence direct RREP reactivates an inactive route and is relayed; stale direct RREP leaves it inactive. An unrelated previous-hop update completes that neighbor's queued search once, including duplicate/rejected-message paths; no later RREQ escapes |
| Hello route acceptance | `Rfc3561NeighborSequence.test`; `Rfc3561HelloLoss.test` | Older/half-range Hello cannot change route state; equal/newer or stored-unknown cases establish the direct route and can complete data searches. Reactivation uses a fresh route expiry rather than the old deletion deadline; active lifetime is not shortened; zero lifetime neither creates a route nor arms a new detector |
| RREP-ACK and blacklisting | `Rfc3561ReplyAck.test` | Final originator and rejected-but-A-flagged RREPs receive RREP-ACKs; two links have independent validation; only the timed-out/failed peer is blacklisted; confirmed RREP failure with/without A blacklists before cleanup; late RREP-ACK cannot clear it or produce a second timeout outcome; exact-time RREP-ACK/timeout orders follow §4 |
| Failed-link attribution | `tests/module/AodvTransitFailure.test`; `Rfc3561ReplyAck.test` | With Hello disabled, use direct AckingMac and independent nonaggregating 802.11 DCF controls: learn bindings through real one-hop control reception, then fail a MAC transmission via N after replacing D's route via M. D's replacement and M's validation survive; only current routes still using N are invalidated. Cover failed unicast RREP, no peer action for missing tags/binding or ambiguous/rebound MAC, and binding cleanup on restart |
| Transit failure | `tests/module/AodvTransitFailure.test` | With default/explicit useLocalRepair=false, missing/inactive transit route causes RERR/drop and no RREQ; valid AODV/non-AODV routes forward; enabled repair is checked in its dedicated fixture |
| Budget recovery | `Rfc3561RequestRateLimit.test` | Exhaust first/retry budgets; finite admitted workload terminates; FIFO is readiness order; jitter crosses a reset; both reset/send event orders count identically, including fractional start/restart times; qualifying routes cancel pending sends; final failure destroys queued packets once with no reinjection or AODV-created application notification |
| Reboot recovery and control bypass | `tests/module/AodvRebootRecovery.test` | Fresh route learned during hold retains queued data; expiry completes without RREQ if usable; extensions defeat old wake-ups in both equal-time orders; A-flagged RREP during hold gets an immediate RREP-ACK; transit failure emits its budget-permitted RERR during hold; these outputs create no data FIFO/discovery; no early discovery, forwarded RREP or delayed pre-restart send |
| Search progression and timing | `Rfc3561DiscoveryTiming.test` | Correct retained-hop start below/at/above threshold, exact threshold ring, first network-wide timeout and doubled waits; correct send/retry counts; TTL-201 counterexample stays within the blacklist bound |
| Reverse lifetime | `Rfc3561ReverseLifetime.test` | Longer existing expiry survives; equal/nonimproving RREQ still extends a shorter expiry; scheduler expires at the new time |
| Expiry and failure sequence actions | `Rfc3561ReverseLifetime.test`; `Rfc3561ReplyAcceptance.test` | Idle expiry preserves known S and validity, so an acceptable equal-S RREP reactivates; confirmed loss advances to S+1 and rejects S. Exercise both exact-deadline orders, wrap, unknown validity and pending discovery. Distinguish case-(ii) increments from RERR copies, N preservation and repair-timeout nonincrement |
| Gratuitous lifetime and quantization | `Rfc3561GratuitousReply.test` | At t=100 s with expiry=105 s, wire lifetime is 5000 ms and recipient expiry is reception+5 s; 0.999 ms remaining emits no advertisement and allows normal RREQ forwarding, 1.999 ms advertises 1 ms; zero-lifetime RREP cannot complete its advertised destination's search but receives an RREP-ACK if A is set; check uint32 millisecond boundaries |
| N-flagged RERR | `Rfc3561NoDeleteError.test` | N preserves route fields and is relayed only for the next-hop/precursor case; ordinary RERR still invalidates |
| Hello loss | `Rfc3561HelloLoss.test` | Under the stable attribution contract, silent neighbor invalidates despite outgoing traffic and data from S via N refreshes N; unknown attributed sender changes no deadlines; missing metadata or changed/ambiguous binding disables only its interface until restart; returning metadata and fresh Hello do not re-enable it; restart permits arming again; ties follow §4; custom equivalent-tag interface works |
| Selected interfaces | `Rfc3561Interfaces.test` | Secondary local address answers discovery; primary-originated RREP can return on secondary; Hello source/destination match each link; own RREQ is suppressed; learned ingress is outgoing next-hop interface; repeated remote address on different links does not conflate state. Control on an unselected interface changes no AODV state, while external data forwarding still works |
| Subnet RREPs and retained prefixes | `Rfc3561SubnetReply.test` | §6 eligibility agrees for new and queued data: a /32 completes discovery from retained /24 state, while known inactive /32 blocks /24 substitution. During recovery, a known /24 replacing unknown /32 authority survives its deletion deadline and rejects older /24 information. Cover sustained blocking/quiescent deletion, output tags and member ICMP preservation |
| Subnet RREQ floors and cached RREPs | `Rfc3561SubnetReply.test` | Exercise §6's originating-search field rows and relay rules. Retained /24 sequence 100 never becomes a floor against independent /32 sequence 5; swap the values and include wrap/half-range. Known-floor RREQs get no aggregate cached RREP or floor promotion; an eligible U=1, D=0 RREQ gets a nonzero-prefix RREP, prefix-owned precursors and downstream aggregate installation. D=1 bypasses that cache |
| RERR prefix identity | `Rfc3561SubnetReply.test`; `Rfc3561NoDeleteError.test` | With /24 and /25 sharing a base, next hop and interface, ordinary and N entries mutate neither route nor repair/queue state and relay nothing. Retaining one inactive candidate still blocks resolution. Distinct-peer/interface and unique-prefix controls affect only their matching record; an unrelated unambiguous entry in the same RERR still works |
| Outgoing RERR identity | `Rfc3561SubnetReply.test`; `tests/module/AodvTransitFailure.test` | Failed member traffic for an inactive aggregate reports the canonical prefix/counter and updates only that record; an isolated blocked-host failure reports its host counter and preserves the active aggregate; absent/unknown host state uses zero. The receiver updates only the matching counter domain |
| Independent route progress | `Rfc3561ReplyAcceptance.test`; `tests/module/AodvGatewayQueues.test` | Add a legitimate non-AODV route while data is queued: the next existing callback releases eligible data before another RREQ or final disposal. One external destination can leave a gateway search while another remains; the last release cancels originating state. Recovery still holds data |
| Sequence ambiguity | `Rfc3561DestinationSequence.test` | RREQ equal, one ahead, several ahead, older and wrapping: RREP follows the explicit RFC §6.1 interpretation |
| Parameter constraints | `tests/module/AodvParameterValidation.test` | Each §3 constraint at valid/invalid boundaries; deletePeriod link-feedback/Hello bounds; zero retries, blacklist defaults/override and overflow; destination/Hello durations whose truncated millisecond counts are zero or exceed UINT32_MAX are rejected |
| P1, external gateway queues | `tests/module/AodvGatewayQueues.test` | Two external destinations complete through one gateway discovery; failure drops both queues; completion/failure occurs once; internal selected subnets bypass the gateway |
| P2, RERR capacity and admission | `Rfc3561ErrorCapacity.test` | Serialize 1/255/256 records without loss or changed sequences. With budget 1, one partition reaches precursors on two interfaces for one charge; a second partition is suppressed as a whole. All affected local routes are invalidated |
| IPv6 compatibility | `tests/module/AodvIpv6Compatibility.test` | IPv6 UDP RREQ/RREP/RREP-ACK establishes a route and releases data; stale/wrapping RREPs, budget deferral and stop/restart follow shared state rules; Hello arms N, then IPv6 data from S via N during Hello silence refreshes N through real prerouting, with loss only after data also stops; custom encoding round trips remain unchanged |
| Local repair, dedicated stage | `Rfc3561LocalRepair.test`; `tests/module/AodvLocalRepairLifecycle.test` | Exercise the §6 eligibility, TTL, success/failure and lifecycle rules: default-off control, known host floor on host repair, one terminal outcome, no old-deadline RERR after success, a second real failure, correct N generation and control-packet exclusion |
| Repair ownership and mixed traffic | `Rfc3561LocalRepair.test`; `tests/module/AodvLocalRepairLifecycle.test` | Exercise §6 admission in both arrival orders. Two members share one U=1, D=1 aggregate repair/deadline; fresh prefix RREP succeeds, unchanged counter times out once. Independent host recovery releases only its FIFO; an aggregate releases an unknown-sequence host repair's FIFO but cannot finish its owner. Preserve independent routes on timeout; after the last member leaves, matching-prefix success still cancels the original notification deadline |

Except where qualified, protocol-test filenames in the table are relative to
`tests/protocol/aodv/`. Extend the acceptance cases with retained-record lifetime during discovery,
confirmed loss raising that record's sequence followed by rejection of an older RREP, completion from
RREQ/Hello, and success during jitter/rate deferral: no later RREQ may
escape. Add enabled local-repair parameter/eligibility bounds to parameter validation, retaining a
default-off control. Extend the IPv6 fixture with enabled local-repair success and timeout when its
dedicated stage lands. Existing serializer PCAP/round-trip
cases remain wire-format controls, including custom IPv6 types in the round-trip fixture. There is
no existing IPv6 AODV example configuration in `examples/aodv/omnetpp.ini`; the named compatibility
fixture must be added, not treated as existing execution evidence. Its profile uses one IPv6
interface and zero jitter; baseline cases disable Hello/local repair/gateway/subnet modes, with one
additional case enabling Hello and exercising the shared loss detector. Establish real ingress tags,
arm from a Hello, suppress later Hellos and deliver transit data through N for longer than
`allowedHelloLoss*helloInterval`; N stays live. Stop that data and observe one loss action at the last
attributed reception plus that interval. Injected control messages reach the real IPv6/UDP receive
path. Do not add unbounded IPv6 feature-parity work or disable untested existing modes.

Pin run 0, seed-set 0, zero broadcast/periodic jitter and scripted delivery for exact timing cases;
exercise before/at/after deadlines with explicit scheduler-order expectations. No statistical claim
requires a seed sweep. Add one bounded compatibility set with existing nonzero jitter and seeds
0, 1 and 2 only for delayed-send/RREP-ACK/retry deadline interactions. Stop at the first unexpected result
and diagnose it; do not rerun until green.

For each behavioral commit, run its new reproduction unchanged against the immediate parent and
the corrected revision in isolated worktrees with fresh debug builds. Record whether the parent
reached the intended assertion: setup/compile failures do not prove a defect. A passing parent is
a control/new coverage, not a claimed reproduction. Integrate the test with its fix so the committed
series stays passing. No unexpected-failure markers may hide an unfinished correction.

## 8. Commit order and validation

Two adjacent corrections have distinct roles in the series:

- **P1 — external-gateway queue ownership:** discovery is keyed by gateway but queued data by actual
  external destination. Track each queued destination's discovery target explicitly, so gateway
  success/failure serves every dependent FIFO once. When multiple interfaces are enabled, classify
  an address as internal if it matches any selected ad-hoc interface subnet. This is a gateway
  integration prerequisite for replacing shared discovery state, not an RFC default-route feature.
- **P2 — RERR count capacity:** outgoing unreachable lists may exceed the one-byte destination count.
  Split into groups of at most 255 without changing each destination's sequence. Apply §5's logical
  partition admission and complete recipient fan-out, invalidating the entire local affected set.
  This is an adjacent serialization correctness fix, not a prerequisite for N-flag reception.

Each row is a concern boundary; split a row further if its reproduction exposes independent fixes.
Include its direct tests and evidence in the same behavioral commit. Preparation must be passing
and behavior-preserving. Every implementing commit names this plan and explains its reproduction.

| Order | Concern | Dependencies |
| --- | --- | --- |
| 1 | Specification decisions, catalogs/check descriptions and minimal passing fixture controls | none |
| 2 | Serial comparison and message-specific route acceptance; destination-sequence interpretation | 1 |
| 3 | Reverse/gratuitous lifetimes and explicit ordinary-expiry sequence behavior | 2 |
| 4a | Discovery state, data-only admission/control bypass and terminal queues, including P1's dependent gateway queues | 2 |
| 4b | TTL progression, retry/backoff, explicit parameter bounds and derived defaults | 4a |
| 5 | Neighbor bindings and failed-link attribution independent of Hello; RREP-ACK correlation, confirmed-send-failure blacklisting and originator RREP-ACKs | 2, 4 for control recognition and actual-send deadlines |
| 6 | Transit failure, cause-specific sequence actions, ordinary/N-flagged RERR and precursor delivery | 2, 3, 4, 5 |
| 6b | P2: representable outgoing RERR lists | 6 |
| 7 | Hello liveness and shared link-loss handling | 5, 6 |
| 8 | Reboot hold and complete lifecycle cancellation | 4–7 |
| 9 | Explicit multi-interface routing and neighbor identity | 5–8 |
| 10 | Subnet routes, RREQ floors, cached-RREP prefix preservation, RERR identity guard and matching discovery completion | 2, 4, 6, 9 |
| 11 | Opt-in local repair, prefix/queue ownership, mixed-traffic admission, N generation and terminal lifecycle paths | 4a, 4b, 5–10 |

Include feature-specific user documentation in its owning row. Follow the
[commit rules](../../doc/project/rule/pull-request.md) and
[baseline procedure](../../doc/project/guide/change-a-baseline.md); no baseline replacement is
authorized by this plan. Numeric dependencies on row 4 mean both 4a and 4b.

From the implementation checkout root, the following commands are planned, not executed:

```sh
make -j"$(nproc)" MODE=debug
doc/project/enforcement/check-architecture.sh src/inet/routing/aodv
opp_run_unit_tests -m debug -f 'AodvSequence'
opp_run_module_tests -m debug -f 'Aodv|AODV'
opp_run_protocol_tests -m debug -f 'aodv'
```

At execution time confirm runner options with `--help`, narrow the filters to each commit's named
cases and inspect the actual selection; zero selected cases is NOT_RUN. The module filter also
covers the four existing `AODV*.test` integration controls. Run directly related serializer cases if
representations change. Add scoped
existing AODV fingerprints as trajectory checks after direct correctness tests, selecting cases
from current dependency evidence. Do not substitute fingerprints for the assertions above.

Before submission, follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md), including
debug/release builds and the canonical review checklist. Check an IPv4-only AODV feature build and
an IPv4+IPv6 build with the new bounded compatibility fixture. Record focused execution evidence in
model results using the [testing rules](../../doc/project/rule/testing.md).

Completion requires every correction in the §7 matrix, including P1/P2, to have either demonstrated
corrected behavior or a documented refutation of its defect hypothesis with the unchanged test as a
control. Compatibility and newly supported feature rows require their stated observations; a parent
setup failure is not a runtime reproduction. The destination-sequence interpretation must be visible
in tests and documentation; optional unsupported modes must be reported accurately. The local-repair
stage must pass its enabled-mode tests while preserving the
default-off controls; the feature is not complete merely because the parameter exists. All required
focused tests must pass on fresh artifacts, lifecycle checks must show no surviving obligations,
and no unrelated dirty files or unauthorized baselines may enter the
series. Full RFC conformance is not a claim of this bounded repair.

## 9. Plan validation status

### Implementation tracking (2026-11-09)

The user authorized implementation, commit, push and an upstream PR. Work proceeds on
`fix/aodv-rfc3561` in `/home/miguel/workspace/inet-aodv`, based on upstream master
`7ec0d4d3f3e0e0e1357e2449b0bdcbb6ef0e1ce4`. The original DSDV checkout is preserved.
This base has unchanged AODV source relative to the planning snapshot.

The current testing rule also requires one reviewer document per behavioral commit. Each document
will describe the row's §7 fixture scenarios, assertions, production path, causal parent comparison,
code action and coverage limits. Proposed assignments, beside `tests/protocol/aodv/`:

| Row | Reviewer document | Protocol fixtures |
| --- | --- | --- |
| 1 | `fixture-controls.md` | `Rfc3561Control.test`, a passing UDP/IP RREQ/RREP and reverse-route control |
| 2 | `route-freshness.md` | SequenceFreshness, ReverseRoute, ReplyAcceptance, NeighborSequence, DestinationSequence, RequestFloors, HelloAcceptance, ErrorFreshness |
| 3 | `route-lifetimes.md` | ReverseLifetime, ReverseLifetimeLong, GratuitousReply, SubmillisecondReply, MillisecondReply, MaximumLifetimeReply, OversizedLifetimeReply, with `tests/module/AodvAdvertisementLifetimeInvalid_1.test` through `_4.test` |
| 4a | `discovery-ownership.md` | ReplyAcceptance, RequestRateLimit, with GatewayQueues module coverage |
| 4b | `discovery-timing.md` | DiscoveryTiming, with ParameterValidation module coverage |
| 5 | `link-validation.md` | ReplyAck, with TransitFailure module coverage |
| 6 | `transit-errors.md` | NoDeleteError, with TransitFailure module coverage |
| 6b | `error-capacity.md` | ErrorCapacity |
| 7 | `hello-liveness.md` | HelloLoss, NeighborSequence |
| 8 | `reboot-recovery.md` | ReplyAcceptance and ReplyAck recovery cases, with RebootRecovery module coverage |
| 9 | `selected-interfaces.md` | Interfaces |
| 10 | `subnet-routes.md` | SubnetReply, NoDeleteError |
| 11 | `local-repair.md` | LocalRepair, with LocalRepairLifecycle and Ipv6Compatibility module coverage |

Fixture names in this table are prefixed `Rfc3561` and suffixed `.test` unless written in full.
The §7 matrix supplies the exact conditions and required observations. Future documents and
fixtures remain proposed until created. Compilation and execution results will be recorded under
`doc/project/evidence/model/aodv/`. No behavioral completion is claimed by this tracking section.

Performed for this planning review (2026-11-09): project-guidance and static AODV/IP contract
inspection, comparison with RFC 3561, and whole-plan review of clarity, correctness, state ownership,
scope and test alignment. The review/revision loop ended with a pass finding no further actionable
corrections within the explicit profiles and limitations above. This is a plan-review outcome;
runtime behavior remains unverified.

Static checks passed for 11 local links, section/table structure, code fences and diff whitespace.
The 23 proposed fixture basenames are unchanged; their acceptance cases now include neighbor-driven
completion, Hello acceptance, independent member release, recovery freshness retention, outgoing
RERR identity and the selected-interface boundary. Arithmetic checks cover uint32 wrap and §3's
TTL-201 example (36.64 s against a 40.96 s bound). Same-base /24 and /25 destination/sequence RERR
records are byte-identical, supporting the explicit ambiguity guard. These checks are not production
serialization/timer tests or exhaustive arithmetic coverage.

Not performed: implementation, new test creation, builds, simulations, parent/feature comparisons,
fingerprints or baseline updates. Protocol fixture names and finer runner selectors remain proposed
until the tests are authored. Hello attribution examples are proposed configurations awaiting
production-path validation. The existing AODV evidence is a survey, not executed correctness coverage
for these corrections. This historical planning review predates implementation authorization.
