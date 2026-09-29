# MLD — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). The MLD pass ran as the twin of the IGMP pass
on 2026-09-24, and most of what it learned about the scenarios, the model shape and the tooling is
in [`igmp/notes.md`](../igmp/notes.md): the ini order, the link stimulus, the channel disabled at
initialization, one Report per query sequence, the requests module, the margins and the windows.
All of it holds for MLD as well. What follows is what MLD added, at level 2 and in the repairs of
the seventeen gaps on 2026-09-25 to 2026-09-28, on `topic/standards-tests-igmp-mld-level2-fixes`.
The gaps themselves are in [`results.md`](results.md#the-model-gaps). A section with a **Fixed
on** paragraph describes the model before the repair, and its line numbers are those of the level
2 commit.

## Model quirks

### `Mldv2` is a port of `Igmpv3`, defects included

The two modules have the same functions with the same names, and twelve of the thirteen IGMP
gaps have a twin in `Mldv2`, at the same place of the same function. The one that has none, the
precedence of the Reports, has no MLD rule. A repair of an IGMP gap is half of a repair: do the
twin in `Mldv2.cc` in the same change, and run both suites.

The repairs kept the two modules parallel: each IGMP repair has an `Mldv2` commit after it, with
the same names (`setQuerierFields`, `cancelHostTimers`, `sendSourceSpecificQueries`). Two
differences stay. The TO_EX loop of `Mldv2` in EXCLUDE mode was right where the `Igmpv3` loop was
wrong. And RFC 9777 has no rule that lowers the version of a router on an older Query, so the
MLDv1 mode of a router comes from its parameter `routerVersion` only, where the IGMPv2 mode of
`Igmpv3` also comes from an IGMPv2 Query.

### The MLDv1 module stops the run on an MLDv2 Query

**Fixed on** 2026-09-25 (gap 16): `Mldv1::processQuery` reads the Query as an `MldMessage`, the
base of both Query classes, and an MLDv2 Query is an MLDv1 Query to it. An earlier commit made
`Mldv1` ignore an MLDv2 Report, which stopped it with "Cannot convert chunk from type
inet::Mldv2Report to type inet::MldMessage" once the hosts reported their solicited-node groups
(gap 5) before the first MLDv1 Query. The throw on another unknown type stays. What follows
describes the model before the repair.

`Mldv1::processQuery` reads every Query as an MLDv1 Query of 24 octets
([Mldv1.cc:351](../../../../../src/inet/networklayer/icmpv6/Mldv1.cc#L351)), and the chunk of an
MLDv2 Query cannot be read so: "Cannot convert chunk from type inet::Mldv2Query to type
inet::MldQuery". Host and router both stop on it. RFC 9777 keeps an MLDv2 querier on MLDv2
Queries when MLDv1 listeners are present (RFC9777-COMPR-23), so every link with an MLDv1 node and
an MLDv2 querier stops at the start, and the model's own MLDv1 module cannot play the older host.
Two checks have no verdict on the router until this is repaired (gap 16). The older querier works:
an MLDv1 router and MLDv2 hosts run together, because the hosts switch to MLDv1 mode.

`Mldv1` also throws on a message type that it does not know
([Mldv1.cc:342](../../../../../src/inet/networklayer/icmpv6/Mldv1.cc#L342)), where `Mldv2` logs
and drops it; that case needs a crafted message, level 3.

### The documentation of `Mldv2` is stale

The "parity gaps" of [Mldv2.ned](../../../../../src/inet/networklayer/icmpv6/Mldv2.ned) say that
State-Change Reports and specific Queries are not repeated and that MLDv1 interoperation is
missing. All three work: the level 1 look found the later commits that added them, and
`Rfc9777StartRepeatedCount`, `Rfc9777LastListenerQuery` and `Rfc9777ListenerV1Mode` reach the
code. Do not plan a check around the comment.

### An MLD message leaves from the global address

**Fixed on** 2026-09-25 (gap 2): `Mldv1` and `Mldv2` give the link-local address as the source,
when Duplicate Address Detection has tested it. Before that, a node has no such address, so the
querier of an interface starts when it has one, up to about 2.3 s after the start, and a Report or
a Done waits in the module. RFC 3810 §5.2.13 would send it from the unspecified address, but the
IPv6 layer of the model replaces an unspecified source with its preferred address. The modules
learn the moment from `interfaceIpv6ConfigChanged`, which `Ipv6InterfaceData` now emits when an
address becomes tentative or valid. What follows describes the model before the repair.

`Mldv2::sendToIPv6` gives the IPv6 layer no source address
([Mldv2.cc:1420-1430](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc#L1420)), and IPv6 then
takes the global address of the interface, also for a message to ff02::16 (gap 2). The MLDv1
messages leave from the global address too. The election of the querier still picks the right
router in the mockups, because the global addresses of R1 and R2 sort as their link-local
addresses do; a mockup where they do not would show a second effect of the gap.

### There is no Router Alert option for IPv6 in the model

**Fixed on** 2026-09-25 (gap 3): `Ipv6RouterAlertOption` is option type 5, and
`createIpv6RouterAlertHeader()` gives the Hop-by-Hop Options header of 8 octets that both MLD
modules ask for. The repair found two defects on its way. `Ipv6::encapsulate` set the Payload
Length before it inserted the requested extension headers, so the receiver cut off the end of
the message; a commit of its own corrected it. And a multicast router delivered every ICMPv6
multicast datagram locally: with the new header, the next header of an MLD message is 0, so the
router now delivers a multicast datagram locally when it carries the option. What follows
describes the model before the repair.

The option types of
[Ipv6ExtensionHeaders.msg](../../../../../src/inet/networklayer/ipv6/Ipv6ExtensionHeaders.msg)
hold no type 5 of RFC 2711, and no MLD message carries a Hop-by-Hop Options header (gap 3). No
comment names it. A repair needs the option first, then the header on each MLD message.

### The ICMPv6 checksum has no pseudo-header, in every ICMPv6 message

**Fixed on** 2026-09-25 (gap 4): the `Ipv6` module computes the checksum in its post-routing hook
with the new `Icmpv6::insertComputedChecksum()`, because only there is the final source address
known. The pseudo-header holds the final destination of a Routing header and the home address of
a Home Address option; an error about a local datagram gets its checksum in `sendErrorMessage`.
The 23 fingerprints of the IPv6 examples moved in ~tND, and a debug run of each with the same
modes showed no drop for a checksum. What follows describes the model before the repair.

`Icmpv6::insertChecksum` sums the serialized message alone, and `Icmpv6::verifyChecksum` checks
the same sum ([Icmpv6.cc:452-475](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L452)).
Two nodes of the model agree with each other, so no run shows it, and the ND pass left the
checksum to "the ICMPv6 suite". The MLD checks read it because RFC 9777 §5.1.2 and §5.2.2 name the
pseudo-header, and the exploration confirmed the cause: the sum of the message alone is correct in
every message of the runs, and the sum with the pseudo-header in none (gap 4). A repair changes
every ICMPv6 message, Neighbor Discovery too, and changes no verdict between two model nodes.

### A node never reports its solicited-node addresses

**Fixed on** 2026-09-25 (gap 5), with the change of the ND repair branch applied again:
`Ipv6InterfaceData::assignAddress` joins the solicited-node group of a unicast address, and
`removeAddress` leaves it. Because a Report waits for the link-local address (gap 2), the Reports
for these groups leave at about 2 s, after Duplicate Address Detection and not before it as RFC
4862 §5.4.2 asks. What follows describes the model before the repair.

A node accepts a packet to a solicited-node address by a match with its own addresses
([Ipv6InterfaceData.cc:375](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc#L375))
and never joins the group, so MLD has nothing to report (gap 5). The ND pass found the same cause
from the other side (its gap 9). A repair joins the groups in the interface data; then MLD reports
them without a change of its own.

### MLD is off by default, and the default module is MLDv1

`Ipv6NetworkLayer` has `hasMld = false`, and the type of its `mld` submodule defaults to `Mldv1`.
An MLD scenario needs `**.ipv6.hasMld = true` and `**.ipv6.mld.typename = "Mldv2"`, before any line
that names an older node.

### The routers report their own groups

A router runs the listener part of MLDv2 too, as RFC9777-RQ-4 asks, and reports ff02::2 and
ff02::16 at the start, and since the repair of gap 5 its solicited-node groups, from its
link-local address since the repair of gap 2. A filter that looks for "the first Report" must
name the address or the sender, or it takes the Report of the router.

### The two MLDv2 default intervals of RFC 3810, and a third one

**Fixed on** 2026-09-25 (gaps 6 and 7): both defaults of `Mldv2.ned` follow RFC 9777, and the
Older Version Querier Present Timer runs for the interval of §9.12. The Older Version Host Present
Timer of a router ran with `groupMembershipInterval`; it has its own parameter now,
`olderVersionHostPresentInterval`, which keeps 260 s. No test measures it (RFC9777-TIMER-19 is
owed). What follows describes the model before the repair.

`Mldv2.ned` has the RFC 3810 value of the Multicast Address Listening Interval, 260 s (line 76),
and the RFC 2710 value of the Unsolicited Report Interval, 10 s (line 82), as `Igmpv3.ned` has. The
Older-Version-Querier-Present Timer of a host runs for the Other Querier Present Interval, 255 s
([Mldv2.cc:1116](../../../../../src/inet/networklayer/icmpv6/Mldv2.cc#L1116)), where RFC 9777 §9.12
gives 260 s: 5 s off, not the 95 s of the IGMP twin, which is why the check needed a tight
bracket.

### A Multicast Address Record has no Aux Data Len field

`Mldv2MulticastAddressRecord` holds its auxiliary data as an array
([Mldv2Message.msg](../../../../../src/inet/networklayer/icmpv6/Mldv2Message.msg)), and the length
comes from the size of the array. The record types are `MLD_MODE_IS_INCLUDE` and the five others
of the enum `MldGroupRecordType`; the checks use those names.

## Scenario quirks

### The IPv6 mockups

- `Router6` and `StandardHost6`, with a fixed MAC address on every interface
  (`0A-00-00-00-00-XX`), so the interface identifiers and the link-local addresses are known:
  fe80::800:ff:fe00:XX. R1 gets 01 and R2 02, so R1 is the lower address.
- `Ipv6NetworkConfigurator` with `assignAddressesToHosts = true` and `addStaticRoutes = false`:
  every node gets the global address of the mockup from the `config` XML at the start — the
  sources need one to send — and the configurator adds no route. The hosts also form SLAAC
  addresses, and can learn a default route, from the advertisements of R; a check that counts the
  addresses of a host must allow for them.
- `**.ipv6.configurator.networkConfiguratorModule = "configurator"`, as in every IPv6 mockup.
- The multicast addresses are of global scope, ff0e::1:1 and its neighbors: IPv6 never forwards
  a multicast datagram of scope 2 or less off the link.
- The route of R comes from `MulticastLeafRoute6` of
  [MldChecks.h](../../../../../tests/protocol/mld/MldChecks.h), on `.ipv6.routingTable`, with the
  unspecified group, which matches every group.
- Since the repair of gap 2, a router sends its first Query when it has a tested link-local
  address, up to about 2.3 s after the start. The checks wait 3 s for it. The MLDv1 router of the
  compatibility checks has no boot delay and `DupAddrDetectTransmits = 0`, which RFC 4862 §5.1
  allows, so its Queries come at the times that the checks compute from 0.
- Since the repair of gap 5, every node reports its solicited-node groups at about 2 s, and in
  MLDv1 mode it answers the Queries for them too. A step that takes "a Report of A" must name the
  group, or it takes one of these. Four tests needed that correction, each in a `tests:` commit
  of its own.

### The older host cannot be the MLDv1 module

**Fixed on** 2026-09-25 (gap 16): the MLDv1 module is the older host of `Rfc9777RouterV1Listener`
and `Rfc9777BlockInV1Mode`, and both pass. What follows describes the model before the repair.

Because `Mldv1` stops on the first MLDv2 Query of R, the two checks with an older host stop at the
start. The tests keep the faithful mockup and fail on gap 16; there is no other MLDv1 host in the
model. An MLDv2 host in MLDv1 mode would need an MLDv1 General Query on the link, and an MLDv1
router that sends one also stops on the Queries of R.

### A configuration option that the model does not have

**Fixed on** 2026-09-28 (gap 17): the parameter `routerVersion` of `Mldv2`, 2 by default, puts a
router into MLDv1 mode, and `Rfc9777QuerierConfiguredV1` sets it to 1 and passes. What follows
describes the model before the repair.

The check of the querier with an MLDv1 router configures R1 into MLDv1 mode, as RFC 9777 §8.3.1
asks of an administrator. `Mldv2` has no such parameter, so the test runs R1 with its defaults and
declares the failure expected: a missing feature. A test cannot set a parameter that does not
exist; its description says what it would set.

## Tooling quirks

### Reading an MLD message out of a frame

- The extension headers of IPv6 are chunks of their own between `Ipv6Header` and the ICMPv6
  message, so `findChunk<Ipv6HopByHopOptionsHeader>` finds the Hop-by-Hop Options header, and the
  protocol of the upper layer is the next header of that chunk when it is there, else the one of
  the IPv6 header (`upperProtocolOf`).
- `Icmpv6Header` is the base of all four MLD messages; `MldMessage` is the base of the Query of
  both versions, so the multicast address and the Maximum Response Code of a Query come from it.
  `Mldv2Report` extends `Icmpv6Header` directly.
- `mldChecksumOk` builds the pseudo-header from the IPv6 addresses (`Ipv6Address::words()`, most
  significant octet first), the length of the message and the next header 58, and sums it with the
  serialized message.
- `solicitedNodeOf` and `addressesOf` give the solicited-node address of each address of an
  interface at the time of the assertion.

### The exploration dump

`mld-explore.py` and `explore-mld.sh` in `audit/igmp-mld-level2/` of `inet-master` (outside git)
write `ZzExplore.test`, the MLD form of the IGMP dump: every MLD message of the named MACs with
source, destination, Hop Limit, Router Alert, next header, checksum with and without the
pseudo-header, and the fields of the Query and the records. The comparison of the two checksums
decided gap 4. Never commit it.

## Follow-ups, in the order I would do them

The seventeen gaps are repaired, on `topic/standards-tests-igmp-mld-level2-fixes`; the follow-ups
that repaired them are gone from this list.

1. **Merge the solicited-node change once.** The ND repair branch and this branch carry the same
   change of `Ipv6InterfaceData`, in two commits; the second merge must take one of them.
2. **Send the Reports before Duplicate Address Detection**, from the unspecified address, as RFC
   3810 §5.2.13 and RFC 4862 §5.4.2 ask. The IPv6 layer of the model replaces an unspecified
   source now; a repair needs a way to keep it.
3. **Make `Mldv1` ignore every message type that it does not know**, as `Mldv2` does
   ([Mldv1.cc:378](../../../../../src/inet/networklayer/icmpv6/Mldv1.cc#L378)).
4. **Correct the documentation of `Mldv2.ned`**: remove the stale "parity gaps", and name RFC 9777
   in place of RFC 3810.
5. **Level 2 and the 58 owed checks**: see
   [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes). Three of them
   measure what the repairs changed without a test: the Older Version Host Present Interval
   (RFC9777-TIMER-19, COMPR-17) and the BLOCK record in EXCLUDE mode (RFC9777-RREP-25).
6. **Level 3**: the receiver rules of MLD-F-MESSAGE-VALIDATION need crafted messages.
