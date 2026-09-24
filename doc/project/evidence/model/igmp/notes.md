# IGMP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks); this pass added its own there too. What
follows is what IGMP added, in one pass: level 2 on 2026-09-24, together with MLD, whose
[`notes.md`](../mld/notes.md) holds the IPv6 side. The gaps themselves are in
[`results.md`](results.md#the-model-gaps); this document says what is behind them and around
them.

## Model quirks

### The IGMPv2 router stops the run on a Version 3 Report

`Igmpv2` throws "Unhandled message type (34)" on an IGMPv3 Report
([Igmpv2.cc:488](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc#L488)). An ordinary IGMPv3
host sends such a Report whenever it is in IGMPv3 mode, so a link with an IGMPv2 router and an
IGMPv3 host stops the run as soon as the host's IGMPv2 mode ends (gap 4 ends it early) or a
pending Version 3 retransmission leaves after an IGMPv2 Query (gap 7). RFC 2236 §2 says
"Unrecognized message types should be silently ignored", but §2 is outside the in-scope set of
this pass, so no check targets it.

The check of the mode change had to work around it: R is off L1 while A sends its Version 3
Report, and joins L1 just before its own IGMPv2 Query. When the test fails, it decides at A's
MAC, before R receives the Report, so the run ends with the verdict and not with the error.

### Neither IGMP module has a lifecycle

`Igmpv2` and `Igmpv3` are plain `SimpleModule`s. A router with `status.initialStatus = "down"`
stops the simulation at initialization — "Tag 'inet::Ipv4InterfaceData' is absent", from
`Igmpv3::initialize` ([Igmpv3.cc:117](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L117)),
because a node that is down has no IPv4 data yet. A router that crashes keeps its timers, and its
next Query stops the simulation: "Message 'Igmpv3 query' received when Ipv4 is down". So no
check of this pass stops a node; see [the link stimulus](#a-router-that-comes-up-late-or-stops-break-its-link-at-run-time).
`Mldv2` has a lifecycle; `Igmpv3`, which it was ported from, has none.

### The model has no multicast routing that works in the mockup with a source

`Ipv4NetworkConfigurator` never marks a multicast route as leaf ("TODOisLeaf",
[Ipv4NetworkConfigurator.cc:1267](../../../../../src/inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.cc#L1267)
and line 2031), so IPv4 forwards to every out interface and never asks IGMP. PIM-DM in the
mockup did not forward at all: "Route does not have any outgoing interface and source is not
directly connected" ([PimDm.cc:1430](../../../../../src/inet/routing/pim/modes/PimDm.cc#L1430)).
The tests add one route with the test module `MulticastLeafRoute` of
[IgmpChecks.h](../../../../../tests/protocol/igmp/IgmpChecks.h): origin L2, group 0.0.0.0 (which
matches every group), in interface eth1, out interface eth0 as a leaf, added at
`INITSTAGE_LAST`. With a leaf route, IPv4 asks `Ipv4InterfaceData::hasMulticastListener` before
it forwards, and that is the question IGMP answers.

### The IGMP router knows the sources; the forwarding does not ask

`Igmpv3::processReport` stores the forwarded sources of each group with `setMulticastListeners`
([Igmpv3.cc:1093](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L1093)), and the interface
data answers `hasMulticastListener(group, source)`. IPv4 calls the overload with the group only
([Ipv4.cc:534](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L534) and line 803). The first
design of the pass declared source-specific forwarding a missing feature; the question of the
guide, "does code exist for this specific behavior?", found both halves and made it a defect,
gap 12. Look for both halves before a declaration.

### The IGMPv2 mode of a host has no timers

`Igmpv3::processOlderVersionQuery` answers an IGMPv2 Query in the same event, for every joined
group ([Igmpv3.cc:1128-1133](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L1128)), and
`multicastSourceListChanged` sends one IGMPv2 Report for a join and returns (lines 198-216). So
the IGMPv2 mode has no Delaying Member state: no random delay, no repetition, no suppression.
The three failures of gap 6 are one missing state machine. The answer leaves 12 microseconds
after the Query, the transmission time, which is why "not at the instant" has a margin of 1 ms.

### The host keeps a group that it left

A leave sets the group entry of the host to INCLUDE({}) and keeps it. Every General Query then
gets a MODE_IS_INCLUDE record without sources for that group, for as long as the host runs
(`processHostGeneralQueryTimer`,
[Igmpv3.cc:557-563](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L557)).
`processHostGroupQueryTimer` sends IS_IN(A ∩ B) without a look at its size (lines 602-611). Both
are gap 8. A run longer than one Query Interval shows the first one in every test with a leave.

### The retransmission of a source-specific Query picks the wrong sources

`Igmpv3::processRexmtTimer` retransmits only the sources whose timer is above the Last Member
Query Time, and never sets the S flag of a Group-and-Source-Specific Query
([Igmpv3.cc:464-532](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L464)). RFC 9776 §6.6.3.2
sends those sources with the flag set, and the others in a second message with the flag clear.
No run reaches this code in the case that shows it, because gap 10 cancels the retransmission
first and gap 11 keeps every timer high. Repair it with gaps 10 and 11.

### The IGMPv2 router sends its Queries without Router Alert and precedence

`Igmpv2::sendToIP` has "TODO add Router Alert option" above it
([Igmpv2.cc:728-729](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc#L728)); the Reports and
Leaves of `Igmpv2` carry the option (lines 703-705 and 721-723). No check reads the messages of
the IGMPv2 router for it.

### Two comments of the code are stale

The "TODO fill Router Alert option" comments of `Igmpv3` stand above code that attaches the
option, so the level 1 fact about the Router Alert is no longer true for `Igmpv3`. The "FIXME
also accept Igmpv1Report and Igmpv2Report" at
[Igmpv3.cc:815](../../../../../src/inet/networklayer/ipv4/Igmpv3.cc#L815) is stale too: the
dispatcher sends the older Reports to `processOlderVersionReport` (lines 672-677).

### The Robustness Variable of a host counts its State-Change Reports

The parameter `robustnessVariable` of a host sets how many times it sends each State-Change
Report. A host does not adopt the QRV of the querier — which is 0 anyway, gap 1. The tests use
the parameter to make a host send each Report once; see
[one Report, one query sequence](#one-report-starts-one-query-sequence).

## Scenario quirks

### The first matching ini line wins

The generator wrote `**.ipv4.igmp.typename = "Igmpv3"` before the lines of the test, so
`*.router.ipv4.igmp.typename = "Igmpv2"` never applied, and every "older" node ran IGMPv3. Every
test with an older node failed for it in the first run. The lines of a test now come first in the
ini file.

### A router that comes up late or stops: break its link, at run time

Because the IGMP modules have no lifecycle, the checks never stop a node. A node joins or leaves
L1 through its link instead:
`<set-channel-param src-module='router' src-gate='ethg[0]' par='disabled' value='true'/>` in the
`ScenarioManager`, and `value='false'` to join again. The rule is in
[`checks.md`](../../protocol/igmp/checks.md#rules-every-check-obeys), in words of the standard.
The MAC of a disabled link drops its frames as "not connected" and emits no `packetSentToLower`,
so a check that reads the sender's MAC sees nothing while the link is broken.

### A channel disabled at initialization never comes back

`Eth100M { disabled = true; }` in the NED, enabled later by the scenario, looks like the simple
form of "comes up late", and it does not work: `EthernetMacBase` subscribes to the parameters of
its channel only when the channel is enabled at initialization
([EthernetMacBase.cc:428](../../../../../src/inet/linklayer/ethernet/base/EthernetMacBase.cc#L428)),
so the later enable never reaches the MAC, which drops every frame as "not connected". Start every
link enabled; to keep a router off L1 from the start, break its link at 1 s, after its first
Query.

### One Report starts one query sequence

A leave of the only member makes R send a Group-Specific Query at once and one more 1 s later.
The repetition of the leave, up to 1 s after it with the interval of the standard, is a new
TO_IN record, and RFC 9776 §6.6.3.1 answers it with a new sequence. The first run read the second
Query of a new sequence as the retransmission of the first, 0.2 s after it. The checks that read
a query sequence give the leaving host a Robustness Variable of 1; the rule is in
[`checks.md`](../../protocol/igmp/checks.md#rules-every-check-obeys).

### The requests of a host come from a test module

`IgmpRequests` of [IgmpChecks.h](../../../../../tests/protocol/igmp/IgmpChecks.h) follows a timed
script, "time request group INCLUDE|EXCLUDE sources", and calls
`NetworkInterface::changeMulticastGroupMembership` with the old and the new filter of each named
request. That is what a socket does, and one module serves every filter mode and every host. Two
requests with different names on one group make the union of RFC 9776 §3.

### Only the routers forward multicast

`multicastForwarding = true` on every node made the hosts IGMP routers too, and they sent Queries.
Set it on the routers only. The source sends with `timeToLive = 16` and `multicastInterface =
"eth0"`, so that its datagrams live through the hop at R.

### Every observation must be able to fail in its scenario

The check of the answer to a Group-and-Source-Specific Query had the observation "no record
without sources", in a scenario where A wanted every queried source, so the observation could
never fail. The MLD twin found it, when an empty record appeared in another test. The corrected
check queries a source that A does not want, and both models fail it. For every observation, ask
what a wrong model would do in that scenario, and whether the observation would see it.

### A window starts at the match of the step before it

`within(t)` of a step counts from the match of the step before it, not from the start. Two first
runs failed on it: the Leave of `Rfc9776LeaveV2Mode` comes 20 s after the Report that the step
before it matched, and the window was 11 s; and the step "a datagram 1.5 s or more after the
leave" had a window of 1.5 s, which ends 20 microseconds before the datagram at 51.50003706 s.
A datagram of the source arrives at x.00003706 and x.50003706 s: leave a margin.

### "From A or from B", a counter, and a step that may not come

- One event of either of two nodes is one `anyOf` step with two patterns, one per node.
- A count over two nodes is two guards that increment one pair of counters, each of which checks
  the sum.
- A second Query that the model may not send, and whose flag the check reads if it comes, is an
  `atMostOnce` step: the test then passes on the first Query alone, and the ledger says so.

## Tooling quirks

### A filter that throws is a non-match, and loses what it did after the throw

The exploration dump compared `c.event.time > 1e9`. The constant overflows the range of
`SimTime`, the comparison throws, and the tester counts the filter as a non-match — after the
dump had printed half of its line and before it updated its record. The output looked as if every
second event were lost. Keep constants inside the range of `SimTime`, and suspect a throw when a
filter seems to miss events.

### The failure line names the pattern, not the description

The tester prints the reason of a failure as `reason: ...` in `work/<Test>/test.out`, a few lines
after `PROTOCOLTEST <name>: FAIL`, with the pattern of the step (`on ... signal=... within=...`)
and not the text of `describe()`. An assertion that throws a sentence is the readable part; a
missed deadline says only which step it was. Collect the reasons with
`grep -A8 ": FAIL" work/*/test.out | grep reason`, and run one test with the `-f` filter of
`inet_run_protocol_tests`.

### A guard holds the run until its window closes

A step started with `meanwhile(...)` starts at the match of the step before it and runs until its
own window closes, also after the last ordered step has matched. A guard whose window is longer
than the observation keeps the run open to `sim-time-limit`. Size a guard window to the
observation.

### The exploration dump

`igmp-explore.py` and `explore.sh` in `audit/igmp-mld-level2/` of `inet-master` (outside git)
write `ZzExplore.test`: a test whose guards print every IGMP message of the named MACs, with its
fields, and the forwarded datagrams with their gaps. It found the cause of every failure of the
pass. Never commit it.

## Follow-ups, in the order I would do them

1. **Make `Igmpv2` ignore a message type that it does not know.** One line at
   [Igmpv2.cc:488](../../../../../src/inet/networklayer/ipv4/Igmpv2.cc#L488). No check of this
   pass fails on it, but every mixed-version link can stop on it.
2. **The few-line gaps**: set the QRV and the QQIC (gap 1), uncomment the DSCP line of the
   Reports (gap 2), correct the two NED defaults (gap 3), give the Older Version Querier Present
   Timer its own interval (gap 4), lower the Group Timer before the S flag is computed (gap 9),
   and send no empty record and drop the entry of a left group (gap 8). Six gaps, eight tests.
3. **The router's query sequence**: keep the retransmission state per source when a Report
   arrives (gap 10), lower the Source Timers of a Group-and-Source-Specific Query (gap 11), and
   split its retransmission into the two messages of §6.6.3.2 at the same time.
4. **Ask the forwarding for the source** (gap 12): the one-argument call at `Ipv4.cc:534` and 803
   becomes the two-argument one. Source-specific multicast starts to work.
5. **The host's state machines**: merge a later change into the pending records (gap 5), give
   the IGMPv2 mode its Delaying Member state (gap 6), and cancel the timers on a mode change
   (gap 7).
6. **The IGMPv2 querier mode of an IGMPv3 router** (gap 13), the one missing feature; its test
   declares the failure expected, and the declaration goes with the implementation.
7. **A lifecycle for `Igmpv2` and `Igmpv3`**, as `Mldv2` has.
8. **Level 3 and the 49 owed checks**: see
   [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes).
9. **Name RFC 9776 in `Igmpv3.ned`**: the claim of RFC 3376 is obsolete; see
   [`conformance.md`](conformance.md#headlines-for-the-next-pass).
