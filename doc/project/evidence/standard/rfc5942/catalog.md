# RFC 5942 (IPv6 Subnet Model: The Relationship between Links and Subnet Prefixes) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC5942-*` · **Stands on:** [standards.md](../../protocol/nd/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 5942, IPv6 Subnet Model: The Relationship between Links and Subnet Prefixes, of
July 2010, a document of the in-scope set of Neighbor Discovery. The catalog comes from the RFC
text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc5942.txt`](../../../../../../standards/RFC/rfc5942.txt) —
  IPv6 Subnet Model: The Relationship between Links and Subnet Prefixes, July 2010. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc5942.txt>.

This document updates [RFC 4861](../rfc4861/catalog.md): §6 deprecates two bullets of the
on-link definition of RFC 4861 §2.1, and §4 states the rules a host follows instead. RFC 4861
§2.1 is outside the in-scope set, so it has no entries to override; the override is recorded in
[`standards.md`](../../protocol/nd/standards.md#override-table). The entries come from §4 and
§6.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/nd/standards.md). The features these statements build are in
[`features.md`](../../protocol/nd/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`nd/coverage.md`](../../model/nd/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc5942.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC5942-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 5942 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them, and
  `description` for a normative sentence with no keyword. Where the text writes a keyword in
  lower case, the entry records the word and says so, for example `must (lower case)`; a
  "might" stays `description`. An entry with two keywords names what each one covers.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link, or
  the presence or absence of a message), `end-to-end` (what a node accepts and acts on, seen
  in what it sends or forwards afterwards), `error-signal` (an ICMPv6 error report),
  `internal` (state inside a node), `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC5942-ONLINK-1](#rfc5942-onlink-1) | The assignment of an address does not make a prefix from that address on-link, and it does not add that prefix to the Prefix List. |
| [RFC5942-ONLINK-2](#rfc5942-onlink-2) | A host treats a prefix as on-link only through explicit means: the on-link definition of RFC 4861 as RFC 5942 changes it, or manual configuration. |
| [RFC5942-ONLINK-3](#rfc5942-onlink-3) | When the Valid Lifetime of an advertised on-link prefix expires and no other on-link information exists, the host treats the addresses of the prefix as off-link. |
| [RFC5942-ONLINK-4](#rfc5942-onlink-4) | When an advertised on-link prefix expires, the host updates its Prefix List, and an address that another entry still covers stays on-link. |
| [RFC5942-ONLINK-5](#rfc5942-onlink-5) | With an empty Default Router List and no on-link information, the host does not assume that all destinations are on-link. |
| [RFC5942-ONLINK-6](#rfc5942-onlink-6) | With an empty Default Router List and no on-link information, the host does not perform address resolution for a destination that is not link-local. |
| [RFC5942-ONLINK-7](#rfc5942-onlink-7) | In that case the host has no route to the destination, and it reports an ICMPv6 Destination Unreachable, for example as a local error. |
| [RFC5942-ONLINK-8](#rfc5942-onlink-8) | On-link information about specific addresses and prefixes makes only those on-link, and the default rules stay for all other destinations. |
| [RFC5942-ONLINK-9](#rfc5942-onlink-9) | A received Neighbor Advertisement for an address no longer makes that address on-link. |
| [RFC5942-ONLINK-10](#rfc5942-onlink-10) | A Neighbor Discovery message from an address no longer makes that address on-link. |

## The on-link determination

### RFC5942-ONLINK-1

**The assignment of an address does not make a prefix from that address on-link, and it does not add that prefix to the Prefix List.**

> "A correctly implemented IPv6 host MUST adhere to the following rules:" — §4,
> `rfc5942.txt:359`, and "1.  The assignment of an IPv6 address -- whether through IPv6
> stateless address autoconfiguration [RFC4862], DHCPv6 [RFC3315],
> or manual configuration -- MUST NOT implicitly cause a prefix
> derived from that address to be treated as on-link and added to
> the Prefix List." — §4, `rfc5942.txt:361-365`

- Strength: must not. Class: wire.
- Check idea: a router advertises prefix P/64 with A = 1 and L = 0. Host A forms P::X, then
  sends a packet to P::Y of host B on the same link. Host A sends the packet to the default
  router, and it sends no Neighbor Solicitation for P::Y. Repeat with a manually configured
  address and no on-link prefix.

### RFC5942-ONLINK-2

**A host treats a prefix as on-link only through explicit means: the on-link definition of RFC 4861 as RFC 5942 changes it, or manual configuration.**

> "A host considers a prefix to be on-link only
> through explicit means, such as those specified in the on-link
> definition in the Terminology section of [RFC4861] (as modified
> by this document) or via manual configuration." — §4, `rfc5942.txt:365-368`

- Strength: description. Class: wire.
- Check idea: a router advertises prefix P/64 with L = 1; in a second run, an operator
  configures P/64 as on-link on host A. In both runs, host A sends a Neighbor Solicitation for
  P::Y and then sends its packet to P::Y directly.

### RFC5942-ONLINK-3

**When the Valid Lifetime of an advertised on-link prefix expires and no other on-link information exists, the host treats the addresses of the prefix as off-link.**

> "2.  In the absence of other sources of on-link information, including
> Redirects, if the RA advertises a prefix with the on-link (L) bit
> set and later the Valid Lifetime expires, the host MUST then
> consider addresses of the prefix to be off-link, as specified by
> the PIO paragraph of Section 6.3.4 of [RFC4861]." — §4, `rfc5942.txt:372-376`

- Strength: must. Class: wire.
- Check idea: a router advertises prefix P/64 with L = 1, Valid Lifetime 30 s and Router
  Lifetime 1800 s, and sends no more Router Advertisements. After 30 s, host A sends its
  packets to P::Y through the router, and it sends no Neighbor Solicitation for P::Y.

### RFC5942-ONLINK-4

**When an advertised on-link prefix expires, the host updates its Prefix List, and an address that another entry still covers stays on-link.**

> "3.  In the absence of other sources of on-link information, including
> Redirects, if the RA advertises a prefix with the on-link (L) bit
> set and later the Valid Lifetime expires, the host MUST then
> update its Prefix List with respect to the entry.  In most cases,
> this will result in the addresses covered by the prefix
> defaulting back to being considered off-link, as specified by the
> PIO paragraph of Section 6.3.4 of [RFC4861].  However, there are
> cases where an address could be covered by multiple entries in
> the Prefix List, where expiration of one prefix would result in
> destinations then being covered by a different entry." — §4, `rfc5942.txt:378-387`

- Strength: must. Class: wire.
- Check idea: a router advertises 2001:db8::/48 with L = 1 and a long Valid Lifetime, and
  2001:db8:0:1::/64 with L = 1 and Valid Lifetime 30 s. After 30 s, host A still sends a
  Neighbor Solicitation for 2001:db8:0:1::5 and sends its packet to that address directly.

### RFC5942-ONLINK-5

**With an empty Default Router List and no on-link information, the host does not assume that all destinations are on-link.**

> "4.  Implementations compliant with [RFC4861] MUST adhere to the
> following rules.  If the Default Router List is empty and there
> is no other source of on-link information about any address or
> prefix:" — §4, `rfc5942.txt:399-402`, and "a.  The host MUST NOT assume that all destinations are on-link." — §4,
> `rfc5942.txt:404`

- Strength: must not. Class: wire.
- Check idea: host A is on a link with no router and has no on-link prefix. An application on
  host A sends to 2001:db8::1. Host A sends no packet to 2001:db8::1 on the link.

### RFC5942-ONLINK-6

**With an empty Default Router List and no on-link information, the host does not perform address resolution for a destination that is not link-local.**

> "b.  The host MUST NOT perform address resolution for non-link-
> local addresses." — §4, `rfc5942.txt:406-407`

- Strength: must not. Class: wire.
- Check idea: the same setup as RFC5942-ONLINK-5. Host A sends no Neighbor Solicitation for
  2001:db8::1. For a link-local destination, it still sends a Neighbor Solicitation.

### RFC5942-ONLINK-7

**In that case the host has no route to the destination, and it reports an ICMPv6 Destination Unreachable, for example as a local error.**

> "c.  Since the host cannot assume the destination is on-link, and
> off-link traffic cannot be sent to a default router (since
> the Default Router List is empty), address resolution cannot
> be performed.  This case is specified in the last paragraph
> of Section 4 of [RFC4943]: when there is no route to the
> destination, the host should send an ICMPv6 Destination
> Unreachable indication (for example, a locally delivered
> error message) as specified in the Terminology section of
> [RFC4861]." — §4, `rfc5942.txt:409-417`

- Strength: should (lower case). Class: error-signal.
- Check idea: the same setup as RFC5942-ONLINK-5. The application on host A that sends to
  2001:db8::1 gets a Destination Unreachable indication from its own host, and no packet
  goes out on the link.

### RFC5942-ONLINK-8

**On-link information about specific addresses and prefixes makes only those on-link, and the default rules stay for all other destinations.**

> "On-link information concerning particular addresses and prefixes
> can make those specific addresses and prefixes on-link, but does
> not change the default behavior mentioned above for addresses and
> prefixes not specified." — §4, `rfc5942.txt:419-422`

- Strength: description. Class: wire.
- Check idea: host A is on a link with no default router. A Router Advertisement with Router
  Lifetime 0 brings prefix P/64 with L = 1. Host A sends a Neighbor Solicitation for P::Y,
  and none for 2001:db8:ffff::1, which is outside P.

### RFC5942-ONLINK-9

**A received Neighbor Advertisement for an address no longer makes that address on-link.**

> "This document deprecates the following two bullets from the on-link
> definition in Section 2.1 of [RFC4861]:" — §6, `rfc5942.txt:461-462`, and
> "o  a Neighbor Advertisement message is received for the (target)
> address, or" — §6, `rfc5942.txt:464-465`

- Strength: description. Class: wire.
- Check idea: host A has a default router and no on-link prefix that covers address G of host
  B on the same link. Host B sends an unsolicited Neighbor Advertisement with Target Address
  G. Host A still sends its packets to G through the default router.

### RFC5942-ONLINK-10

**A Neighbor Discovery message from an address no longer makes that address on-link.**

> "This document deprecates the following two bullets from the on-link
> definition in Section 2.1 of [RFC4861]:" — §6, `rfc5942.txt:461-462`, and
> "o  any Neighbor Discovery message is received from the address." — §6, `rfc5942.txt:467`

- Strength: description. Class: wire.
- Check idea: the same setup as RFC5942-ONLINK-9. Host B sends host A a Neighbor Solicitation
  from source G. Host A still sends its later packets to G through the default router.

## Out of scope in this catalog

- **The sections outside §4 and §6**: §1 to §3 (introduction, requirements language, host
  behavior as background), §5 (observed incorrect behavior), and §7 to §11.

Within §4 and §6, these parts have no entry, with the reason:

- The note that RFC 4861 does not name manually configured addresses, `rfc5942.txt:368-370`.
  A remark on the older text; the rule is in RFC5942-ONLINK-1 and RFC5942-ONLINK-2.
- The reference to RFC 4943 for the justification of rule 4, `rfc5942.txt:422-423`.
- Nothing else of §4 (`rfc5942.txt:357-424`) is left out: the lead sentence, rules 1 to 3,
  and rule 4 with a., b. and c. are all in entries.
- The §6 heading, `rfc5942.txt:459`. No content.
