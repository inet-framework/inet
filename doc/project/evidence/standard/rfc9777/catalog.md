# RFC 9777 (Multicast Listener Discovery Version 2 (MLDv2) for IPv6) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9777-*` · **Stands on:** [standards.md](../../protocol/mld/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 9777, Multicast Listener Discovery Version 2 (MLDv2) for IPv6, of
March 2025, a document of the in-scope set of MLD. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc9777.txt`](../../../../../../standards/RFC/rfc9777.txt) —
  Multicast Listener Discovery Version 2 (MLDv2) for IPv6, March 2025. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc9777.txt>.

The other document of the in-scope set has a catalog of its own:
[`rfc2710/catalog.md`](../rfc2710/catalog.md) (MLDv1, whose node and router state diagrams the
interoperation rules of §8 fall back to).

The in-scope sections are §5 to §9. RFC 9777 is the 2025 revision of RFC 3810; its Appendix B
names the changes.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mld/standards.md). The features these statements build are in
[`features.md`](../../protocol/mld/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mld/coverage.md`](../../model/mld/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc9777.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC9777-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 9777 uses the keywords of RFC 2119 in capitals;
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

## Message formats: general rules

### RFC9777-GEN-1

**The IPv6 Next Header value 58 identifies an MLDv2 message.**

> "MLDv2 messages are identified in IPv6
>    packets by a preceding Next Header value of 58." — §5, `rfc9777.txt:737-738`

- Strength: description. Class: wire.
- Check idea: send an IPv6 packet that carries an MLDv2 message and observe that its Next Header field, or the Next Header field of the last extension header before the MLD payload, holds the value 58.

### RFC9777-GEN-2

**Every MLDv2 message must use a link-local IPv6 source address.**

> "All MLDv2 messages
>    described in this document MUST be sent with a link-local IPv6 Source
>    Address" — §5, `rfc9777.txt:738-740`

- Strength: must. Class: wire.
- Check idea: capture a Query or Report sent by a node and observe that the IPv6 source address is a link-local address.

### RFC9777-GEN-3

**Every MLDv2 message must have an IPv6 Hop Limit of 1.**

> "an IPv6 Hop Limit of 1" — §5, `rfc9777.txt:740`

- Strength: must. Class: wire.
- Check idea: capture a Query or Report sent by a node and observe that the IPv6 Hop Limit field holds 1.

### RFC9777-GEN-4

**Every MLDv2 message must carry an IPv6 Router Alert option in a Hop-by-Hop Options header.**

> "an IPv6 Router Alert option
>    [RFC2711] in a Hop-by-Hop Options header." — §5, `rfc9777.txt:740-741`

- Strength: must. Class: wire.
- Check idea: capture a Query or Report sent by a node and observe a Hop-by-Hop Options header that carries the Router Alert option.

### RFC9777-GEN-5

**The Multicast Listener Query message has Type 130.**

> "Multicast Listener Query (Type = decimal 130)" — §5, `rfc9777.txt:752`

- Strength: description. Class: wire.
- Check idea: capture a Query message and observe that its Type field holds 130.

### RFC9777-GEN-6

**The Version 2 Multicast Listener Report message has Type 143.**

> "Version 2 Multicast Listener Report (Type = decimal 143)" — §5, `rfc9777.txt:754`

- Strength: description. Class: wire.
- Check idea: capture a Version 2 Report message and observe that its Type field holds 143.

### RFC9777-GEN-7

**An MLDv2 implementation must also support the MLDv1 Report and Done message types for interoperability.**

> "an implementation of MLDv2 must also support the
>    following two message types:" and "Multicast Listener Report (Type = decimal 131)" and "Multicast Listener Done (Type = decimal 132)" — §5, `rfc9777.txt:758-759`, `rfc9777.txt:761`, `rfc9777.txt:763`

- Strength: must (lower case). Class: end-to-end.
- Check idea: send an MLDv1 Multicast Listener Report (Type 131) and an MLDv1 Multicast Listener Done (Type 132) to a node and observe that the node accepts and processes each, instead of rejecting the type.

### RFC9777-GEN-8

**A node must silently ignore an unrecognized MLD message type.**

> "Unrecognized message types MUST be silently ignored." — §5, `rfc9777.txt:765`

- Strength: must. Class: end-to-end.
- Check idea: send a node an ICMPv6 message of an MLD type it does not recognize and observe that the node takes no listening-state action and sends no report about it.
## Multicast Listener Query message

### RFC9777-QRY-1

**A multicast router in Querier state sends Multicast Listener Queries to learn the listening state of neighboring interfaces.**

> "Multicast Listener Queries are sent by multicast routers in Querier
>    state to query the Multicast Address Listening state of neighboring
>    interfaces." — §5.1, `rfc9777.txt:775-777`

- Strength: description. Class: wire.
- Check idea: put a router into Querier state on a link and observe that it sends Multicast Listener Query messages on that link.

### RFC9777-QRY-2

**The sender sets the Code field of a Query to zero.**

> "The Code field is initialized to zero by the sender" — §5.1.1, `rfc9777.txt:826`

- Strength: description. Class: wire.
- Check idea: capture a Query sent by a router and observe that the Code field holds zero.

### RFC9777-QRY-3

**A receiver ignores the Code field of a Query.**

> "ignored by
>    receivers." — §5.1.1, `rfc9777.txt:826-827`

- Strength: description. Class: end-to-end.
- Check idea: send a node Queries that differ only in the Code field and observe that the node's response behavior does not change with the Code value.

### RFC9777-QRY-4

**The Checksum field of a Query is the standard ICMPv6 checksum over the whole MLDv2 message and a pseudo-header of IPv6 fields.**

> "The Checksum field is the standard ICMPv6 checksum; it covers the
>    entire MLDv2 message, plus a "pseudo-header" of IPv6 header fields
>    [RFC4443] [RFC8200]." — §5.1.2, `rfc9777.txt:831-833`

- Strength: description. Class: encoding.
- Check idea: compute the ICMPv6 checksum of a captured Query over the message and the IPv6 pseudo-header and compare it to the value in the Checksum field.

### RFC9777-QRY-5

**A sender sets the Checksum field to zero before it computes the checksum.**

> "For computing the checksum, the Checksum field
>    is set to zero." — §5.1.2, `rfc9777.txt:833-834`

- Strength: description. Class: encoding.
- Check idea: recompute the checksum of a captured Query with the Checksum field zeroed and confirm the result matches the transmitted value.

### RFC9777-QRY-6

**A node must verify the checksum of a received Query before it processes the message.**

> "the checksum MUST be
>    verified before processing it." — §5.1.2, `rfc9777.txt:834-835`

- Strength: must. Class: end-to-end.
- Check idea: send a node a Query with an invalid checksum and observe that the node does not act on the message, then send one with a valid checksum and observe that it does.

### RFC9777-QRY-7

**The Maximum Response Code field of a Query encodes the Maximum Response Delay directly below 32768 and as a floating-point value at or above it.**

> "The Maximum Response Code field specifies the maximum time allowed
>    before sending a responding Report." and "If Maximum Response Code < 32768, Maximum Response Delay = Maximum
>    Response Code." and "If Maximum Response Code >=32768, Maximum Response Code represents
>    a floating-point value as follows:" and "Maximum Response Delay = (mant | 0x1000) << (exp+3)" — §5.1.3, `rfc9777.txt:839-840`, `rfc9777.txt:844-845`, `rfc9777.txt:847-848`, `rfc9777.txt:855`

- Strength: description. Class: encoding.
- Check idea: send Queries with a Maximum Response Code below 32768 and with one at or above 32768, and confirm the receiver derives the Maximum Response Delay by the direct value in the first case and by the exponent/mantissa formula in the second.

### RFC9777-QRY-8

**A sender sets the Reserved field of a Query to zero on transmission.**

> "The Reserved field is set to zero on transmission" — §5.1.4, `rfc9777.txt:866`

- Strength: description. Class: wire.
- Check idea: capture a Query sent by a router and observe that the Reserved field holds zero.

### RFC9777-QRY-9

**A receiver ignores the Reserved field of a Query.**

> "ignored on
>    reception." — §5.1.4, `rfc9777.txt:866-867`

- Strength: description. Class: end-to-end.
- Check idea: send a node Queries that differ only in the Reserved field and observe that the node's behavior does not change with that field.

### RFC9777-QRY-10

**In a General Query, the Multicast Address field is set to zero.**

> "For a General Query, the Multicast Address field is set to zero." — §5.1.5, `rfc9777.txt:871`

- Strength: description. Class: wire.
- Check idea: capture a General Query and observe that its Multicast Address field holds the all-zero address.

### RFC9777-QRY-11

**In a Multicast Address Specific or Multicast Address and Source Specific Query, the Multicast Address field holds the address being queried.**

> "For
>    a Multicast Address Specific Query or Multicast Address and Source
>    Specific Query, it is set to the multicast address being queried" — §5.1.5, `rfc9777.txt:871-873`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Specific Query and a Multicast Address and Source Specific Query and observe that each carries the queried multicast address in the Multicast Address field.

### RFC9777-QRY-12

**When the S flag of a Query is set, a receiving router suppresses its normal timer updates but still performs querier election and its own host-side processing.**

> "When set to one, the S flag indicates to any receiving multicast
>    routers that they have to suppress the normal timer updates they
>    perform upon receiving a Query.  Nevertheless, it does not suppress
>    the querier election or the normal "host-side" processing of a Query
>    that a router may be required to perform as a consequence of itself
>    being a multicast listener." — §5.1.7, `rfc9777.txt:884-889`

- Strength: may (lower case). Class: end-to-end.
- Check idea: send a router a Query with the S flag set to one and observe that the router does not restart its normal per-group timers, yet still takes part in querier election and in its own listener-side response processing.

### RFC9777-QRY-13

**The QRV field of a Query carries the Querier's Robustness Variable, or zero when that value exceeds 7.**

> "If non-zero, the QRV field contains the [Robustness Variable] value
>    used by the Querier.  If the Querier's [Robustness Variable] exceeds
>    7 (the maximum value of the QRV field), the QRV field is set to zero." — §5.1.8, `rfc9777.txt:893-895`

- Strength: description. Class: wire.
- Check idea: set the Querier's Robustness Variable to a value at most 7 and observe the same value in the QRV field of its Queries; set it above 7 and observe QRV zero.

### RFC9777-QRY-14

**A router adopts the QRV of the most recently received Query as its Robustness Variable, or a default or configured value when that QRV is zero.**

> "Routers adopt the QRV value from the most recently received Query as
>    their own [Robustness Variable] value, unless that most recently
>    received QRV was zero, in which case they use the default [Robustness
>    Variable] value specified in Section 9.1 or a statically configured
>    value." — §5.1.8, `rfc9777.txt:897-901`

- Strength: description. Class: internal.
- Check idea: send a router a Query with a non-zero QRV and observe that the router's own Robustness Variable takes that value; send one with QRV zero and observe the router keeps its default or configured value.

### RFC9777-QRY-15

**The QQIC field of a Query encodes the Querier's Query Interval directly below 128 and as a floating-point value at or above it.**

> "The QQIC field specifies the [Query Interval] used by the Querier." and "If QQIC < 128, QQI = QQIC" and "If QQIC >= 128, QQIC represents a floating-point value as follows:" and "QQI = (mant | 0x10) << (exp + 3)" — §5.1.9, `rfc9777.txt:905`, `rfc9777.txt:910`, `rfc9777.txt:912`, `rfc9777.txt:919`

- Strength: description. Class: encoding.
- Check idea: send Queries with a QQIC below 128 and with one at or above 128, and confirm the receiver derives the Querier's Query Interval by the direct value in the first case and by the exponent/mantissa formula in the second.

### RFC9777-QRY-16

**A router that is not the current Querier adopts the QQI of the most recently received Query as its Query Interval, or the default value when that QQI is zero.**

> "Multicast routers that are not the current Querier adopt the QQI
>    value from the most recently received Query as their own [Query
>    Interval] value, unless that most recently received QQI was zero, in
>    which case the receiving routers use the default [Query Interval]
>    value specified in Section 9.2." — §5.1.9, `rfc9777.txt:921-925`

- Strength: description. Class: internal.
- Check idea: as a non-Querier router, receive a Query with a non-zero QQIC and observe that the router's own Query Interval takes the derived value; receive one with QQIC zero and observe the router keeps the default.

### RFC9777-QRY-17

**The Number of Sources field of a Query is zero for a General Query or a Multicast Address Specific Query, and non-zero for a Multicast Address and Source Specific Query.**

> "The Number of Sources (N) field specifies how many source addresses
>    are present in the Query.  This number is zero in a General Query or
>    a Multicast Address Specific Query and non-zero in a Multicast
>    Address and Source Specific Query." — §5.1.10, `rfc9777.txt:929-932`

- Strength: description. Class: wire.
- Check idea: capture a General Query, a Multicast Address Specific Query, and a Multicast Address and Source Specific Query, and observe the Number of Sources field value in each.

### RFC9777-QRY-18

**The Number of Sources field of a Query is limited by the MTU of the link the Query is sent on.**

> "This number is limited by the MTU
>    of the link over which the Query is transmitted." — §5.1.10, `rfc9777.txt:932-933`

- Strength: description. Class: wire.
- Check idea: on a link of a given MTU, observe that a Querier never sends a Query whose Number of Sources field would make the message exceed that MTU.

### RFC9777-QRY-19

**The Source Address fields of a Query hold as many unicast addresses as the Number of Sources field states.**

> "The Source Address [i] fields are a vector of n unicast addresses,
>    where n is the value in the Number of Sources (N) field." — §5.1.11, `rfc9777.txt:943-944`

- Strength: description. Class: encoding.
- Check idea: capture a Multicast Address and Source Specific Query and count the Source Address entries against the value of the Number of Sources field.

### RFC9777-QRY-20

**A receiver must include any additional octets found beyond the defined Query fields when it verifies the MLD checksum.**

> "MLDv2 implementations MUST include those
>    octets in the computation to verify the received MLD Checksum" — §5.1.12, `rfc9777.txt:950-951`

- Strength: must. Class: wire.
- Check idea: send a node a Query with extra octets appended after the defined fields, computed into the checksum, and observe that the node accepts the message when the checksum, extra octets included, is valid.

### RFC9777-QRY-21

**A receiver must otherwise ignore any additional octets found beyond the defined Query fields.**

> "MUST otherwise ignore those additional octets." — §5.1.12, `rfc9777.txt:951-952`

- Strength: must. Class: end-to-end.
- Check idea: send a node a Query with extra, checksum-valid octets appended after the defined fields and observe that the node's processing of the Query does not change because of their content.

### RFC9777-QRY-22

**A sender must not add octets beyond the defined fields when it sends a Query.**

> "an MLDv2 implementation MUST NOT include additional octets beyond the
>    fields described above." — §5.1.12, `rfc9777.txt:953-954`

- Strength: must not. Class: wire.
- Check idea: capture a Query sent by a router and observe that the message ends at the last Source Address field with no trailing octets.

### RFC9777-QRY-23

**A General Query has the Multicast Address field and the Number of Sources field both zero.**

> "A "General Query" is sent by the Querier to learn which multicast
>    addresses have listeners on an attached link.  In a General Query,
>    both the Multicast Address field and the Number of Sources (N)
>    field are zero." — §5.1.13, `rfc9777.txt:960-963`

- Strength: description. Class: wire.
- Check idea: capture a General Query sent by the Querier and observe that both the Multicast Address field and the Number of Sources field hold zero.

### RFC9777-QRY-24

**A Multicast Address Specific Query carries the queried address in the Multicast Address field and zero in the Number of Sources field.**

> "A "Multicast Address Specific Query" is sent by the Querier to
>    learn if a particular multicast address has any listeners on an
>    attached link.  In a Multicast Address Specific Query, the
>    Multicast Address field contains the multicast address of
>    interest, while the Number of Sources (N) field is set to zero." — §5.1.13, `rfc9777.txt:965-969`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Specific Query and observe the queried address in the Multicast Address field and zero in the Number of Sources field.

### RFC9777-QRY-25

**A Multicast Address and Source Specific Query carries the queried address in the Multicast Address field and the sources of interest in the Source Address fields.**

> "A "Multicast Address and Source Specific Query" is sent by the
>    Querier to learn if any of the sources from the specified list for
>    the particular multicast address has any listeners on an attached
>    link or not.  In a Multicast Address and Source Specific Query the
>    Multicast Address field contains the multicast address of
>    interest, while the Source Address [i] field(s) contain(s) the
>    source address(es) of interest." — §5.1.13, `rfc9777.txt:971-977`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address and Source Specific Query and observe the queried address in the Multicast Address field and the queried sources in the Source Address fields.

### RFC9777-QRY-26

**Every MLDv2 Query must be sent with a valid IPv6 link-local source address.**

> "All MLDv2 Queries MUST be sent with a valid IPv6 link-local source
>    address." — §5.1.14, `rfc9777.txt:981-982`

- Strength: must. Class: wire.
- Check idea: capture a Query sent by the Querier and observe that its IPv6 source address is a valid link-local address.

### RFC9777-QRY-27

**A node must silently discard a Query with an unspecified or otherwise invalid link-local source address, and should log a warning.**

> "If a node (router or host) receives a Query Message with
>    the IPv6 Source Address set to the unspecified address (::), or any
>    other address that is not a valid IPv6 link-local address, it MUST
>    silently discard the message and SHOULD log a warning." — §5.1.14, `rfc9777.txt:982-985`

- Strength: must (discard); should (log a warning). Class: end-to-end.
- Check idea: send a node a Query with the unspecified source address and observe that the node discards it and takes no listening-state action, and check for a logged warning.

### RFC9777-QRY-28

**A General Query is sent to the link-scope all-nodes multicast address ff02::1.**

> "In MLDv2, General Queries are sent to the link-scope all-nodes
>    multicast address (ff02::1)." — §5.1.15, `rfc9777.txt:989-990`

- Strength: description. Class: wire.
- Check idea: capture a General Query sent by the Querier and observe that its IPv6 destination address is ff02::1.

### RFC9777-QRY-29

**A Multicast Address Specific or Multicast Address and Source Specific Query is sent to the multicast address of interest.**

> "Multicast Address Specific and
>    Multicast Address and Source Specific Queries are sent with an IP
>    destination address equal to the multicast address of interest." — §5.1.15, `rfc9777.txt:990-992`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Specific Query and observe that its IPv6 destination address equals the queried multicast address.

### RFC9777-QRY-30

**A node must accept and process a Query addressed to any address assigned to the interface it arrives on.**

> "a node MUST accept and process any Query whose IP
>    Destination Address field contains any of the addresses (unicast or
>    multicast) assigned to the interface on which the Query arrives." — §5.1.15, `rfc9777.txt:993-995`

- Strength: must. Class: end-to-end.
- Check idea: send a node a Query addressed to a unicast address assigned to the receiving interface and observe that the node processes it as it would a Query addressed to the expected multicast address.
## Version 2 Multicast Listener Report message

### RFC9777-REP-1

**An IP node sends a Version 2 Report to tell neighboring routers its current or changed Multicast Address Listening state.**

> "Version 2 Multicast Listener Reports are sent by IP nodes to report
>    (to neighboring routers) the current Multicast Address Listening
>    state, or changes in the Multicast Address Listening state, of their
>    interfaces." — §5.2, `rfc9777.txt:1000-1003`

- Strength: description. Class: wire.
- Check idea: change a node's set of joined multicast addresses and observe that it sends a Version 2 Report describing the current or changed state.

### RFC9777-REP-2

**A sender sets the Reserved field of a Report to zero on transmission.**

> "The Reserved field is set to zero on transmission" — §5.2.1, `rfc9777.txt:1085`

- Strength: description. Class: wire.
- Check idea: capture a Report sent by a node and observe that the Reserved field holds zero.

### RFC9777-REP-3

**A receiver ignores the Reserved field of a Report.**

> "ignored on
>    reception." — §5.2.1, `rfc9777.txt:1085-1086`

- Strength: description. Class: end-to-end.
- Check idea: send a router Reports that differ only in the Reserved field and observe that the router's processing does not change with that field.

### RFC9777-REP-4

**The Checksum field of a Report is the standard ICMPv6 checksum over the whole MLDv2 message and a pseudo-header of IPv6 fields.**

> "The Checksum Field is the standard ICMPv6 checksum; it covers the
>    entire MLDv2 message, plus a "pseudo-header" of IPv6 header fields
>    [RFC4443] [RFC8200]." — §5.2.2, `rfc9777.txt:1090-1092`

- Strength: description. Class: encoding.
- Check idea: compute the ICMPv6 checksum of a captured Report over the message and the IPv6 pseudo-header and compare it to the value in the Checksum field.

### RFC9777-REP-5

**A sender sets the Checksum field to zero before it computes the checksum.**

> "In order to compute the checksum, the Checksum
>    field is set to zero." — §5.2.2, `rfc9777.txt:1092-1093`

- Strength: description. Class: encoding.
- Check idea: recompute the checksum of a captured Report with the Checksum field zeroed and confirm the result matches the transmitted value.

### RFC9777-REP-6

**A node must verify the checksum of a received Report before it processes the message.**

> "the checksum MUST
>    be verified before processing it." — §5.2.2, `rfc9777.txt:1093-1094`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report with an invalid checksum and observe that the router does not update its multicast forwarding state, then send one with a valid checksum and observe that it does.

### RFC9777-REP-7

**The Nr of Mcast Address Records field states how many Multicast Address Records the Report carries.**

> "The Nr of the Mcast Address Records (M) field specifies how many
>    Multicast Address Records are present in this Report." — §5.2.4, `rfc9777.txt:1104-1105`

- Strength: description. Class: wire.
- Check idea: capture a Report and count its Multicast Address Records against the value of the Nr of Mcast Address Records field.

### RFC9777-REP-8

**Each Multicast Address Record of a Report holds the sending interface's listening information for one multicast address.**

> "Each Multicast Address Record is a block of fields that contain
>    information on the sender listening to a single multicast address on
>    the interface from which the Report is sent." — §5.2.5, `rfc9777.txt:1109-1111`

- Strength: description. Class: encoding.
- Check idea: capture a Report with several Multicast Address Records and observe that each record pertains to exactly one multicast address.

### RFC9777-REP-9

**The Record Type field of a Multicast Address Record states its type.**

> "The Record Type field specifies the type of the Multicast Address
>    Record." — §5.2.6, `rfc9777.txt:1115-1116`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Record and read its Record Type field against the defined record type values.

### RFC9777-REP-10

**The Aux Data Len field gives the length of the Auxiliary Data field in 32-bit words, or zero when there is none.**

> "The Aux Data Len field contains the length of the Auxiliary Data
>    field in this Multicast Address Record, in units of 32-bit words.  It
>    may contain zero, to indicate the absence of any auxiliary data." — §5.2.7, `rfc9777.txt:1121-1123`

- Strength: may (lower case). Class: encoding.
- Check idea: capture a Multicast Address Record without auxiliary data and observe that the Aux Data Len field holds zero and the Auxiliary Data field is absent.

### RFC9777-REP-11

**The Number of Sources field of a Multicast Address Record states how many source addresses it carries.**

> "The Number of Sources (N) field specifies how many source addresses
>    are present in this Multicast Address Record." — §5.2.8, `rfc9777.txt:1127-1128`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Record and count its Source Address entries against the value of its Number of Sources field.

### RFC9777-REP-12

**The Multicast Address field of a Multicast Address Record holds the address the record pertains to.**

> "The Multicast Address field contains the multicast address to which
>    this Multicast Address Record pertains." — §5.2.9, `rfc9777.txt:1132-1133`

- Strength: description. Class: wire.
- Check idea: capture a Multicast Address Record and observe that its Multicast Address field holds the address the record reports on.

### RFC9777-REP-13

**The Source Address fields of a Multicast Address Record hold as many unicast addresses as its Number of Sources field states.**

> "The Source Address [i] fields are a vector of n unicast addresses,
>    where n is the value in this record's Number of Sources (N) field." — §5.2.10, `rfc9777.txt:1137-1138`

- Strength: description. Class: encoding.
- Check idea: capture a Multicast Address Record and count its Source Address entries against the value of its own Number of Sources field.

### RFC9777-REP-14

**The Auxiliary Data field of a Multicast Address Record, when present, holds additional information about that record.**

> "The Auxiliary Data field, if present, contains additional information
>    that pertains to this Multicast Address Record." — §5.2.11, `rfc9777.txt:1142-1143`

- Strength: description. Class: encoding.
- Check idea: capture a Multicast Address Record whose Aux Data Len field is non-zero and observe that the Auxiliary Data field is present and belongs to that same record.

### RFC9777-REP-15

**A sender must not send auxiliary data in a Multicast Address Record, and must set the Aux Data Len field to zero.**

> "implementations of MLDv2 MUST NOT include any
>    auxiliary data (i.e., MUST set the Aux Data Len field to zero) in any
>    transmitted Multicast Address Record" — §5.2.11, `rfc9777.txt:1145-1147`

- Strength: must. Class: wire.
- Check idea: capture a Multicast Address Record sent by a node and observe that its Aux Data Len field holds zero and no Auxiliary Data follows.

### RFC9777-REP-16

**A receiver must ignore any auxiliary data present in a received Multicast Address Record.**

> "MUST ignore any such data
>    present in any received Multicast Address Record." — §5.2.11, `rfc9777.txt:1147-1148`

- Strength: must. Class: end-to-end.
- Check idea: send a node a Multicast Address Record that carries non-zero auxiliary data and observe that the node's resulting listening state does not depend on the content of that data.

### RFC9777-REP-17

**A receiver must include any additional octets found beyond the last Multicast Address Record when it verifies the MLD checksum.**

> "the last Multicast Address Record, MLDv2 implementations MUST include
>    those octets in the computation to verify the received MLD Checksum" — §5.2.12, `rfc9777.txt:1156-1157`

- Strength: must. Class: wire.
- Check idea: send a node a Report with extra octets appended after the last Multicast Address Record, computed into the checksum, and observe that the node accepts the message when the checksum, extra octets included, is valid.

### RFC9777-REP-18

**A receiver must otherwise ignore any additional octets found beyond the last Multicast Address Record.**

> "MUST otherwise ignore those additional octets." — §5.2.12, `rfc9777.txt:1158`

- Strength: must. Class: end-to-end.
- Check idea: send a node a Report with extra, checksum-valid octets appended after the last Multicast Address Record and observe that its processing of the Report does not change because of their content.

### RFC9777-REP-19

**A sender must not add octets beyond the last Multicast Address Record when it sends a Report.**

> "an MLDv2 implementation MUST NOT include additional octets
>    beyond the last Multicast Address Record." — §5.2.12, `rfc9777.txt:1159-1160`

- Strength: must not. Class: wire.
- Check idea: capture a Report sent by a node and observe that the message ends at the last Multicast Address Record with no trailing octets.

### RFC9777-REP-20

**A node sends a Current-State Record, in response to a Query, to report the current listening state of an interface for one multicast address.**

> "A "Current-State Record" is sent by a node in response to a Query
>       received on an interface.  It reports the current listening state
>       of that interface, with respect to a single multicast address." — §5.2.13, `rfc9777.txt:1167-1169`

- Strength: description. Class: wire.
- Check idea: send a node a Query and observe that its Report carries a Current-State Record for each multicast address it currently listens to.

### RFC9777-REP-21

**A MODE_IS_INCLUDE record (Record Type 1) reports an INCLUDE filter-mode and carries the interface's source-list.**

> "MODE_IS_INCLUDE - indicates that the interface has a filter-
>           mode of INCLUDE for the specified multicast address.  The
>           Source Address [i] fields in this Multicast Address Record
>           contain the interface's source-list for the specified
>           multicast address." — §5.2.13, `rfc9777.txt:1173-1177`

- Strength: description. Class: wire.
- Check idea: put an interface in INCLUDE filter-mode for a multicast address and observe that the resulting Current-State Record has Record Type 1 and lists the interface's source-list.

### RFC9777-REP-22

**A MODE_IS_INCLUDE record is never sent with an empty source-list.**

> "A MODE_IS_INCLUDE Record is never sent
>           with an empty source-list." — §5.2.13, `rfc9777.txt:1177-1178`

- Strength: description. Class: wire.
- Check idea: put an interface in INCLUDE filter-mode with an empty source-list for a multicast address and observe that no MODE_IS_INCLUDE record is sent for that address.

### RFC9777-REP-23

**A MODE_IS_EXCLUDE record (Record Type 2) reports an EXCLUDE filter-mode and carries the interface's source-list when it is non-empty.**

> "MODE_IS_EXCLUDE - indicates that the interface has a filter-
>           mode of EXCLUDE for the specified multicast address.  The
>           Source Address [i] fields in this Multicast Address Record
>           contain the interface's source-list for the specified
>           multicast address, if it is non-empty." — §5.2.13, `rfc9777.txt:1180-1184`

- Strength: description. Class: wire.
- Check idea: put an interface in EXCLUDE filter-mode for a multicast address with a non-empty source-list and observe that the resulting Current-State Record has Record Type 2 and lists that source-list.

### RFC9777-REP-24

**An SSM-aware host should not send a MODE_IS_EXCLUDE record for a multicast address within the SSM address range.**

> "An SSM-aware host
>           SHOULD NOT send a MODE_IS_EXCLUDE record type for multicast
>           addresses that fall within the SSM address range" — §5.2.13, `rfc9777.txt:1184-1186`

- Strength: should not. Class: wire.
- Check idea: on an SSM-aware host, put an interface in EXCLUDE filter-mode for a multicast address inside the SSM range and observe that the host's Report carries no MODE_IS_EXCLUDE record for that address.

### RFC9777-REP-25

**A node sends a Filter-Mode-Change Record whenever a local IPv6MulticastListen invocation changes the interface's filter-mode for a multicast address.**

> "A "Filter-Mode-Change Record" is sent by a node whenever a local
>       invocation of IPv6MulticastListen causes a change of the filter-
>       mode (i.e., a change from INCLUDE to EXCLUDE, or from EXCLUDE to
>       INCLUDE) of the interface-level state entry for a particular
>       multicast address, whether the source-list changes at the same
>       time or not." — §5.2.13, `rfc9777.txt:1189-1195`

- Strength: description. Class: wire.
- Check idea: change an interface's filter-mode for a multicast address and observe that the node's next Report carries a Filter-Mode-Change Record for that address.

### RFC9777-REP-26

**A CHANGE_TO_INCLUDE_MODE record (Record Type 3) reports a change to INCLUDE filter-mode and carries the interface's new source-list when non-empty.**

> "CHANGE_TO_INCLUDE_MODE - indicates that the interface has
>           changed to INCLUDE filter-mode for the specified multicast
>           address.  The Source Address [i] fields in this Multicast
>           Address Record contain the interface's new source-list for the
>           specified multicast address, if it is non-empty." — §5.2.13, `rfc9777.txt:1198-1202`

- Strength: description. Class: wire.
- Check idea: change an interface's filter-mode for a multicast address to INCLUDE with a non-empty source-list and observe that the resulting record has Record Type 3 and lists the new source-list.

### RFC9777-REP-27

**A CHANGE_TO_EXCLUDE_MODE record (Record Type 4) reports a change to EXCLUDE filter-mode and carries the interface's new source-list when non-empty.**

> "CHANGE_TO_EXCLUDE_MODE - indicates that the interface has
>           changed to EXCLUDE filter-mode for the specified multicast
>           address.  The Source Address [i] fields in this Multicast
>           Address Record contain the interface's new source-list for the
>           specified multicast address, if it is non-empty." — §5.2.13, `rfc9777.txt:1204-1208`

- Strength: description. Class: wire.
- Check idea: change an interface's filter-mode for a multicast address to EXCLUDE with a non-empty source-list and observe that the resulting record has Record Type 4 and lists the new source-list.

### RFC9777-REP-28

**An SSM-aware host should not send a CHANGE_TO_EXCLUDE_MODE record for a multicast address within the SSM address range.**

> "An SSM-aware
>           host SHOULD NOT send a CHANGE_TO_EXCLUDE_MODE record type for
>           multicast addresses that fall within the SSM address range." — §5.2.13, `rfc9777.txt:1208-1210`

- Strength: should not. Class: wire.
- Check idea: on an SSM-aware host, change an interface's filter-mode to EXCLUDE for a multicast address inside the SSM range and observe that the host's Report carries no CHANGE_TO_EXCLUDE_MODE record for that address.

### RFC9777-REP-29

**A node sends a Source-List-Change Record whenever a local IPv6MulticastListen invocation changes the source-list without changing the filter-mode.**

> "A "Source-List-Change Record" is sent by a node whenever a local
>       invocation of IPv6MulticastListen causes a change of source-list
>       that is not coincident with a change of filter-mode, of the
>       interface-level state entry for a particular multicast address." — §5.2.13, `rfc9777.txt:1212-1215`

- Strength: description. Class: wire.
- Check idea: change an interface's source-list for a multicast address without changing its filter-mode and observe that the node's next Report carries a Source-List-Change Record for that address.

### RFC9777-REP-30

**An ALLOW_NEW_SOURCES record (Record Type 5) lists the sources newly listened to for a multicast address.**

> "ALLOW_NEW_SOURCES - indicates that the Source Address [i]
>           fields in this Multicast Address Record contain a list of the
>           additional sources that the node wishes to listen to, for
>           packets sent to the specified multicast address." — §5.2.13, `rfc9777.txt:1220-1223`

- Strength: description. Class: wire.
- Check idea: add sources to an interface's source-list for a multicast address and observe that the resulting record has Record Type 5 and lists exactly those added sources.

### RFC9777-REP-31

**A BLOCK_OLD_SOURCES record (Record Type 6) lists the sources no longer listened to for a multicast address.**

> "BLOCK_OLD_SOURCES - indicates that the Source Address [i]
>           fields in this Multicast Address Record contain a list of the
>           sources that the node no longer wishes to listen to, for
>           packets sent to the specified multicast address." — §5.2.13, `rfc9777.txt:1229-1232`

- Strength: description. Class: wire.
- Check idea: remove sources from an interface's source-list for a multicast address and observe that the resulting record has Record Type 6 and lists exactly those removed sources.

### RFC9777-REP-32

**When a source-list change both allows new sources and blocks old sources, a node sends one ALLOW_NEW_SOURCES record and one BLOCK_OLD_SOURCES record for that address.**

> "If a change of source-list results in both allowing new sources and
>    blocking old sources, then two Multicast Address Records are sent for
>    the same multicast address, one of type ALLOW_NEW_SOURCES and one of
>    type BLOCK_OLD_SOURCES." — §5.2.13, `rfc9777.txt:1238-1241`

- Strength: description. Class: wire.
- Check idea: change an interface's source-list for a multicast address by both adding and removing sources in the same update, and observe two records for that address, one of each type.

### RFC9777-REP-33

**A node must silently ignore a Multicast Address Record with an unrecognized Record Type and continue processing the rest of the Report.**

> "Multicast Address Records with an unrecognized Record Type value MUST
>    be silently ignored, with the rest of the report being processed." — §5.2.13, `rfc9777.txt:1246-1247`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report whose first Multicast Address Record has an unrecognized Record Type and observe that the router ignores that record but still acts on the remaining records.

### RFC9777-REP-34

**An MLDv2 Report must be sent with a valid link-local source address, or with the unspecified address if the interface has none yet.**

> "An MLDv2 Report MUST be sent with a valid IPv6 link-local source
>    address, or the unspecified address (::), if the sending interface
>    has not acquired a valid link-local address yet." — §5.2.14, `rfc9777.txt:1271-1273`

- Strength: must. Class: wire.
- Check idea: capture a Report sent by a node that already has a link-local address and observe a valid link-local source address; capture one sent before the interface has such an address and observe the unspecified address.

### RFC9777-REP-35

**A router must silently discard a Report that is not sent with a valid link-local address, without acting on its contents.**

> "routers MUST silently discard a message that is
>    not sent with a valid link-local address, without taking any action
>    on the contents of the packet." — §5.2.14, `rfc9777.txt:1283-1285`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report from an invalid, non-link-local source address and observe that the router's multicast listening state for the reported addresses does not change.

### RFC9777-REP-36

**A router discards a Report when it cannot identify the source address as belonging to a link connected to the receiving interface.**

> "Thus, a Report is discarded if the
>    router cannot identify the source address of the packet as belonging
>    to a link connected to the interface on which the packet was
>    received." — §5.2.14, `rfc9777.txt:1285-1288`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Report whose link-local source address does not belong to the link of the receiving interface and observe that the router discards it.

### RFC9777-REP-37

**A router discards a Report sent with the unspecified address.**

> "A Report sent with the unspecified address is also
>    discarded by the router." — §5.2.14, `rfc9777.txt:1288-1289`

- Strength: description. Class: end-to-end.
- Check idea: send a router a Report from the unspecified source address and observe that the router does not change its multicast forwarding state because of it.

### RFC9777-REP-38

**A node that sends a Report still applies the new listening state from that Report's records to the packets it treats, even when the Report is discarded by the router.**

> "the reporting node has modified its listening state for
>    multicast addresses that are contained in the Multicast Address
>    Records of the Report message.  From now on, it will treat packets
>    sent to those multicast addresses according to this new listening
>    state." — §5.2.14, `rfc9777.txt:1291-1295`

- Strength: description. Class: internal.
- Check idea: have a node send a Report from the unspecified address that changes its listening state for a multicast address, and observe that the node itself now treats packets to that address according to the new state.

### RFC9777-REP-39

**Once a valid link-local address becomes available, a node should generate new Reports for every multicast address it has joined on the interface.**

> "Once a valid link-local address is available, a node SHOULD
>    generate new MLDv2 Report messages for all multicast addresses joined
>    on the interface." — §5.2.14, `rfc9777.txt:1295-1297`

- Strength: should. Class: wire.
- Check idea: let a node acquire a link-local address after joining multicast addresses using the unspecified source address, and observe that it then sends fresh Reports covering all of those addresses.

### RFC9777-REP-40

**A Version 2 Report is sent to the IP destination address ff02::16.**

> "Version 2 Multicast Listener Reports are sent with an IP destination
>    address of ff02::16, to which all MLDv2-capable multicast routers
>    listen" — §5.2.15, `rfc9777.txt:1301-1303`

- Strength: description. Class: wire.
- Check idea: capture a Version 2 Report sent by a node and observe that its IPv6 destination address is ff02::16.

### RFC9777-REP-41

**A node in version 1 compatibility mode sends version 1 Reports to the multicast address named in the Report's own Multicast Address field.**

> "A node that operates in version 1
>    compatibility mode (see details in Section 8) sends version 1 Reports
>    to the multicast address specified in the Multicast Address field of
>    the Report." — §5.2.15, `rfc9777.txt:1304-1307`

- Strength: description. Class: wire.
- Check idea: put a node in version 1 compatibility mode and observe that a version 1 Report it sends for a multicast address is addressed to that same multicast address.

### RFC9777-REP-42

**A node must accept and process a version 1 Report addressed to any address assigned to the interface it arrives on.**

> "a node MUST accept and process any version
>    1 Report whose IP Destination Address field contains any of the IPv6
>    addresses (unicast or multicast) assigned to the interface on which
>    the Report arrives." — §5.2.15, `rfc9777.txt:1307-1310`

- Strength: must. Class: end-to-end.
- Check idea: send a node a version 1 Report addressed to a unicast address assigned to the receiving interface and observe that the node processes it as it would one addressed to the expected multicast address.

### RFC9777-REP-43

**When the Multicast Address Records for a Report do not fit the link MTU, a node sends them in as many Reports as needed.**

> "If the set of Multicast Address Records required in a Report does not
>    fit within the size limit of a single Report message (as determined
>    by the MTU of the link on which it will be sent), the Multicast
>    Address Records are sent in as many Report messages as needed to
>    report the entire set." — §5.2.16, `rfc9777.txt:1315-1319`

- Strength: description. Class: wire.
- Check idea: give a node enough joined multicast addresses that their records exceed one Report's MTU-limited size, and observe that the node sends multiple Reports that together carry every record.

### RFC9777-REP-44

**When a single oversized record is not of type IS_EX or TO_EX, a node splits it into several records, each in a separate Report.**

> "if its Type is not IS_EX or TO_EX, it is split into multiple
>       Multicast Address Records; each such record contains a different
>       subset of the source addresses and is sent in a separate Report." — §5.2.16, `rfc9777.txt:1325-1327`

- Strength: description. Class: wire.
- Check idea: give a node an IS_IN or TO_IN record with enough source addresses to exceed one Report's MTU-limited size, and observe that the node sends it as several records, each with a distinct subset of the sources, in separate Reports.

### RFC9777-REP-45

**When a single oversized record is of type IS_EX or TO_EX, a node sends one record with as many sources as fit and leaves out the rest.**

> "if its Type is IS_EX or TO_EX, a single Multicast Address Record
>       is sent, with as many source addresses as can fit; the remaining
>       source addresses are not reported." — §5.2.16, `rfc9777.txt:1329-1333`

- Strength: description. Class: wire.
- Check idea: give a node an IS_EX or TO_EX record with enough source addresses to exceed one Report's MTU-limited size, and observe that it sends a single record holding as many sources as fit, with the rest absent from that Report.

## Listener: action on change of per-interface state

### RFC9777-LSN-1

**A node runs the MLDv2 listener protocol on every interface that supports multicast reception, even when several such interfaces share one link.**

> "A node performs the protocol described in this section over all
>    interfaces on which multicast reception is supported, even if more
>    than one of those interfaces are connected to the same link." — §6, `rfc9777.txt:1348-1350`

- Strength: description. Class: internal.
- Check idea: give a node two interfaces on the same link; confirm each interface keeps and runs its own listening state and reports independently.

### RFC9777-LSN-2

**A node permanently listens to the link-scope all-nodes multicast address, from all sources, on every interface that supports multicast listening.**

> "On all nodes -- that is, all hosts and routers
>    including multicast routers -- listening to packets destined to the
>    all-nodes multicast address, from all sources, is permanently enabled
>    on all interfaces on which multicast listening is supported." — §6, `rfc9777.txt:1360-1364`

- Strength: description. Class: end-to-end.
- Check idea: send a packet to ff02::1 from any source to a node's interface; confirm the node accepts it without any prior MLD report for that address.

### RFC9777-LSN-3

**A listener never sends an MLD message for the all-nodes address or for an address of scope 0 or 1, and it must send MLD messages for every other multicast address it listens to.**

> "No MLD
>    messages are ever sent regarding neither the link-scope all-nodes
>    multicast address, nor any multicast address of scope 0 (reserved) or
>    1 (node-local)." — §6, `rfc9777.txt:1364-1367`, and "Multicast listeners MUST send MLD messages for all
>    multicast addresses except for the link-scope all-nodes multicast
>    address and any multicast addresses of scope less than 2." — §6, `rfc9777.txt:1367-1369`

- Strength: must. Class: wire.
- Check idea: make a listener join the all-nodes address, a node-local address, and an ordinary multicast address; confirm it sends no report for the first two and does send one for the third.

### RFC9777-LSN-4

**A listener silently ignores every received MLD message that is not a Query, except where interoperation with MLDv1 requires otherwise.**

> "(Received MLD messages of types other than Query are silently
>    ignored, except as required for interoperation with nodes that
>    implement MLDv1.)" — §6, `rfc9777.txt:1381-1383`

- Strength: description. Class: end-to-end.
- Check idea: send a listener a Report or other non-Query MLD message; confirm the listener produces no reaction to it.

### RFC9777-LSN-5

**A change of per-interface listening state makes the node send a State-Change Report from that interface immediately.**

> "A change of per-interface state causes the node to immediately
>    transmit a State-Change Report from that interface." — §6.1, `rfc9777.txt:1396-1398`

- Strength: description. Class: wire.
- Check idea: change a listener's desired reception for a multicast address on an interface; confirm a State-Change Report leaves that interface without delay.

### RFC9777-LSN-6

**The State-Change Report's record type and source-list are found by comparing the filter-mode and source-list before and after the change.**

> "The type and
>    contents of the Multicast Address Record(s) in that Report are
>    determined by comparing the filter-mode and source-list for the
>    affected multicast address before and after the change, according to
>    Table 1." — §6.1, `rfc9777.txt:1397-1401`

- Strength: description. Class: wire.
- Check idea: drive several before/after filter-mode and source-list combinations for one multicast address and confirm the record type sent matches Table 1.

### RFC9777-LSN-7

**A per-interface record that is newly created or newly deleted is treated, on its non-existent side, as an INCLUDE filter-mode with an empty source-list.**

> "If no per-interface state existed for that multicast
>    address before the change (i.e., the change consisted of creating a
>    new per-interface record), or if no state exists after the change
>    (i.e., the change consisted of deleting a per-interface record), then
>    the "non-existent" state is considered to have an INCLUDE filter-mode
>    and an empty source-list." — §6.1, `rfc9777.txt:1401-1406`

- Strength: description. Class: internal.
- Check idea: let a listener join a fresh multicast address and separately leave one entirely; confirm the resulting report matches the row of Table 1 that starts, respectively ends, at INCLUDE with an empty list.

### RFC9777-LSN-8

**Table 1, row INCLUDE(A)→INCLUDE(B): the listener sends ALLOW(B-A) and BLOCK(A-B).**

> "| Old State   | New State   | State-Change Record Sent |" — `rfc9777.txt:1409` and "| INCLUDE (A) | INCLUDE (B) | ALLOW (B-A), BLOCK (A-B) |" — `rfc9777.txt:1411`

- Strength: description. Class: wire.
- Check idea: while in INCLUDE mode, add and drop sources at the same time; confirm the State-Change Report carries ALLOW for the newly added sources and BLOCK for the dropped ones.

### RFC9777-LSN-9

**Table 1, row EXCLUDE(A)→EXCLUDE(B): the listener sends ALLOW(A-B) and BLOCK(B-A).**

> "| Old State   | New State   | State-Change Record Sent |" — `rfc9777.txt:1409` and "| EXCLUDE (A) | EXCLUDE (B) | ALLOW (A-B), BLOCK (B-A) |" — `rfc9777.txt:1413`

- Strength: description. Class: wire.
- Check idea: while in EXCLUDE mode, change the excluded-source set; confirm the State-Change Report's ALLOW and BLOCK lists match the sources removed from, and added to, the exclude list.

### RFC9777-LSN-10

**Table 1, row INCLUDE(A)→EXCLUDE(B): the listener sends TO_EX(B).**

> "| Old State   | New State   | State-Change Record Sent |" — `rfc9777.txt:1409` and "| INCLUDE (A) | EXCLUDE (B) | TO_EX (B)                |" — `rfc9777.txt:1415`

- Strength: description. Class: wire.
- Check idea: switch a listener from INCLUDE to EXCLUDE mode for a multicast address; confirm the State-Change Report is a single TO_EX record with the new exclude source-list.

### RFC9777-LSN-11

**Table 1, row EXCLUDE(A)→INCLUDE(B): the listener sends TO_IN(B).**

> "| Old State   | New State   | State-Change Record Sent |" — `rfc9777.txt:1409` and "| EXCLUDE (A) | INCLUDE (B) | TO_IN (B)                |" — `rfc9777.txt:1417`

- Strength: description. Class: wire.
- Check idea: switch a listener from EXCLUDE to INCLUDE mode for a multicast address; confirm the State-Change Report is a single TO_IN record with the new include source-list.

### RFC9777-LSN-12

**An ALLOW or BLOCK record whose computed source-list is empty is left out of the report.**

> "If the computed source-list for either an ALLOW or a BLOCK State-
>    Change Record is empty, that record is omitted from the Report." — §6.1, `rfc9777.txt:1422-1423`, and "If the computed source-list for either an ALLOW or a BLOCK record is
>    empty, that record is omitted from the State-Change Report." — §6.1, `rfc9777.txt:1512-1513`

- Strength: description. Class: wire.
- Check idea: make a change whose ALLOW set (or BLOCK set) works out empty; confirm the sent report has no ALLOW (respectively BLOCK) record at all.

### RFC9777-LSN-13

**A listener schedules [Robustness Variable]-1 retransmissions of a State-Change Report, through a Retransmission Timer, at intervals chosen at random from (0, [Unsolicited Report Interval]).**

> "To cover the possibility of the State-Change Report being missed by
>    one or more multicast routers, [Robustness Variable] - 1
>    retransmissions are scheduled, through a Retransmission Timer, at
>    intervals chosen at random from the range (0, [Unsolicited Report
>    Interval])." — §6.1, `rfc9777.txt:1425-1429`

- Strength: description. Class: internal.
- Check idea: trigger one state change and count the further copies of the State-Change Report the listener sends; confirm the count and the spacing of their timers.

### RFC9777-LSN-14

**A further state change to the same per-interface entry, made before the first change's retransmissions finish, makes the node send a new State-Change Report immediately.**

> "If more changes to the same per-interface state entry occur before
>    all the retransmissions of the State-Change Report for the first
>    change have been completed, each such additional change triggers the
>    immediate transmission of a new State-Change Report." — §6.1, `rfc9777.txt:1431-1434`

- Strength: description. Class: wire.
- Check idea: change a listener's state again while retransmissions of the previous State-Change Report are still pending; confirm a new report is sent right away.

### RFC9777-LSN-15

**The new report's records are the difference for the latest change, merged into the pending report rather than sent as a separate message.**

> "As for the first Report, the per-interface state for the affected
>    multicast address before and after the latest change is compared." — §6.1, `rfc9777.txt:1438-1439`, and "The records that express the difference are built according to the
>    table above.  Nevertheless, these records are not transmitted in a
>    separate message, but they are instead merged with the contents of
>    the pending report, to create the new State-Change Report." — §6.1, `rfc9777.txt:1441-1445`

- Strength: description. Class: wire.
- Check idea: change state twice in quick succession; confirm only one merged State-Change Report appears on the link for the pending period, not two.

### RFC9777-LSN-16

**Sending the merged State-Change Report ends retransmissions of the earlier report and starts a fresh count of [Robustness Variable] transmissions of the new one.**

> "The transmission of the merged State-Change Report terminates
>    retransmissions of the earlier State-Change Reports for the same
>    multicast address and becomes the first of [Robustness Variable]
>    transmissions of the new State-Change Reports." — §6.1, `rfc9777.txt:1447-1452`

- Strength: description. Class: internal.
- Check idea: merge a new change into a pending report and then count further retransmissions; confirm exactly [Robustness Variable] total copies of the merged report are sent and none of the old report.

### RFC9777-LSN-17

**A source's retransmission counter is set to [Robustness Variable] when the source enters the Retransmission List, decreases by one each time a State-Change Report is sent, and the source leaves the list when the counter reaches zero.**

> "When a source is included in
>    the list, its counter is set to [Robustness Variable].  Each time a
>    State-Change Report is sent, the counter is decreased by one unit.
>    When the counter reaches zero, the source is deleted from the
>    Retransmission List for that multicast address." — §6.1, `rfc9777.txt:1461-1465`

- Strength: description. Class: internal.
- Check idea: add a source to the Retransmission List and observe successive State-Change Reports; confirm the source appears in exactly [Robustness Variable] of them and then stops appearing.

### RFC9777-LSN-18

**A filter-mode change makes the node include a Filter-Mode-Change Record in each of the next [Robustness Variable] State-Change Reports for that address.**

> "If the per-interface listening change that triggers the new report is
>    a filter-mode change, then the next [Robustness Variable] State-
>    Change Reports will include a Filter-Mode-Change Record." — §6.1, `rfc9777.txt:1467-1469`

- Strength: description. Class: wire.
- Check idea: change filter-mode for a multicast address and observe the following [Robustness Variable] reports; confirm each one carries a Filter-Mode-Change Record (TO_IN or TO_EX).

### RFC9777-LSN-19

**The Filter Mode Retransmission Counter is set to [Robustness Variable] on a filter-mode change, decreases by one per report sent, and once it reaches zero, later reports carry Source-List-Change Records instead if source-list changes are still pending.**

> "When the filter-mode
>    changes, the counter is set to [Robustness Variable].  Each time a
>    State-Change Report is sent the counter is decreased by one unit.
>    When the counter reaches zero, i.e., [Robustness Variable] State-
>    Change Reports with Filter-Mode-Change Records have been transmitted
>    after the last filter-mode change, and if source-list changes have
>    resulted in additional reports being scheduled, then the next State-
>    Change Report will include Source-List-Change Records." — §6.1, `rfc9777.txt:1474-1481`

- Strength: description. Class: internal.
- Check idea: cause a filter-mode change followed by source-list changes; after [Robustness Variable] Filter-Mode-Change reports, confirm the next report switches to a Source-List-Change Record.

### RFC9777-LSN-20

**While the Filter Mode Retransmission Counter is above zero, a new State-Change Report carries a TO_IN record if the current filter-mode is INCLUDE, and a TO_EX record otherwise.**

> "If the report should contain a Filter-Mode-
>    Change Record, i.e., the Filter Mode Retransmission Counter for that
>    multicast address has a value higher than zero, then, if the current
>    filter-mode of the interface is INCLUDE, a TO_IN record is included
>    in the report; otherwise, a TO_EX record is included." — §6.1, `rfc9777.txt:1485-1490`

- Strength: should (lower case). Class: wire.
- Check idea: trigger a report while the Filter Mode Retransmission Counter is still positive, once from INCLUDE mode and once from EXCLUDE mode; confirm TO_IN, respectively TO_EX, is sent.

### RFC9777-LSN-21

**Once the Filter Mode Retransmission Counter reaches zero, a new State-Change Report carries an ALLOW and a BLOCK record instead.**

> "If instead the
>    report should contain Source-List-Change Records, i.e., the Filter
>    Mode Retransmission Counter for that multicast address is zero, an
>    ALLOW and a BLOCK record is included." — §6.1, `rfc9777.txt:1489-1492`

- Strength: should (lower case). Class: wire.
- Check idea: let the Filter Mode Retransmission Counter reach zero and then trigger a further report; confirm it carries ALLOW/BLOCK records rather than TO_IN/TO_EX.

### RFC9777-LSN-22

**Table 2, row TO_IN: the record carries every source in the current per-interface state that must be forwarded.**

> "| Record | Sources Included                                      |" — `rfc9777.txt:1496` and "| TO_IN  | All in the current per-interface state that must be   |" and "|        | forwarded.                                            |" — `rfc9777.txt:1498-1499`

- Strength: must (lower case). Class: wire.
- Check idea: force a TO_IN record and compare its source-list against the sources the listener's current per-interface state is forwarding.

### RFC9777-LSN-23

**Table 2, row TO_EX: the record carries every source in the current per-interface state that must be blocked.**

> "| Record | Sources Included                                      |" — `rfc9777.txt:1496` and "| TO_EX  | All in the current per-interface state that must be   |" and "|        | blocked.                                              |" — `rfc9777.txt:1501-1502`

- Strength: must (lower case). Class: wire.
- Check idea: force a TO_EX record and compare its source-list against the sources the listener's current per-interface state is blocking.

### RFC9777-LSN-24

**Table 2, row ALLOW: the record carries every source with retransmission state (the Retransmission List) that must be forwarded.**

> "| Record | Sources Included                                      |" — `rfc9777.txt:1496` and "| ALLOW  | All with retransmission state (i.e., all sources from |" and "|        | the Retransmission List) that must be forwarded.      |" — `rfc9777.txt:1504-1505`

- Strength: must (lower case). Class: wire.
- Check idea: with sources pending in the Retransmission List, force a report and confirm the ALLOW record lists exactly the forwarded sources of that list.

### RFC9777-LSN-25

**Table 2, row BLOCK: the record carries every source with retransmission state that must be blocked.**

> "| Record | Sources Included                                      |" — `rfc9777.txt:1496` and "| BLOCK  | All with retransmission state that must be blocked.   |" — `rfc9777.txt:1507`

- Strength: must (lower case). Class: wire.
- Check idea: with sources pending in the Retransmission List, force a report and confirm the BLOCK record lists exactly the blocked sources of that list.

## Listener: action on reception of a query

### RFC9777-LQRY-1

**A listener drops a received Query unless its source address is a valid link-local address, its Hop Limit is 1, and the Router Alert option is present.**

> "Upon reception of an MLD message that contains a Query, the node
>    checks if the source address of the message is a valid link-local
>    address, if the Hop Limit is set to 1, and if the Router Alert option
>    is present in the Hop-by-Hop Options header of the IPv6 packet.  If
>    any of these checks fail, the packet is dropped." — §6.2, `rfc9777.txt:1526-1530`

- Strength: description. Class: end-to-end.
- Check idea: send Queries that each fail one of the three checks (bad source address, wrong Hop Limit, missing Router Alert) and one valid Query; confirm the node acts only on the valid one.

### RFC9777-LQRY-2

**A listener delays its response to a valid Query by a random amount bounded by the Maximum Response Delay derived from the Query's Maximum Response Code.**

> "If the validity of the MLD message is verified, the node starts to
>    process the Query.  Instead of responding immediately, the node
>    delays its response by a random amount of time, bounded by the
>    Maximum Response Delay value derived from the Maximum Response Code
>    in the received Query Message." — §6.2, `rfc9777.txt:1532-1536`

- Strength: description. Class: wire.
- Check idea: send a valid Query and measure the delay before the listener's report; confirm it falls within the Maximum Response Delay the Query's Maximum Response Code encodes, and is not immediate.

### RFC9777-LQRY-3

**For each interface running the listener part of MLDv2, a node must be able to maintain an Interface Timer, a Multicast Address Timer per address to report, and a per-address list of sources to report.**

> "the node must be
>    able to maintain the following state:
>
>    *  an Interface Timer for scheduling responses to General Queries;
>
>    *  a Multicast Address Timer for scheduling responses to Multicast
>       Address (and Source) Specific Queries, for each multicast address
>       the node has to report on; and
>
>    *  a per-multicast-address list of sources to be reported in response
>       to a Multicast Address and Source Specific Query." — §6.2, `rfc9777.txt:1545-1555`

- Strength: must (lower case). Class: internal.
- Check idea: schedule overlapping General and Multicast-Address-Specific Query responses on one interface and confirm each is tracked with its own timer and, for a Source Specific Query, its own recorded source-list.

### RFC9777-LQRY-4

**On a valid Query, a node checks whether it has matching per-interface listening state to report, and if so picks a response delay at random from (0, Maximum Response Delay).**

> "When a valid General Query arrives on an interface, the node checks
>    whether it has any per-interface listening state record to report on,
>    or not.  Similarly, when a valid Multicast Address (and Source)
>    Specific Query arrives on an interface, the node checks whether it
>    has a per-interface listening state record that corresponds to the
>    queried multicast address (and source), or not.  If it does, a delay
>    for a response is randomly selected in the range (0, [Maximum
>    Response Delay])" — §6.2, `rfc9777.txt:1557-1564`

- Strength: description. Class: wire.
- Check idea: send a General Query and a Multicast-Address-Specific Query to a node with no matching listening state; confirm no delay is scheduled and no report follows.

### RFC9777-LQRY-5

**Rule 1: a pending response to an earlier General Query, scheduled sooner than the new delay, means no extra response is scheduled.**

> "If there is a pending response to a previous General Query
>        scheduled sooner than the selected delay, no additional response
>        needs to be scheduled." — §6.2, `rfc9777.txt:1570-1572`

- Strength: description. Class: wire.
- Check idea: send two General Queries close together where the first response is already due sooner than the second would require; confirm only one report is sent, at the earlier time.

### RFC9777-LQRY-6

**Rule 2: for a General Query, the Interface Timer is (re)armed to the newly selected delay, canceling any earlier pending General-Query response.**

> "If the received Query is a General Query, the Interface Timer is
>        used to schedule a response to the General Query after the
>        selected delay.  Any previously pending response to a General
>        Query is canceled." — §6.2, `rfc9777.txt:1574-1577`

- Strength: description. Class: wire.
- Check idea: send a General Query while an earlier General-Query response is pending further out; confirm the earlier one is replaced by a single response at the new, sooner delay.

### RFC9777-LQRY-7

**Rule 3: with no pending response for that address, a Multicast-Address(-and-Source) Specific Query arms the Multicast Address Timer, recording the queried sources for a Source Specific Query.**

> "If the received Query is a Multicast Address Specific Query or a
>        Multicast Address and Source Specific Query and there is no
>        pending response to a previous Query for this multicast address,
>        then the Multicast Address Timer is used to schedule a report.
>        If the received Query is a Multicast Address and Source Specific
>        Query, the list of queried sources is recorded for use when
>        generating a response." — §6.2, `rfc9777.txt:1579-1585`

- Strength: description. Class: wire.
- Check idea: send a fresh Multicast Address and Source Specific Query for an address with no pending response; confirm a Multicast Address Timer is armed and the queried source-list is recorded.

### RFC9777-LQRY-8

**Rule 4: a pending response for the address, met by a Multicast Address Specific Query or an empty recorded source-list, clears the source-list and schedules one response at the earlier of the two delays.**

> "If there is already a pending response to a previous Query
>        scheduled for this multicast address, and either the new Query is
>        a Multicast Address Specific Query or the recorded source-list
>        associated with the multicast address is empty, then the
>        multicast address source-list is cleared and a single response is
>        scheduled, using the Multicast Address Timer.  The new response
>        is scheduled to be sent at the earliest of the remaining time for
>        the pending report and the selected delay." — §6.2, `rfc9777.txt:1587-1594`

- Strength: description. Class: wire.
- Check idea: while a Source Specific Query response is pending, send a Multicast Address Specific Query for the same address; confirm the recorded source-list is cleared and a single response is scheduled at the sooner of the two delays.

### RFC9777-LQRY-9

**Rule 5: a Multicast Address and Source Specific Query, met by a pending response with a non-empty source-list, adds its sources to the recorded list and schedules one response at the earlier of the two delays.**

> "If the received Query is a Multicast Address and Source Specific
>        Query and there is a pending response for this multicast address
>        with a non-empty source-list, then the multicast address source-
>        list is augmented to contain the list of sources in the new
>        Query, and a single response is scheduled using the Multicast
>        Address Timer." — §6.2, `rfc9777.txt:1596-1601`

- Strength: description. Class: wire.
- Check idea: while a Source Specific Query response with sources {S1} is pending, send a second Source Specific Query for {S2}; confirm the recorded list becomes {S1,S2} and only one response is scheduled.

## Listener: action on timer expiration

### RFC9777-LTIM-1

**When the Interface Timer expires, the node sends one Current-State Record for each multicast address the interface has listening state for.**

> "If the expired timer is the Interface Timer (i.e., there is a
>        pending response to a General Query), then one Current-State
>        Record is sent for each multicast address for which the specified
>        interface has listening state, as described in Section 4.2." — §6.3, `rfc9777.txt:1611-1614`

- Strength: description. Class: wire.
- Check idea: let the Interface Timer expire while the node listens to several multicast addresses; confirm one Current-State Record appears for each.

### RFC9777-LTIM-2

**A Current-State Record carries the multicast address, its filter-mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE), and its source-list.**

> "The
>        Current-State Record carries the multicast address and its
>        associated filter-mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and
>        source-list." — §6.3, `rfc9777.txt:1614-1617`

- Strength: description. Class: wire.
- Check idea: inspect a sent Current-State Record and confirm it holds the multicast address, the correct filter-mode code, and the matching source-list.

### RFC9777-LTIM-3

**Multiple Current-State Records are packed into individual Report messages as far as possible.**

> "Multiple Current-State Records are packed into
>        individual Report messages, to the extent possible." — §6.3, `rfc9777.txt:1617-1618`

- Strength: description. Class: wire.
- Check idea: make the Interface Timer expire while several addresses need reporting; confirm as many Current-State Records as fit are packed into one Report before a second Report is used.

### RFC9777-LTIM-4

**Implementations are recommended to spread transmission of the Interface-Timer Report messages over (0, [Maximum Response Delay]) instead of a single timer, to avoid bursts.**

> "This naive algorithm may result in bursts of packets when a node
>        listens to a large number of multicast addresses.  Instead of
>        using a single Interface Timer, implementations are recommended
>        to spread transmission of such Report messages over the interval
>        (0, [Maximum Response Delay])." — §6.3, `rfc9777.txt:1620-1624`

- Strength: may (lower case). Class: wire.
- Check idea: let a node with many listened addresses respond to a General Query; confirm the Report messages spread across the interval rather than leaving in one burst.

### RFC9777-LTIM-5

**An implementation must avoid the ack-implosion problem: it must not send a Report immediately on reception of a General Query.**

> "Note that any such implementation
>        MUST avoid the "ack-implosion" problem, i.e., MUST NOT send a
>        Report immediately upon reception of a General Query." — §6.3, `rfc9777.txt:1624-1626`

- Strength: must, must not. Class: wire.
- Check idea: send a General Query to a node listening to several addresses; confirm no Report appears at time zero and the responses are spread out.

### RFC9777-LTIM-6

**When a Multicast Address Timer with an empty recorded source-list expires, the node sends a single Current-State Record for that address if, and only if, the interface still has listening state for it.**

> "If the expired timer is a Multicast Address Timer and the list of
>        recorded sources for that multicast address is empty (i.e., there
>        is a pending response to a Multicast Address Specific Query),
>        then if, and only if, the interface has listening state for that
>        multicast address, a single Current-State Record is sent for that
>        address.  The Current-State Record carries the multicast address
>        and its associated filter-mode (MODE_IS_INCLUDE or
>        MODE_IS_EXCLUDE) and source-list, if any." — §6.3, `rfc9777.txt:1628-1635`

- Strength: description. Class: wire.
- Check idea: let such a timer expire once while the node still listens to the address, and once after the node has left it; confirm a Current-State Record is sent only in the first case.

### RFC9777-LTIM-7

**When a Multicast Address Timer with a non-empty recorded source-list expires, and the interface still has listening state for that address, the Current-State Record's content is found from Table 3.**

> "If the expired timer is a Multicast Address Timer and the list of
>        recorded sources for that multicast address is non-empty (i.e.,
>        there is a pending response to a Multicast Address and Source
>        Specific Query), then if, and only if, the interface has
>        listening state for that multicast address, the contents of the
>        corresponding Current-State Record are determined from the per-
>        interface state and the pending response record, as specified in
>        Table 3." — §6.3, `rfc9777.txt:1637-1644`

- Strength: description. Class: wire.
- Check idea: let a Source Specific Query's Multicast Address Timer expire while listening state still exists for the address; confirm a Current-State Record is built per Table 3, and none is sent if the state was removed meanwhile.

### RFC9777-LTIM-8

**Table 3, row INCLUDE(A): with pending sources B, the Current-State Record is IS_IN(A*B).**

> "| Per-Interface       | Set of Sources in the   | Current-State |" and "| State               | Pending Response Record | Record        |" — `rfc9777.txt:1647-1648` and "| INCLUDE (A)         | B                       | IS_IN (A*B)   |" — `rfc9777.txt:1650`

- Strength: description. Class: wire.
- Check idea: put the listener in INCLUDE(A) and have a pending Source-Specific query response for sources B; confirm the reported Current-State Record is IS_IN with the intersection of A and B.

### RFC9777-LTIM-9

**Table 3, row EXCLUDE(A): with pending sources B, the Current-State Record is IS_IN(B-A).**

> "| Per-Interface       | Set of Sources in the   | Current-State |" and "| State               | Pending Response Record | Record        |" — `rfc9777.txt:1647-1648` and "| EXCLUDE (A)         | B                       | IS_IN (B-A)   |" — `rfc9777.txt:1652`

- Strength: description. Class: wire.
- Check idea: put the listener in EXCLUDE(A) and have a pending Source-Specific query response for sources B; confirm the reported Current-State Record is IS_IN with B minus A.

### RFC9777-LTIM-10

**A resulting Current-State Record with an empty source-list is not sent.**

> "If the resulting Current-State Record has an empty set of source
>        addresses, then no response is sent." — §6.3, `rfc9777.txt:1657-1658`

- Strength: description. Class: wire.
- Check idea: pick A and B in Table 3 so the resulting source-list is empty; confirm no Current-State Record, and no Report, is sent for that address.

### RFC9777-LTIM-11

**After the required Report messages are generated, the source-lists recorded for the reported multicast addresses are cleared.**

> "After the required Report
>        messages have been generated, the source-lists associated with
>        any reported multicast addresses are cleared." — §6.3, `rfc9777.txt:1658-1660`

- Strength: description. Class: internal.
- Check idea: after a Source-Specific-Query response is sent, check the node's recorded source-list for that address is now empty.

### RFC9777-LTIM-12

**On Retransmission Timer expiration, while the Filter Mode Retransmission Counter is above zero, the report carries TO_IN if the current filter-mode is INCLUDE and TO_EX otherwise.**

> "If the expired timer is a Retransmission Timer for a multicast
>        address (i.e., there is a pending State-Change Report for that
>        multicast address), the contents of the report are determined as
>        follows.  If the report should contain a Filter-Mode-Change
>        Record, i.e., the Filter Mode Retransmission Counter for that
>        multicast address has a value higher than zero, then, if the
>        current filter-mode of the interface is INCLUDE, a TO_IN record
>        is included in the report; otherwise a TO_EX record is included." — §6.3, `rfc9777.txt:1662-1669`

- Strength: should (lower case). Class: wire.
- Check idea: let a Retransmission Timer fire while the Filter Mode Retransmission Counter is still positive, once from INCLUDE and once from EXCLUDE; confirm TO_IN, respectively TO_EX, is sent.

### RFC9777-LTIM-13

**In either case, the Filter Mode Retransmission Counter for that address is decremented by one after the report is sent.**

> "In both cases, the Filter Mode Retransmission Counter for that
>        multicast address is decremented by one unit after the
>        transmission of the report." — §6.3, `rfc9777.txt:1669-1672`

- Strength: description. Class: internal.
- Check idea: observe the Filter Mode Retransmission Counter before and after a scheduled retransmission; confirm it drops by exactly one.

### RFC9777-LTIM-14

**Once the Filter Mode Retransmission Counter is zero, a scheduled retransmission carries an ALLOW and a BLOCK record instead, built per Table 4.**

> "If instead the report should contain Source-List-Change Records,
>        i.e., the Filter Mode Retransmission Counter for that multicast
>        address is zero, an ALLOW and a BLOCK record is included.  The
>        contents of these records are built according to Table 4." — §6.3, `rfc9777.txt:1674-1677`

- Strength: should (lower case). Class: wire.
- Check idea: let the Filter Mode Retransmission Counter reach zero and let the Retransmission Timer fire again; confirm the report carries ALLOW/BLOCK records instead of TO_IN/TO_EX.

### RFC9777-LTIM-15

**Table 4, row TO_IN: the record carries every source in the current per-interface state that must be forwarded.**

> "| Record | Sources Included                                    |" — `rfc9777.txt:1680` and "| TO_IN  | All in the current per-interface state that must be |" and "|        | forwarded.                                          |" — `rfc9777.txt:1682-1683`

- Strength: must (lower case). Class: wire.
- Check idea: force a scheduled TO_IN retransmission and compare its source-list against the sources the current per-interface state forwards.

### RFC9777-LTIM-16

**Table 4, row TO_EX: the record carries every source in the current per-interface state that must be blocked.**

> "| Record | Sources Included                                    |" — `rfc9777.txt:1680` and "| TO_EX  | All in the current per-interface state that must be |" and "|        | blocked.                                            |" — `rfc9777.txt:1685-1686`

- Strength: must (lower case). Class: wire.
- Check idea: force a scheduled TO_EX retransmission and compare its source-list against the sources the current per-interface state blocks.

### RFC9777-LTIM-17

**Table 4, row ALLOW: the record carries every retransmission-state source that must be forwarded, and each included source's Source Retransmission Counter is decreased by one after transmission, deleting the source from the list when it reaches zero.**

> "| Record | Sources Included                                    |" — `rfc9777.txt:1680` and "| ALLOW  | All with retransmission state (i.e., all sources    |" and "|        | from the Retransmission List) that must be          |" and "|        | forwarded.  For each included source, its Source    |" and "|        | Retransmission Counter is decreased with one unit   |" and "|        | after the transmission of the report.  If the       |" and "|        | counter reaches zero, the source is deleted from    |" and "|        | the Retransmission List for that multicast address. |" — `rfc9777.txt:1688-1694`

- Strength: must (lower case). Class: wire.
- Check idea: with sources pending in the Retransmission List, watch successive scheduled reports; confirm each ALLOW record lists the forwarded pending sources and each source drops out once its own counter reaches zero.

### RFC9777-LTIM-18

**Table 4, row BLOCK: the record carries every retransmission-state source that must be blocked, and each included source's Source Retransmission Counter is decreased by one after transmission, deleting the source from the list when it reaches zero.**

> "| BLOCK  | All with retransmission state (i.e., all sources    |" and "|        | from the Retransmission List) that must be blocked. |" and "|        | For each included source, its Source Retransmission |" and "|        | Counter is decreased with one unit after the        |" and "|        | transmission of the report.  If the counter reaches |" and "|        | zero, the source is deleted from the Retransmission |" and "|        | List for that multicast address.                    |" — `rfc9777.txt:1696-1702`

- Strength: must (lower case). Class: wire.
- Check idea: with sources pending in the Retransmission List, watch successive scheduled reports; confirm each BLOCK record lists the blocked pending sources and each source drops out once its own counter reaches zero.

### RFC9777-LTIM-19

**In a scheduled retransmission, an ALLOW or BLOCK record whose computed source-list is empty is left out of the report.**

> "If the computed source-list for either an ALLOW or a BLOCK record
>        is empty, that record is omitted from the State-Change Report." — §6.3, `rfc9777.txt:1707-1708`

- Strength: description. Class: wire.
- Check idea: arrange a scheduled retransmission where the ALLOW set (or BLOCK set) works out empty; confirm that record is absent from the sent report.

## Router: conditions for queries

### RFC9777-RQ-1

**A multicast router runs MLD once per attached link, using only one of its interfaces when more than one reaches the same link.**

> "A multicast router performs the protocol described in this section
>    over each of its directly attached links.  If a multicast router has
>    more than one interface to the same link, it only needs to operate
>    this protocol over one of those interfaces." — §7, `rfc9777.txt:1727-1730`

- Strength: description. Class: internal.
- Check idea: attach a router to one link through two interfaces; confirm MLD queries and per-link state are handled once for the link, not duplicated per interface.

### RFC9777-RQ-2

**A router must configure each MLD interface to listen to every link-layer multicast address that an IPv6 multicast can generate.**

> "For each interface over which the router operates the MLD protocol,
>    the router must configure that interface to listen to all link-layer
>    multicast addresses that can be generated by IPv6 multicasts." — §7, `rfc9777.txt:1732-1734`

- Strength: must (lower case). Class: end-to-end.
- Check idea: send frames to several IPv6-multicast-derived link-layer multicast addresses on a router's MLD interface; confirm the router's link layer accepts all of them.

### RFC9777-RQ-3

**An Ethernet-attached router must accept Ethernet multicast addresses starting with 3333, or, if it cannot filter that range, must accept all Ethernet multicast addresses.**

> "For
>    example, an Ethernet-attached router must set its Ethernet address
>    reception filter to accept all Ethernet multicast addresses that
>    start with the hexadecimal value 3333 [RFC2464]; in the case of an
>    Ethernet interface that does not support the filtering of such a
>    multicast address range, it must be configured to accept ALL Ethernet
>    multicast addresses, in order to meet the requirements of MLD." — §7, `rfc9777.txt:1734-1740`

- Strength: must (lower case). Class: end-to-end.
- Check idea: send frames to destinations with and without the 3333 prefix on an Ethernet-attached router's interface; confirm every 3333-prefixed frame is accepted (or, on a router that cannot filter, every multicast frame is accepted).

### RFC9777-RQ-4

**On each MLD interface, a router must enable reception of the "all MLDv2-capable routers" address from all sources and must run the listener part of MLDv2 for that address.**

> "On each interface over which this protocol is being run, the router
>    MUST enable reception of the link-scope "all MLDv2-capable routers"
>    multicast address from all sources and MUST perform the Multicast
>    Address Listener part of MLDv2 for that address on that interface." — §7, `rfc9777.txt:1742-1745`

- Strength: must. Class: end-to-end.
- Check idea: send a packet to the all-MLDv2-routers address on a router's MLD interface and confirm it is accepted; confirm the router itself also acts as a listener for that address (e.g., answers Queries for it).

### RFC9777-RQ-5

**A router only needs to know that at least one node on a link listens to a given source and address; it is not required to track each neighboring node's interests individually.**

> "Multicast routers only need to know that at least one node on an
>    attached link listens to packets for a particular multicast address
>    from a particular source; a multicast router is not required to
>    individually keep track of the interests of each neighboring node." — §7, `rfc9777.txt:1747-1751`

- Strength: description. Class: internal.
- Check idea: have two listeners on a link request the same source and address; confirm the router keeps one combined record, not one record per listener.

### RFC9777-RQ-6

**Every multicast router on a subnet listens to the messages sent by listeners and keeps the same multicast listening state.**

> "All the multicast routers on the subnet listen to the
>    messages sent by Multicast Address Listeners, and maintain the same
>    multicast listening information state" — §7.1, `rfc9777.txt:1762-1764`

- Strength: description. Class: internal.
- Check idea: put two routers on one subnet with one in Non-Querier state; change a listener's state and confirm both routers' recorded listening state update the same way.

### RFC9777-RQ-7

**Only the Querier sends periodic or triggered Query Messages on the subnet.**

> "it is only the Querier that sends
>    periodical or triggered Query Messages on the subnet." — §7.1, `rfc9777.txt:1766-1767`

- Strength: description. Class: wire.
- Check idea: put two routers on one subnet; confirm Query Messages appear on the link only from the elected Querier, never from the Non-Querier.

### RFC9777-RQ-8

**The Querier periodically sends General Queries to build and refresh the listening state of routers on the attached link.**

> "The Querier periodically sends General Queries to request Multicast
>    Address Listener information from an attached link.  These queries
>    are used to build and refresh the Multicast Address Listening state
>    of routers on attached links." — §7.1, `rfc9777.txt:1769-1772`

- Strength: description. Class: wire.
- Check idea: watch a Querier's link over several query intervals and confirm General Queries recur at that period, and that router state for listened addresses is refreshed after each.

### RFC9777-RQ-9

**A node responds to Queries by reporting its listening state and set of listened sources as Current-State Records in a Version 2 Report.**

> "Nodes respond to these queries by reporting their Multicast Address
>    Listening state (and set of sources they listen to) with Current
>    State Multicast Address Records in MLD Version 2 Multicast Listener
>    Reports." — §7.1, `rfc9777.txt:1774-1777`

- Strength: description. Class: wire.
- Check idea: send a Query to a listener and confirm the reply is an MLDv2 Report whose Current-State Records match the listener's filter-mode and source set.

### RFC9777-RQ-10

**A node reports a change in its desired listening state with a Filter-Mode-Change Record or a Source-List-Change Record, marking an explicit change in a Multicast Address Record's filter-mode or source-list.**

> "As the desired listening state of a node changes, it reports these
>    changes using Filter-Mode-Change Records or Source-List-Change
>    Records.  These records indicate an explicit state change in a
>    multicast address at a node in either the Multicast Address Record's
>    source-list or its filter-mode." — §7.1, `rfc9777.txt:1780-1785`

- Strength: description. Class: wire.
- Check idea: change a node's filter-mode for an address, and separately change only its source-list; confirm the first produces a Filter-Mode-Change Record and the second a Source-List-Change Record.

### RFC9777-RQ-11

**Before deleting a multicast address or source from its state and pruning traffic, the Querier must query for other listeners of that address or source.**

> "When Multicast Address Listening is
>    terminated at a node or traffic from a particular source is no longer
>    desired, the Querier must query for other listeners of the multicast
>    address or of the source before deleting the multicast address (or
>    source) from its Multicast Address Listening state and pruning its
>    traffic." — §7.1, `rfc9777.txt:1785-1790`

- Strength: must (lower case). Class: wire.
- Check idea: have one listener leave a multicast address while another still listens; confirm the Querier sends a specific Query before any deletion or pruning, and that the surviving listener's reply prevents deletion.

### RFC9777-RQ-12

**A Multicast Address Specific Query verifies that no node still listens to the specified address, or rebuilds the listening state for it.**

> "A Multicast
>    Address Specific Query is sent to verify that there are no nodes that
>    listen to the specified multicast address or to "rebuild" the
>    listening state for a particular multicast address." — §7.1, `rfc9777.txt:1793-1796`

- Strength: description. Class: wire.
- Check idea: send a Multicast Address Specific Query for an address with no listeners and confirm no report follows; repeat with a listener present and confirm it reports back.

### RFC9777-RQ-13

**Multicast Address Specific Queries are sent when the Querier receives a State-Change Record showing that a node has stopped listening to a multicast address.**

> "Multicast
>    Address Specific Queries are sent when the Querier receives a State-
>    Change Record indicating that a node ceases to listen to a multicast
>    address." — §7.1, `rfc9777.txt:1796-1799`

- Strength: description. Class: wire.
- Check idea: send a State-Change Record that stops listening to an address and confirm the Querier answers with a Multicast Address Specific Query for it.

### RFC9777-RQ-14

**Multicast Address Specific Queries also enable a fast transition of a router from EXCLUDE to INCLUDE mode when a received State-Change Record motivates it.**

> "They are also sent in order to enable a fast transition of
>    a router from EXCLUDE to INCLUDE mode, in case a received State-
>    Change Record motivates this action." — §7.1, `rfc9777.txt:1799-1801`

- Strength: description. Class: wire.
- Check idea: send a State-Change Record that would let a router drop from EXCLUDE to INCLUDE mode for an address; confirm a Multicast Address Specific Query is sent for it.

### RFC9777-RQ-15

**A Multicast Address and Source Specific Query is sent by the Querier to learn whether any node still listens to the given sources for the given address.**

> "A Multicast Address and Source Specific Query is used to verify that
>    there are no nodes on a link that listen to traffic from a specific
>    set of sources." — §7.1, `rfc9777.txt:1803-1805`, and "This query is sent by the Querier in
>    order to learn if any node listens to packets sent to the specified
>    multicast address, from the specified source addresses." — §7.1, `rfc9777.txt:1807-1809`

- Strength: description. Class: wire.
- Check idea: send a Multicast Address and Source Specific Query for sources with no remaining listener and confirm no report follows; repeat with a listener present and confirm it reports back.

### RFC9777-RQ-16

**A Multicast Address and Source Specific Query lists the sources of that address that have been requested to no longer be forwarded.**

> "Multicast Address and Source Specific Queries list
>    sources for a particular multicast address that have been requested
>    to no longer be forwarded." — §7.1, `rfc9777.txt:1805-1807`

- Strength: description. Class: wire.
- Check idea: inspect the source-list of a sent Multicast Address and Source Specific Query and confirm it matches the sources a BLOCK or TO_EX record asked to stop forwarding.

### RFC9777-RQ-17

**Multicast Address and Source Specific Queries are sent only in response to State-Change Records, never in response to Current-State Records.**

> "Multicast
>    Address and Source Specific Queries are only sent in response to
>    State-Change Records and never in response to Current-State Records." — §7.1, `rfc9777.txt:1809-1811`

- Strength: description. Class: wire.
- Check idea: send a Current-State Record that lists fewer sources than the router currently forwards, and separately a State-Change BLOCK record for the same sources; confirm the Query is sent only after the State-Change Record.

## Router state: filter mode, filter timers and source timers

### RFC9777-RST-1

**A router keeps, per multicast address per attached link, a record of a filter-mode, a source-list, and various timers.**

> "Multicast routers that implement the MLDv2 protocol keep state per
>    multicast address per attached link.  This multicast address state
>    consists of a filter-mode, a list of sources, and various timers." — §7.2, `rfc9777.txt:1816-1818`

- Strength: description. Class: internal.
- Check idea: after a listener joins an address, inspect the router's per-link record for that address and confirm it holds a filter-mode, a source-list, and timers.

### RFC9777-RST-2

**A multicast address record is conceptually of the form (IPv6 multicast address, Filter Timer, Router Filter Mode, source records).**

> "(IPv6 multicast address, Filter Timer,
>           Router Filter Mode, (source records) )" — §7.2, `rfc9777.txt:1823-1824`

- Strength: description. Class: internal.
- Check idea: inspect a router's record for a listened multicast address and confirm it exposes an address, a Filter Timer, a Router Filter Mode, and a set of source records.

### RFC9777-RST-3

**Each source record is of the form (IPv6 source address, source timer).**

> "(IPv6 source address, source timer)" — §7.2, `rfc9777.txt:1826-1828`

- Strength: description. Class: internal.
- Check idea: inspect a router's source record for a listened source and confirm it exposes the source address and a source timer.

### RFC9777-RST-4

**When all sources of a multicast address are listened to, the router keeps an empty source-list with the Router Filter Mode set to EXCLUDE — the MLDv2 equivalent of an MLDv1 listening state.**

> "If all sources for a multicast address are listened to, an empty
>    source record list is kept with the Router Filter Mode set to
>    EXCLUDE." — §7.2, `rfc9777.txt:1830-1832`

- Strength: description. Class: internal.
- Check idea: have a listener request all sources of an address (EXCLUDE with an empty exclude-list); confirm the router's record is EXCLUDE with no source records.

### RFC9777-RST-5

**A router is in INCLUDE mode for a multicast address on an interface if every interested listener on the link is in INCLUDE mode.**

> "A router is in INCLUDE mode for a specific multicast address on a
>    given interface if all the listeners on the link interested in that
>    address are in INCLUDE mode." — §7.2.1, `rfc9777.txt:1849-1851`

- Strength: description. Class: internal.
- Check idea: have every listener of an address request INCLUDE mode; confirm the router's Router Filter Mode for that address is INCLUDE.

### RFC9777-RST-6

**In INCLUDE mode, the router's Include List is the set of sources one or more listeners on the link have requested to receive.**

> "The router state is represented through
>    the notation INCLUDE (A), where A is called the "Include List".  The
>    Include List is the set of sources that one or more listeners on the
>    link have requested to receive." — §7.2.1, `rfc9777.txt:1851-1854`

- Strength: description. Class: internal.
- Check idea: have listeners request different, overlapping sources of an address in INCLUDE mode; confirm the router's Include List is the union of the requested sources.

### RFC9777-RST-7

**In INCLUDE mode, sources in the Include List are forwarded by the router and every other source is blocked.**

> "All the sources from the Include
>    List will be forwarded by the router.  Any other source that is not
>    in the Include List will be blocked by the router." — §7.2.1, `rfc9777.txt:1854-1856`

- Strength: description. Class: end-to-end.
- Check idea: send traffic from a source in the Include List and from one outside it; confirm the router forwards only the first.

### RFC9777-RST-8

**A router is in EXCLUDE mode for a multicast address on an interface if at least one listener on the link is in EXCLUDE mode for it.**

> "A router is in EXCLUDE mode for a specific multicast address on a
>    given interface if there is at least one listener in EXCLUDE mode
>    interested in that address on the link." — §7.2.1, `rfc9777.txt:1858-1860`

- Strength: description. Class: internal.
- Check idea: have one listener of an address request EXCLUDE mode while others request INCLUDE; confirm the router's Router Filter Mode for that address is EXCLUDE.

### RFC9777-RST-9

**Once a Multicast Address Record with EXCLUDE filter-mode is received, the Router Filter Mode for that address is set to EXCLUDE.**

> "As a rule, once a Multicast Address
>    Record with a filter-mode of EXCLUDE is received, the Router Filter
>    Mode for that multicast address will be set to EXCLUDE." — §7.2.1, `rfc9777.txt:1863-1865`

- Strength: description. Class: internal.
- Check idea: send the router an EXCLUDE-mode report for an address currently in INCLUDE mode; confirm the Router Filter Mode switches to EXCLUDE.

### RFC9777-RST-10

**In EXCLUDE mode, the router's state is written EXCLUDE(X,Y), where X is the Requested List and Y is the Exclude List.**

> "When the router is in EXCLUDE mode, the router state is represented
>    through the notation EXCLUDE (X,Y), where X is called the "Requested
>    List" and Y is called the "Exclude List"." — §7.2.1, `rfc9777.txt:1872-1874`

- Strength: description. Class: internal.
- Check idea: inspect a router's EXCLUDE-mode record for an address and confirm it separately exposes a Requested List and an Exclude List.

### RFC9777-RST-11

**In EXCLUDE mode, every source except those in the Exclude List is forwarded by the router.**

> "All sources, except those
>    from the Exclude List, will be forwarded by the router." — §7.2.1, `rfc9777.txt:1874-1875`

- Strength: description. Class: end-to-end.
- Check idea: send traffic from a source in the Exclude List and from a source not in it; confirm the router forwards only the second.

### RFC9777-RST-12

**The Requested List has no effect on forwarding, but the router keeps maintaining it.**

> "The
>    Requested List has no effect on forwarding.  Nevertheless, it has to
>    be maintained for several reasons" — §7.2.1, `rfc9777.txt:1875-1877`

- Strength: description. Class: internal.
- Check idea: add or remove a source from the Requested List without touching the Exclude List; confirm the set of forwarded traffic does not change.

### RFC9777-RST-13

**The Filter Timer is used only in EXCLUDE mode; it represents the time left for the Router Filter Mode to expire and switch to INCLUDE.**

> "The Filter Timer is only used when the router is in EXCLUDE mode for
>    a specific multicast address, and it represents the time for the
>    Router Filter Mode of the multicast address to expire and switch to
>    INCLUDE mode." — §7.2.2, `rfc9777.txt:1885-1888`

- Strength: description. Class: internal.
- Check idea: inspect a router's record in INCLUDE mode and confirm no active Filter Timer; switch it to EXCLUDE mode and confirm a Filter Timer is now running.

### RFC9777-RST-14

**A Filter Timer is a decrementing timer with a lower bound of zero, and one exists per Multicast Address Record.**

> "A Filter Timer is a decrementing timer with a lower
>    bound of zero.  One Filter Timer exists per Multicast Address Record." — §7.2.2, `rfc9777.txt:1888-1889`

- Strength: description. Class: internal.
- Check idea: watch a router's Filter Timer for an address count down and confirm it never goes below zero, and that a second listened address has its own independent timer.

### RFC9777-RST-15

**When a Filter Timer expires while the Router Filter Mode is EXCLUDE, the router treats this as no more EXCLUDE-mode listeners and switches to INCLUDE mode.**

> "If a Filter Timer expires, with the Router Filter Mode for that
>    multicast address being EXCLUDE, it means that there are no more
>    listeners in EXCLUDE mode on the attached link.  At this point, the
>    router transitions to INCLUDE filter-mode." — §7.2.2, `rfc9777.txt:1893-1897`

- Strength: description. Class: internal.
- Check idea: let an address's Filter Timer run out to zero while the Router Filter Mode is EXCLUDE; confirm the Router Filter Mode becomes INCLUDE.

### RFC9777-RST-16

**Table 5, row INCLUDE: the Filter Timer is not used, meaning all listeners are in INCLUDE mode.**

> "| Router  | Filter | Actions/Comments                               |" and "| Filter  | Timer  |                                                |" and "| Mode    | Value  |                                                |" — `rfc9777.txt:1904-1906` and "| INCLUDE | Not    | All listeners in INCLUDE mode.                 |" and "|         | Used   |                                                |" — `rfc9777.txt:1908-1909`

- Strength: description. Class: internal.
- Check idea: inspect a router in INCLUDE mode for an address and confirm no Filter Timer is running for it.

### RFC9777-RST-17

**Table 5, row EXCLUDE with Timer > 0: at least one listener is still in EXCLUDE mode.**

> "| EXCLUDE | Timer  | At least one listener in EXCLUDE mode.         |" and "|         | > 0    |                                                |" — `rfc9777.txt:1911-1912`

- Strength: description. Class: internal.
- Check idea: keep one listener in EXCLUDE mode for an address; confirm the router's Filter Timer for it stays above zero as long as that listener is present.

### RFC9777-RST-18

**Table 5, row EXCLUDE with Timer == 0: no more EXCLUDE-mode listeners remain; the record is deleted if the Requested List is empty, otherwise the router switches to INCLUDE mode moving the Requested List into the Include List and deleting the Exclude List.**

> "| EXCLUDE | Timer  | No more listeners in EXCLUDE mode for          |" and "|         | == 0   | the multicast address.  If the Requested       |" and "|        |        | List is empty, delete Multicast Address        |" and "|        |        | Record.  If not, switch to INCLUDE             |" and "|        |        | filter-mode; the sources in the                |" and "|        |        | Requested List are moved to the Include        |" and "|        |        | List, and the Exclude List is deleted.         |" — `rfc9777.txt:1914-1920`

- Strength: description. Class: internal.
- Check idea: let the Filter Timer reach zero once with an empty Requested List and once with a non-empty one; confirm the record is deleted in the first case and switched to INCLUDE(Requested List) in the second.

### RFC9777-RST-19

**A Source Timer is a decrementing timer with a lower bound of zero, and one exists per source record.**

> "A Source Timer is a decrementing timer with a lower bound of zero.
>    One Source Timer is kept per source record." — §7.2.3, `rfc9777.txt:1927-1929`

- Strength: description. Class: internal.
- Check idea: watch a router's source timer for a listened source count down and confirm it never goes below zero, and that another source has its own independent timer.

### RFC9777-RST-20

**MALI (Multicast Address Listening Interval) is the time in which multicast address listening state times out.**

> "The variable MALI
>    stands for the Multicast Address Listening Interval, which is the
>    time in which multicast address listening state will time out." — §7.2.3, `rfc9777.txt:1934-1936`

- Strength: description. Class: internal.
- Check idea: stop refreshing a listened address and confirm its state times out after MALI.

### RFC9777-RST-21

**LLQT (Last Listener Query Time) is the total time the router waits for a report after the Querier sends its first specific query.**

> "The
>    variable LLQT is the Last Listener Query Time, which is the total
>    time the router should wait for a report, after the Querier has sent
>    the first query." — §7.2.3, `rfc9777.txt:1936-1939`

- Strength: should (lower case). Class: internal.
- Check idea: send a specific Query and measure the time until the router prunes if no report arrives; confirm it equals LLQT.

### RFC9777-RST-22

**During LLQT, the Querier should send [Last Member Query Count]-1 further retransmissions of the query.**

> "During this time, the Querier should send [Last
>    Member Query Count]-1 retransmissions of the query." — §7.2.3, `rfc9777.txt:1939-1941`

- Strength: should (lower case). Class: wire.
- Check idea: send a specific Query and count the retransmissions on the link during LLQT; confirm there are [Last Member Query Count]-1 of them.

### RFC9777-RST-23

**In INCLUDE mode, a source is added to the Include List when a listener in INCLUDE mode reports it in a Current-State or State-Change Report.**

> "If the router is in INCLUDE filter-mode, a source can be added to the
>    current Include List if a listener in INCLUDE mode sends a Current-
>    State or a State-Change Report that includes that source." — §7.2.3, `rfc9777.txt:1945-1948`

- Strength: description. Class: internal.
- Check idea: send a Current-State (or State-Change) Report from an INCLUDE-mode listener naming a new source; confirm the router adds it to the Include List.

### RFC9777-RST-24

**Each Include-List source's Source Timer is refreshed whenever an INCLUDE-mode listener's report confirms interest in that source.**

> "Each
>    source from the Include List is associated with a Source Timer that
>    is updated whenever a listener in INCLUDE mode sends a report that
>    confirms its interest in that specific source." — §7.2.3, `rfc9777.txt:1947-1950`

- Strength: description. Class: internal.
- Check idea: repeatedly report interest in the same INCLUDE-mode source and confirm its Source Timer resets each time.

### RFC9777-RST-25

**When an Include-List source's timer expires, the source is deleted from the Include List, and the record itself is deleted once no source records remain.**

> "If the timer of a
>    source from the Include List expires, the source is deleted from the
>    Include List.  If there are no more source records left, the
>    Multicast Address Record is deleted from the router." — §7.2.3, `rfc9777.txt:1950-1953`

- Strength: description. Class: internal.
- Check idea: stop reporting interest in an INCLUDE-mode source and let its timer expire; confirm the source is removed, and, if it was the last one, that the whole record disappears.

### RFC9777-RST-26

**Fast leave: when an INCLUDE-mode node stops wanting a source, every router on the link lowers its timer for that source to LLQT.**

> "When
>    a node in INCLUDE mode expresses its desire to stop listening to a
>    specific source, all the multicast routers on the link lower their
>    timer for that source to a small interval of LLQT milliseconds." — §7.2.3, `rfc9777.txt:1956-1959`

- Strength: description. Class: internal.
- Check idea: have an INCLUDE-mode listener report it no longer wants a source; confirm every router on the link lowers that source's timer to LLQT.

### RFC9777-RST-27

**The Querier then sends a Multicast Address and Source Specific Query to check for other listeners of that source.**

> "The
>    Querier then sends then a Multicast Address and Source Specific
>    Query, to verify whether there are other listeners for that source on
>    the link, or not." — §7.2.3, `rfc9777.txt:1959-1962`

- Strength: description. Class: wire.
- Check idea: trigger fast leave for a source and confirm a Multicast Address and Source Specific Query for it appears on the link.

### RFC9777-RST-28

**If a confirming report is received before the lowered timer expires, every router on the link updates its source timer.**

> "If a corresponding report is received before the
>    timer expires, all the multicast routers on the link update their
>    source timer." — §7.2.3, `rfc9777.txt:1962-1964`

- Strength: description. Class: internal.
- Check idea: answer the fast-leave query from another listener before LLQT elapses; confirm the source timer is refreshed on every router of the link.

### RFC9777-RST-29

**If no such report arrives in time, the source is deleted from the Include List.**

> "If not, the source is deleted from the Include List." — §7.2.3, `rfc9777.txt:1964`

- Strength: description. Class: internal.
- Check idea: let LLQT elapse after fast leave with no other listener answering; confirm the source is removed from the Include List.

### RFC9777-RST-30

**In EXCLUDE mode, Requested-List sources keep running source timers and are forwarded; Exclude-List sources have their timer held at zero and are blocked.**

> "For sources from the Requested List,
>    the source timers have running values; these sources are forwarded by
>    the router.  For sources from the Exclude List, the source timers are
>    set to zero; these sources are blocked by the router." — §7.2.3, `rfc9777.txt:1969-1972`

- Strength: description. Class: internal.
- Check idea: inspect an EXCLUDE-mode record and confirm Requested-List sources carry a positive timer and are forwarded, while Exclude-List sources carry timer zero and are blocked.

### RFC9777-RST-31

**When a Requested-List source's timer expires, it is moved to the Exclude List, and the router informs the routing protocol that no listener wants that source.**

> "If the timer
>    of a source from the Requested List expires, the source is moved to
>    the Exclude List.  Then, the router informs the routing protocol that
>    there is no longer a listener on the link interested in traffic from
>    this source." — §7.2.3, `rfc9777.txt:1972-1976`

- Strength: description. Class: internal.
- Check idea: let a Requested-List source's timer expire with no report renewing it; confirm the source moves to the Exclude List and the routing protocol is told to stop forwarding it.

### RFC9777-RST-32

**When the router switches to INCLUDE mode, the Requested List's sources move to the Include List and the Exclude List is deleted.**

> "When the router switches to INCLUDE mode, the sources in the
>        Requested List are moved to the Include List, and the Exclude
>        List is deleted." — §7.2.3, `rfc9777.txt:1989-1991`

- Strength: description. Class: internal.
- Check idea: let an EXCLUDE-mode record's Filter Timer expire and switch to INCLUDE; confirm the resulting Include List equals the former Requested List and no Exclude List remains.

### RFC9777-RST-33

**Fast blocking: sources named in a report requesting them no longer be forwarded are added to the Requested List.**

> "If the
>        router receives a report that contains such a request, the
>        concerned sources are added to the Requested List." — §7.2.3, `rfc9777.txt:2007-2009`

- Strength: description. Class: internal.
- Check idea: send the router a report asking to block sources not currently in the Requested List; confirm they are added to it.

### RFC9777-RST-34

**Fast blocking: their timers are set to LLQT, and the Querier sends a Multicast Address and Source Specific Query for them.**

> "Their timers
>        are set to a small interval of LLQT milliseconds, and a Multicast
>        Address and Source Specific Query is sent by the Querier, to
>        check whether there are nodes on the link still interested in
>        those sources, or not." — §7.2.3, `rfc9777.txt:2009-2013`

- Strength: description. Class: internal.
- Check idea: trigger fast blocking for a source and confirm its timer drops to LLQT and a matching specific Query appears on the link.

### RFC9777-RST-35

**Fast blocking: if no node confirms interest before the timer expires, the source moves from the Requested List to the Exclude List and is then blocked.**

> "If no node confirms its interest in
>        receiving a specific source, the timer of that source expires.
>        Then, the source is moved from the Requested List to the Exclude
>        List.  From then on, the source will be blocked by the router." — §7.2.3, `rfc9777.txt:2013-2016`

- Strength: description. Class: end-to-end.
- Check idea: let LLQT elapse after fast blocking with no listener confirming interest; confirm the source moves to the Exclude List and traffic from it is then blocked.

### RFC9777-RST-36

**In EXCLUDE mode, source records are deleted only when the Filter Timer expires or when a newly received Multicast Address Record modifies the source-record list.**

> "When the Router Filter Mode for a multicast address is EXCLUDE,
>    source records are only deleted when the Filter Timer expires or when
>    newly received Multicast Address Records modify the source record
>    list of the router." — §7.2.3, `rfc9777.txt:2021-2024`

- Strength: description. Class: internal.
- Check idea: in EXCLUDE mode, let time pass with no Filter Timer expiry and no new report; confirm no source record disappears on its own.

## Source-specific forwarding rules

### RFC9777-FWD-1

**The multicast routing protocol should use MLDv2 information so every source/address combination with listeners on a link is forwarded to that link.**

> "The multicast
>    routing protocol in use is in charge of this decision and should use
>    the MLDv2 information to ensure that all sources/multicast addresses
>    that have listeners on a link are forwarded to that link." — §7.3, `rfc9777.txt:2030-2033`

- Strength: should (lower case). Class: end-to-end.
- Check idea: have a listener request a source/address pair; confirm traffic for that pair is forwarded to the listener's link.

### RFC9777-FWD-2

**MLDv2 information does not override multicast routing information: for example, a router may still forward EXCLUDE-mode-excluded sources onto a transit link.**

> "MLDv2
>    information does not override multicast routing information; for
>    example, if the MLDv2 filter-mode for a multicast address is EXCLUDE,
>    a router may still forward packets for excluded sources to a transit
>    link." — §7.3, `rfc9777.txt:2033-2037`

- Strength: may (lower case). Class: end-to-end.
- Check idea: on a transit link with EXCLUDE-mode state excluding a source, check that a router forwarding that source there for routing-protocol reasons does not contradict the specification.

### RFC9777-FWD-3

**Table 6, row INCLUDE with Source Timer > 0: MLD suggests forwarding traffic from that source.**

> "| Router  | Source  | Action                                  |" and "| Filter  | Timer   |                                         |" and "| Mode    | Value   |                                         |" — `rfc9777.txt:2046-2048` and "| INCLUDE | TIMER > | Suggest to forward traffic from source. |" and "|         | 0       |                                         |" — `rfc9777.txt:2050-2051`

- Strength: description. Class: end-to-end.
- Check idea: with a router in INCLUDE mode and a source's timer still running, confirm traffic from that source is forwarded to the link.

### RFC9777-FWD-4

**Table 6, row INCLUDE with Source Timer == 0: MLD suggests stopping forwarding and removing the source record, deleting the Multicast Address Record if no source records remain.**

> "| INCLUDE | TIMER   | Suggest to stop forwarding traffic from |" and "|         | == 0    | source and remove the source record.    |" and "|        |         | If there are no more source records,    |" and "|        |         | delete the Multicast Address Record.    |" — `rfc9777.txt:2053-2056`

- Strength: description. Class: end-to-end.
- Check idea: let an INCLUDE-mode source's timer reach zero; confirm forwarding of that source stops and, if it was the last source, the address record is deleted.

### RFC9777-FWD-5

**Table 6, row INCLUDE with no source element: MLD suggests not forwarding traffic from that source.**

> "| INCLUDE | No      | Suggest to not forward traffic from     |" and "|         | Source  | source.                                 |" — `rfc9777.txt:2058-2059`

- Strength: description. Class: end-to-end.
- Check idea: send traffic in INCLUDE mode from a source with no record at all on the router; confirm it is not forwarded.

### RFC9777-FWD-6

**Table 6, row EXCLUDE with Source Timer > 0: MLD suggests forwarding traffic from that source.**

> "| EXCLUDE | TIMER > | Suggest to forward traffic from source. |" and "|         | 0       |                                         |" — `rfc9777.txt:2062-2063`

- Strength: description. Class: end-to-end.
- Check idea: with a router in EXCLUDE mode and a Requested-List source's timer still running, confirm traffic from that source is forwarded.

### RFC9777-FWD-7

**Table 6, row EXCLUDE with Source Timer == 0: MLD suggests not forwarding that source, and moves it from the Requested List to the Exclude List without removing the source record.**

> "| EXCLUDE | TIMER   | Suggest to not forward traffic from     |" and "|         | == 0    | source.  Move the source from the       |" and "|        |         | Requested List to the Exclude List (DO |" and "|        |         | NOT remove the source record).          |" — `rfc9777.txt:2065-2068`

- Strength: description. Class: end-to-end.
- Check idea: let a Requested-List source's timer reach zero; confirm forwarding of that source stops and the source record moves to the Exclude List rather than being deleted.

### RFC9777-FWD-8

**Table 6, row EXCLUDE with no source element: MLD suggests forwarding traffic from all sources.**

> "| EXCLUDE | No      | Suggest to forward traffic from all     |" and "|         | Source  | sources.                                |" — `rfc9777.txt:2070-2071`

- Strength: description. Class: end-to-end.
- Check idea: send traffic in EXCLUDE mode from a source with no record on the router at all; confirm it is forwarded.

## Router: reception of reports

### RFC9777-RREP-1

**A router drops a received Report unless its source address is a valid link-local address, its Hop Limit is 1, and the Router Alert option is present, and only then starts processing it.**

> "Upon reception of an MLD message that contains a Report, the router
>    checks if the source address of the message is a valid link-local
>    address, if the Hop Limit is set to 1, and if the Router Alert option
>    is present in the Hop-by-Hop Options header of the IPv6 packet.  If
>    any of these checks fail, the packet is dropped.  If the validity of
>    the MLD message is verified, the router starts to process the Report." — §7.4, `rfc9777.txt:2079-2084`

- Strength: description. Class: end-to-end.
- Check idea: send Reports that each fail one of the three checks, and one valid Report; confirm the router acts only on the valid one.

### RFC9777-RREP-2

**An SSM-aware router should ignore records for SSM-range addresses whose type is MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE.**

> "SSM-aware routers SHOULD ignore records that contain multicast
>    addresses in the SSM address range if the record type is
>    MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE." — §7.4, `rfc9777.txt:2086-2088`

- Strength: should. Class: end-to-end.
- Check idea: send an SSM-range address a MODE_IS_EXCLUDE and a CHANGE_TO_EXCLUDE_MODE record on an SSM-aware router; confirm neither changes the router's forwarding state.

### RFC9777-RREP-3

**An SSM-aware router should ignore MLDv1 Report/Done messages for SSM-range addresses, should not use them to build forwarding state, and may log an error on receiving one.**

> "SSM-aware routers SHOULD
>    ignore MLDv1 Report and DONE messages that contain multicast
>    addresses in the SSM address range, SHOULD NOT use such Reports to
>    establish IP forwarding state, and MAY log an error if it receives
>    such a message." — §7.4, `rfc9777.txt:2088-2092`

- Strength: should, should not, may. Class: end-to-end.
- Check idea: send an SSM-aware router an MLDv1 Report for an SSM-range address; confirm no forwarding state results and, optionally, that an error is logged.

### RFC9777-RREP-4

**A node sends a Source-List-Change Record or a Filter-Mode-Change Record on a global-state change, and routers must act on these records, possibly changing their own state.**

> "When a change in the global state of a multicast address occurs in a
>    node, the node sends either a Source-List-Change Record or a Filter
>    Mode Change Record for that multicast address.  As with Current-State
>    Records, routers must act upon these records and possibly change
>    their own state to reflect the new listening state of the link." — §7.4.2, `rfc9777.txt:2148-2152`

- Strength: must (lower case). Class: internal.
- Check idea: send a router a Source-List-Change Record and a Filter-Mode-Change Record in turn; confirm each changes the router's recorded state for the link.

### RFC9777-RREP-5

**The Querier must query sources or multicast addresses that a report requests be no longer forwarded.**

> "The Querier must query sources or multicast addresses that are
>    requested to be no longer forwarded." — §7.4.2, `rfc9777.txt:2154-2155`

- Strength: must (lower case). Class: wire.
- Check idea: send a report requesting to stop forwarding a source or address; confirm the Querier sends a specific Query for it before removing it.

### RFC9777-RREP-6

**Querying or receiving a query for specific sources lowers the router's source timers for those sources to LLQT.**

> "When a router queries or
>    receives a query for a specific set of sources, it lowers its source
>    timers for those sources to a small interval of Last Listener Query
>    Time milliseconds." — §7.4.2, `rfc9777.txt:2155-2158`

- Strength: description. Class: internal.
- Check idea: send or receive a Multicast Address and Source Specific Query for given sources; confirm the router's timers for those sources drop to LLQT.

### RFC9777-RREP-7

**Multicast Address Records received in response to a query that confirm interest in the queried sources update the corresponding timers.**

> "If Multicast Address Records are received in
>    response to the queries that express interest in listening to the
>    queried sources, the corresponding timers are updated." — §7.4.2, `rfc9777.txt:2158-2160`

- Strength: description. Class: internal.
- Check idea: answer a specific Query with a record confirming interest in the queried sources; confirm the router's timers for those sources are refreshed.

### RFC9777-RREP-8

**A fast EXCLUDE-to-INCLUDE transition lowers the address's Filter Timer to LLQT.**

> "The Filter Timer for that multicast address is lowered to a small
>    interval of Last Listener Query Time milliseconds." — §7.4.2, `rfc9777.txt:2164-2166`

- Strength: description. Class: internal.
- Check idea: trigger a fast EXCLUDE-to-INCLUDE transition for an address and confirm its Filter Timer drops to LLQT.

### RFC9777-RREP-9

**If an EXCLUDE-mode record for the address is received within that interval, the Filter Timer is updated and the forwarding suggestion continues uninterrupted.**

> "If any Multicast
>    Address Records that express EXCLUDE mode interest in the multicast
>    address are received within this interval, the Filter Timer is
>    updated and the suggestion to the routing protocol to forward the
>    multicast address stands without any interruption." — §7.4.2, `rfc9777.txt:2166-2170`

- Strength: description. Class: end-to-end.
- Check idea: during the lowered-Filter-Timer interval, send an EXCLUDE-mode record for the address; confirm the timer is refreshed and forwarding is not interrupted.

### RFC9777-RREP-10

**If no such record arrives, the router switches to INCLUDE filter-mode for that address.**

> "If not, the
>    router will switch to INCLUDE filter-mode for that multicast address." — §7.4.2, `rfc9777.txt:2170-2171`

- Strength: description. Class: internal.
- Check idea: let the lowered-Filter-Timer interval elapse with no EXCLUDE-mode record received; confirm the router switches to INCLUDE mode for the address.

### RFC9777-RREP-11

**During the query period, the router keeps suggesting forwarding for the queried multicast addresses or sources.**

> "During the query period (i.e., Last Listener Query Time
>    milliseconds), the MLD component in the router continues to suggest
>    to the routing protocol to forward traffic from the multicast
>    addresses or sources that are queried." — §7.4.2, `rfc9777.txt:2173-2176`

- Strength: description. Class: end-to-end.
- Check idea: send a specific Query and check forwarding for the queried address or source during LLQT; confirm it continues unchanged until the timer expires.

### RFC9777-RREP-12

**Only after LLQT, and without a record expressing renewed interest, may the router prune the queried address or sources from the link.**

> "It is not until after Last
>    Listener Query Time milliseconds, and without receiving a record that
>    expresses interest in the queried multicast address or sources, that
>    the router may prune the multicast address or sources from the link." — §7.4.2, `rfc9777.txt:2176-2179`

- Strength: may (lower case). Class: end-to-end.
- Check idea: let LLQT elapse with no renewed-interest record; confirm pruning happens no earlier than that, and not at all if a renewal arrived first.

### RFC9777-RREP-13

**When the computed source-list for a Table 8 action is empty, no query is sent for that action.**

> "If source-list A is null as a
>    result of the action (e.g. A*B), then no query is sent as a result of
>    the operation." — §7.4.2, `rfc9777.txt:2190-2192`

- Strength: description. Class: wire.
- Check idea: drive a Table 8 case whose computed query source-list works out empty; confirm no Query Message for it appears on the link.

### RFC9777-RREP-14

**Table 8 queries are transmitted [Last Listener Query Count] times, once every [Last Listener Query Interval].**

> "In order to maintain protocol robustness, queries defined in the
>    Actions column of Table 8 need to be transmitted [Last Listener Query
>    Count] times, once every [Last Listener Query Interval] period." — §7.4.2, `rfc9777.txt:2194-2196`

- Strength: description. Class: wire.
- Check idea: trigger a Table 8 query action and count the copies sent and their spacing; confirm [Last Listener Query Count] copies spaced by [Last Listener Query Interval].

### RFC9777-RREP-15

**Pending retransmitted queries for the same multicast address are merged with newly scheduled ones.**

> "If while scheduling new queries there are already pending queries to
>    be retransmitted for the same multicast address, the new and pending
>    queries have to be merged." — §7.4.2, `rfc9777.txt:2198-2200`

- Strength: description. Class: internal.
- Check idea: schedule a new specific query for an address that already has a pending retransmitted query; confirm the two are merged into one series rather than sent independently.

### RFC9777-RREP-16

**Table 7, row INCLUDE(A) receiving IS_IN(B): new state INCLUDE(A+B); action (B)=MALI.**

> "| Router  | Report   | New     | Actions              |" and "| State   | Received | Router  |                      |" and "|        |          | State   |                      |" — `rfc9777.txt:2118-2120` and "| INCLUDE | IS_IN    | INCLUDE | (B)=MALI             |" and "| (A)     | (B)      | (A+B)   |                      |" — `rfc9777.txt:2122-2123`

- Strength: description. Class: internal.
- Check idea: with a router in INCLUDE(A), send an IS_IN(B) Current-State Record; confirm the state becomes INCLUDE(A+B) and every source in B has its timer set to MALI.

### RFC9777-RREP-17

**Table 7, row INCLUDE(A) receiving IS_EX(B): new state EXCLUDE(A*B,B-A); actions (B-A)=0, Delete(A-B), Filter Timer=MALI.**

> "| INCLUDE | IS_EX    | EXCLUDE | (B-A)=0              |" and "| (A)     | (B)      | (A*B,   |                      |" and "|        |          | B-A)    | Delete (A-B)         |" and "|        |          |         |                      |" and "|        |          |         | Filter Timer=MALI    |" — `rfc9777.txt:2125-2129`

- Strength: description. Class: internal.
- Check idea: with a router in INCLUDE(A), send an IS_EX(B) Current-State Record; confirm the resulting state is EXCLUDE(A*B, B-A), sources B-A carry timer zero, sources A-B are deleted, and the Filter Timer is set to MALI.

### RFC9777-RREP-18

**Table 7, row EXCLUDE(X,Y) receiving IS_IN(A): new state EXCLUDE(X+A,Y-A); action (A)=MALI.**

> "| EXCLUDE | IS_IN    | EXCLUDE | (A)=MALI             |" and "| (X,Y)   | (A)      | (X+A,   |                      |" and "|        |          | Y-A)    |                      |" — `rfc9777.txt:2131-2133`

- Strength: description. Class: internal.
- Check idea: with a router in EXCLUDE(X,Y), send an IS_IN(A) Current-State Record; confirm the resulting state is EXCLUDE(X+A, Y-A) and sources in A have their timer set to MALI.

### RFC9777-RREP-19

**Table 7, row EXCLUDE(X,Y) receiving IS_EX(A): new state EXCLUDE(A-Y,Y*A); actions (A-X-Y)=MALI, Delete(X-A), Delete(Y-A), Filter Timer=MALI.**

> "| EXCLUDE | IS_EX    | EXCLUDE | (A-X-Y)=MALI         |" and "| (X,Y)   | (A)      | (A-Y,   |                      |" and "|        |          | Y*A)    | Delete (X-A)         |" and "|        |          |         |                      |" and "|        |          |         | Delete (Y-A)         |" and "|        |          |         |                      |" and "|        |          |         | Filter Timer=MALI    |" — `rfc9777.txt:2135-2141`

- Strength: description. Class: internal.
- Check idea: with a router in EXCLUDE(X,Y), send an IS_EX(A) Current-State Record; confirm the resulting state is EXCLUDE(A-Y, Y*A), sources in A-X-Y get timer MALI, sources X-A and Y-A are deleted, and the Filter Timer is set to MALI.

### RFC9777-RREP-20

**Table 8, row INCLUDE(A) receiving ALLOW(B): new state INCLUDE(A+B); action (B)=MALI.**

> "| Router  | Report   | New Router State   | Actions                |" and "| State   | Received |                    |                        |" — `rfc9777.txt:2206-2207` and "| INCLUDE | ALLOW    | INCLUDE(A+B)       | (B)=MALI               |" — `rfc9777.txt:2209`

- Strength: description. Class: internal.
- Check idea: with a router in INCLUDE(A), send an ALLOW(B) Source-List-Change Record; confirm the state becomes INCLUDE(A+B) and sources in B get timer MALI.

### RFC9777-RREP-21

**Table 8, row INCLUDE(A) receiving BLOCK(B): state stays INCLUDE(A); action Send Q(MA,A*B).**

> "| INCLUDE | BLOCK    | INCLUDE(A)         | Send Q(MA,A*B)         |" — `rfc9777.txt:2212`

- Strength: description. Class: wire.
- Check idea: with a router in INCLUDE(A), send a BLOCK(B) Source-List-Change Record; confirm the state is unchanged and a Multicast Address and Source Specific Query is sent for A*B.

### RFC9777-RREP-22

**Table 8, row INCLUDE(A) receiving TO_EX(B): new state EXCLUDE(A*B,B-A); actions (B-A)=0, Delete(A-B), Send Q(MA,A*B), Filter Timer=MALI.**

> "| INCLUDE | TO_EX    | EXCLUDE(A*B,B-A)   | (B-A)=0, Delete (A-B), |" and "| (A)     | (B)      |                    | Send Q(MA,A*B), Filter |" and "|        |          |                    | Timer=MALI             |" — `rfc9777.txt:2215-2217`

- Strength: description. Class: wire.
- Check idea: with a router in INCLUDE(A), send a TO_EX(B) Filter-Mode-Change Record; confirm the resulting state is EXCLUDE(A*B, B-A), sources B-A get timer zero, sources A-B are deleted, a Query for A*B is sent, and the Filter Timer is set to MALI.

### RFC9777-RREP-23

**Table 8, row INCLUDE(A) receiving TO_IN(B): new state INCLUDE(A+B); actions (B)=MALI, Send Q(MA,A-B).**

> "| INCLUDE | TO_IN    | INCLUDE(A+B)       | (B)=MALI, Send         |" and "| (A)     | (B)      |                    | Q(MA,A-B)              |" — `rfc9777.txt:2219-2220`

- Strength: description. Class: wire.
- Check idea: with a router in INCLUDE(A), send a TO_IN(B) Filter-Mode-Change Record; confirm the resulting state is INCLUDE(A+B), sources in B get timer MALI, and a Query for A-B is sent.

### RFC9777-RREP-24

**Table 8, row EXCLUDE(X,Y) receiving ALLOW(A): new state EXCLUDE(X+A,Y-A); action (A)=MALI.**

> "| EXCLUDE | ALLOW    | EXCLUDE(X+A,Y-A)   | (A)=MALI               |" — `rfc9777.txt:2222`

- Strength: description. Class: internal.
- Check idea: with a router in EXCLUDE(X,Y), send an ALLOW(A) Source-List-Change Record; confirm the resulting state is EXCLUDE(X+A, Y-A) and sources in A get timer MALI.

### RFC9777-RREP-25

**Table 8, row EXCLUDE(X,Y) receiving BLOCK(A): new state EXCLUDE(X+(A-Y),Y); actions (A-X-Y)=Filter Timer, Send Q(MA,A-Y).**

> "| EXCLUDE | BLOCK    | EXCLUDE(X+(A-Y),Y) | (A-X-Y)=Filter Timer,  |" and "| (X,Y)   | (A)      |                    | Send Q(MA,A-Y)         |" — `rfc9777.txt:2225-2226`

- Strength: description. Class: wire.
- Check idea: with a router in EXCLUDE(X,Y), send a BLOCK(A) Source-List-Change Record; confirm the resulting state is EXCLUDE(X+(A-Y), Y), sources in A-X-Y get the Filter Timer's value, and a Query for A-Y is sent.

### RFC9777-RREP-26

**Table 8, row EXCLUDE(X,Y) receiving TO_EX(A): new state EXCLUDE(A-Y,Y*A); actions (A-X-Y)=Filter Timer, Delete(X-A), Delete(Y-A), Send Q(MA,A-Y), Filter Timer=MALI.**

> "| EXCLUDE | TO_EX    | EXCLUDE(A-Y,Y*A)   | (A-X-Y)=Filter Timer,  |" and "| (X,Y)   | (A)      |                    | Delete (X-A), Delete   |" and "|        |          |                    | (Y-A), Send Q(MA,A-Y), |" and "|        |          |                    | Filter Timer=MALI      |" — `rfc9777.txt:2228-2231`

- Strength: description. Class: wire.
- Check idea: with a router in EXCLUDE(X,Y), send a TO_EX(A) Filter-Mode-Change Record; confirm the resulting state is EXCLUDE(A-Y, Y*A), sources in A-X-Y get the Filter Timer's value, X-A and Y-A are deleted, a Query for A-Y is sent, and the Filter Timer is set to MALI.

### RFC9777-RREP-27

**Table 8, row EXCLUDE(X,Y) receiving TO_IN(A): new state EXCLUDE(X+A,Y-A); actions (A)=MALI, Send Q(MA,X-A), Send Q(MA).**

> "| EXCLUDE | TO_IN    | EXCLUDE(X+A,Y-A)   | (A)=MALI, Send         |" and "| (X,Y)   | (A)      |                    | Q(MA,X-A), Send Q(MA)  |" — `rfc9777.txt:2233-2234`

- Strength: description. Class: wire.
- Check idea: with a router in EXCLUDE(X,Y), send a TO_IN(A) Filter-Mode-Change Record; confirm the resulting state is EXCLUDE(X+A, Y-A), sources in A get timer MALI, a Query for X-A is sent, and a Multicast Address Specific Query for the address itself is also sent.

## Switching router filter modes

### RFC9777-RSW-1

**When a Filter Timer expires while the Router Filter Mode is EXCLUDE, the router assumes no EXCLUDE-mode nodes remain on the link and transitions to INCLUDE filter-mode.**

> "When a Filter Timer expires with a Router Filter Mode of EXCLUDE, a
>    router assumes that there are no nodes with a filter-mode of EXCLUDE
>    present on the attached link.  Thus, the router transitions to
>    INCLUDE filter-mode for the multicast address." — §7.5, `rfc9777.txt:2244-2247`

- Strength: description. Class: internal.
- Check idea: let an EXCLUDE-mode address's Filter Timer run to zero with no EXCLUDE-mode listener left; confirm the router's Router Filter Mode becomes INCLUDE.

### RFC9777-RSW-2

**On the switch to INCLUDE, the Requested List's sources move to the Include List and the Exclude List's sources are deleted.**

> "A router uses the sources from the Requested List as its state for
>    the switch to a filter-mode of INCLUDE.  Sources from the Requested
>    List are moved in the Include List, while sources from the Exclude
>    List are deleted." — §7.5, `rfc9777.txt:2249-2252`

- Strength: description. Class: internal.
- Check idea: switch an EXCLUDE(X,Y) record to INCLUDE mode; confirm the resulting Include List is X and no trace of Y's sources remains.

### RFC9777-RSW-3

**If the Requested List is empty at the moment of the switch, the Multicast Address Record is deleted from the router.**

> "If at the moment of the switch the Requested List
>    (X) is empty, the Multicast Address Record is deleted from the
>    router." — §7.5, `rfc9777.txt:2255-2257`

- Strength: description. Class: internal.
- Check idea: let a Filter Timer expire for an EXCLUDE(∅,Y) record; confirm the whole Multicast Address Record disappears rather than becoming an empty INCLUDE record.

## Router: reception of queries, querier election and specific queries

### RFC9777-RQRY-1

**A router drops a received Query unless its source address is a valid link-local address, its Hop Limit is 1, and the Router Alert option is present, and only then processes it.**

> "Upon reception of an MLD message that contains a Query, the router
>    checks if the source address of the message is a valid link-local
>    address, if the Hop Limit is set to 1, and if the Router Alert option
>    is present in the Hop-by-Hop Options header of the IPv6 packet.  If
>    any of these checks fail, the packet is dropped." — §7.6, `rfc9777.txt:2261-2265`, and "If the validity of the MLD message is verified, the router starts to
>    process the Query." — §7.6, `rfc9777.txt:2267-2268`

- Strength: description. Class: end-to-end.
- Check idea: send Queries that each fail one of the three checks, and one valid Query, to a router; confirm it processes only the valid one.

### RFC9777-RQRY-2

**When a router sends or receives a query with a clear S flag, it must update its timers to the correct timeout values for the queried address or sources.**

> "When a router sends or receives a query with a clear S
>    flag, it must update its timers to reflect the correct timeout values
>    for the multicast address or sources being queried." — §7.6.1, `rfc9777.txt:2273-2276`

- Strength: must (lower case). Class: internal.
- Check idea: send and receive an S-flag-clear specific Query for an address/source; confirm the router's corresponding timers change per Table 9.

### RFC9777-RQRY-3

**Table 9, row Q(MA,A): Source Timers for sources in A are lowered to LLQT.**

> "| Query   | Action                                              |" — `rfc9777.txt:2281` and "| Q(MA,A) | Source Timers for sources in A are lowered to LLQT. |" — `rfc9777.txt:2283`

- Strength: description. Class: internal.
- Check idea: send or receive a Q(MA,A) query with the S flag clear; confirm the Source Timers of the sources in A drop to LLQT.

### RFC9777-RQRY-4

**Table 9, row Q(MA): the Filter Timer is lowered to LLQT.**

> "| Query   | Action                                              |" — `rfc9777.txt:2281` and "| Q(MA)   | The Filter Timer is lowered to LLQT.                |" — `rfc9777.txt:2285`

- Strength: description. Class: internal.
- Check idea: send or receive a Q(MA) query with the S flag clear; confirm the address's Filter Timer drops to LLQT.

### RFC9777-RQRY-5

**A router sending or receiving a query with the S flag set does not update its timers.**

> "When a router sends or receives a query with the S flag set, it will
>    not update its timers." — §7.6.1, `rfc9777.txt:2290-2291`

- Strength: description. Class: internal.
- Check idea: send or receive an S-flag-set specific Query for an address/source; confirm the corresponding timers stay unchanged.

### RFC9777-RQRY-6

**MLDv2 elects one router per subnet into Querier state; every other router on the subnet should be in Non-Querier state.**

> "MLDv2 elects a single router per subnet to be in Querier state; all
>    the other routers on the subnet should be in Non-Querier state." — §7.6.2, `rfc9777.txt:2295-2297`

- Strength: should (lower case). Class: internal.
- Check idea: put several routers on one subnet; confirm exactly one ends up in Querier state and the rest in Non-Querier state.

### RFC9777-RQRY-7

**On starting on a subnet, a router by default considers itself Querier and sends several General Queries separated by a small interval.**

> "When a router starts operating on a subnet, by default
>    it considers itself as being the Querier.  Thus, it sends several
>    General Queries separated by a small time interval" — §7.6.2, `rfc9777.txt:2298-2301`

- Strength: description. Class: wire.
- Check idea: bring up a router on a subnet with no prior Querier traffic; confirm it sends several General Queries spaced by the startup interval.

### RFC9777-RQRY-8

**On receiving a query from a lower IPv6 address, a router arms the Other-Querier-Present Timer and, if it was Querier, switches to Non-Querier state and stops sending queries.**

> "When a router receives a query with a lower IPv6 address than its
>    own, it sets the Other-Querier-Present Timer to [Other Querier
>    Present Interval]; if it was previously in Querier state, it switches
>    to Non- Querier state and ceases to send queries on the link." — §7.6.2, `rfc9777.txt:2303-2306`

- Strength: description. Class: wire.
- Check idea: send a Querier router a query from a lower-addressed router; confirm it arms the Other-Querier-Present Timer, drops to Non-Querier state, and stops sending queries.

### RFC9777-RQRY-9

**After the Other-Querier-Present Timer expires, the router should re-enter Querier state and begin sending General Queries again.**

> "After
>    the Other-Querier-Present Timer expires, it should re-enter the
>    Querier state and begin sending General Queries." — §7.6.2, `rfc9777.txt:2306-2308`

- Strength: should (lower case). Class: wire.
- Check idea: let the Other-Querier-Present Timer expire with no further query from the other router; confirm the router resumes Querier state and General Queries reappear on the link.

### RFC9777-RQRY-10

**All MLDv2 queries must be sent with a fe80::/64 link-local source address.**

> "All MLDv2 queries MUST be sent with the fe80::/64 link-local source
>    address prefix." — §7.6.2, `rfc9777.txt:2310-2311`

- Strength: must. Class: wire.
- Check idea: inspect the source address of every Query Message a router sends; confirm it always falls in fe80::/64.

### RFC9777-RQRY-11

**For querier election, an IPv6 address A is lower than address B if A's last-64-bit interface ID, read big-endian, is numerically lower than B's.**

> "for the purpose of MLDv2
>    querier election, an IPv6 address A is considered to be lower than an IPv6
>    address B if the interface ID represented by the last 64 bits of
>    address A, in big-endian bit order, is lower than the interface ID
>    represented by the last 64 bits of address B." — §7.6.2, `rfc9777.txt:2311-2315`

- Strength: description. Class: internal.
- Check idea: compare two routers' query source addresses whose interface IDs differ only in ordering-sensitive bits; confirm the election picks the one with the numerically lower big-endian interface ID.

### RFC9777-RQRY-12

**On a "Send Q(MA)" table action, the Filter Timer must be lowered to LLQT.**

> "When a table action "Send Q(MA)" is encountered, the Filter Timer
>    must be lowered to LLQT." — §7.6.3.1, `rfc9777.txt:2321-2322`

- Strength: must (lower case). Class: internal.
- Check idea: trigger a table action that reads "Send Q(MA)"; confirm the address's Filter Timer is set to LLQT.

### RFC9777-RQRY-13

**The Querier must then immediately send a Multicast Address Specific Query and schedule [Last Listener Query Count - 1] further retransmissions every [Last Listener Query Interval], over [Last Listener Query Time].**

> "The Querier must then immediately send a
>    Multicast Address Specific Query as well as schedule [Last Listener
>    Query Count - 1] query retransmissions to be sent every [Last
>    Listener Query Interval], over [Last Listener Query Time]." — §7.6.3.1, `rfc9777.txt:2322-2325`

- Strength: must (lower case). Class: wire.
- Check idea: trigger a "Send Q(MA)" action and observe the link; confirm one immediate Multicast Address Specific Query followed by [Last Listener Query Count - 1] further copies spaced by [Last Listener Query Interval].

### RFC9777-RQRY-14

**When the address's Filter Timer is larger than LLQT at transmission time, the outgoing Multicast Address Specific Query sets the Suppress Router-Side Processing bit.**

> "When transmitting a Multicast Address Specific Query, if the Filter
>    Timer is larger than LLQT, the "Suppress Router-Side Processing" bit
>    is set in the Query Message." — §7.6.3.1, `rfc9777.txt:2327-2329`

- Strength: description. Class: wire.
- Check idea: send a Multicast Address Specific Query while the address's Filter Timer is still above LLQT; confirm the Suppress Router-Side Processing bit is set.

### RFC9777-RQRY-15

**On a "Send Q(MA,X)" table action, for each source in X sent to MA with Source Timer above LLQT, the Querier must lower its timer to LLQT, add it to the Retransmission List, and set its Source Retransmission Counter to [Last Listener Query Count].**

> "the following actions must be performed for
>    each of the sources in X that send to multicast address MA, with the
>    Source Timer larger than LLQT:
>
>    *  lower Source Timer to LLQT;
>
>    *  add the sources to the Retransmission List; and
>
>    *  set the Source Retransmission Counter for each source to [Last
>       Listener Query Count]." — §7.6.3.2, `rfc9777.txt:2335-2344`

- Strength: must (lower case). Class: internal.
- Check idea: trigger a "Send Q(MA,X)" action for sources whose timers exceed LLQT; confirm each one's timer drops to LLQT, is added to the Retransmission List, and gets counter [Last Listener Query Count].

### RFC9777-RQRY-16

**The Querier must then immediately send a Multicast Address and Source Specific Query and schedule [Last Listener Query Count - 1] further retransmissions every [Last Listener Query Interval], over [Last Listener Query Time].**

> "The Querier must then immediately send a Multicast Address and Source
>    Specific Query as well as schedule [Last Listener Query Count -1]
>    query retransmissions to be sent every [Last Listener Query
>    Interval], over [Last Listener Query Time]." — §7.6.3.2, `rfc9777.txt:2346-2349`

- Strength: must (lower case). Class: wire.
- Check idea: trigger a "Send Q(MA,X)" action and observe the link; confirm one immediate Multicast Address and Source Specific Query followed by [Last Listener Query Count - 1] further copies spaced by [Last Listener Query Interval].

### RFC9777-RQRY-17

**Building a Multicast Address and Source Specific Query for MA sends two separate Query Messages for that address.**

> "When building a Multicast Address and Source Specific Query for a
>    multicast address MA, two separate Query Messages are sent for the
>    multicast address." — §7.6.3.2, `rfc9777.txt:2352-2354`

- Strength: description. Class: wire.
- Check idea: trigger a Multicast Address and Source Specific Query build for an address; confirm exactly two distinct Query Messages appear for it (subject to RQRY-20's suppression rule).

### RFC9777-RQRY-18

**The first message has the Suppress Router-Side Processing bit set and carries the retransmission-state sources whose timers are still greater than LLQT.**

> "The first one has the "Suppress Router-Side
>    Processing" bit set and contains all the sources with retransmission
>    state (i.e., sources from the Retransmission List of that multicast
>    address) and timers greater than LLQT." — §7.6.3.2, `rfc9777.txt:2354-2357`

- Strength: description. Class: wire.
- Check idea: inspect the first of the two built Query Messages; confirm its Suppress Router-Side Processing bit is set and its source-list holds only Retransmission-List sources with timer above LLQT.

### RFC9777-RQRY-19

**The second message has the Suppress Router-Side Processing bit clear and carries the retransmission-state sources whose timers are at or below LLQT.**

> "The second has the "Suppress
>    Router-Side Processing" bit clear and contains all the sources with
>    retransmission state and timers lower or equal to LLQT." — §7.6.3.2, `rfc9777.txt:2357-2359`

- Strength: description. Class: wire.
- Check idea: inspect the second of the two built Query Messages; confirm its Suppress Router-Side Processing bit is clear and its source-list holds only Retransmission-List sources with timer at or below LLQT.

### RFC9777-RQRY-20

**If either of the two calculated Query Messages holds no sources, its transmission is suppressed.**

> "If either of
>    the two calculated messages does not contain any sources, then its
>    transmission is suppressed." — §7.6.3.2, `rfc9777.txt:2359-2361`

- Strength: description. Class: wire.
- Check idea: arrange all Retransmission-List sources on one side of the LLQT split; confirm only the non-empty one of the two Query Messages appears on the link.

## Query version distinctions

### RFC9777-VER-1

**A Multicast Listener Query's MLD version is determined by its length: 24 octets is MLDv1, 28 or more octets is MLDv2.**

> "*  MLDv1 Query: length = 24 octets
>
>    *  MLDv2 Query: length >= 28 octets" — §8.1, `rfc9777.txt:2383-2385`

- Strength: description. Class: encoding.
- Check idea: send Query Messages of length 24 and of length 28 or more; confirm the receiver classifies them as MLDv1 and MLDv2 respectively.

### RFC9777-VER-2

**A Query Message whose length matches neither MLDv1 nor MLDv2 (e.g. 26 octets) must be silently ignored.**

> "Query Messages that do not match any of the above conditions (e.g., a
>    Query of length 26 octets) MUST be silently ignored." — §8.1, `rfc9777.txt:2387-2388`

- Strength: must. Class: end-to-end.
- Check idea: send a Query Message of length 26 octets; confirm the receiver produces no reaction to it.

## Listener behavior with MLDv1

### RFC9777-COMPL-1

**To be compatible with MLDv1 routers, MLDv2 hosts must operate in version 1 compatibility mode.**

> "In order to be compatible with MLDv1 routers, MLDv2 hosts MUST
>    operate in version 1 compatibility mode." — §8.2.1, `rfc9777.txt:2394-2395`

- Strength: must. Class: end-to-end.
- Check idea: place an MLDv2 host on a link with an MLDv1 router and confirm the host's MLD traffic follows the MLDv1 protocol on that link.

### RFC9777-COMPL-2

**MLDv2 hosts must keep, per local interface, state for the compatibility mode of each attached link.**

> "MLDv2 hosts MUST keep state
>    per local interface regarding the compatibility mode of each attached
>    link." — §8.2.1, `rfc9777.txt:2395-2397`

- Strength: must. Class: internal.
- Check idea: attach one host to two links, one running MLDv1 and one MLDv2; confirm the host tracks a separate compatibility mode per interface.

### RFC9777-COMPL-3

**A host's compatibility mode comes from its Host Compatibility Mode variable, which is either MLDv1 or MLDv2.**

> "A host's compatibility mode is determined from the Host
>    Compatibility Mode variable that can be in one of the two states:
>    MLDv1 or MLDv2." — §8.2.1, `rfc9777.txt:2396-2399`

- Strength: description. Class: internal.
- Check idea: inspect a host's per-interface Host Compatibility Mode variable and confirm it always reads MLDv1 or MLDv2, never anything else.

### RFC9777-COMPL-4

**The Host Compatibility Mode of an interface is set to MLDv1 whenever an MLDv1 General Query is received on it.**

> "The Host Compatibility Mode of an interface is set to MLDv1 whenever
>    an MLDv1 Multicast Address Listener General Query is received on that
>    interface." — §8.2.1, `rfc9777.txt:2401-2403`

- Strength: description. Class: internal.
- Check idea: send an MLDv1 General Query to an MLDv2-mode host interface; confirm its Host Compatibility Mode switches to MLDv1.

### RFC9777-COMPL-5

**At the same time, the Older-Version-Querier-Present Timer for the interface is set to [Older Version Querier Present Interval] seconds.**

> "At the same time, the Older-Version-Querier-Present Timer
>    for the interface is set to [Older Version Querier Present Interval]
>    seconds." — §8.2.1, `rfc9777.txt:2403-2405`

- Strength: description. Class: internal.
- Check idea: receive an MLDv1 General Query and confirm the Older-Version-Querier-Present Timer is armed to the configured interval.

### RFC9777-COMPL-6

**The Older-Version-Querier-Present Timer resets on every new MLDv1 General Query received on that interface.**

> "The timer is reset whenever a new MLDv1 General Query is
>    received on that interface." — §8.2.1, `rfc9777.txt:2405-2406`

- Strength: description. Class: internal.
- Check idea: send repeated MLDv1 General Queries before the timer expires; confirm each one resets the Older-Version-Querier-Present Timer.

### RFC9777-COMPL-7

**If the Older-Version-Querier-Present Timer expires, the host switches back to Host Compatibility Mode MLDv2.**

> "If the Older-Version-Querier-Present
>    Timer expires, the host switches back to Host Compatibility Mode of
>    MLDv2." — §8.2.1, `rfc9777.txt:2406-2408`

- Strength: description. Class: internal.
- Check idea: stop sending MLDv1 General Queries and let the timer run out; confirm the host's Host Compatibility Mode reverts to MLDv2.

### RFC9777-COMPL-8

**With Host Compatibility Mode MLDv2, a host acts using the MLDv2 protocol on that interface.**

> "When Host Compatibility Mode is MLDv2, a host acts using the MLDv2
>    protocol on that interface." — §8.2.1, `rfc9777.txt:2410-2411`

- Strength: description. Class: end-to-end.
- Check idea: with Host Compatibility Mode MLDv2, trigger a state change and confirm the resulting messages are MLDv2 Reports.

### RFC9777-COMPL-9

**With Host Compatibility Mode MLDv1, a host acts in MLDv1 compatibility mode, using only the MLDv1 protocol, on that interface.**

> "When Host Compatibility Mode is MLDv1, a
>    host acts in MLDv1 compatibility mode, using only the MLDv1 protocol,
>    on that interface." — §8.2.1, `rfc9777.txt:2411-2413`

- Strength: description. Class: end-to-end.
- Check idea: with Host Compatibility Mode MLDv1, trigger a state change and confirm the resulting messages are MLDv1 Report/Done messages, not MLDv2 records.

### RFC9777-COMPL-10

**An MLDv1 Query's Maximum Response Code is the desired Maximum Response Delay directly: the field is linear, not the MLDv2 exponential encoding.**

> "An MLDv1 Querier will send General Queries with the Maximum Response
>    Code set to the desired Maximum Response Delay, i.e., the full range
>    of this field is linear and the exponential algorithm described in
>    Section 5.1.3. is not used." — §8.2.1, `rfc9777.txt:2415-2418`

- Strength: description. Class: wire.
- Check idea: inspect the Maximum Response Code of an MLDv1 General Query and confirm it equals the delay value directly, without exponential decoding.

### RFC9777-COMPL-11

**Whenever a host changes its compatibility mode, it cancels all its pending responses and retransmission timers.**

> "Whenever a host changes its compatibility mode, it cancels all its
>    pending responses and retransmission timers." — §8.2.1, `rfc9777.txt:2420-2421`

- Strength: description. Class: internal.
- Check idea: schedule a pending response or retransmission and then trigger a compatibility-mode change; confirm the pending item is canceled rather than sent.

### RFC9777-COMPL-12

**An SSM-aware host that receives an MLDv1 General or Group Specific Query for an SSM-range address should log an error.**

> "An SSM-aware host that receives an MLDv1 General Query or MLDv1 Group
>    Specific Query for a multicast address in the SSM address range
>    SHOULD log an error." — §8.2.1, `rfc9777.txt:2423-2425`

- Strength: should. Class: internal.
- Check idea: send an SSM-aware host an MLDv1 General Query naming an SSM-range address and confirm an error is logged.

### RFC9777-COMPL-13

**Implementations are recommended to provide a configuration option that disables use of Host Compatibility Mode, to allow SSM-only operation.**

> "It is RECOMMENDED that implementations provide
>    a configuration option to disable the use of Host Compatibility Mode
>    to allow networks to operate only in SSM mode." — §8.2.1, `rfc9777.txt:2425-2427`

- Strength: should. Class: internal.
- Check idea: check for a configuration option that keeps a host permanently in MLDv2/SSM mode regardless of any MLDv1 Query received.

### RFC9777-COMPL-14

**This configuration option should be disabled by default.**

> "This configuration
>    option SHOULD be disabled by default." — §8.2.1, `rfc9777.txt:2427-2428`

- Strength: should. Class: internal.
- Check idea: inspect a freshly configured host and confirm the Host-Compatibility-Mode-disabling option starts off.

### RFC9777-COMPL-15

**A host may allow its MLDv2 Multicast Listener Report to be suppressed by an MLDv1 Multicast Listener Report (Type 131).**

> "A
>    host MAY allow its MLD Version 2 Multicast Listener Report to be
>    suppressed by an MLDv1 Multicast Listener Report (Type = decimal
>    131)." — §8.2.2, `rfc9777.txt:2432-2435`

- Strength: may. Class: wire.
- Check idea: let another node send an MLDv1 Report (Type 131) for an address just before a host's own MLDv2 Report is due; confirm the host's Report may be suppressed.

## Router behavior with MLDv1

### RFC9777-COMPR-1

**If an MLDv1 router is present on the link, the Querier must use the lowest MLD version present on the network.**

> "If an MLDv1 router is present on the link, the Querier MUST use
>       the lowest version of MLD present on the network." — §8.3.1, `rfc9777.txt:2444-2445`

- Strength: must. Class: wire.
- Check idea: put an MLDv1 router and an MLDv2 router on the same link; confirm the elected Querier's Query Messages are MLDv1-format.

### RFC9777-COMPR-2

**This lowest-version requirement must be administratively assured.**

> "This must be
>       administratively assured." — §8.3.1, `rfc9777.txt:2445-2446`

- Strength: must (lower case). Class: internal.
- Check idea: check that the mechanism for guaranteeing the lowest-version rule is a configuration act, not an automatic protocol negotiation.

### RFC9777-COMPR-3

**Routers that want MLDv1 compatibility must have a configuration option to act in MLDv1 mode.**

> "Routers that desire to be compatible
>       with MLDv1 MUST have a configuration option to act in MLDv1 mode" — §8.3.1, `rfc9777.txt:2446-2448`

- Strength: must. Class: internal.
- Check idea: check an MLDv2 router for a configuration option that switches it into MLDv1 mode.

### RFC9777-COMPR-4

**If an MLDv1 router is present, the administrator must explicitly configure every MLDv2 router on the link into MLDv1 mode.**

> "if
>       an MLDv1 router is present on the link, the system
>       administrator must explicitly configure all MLDv2 routers to act
>       in MLDv1 mode." — §8.3.1, `rfc9777.txt:2448-2450`

- Strength: must (lower case). Class: internal.
- Check idea: check that when an MLDv1 router is added to a link, every other router's MLDv1-mode option is manually enabled rather than left automatic.

### RFC9777-COMPR-5

**In MLDv1 mode, the Querier must send periodic General Queries truncated at the Multicast Address field (24 bytes long).**

> "When in MLDv1 mode, the Querier MUST send periodic
>       General Queries truncated at the Multicast Address field (i.e., 24
>       bytes long)" — §8.3.1, `rfc9777.txt:2450-2452`

- Strength: must. Class: wire.
- Check idea: put a router in MLDv1 mode and measure its periodic General Query length; confirm each one is 24 bytes long.

### RFC9777-COMPR-6

**An MLDv1-mode Querier should warn about receiving an MLDv2 Query, and such warnings must be rate-limited.**

> "SHOULD also warn about receiving an MLDv2 Query
>       (such warnings MUST be rate-limited)." — §8.3.1, `rfc9777.txt:2452-2453`

- Strength: should, must. Class: internal.
- Check idea: send an MLDv2 Query to a router in MLDv1 mode repeatedly; confirm a warning is produced and further warnings are rate-limited rather than logged once per Query.

### RFC9777-COMPR-7

**The Querier must also fill in the Maximum Response Delay directly into the Maximum Response Code field, not using the exponential algorithm.**

> "The Querier MUST also fill
>       in the Maximum Response Delay in the Maximum Response Code field,
>       i.e., the exponential algorithm described in Section 5.1.3 is not
>       used." — §8.3.1, `rfc9777.txt:2453-2456`

- Strength: must. Class: wire.
- Check idea: inspect the Maximum Response Code of an MLDv1-mode Querier's General Query and confirm it equals the delay value directly.

### RFC9777-COMPR-8

**A router not configured for MLDv1 that receives an MLDv1 General Query should log a warning.**

> "If a router is not explicitly configured to use MLDv1 and receives
>       an MLDv1 General Query, it SHOULD log a warning." — §8.3.1, `rfc9777.txt:2458-2459`

- Strength: should. Class: internal.
- Check idea: send an MLDv1 General Query to a router that has no MLDv1-mode configuration; confirm a warning is logged.

### RFC9777-COMPR-9

**These warnings must be rate-limited.**

> "These warnings
>       MUST be rate-limited." — §8.3.1, `rfc9777.txt:2459-2460`

- Strength: must. Class: internal.
- Check idea: send repeated MLDv1 General Queries to an unconfigured router in quick succession; confirm the warnings are rate-limited rather than one per Query.

### RFC9777-COMPR-10

**Implementations are recommended to provide a configuration option that disables compatibility mode, for SSM-only networks.**

> "It is RECOMMENDED that implementations provide a configuration
>       option to disable use of compatibility mode to allow networks to
>       operate only in SSM mode." — §8.3.1, `rfc9777.txt:2462-2464`

- Strength: should. Class: internal.
- Check idea: check for a configuration option that keeps a router permanently in MLDv2/SSM mode regardless of any MLDv1 message received.

### RFC9777-COMPR-11

**This configuration option should be disabled by default.**

> "This configuration option SHOULD be
>       disabled by default." — §8.3.1, `rfc9777.txt:2464-2465`

- Strength: should. Class: internal.
- Check idea: inspect a freshly configured router and confirm the compatibility-mode-disabling option starts off.

### RFC9777-COMPR-12

**To be compatible with MLDv1 hosts, MLDv2 routers must operate in version 1 compatibility mode.**

> "In order to be compatible with
>    MLDv1 hosts, MLDv2 routers MUST operate in version 1 compatibility
>    mode." — §8.3.2, `rfc9777.txt:2470-2472`

- Strength: must. Class: end-to-end.
- Check idea: place an MLDv1 host on a link with an MLDv2 router; confirm the router translates the host's MLDv1 messages per its compatibility mode rather than ignoring them.

### RFC9777-COMPR-13

**A router keeps a compatibility mode per Multicast Address Record, held in a Multicast Address Compatibility Mode variable that is either MLDv1 or MLDv2.**

> "MLDv2 routers keep a compatibility mode per Multicast Address
>    Record." — §8.3.2, `rfc9777.txt:2472-2473`, and "The compatibility mode of a multicast address is determined
>    from the Multicast Address Compatibility Mode variable, which can be
>    in one of the two following states: MLDv1 or MLDv2." — §8.3.2, `rfc9777.txt:2473-2476`

- Strength: description. Class: internal.
- Check idea: inspect a router's per-address compatibility mode for two different addresses, one with an MLDv1 host and one without; confirm each address carries its own MLDv1/MLDv2 value.

### RFC9777-COMPR-14

**A Multicast Address Record's compatibility mode is set to MLDv1 whenever an MLDv1 Multicast Listener Report (Type 131) is received for that address.**

> "The Multicast Address Compatibility Mode of a Multicast Address
>    Record is set to MLDv1 whenever an MLDv1 Multicast Listener Report
>    (Type = decimal 131) is received for that multicast address." — §8.3.2, `rfc9777.txt:2477-2479`

- Strength: description. Class: internal.
- Check idea: send a router an MLDv1 Report for an address currently in MLDv2 compatibility mode; confirm the address's mode switches to MLDv1.

### RFC9777-COMPR-15

**At the same time, the Older-Version-Host-Present Timer for that address is set to [Older Version Host Present Interval] seconds.**

> "At the
>    same time, the Older-Version-Host-Present Timer for the multicast
>    address is set to [Older Version Host Present Interval] seconds." — §8.3.2, `rfc9777.txt:2479-2481`

- Strength: description. Class: internal.
- Check idea: receive an MLDv1 Report for an address and confirm the Older-Version-Host-Present Timer for that address is armed to the configured interval.

### RFC9777-COMPR-16

**The Older-Version-Host-Present Timer resets on every new MLDv1 Report received for that multicast address.**

> "The
>    timer is reset whenever a new MLDv1 Report is received for that
>    multicast address." — §8.3.2, `rfc9777.txt:2481-2483`

- Strength: description. Class: internal.
- Check idea: send repeated MLDv1 Reports for an address before the timer expires; confirm each one resets the Older-Version-Host-Present Timer.

### RFC9777-COMPR-17

**If the Older-Version-Host-Present Timer expires, the router switches that address back to MLDv2 Multicast Address Compatibility Mode.**

> "If the Older-Version-Host-Present Timer expires,
>    the router switches back to the Multicast Address Compatibility Mode
>    of MLDv2 for that multicast address." — §8.3.2, `rfc9777.txt:2483-2485`

- Strength: description. Class: internal.
- Check idea: stop sending MLDv1 Reports for an address and let the timer run out; confirm the address's compatibility mode reverts to MLDv2.

### RFC9777-COMPR-18

**On switching back to MLDv2 mode, source-specific state must be relearned via the next General Query, and sources due for blocking stay unblocked until [Multicast Address Listening Interval] afterward.**

> "Source-specific
>    information will be learned during the next General Query, but
>    sources that should be blocked will not be blocked until [Multicast
>    Address Listening Interval] after that." — §8.3.2, `rfc9777.txt:2489-2492`

- Strength: should (lower case). Class: end-to-end.
- Check idea: switch an address back to MLDv2 compatibility mode and track a source that should now be blocked; confirm it keeps being forwarded until [Multicast Address Listening Interval] after the next General Query.

### RFC9777-COMPR-19

**With Multicast Address Compatibility Mode MLDv2, a router acts using the MLDv2 protocol for that address.**

> "When Multicast Address Compatibility Mode is MLDv2, a router acts
>    using the MLDv2 protocol for that multicast address." — §8.3.2, `rfc9777.txt:2494-2495`

- Strength: description. Class: end-to-end.
- Check idea: with an address's compatibility mode MLDv2, send it MLDv2-only record types (e.g. ALLOW/BLOCK) and confirm the router processes them per the MLDv2 tables.

### RFC9777-COMPR-20

**Table 10, row Report: an MLDv1 Report is internally translated to the MLDv2 equivalent IS_EX({}).**

> "| MLDv1 Message | MLDv2 Equivalent |" — `rfc9777.txt:2501` and "| Report        | IS_EX( {} )      |" — `rfc9777.txt:2503`

- Strength: description. Class: internal.
- Check idea: send a router in MLDv1 compatibility mode an MLDv1 Report for an address; confirm it is processed as an IS_EX({}) Current-State Record per Table 7.

### RFC9777-COMPR-21

**Table 10, row Done: an MLDv1 Done is internally translated to the MLDv2 equivalent TO_IN({}).**

> "| MLDv1 Message | MLDv2 Equivalent |" — `rfc9777.txt:2501` and "| Done          | TO_IN( {} )      |" — `rfc9777.txt:2505`

- Strength: description. Class: internal.
- Check idea: send a router in MLDv1 compatibility mode an MLDv1 Done for an address; confirm it is processed as a TO_IN({}) Filter-Mode-Change Record per Table 8.

### RFC9777-COMPR-22

**MLDv2 BLOCK messages are ignored in MLDv1 compatibility mode, and any TO_EX() message's source-list is ignored, treating it as TO_EX({}).**

> "MLDv2 BLOCK messages are ignored, as are source-lists in TO_EX()
>    messages (i.e., any TO_EX() message is treated as TO_EX( {} ))." — §8.3.2, `rfc9777.txt:2510-2511`

- Strength: description. Class: end-to-end.
- Check idea: while an address is in MLDv1 compatibility mode, send a BLOCK record and, separately, a TO_EX(B) record with a non-empty B; confirm the BLOCK record has no effect and the TO_EX record is treated as if B were empty.

### RFC9777-COMPR-23

**The Querier keeps sending MLDv2 queries regardless of any address's Multicast Address Compatibility Mode.**

> "On
>    the other hand, the Querier continues to send MLDv2 queries,
>    regardless of its Multicast Address Compatibility Mode." — §8.3.2, `rfc9777.txt:2511-2513`

- Strength: description. Class: wire.
- Check idea: put an address into MLDv1 compatibility mode and trigger a specific query for it; confirm the Query Message sent is still MLDv2-format.

## Timers, counters and default values

### RFC9777-TIMER-1

**When a link uses non-default timer settings, they must be the same on every node of that link.**

> "If non-default settings are
>    used, they MUST be consistent among all nodes on a single link." — §9, `rfc9777.txt:2517-2518`

- Strength: must. Class: internal.
- Check idea: configure a non-default timer value on one node of a link and a different value on another node of the same link, and observe that this is a configuration state a conformant deployment must avoid.

### RFC9777-TIMER-2

**The Robustness Variable governs how many packet losses MLD tolerates; its default value is 2.**

> "MLD is robust to [Robustness
>    Variable] - 1 packet losses." and "Default value: 2." — §9.1, `rfc9777.txt:2526-2527`, `rfc9777.txt:2528`

- Strength: description. Class: internal.
- Check idea: read a node's Robustness Variable when unconfigured and observe the value 2; drop that many minus one consecutive MLD messages on a link and observe the node's state stays correct.

### RFC9777-TIMER-3

**The Robustness Variable must not be zero and should not be one.**

> "The value of the Robustness Variable
>    MUST NOT be zero and SHOULD NOT be one." — §9.1, `rfc9777.txt:2527-2528`

- Strength: must not (zero); should not (one). Class: internal.
- Check idea: attempt to configure a node's Robustness Variable to zero and to one, and observe that zero is rejected and one is discouraged.

### RFC9777-TIMER-4

**The Query Interval is the interval between General Queries sent by the Querier; its default value is 125 seconds.**

> "The Query Interval variable denotes the interval between General
>    Queries sent by the Querier.  Default value: 125 seconds." — §9.2, `rfc9777.txt:2532-2533`

- Strength: description. Class: internal.
- Check idea: leave the Query Interval unconfigured on a Querier and measure that it sends General Queries 125 seconds apart.

### RFC9777-TIMER-5

**The Query Response Interval is the Maximum Response Delay used in periodic General Queries; its default value is 10000 milliseconds.**

> "The Query Response Interval is the Maximum Response Delay used to
>    calculate the Maximum Response Code that is inserted into the
>    periodic General Queries.  Default value: 10000 (10 seconds)" — §9.3, `rfc9777.txt:2541-2543`

- Strength: description. Class: internal.
- Check idea: leave the Query Response Interval unconfigured on a Querier and observe that its periodic General Queries carry a Maximum Response Code corresponding to 10 seconds.

### RFC9777-TIMER-6

**The Query Response Interval must be less than the Query Interval.**

> "The number of seconds represented by [Query Response
>    Interval] must be less than the [Query Interval]." — §9.3, `rfc9777.txt:2548-2549`

- Strength: must (lower case). Class: internal.
- Check idea: configure a Querier with a Query Response Interval at or above its Query Interval and observe that this is a configuration state the specification excludes.

### RFC9777-TIMER-7

**The Multicast Address Listening Interval is the time before a router decides a link has no more listeners for an address or source; it must equal Robustness Variable times Query Interval plus twice the Query Response Interval.**

> "The Multicast Address Listening Interval (MALI) is the amount of time
>    that must pass before a multicast router decides there are no more
>    listeners of a multicast address or a particular source on a link.
>    This value MUST be ([Robustness Variable] times [Query Interval])
>    plus 2 times [Query Response Interval]." — §9.4, `rfc9777.txt:2553-2557`

- Strength: must. Class: internal.
- Check idea: configure known Robustness Variable, Query Interval, and Query Response Interval on a router, stop all Reports for an address, and measure that the router declares no listeners after the formula's interval.

### RFC9777-TIMER-8

**The Other Querier Present Interval is the time before a router decides no other Querier remains; it must equal Robustness Variable times Query Interval plus half the Query Response Interval.**

> "The Other Querier Present Interval is the length of time that must
>    pass before a multicast router decides that there is no longer
>    another multicast router that should be the Querier.  This value MUST
>    be ([Robustness Variable] times ([Query Interval]) plus (0.5 times
>    [Query Response Interval])." — §9.5, `rfc9777.txt:2561-2565`

- Strength: must. Class: internal.
- Check idea: configure known Robustness Variable, Query Interval, and Query Response Interval on a non-Querier router, stop Queries from the current Querier, and measure that the router takes over after the formula's interval.

### RFC9777-TIMER-9

**The Startup Query Interval is the interval between General Queries sent by a Querier at startup; its default value is a quarter of the Query Interval.**

> "The Startup Query Interval is the interval between General Queries
>    sent by a Querier on startup.  Default value: 1/4 the [Query
>    Interval]." — §9.6, `rfc9777.txt:2569-2571`

- Strength: description. Class: internal.
- Check idea: start a router as Querier with a known Query Interval and measure that its first General Queries are spaced one quarter of that interval apart.

### RFC9777-TIMER-10

**The Startup Query Count is the number of Queries a Querier sends at startup; its default value is the Robustness Variable.**

> "The Startup Query Count is the number of Queries sent out on startup,
>    separated by the Startup Query Interval.  Default value: [Robustness
>    Variable]." — §9.7, `rfc9777.txt:2575-2577`

- Strength: description. Class: internal.
- Check idea: start a router as Querier with a known Robustness Variable and count the number of Queries it sends during startup.

### RFC9777-TIMER-11

**The Last Listener Query Interval is the Maximum Response Delay used in Multicast Address Specific and Multicast Address and Source Specific Queries; its default value is 1000 milliseconds.**

> "The Last Listener Query Interval (LLQI) is the Maximum Response Delay
>    used to calculate the Maximum Response Code inserted into Multicast
>    Address Specific Queries sent in response to MLDv1 Multicast Listener
>    Done (Type = decimal 132) messages.  It is also the Maximum Response
>    Delay used to calculate the Maximum Response Code inserted into
>    Multicast Address and Source Specific Query Messages.  Default value:
>    1000 (1 second)." — §9.8, `rfc9777.txt:2581-2587`

- Strength: description. Class: internal.
- Check idea: leave the Last Listener Query Interval unconfigured on a router and observe that a Multicast Address Specific Query it sends carries a Maximum Response Code corresponding to 1 second.

### RFC9777-TIMER-12

**When a configured time does not exactly fit the Maximum Response Code encoding, an implementation uses the exact value if possible or otherwise the next lower representable value.**

> "When converting a configured time to a
>    Maximum Response Code value, it is recommended to use the exact value
>    if possible, or the next lower value if the requested value is not
>    exactly representable." — §9.8, `rfc9777.txt:2591-2594`

- Strength: description. Class: encoding.
- Check idea: configure the Last Listener Query Interval to a value above 32.768 seconds that the Maximum Response Code cannot represent exactly, and observe that the encoded value rounds down to the next representable one.

### RFC9777-TIMER-13

**The Last Listener Query Count is the number of Multicast Address Specific, or Multicast Address and Source Specific, Queries a router sends before it assumes no listeners remain; its default value is the Robustness Variable.**

> "The Last Listener Query Count is the number of Multicast Address
>    Specific Queries sent before the router assumes there are no local
>    listeners.  The Last Listener Query Count is also the number of
>    Multicast Address and Source Specific Queries sent before the router
>    assumes there are no listeners for a particular source.  Default
>    value: [Robustness Variable]." — §9.9, `rfc9777.txt:2602-2607`

- Strength: description. Class: internal.
- Check idea: with a known Robustness Variable, cause a router to start last-listener queries for an address and count how many it sends before declaring no listeners.

### RFC9777-TIMER-14

**The Last Listener Query Time is the Last Listener Query Interval times the Last Listener Query Count.**

> "The Last Listener Query Time is the time value represented by the
>    [Last Listener Query Interval] times [Last Listener Query Count]." — §9.10, `rfc9777.txt:2611-2613`

- Strength: description. Class: internal.
- Check idea: with known Last Listener Query Interval and Count on a router, measure the total time from the first Multicast Address Specific Query to its last against their product.

### RFC9777-TIMER-15

**The Unsolicited Report Interval is the time between repetitions of a node's initial report of interest; its default value is 1 second.**

> "The Unsolicited Report Interval is the time between repetitions of a
>    node's initial report of interest in a multicast address.  Default
>    value: 1 second." — §9.11, `rfc9777.txt:2618-2620`

- Strength: description. Class: internal.
- Check idea: have a node join a multicast address and measure that it repeats its unsolicited Report 1 second after the first, when unconfigured.

### RFC9777-TIMER-16

**The Older Version Querier Present Interval is the timeout for a host to return to MLDv2 Host Compatibility Mode; on receiving an MLDv1 Query, an MLDv2 host sets its Older-Version-Querier-Present Timer to this interval.**

> "The Older Version Querier Present Interval is the timeout for
>    transitioning a host back to MLDv2 Host Compatibility Mode.  When an
>    MLDv1 query is received, MLDv2 hosts set their Older-Version-Querier-
>    Present Timer to [Older Version Querier Present Interval]." — §9.12, `rfc9777.txt:2624-2627`

- Strength: description. Class: internal.
- Check idea: send an MLDv1 Query to an MLDv2 host and observe that it starts an Older-Version-Querier-Present Timer set to this interval.

### RFC9777-TIMER-17

**The Older Version Querier Present Interval value must equal Robustness Variable times the Query Interval from the last Query received, plus the Query Response Interval.**

> "This value MUST be ([Robustness Variable] times [Query Interval] in
>    the last Query received) plus ([Query Response Interval])." — §9.12, `rfc9777.txt:2629-2630`

- Strength: must. Class: internal.
- Check idea: send an MLDv1 host a Query carrying a known Query Interval, with known Robustness Variable and Query Response Interval configured, and measure the resulting Older-Version-Querier-Present Timer against the formula.

### RFC9777-TIMER-18

**The Older Version Host Present Interval is the timeout for a router to return to MLDv2 Multicast Address Compatibility Mode for an address; on receiving an MLDv1 report for that address, a router sets its Older-Version-Host-Present Timer to this interval.**

> "The Older Version Host Present Interval is the timeout for
>    transitioning a router back to MLDv2 Multicast Address Compatibility
>    Mode for a specific multicast address.  When an MLDv1 report is
>    received for that multicast address, routers set their Older-Version-
>    Host-Present Timer to [Older Version Host Present Interval]." — §9.13, `rfc9777.txt:2634-2638`

- Strength: description. Class: internal.
- Check idea: send an MLDv1 Report for a multicast address to an MLDv2 router and observe that it starts an Older-Version-Host-Present Timer for that address, set to this interval.

### RFC9777-TIMER-19

**The Older Version Host Present Interval value must equal Robustness Variable times Query Interval plus the Query Response Interval.**

> "This value MUST be ([Robustness Variable] times [Query Interval])
>    plus ([Query Response Interval])." — §9.13, `rfc9777.txt:2640-2641`

- Strength: must. Class: internal.
- Check idea: with known Robustness Variable, Query Interval, and Query Response Interval configured on a router, send it an MLDv1 Report and measure the resulting Older-Version-Host-Present Timer against the formula.

### RFC9777-TIMER-20

**The Query Interval value must be equal to or greater than the Maximum Response Delay used in General Query messages.**

> "The value of the Query Interval MUST
>    be equal to or greater than the Maximum Response Delay used to
>    calculate the Maximum Response Code inserted in General Query
>    Messages." — §9.14.2, `rfc9777.txt:2667-2670`

- Strength: must. Class: internal.
- Check idea: configure a Querier with a Query Interval shorter than the Maximum Response Delay of its General Queries and observe that this is a configuration state the specification excludes.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§5 to §9, and every field of every message. What the catalog leaves out:

- **The sections outside the in-scope set.** §1 to §4 (introduction, protocol overview, the
  service interface, the listening state of a node), §10 to §12 (security, IANA, references)
  and the appendices. See [`standards.md`](../../protocol/mld/standards.md#target-level) for
  the level at which each one enters.

Within the in-scope sections, these parts have no entry, with the reason:

In §5 and §9:

- §5 (734-736): "MLDv2 is a sub-protocol of ICMPv6, that is, MLDv2 message types are a subset of ICMPv6 messages" — introductory classification, not itself a checkable behavior; the Next Header value it leads into is GEN-1.
- §5 (741-744): "(The Router Alert option is necessary to cause routers to examine MLDv2 messages ...)" — parenthetical rationale for GEN-4, not a separate rule.
- §5 (744-747): "MLDv2 Reports can be sent with the source address set to the unspecified address ... (See Section 5.2.14 for details.)" — anticipates §5.2.14, cataloged in full there as REP-34, REP-37, REP-38.
- §5 (766-767): "Other message types may be used by newer versions or extensions of MLD, by multicast routing protocols, or for other uses." — informative note about future/other-protocol use, not a requirement on an MLDv2 node.
- §5 (769-771): definition of the capitalized terms "Query" and "Report" for the rest of the document — pure terminology, not checkable.
- §5.1 (779-822): the Query message ASCII-art diagram — wire layout shown graphically; each field is cataloged individually under §5.1.1-§5.1.12 and §5.1.14-§5.1.15.
- §5.1.3 (857-862): informative discussion of how small/large Maximum Response Delay values tune leave latency and burstiness — explanatory, not a requirement.
- §5.1.6 (876-880) Flags: bit allocation and meaning are deferred entirely to RFC9778 §2.2; RFC9777 itself states no checkable value for this document.
- §5.1.10 (933-939): the Ethernet MTU worked example (89 sources on a 1500-octet link) — illustrative example of QRY-18, not an added rule.
- §5.2 (1005-1081): the Report message and Multicast Address Record ASCII-art diagrams — wire layout shown graphically; each field is cataloged individually under §5.2.1-§5.2.11.
- §5.2.3 (1096-1100) Flags: bit allocation and meaning are deferred entirely to RFC9778 §2.3; RFC9777 itself states no checkable value for this document.
- §5.2.11 (1148-1150): "The semantics and the internal encoding of the Auxiliary Data field are to be defined by any future version..." — informative, defers to future documents.
- §5.2.13 (1223-1227, 1232-1236): the "if the change was to an INCLUDE/EXCLUDE source-list, these are the addresses that were added/deleted" clarifications within ALLOW_NEW_SOURCES and BLOCK_OLD_SOURCES — restate the same rule already captured by REP-30/REP-31 from the derivation side; a single check on the record's content covers both.
- §5.2.13 (1243-1244): "We use the term 'State-Change Record' to refer to..." — a defined shorthand used later in the document, not itself a checkable statement.
- §5.2.13 (1249-1267): the IS_IN()/IS_EX()/TO_IN()/TO_EX()/ALLOW()/BLOCK() notation and the set-expression notation ("A+B", "A*B", "A-B") — shorthand the document defines for use in sections outside this range (§6/§7 state tables), not itself a rule.
- §5.2.14 (1273-1281): informative background on why the unspecified address is allowed (Neighbor Discovery multicast use, Duplicate Address Detection) — explanatory, not a separate requirement.
- §5.2.14 (1289-1290): "This enhances security, as unidentified reporting nodes cannot influence the state of the MLDv2 router(s)." — rationale for REP-35/REP-37, not a new rule.
- §5.2.15 (1310-1311, 1313-1314): "This might be useful, e.g., for debugging purposes." — rationale for REP-42, not a new rule.
- §5.2.16 (1333-1335): "Although the choice of which sources to report is arbitrary, it is preferable to report the same set of sources in each subsequent report, rather than reporting different sources each time." — a soft, non-keyword preference about which sources to pick across repeated reports, additional to the field rule in REP-45; not independently checkable as a single-report observation.
- §9 (2515-2517, 2519-2520): "List of Timers, Counters, and Their Default Values" heading, "Most of these timers are configurable", and the note about parentheses grouping the algebra — framing and notation, not checkable statements.
- §9.1 (2525-2526): "If a link is expected to be lossy, the value of the Robustness Variable may be increased." — informal administrator guidance, not a requirement on a node.
- §9.2 (2535-2537): "By varying the Query Interval, an administrator may tune the number of MLD messages on the link..." — informal administrator guidance.
- §9.3 (2545-2547): "By varying the Query Response Interval, an administrator may tune the burstiness of MLD messages on the link..." — informal administrator guidance.
- §9.8 (2596-2598): "This value may be tuned to modify the leave latency of the link. A reduced value results in reduced time to detect the departure of the last listener..." — informal administrator guidance about the effect of the (already-cataloged) LLQI.
- §9.10 (2613-2614): "It is not a tunable value, but it may be tuned by changing its components." — restates that Last Listener Query Time is derived, already implicit in TIMER-14's formula.
- §9.14 (2643-2648): "Configuring Timers" section introduction — states the section is advice to administrators, not itself a rule.
- §9.14.1 (2650-2661): Robustness Variable tuning guidance — administrator advice repeating the default and trade-off already cataloged as TIMER-2/TIMER-3, with no new keyword or field value.
- §9.14.2 (2665-2667): "The overall level of periodic MLD traffic is inversely proportional to the Query Interval. A longer Query Interval results in a lower overall level of MLD traffic." — explanatory lead-in to the MUST captured as TIMER-20.
- §9.14.3 (2672-2703) entire subsection, including Table 11: burstiness/leave-latency discussion, the optional ("may be dynamically calculated") per-query Maximum Response Delay tuning, and the explicit disclaimer "A router is not required to calculate these populations or tune the Maximum Response Delay dynamically; these are simply guidelines." — the subsection is self-declared non-normative guidance, with no MUST/SHOULD and no fixed field value to check.

In §6 to §8:

- `rfc9777.txt:1342-1346` — note that a router that is also a listener performs both parts of MLDv2, plus a forward reference to §7; background/cross-reference, not itself a checkable statement.
- `rfc9777.txt:1352-1358` — mention that nodes maintain a Host Compatibility Mode variable; background introduction, the normative definition is cataloged precisely under COMPL (§8.2.1).
- `rfc9777.txt:1371-1379` — enumeration of the three MLDv2 trigger events (state change, timer, Query); purely organizational, each event type is covered by its own area (LSN/LTIM/LQRY).
- `rfc9777.txt:1385-1387` — pointer to the following subsections and to §9 for timer/counter defaults; organizational, not a behavior statement.
- `rfc9777.txt:1391-1394` — reference to the IPv6MulticastListen service-interface invocation and §4.2; background explanation of the service interface, out of scope per the brief.
- `rfc9777.txt:1515-1518` — explicit "Note:" about treating the first pending report as an empty Source-Change Report; informative aside.
- `rfc9777.txt:1520-1522` — cross-reference to §6.3 for scheduled-retransmission report building; purely referential.
- `rfc9777.txt:1536-1540` — remark that a node may face several concurrent Queries needing separate delayed responses; informative framing, not itself a rule (the handling rules are cataloged as LQRY-5..9).
- `rfc9777.txt:1542-1544` — general statement that the node must consider pending responses before scheduling a combined one; restated precisely by the five numbered rules (LQRY-5 through LQRY-9).
- `rfc9777.txt:1607-1609` — intro sentence to the §6.3 numbered timer-expiration list; organizational only.
- `rfc9777.txt:1712-1720` — purpose/goal paragraph describing what MLD is for; background, not a single checkable node behavior.
- `rfc9777.txt:1722-1725` — section intro plus cross-reference to §6 for the router's own listener role; background/reference.
- `rfc9777.txt:1753-1754` — pointer to §8 for MLDv1 compatibility; purely referential (the material itself is cataloged under VER/COMPL/COMPR).
- `rfc9777.txt:1795-1796` (fragment) and `1811-1812` — "Section 5.1.13 describes each query in more detail."; cross-reference.
- `rfc9777.txt:1838-1847` — §7.2.1 intro paragraph on reducing internal state and the filter-mode possibly changing; background, restated precisely by the RREP tables (§7.4).
- `rfc9777.txt:1866-1870` — rationale ("it is desirable") for the EXCLUDE-to-INCLUDE transition with a forward reference to §7.5; the precise rule is cataloged as RST-15 (§7.2.2) and RSW-1 (§7.5).
- `rfc9777.txt:1879-1881` — cross-reference to §7.4.1/§7.4.2.
- `rfc9777.txt:1889-1891` (tail) — "Filter timers are updated according to the types of Multicast Address Records received."; cross-reference, precise rule is in the RREP tables.
- `rfc9777.txt:1899-1901` — intro sentence to Table 5; organizational.
- `rfc9777.txt:1931` (tail) — "Section 7.4 describes the setting of source timers..."; cross-reference.
- `rfc9777.txt:1941-1943` (tail) — explanation of what LLQT "represents" (leave latency); explanatory gloss, not a separate rule.
- `rfc9777.txt:1955-1957` — intro naming the "fast leave" scheme; organizational framing for RST-26..29.
- `rfc9777.txt:1978-1980` — "The router has to maintain the Requested List for two reasons:"; list intro.
- `rfc9777.txt:1991-2005` — rationale for why the Requested List can be an inexact guess and how the protocol ensures eventual accuracy, plus an Appendix A.3 reference; explanatory reasoning, not an independently checkable rule.
- `rfc9777.txt:1995-1999` — general remark that fast blocking may delete Requested-List sources with the Filter Timer updated "as shown in Sections 7.4.1 and 7.4.2"; the precise mechanism is already cataloged via the RREP tables.
- `rfc9777.txt:2004-2005`, `2018-2019` — cross-references to Appendix A.3 and to §7.4.1/§7.4.2.
- `rfc9777.txt:2039-2043` — intro sentence to Table 6; organizational.
- `rfc9777.txt:2096-2101` — intro sentence to Table 7 ("a router updates both its Filter Timer and source timers..."); summarized precisely by the table-row entries themselves.
- `rfc9777.txt:2103-2115` — notation legend explaining the INCLUDE(A)/EXCLUDE(X,Y) and "(A)=J"/"Delete(A)" table notation; a reading aid, not a behavior statement.
- `rfc9777.txt:2181-2184` — intro sentences to Table 8; organizational.
- `rfc9777.txt:2186-2190` — notation legend for Q(MA) and Q(MA,A); reading aid.
- `rfc9777.txt:2200-2203` — vague remark that pending queries "may affect the contents" of new ones, plus a forward reference to §7.6.3; the concrete building rules are cataloged under RQRY (§7.6.3).
- `rfc9777.txt:2241-2242` — summary sentence that the Filter Timer is "used as a mechanism" for the EXCLUDE-to-INCLUDE switch; restated precisely by RSW-1.
- `rfc9777.txt:2252-2255` — worked example of the INCLUDE-switch rule; example, not an independent statement (the rule itself is RSW-2).
- `rfc9777.txt:2267-2268` — folded into RQRY-1's quote (continuation of the same validity-check sentence group), not separately left out.
- `rfc9777.txt:2272-2273` — cross-reference to §2.1 for the S flag.
- `rfc9777.txt:2278` (tail) — intro sentence to Table 9; organizational.
- `rfc9777.txt:2297-2299` — remark that MLDv2 reuses MLDv1's IPv6-address election mechanism; background, the precise "lower address" rule is cataloged as RQRY-11.
- `rfc9777.txt:2363-2368` — explicit "Note:" about possible suppression of a Multicast Address Specific Query; informative aside.
- `rfc9777.txt:2372-2376` — §8 introductory paragraph on interoperation in general; background.
- `rfc9777.txt:2432-2433` — framing sentence "An MLDv2 host may be placed on a link where there are MLDv1 hosts."; background lead-in to COMPL-15.
- `rfc9777.txt:2441-2442` — "MLDv2 routers may be placed on a network where there is at least one MLDv1 router. The following requirements apply:"; list intro.
- `rfc9777.txt:2469-2470` — "MLDv2 routers may be placed on a network where there are hosts that have not yet been upgraded to MLDv2."; background lead-in to COMPR-12.
- `rfc9777.txt:2487-2489` (lead) — "Note that when a router switches back to MLDv2 ... it takes some time to regain source-specific state information."; explanatory lead-in, the concrete consequence is cataloged as COMPR-18.
- `rfc9777.txt:2495-2498` — intro sentence to Table 10 ("a router internally translates the following MLDv1 messages... to their MLDv2 equivalents"); summarized by the table-row entries COMPR-20/21.
