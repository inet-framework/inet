# RFC 2710 (Multicast Listener Discovery (MLD) for IPv6) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2710-*` · **Stands on:** [standards.md](../../protocol/mld/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 2710, Multicast Listener Discovery (MLD) for IPv6, of
October 1999, a document of the in-scope set of MLD. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc2710.txt`](../../../../../../standards/RFC/rfc2710.txt) —
  Multicast Listener Discovery (MLD) for IPv6, October 1999. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc2710.txt>.

MLDv2 itself is in [`rfc9777/catalog.md`](../rfc9777/catalog.md); this document gives the
version 1 behavior that the interoperation rules of RFC 9777 §8 fall back to. The in-scope
sections are §5 to §7.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mld/standards.md). The features these statements build are in
[`features.md`](../../protocol/mld/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mld/coverage.md`](../../model/mld/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc2710.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC2710-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 2710 uses the keywords of RFC 2119 in capitals;
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

<!-- index -->

## Node state transition diagram

### RFC2710-NODE-1

**A node begins in Non-Listener state for a multicast address on an interface, and this state needs no storage.**

> "Non-Listener" and "state, when the node is not listening to the address on the interface (i.e., no upper-layer protocol or application has requested reception of packets to that multicast address).  This is the initial state for all multicast addresses on all interfaces; it requires no storage in the node." — §5, `rfc2710.txt:383-387`

- Strength: description. Class: internal.
- Check idea: before an application asks to receive a multicast address, a node holds no listener state for that address on the interface.

### RFC2710-NODE-2

**A node accepts a Query only from a link-local address, at least 24 octets long, with a correct checksum.**

> "To be valid, the Query message MUST come from a link-
> local IPv6 Source Address, be at least 24 octets long, and have a
> correct MLD checksum." — §5, `rfc2710.txt:420-422`

- Strength: must. Class: end-to-end.
- Check idea: send the node a Query from a non-link-local address, or shorter than 24 octets, or with a wrong checksum, and confirm the node does not act on it.

### RFC2710-NODE-3

**The Multicast Address field of a Query is zero for a General Query, or a multicast address for a Multicast-Address-Specific Query.**

> "The Multicast Address field in the MLD
> message must contain either zero (a General Query) or a valid
> multicast address (a Multicast- Address-Specific Query)." — §5, `rfc2710.txt:422-424`

- Strength: must (lower case). Class: wire.
- Check idea: a General Query on the wire carries a zero Multicast Address field; a Multicast-Address-Specific Query carries the queried address in that field.

### RFC2710-NODE-4

**A General Query applies to every address on the interface; a Multicast-Address-Specific Query applies to one address.**

> "A
> General Query applies to all multicast addresses on the interface
> from which the Query is received.  A Multicast-Address-Specific
> Query applies to a single multicast address on the interface from
> which the Query is received." — §5, `rfc2710.txt:424-428`

- Strength: description. Class: end-to-end.
- Check idea: a General Query makes a node act for every address it listens to on that interface; a Multicast-Address-Specific Query makes it act only for the named address.

### RFC2710-NODE-5

**A node ignores a Query for an address it is not listening to.**

> "Queries are ignored for addresses in
> the Non-Listener state." — §5, `rfc2710.txt:428-429`

- Strength: description. Class: end-to-end.
- Check idea: send a Query for an address the node does not listen to, and confirm the node sends no Report for it.

### RFC2710-NODE-6

**A node accepts a Report only from a link-local address, at least 24 octets long, with a correct checksum.**

> "To be valid, the Report message MUST come from a link-
> local IPv6 Source Address, be at least 24 octets long, and have a
> correct MLD checksum." — §5, `rfc2710.txt:432-434`

- Strength: must. Class: end-to-end.
- Check idea: send the node a Report from a non-link-local address, or shorter than 24 octets, or with a wrong checksum, and confirm the node does not act on it.

### RFC2710-NODE-7

**A Report applies only to the address named in its Multicast Address field, on the interface it arrives on.**

> "A Report applies only to the address
> identified in the Multicast Address field of the Report, on the
> interface from which the Report is received." — §5, `rfc2710.txt:434-436`

- Strength: description. Class: end-to-end.
- Check idea: a Report for one address does not change a node's timer for a different address.

### RFC2710-NODE-8

**A node ignores a Report while it is in Non-Listener or Idle Listener state for that address.**

> "It is ignored in the
> Non-Listener or Idle Listener state." — §5, `rfc2710.txt:436-437`

- Strength: description. Class: end-to-end.
- Check idea: send a Report for an address while the node is a non-listener, or while it is an idle listener with no timer running, and confirm the node's state does not change.

### RFC2710-NODE-9

**A node ignores every other event, such as an invalid MLD message or a message type other than Query or Report.**

> "All other events, such as receiving invalid MLD messages or MLD
> message types other than Query or Report, are ignored in all states." — §5, `rfc2710.txt:455-456`

- Strength: description. Class: end-to-end.
- Check idea: send the node an invalid MLD message or an unrelated MLD message type, and confirm no listener state changes.

### RFC2710-NODE-10

**The send-report action sends the Report message to the address being reported.**

> "The Report message
> is sent to the address being reported." — §5, `rfc2710.txt:461-462`

- Strength: description. Class: wire.
- Check idea: every Report a node sends carries a destination address equal to the multicast address in its own Multicast Address field.

### RFC2710-NODE-11

**A node may skip sending Done if it was not the last node to report for the address.**

> "If the flag saying
> we were the last node to report is cleared, this action MAY be
> skipped." — §5, `rfc2710.txt:464-466`

- Strength: may. Class: wire.
- Check idea: a node that has heard another node report for the address after its own last report stops listening without sending a Done.

### RFC2710-NODE-12

**The send-done action sends the Done message to the link-scope all-routers address.**

> "The Done message is sent to the link-scope all-routers
> address (FF02::2)." — §5, `rfc2710.txt:466-467`

- Strength: description. Class: wire.
- Check idea: a Done message that a node sends carries FF02::2 as its destination address.

### RFC2710-NODE-13

**On a Query, a node starts its report timer to a value chosen at random between zero and the Query's Maximum Response Delay.**

> "using a delay value
> chosen uniformly from the interval [0, Maximum Response Delay],
> where Maximum Response Delay is specified in the Query." — §5, `rfc2710.txt:475-478`

- Strength: description. Class: end-to-end.
- Check idea: after a node receives a Query with a given Maximum Response Delay, its Report appears on the link no later than that delay after receipt.

### RFC2710-NODE-14

**For an unsolicited report, a node sets its timer to a value chosen at random within the Unsolicited Report Interval.**

> "If this
> is an unsolicited Report, the timer is set to a delay value chosen
> uniformly from the interval [0, [Unsolicited Report Interval] ]." — §5, `rfc2710.txt:477-479`

- Strength: description. Class: end-to-end.
- Check idea: when a node starts listening without a prior Query, its repeated unsolicited Report appears within the Unsolicited Report Interval of the first one.

### RFC2710-NODE-15

**The reset-timer action sets the timer to a new value chosen the same way as the start-timer action.**

> "for the address on the interface to a new value,
> using a delay value chosen uniformly from the interval [0, Maximum
> Response Delay], as described in" — §5, `rfc2710.txt:481-483`

- Strength: description. Class: internal.
- Check idea: when a node resets its report timer for a new Query, the new timer value lies in the same [0, Maximum Response Delay] range as a freshly started timer.

### RFC2710-NODE-16

**A state transition always happens on its event, even when the event's action is conditional.**

> "the transition is always triggered by the event; even if the
> action is conditional, the transition still occurs." — §5, `rfc2710.txt:490-491`

- Strength: description. Class: internal.
- Check idea: a stop-listening event that finds the last-reporter flag cleared still moves the node to Non-Listener state, even though it sends no Done.

### RFC2710-NODE-17

**A node that starts listening to an address sends a Report, sets the last-reporter flag, starts its timer, and enters Delaying Listener state.**

> "start listening" and "(send report," and "set flag," and "start timer)" — §5, `rfc2710.txt:522-525`

- Strength: description. Class: wire.
- Check idea: an application request to receive a multicast address makes the node send an immediate Report for that address.

### RFC2710-NODE-18

**A Delaying Listener that stops listening stops its timer, sends Done if the last-reporter flag is set, and enters Non-Listener state.**

> "stop listening" and "(stop timer," and "send done if" and "flag set)" — §5, `rfc2710.txt:522-525`

- Strength: description. Class: wire.
- Check idea: an application request to stop receiving a multicast address, made while the node's report timer is still running, stops the timer and, if the node was the last reporter, sends a Done.

### RFC2710-NODE-19

**An Idle Listener that stops listening sends Done if the last-reporter flag is set, and enters Non-Listener state.**

> "stop listening" and "(send done if" and "flag set)" — §5, `rfc2710.txt:522-524`

- Strength: description. Class: wire.
- Check idea: an application request to stop receiving a multicast address, made while the node holds no report timer for it, sends a Done only if the node was the last reporter.

### RFC2710-NODE-20

**An Idle Listener that receives a Query starts its timer and enters Delaying Listener state.**

> "query received" and "(start timer)" — §5, `rfc2710.txt:529-531`

- Strength: description. Class: internal.
- Check idea: a Query for an address the node holds with no timer running starts the node's report timer for that address.

### RFC2710-NODE-21

**A Delaying Listener that receives a Report stops its timer, clears the last-reporter flag, and enters Idle Listener state.**

> "report received" and "(stop timer," and "clear flag)" — §5, `rfc2710.txt:533-536`

- Strength: description. Class: internal.
- Check idea: a Report from another node for an address whose timer is running stops that timer and clears the node's own last-reporter flag.

### RFC2710-NODE-22

**A Delaying Listener whose timer expires sends a Report, sets the last-reporter flag, and enters Idle Listener state.**

> "timer expired" and "(send report," and "set flag)" — §5, `rfc2710.txt:537-539`

- Strength: description. Class: wire.
- Check idea: a node's report timer that runs out without an intervening Report from another node produces a Report from this node.

### RFC2710-NODE-23

**A Delaying Listener that receives a Query resets its timer only if the Query's Maximum Response Delay is less than the current timer value.**

> "query received" and "(reset timer if" and "Max Resp Delay" and "< current timer)" — §5, `rfc2710.txt:537-540`

- Strength: description. Class: internal.
- Check idea: a second Query received while the report timer is running lowers the timer only when the new Query's Maximum Response Delay is smaller than the time left.

### RFC2710-NODE-24

**The link-scope all-nodes address stays in Idle Listener state forever, and a node never reports or leaves it.**

> "The node starts in Idle Listener state for that address on
> every interface, never transitions to another state, and never sends
> a Report or Done for that address." — §5, `rfc2710.txt:544-546`

- Strength: description. Class: wire.
- Check idea: a node never sends a Report or a Done for FF02::1, even on a Query for it or on interface startup or shutdown.

### RFC2710-NODE-25

**A node never sends MLD messages for a reserved-scope or node-local-scope multicast address.**

> "MLD messages are never sent for multicast addresses whose scope is 0
> (reserved) or 1 (node-local)." — §5, `rfc2710.txt:548-549`

- Strength: description. Class: wire.
- Check idea: a node given a reserved-scope or node-local-scope address to listen to sends no Report or Done for it.

### RFC2710-NODE-26

**A node sends MLD messages for link-local-scope addresses, including Solicited-Node addresses, except for the all-nodes address.**

> "MLD messages ARE sent for multicast addresses whose scope is 2
> (link-local), including Solicited-Node multicast addresses [ADDR-
> ARCH], except for the link-scope, all-nodes address (FF02::1)." — §5, `rfc2710.txt:551-553`

- Strength: description. Class: wire.
- Check idea: a node listening to a link-local address other than FF02::1, including a Solicited-Node address, sends a Report for it.

## Router state transition diagram

### RFC2710-ROUTER-1

**A router is a Querier when it is designated to send MLD Queries on a link, and a Non-Querier otherwise.**

> "Querier" and "when this router is designated to transmit MLD Queries
> on this link." and "Non-Querier" and "when there is another router designated to transmit
> MLD Queries on this link." — §6, `rfc2710.txt:575-579`

- Strength: description. Class: internal.
- Check idea: on a link with one router, that router sends the periodic Queries; on a link with two routers, only the one with the lower source address does.

### RFC2710-ROUTER-2

**A router accepts a Query from a lower-addressed router only from a link-local address, at least 24 octets long, with a correct checksum.**

> "a valid MLD Query is received from a router on the same link with
> a lower IPv6 Source Address. To be valid, the Query message MUST
> come from a link-local IPv6 Source Address, be at least 24 octets
> long, and have a correct MLD checksum." — §6, `rfc2710.txt:588-591`

- Strength: must. Class: end-to-end.
- Check idea: a Query from a lower-addressed router that is not link-local, or too short, or has a wrong checksum, does not make a Querier step down.

### RFC2710-ROUTER-3

**The start-general-query-timer action sets the timer to the Query Interval.**

> "for the attached link to [Query
> Interval]." — §6, `rfc2710.txt:601-602`

- Strength: description. Class: internal.
- Check idea: the time between two consecutive periodic General Queries that a Querier sends equals the Query Interval.

### RFC2710-ROUTER-4

**The start-other-querier-present-timer action sets the timer to the Other Querier Present Interval.**

> "for the attached link to [Other
> Querier Present Interval]." — §6, `rfc2710.txt:604-605`

- Strength: description. Class: internal.
- Check idea: a Non-Querier that stops hearing Queries from the current Querier resumes querying after the Other Querier Present Interval.

### RFC2710-ROUTER-5

**The send-general-query action sends the General Query to the all-nodes address with the Query Response Interval as Maximum Response Delay.**

> "The General Query is
> sent to the link-scope all-nodes address (FF02::1), and has a
> Maximum Response Delay of [Query Response Interval]." — §6, `rfc2710.txt:607-609`

- Strength: description. Class: wire.
- Check idea: a General Query that a Querier sends carries destination address FF02::1 and a Maximum Response Delay equal to the Query Response Interval.

### RFC2710-ROUTER-6

**A router starts in Initial state on every attached link and immediately becomes a Querier, sending a General Query and starting its query timer.**

> "A router starts in the Initial state on all attached links, and
> immediately transitions to Querier state." and "(send gen. q.," and "start initial gen. q." and "timer)" — §6, `rfc2710.txt:654-655`, `rfc2710.txt:626-629`

- Strength: description. Class: wire.
- Check idea: a router that attaches to a link sends a General Query immediately, without waiting for the Query Interval to pass.

### RFC2710-ROUTER-7

**A Querier whose general query timer expires sends a new General Query and restarts the timer.**

> "gen. query timer" and "expired" and "(send general query," and "start gen. q. timer)" — §6, `rfc2710.txt:624-627`

- Strength: description. Class: wire.
- Check idea: a Querier sends a new General Query each time its query timer runs out, and keeps sending them at that interval.

### RFC2710-ROUTER-8

**A Querier that receives a Query from a lower-addressed router starts its other-querier-present timer and becomes a Non-Querier.**

> "query received from a" and "router with a lower" and "IP address" and "(start other querier" and "present timer)" — §6, `rfc2710.txt:634-638`

- Strength: description. Class: internal.
- Check idea: a router acting as Querier that receives a valid Query from a router with a lower source address stops sending its own Queries.

### RFC2710-ROUTER-9

**A Non-Querier whose other-querier-present timer expires sends a General Query, starts its query timer, and becomes the Querier.**

> "other querier" and "present timer" and "expired" and "(send gen. query," and "start gen. q. timer)" — §6, `rfc2710.txt:634-638`

- Strength: description. Class: wire.
- Check idea: a Non-Querier that stops hearing Queries from the router with the lower address takes over as Querier after the other-querier-present timer runs out.

### RFC2710-ROUTER-10

**A Non-Querier that receives a Query from a lower-addressed router restarts its other-querier-present timer and stays a Non-Querier.**

> "query received from a" and "router with a lower IP" and "address" and "(start other querier" and "present timer)" — §6, `rfc2710.txt:647-651`

- Strength: description. Class: internal.
- Check idea: repeated Queries from the router with the lower address keep a Non-Querier from ever taking over as Querier.

### RFC2710-ROUTER-11

**No Listeners Present is the initial, storage-free state of a router for a multicast address, meaning no node on the link has reported it.**

> "No Listeners Present" and "state, when there are no nodes on the link
> that have sent a Report for this multicast address.  This is the
> initial state for all multicast addresses on the router; it
> requires no storage in the router." — §6, `rfc2710.txt:662-665`

- Strength: description. Class: internal.
- Check idea: before any node on a link reports an address, a router holds no per-address state for it.

### RFC2710-ROUTER-12

**Checking Listeners is the router state after a Done message for an address, before a Report for it arrives.**

> "Checking Listeners" and "state, when the router has received a Done
> message but has not yet heard a Report for the identified address." — §6, `rfc2710.txt:679-680`

- Strength: description. Class: internal.
- Check idea: after a router receives a Done for an address and before any Report for it arrives, the router treats the address as still possibly having listeners.

### RFC2710-ROUTER-13

**A router accepts a per-group Report only from a link-local address, at least 24 octets long, with a correct checksum.**

> "To be valid, the Report message MUST come
> from a link-local IPv6 Source Address, be at least 24 octets long,
> and have a correct MLD checksum." — §6, `rfc2710.txt:686-688`

- Strength: must. Class: end-to-end.
- Check idea: a Report that is not link-local, or too short, or has a wrong checksum, does not move a router out of No Listeners Present.

### RFC2710-ROUTER-14

**A router accepts a Done message only from a link-local address, at least 24 octets long, with a correct checksum.**

> "To be valid, the Done message MUST
> come from a link-local IPv6 Source Address, be at least 24 octets
> long, and have a correct MLD checksum." — §6, `rfc2710.txt:691-693`

- Strength: must. Class: end-to-end.
- Check idea: a Done message that is not link-local, or too short, or has a wrong checksum, does not move a router out of Listeners Present.

### RFC2710-ROUTER-15

**A Done message matters only to a router in Listeners Present state that is the Querier for the link.**

> "This event is significant only in the" and "Listerners Present" and "state and when the router is a Querier." — §6, `rfc2710.txt:693-695`

- Strength: description. Class: internal.
- Check idea: a Done message sent to a router that is a Non-Querier, or that has no listener recorded for the address, changes nothing.

### RFC2710-ROUTER-16

**A router accepts a Multicast-Address-Specific Query only from a link-local address, at least 24 octets long, with a correct checksum.**

> "To be valid, the Query message MUST come from a link-
> local IPv6 Source Address, be at least 24 octets long, and have a
> correct MLD checksum." — §6, `rfc2710.txt:699-701`

- Strength: must. Class: end-to-end.
- Check idea: a Multicast-Address-Specific Query that is not link-local, or too short, or has a wrong checksum, does not move a router out of Listeners Present.

### RFC2710-ROUTER-17

**A Multicast-Address-Specific Query matters only to a router in Listeners Present state that is a Non-Querier for the link.**

> "This event is significant only in the" and "Listeners Present" and "state and when the router is a Non-Querier." — §6, `rfc2710.txt:701-702`

- Strength: description. Class: internal.
- Check idea: a Multicast-Address-Specific Query received by a router acting as Querier for the link changes nothing in that router's own listener state.

### RFC2710-ROUTER-18

**The start-timer action for a router sets, or if already running resets, the address's timer to the Multicast Listener Interval.**

> "also resets the timer
> to its initial value [Multicast Listener Interval] if the timer is
> currently running." — §6, `rfc2710.txt:715-717`

- Strength: description. Class: internal.
- Check idea: a fresh Report for an address that already has a running timer restarts that timer at the full Multicast Listener Interval.

### RFC2710-ROUTER-19

**The start-timer* action sets the timer to the lower of its current value and a Last-Listener-based value that depends on whether the router is a Querier.**

> "this alternate action
> sets the timer to the minimum of its current value and either
> [Last Listener Query Interval] * [Last Listener Query Count] if
> this router is a Querier, or the Maximum Response Delay in the
> Query message * [Last Listener Query Count] if this router is a
> non-Querier." — §6, `rfc2710.txt:719-724`

- Strength: description. Class: internal.
- Check idea: on a Done message, a Querier lowers the address's timer to at most the Last Listener Query Interval times the Last Listener Query Count; on a Multicast-Address-Specific Query, a Non-Querier lowers it to at most the Query's Maximum Response Delay times that same count.

### RFC2710-ROUTER-20

**The start-retransmit-timer action sets the retransmit timer to the Last Listener Query Interval.**

> "for the address on the link [Last Listener
> Query Interval]." — §6, `rfc2710.txt:735-736`

- Strength: description. Class: internal.
- Check idea: the time between two consecutive Multicast-Address-Specific Queries that a Querier sends for the same address equals the Last Listener Query Interval.

### RFC2710-ROUTER-21

**The send-multicast-address-specific-query action sends the query to the address being queried, with the Last Listener Query Interval as Maximum Response Delay.**

> "The Multicast-Address-Specific Query is sent to the address
> being queried, and has a Maximum Response Delay of [Last Listener
> Query Interval]." — §6, `rfc2710.txt:741-743`

- Strength: description. Class: wire.
- Check idea: a Multicast-Address-Specific Query that a Querier sends carries the queried address as destination and a Maximum Response Delay equal to the Last Listener Query Interval.

### RFC2710-ROUTER-22

**The notify-routing-plus action tells the multicast routing component that the address now has listeners on the link.**

> "internally notify the multicast routing protocol
> that there are listeners to this address on this link." — §6, `rfc2710.txt:745-746`

- Strength: description. Class: internal.
- Check idea: the first Report for an address on a link makes the router's multicast routing component learn that the address has a listener there.

### RFC2710-ROUTER-23

**The notify-routing-minus action tells the multicast routing component that the address no longer has listeners on the link.**

> "internally notify the multicast routing protocol
> that there are no longer any listeners to this address on this
> link." — §6, `rfc2710.txt:748-750`

- Strength: description. Class: internal.
- Check idea: the router's multicast routing component learns that an address has no listener on a link once that address's per-group timer runs out.

### RFC2710-ROUTER-24

**A group in No Listeners Present or Listeners Present state switches to the diagram of the router's new Querier or Non-Querier role at the moment that role changes.**

> "All groups on that link in" and "states switch state transition diagrams when the Querier/Non-Querier state transition occurs." — §6, `rfc2710.txt:755-758`

- Strength: description. Class: internal.
- Check idea: a router that becomes Non-Querier while a group is in Listeners Present state starts using the Non-Querier actions for that group from that moment on.

### RFC2710-ROUTER-25

**A group in Checking Listeners state keeps using its diagram until it leaves Checking Listeners, regardless of a Querier/Non-Querier role change in between.**

> "However, any groups in" and "state continue" and "with the same state transition diagram until the" and "state is exited." — §6, `rfc2710.txt:758-760`

- Strength: description. Class: internal.
- Check idea: a router that receives a Done for a group and then becomes Non-Querier before hearing a Report keeps sending Multicast-Address-Specific Queries for that group as a Querier would, until a Report arrives or its timer runs out.

### RFC2710-ROUTER-26

**A Querier in No Listeners Present state that receives a Report notifies the routing component, starts the timer, and enters Listeners Present state.**

> "report received" and "(notify routing +," and "start timer)" — §6, `rfc2710.txt:803-805`

- Strength: description. Class: internal.
- Check idea: the first Report for an address on a link, received by the Querier, makes it start tracking that address as having a listener.

### RFC2710-ROUTER-27

**A Querier in Listeners Present state whose timer expires notifies the routing component and enters No Listeners Present state.**

> "timer expired" and "(notify routing -)" — §6, `rfc2710.txt:796-797`

- Strength: description. Class: internal.
- Check idea: a Querier that hears no Report for an address before that address's timer runs out stops tracking it as having a listener.

### RFC2710-ROUTER-28

**A Querier in Checking Listeners state whose timer expires notifies the routing component, clears the retransmit timer, and enters No Listeners Present state.**

> "timer expired" and "(notify routing -," and "clear rxmt tmr)" — §6, `rfc2710.txt:795-797`

- Strength: description. Class: internal.
- Check idea: a Querier that sent Multicast-Address-Specific Queries after a Done and heard no Report before the address's timer runs out stops tracking it as having a listener.

### RFC2710-ROUTER-29

**A Querier in Checking Listeners state whose retransmit timer expires sends another Multicast-Address-Specific Query and restarts the retransmit timer.**

> "rexmt timer" and "expired" and "(send m-a-s" and "query," and "st rxmt" and "tmr)" — §6, `rfc2710.txt:802-807`

- Strength: description. Class: wire.
- Check idea: after a Done message and before a Report or the timer running out, a Querier sends repeated Multicast-Address-Specific Queries spaced by the retransmit interval.

### RFC2710-ROUTER-30

**A Querier in Checking Listeners state that receives a Report starts the timer, clears the retransmit timer, and enters Listeners Present state.**

> "report received" and "(start timer," and "clear rxmt tmr)" — §6, `rfc2710.txt:809-811`

- Strength: description. Class: internal.
- Check idea: a Report that arrives at a Querier after a Done for the same address, but before the address's timer runs out, stops the Querier's Multicast-Address-Specific Queries for it.

### RFC2710-ROUTER-31

**A Querier in Listeners Present state that receives a Done starts the timer at the shortened value, starts the retransmit timer, and sends a Multicast-Address-Specific Query.**

> "done received" and "(start timer*," and "start rxmt timer," and "send m-a-s query)" — §6, `rfc2710.txt:813-816`

- Strength: description. Class: wire.
- Check idea: a Done message for an address that a Querier tracks as having a listener makes the Querier send a Multicast-Address-Specific Query for that address.

### RFC2710-ROUTER-32

**A Querier in Listeners Present state that receives another Report restarts the timer and stays in Listeners Present state.**

> "report received" and "(start timer)" — §6, `rfc2710.txt:819-820`

- Strength: description. Class: internal.
- Check idea: a second Report for an address that already has a listener restarts that address's timer without a new routing notification.

### RFC2710-ROUTER-33

**A Non-Querier sends no MLD messages of its own and reacts only to messages it receives.**

> "non-Queriers do not send any messages and are only
> driven by message reception." — §6, `rfc2710.txt:848-849`

- Strength: description. Class: wire.
- Check idea: a router acting as Non-Querier for a link sends no Query, and its per-group state changes only after it receives a Report, a Multicast-Address-Specific Query, or a timer runs out.

### RFC2710-ROUTER-34

**A Non-Querier in No Listeners Present state that receives a Report notifies the routing component, starts the timer, and enters Listeners Present state.**

> "report received" and "(notify routing +," and "start timer)" — §6, `rfc2710.txt:862-864`

- Strength: description. Class: internal.
- Check idea: the first Report for an address on a link, received by a Non-Querier, makes it start tracking that address as having a listener.

### RFC2710-ROUTER-35

**A Non-Querier in Listeners Present state whose timer expires notifies the routing component and enters No Listeners Present state.**

> "timer expired" and "(notify routing -)" — §6, `rfc2710.txt:854-855`

- Strength: description. Class: internal.
- Check idea: a Non-Querier that hears no Report for an address before that address's timer runs out stops tracking it as having a listener.

### RFC2710-ROUTER-36

**A Non-Querier in Checking Listeners state whose timer expires notifies the routing component and enters No Listeners Present state.**

> "timer expired" and "(notify routing -)" — §6, `rfc2710.txt:854-855`

- Strength: description. Class: internal.
- Check idea: a Non-Querier that saw a Multicast-Address-Specific Query for an address and heard no Report before its timer runs out stops tracking it as having a listener.

### RFC2710-ROUTER-37

**A Non-Querier in Checking Listeners state that receives a Report starts the timer and enters Listeners Present state.**

> "report received" and "(start timer)" — §6, `rfc2710.txt:867-868`

- Strength: description. Class: internal.
- Check idea: a Report that arrives at a Non-Querier after it saw a Multicast-Address-Specific Query for the same address restores that address to Listeners Present.

### RFC2710-ROUTER-38

**A Non-Querier in Listeners Present state that sees a Multicast-Address-Specific Query starts the timer at the shortened value and enters Checking Listeners state.**

> "m-a-s query rec'd" and "(start timer*)" — §6, `rfc2710.txt:870-871`

- Strength: description. Class: internal.
- Check idea: a Multicast-Address-Specific Query for an address that a Non-Querier tracks as having a listener lowers that address's timer toward the Last Listener Query Count of Query intervals.

### RFC2710-ROUTER-39

**A Non-Querier in Listeners Present state that receives another Report restarts the timer and stays in Listeners Present state.**

> "report received" and "(start timer)" — §6, `rfc2710.txt:874-875`

- Strength: description. Class: internal.
- Check idea: a second Report for an address that a Non-Querier already tracks as having a listener restarts that address's timer without a new routing notification.

## Timers and default values

### RFC2710-TIMER-1

**A non-default timer setting must be the same on every router on a link.**

> "Most of these timers are configurable.  If non-default settings are
> used, they MUST be consistent among all routers on a single link." — §7, `rfc2710.txt:880-881`

- Strength: must. Class: internal.
- Check idea: on a link where every router uses the same non-default value for a timer, MLD works as specified; a router configured with a different value is a misconfiguration this document rules out.

### RFC2710-TIMER-2

**The Robustness Variable tunes MLD for expected packet loss, and its default is 2.**

> "The Robustness Variable allows tuning for the expected packet loss on
> a link." and "Default: 2" — §7.1, `rfc2710.txt:887-888`, `rfc2710.txt:891`

- Strength: description. Class: internal.
- Check idea: a router or node with no configured Robustness Variable uses the value 2 wherever the variable appears in a timer formula or a retry count.

### RFC2710-TIMER-3

**MLD tolerates one fewer packet loss than the Robustness Variable without breaking listener detection.**

> "MLD is robust to (Robustness Variable - 1) packet
> losses." — §7.1, `rfc2710.txt:889-890`

- Strength: description. Class: internal.
- Check idea: with the default Robustness Variable of 2, the loss of a single Report or Query does not make a router forget a listener.

### RFC2710-TIMER-4

**The Robustness Variable is never zero and should not be one.**

> "The Robustness Variable MUST NOT be zero, and SHOULD NOT be
> one." — §7.1, `rfc2710.txt:890-891`

- Strength: must not, should not. Class: internal.
- Check idea: a router or node rejects, or never allows, a configured Robustness Variable of zero, and a configured value of one is a discouraged setting.

### RFC2710-TIMER-5

**The Query Interval is the time between General Queries a Querier sends, and its default is 125 seconds.**

> "The Query Interval is the interval between General Queries sent by
> the Querier." and "Default: 125 seconds." — §7.2, `rfc2710.txt:905-906`

- Strength: description. Class: internal.
- Check idea: with no configured Query Interval, a Querier sends its periodic General Queries 125 seconds apart.

### RFC2710-TIMER-6

**The Query Response Interval is the Maximum Response Delay in periodic General Queries, and its default is 10 seconds.**

> "The Maximum Response Delay inserted into the periodic General
> Queries." and "Default: 10000 (10 seconds)" — §7.3, `rfc2710.txt:914-915`

- Strength: description. Class: internal.
- Check idea: with no configured Query Response Interval, a periodic General Query that a Querier sends carries a Maximum Response Delay of 10000 (10 seconds).

### RFC2710-TIMER-7

**The Query Response Interval, in seconds, is smaller than the Query Interval.**

> "The number of seconds represented by the [Query Response
> Interval] must be less than the [Query Interval]." — §7.3, `rfc2710.txt:920-921`

- Strength: must (lower case). Class: internal.
- Check idea: a router configured with a Query Response Interval equal to or larger than the Query Interval, in seconds, uses an invalid pair of values.

### RFC2710-TIMER-8

**The Multicast Listener Interval is the time that must pass with no Report before a router decides an address has no listener on a link.**

> "The Multicast Listener Interval is the amount of time that must pass
> before a router decides there are no more listeners for an address on
> a link." — §7.4, `rfc2710.txt:925-927`

- Strength: must (lower case). Class: internal.
- Check idea: a router that hears no Report for an address for the Multicast Listener Interval decides the address has no listener on that link.

### RFC2710-TIMER-9

**The Multicast Listener Interval equals the Robustness Variable times the Query Interval, plus one Query Response Interval.**

> "This value MUST be ((the
> Robustness Variable) times (the
> Query Interval)) plus (one Query Response Interval)." — §7.4, `rfc2710.txt:927-928`

- Strength: must. Class: internal.
- Check idea: with the default Robustness Variable, Query Interval and Query Response Interval, a router waits (2 * 125) + 10 = 260 seconds of silence before it decides an address has no listener.

### RFC2710-TIMER-10

**The Other Querier Present Interval is the time that must pass with no Query from the current Querier before a router decides it should query itself.**

> "The Other Querier Present Interval is the length of time that must
> pass before a router decides that there is no longer another router
> which should be the querier on a link." — §7.5, `rfc2710.txt:932-934`

- Strength: must, should (lower case). Class: internal.
- Check idea: a Non-Querier that hears no Query from the current Querier for the Other Querier Present Interval starts sending Queries itself.

### RFC2710-TIMER-11

**The Other Querier Present Interval equals the Robustness Variable times the Query Interval, plus half a Query Response Interval.**

> "This value MUST be ((the
> Robustness Variable) times (the Query Interval)) plus (one half of
> one Query Response Interval)." — §7.5, `rfc2710.txt:934-936`

- Strength: must. Class: internal.
- Check idea: with the default Robustness Variable, Query Interval and Query Response Interval, a router waits (2 * 125) + 5 = 255 seconds of silence before it resumes querying.

### RFC2710-TIMER-12

**The Startup Query Interval is the time between General Queries a Querier sends at startup, and its default is a quarter of the Query Interval.**

> "The Startup Query Interval is the interval between General Queries
> sent by a Querier on startup." and "Default: 1/4 the Query Interval." — §7.6, `rfc2710.txt:940-941`

- Strength: description. Class: internal.
- Check idea: with no configured Startup Query Interval and the default Query Interval, a router just becoming Querier sends its first few General Queries about 31.25 seconds apart.

### RFC2710-TIMER-13

**The Startup Query Count is the number of General Queries a Querier sends at startup, and its default is the Robustness Variable.**

> "The Startup Query Count is the number of Queries sent out on startup,
> separated by the Startup Query Interval." and "Default: the Robustness
> Variable." — §7.7, `rfc2710.txt:945-947`

- Strength: description. Class: internal.
- Check idea: with no configured Startup Query Count, a router just becoming Querier sends 2 General Queries spaced by the Startup Query Interval before falling back to the normal Query Interval.

### RFC2710-TIMER-14

**The Last Listener Query Interval is the Maximum Response Delay in Multicast-Address-Specific Queries and the spacing between them, and its default is 1 second.**

> "The Last Listener Query Interval is the Maximum Response Delay
> inserted into Multicast-Address-Specific Queries sent in response to
> Done messages, and is also the amount of time between Multicast-
> Address-Specific Query messages." and "Default: 1000 (1 second)" — §7.8, `rfc2710.txt:961-964`

- Strength: description. Class: internal.
- Check idea: with no configured Last Listener Query Interval, a Multicast-Address-Specific Query carries a Maximum Response Delay of 1000 (1 second), and two such Queries for the same address are 1 second apart.

### RFC2710-TIMER-15

**The Last Listener Query Count is the number of Multicast-Address-Specific Queries a router sends before giving up on an address, and its default is the Robustness Variable.**

> "The Last Listener Query Count is the number of Multicast-Address-
> Specific Queries sent before the router assumes there are no
> remaining listeners for an address on a link." and "Default: the
> Robustness Variable." — §7.9, `rfc2710.txt:972-975`

- Strength: description. Class: internal.
- Check idea: with no configured Last Listener Query Count, a Querier sends 2 Multicast-Address-Specific Queries after a Done before it decides the address has no listener left.

### RFC2710-TIMER-16

**The Unsolicited Report Interval is the time between repeats of a node's own unprompted Report, and its default is 10 seconds.**

> "The Unsolicited Report Interval is the time between repetitions of a
> node's initial report of interest in a multicast address." and "Default:
> 10 seconds." — §7.10, `rfc2710.txt:979-981`

- Strength: description. Class: internal.
- Check idea: with no configured Unsolicited Report Interval, a node that starts listening to an address, with no Query involved, repeats its Report about 10 seconds after the first one.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§5 to §7. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 to §4 (definitions, message format, protocol
  description) and §8 to §13 (message destinations, security, references).

Within the in-scope sections, these parts have no entry, with the reason:

In §5 to §7:

- Lines 379-381 (§5 lead-in: "Node behavior is more formally specified..."), 407-409 (events intro), 457-459 (actions intro), 487-489 (notation note: "each state transition arc is labeled..."), 571 (§6 lead-in), 581-582 and 598-599 (router events/actions intros), 657-661 (per-group states intro), 682-684 and 712-714 (per-group events/actions intros), 752-754 (diagrams intro: "The following state diagrams apply per group per link..."), 791-792 and 847-848 (per-diagram lead sentences), 882-883 (§7 typographic note about parentheses): pure lead-in or notation sentences with no independent checkable content; the facts they introduce are cataloged where the diagrams and the definitions themselves appear.
- Lines 399-405 (bare definitions of "Delaying Listener" and "Idle Listener" state, with no fact beyond "has/does not have a timer running") and 667-669 (bare definition of "Listeners Present" router state): the only checkable content of these definitions is which events and actions apply, already covered by the node and router transition entries (RFC2710-NODE-17 through 23, RFC2710-ROUTER-26 through 39).
- Lines 410-412, 414-417, 439-441 (node event applicability: "start listening" only in Non-Listener, "stop listening" only in Delaying/Idle Listener, "timer expired" only in Delaying Listener) and lines 583-585, 593-596, 704-706, 708-710 (router event applicability: "query timer expired" only when Querier, "other querier present timer expired" only when Non-Querier, "timer expired" only in Listeners Present or Checking Listeners, "retransmit timer expired" only in Checking Listeners): these restatements are redundant with which states the corresponding arcs leave from in the state diagrams already cataloged; no separate entry adds a fact the diagram entries do not already carry.
- Lines 469-471 and 472-474 (node actions "set flag" / "clear flag") and lines 738-739 (router action "clear retransmit timer"): pure naming of an action with no content beyond what the transition entries that use them already state (which transition sets or clears the flag, or clears the retransmit timer).
- Lines 485-486 (node action "stop timer"): pure naming, no content beyond the transition entries (RFC2710-NODE-18, 21) that already state which event stops the timer.
- Lines 761-766 (worked example following RFC2710-ROUTER-25: "E.g. a router that starts as a Querier, receives a Done message..."): an illustrative example of the rule already cataloged in RFC2710-ROUTER-25, not an independent statement.
- Lines 888-889 (§7.1: "the Robustness Variable may be increased"), 908-910 (§7.2 tuning note: "an administrator may tune the number of MLD messages..."), 917-919 (§7.3 tuning note: "an administrator may tune the burstiness..."), and 966-968 (§7.8 tuning note: "This value may be tuned to modify the 'leave latency'..."): administrative tuning advice about why an operator might choose a non-default value; not a testable requirement on a node's or router's behavior beyond what the definition-and-default entries already state (that these timers are configurable, RFC2710-TIMER-1).
