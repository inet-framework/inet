# AODV — model claims

> **Kind:** report · **Status:** snapshot 2026-11-09 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/aodv/standards.md), [coverage.md](coverage.md)

The historical survey below records the documentation claims inspected on 2026-09-23. Selected implementation decisions and direct regression evidence from the 2026-11-09 correction effort are appended separately. They do not establish complete RFC conformance or a higher achieved level.

## Selected correction decisions, 2026-11-09

### Final native IPv6 compatibility profile

The single selected IPv6 interface joins the existing control multicast group on
every start. Native request reception, rate deferral, pre-admission completion,
stop/crash/restart and three-node Hello/data attribution have direct paired
coverage. Native ND activity renews an armed neighbor deadline just like data.
See [ipv6-compatibility.md](../../../../../tests/protocol/aodv/ipv6-compatibility.md)
and [results.md](results.md). Earlier stage-specific pending statements below
describe the coverage available at those stages, not the final branch.

### Bounded local repair

The optional mode is implemented with default false. One transit-only repair
owns an exact route key, its original wire target, old metric, saved precursors
and fixed total deadline. maxRepairTTL defaults to floor(0.3*netDiameter), with
zero disabling eligibility. Missing reverse distance and excessive computed TTL
decline repair. The chosen local policy is one attempt with a reply wait from
actual send and a creation-time pathDiscoveryTime bound over jitter/rate delay.
Member arrivals do not change purpose or deadlines. Existing originating searches
take precedence over new repair for their retained prefix.

Independent member recovery releases its own FIFO without finishing a different
prefix owner. An empty owner retains its notification obligation. A committed
matching-prefix update finishes the owner before callbacks, including one-hop,
RREQ-originator and Hello host updates. A longer path sends N while staying active;
timeout disposes remaining members and reports the stored counter without another
increment. Other eligible failed-link records have independent deferred flags and
deadlines. These are model choices within RFC 3561 section 6.12's optional search.

Native IPv6 host repair has direct success/failure runtime coverage. ND messages
are link control and cannot wait behind AODV route discovery. The existing IPv6
pinned local-out link-broadcast behavior remains visible in the bounded fixture;
actual repaired transit uses its resolved alternate next-hop MAC. This stage does
not extend explicit interface selection or received subnet routing to IPv6. The
final IPv6 rate/restart and three-node observation checks are documented above.
See [local-repair.md](../../../../../tests/protocol/aodv/local-repair.md).

### Received subnet profile

IPv4 RREP prefix zero means a host; nonzero means the normalized advertised
prefix. Freshness and precursor state belong to that exact prefix. No counter
comparison crosses prefixes, even if the advertising router is shared. An
inactive known more-specific record blocks a covering aggregate for that target;
an unknown placeholder does not. The same selection and explicit output tags
serve new data and queued completion, respecting active non-AODV precedence.

The model interprets a known RREQ floor as host knowledge only. Retained aggregate
history produces U=1,D=1 and numeric zero, while retaining its counter locally.
Relays promote exact-host floors only. A cached aggregate can answer U=1,D=0,
preserving the requested member address and nonzero prefix. Peers exporting
aggregate counters in RREQ floors are outside this bounded profile. Same-prefix
operation assumes consistent subnet counter semantics and stable unambiguous
RERR identity; ambiguity is skipped conservatively. Authoritative subnet-router
election/advertisement and live handover between independent subnet counters are
unsupported. Continued demand can retain known host blocking indefinitely;
quiescence permits normal deletion. See [subnet-routes.md](../../../../../tests/protocol/aodv/subnet-routes.md).

The freshness stage has direct IPv4 UDP/IP evidence in [results.md](results.md), including eight
distinguishing parent failures followed by corrected passes. It covers selected host-route cases
and one-datagram discovery completion. Subsequent sections record the additional discovery and
link corrections. IPv6 runtime compatibility remains pending.

| Question | Model decision |
| --- | --- |
| Exactly half the 32-bit sequence range | Neither direction establishes freshness. Unknown numeric placeholders are excluded from comparisons. |
| RFC 3561 §§6.1/6.6.1 destination update ambiguity | Adopt a known fresher requested destination sequence under §6.1, without an extra unconditional increment. An unknown request cannot advance it. |
| Protocol state lookup | Use an exact owned host record when selecting host sequence information and direct-neighbor state. Forwarding and subnet support are separate plan stages. |
| Sender aliases advertised originator/destination | Evaluate the message-specific transition before any generic neighbor refresh can change that record. Stale advertised information cannot reactivate it. |
| Ordinary RREP acceptance | Forward or complete its matching discovery only after accepting its destination update. Equal sequence can reactivate an inactive route with a longer metric. |
| Hello acceptance | Require a positive advertised lifetime and the sender's own destination address. Preserve fresher known state. Active-route expiry and inactive retention deadlines remain distinct. |
| Reverse-route lifetime | Apply the RFC §6.5 maximum independently of path improvement, preserving a later existing deadline on replacement as well. |
| Advertisement duration | Use expiry minus current time for route-derived replies, truncated downward to whole milliseconds. Suppress a cached/gratuitous advertisement outside the positive uint32 millisecond range. |
| Configured advertisement duration | Reject an unencodable destination or Hello duration during initialization, including when Hello transmission is disabled. |

The [reviewer document](../../../../../tests/protocol/aodv/route-freshness.md) describes the tested
freshness scenarios and limits. Subsequent correction sections extend that scope. Hello loss,
reboot, selected interfaces, subnet routing and local repair remain assigned in the plan.
The historical survey below is preserved as its original read-only evidence.

## Discovery ownership correction

The discovery stage adds an actual-destination FIFO with an explicit search owner, so gateway
completion and exhaustion reach every dependent packet. Every live originating search has either
a scheduled phase timer or a ready admission served by the counter timer. IDs, sequences and
request budget are charged at actual send. Route selection is repeated before a wake-up can send,
fail or release data. Controlled output constraints dispose of only the incompatible packet.

The supported IPv4/IPv6 tables enumerate routes in their native lookup order. A connected subnet
on the selected ad hoc interface is treated as address scope, not independent evidence of MANET
reachability. Explicit static and other-protocol routes remain eligible. This decision preserves
discovery in configurations whose interface netmask installs a connected route even when static
route generation is disabled.

RFC 3561 section 6.3 recommends an application Destination Unreachable indication on exhaustion.
The existing INetfilter disposal API supplies deletion only. This correction retains that API and
records the recommendation as unmet, rather than inventing a transport callback. Destructor-count
fixtures require terminal disposal and no spurious application error. Selected executions and
the separate-parent comparison are recorded in results.md and discovery-ownership.md.

This stage established queue ownership and basic scheduler progress. The following sections
extend timing and link coverage. Reboot, selected interfaces, aggregate completion
and local repair remain assigned to subsequent stages. The search key is a full-width address
until exact-prefix repair ownership is introduced in its assigned stage.

## Discovery timing correction

The five timing fixtures observe actual UDP RREQ TTLs, IDs and reception intervals. They pass on
the ownership-stage parent and remain added coverage. Reply waiting begins at actual send,
network-wide waits double, and zero additional retries permit one network-wide attempt.

Initialization now enforces the plan's positive durations/rates, TTL order, representable search
arithmetic, retention and advertisement minima. Jitter samples are checked when used. The default
destination lifetime is max(2*activeRouteTimeout, 2*pathDiscoveryTime). The blacklist default bounds
an unthrottled search, including a retained-hop initial ring, threshold rings, exponential waits
and declared maximum jitter. This conservative formula is a model decision rather than an RFC
equality. Rate-limit workload can extend discovery beyond that bound. Positive explicit blacklist
overrides are supported. The new migration note lists configurations that must be corrected.

The bounded IPv4 executions and the six already-rejected parameter controls are separated in
results.md. No added IPv6 feature or local-repair validation claim follows from these tests.

## Link validation correction

An A-flagged RREP produces a one-hop ACK on its ingress interface independently of advertised
route acceptance, including final-originator, stale, direct-inactive and zero-lifetime cases.
ACKs clear only the matching outstanding neighbor/interface validation. Repeated sends retain
the first deadline. ACK processing never reactivates a route or removes an existing blacklist.
At an exact deadline, normal simulation event order decides the outcome. The two calibrated
Ethernet/UDP fixtures observe both orders without changing a private timer's priority.

Pending validation begins at actual control send. Confirmed RREP failure blacklists that
attempted link even without A, removes its pending validation and leaves other links independent.
Locally generated direct replies retain the existing ACK-request rule. Forwarded replies retain
their cleared-A policy, preserving the established default traffic pattern. MAC failure feedback
supplies the confirmed-failure outcome independently of whether that reply requests an ACK.

Stable interface-qualified IP/MAC bindings are learned only from one-hop control. They survive
route expiry and deletion. Rebinding or a shared MAC quarantines all affected MAC associations
until restart. Failure attribution reads retained output tags from the borrowed failed packet;
it never looks up that packet's final destination to guess its attempted next hop. Only owned
current routes still using the identified link are invalidated. Missing/unknown ingress metadata
ignores control; missing or ambiguous failed-link attribution skips peer-specific handling with
a diagnostic. Interface lookup for invalid metadata is nonthrowing.

The direct AckingMac and independent nonaggregating DCF tests establish immediate-peer meaning
and retained tags in those paths. The DCF failure asserts Retry, receiver address and both DS
bits in the actual failed frame. These are tested profiles, not a concrete-interface whitelist.
Infrastructure/WDS, aggregation and custom metadata semantics need their own evidence. The
per-interface Hello observation contract and restart cleanup are assigned to later stages.

The classifier inspects a temporary copy using the existing generic packet dissector, stops at
UDP, and bounds the raw control peek by its payload length. A known different UDP port takes a
metadata-only fast path. A reserved control that fails later parsing is still excluded from data
admission. Default port 654 and custom port 60654 have direct failure/blacklist evidence without
changing global protocol registrations or wire formats. See link-validation.md and results.md
for the individual parent comparisons and their limits.

## Transit-error correction

The applicable retained record owns both the address and counter in a failed-data RERR. A
prefix failure reports the canonical prefix, while a known inactive host blocks and retains
its own independent counter domain. Case-(ii) data failures increment known counters once per
failed packet and refresh deletion even when the send budget is exhausted. Unknown placeholders
remain unchanged and appear as zero on the wire. Received ordinary errors copy applicable
nonstale information; N notifications preserve every route field, including inactive state.

RERR identity resolution includes inactive records before any freshness check. Several owned
prefix lengths for the same wire address and sender/interface suppress that entry. Full error
coverage assumes a fixed prefix identity for that tuple throughout the run; a delayed error
cannot be disambiguated after tuple reassignment. There is no history registry or wire extension.

Precursors carry interface IDs. One admitted logical error delivers every required usable
precursor/interface copy for one budget charge at actual send after jitter. Local invalidation
precedes that decision. Ordinary transit failure can add the attributable immediate sender;
without attribution or another precursor it uses the primary broadcast fallback. An unrelated
manual route remains eligible for forwarding and immune to AODV error mutation. Outgoing list
capacity and complete lifecycle behavior remain separate subsequent stages.

The 25 protocol and three module cases, parent distinctions and limits are recorded in
results.md and the transit-errors.md reviewer document.

## Route-error capacity correction

Outgoing lists are split into groups of at most 255 records before header construction and
admission. Each group preserves its counters and gets one actual-send budget decision for every
required precursor/interface copy. Full local invalidation precedes all partition decisions.
The One/Maximum controls and Partition/Budget distinctions serialize and decode emitted headers;
two-interface precursor state is supplied through the public table to isolate this mechanism.
Selected-interface learning remains stage 9 work. The wire layout is unchanged.

## Hello observation correction

Neighbor/interface observation deadlines are separate from route-use lifetimes. A positive
attributable Hello arms its sender; incoming traffic refreshes an already armed immediate
sender using stable interface/MAC bindings. Data alone never arms one. Outgoing traffic cannot
postpone loss. Route-sequence acceptance remains independent, so stale positive Hello information
can refresh link observation while leaving fresher route state untouched.

Missing or unstable immediate-sender metadata disables observation for the whole selected
interface until restart and cancels its deadlines. Later valid traffic does not re-enable it.
At expiry, pending validation on that link is resolved as failed and the ordinary current-link
fan-out runs. Records without eligible precursors do not cause an error flood. Exact deadline
refresh/metadata races follow simulation event order, without production priority changes.

The IPv4 profiles include AckingMac, independent nonaggregating DCF and Ethernet, plus two real
three-node forwarding cases asserting IP source S and immediate sender N. The shared hook also
serves IPv6, whose runtime compatibility remains pending. No concrete-interface whitelist is used.
See hello-liveness.md and results.md for controls, causal parent failures and limits. Restart
reset and selected-interface integration remain subsequent stages.

## Recovery and lifecycle correction

Initial startup is distinct from restart even at time zero. Restart holds local demand in a
deferred discovery phase served by one recovery timer, before any active-route completion.
Transit data extends that timer and follows the permitted error/drop path. Accepted control
can restore state during the hold. One-hop ACK and RERR output remain permitted; other control
advertisements and originating discovery wait. A fresh self-directed request may restore the
shared node sequence without advertising it early. At expiry, recheck the route before any RREQ.
Exact deadline extension/wake-up order follows normal simulation event insertion order.

Stop/crash clears owned state/events and only this instance's routes. IP owns queued packet
disposal; its identity-based drop API tolerates whole-node ordering that has already removed an
obligation. A new socket identity prevents late close indications from affecting the restarted
socket. Operational generation checks stop a detached release batch after lifecycle reentry,
without dereferencing remaining packets already disposed by IP. Destructor cleanup has no
cross-module calls. The selected IPv4 lifecycle cases and limits are in reboot-recovery.md and
results.md; IPv6 runtime compatibility, explicit selected interfaces and repair remain pending.

## Selected-interface correction

An optional explicit IPv4 interface set retains one primary originator identity, RREQ-ID stream
and node sequence. Physical discovery copies share a logical attempt/rate charge. Selected
secondary addresses answer requests and self-originator checks cover the entire selected set.
Ingress and owned next-hop interfaces determine unicast output and qualified precursor delivery.
An unrelated manual forwarding route cannot replace the owned reverse control path.

Selection validation occurs at actual start, including a node initialized down. Mutable selection
parameters apply at the next start; live interface removal/address reassignment is unsupported.
Explicit selection is IPv4-only and requires unique exact names, primary membership, usable
nonloopback interfaces and distinct local IPv4 addresses. Empty keeps the legacy primary path.

Hello output and actual-send broadcast suppression are per interface; exactly one elapsed interval
permits a Hello. Missing sender MAC disables only its identified selected interface, while missing
ingress identity cancels observation across the selected set conservatively. Control from a known
unselected link has no AODV state effect. Legitimate external data forwarding remains available.
The generic routing-table factory creates the gateway adapter, which follows its learned interface.

The selected-interfaces.md reviewer document records wire assertions, real learned precursor fan-out,
startup rejection cases, external data controls and a radio startup-mode fixture diagnostic. Native
IPv6 runtime compatibility and subnet/repair integration remain assigned to later stages.

## Historical survey

Read record — no build, no run in the 2026-09-23 survey:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the AODV family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 3561 | [`Aodv.ned:26-27`](../../../../../src/inet/routing/aodv/Aodv.ned) | "This implementation is based on RFC 3561. For more information, you may refer to the following link: ...". This is the documentation comment of the module, which the module reference publishes. |
| RFC 3561 §6.6.3 | [`Aodv.ned:37`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `askGratuitousRREP` parameter: "See RFC 3561: 6.6.3". |
| RFC 3561 §6.9 | [`Aodv.ned:38`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `useHelloMessages` parameter: "See RFC 3561: 6.9". |
| RFC 3561 §6.12 | [`Aodv.ned:39`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `useLocalRepair` parameter: "See RFC 3561: 6.12 *not implemented yet*". |
| RFC 3561 §5.1 | [`Aodv.ned:40`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `destinationOnlyFlag` parameter: "See RFC 3561: 5.1". |
| RFC 5148 | [`Aodv.ned:45`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `maxPeriodicJitter` parameter carries the bound of RFC 5148 §5.4 verbatim, unlabeled: "It MUST NOT be negative; it MUST NOT be greater than MESSAGE_INTERVAL/2; it SHOULD NOT be greater than MESSAGE_INTERVAL/4." |
| RFC 5148 | [`Aodv.ned:48-54`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `maxJitter` and `jitter` parameters, above a comment labeled "RFC 5148:" that quotes the MAXJITTER advice of §5.4. |
| RFC 3561 §6.1 | [`Aodv.h:112`](../../../../../src/inet/routing/aodv/Aodv.h) | The `sequenceNum` field: "it helps to prevent loops in the routes (RFC 3561 6.1 p11.)". |
| RFC 3561, AODV Terminology | [`Aodv.cc:1247`](../../../../../src/inet/routing/aodv/Aodv.cc) | "it means invalid, see 3. AODV Terminology p.3. in RFC 3561". |
| RFC 5148 | [`Aodv.cc:1292`, `Aodv.cc:1369`](../../../../../src/inet/routing/aodv/Aodv.cc) | Two comments labeled "RFC 5148:" above the jitter computation for a forwarded and a generated message. |
| RFC 3561 | [`ch-adhoc-routing.rst:58`](../../../../../doc/src/users-guide/ch-adhoc-routing.rst) | "The :ned:`Aodv` module type implements AODV, based on RFC 3561." The user's guide repeats the claim of the NED documentation. |

Every citation names RFC 3561 or RFC 5148. No comment, parameter, `.msg` field or line of the user's guide names any other document.

Mapped onto the standards map, [`standards.md`](../../protocol/aodv/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 3561 | yes, `base` | **yes**, in the published documentation and at the level of clauses | The module documentation, the user's guide, and per-parameter comments that name §5.1, §6.1, §6.6.3, §6.9 and §6.12. |
| RFC 5148 | yes, `companion` | **yes**, in four source comments | No documentation comment of the `Aodv` module names RFC 5148; the citations sit next to the `maxJitter`/`jitter`/`periodicJitter` parameters and the forwarding code, where a reader of the module reference does not see them. |

**The finding of the survey.** Both documents of the in-scope set are current: neither RFC 3561 nor RFC 5148 has an `obsoleted_by` or `updated_by` entry in the register ([`standards.md`](../../protocol/aodv/standards.md#document-list)), so AODV carries none of the obsolete-citation headline that RIP, ARP and RTP each show in this wave. The two documents the model names are the two documents that govern today.

The headline of this survey is a different kind of gap: a citation that reads as a refusal but that the workflow's own rule for a claim does not let a later pass treat as one.

`Aodv.ned:39` documents `useLocalRepair` as "*not implemented yet*", and the code at `Aodv.cc:1100` carries only `// TODO Implement: local repair` at the point the mechanism would run — no branch, no partial state machine, nothing beyond the switch and the comment. Read in isolation, this looks like the third row of [the claim principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test): a stated limitation, so a check that fails would declare the failure expected. The guide's own rule says otherwise. The comment gives no reason the mechanism cannot be built and no argument that it is not needed — it is the "TODO implement X" row of the guide's table, a plain promise, and the guide names exactly this shape as the giveaway that settles it: **"if the parameter... the behavior needs is already there and carries a placeholder value, the mechanism is claimed whatever the comment says."** `useLocalRepair` is that parameter. Local repair is therefore a **claim**: a level 2 or 3 check of it that fails is a defect, not a statement the model declined in words. RFC 3561 itself marks the mechanism `MAY` (`rfc3561.txt:1410`), so the standard would not have penalized a true refusal; the model chose to expose the switch anyway, and that choice is what makes it a claim.

For the level 2 pass, four facts from the code (the guide permits this look to decide the claim question, and the pass must check each one):

1. **Local repair is a claim, not a declared refusal.** See the finding above. A check of RFC 3561 §6.12 that fails is a **defect**.
2. **Actions after reboot (§6.13) is the same shape.** `Aodv.cc:1581` carries a bare `// TODO Implement: Actions After Reboot`, followed by a paraphrase of the MUST/SHOULD text of §6.13. No reason is given. A related check that fails is a defect, not a declared expected failure.
3. **The model assumes one interface throughout.** `Aodv.cc:808` and `Aodv.cc:1060` each carry a bare `// TODO Implement: support for multiple interfaces` / `// TODO IMPLEMENT: multiple interfaces`, and `Aodv.cc:1321` adds `// FIXME Drop the queued datagrams.` at a related point. RFC 3561 §6.14 states the multi-interface behavior as description, not as a `MAY`; the model's own TODOs are bare, so this is a claim by the same rule, and a check that needs a second interface meets an unfinished mechanism, not an unclaimed one.
4. **The claimed status of both documents is below Internet Standard.** RFC 3561 is Experimental and RFC 5148 is Informational ([`standards.md`](../../protocol/aodv/standards.md#document-list)). A later pass states a `MUST` of RFC 3561 as an AODV-interoperability requirement, not as an IETF-wide one.
