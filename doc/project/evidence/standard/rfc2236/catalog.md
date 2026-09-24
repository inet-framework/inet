# RFC 2236 (Internet Group Management Protocol, Version 2) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2236-*` · **Stands on:** [standards.md](../../protocol/igmp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 2236, Internet Group Management Protocol, Version 2, of
November 1997, a document of the in-scope set of IGMP. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc2236.txt`](../../../../../../standards/RFC/rfc2236.txt) —
  Internet Group Management Protocol, Version 2, November 1997. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc2236.txt>.

IGMPv3 itself is in [`rfc9776/catalog.md`](../rfc9776/catalog.md); this document gives the
version 2 behavior that the compatibility rules of RFC 9776 §7 fall back to. The in-scope
sections are §6 and §7.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/igmp/standards.md). The features these statements build are in
[`features.md`](../../protocol/igmp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`igmp/coverage.md`](../../model/igmp/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc2236.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC2236-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 2236 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them, and
  `description` for a normative sentence with no keyword. Where the text writes a keyword in
  lower case, the entry records the word and says so, for example `must (lower case)`; a
  "might" stays `description`. An entry with two keywords names what each one covers. A row of
  a state table that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link, or
  the presence or absence of a message), `end-to-end` (what a node accepts and acts on, seen
  in what it sends or forwards afterwards), `error-signal` (an error report),
  `internal` (state inside a node), `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC2236-HOST-1](#rfc2236-host-1) | In Non-Member state, a host does not belong to the group on the interface. |
| [RFC2236-HOST-2](#rfc2236-host-2) | In Delaying Member state, a host belongs to the group and has a report delay timer running. |
| [RFC2236-HOST-3](#rfc2236-host-3) | In Idle Member state, a host belongs to the group and has no report delay timer running. |
| [RFC2236-HOST-4](#rfc2236-host-4) | The join group event occurs, in Non-Member state only, when a host decides to join a group on an interface. |
| [RFC2236-HOST-5](#rfc2236-host-5) | The leave group event occurs, in Delaying Member or Idle Member state only, when a host decides to leave a group. |
| [RFC2236-HOST-6](#rfc2236-host-6) | The query received event occurs when a host receives a valid General or Group-Specific Membership Query. |
| [RFC2236-HOST-7](#rfc2236-host-7) | A Query message is valid only when it is at least 8 octets long and carries a correct IGMP checksum. |
| [RFC2236-HOST-8](#rfc2236-host-8) | The group address field of a Query message must be zero for a General Query or a valid multicast group address for a Group-Specific Query. |
| [RFC2236-HOST-9](#rfc2236-host-9) | A General Query affects every membership on the interface; a Group-Specific Query affects only the named membership; a query for a membership in Non-Member state is ignored. |
| [RFC2236-HOST-10](#rfc2236-host-10) | The report received event occurs when a host receives a valid Version 1 or Version 2 Membership Report. |
| [RFC2236-HOST-11](#rfc2236-host-11) | A Membership Report message is valid only when it is at least 8 octets long and carries a correct IGMP checksum. |
| [RFC2236-HOST-12](#rfc2236-host-12) | A Membership Report affects only the reported group on the interface it arrives on, and is ignored for a membership in Non-Member or Idle Member state. |
| [RFC2236-HOST-13](#rfc2236-host-13) | The timer expired event occurs, in Delaying Member state only, when the report delay timer for the group expires. |
| [RFC2236-HOST-14](#rfc2236-host-14) | A host ignores every other event, such as an invalid IGMP message or an IGMP message that is not a Query or a Report, in all states. |
| [RFC2236-HOST-15](#rfc2236-host-15) | The send report action sends the Report Message to the group being reported, with the report type set by the interface state. |
| [RFC2236-HOST-16](#rfc2236-host-16) | A host should skip the send leave action when the interface state says the Querier runs IGMPv1. |
| [RFC2236-HOST-17](#rfc2236-host-17) | A host may skip the send leave action when the flag that it was the last host to report is cleared. |
| [RFC2236-HOST-18](#rfc2236-host-18) | The Leave Message is sent to the ALL-ROUTERS group, 224.0.0.2. |
| [RFC2236-HOST-19](#rfc2236-host-19) | The set flag action marks that the host was the last host to send a report for the group. |
| [RFC2236-HOST-20](#rfc2236-host-20) | The clear flag action marks that the host was not the last host to send a report for the group. |
| [RFC2236-HOST-21](#rfc2236-host-21) | The start timer action chooses a delay value uniformly from the interval (0, Max Response Time], where Max Response Time comes from the Query. |
| [RFC2236-HOST-22](#rfc2236-host-22) | For an unsolicited report, the start timer action chooses a delay value uniformly from the interval (0, Unsolicited Report Interval]. |
| [RFC2236-HOST-23](#rfc2236-host-23) | The reset timer action sets the timer to a new value, chosen the same way as the start timer action. |
| [RFC2236-HOST-24](#rfc2236-host-24) | The stop timer action stops the report delay timer for the group on the interface. |
| [RFC2236-HOST-25](#rfc2236-host-25) | On the join group event, a host in Non-Member state sends a report, sets the flag, starts the timer, and moves to Delaying Member state. |
| [RFC2236-HOST-26](#rfc2236-host-26) | On the leave group event, a host in Delaying Member state stops the timer, sends a leave if the flag is set, and moves to Non-Member state. |
| [RFC2236-HOST-27](#rfc2236-host-27) | On the leave group event, a host in Idle Member state sends a leave if the flag is set, and moves to Non-Member state. |
| [RFC2236-HOST-28](#rfc2236-host-28) | On the query received event, a host in Idle Member state starts the timer and moves to Delaying Member state. |
| [RFC2236-HOST-29](#rfc2236-host-29) | On the report received event, a host in Delaying Member state stops the timer, clears the flag, and moves to Idle Member state. |
| [RFC2236-HOST-30](#rfc2236-host-30) | On the query received event, a host in Delaying Member state resets the timer only if the query's Max Response Time is less than the current timer, and stays in Delaying Member state. |
| [RFC2236-HOST-31](#rfc2236-host-31) | On the timer expired event, a host in Delaying Member state sends a report, sets the flag, and moves to Idle Member state. |
| [RFC2236-HOST-32](#rfc2236-host-32) | The all-systems group, 224.0.0.1, is a special case: a host stays an Idle Member for it on every interface and never sends a report for it. |
| [RFC2236-HOST-33](#rfc2236-host-33) | In No IGMPv1 Router Present state, a host has not heard an IGMPv1-style query within the Version 1 Router Present Timeout. |
| [RFC2236-HOST-34](#rfc2236-host-34) | In IGMPv1 Router Present state, a host has heard an IGMPv1-style query within the Version 1 Router Present Timeout. |
| [RFC2236-HOST-35](#rfc2236-host-35) | The IGMPv1 query received event occurs when a host receives a query with the Max Response Time field set to 0. |
| [RFC2236-HOST-36](#rfc2236-host-36) | The timer expires event occurs when the timer noting the presence of an IGMPv1 router expires. |
| [RFC2236-HOST-37](#rfc2236-host-37) | The set timer action sets the version 1 router present timer to its maximum value and restarts it. |
| [RFC2236-HOST-38](#rfc2236-host-38) | On the IGMPv1 query received event, a host in No IGMPv1 Router Present state sets the timer and moves to IGMPv1 Router Present state. |
| [RFC2236-HOST-39](#rfc2236-host-39) | On the timer expires event, a host in IGMPv1 Router Present state moves to No IGMPv1 Router Present state. |
| [RFC2236-HOST-40](#rfc2236-host-40) | On the IGMPv1 query received event, a host already in IGMPv1 Router Present state resets the timer and stays in IGMPv1 Router Present state. |
| [RFC2236-ROUTER-1](#rfc2236-router-1) | In Querier state, a router is designated to transmit IGMP Membership Queries on the network. |
| [RFC2236-ROUTER-2](#rfc2236-router-2) | In Non-Querier state, another router is designated to transmit IGMP Membership Queries on the network. |
| [RFC2236-ROUTER-3](#rfc2236-router-3) | The query timer expired event occurs when the timer set for query transmission expires. |
| [RFC2236-ROUTER-4](#rfc2236-router-4) | The query received from a router with a lower IP address event occurs when a router hears an IGMP Membership Query from another router on the same network with a lower IP address. |
| [RFC2236-ROUTER-5](#rfc2236-router-5) | The other querier present timer expired event occurs when the timer noting the presence of another, lower-IP-address querier expires. |
| [RFC2236-ROUTER-6](#rfc2236-router-6) | The start general query timer action starts the general query timer for the attached network. |
| [RFC2236-ROUTER-7](#rfc2236-router-7) | The start other querier present timer action starts that timer with the Other Querier Present Interval. |
| [RFC2236-ROUTER-8](#rfc2236-router-8) | The send general query action sends a General Query to the all-systems group, 224.0.0.1, with a Max Response Time of the Query Response Interval. |
| [RFC2236-ROUTER-9](#rfc2236-router-9) | On entry, a router in Initial state sends a general query, sets the initial general query timer, and moves to Querier state. |
| [RFC2236-ROUTER-10](#rfc2236-router-10) | On the query timer expired event, a router in Querier state sends a general query, restarts the general query timer, and stays Querier. |
| [RFC2236-ROUTER-11](#rfc2236-router-11) | On the query received from a router with a lower IP address event, a router in Querier state sets the other querier present timer and moves to Non-Querier state. |
| [RFC2236-ROUTER-12](#rfc2236-router-12) | On the other querier present timer expired event, a router in Non-Querier state sends a general query, sets the general query timer, and moves to Querier state. |
| [RFC2236-ROUTER-13](#rfc2236-router-13) | On the query received from a router with a lower IP address event, a router in Non-Querier state resets the other querier present timer and stays Non-Querier. |
| [RFC2236-ROUTER-14](#rfc2236-router-14) | A router should start in the Initial state on every attached network, and move immediately to Querier state. |
| [RFC2236-ROUTER-15](#rfc2236-router-15) | In No Members Present state, no host on the network has sent a report for the multicast group. |
| [RFC2236-ROUTER-16](#rfc2236-router-16) | In Members Present state, a host on the network has sent a Membership Report for the multicast group. |
| [RFC2236-ROUTER-17](#rfc2236-router-17) | In Version 1 Members Present state, an IGMPv1 host on the network has sent a Version 1 Membership Report for the multicast group. |
| [RFC2236-ROUTER-18](#rfc2236-router-18) | In Checking Membership state, a router has received a Leave Group message but has not yet heard a Membership Report for the group. |
| [RFC2236-ROUTER-19](#rfc2236-router-19) | The v2 report received event occurs when a router receives a Version 2 Membership Report for the group on the interface. |
| [RFC2236-ROUTER-20](#rfc2236-router-20) | A Version 2 Membership Report is valid only when it is at least 8 octets long and carries a correct IGMP checksum. |
| [RFC2236-ROUTER-21](#rfc2236-router-21) | The v1 report received event occurs, under the same validity rule as a Version 2 report, when a router receives a Version 1 Membership Report for the group on the interface. |
| [RFC2236-ROUTER-22](#rfc2236-router-22) | The leave received event occurs when a router receives an IGMP Group Leave message for the group on the interface. |
| [RFC2236-ROUTER-23](#rfc2236-router-23) | An IGMP Group Leave message is valid only when it is at least 8 octets long and carries a correct IGMP checksum. |
| [RFC2236-ROUTER-24](#rfc2236-router-24) | The timer expired event occurs when the timer set for a group membership expires. |
| [RFC2236-ROUTER-25](#rfc2236-router-25) | The retransmit timer expired event occurs when the timer set to retransmit a group-specific Membership Query expires. |
| [RFC2236-ROUTER-26](#rfc2236-router-26) | The v1 host timer expired event occurs when the timer noting the presence of version 1 hosts as group members expires. |
| [RFC2236-ROUTER-27](#rfc2236-router-27) | The start timer action starts the group membership timer at the Group Membership Interval, and also resets an already-running timer to that value. |
| [RFC2236-ROUTER-28](#rfc2236-router-28) | When this router is a Querier, the start timer* action sets the timer to the Last Member Query Interval times the Last Member Query Count. |
| [RFC2236-ROUTER-29](#rfc2236-router-29) | When this router is a non-Querier, the start timer* action sets the timer to the packet's Max Response Time times the Last Member Query Count. |
| [RFC2236-ROUTER-30](#rfc2236-router-30) | The start retransmit timer action starts that timer for the group membership at the Last Member Query Interval. |
| [RFC2236-ROUTER-31](#rfc2236-router-31) | The start v1 host timer action starts the version 1 host timer at the Group Membership Interval, and also resets an already-running timer to that value. |
| [RFC2236-ROUTER-32](#rfc2236-router-32) | The send group-specific query action sends a Group-Specific Query to the group being queried, with a Max Response Time of the Last Member Query Interval. |
| [RFC2236-ROUTER-33](#rfc2236-router-33) | The notify routing + action tells the routing protocol that the connected network has members of the group. |
| [RFC2236-ROUTER-34](#rfc2236-router-34) | The notify routing - action tells the routing protocol that the connected network no longer has members of the group. |
| [RFC2236-ROUTER-35](#rfc2236-router-35) | On the v2 report received event, a Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, and moves to Members Present state. |
| [RFC2236-ROUTER-36](#rfc2236-router-36) | On the v1 report received event, a Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state. |
| [RFC2236-ROUTER-37](#rfc2236-router-37) | On the timer expired event, a Querier router in Members Present state notifies the routing protocol that no members remain and moves to No Members Present state. |
| [RFC2236-ROUTER-38](#rfc2236-router-38) | On the v2 report received event, a Querier router already in Members Present state starts the timer and stays in Members Present state. |
| [RFC2236-ROUTER-39](#rfc2236-router-39) | On the leave received event, a Querier router in Members Present state starts the timer with the leave-triggered value, starts the retransmit timer, sends a group-specific query, and moves to Checking Membership state. |
| [RFC2236-ROUTER-40](#rfc2236-router-40) | On the v1 report received event, a Querier router in Members Present state starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state. |
| [RFC2236-ROUTER-41](#rfc2236-router-41) | On the v2 report received event, a Querier router in Checking Membership state starts the timer and moves to Members Present state. |
| [RFC2236-ROUTER-42](#rfc2236-router-42) | On the v1 report received event, a Querier router in Checking Membership state starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state. |
| [RFC2236-ROUTER-43](#rfc2236-router-43) | On the retransmit timer expired event, a Querier router in Checking Membership state sends a group-specific query, restarts the retransmit timer, and stays in Checking Membership state. |
| [RFC2236-ROUTER-44](#rfc2236-router-44) | On the timer expired event, a Querier router in Checking Membership state notifies the routing protocol that no members remain, clears the retransmit timer, and moves to No Members Present state. |
| [RFC2236-ROUTER-45](#rfc2236-router-45) | On the v2 report received event, a Querier router already in Version 1 Members Present state starts the timer and stays in Version 1 Members Present state. |
| [RFC2236-ROUTER-46](#rfc2236-router-46) | On the v1 host timer expired event, a Querier router in Version 1 Members Present state moves to Members Present state. |
| [RFC2236-ROUTER-47](#rfc2236-router-47) | On the timer expired event, a Querier router in Version 1 Members Present state notifies the routing protocol that no members remain and moves to No Members Present state. |
| [RFC2236-ROUTER-48](#rfc2236-router-48) | Non-Queriers send no messages and act only when a message arrives; a non-Querier does not distinguish a Version 1 from a Version 2 Membership Report. |
| [RFC2236-ROUTER-49](#rfc2236-router-49) | On the report received event, a Non-Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, and moves to Members Present state. |
| [RFC2236-ROUTER-50](#rfc2236-router-50) | On the timer expired event, a Non-Querier router in Members Present state notifies the routing protocol that no members remain and moves to No Members Present state. |
| [RFC2236-ROUTER-51](#rfc2236-router-51) | On the report received event, a Non-Querier router already in Members Present state starts the timer and stays in Members Present state. |
| [RFC2236-ROUTER-52](#rfc2236-router-52) | On the g-s query received event, a Non-Querier router in Members Present state starts the timer with the query-triggered value and moves to Checking Membership state. |
| [RFC2236-ROUTER-53](#rfc2236-router-53) | On the report received event, a Non-Querier router in Checking Membership state starts the timer and moves to Members Present state. |
| [RFC2236-ROUTER-54](#rfc2236-router-54) | On the timer expired event, a Non-Querier router in Checking Membership state notifies the routing protocol that no members remain and moves to No Members Present state. |

## Host state diagram

### RFC2236-HOST-1

**In Non-Member state, a host does not belong to the group on the interface.**

> ""Non-Member" state, when the host does not belong to the group on
> the interface.  This is the initial state for all memberships on
> all network interfaces; it requires no storage in the host." — §6, `rfc2236.txt:377-379`

- Strength: description. Class: internal.
- Check idea: observe that a host with no interest in a group keeps no stored membership state for that group on an interface, and starts every membership there.

### RFC2236-HOST-2

**In Delaying Member state, a host belongs to the group and has a report delay timer running.**

> ""Delaying Member" state, when the host belongs to the group on the
> interface and has a report delay timer running for that membership." — §6, `rfc2236.txt:381-383`

- Strength: description. Class: internal.
- Check idea: after a host joins a group or answers a query, observe that a report delay timer is running for that membership.

### RFC2236-HOST-3

**In Idle Member state, a host belongs to the group and has no report delay timer running.**

> ""Idle Member" state, when the host belongs to the group on the
> interface and does not have a report delay timer running for that
> membership." — §6, `rfc2236.txt:384-386`

- Strength: description. Class: internal.
- Check idea: after a host reports for a group and no query is outstanding, observe that no report delay timer runs for that membership.

### RFC2236-HOST-4

**The join group event occurs, in Non-Member state only, when a host decides to join a group on an interface.**

> ""join group" occurs when the host decides to join the group on the
> interface.  It may occur only in the Non-Member state." — §6, `rfc2236.txt:402-403`

- Strength: may (lower case). Class: internal.
- Check idea: command a host with no membership for a group to join it, and confirm the join event applies only from Non-Member state.

### RFC2236-HOST-5

**The leave group event occurs, in Delaying Member or Idle Member state only, when a host decides to leave a group.**

> ""leave group" occurs when the host decides to leave the group on
> the interface.  It may occur only in the Delaying Member and Idle
> Member states." — §6, `rfc2236.txt:405-407`

- Strength: may (lower case). Class: internal.
- Check idea: command a host that is a Delaying Member or an Idle Member of a group to leave it, and confirm the leave event does not apply in Non-Member state.

### RFC2236-HOST-6

**The query received event occurs when a host receives a valid General or Group-Specific Membership Query.**

> ""query received" occurs when the host receives either a valid
> General Membership Query message, or a valid Group-Specific
> Membership Query message." — §6, `rfc2236.txt:409-411`

- Strength: description. Class: end-to-end.
- Check idea: send a host a Membership Query message and confirm the query-received event fires only when the message is valid.

### RFC2236-HOST-7

**A Query message is valid only when it is at least 8 octets long and carries a correct IGMP checksum.**

> "To be valid, the Query message must be
> at least 8 octets long, and have a correct IGMP checksum." — §6, `rfc2236.txt:411-412`

- Strength: must (lower case). Class: wire.
- Check idea: send a host a Query message shorter than 8 octets, and separately one with an incorrect checksum, and confirm the host treats neither as valid.

### RFC2236-HOST-8

**The group address field of a Query message must be zero for a General Query or a valid multicast group address for a Group-Specific Query.**

> "The
> group address in the IGMP header must either be zero (a General
> Query) or a valid multicast group address (a Group-Specific Query)." — §6, `rfc2236.txt:412-414`

- Strength: must (lower case). Class: wire.
- Check idea: send a host queries with the group address field set to zero and to a multicast group address, and confirm it classifies them as a General Query and a Group-Specific Query.

### RFC2236-HOST-9

**A General Query affects every membership on the interface; a Group-Specific Query affects only the named membership; a query for a membership in Non-Member state is ignored.**

> "A General Query applies to all memberships on the interface from
> which the Query is received.  A Group-Specific Query applies to
> membership in a single group on the interface from which the Query
> is received.  Queries are ignored for memberships in the Non-Member
> state." — §6, `rfc2236.txt:415-419`

- Strength: description. Class: end-to-end.
- Check idea: send a host a General Query and confirm it affects every membership on the interface; send a Group-Specific Query and confirm it affects only that group; confirm a host with no membership for a group ignores a query for it.

### RFC2236-HOST-10

**The report received event occurs when a host receives a valid Version 1 or Version 2 Membership Report.**

> ""report received" occurs when the host receives a valid IGMP
> Membership Report message (Version 1 or Version 2)." — §6, `rfc2236.txt:421-423`

- Strength: description. Class: end-to-end.
- Check idea: send a host a Version 1 Membership Report and separately a Version 2 Membership Report, and confirm each fires the report-received event.

### RFC2236-HOST-11

**A Membership Report message is valid only when it is at least 8 octets long and carries a correct IGMP checksum.**

> "Membership Report message (Version 1 or Version 2).  To be valid,
> the Report message must be at least 8 octets long and have a
> correct IGMP checksum." — §6, `rfc2236.txt:422-424`

- Strength: must (lower case). Class: wire.
- Check idea: send a host a Membership Report shorter than 8 octets, and separately one with an incorrect checksum, and confirm the host treats neither as valid.

### RFC2236-HOST-12

**A Membership Report affects only the reported group on the interface it arrives on, and is ignored for a membership in Non-Member or Idle Member state.**

> "A Membership Report applies only to the
> membership in the group identified by the Membership Report, on the
> interface from which the Membership Report is received.  It is
> ignored for memberships in the Non-Member or Idle Member state." — §6, `rfc2236.txt:424-427`

- Strength: description. Class: end-to-end.
- Check idea: send a host a Membership Report for a group while it is an Idle Member, or has no membership, for that group, and confirm the report has no effect on its state.

### RFC2236-HOST-13

**The timer expired event occurs, in Delaying Member state only, when the report delay timer for the group expires.**

> ""timer expired" occurs when the report delay timer for the group on
> the interface expires.  It may occur only in the Delaying Member
> state." — §6, `rfc2236.txt:429-431`

- Strength: may (lower case). Class: internal.
- Check idea: let a host's report delay timer for a group run out while the host is a Delaying Member, and confirm the timer-expired event fires only then.

### RFC2236-HOST-14

**A host ignores every other event, such as an invalid IGMP message or an IGMP message that is not a Query or a Report, in all states.**

> "All other events, such as receiving invalid IGMP messages, or IGMP
> messages other than Query or Report, are ignored in all states." — §6, `rfc2236.txt:433-434`

- Strength: description. Class: end-to-end.
- Check idea: send a host an invalid IGMP message, or an IGMP message that is not a Query or a Report, and confirm no membership state changes.

### RFC2236-HOST-15

**The send report action sends the Report Message to the group being reported, with the report type set by the interface state.**

> ""send report" for the group on the interface.  The type of report
> is determined by the state of the interface.  The Report Message is
> sent to the group being reported." — §6, `rfc2236.txt:439-441`

- Strength: description. Class: wire.
- Check idea: trigger a send-report action for a group, and confirm the Report Message carries the reported group as its destination address.

### RFC2236-HOST-16

**A host should skip the send leave action when the interface state says the Querier runs IGMPv1.**

> ""send leave" for the group on the interface.  If the interface
> state says the Querier is running IGMPv1, this action SHOULD be
> skipped." — §6, `rfc2236.txt:455-457`

- Strength: should. Class: wire.
- Check idea: let a host leave a group while its interface's Querier is known to run IGMPv1, and confirm the host sends no Leave Message.

### RFC2236-HOST-17

**A host may skip the send leave action when the flag that it was the last host to report is cleared.**

> "If the flag saying we were the last host to report is
> cleared, this action MAY be skipped." — §6, `rfc2236.txt:457-458`

- Strength: may. Class: wire.
- Check idea: let a host leave a group when its last-reporter flag is cleared, and confirm the host is permitted to send no Leave Message.

### RFC2236-HOST-18

**The Leave Message is sent to the ALL-ROUTERS group, 224.0.0.2.**

> "The Leave Message is sent to
> the ALL-ROUTERS group (224.0.0.2)." — §6, `rfc2236.txt:458-459`

- Strength: description. Class: wire.
- Check idea: let a host send a Leave Message and confirm its destination address is 224.0.0.2.

### RFC2236-HOST-19

**The set flag action marks that the host was the last host to send a report for the group.**

> ""set flag" that we were the last host to send a report for this
> group." — §6, `rfc2236.txt:461-462`

- Strength: description. Class: internal.
- Check idea: after a host sends a report for a group, confirm its internal flag records it as the last host to report.

### RFC2236-HOST-20

**The clear flag action marks that the host was not the last host to send a report for the group.**

> ""clear flag" since we were not the last host to send a report for
> this group." — §6, `rfc2236.txt:464-465`

- Strength: description. Class: internal.
- Check idea: after a host learns that another host reported for a group, confirm its internal flag no longer marks it as the last reporter.

### RFC2236-HOST-21

**The start timer action chooses a delay value uniformly from the interval (0, Max Response Time], where Max Response Time comes from the Query.**

> ""start timer" for the group on the interface, using a delay value
> chosen uniformly from the interval (0, Max Response Time], where
> Max Response time is specified in the Query." — §6, `rfc2236.txt:467-469`

- Strength: description. Class: internal.
- Check idea: start the timer in response to a query, and confirm the chosen delay lies within (0, Max Response Time] taken from that query.

### RFC2236-HOST-22

**For an unsolicited report, the start timer action chooses a delay value uniformly from the interval (0, Unsolicited Report Interval].**

> "If this is an
> unsolicited Report, the timer is set to a delay value chosen
> uniformly from the interval (0, [Unsolicited Report Interval] ]." — §6, `rfc2236.txt:469-471`

- Strength: description. Class: internal.
- Check idea: start the timer for an unsolicited report, and confirm the chosen delay lies within (0, Unsolicited Report Interval].

### RFC2236-HOST-23

**The reset timer action sets the timer to a new value, chosen the same way as the start timer action.**

> ""reset timer" for the group on the interface to a new value, using
> a delay value chosen uniformly from the interval (0, Max Response
> Time], as described in "start timer"." — §6, `rfc2236.txt:473-475`

- Strength: description. Class: internal.
- Check idea: reset a running timer for a group, and confirm the new value is chosen from the same interval as when the timer starts.

### RFC2236-HOST-24

**The stop timer action stops the report delay timer for the group on the interface.**

> ""stop timer" for the group on the interface." — §6, `rfc2236.txt:477`

- Strength: description. Class: internal.
- Check idea: trigger a stop-timer action for a group, and confirm no report delay timer stays running for it.

### RFC2236-HOST-25

**On the join group event, a host in Non-Member state sends a report, sets the flag, starts the timer, and moves to Delaying Member state.**

> "Non-Member" — `rfc2236.txt:516`; "join group" and "(send report," and "set flag," and "start timer)" — `rfc2236.txt:522`, `rfc2236.txt:523`, `rfc2236.txt:524`, `rfc2236.txt:525`; "Delaying Member" — `rfc2236.txt:531`

- Strength: description. Class: wire.
- Check idea: command a host with no membership for a group to join it, and confirm it sends a report, sets its last-reporter flag, starts the timer, and becomes a Delaying Member.

### RFC2236-HOST-26

**On the leave group event, a host in Delaying Member state stops the timer, sends a leave if the flag is set, and moves to Non-Member state.**

> "leave group" and "(stop timer," and "send leave if" and "flag set)" — `rfc2236.txt:522`, `rfc2236.txt:523`, `rfc2236.txt:524`, `rfc2236.txt:525`; "Delaying Member" — `rfc2236.txt:531`; "Non-Member" — `rfc2236.txt:516`

- Strength: description. Class: wire.
- Check idea: command a Delaying Member host to leave a group, and confirm it stops its report delay timer and, if it was the last host to report, sends a Leave Message, then becomes Non-Member.

### RFC2236-HOST-27

**On the leave group event, a host in Idle Member state sends a leave if the flag is set, and moves to Non-Member state.**

> "leave group" — `rfc2236.txt:522`; "(send leave" and "if flag set)" — `rfc2236.txt:523`, `rfc2236.txt:524`; "Idle Member" — `rfc2236.txt:531`

- Strength: description. Class: wire.
- Check idea: command an Idle Member host to leave a group, and confirm it sends a Leave Message only if it was the last host to report, then becomes Non-Member.

### RFC2236-HOST-28

**On the query received event, a host in Idle Member state starts the timer and moves to Delaying Member state.**

> "query received" and "(start timer)" — `rfc2236.txt:530`, `rfc2236.txt:531`; "Idle Member" — `rfc2236.txt:531`

- Strength: description. Class: end-to-end.
- Check idea: send an Idle Member host a Membership Query for its group, and confirm it starts the report delay timer and becomes a Delaying Member.

### RFC2236-HOST-29

**On the report received event, a host in Delaying Member state stops the timer, clears the flag, and moves to Idle Member state.**

> "report received" and "(stop timer," and "clear flag)" — `rfc2236.txt:533`, `rfc2236.txt:534`, `rfc2236.txt:535`

- Strength: description. Class: end-to-end.
- Check idea: send a Delaying Member host a Membership Report for its group from another host, and confirm it cancels its own pending report and becomes an Idle Member.

### RFC2236-HOST-30

**On the query received event, a host in Delaying Member state resets the timer only if the query's Max Response Time is less than the current timer, and stays in Delaying Member state.**

> "query received" and "(reset timer if" and "Max Resp Time" and "< current timer)" — `rfc2236.txt:537`, `rfc2236.txt:538`, `rfc2236.txt:539`, `rfc2236.txt:540`

- Strength: description. Class: end-to-end.
- Check idea: send a Delaying Member host a second query with a smaller Max Response Time than its running timer, and confirm the timer resets to the smaller value.

### RFC2236-HOST-31

**On the timer expired event, a host in Delaying Member state sends a report, sets the flag, and moves to Idle Member state.**

> "timer expired" and "(send report," and "set flag)" — `rfc2236.txt:537`, `rfc2236.txt:538`, `rfc2236.txt:539`

- Strength: description. Class: wire.
- Check idea: let a Delaying Member host's report delay timer run out with no competing report received, and confirm it sends a Membership Report and becomes an Idle Member.

### RFC2236-HOST-32

**The all-systems group, 224.0.0.1, is a special case: a host stays an Idle Member for it on every interface and never sends a report for it.**

> "The all-systems group (address 224.0.0.1) is handled as a special
> case.  The host starts in Idle Member state for that group on every
> interface, never transitions to another state, and never sends a
> report for that group." — §6, `rfc2236.txt:544-547`

- Strength: description. Class: internal.
- Check idea: observe a host's membership for the all-systems group 224.0.0.1: confirm it is Idle Member on every interface and that the host never sends a report for that group.

### RFC2236-HOST-33

**In No IGMPv1 Router Present state, a host has not heard an IGMPv1-style query within the Version 1 Router Present Timeout.**

> ""No IGMPv1 Router Present", when the host has not heard an IGMPv1
> style query for the [Version 1 Router Present Timeout].  This is
> the initial state." — §6, `rfc2236.txt:552-554`

- Strength: description. Class: internal.
- Check idea: observe that a host starts, and reverts, to No IGMPv1 Router Present once no IGMPv1-style query for an interface arrives within the timeout.

### RFC2236-HOST-34

**In IGMPv1 Router Present state, a host has heard an IGMPv1-style query within the Version 1 Router Present Timeout.**

> ""IGMPv1 Router Present", when the host has heard an IGMPv1 style
> query within the [Version 1 Router Present Timeout]." — §6, `rfc2236.txt:556-557`

- Strength: description. Class: internal.
- Check idea: send a host an IGMPv1-style query, and confirm it is in IGMPv1 Router Present state for the following Version 1 Router Present Timeout.

### RFC2236-HOST-35

**The IGMPv1 query received event occurs when a host receives a query with the Max Response Time field set to 0.**

> ""IGMPv1 query received", when the host receives a query with the
> Max Response Time field set to 0." — §6, `rfc2236.txt:569-570`

- Strength: description. Class: wire.
- Check idea: send a host a Membership Query with the Max Response Time field set to 0, and confirm the host treats it as an IGMPv1-style query.

### RFC2236-HOST-36

**The timer expires event occurs when the timer noting the presence of an IGMPv1 router expires.**

> ""timer expires", when the timer set to note the presence of an
> IGMPv1 router expires." — §6, `rfc2236.txt:572-573`

- Strength: description. Class: internal.
- Check idea: let a host's version 1 router present timer run out with no further IGMPv1-style query, and confirm the timer-expires event fires.

### RFC2236-HOST-37

**The set timer action sets the version 1 router present timer to its maximum value and restarts it.**

> ""set timer", setting the timer to its maximum value [Version 1
> Router Present Timeout] and (re)starting it." — §6, `rfc2236.txt:577-578`

- Strength: description. Class: internal.
- Check idea: send a host an IGMPv1-style query, and confirm the version 1 router present timer is set to its maximum value and restarted, even if it was already running.

### RFC2236-HOST-38

**On the IGMPv1 query received event, a host in No IGMPv1 Router Present state sets the timer and moves to IGMPv1 Router Present state.**

> "No IGMPv1" — `rfc2236.txt:583`; "IGMPv1 query" and "(set timer)" — `rfc2236.txt:590`, `rfc2236.txt:592`; "IGMPv1" and "Router" and "Present" — `rfc2236.txt:595-597`

- Strength: description. Class: end-to-end.
- Check idea: send a host in No IGMPv1 Router Present state an IGMPv1-style query, and confirm it starts the version 1 router present timer and becomes IGMPv1 Router Present.

### RFC2236-HOST-39

**On the timer expires event, a host in IGMPv1 Router Present state moves to No IGMPv1 Router Present state.**

> "timer expires" — `rfc2236.txt:590`; "No IGMPv1" — `rfc2236.txt:583`

- Strength: description. Class: internal.
- Check idea: let a host's version 1 router present timer run out with no further IGMPv1-style query, and confirm it moves from IGMPv1 Router Present to No IGMPv1 Router Present.

### RFC2236-HOST-40

**On the IGMPv1 query received event, a host already in IGMPv1 Router Present state resets the timer and stays in IGMPv1 Router Present state.**

> "IGMPv1 query received" and "(set timer)" — `rfc2236.txt:602`, `rfc2236.txt:603`

- Strength: description. Class: end-to-end.
- Check idea: send a host in IGMPv1 Router Present state a further IGMPv1-style query, and confirm the timer restarts and the state does not change.

## Router state diagram

### RFC2236-ROUTER-1

**In Querier state, a router is designated to transmit IGMP Membership Queries on the network.**

> ""Querier", when this router is designated to transmit IGMP
> Membership Queries on this network." — §7, `rfc2236.txt:631-632`

- Strength: description. Class: internal.
- Check idea: observe a router's role on a network and confirm it sends Membership Queries only while it is Querier for that network.

### RFC2236-ROUTER-2

**In Non-Querier state, another router is designated to transmit IGMP Membership Queries on the network.**

> ""Non-Querier", when there is another router designated to transmit
> IGMP membership Queries on this network." — §7, `rfc2236.txt:634-635`

- Strength: description. Class: internal.
- Check idea: observe a router's role on a network with another Querier present, and confirm it does not itself transmit Membership Queries while Non-Querier.

### RFC2236-ROUTER-3

**The query timer expired event occurs when the timer set for query transmission expires.**

> ""query timer expired" occurs when the timer set for query
> transmission expires." — §7, `rfc2236.txt:639-640`

- Strength: description. Class: internal.
- Check idea: let a Querier's general query timer run out, and confirm the query-timer-expired event fires.

### RFC2236-ROUTER-4

**The query received from a router with a lower IP address event occurs when a router hears an IGMP Membership Query from another router on the same network with a lower IP address.**

> ""query received from a router with a lower IP address" occurs when
> an IGMP Membership Query is received from a router on the same
> network with a lower IP address." — §7, `rfc2236.txt:642-644`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Membership Query from another router on the same network whose IP address is lower, and confirm the event fires only then, not for a higher address.

### RFC2236-ROUTER-5

**The other querier present timer expired event occurs when the timer noting the presence of another, lower-IP-address querier expires.**

> ""other querier present timer expired" occurs when the timer set to
> note the presence of another querier with a lower IP address on the
> network expires." — §7, `rfc2236.txt:646-648`

- Strength: description. Class: internal.
- Check idea: let a Non-Querier's other-querier-present timer run out with no further query from the lower-address router, and confirm the event fires.

### RFC2236-ROUTER-6

**The start general query timer action starts the general query timer for the attached network.**

> ""start general query timer" for the attached network." — §7, `rfc2236.txt:653`

- Strength: description. Class: internal.
- Check idea: trigger the start-general-query-timer action, and confirm the general query timer for the network starts running.

### RFC2236-ROUTER-7

**The start other querier present timer action starts that timer with the Other Querier Present Interval.**

> ""start other querier present timer" for the attached network [Other
> Querier Present Interval]." — §7, `rfc2236.txt:655-656`

- Strength: description. Class: internal.
- Check idea: trigger the start-other-querier-present-timer action, and confirm the timer starts and runs for the Other Querier Present Interval.

### RFC2236-ROUTER-8

**The send general query action sends a General Query to the all-systems group, 224.0.0.1, with a Max Response Time of the Query Response Interval.**

> ""send general query" on the attached network.  The General Query is
> sent to the all-systems group (224.0.0.1), and has a Max Response
> Time of [Query Response Interval]." — §7, `rfc2236.txt:658-660`

- Strength: description. Class: wire.
- Check idea: trigger a send-general-query action, and confirm the General Query is addressed to 224.0.0.1 and carries the Query Response Interval as its Max Response Time.

### RFC2236-ROUTER-9

**On entry, a router in Initial state sends a general query, sets the initial general query timer, and moves to Querier state.**

> "Initial" — `rfc2236.txt:682`; "(send gen. q.," and "set initial gen. q." and "timer)" — `rfc2236.txt:683`, `rfc2236.txt:684`, `rfc2236.txt:685`; "Querier" — `rfc2236.txt:685`

- Strength: description. Class: wire.
- Check idea: start a router with no attached-network state, and confirm it sends a general query, starts the general query timer, and is Querier.

### RFC2236-ROUTER-10

**On the query timer expired event, a router in Querier state sends a general query, restarts the general query timer, and stays Querier.**

> "gen. query timer" — `rfc2236.txt:680`; "(send general query," and "set gen. q. timer)" — `rfc2236.txt:682`, `rfc2236.txt:683`

- Strength: description. Class: wire.
- Check idea: let the general query timer expire while a router is Querier, and confirm it sends another general query, restarts the timer, and remains Querier.

### RFC2236-ROUTER-11

**On the query received from a router with a lower IP address event, a router in Querier state sets the other querier present timer and moves to Non-Querier state.**

> "query received from a" and "router with a lower" and "IP address" — `rfc2236.txt:690`, `rfc2236.txt:691`, `rfc2236.txt:692`; "(set other querier" and "present timer)" — `rfc2236.txt:693`, `rfc2236.txt:694`; "Non" — `rfc2236.txt:697`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Membership Query from a router with a lower IP address while it is Querier, and confirm it starts the other querier present timer and becomes Non-Querier.

### RFC2236-ROUTER-12

**On the other querier present timer expired event, a router in Non-Querier state sends a general query, sets the general query timer, and moves to Querier state.**

> "other querier" and "present timer" — `rfc2236.txt:690`, `rfc2236.txt:691`; "(send general" and "query,set gen." and "q. timer)" — `rfc2236.txt:693`, `rfc2236.txt:694`, `rfc2236.txt:695`

- Strength: description. Class: wire.
- Check idea: let the other querier present timer expire while a router is Non-Querier, and confirm it sends a general query and becomes Querier.

### RFC2236-ROUTER-13

**On the query received from a router with a lower IP address event, a router in Non-Querier state resets the other querier present timer and stays Non-Querier.**

> "query received from a" and "router with a lower IP" and "address" — `rfc2236.txt:703`, `rfc2236.txt:704`, `rfc2236.txt:705`; "(set other querier" and "present timer)" — `rfc2236.txt:706`, `rfc2236.txt:707`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Membership Query from a lower-IP-address router while it is Non-Querier, and confirm the timer restarts and the router stays Non-Querier.

### RFC2236-ROUTER-14

**A router should start in the Initial state on every attached network, and move immediately to Querier state.**

> "A router should start in the Initial state on all attached networks,
> and immediately move to Querier state." — §7, `rfc2236.txt:710-711`

- Strength: should (lower case). Class: internal.
- Check idea: start a router on an attached network, and confirm it enters Querier state right away rather than staying in Initial state or waiting.

### RFC2236-ROUTER-15

**In No Members Present state, no host on the network has sent a report for the multicast group.**

> ""No Members Present" state, when there are no hosts on the network
> which have sent reports for this multicast group.  This is the
> initial state for all groups on the router; it requires no storage
> in the router." — §7, `rfc2236.txt:717-720`

- Strength: description. Class: internal.
- Check idea: observe that a router holds no stored state for a multicast group until a host on the network reports for it.

### RFC2236-ROUTER-16

**In Members Present state, a host on the network has sent a Membership Report for the multicast group.**

> ""Members Present" state, when there is a host on the network which
> has sent a Membership Report for this multicast group." — §7, `rfc2236.txt:722-723`

- Strength: description. Class: internal.
- Check idea: send a router a Membership Report for a group, and confirm the router's state for that group becomes Members Present.

### RFC2236-ROUTER-17

**In Version 1 Members Present state, an IGMPv1 host on the network has sent a Version 1 Membership Report for the multicast group.**

> ""Version 1 Members Present" state, when there is an IGMPv1 host on
> the network which has sent a Version 1 Membership Report for this
> multicast group." — §7, `rfc2236.txt:735-737`

- Strength: description. Class: internal.
- Check idea: send a router a Version 1 Membership Report for a group, and confirm the router's state for that group becomes Version 1 Members Present.

### RFC2236-ROUTER-18

**In Checking Membership state, a router has received a Leave Group message but has not yet heard a Membership Report for the group.**

> ""Checking Membership" state, when the router has received a
> Leave Group message but has not yet heard a Membership Report for
> the multicast group." — §7, `rfc2236.txt:739-741`

- Strength: description. Class: internal.
- Check idea: send a router a Leave Group message for a group with members present, and confirm its state becomes Checking Membership until a further report arrives or the timer expires.

### RFC2236-ROUTER-19

**The v2 report received event occurs when a router receives a Version 2 Membership Report for the group on the interface.**

> ""v2 report received" occurs when the router receives a Version 2
> Membership Report for the group on the interface." — §7, `rfc2236.txt:746-747`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 2 Membership Report for a group on an interface, and confirm the v2-report-received event fires for that group and interface only.

### RFC2236-ROUTER-20

**A Version 2 Membership Report is valid only when it is at least 8 octets long and carries a correct IGMP checksum.**

> "To be valid, the
> Report message must be at least 8 octets long and must have a
> correct IGMP checksum." — §7, `rfc2236.txt:747-749`

- Strength: must (lower case). Class: wire.
- Check idea: send a router a Version 2 Membership Report shorter than 8 octets, and separately one with an incorrect checksum, and confirm the router treats neither as valid.

### RFC2236-ROUTER-21

**The v1 report received event occurs, under the same validity rule as a Version 2 report, when a router receives a Version 1 Membership Report for the group on the interface.**

> ""v1 report received" occurs when the router receives a Version 1
> Membership report for the group on the interface.  The same
> validity requirements apply." — §7, `rfc2236.txt:751-753`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 1 Membership Report for a group on an interface, and confirm the v1-report-received event fires only when the message meets the same length and checksum rule as a Version 2 report.

### RFC2236-ROUTER-22

**The leave received event occurs when a router receives an IGMP Group Leave message for the group on the interface.**

> ""leave received" occurs when the router receives an IGMP Group
> Leave message for the group on the interface." — §7, `rfc2236.txt:755-756`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Group Leave message for a group on an interface, and confirm the leave-received event fires for that group and interface only.

### RFC2236-ROUTER-23

**An IGMP Group Leave message is valid only when it is at least 8 octets long and carries a correct IGMP checksum.**

> "To be
> valid, the Leave message must be at least 8 octets long and must have a
> correct IGMP checksum." — §7, `rfc2236.txt:756-758`

- Strength: must (lower case). Class: wire.
- Check idea: send a router a Group Leave message shorter than 8 octets, and separately one with an incorrect checksum, and confirm the router treats neither as valid.

### RFC2236-ROUTER-24

**The timer expired event occurs when the timer set for a group membership expires.**

> ""timer expired" occurs when the timer set for a group membership
> expires." — §7, `rfc2236.txt:760-761`

- Strength: description. Class: internal.
- Check idea: let a router's group membership timer for a group run out, and confirm the timer-expired event fires for that group.

### RFC2236-ROUTER-25

**The retransmit timer expired event occurs when the timer set to retransmit a group-specific Membership Query expires.**

> ""retransmit timer expired" occurs when the timer set to retransmit
> a group-specific Membership Query expires." — §7, `rfc2236.txt:763-764`

- Strength: description. Class: internal.
- Check idea: let a router's retransmit timer for a group-specific query run out with no report received, and confirm the retransmit-timer-expired event fires.

### RFC2236-ROUTER-26

**The v1 host timer expired event occurs when the timer noting the presence of version 1 hosts as group members expires.**

> ""v1 host timer expired" occurs when the timer set to note the
> presence of version 1 hosts as group members expires." — §7, `rfc2236.txt:766-767`

- Strength: description. Class: internal.
- Check idea: let a router's version 1 host timer for a group run out with no further Version 1 report, and confirm the v1-host-timer-expired event fires.

### RFC2236-ROUTER-27

**The start timer action starts the group membership timer at the Group Membership Interval, and also resets an already-running timer to that value.**

> ""start timer" for the group membership on the interface - also
> resets the timer to its initial value [Group Membership Interval]
> if the timer is currently running." — §7, `rfc2236.txt:772-774`

- Strength: description. Class: internal.
- Check idea: trigger the start-timer action for a group with no timer running, and confirm the timer starts at the Group Membership Interval; trigger it again while the timer runs, and confirm it resets to that value.

### RFC2236-ROUTER-28

**When this router is a Querier, the start timer* action sets the timer to the Last Member Query Interval times the Last Member Query Count.**

> ""start timer*" for the group membership on the interface - this
> alternate action sets the timer to [Last Member Query Interval] *
> [Last Member Query Count] if this router is a Querier" — §7, `rfc2236.txt:776-778`

- Strength: description. Class: internal.
- Check idea: trigger the start-timer* action for a group on a router that is Querier, and confirm the timer is set to the Last Member Query Interval multiplied by the Last Member Query Count.

### RFC2236-ROUTER-29

**When this router is a non-Querier, the start timer* action sets the timer to the packet's Max Response Time times the Last Member Query Count.**

> "or the [Max
> Response Time] in the packet * [Last Member Query Count] if this
> router is a non-Querier." — §7, `rfc2236.txt:778-780`

- Strength: description. Class: internal.
- Check idea: trigger the start-timer* action for a group on a router that is Non-Querier, and confirm the timer is set to the received packet's Max Response Time multiplied by the Last Member Query Count.

### RFC2236-ROUTER-30

**The start retransmit timer action starts that timer for the group membership at the Last Member Query Interval.**

> ""start retransmit timer" for the group membership on the interface
> [Last Member Query Interval]." — §7, `rfc2236.txt:791-792`

- Strength: description. Class: internal.
- Check idea: trigger the start-retransmit-timer action for a group, and confirm the retransmit timer runs for the Last Member Query Interval.

### RFC2236-ROUTER-31

**The start v1 host timer action starts the version 1 host timer at the Group Membership Interval, and also resets an already-running timer to that value.**

> ""start v1 host timer" for the group membership on the interface,
> also resets the timer to its initial value [Group Membership
> Interval] if the timer is currently running." — §7, `rfc2236.txt:794-796`

- Strength: description. Class: internal.
- Check idea: trigger the start-v1-host-timer action for a group, and confirm it starts, or resets an already-running timer, at the Group Membership Interval.

### RFC2236-ROUTER-32

**The send group-specific query action sends a Group-Specific Query to the group being queried, with a Max Response Time of the Last Member Query Interval.**

> ""send group-specific query" for the group on the attached network.
> The Group-Specific Query is sent to the group being queried, and
> has a Max Response Time of [Last Member Query Interval]." — §7, `rfc2236.txt:798-800`

- Strength: description. Class: wire.
- Check idea: trigger a send-group-specific-query action for a group, and confirm the query is addressed to that group and carries the Last Member Query Interval as its Max Response Time.

### RFC2236-ROUTER-33

**The notify routing + action tells the routing protocol that the connected network has members of the group.**

> ""notify routing +" notify the routing protocol that there are
> members of this group on this connected network." — §7, `rfc2236.txt:802-803`

- Strength: description. Class: internal.
- Check idea: trigger the notify-routing-+ action for a group, and confirm the routing protocol is told the connected network has members of that group.

### RFC2236-ROUTER-34

**The notify routing - action tells the routing protocol that the connected network no longer has members of the group.**

> ""notify routing -" notify the routing protocol that there are no
> longer any members of this group on this connected network." — §7, `rfc2236.txt:805-806`

- Strength: description. Class: internal.
- Check idea: trigger the notify-routing-- action for a group, and confirm the routing protocol is told the connected network no longer has members of that group.

### RFC2236-ROUTER-35

**On the v2 report received event, a Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, and moves to Members Present state.**

> "No Members" — `rfc2236.txt:851`; "v2 report received" and "(notify routing +," and "start timer)" — `rfc2236.txt:857`, `rfc2236.txt:858`, `rfc2236.txt:859`; "Members Present" — `rfc2236.txt:865`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 2 Membership Report for a group with no known members, and confirm it starts the group timer, tells the routing protocol of members present, and becomes Members Present for that group.

### RFC2236-ROUTER-36

**On the v1 report received event, a Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state.**

> "v1 report rec'd" and "(notify routing +," and "start timer," and "start v1 host" and "timer)" — `rfc2236.txt:854`, `rfc2236.txt:855`, `rfc2236.txt:856`, `rfc2236.txt:857`, `rfc2236.txt:858`; "Version 1" and "Members Present" — `rfc2236.txt:879-880`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 1 Membership Report for a group with no known members, and confirm it starts both the group timer and the version 1 host timer, tells the routing protocol of members present, and becomes Version 1 Members Present for that group.

### RFC2236-ROUTER-37

**On the timer expired event, a Querier router in Members Present state notifies the routing protocol that no members remain and moves to No Members Present state.**

> "timer expired" and "(notify routing -)" — `rfc2236.txt:850`, `rfc2236.txt:851`; "No Members" — `rfc2236.txt:851`

- Strength: description. Class: internal.
- Check idea: let the group membership timer expire while a router is in Members Present state for a group, and confirm it tells the routing protocol no members remain and returns to No Members Present.

### RFC2236-ROUTER-38

**On the v2 report received event, a Querier router already in Members Present state starts the timer and stays in Members Present state.**

> "v2 report rec'd" and "(start timer)" — `rfc2236.txt:872`, `rfc2236.txt:873`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 2 Membership Report for a group already in Members Present state, and confirm the group timer restarts and the state does not change.

### RFC2236-ROUTER-39

**On the leave received event, a Querier router in Members Present state starts the timer with the leave-triggered value, starts the retransmit timer, sends a group-specific query, and moves to Checking Membership state.**

> "leave received" and "(start timer*," and "start rexmt timer," and "send g-s query)" — `rfc2236.txt:866`, `rfc2236.txt:867`, `rfc2236.txt:868`, `rfc2236.txt:869`; "Checking" and "Membership" — `rfc2236.txt:865-866`

- Strength: description. Class: wire.
- Check idea: send a router a Group Leave message for a group in Members Present state, and confirm it sends a group-specific query and moves to Checking Membership.

### RFC2236-ROUTER-40

**On the v1 report received event, a Querier router in Members Present state starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state.**

> "v1 report rec'd" and "(start timer," and "start v1 host timer)" — `rfc2236.txt:873`, `rfc2236.txt:874`, `rfc2236.txt:875`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 1 Membership Report for a group in Members Present state, and confirm it starts the version 1 host timer and moves to Version 1 Members Present.

### RFC2236-ROUTER-41

**On the v2 report received event, a Querier router in Checking Membership state starts the timer and moves to Members Present state.**

> "v2 report received" and "(start timer)" — `rfc2236.txt:863`, `rfc2236.txt:864`; "Checking" — `rfc2236.txt:865`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 2 Membership Report for a group in Checking Membership state, and confirm the timer restarts and the state returns to Members Present.

### RFC2236-ROUTER-42

**On the v1 report received event, a Querier router in Checking Membership state starts the timer, starts the version 1 host timer, and moves to Version 1 Members Present state.**

> "v1 report rec'd" and "(start timer," and "start v1 host" and "timer)" — `rfc2236.txt:873`, `rfc2236.txt:874`, `rfc2236.txt:875`, `rfc2236.txt:876`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 1 Membership Report for a group in Checking Membership state, and confirm it starts the version 1 host timer and moves to Version 1 Members Present.

### RFC2236-ROUTER-43

**On the retransmit timer expired event, a Querier router in Checking Membership state sends a group-specific query, restarts the retransmit timer, and stays in Checking Membership state.**

> "rexmt timer" and "(send g-s" and "query," and "st rxmt" and "tmr)" — `rfc2236.txt:855`, `rfc2236.txt:857`, `rfc2236.txt:858`, `rfc2236.txt:859`, `rfc2236.txt:860`; "Checking" and "Membership" — `rfc2236.txt:865-866`

- Strength: description. Class: wire.
- Check idea: let the retransmit timer expire while a router is in Checking Membership state for a group, and confirm it sends another group-specific query and remains in Checking Membership.

### RFC2236-ROUTER-44

**On the timer expired event, a Querier router in Checking Membership state notifies the routing protocol that no members remain, clears the retransmit timer, and moves to No Members Present state.**

> "timer expired" and "(notify routing -," and "clear rxmt tmr)" — `rfc2236.txt:849`, `rfc2236.txt:850`, `rfc2236.txt:851`

- Strength: description. Class: internal.
- Check idea: let the group membership timer expire while a router is in Checking Membership state for a group, and confirm it stops the retransmit timer, tells the routing protocol no members remain, and returns to No Members Present.

### RFC2236-ROUTER-45

**On the v2 report received event, a Querier router already in Version 1 Members Present state starts the timer and stays in Version 1 Members Present state.**

> "v2 report rec'd" and "(start timer)" — `rfc2236.txt:884`, `rfc2236.txt:885`; "Version 1" — `rfc2236.txt:879`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Version 2 Membership Report for a group already in Version 1 Members Present state, and confirm the timer restarts and the state does not change.

### RFC2236-ROUTER-46

**On the v1 host timer expired event, a Querier router in Version 1 Members Present state moves to Members Present state.**

> "v1 host" and "tmr" and "exp'd" — `rfc2236.txt:875`, `rfc2236.txt:876`, `rfc2236.txt:877`; "Version 1" — `rfc2236.txt:879`; "Members Present" — `rfc2236.txt:865`

- Strength: description. Class: internal.
- Check idea: let the version 1 host timer expire while a router is in Version 1 Members Present state for a group, and confirm it moves to Members Present without the group membership timer being disturbed.

### RFC2236-ROUTER-47

**On the timer expired event, a Querier router in Version 1 Members Present state notifies the routing protocol that no members remain and moves to No Members Present state.**

> "timer expired" — `rfc2236.txt:879`; "(notify routing -)" — `rfc2236.txt:880`; "Version 1" — `rfc2236.txt:879`

- Strength: description. Class: internal.
- Check idea: let the group membership timer expire while a router is in Version 1 Members Present state for a group, and confirm it tells the routing protocol no members remain and returns to No Members Present.

### RFC2236-ROUTER-48

**Non-Queriers send no messages and act only when a message arrives; a non-Querier does not distinguish a Version 1 from a Version 2 Membership Report.**

> "but non-Queriers do not send any messages and are only driven by
> message reception." and "Note that non-Queriers do not care whether a
> Membership Report message is Version 1 or Version 2." — §7, `rfc2236.txt:904-906`

- Strength: description. Class: internal.
- Check idea: observe a Non-Querier router: confirm it sends no Membership Query or notification message, and confirm it reacts the same way to a Version 1 and a Version 2 Membership Report for a group.

### RFC2236-ROUTER-49

**On the report received event, a Non-Querier router in No Members Present state notifies the routing protocol of members present, starts the timer, and moves to Members Present state.**

> "report received" and "(notify routing +," and "start timer)" — `rfc2236.txt:919`, `rfc2236.txt:920`, `rfc2236.txt:921`

- Strength: description. Class: end-to-end.
- Check idea: send a Non-Querier router a Membership Report for a group with no known members, and confirm it starts the group timer and tells the routing protocol of members present.

### RFC2236-ROUTER-50

**On the timer expired event, a Non-Querier router in Members Present state notifies the routing protocol that no members remain and moves to No Members Present state.**

> "timer expired" and "(notify routing -)" — `rfc2236.txt:911`, `rfc2236.txt:912`; "No Members" — `rfc2236.txt:912`

- Strength: description. Class: internal.
- Check idea: let the group membership timer expire while a Non-Querier router is in Members Present state for a group, and confirm it tells the routing protocol no members remain and returns to No Members Present.

### RFC2236-ROUTER-51

**On the report received event, a Non-Querier router already in Members Present state starts the timer and stays in Members Present state.**

> "report received" and "(start timer)" — `rfc2236.txt:931`, `rfc2236.txt:932`

- Strength: description. Class: end-to-end.
- Check idea: send a Non-Querier router a Membership Report for a group already in Members Present state, and confirm the group timer restarts and the state does not change.

### RFC2236-ROUTER-52

**On the g-s query received event, a Non-Querier router in Members Present state starts the timer with the query-triggered value and moves to Checking Membership state.**

> "g-s query rec'd" and "(start timer*)" — `rfc2236.txt:927`, `rfc2236.txt:928`

- Strength: description. Class: end-to-end.
- Check idea: send a Non-Querier router a Group-Specific Query for a group in Members Present state, and confirm it moves to Checking Membership state.

### RFC2236-ROUTER-53

**On the report received event, a Non-Querier router in Checking Membership state starts the timer and moves to Members Present state.**

> "report received" and "(start timer)" — `rfc2236.txt:924`, `rfc2236.txt:925`

- Strength: description. Class: end-to-end.
- Check idea: send a Non-Querier router a Membership Report for a group in Checking Membership state, and confirm the timer restarts and the state returns to Members Present.

### RFC2236-ROUTER-54

**On the timer expired event, a Non-Querier router in Checking Membership state notifies the routing protocol that no members remain and moves to No Members Present state.**

> "timer expired" and "(notify routing -)" — `rfc2236.txt:911`, `rfc2236.txt:912`

- Strength: description. Class: internal.
- Check idea: let the group membership timer expire while a Non-Querier router is in Checking Membership state for a group, and confirm it tells the routing protocol no members remain and returns to No Members Present.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§6 and §7. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 to §5 (definitions, message format, protocol
  description, compatibility with IGMPv1) and §8 to §15 (timers, message destinations,
  security, references, the changes from IGMPv1).

Within the in-scope sections, these parts have no entry, with the reason:

In §6 and §7:

- `rfc2236.txt:372-376` — lead-in sentence introducing the host state diagram and the count of states; no checkable content of its own.
- `rfc2236.txt:394-397`, `450-453`, `506-509`, `562-565`, `618-621`, `674-677`, `730-733`, `786-789`, `842-845`, `898-901` — RFC page footers and headers ("Fenner Standards Track [Page N]" / "RFC 2236 Internet Group Management Protocol November 1997") that fall inside the range; formatting boilerplate, not protocol text.
- `rfc2236.txt:436-438` — lead-in sentence counting the seven host actions; no checkable content of its own.
- `rfc2236.txt:479-483` — note explaining how to read the arc labels of the state diagrams (event triggers the transition even if the action is conditional); a reading instruction, not a checkable statement about host or router behavior.
- `rfc2236.txt:549-551` — lead-in sentence introducing the second host state pair; no checkable content of its own.
- `rfc2236.txt:567-568` — lead-in sentence counting the two events of the second host state pair; no checkable content of its own.
- `rfc2236.txt:575-576` — lead-in sentence introducing the single action of the second host state pair; no checkable content of its own.
- `rfc2236.txt:625-627` — lead-in sentence introducing the router state diagrams; no checkable content of its own.
- `rfc2236.txt:628-630` — lead-in sentence introducing the Querier/Non-Querier state pair; no checkable content of its own (the two states themselves are RFC2236-ROUTER-1 and RFC2236-ROUTER-2).
- `rfc2236.txt:637-638` — lead-in sentence counting the three Querier/Non-Querier events; no checkable content of its own.
- `rfc2236.txt:650-652` — lead-in sentence counting the three Querier/Non-Querier actions; no checkable content of its own.
- `rfc2236.txt:713-716` — lead-in sentence introducing the four group-membership states; no checkable content of its own (the four states themselves are RFC2236-ROUTER-15 through RFC2236-ROUTER-18).
- `rfc2236.txt:743-745` — lead-in sentence counting the six group-membership events; no checkable content of its own.
- `rfc2236.txt:769-771` — lead-in sentence counting the six group-membership actions; no checkable content of its own.
- `rfc2236.txt:808` — sentence announcing that the Querier state diagram follows; no checkable content of its own.
- `rfc2236.txt:903-904` — clause "The state diagram for a router in Non-Querier state is similar" is a framing remark pointing at the diagram figure itself (already cataloged transition by transition as RFC2236-ROUTER-49 through RFC2236-ROUTER-54); the rest of the sentence (non-Queriers send no messages) is cataloged as RFC2236-ROUTER-48.
- `rfc2236.txt:935` (§8, "List of timers and default values") and beyond — outside the assigned line range (370-934).
