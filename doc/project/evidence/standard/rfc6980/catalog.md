# RFC 6980 (Security Implications of IPv6 Fragmentation with IPv6 Neighbor Discovery) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6980-*` · **Stands on:** [standards.md](../../protocol/nd/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 6980, Security Implications of IPv6 Fragmentation with IPv6 Neighbor Discovery, of
August 2013, a document of the in-scope set of Neighbor Discovery. The catalog comes from the RFC
text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc6980.txt`](../../../../../../standards/RFC/rfc6980.txt) —
  Security Implications of IPv6 Fragmentation with IPv6 Neighbor Discovery, August 2013. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc6980.txt>.

This document updates [RFC 4861](../rfc4861/catalog.md): Neighbor Discovery messages are not
fragmented. The in-scope section is §5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/nd/standards.md). The features these statements build are in
[`features.md`](../../protocol/nd/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`nd/coverage.md`](../../model/nd/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc6980.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC6980-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 6980 uses the keywords of RFC 2119 in capitals;
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
| [RFC6980-FRAG-1](#rfc6980-frag-1) | A node does not use IPv6 fragmentation to send a Neighbor Solicitation, Neighbor Advertisement, Router Solicitation, Router Advertisement or Redirect. |
| [RFC6980-FRAG-2](#rfc6980-frag-2) | A node silently ignores a Neighbor Solicitation in a packet with a Fragment header. |
| [RFC6980-FRAG-3](#rfc6980-frag-3) | A node silently ignores a Neighbor Advertisement in a packet with a Fragment header. |
| [RFC6980-FRAG-4](#rfc6980-frag-4) | A node silently ignores a Router Solicitation in a packet with a Fragment header. |
| [RFC6980-FRAG-5](#rfc6980-frag-5) | A node silently ignores a Router Advertisement in a packet with a Fragment header. |
| [RFC6980-FRAG-6](#rfc6980-frag-6) | A node silently ignores a Redirect in a packet with a Fragment header. |

## Fragmentation of Neighbor Discovery messages

### RFC6980-FRAG-1

**A node does not use IPv6 fragmentation to send a Neighbor Solicitation, Neighbor Advertisement, Router Solicitation, Router Advertisement or Redirect.**

> "Nodes MUST NOT employ IPv6 fragmentation for sending any of the
> following Neighbor Discovery and SEcure Neighbor Discovery messages:" — §5,
> `rfc6980.txt:312-313`, and "o  Neighbor Solicitation
>
> o  Neighbor Advertisement
>
> o  Router Solicitation
>
> o  Router Advertisement
>
> o  Redirect" — §5, `rfc6980.txt:315-323`

- Strength: must not. Class: wire.
- Check idea: a router on a link with MTU 1280 has so many prefixes and options that they do
  not fit in one Router Advertisement. No Router Advertisement on the link has a Fragment
  header. Also, no Neighbor Solicitation, Neighbor Advertisement, Router Solicitation or
  Redirect on the link has a Fragment header.

### RFC6980-FRAG-2

**A node silently ignores a Neighbor Solicitation in a packet with a Fragment header.**

> "Nodes MUST silently ignore the following Neighbor Discovery and
> SEcure Neighbor Discovery messages if the packets carrying them
> include an IPv6 Fragmentation Header:" — §5, `rfc6980.txt:343-345`, and
> "o  Neighbor Solicitation" — §5, `rfc6980.txt:347`

- Strength: must. Class: end-to-end.
- Check idea: host B sends host A a Neighbor Solicitation for an address of host A in a
  packet with a Fragment header: an atomic fragment, and in a second run two fragments. Host
  A sends no Neighbor Advertisement.

### RFC6980-FRAG-3

**A node silently ignores a Neighbor Advertisement in a packet with a Fragment header.**

> "Nodes MUST silently ignore the following Neighbor Discovery and
> SEcure Neighbor Discovery messages if the packets carrying them
> include an IPv6 Fragmentation Header:" — §5, `rfc6980.txt:343-345`, and
> "o  Neighbor Advertisement" — §5, `rfc6980.txt:349`

- Strength: must. Class: end-to-end.
- Check idea: host A sends a Neighbor Solicitation for address Y of host B. Host B answers
  with a Neighbor Advertisement in a packet with a Fragment header. Host A does not send the
  queued packet to host B, and it sends another Neighbor Solicitation for Y.

### RFC6980-FRAG-4

**A node silently ignores a Router Solicitation in a packet with a Fragment header.**

> "Nodes MUST silently ignore the following Neighbor Discovery and
> SEcure Neighbor Discovery messages if the packets carrying them
> include an IPv6 Fragmentation Header:" — §5, `rfc6980.txt:343-345`, and
> "o  Router Solicitation" — §5, `rfc6980.txt:351`

- Strength: must. Class: end-to-end.
- Check idea: host B sends a Router Solicitation in a packet with a Fragment header to a
  router with a long advertisement interval. The router sends no Router Advertisement in
  answer; only its periodic Router Advertisements appear on the link.

### RFC6980-FRAG-5

**A node silently ignores a Router Advertisement in a packet with a Fragment header.**

> "Nodes MUST silently ignore the following Neighbor Discovery and
> SEcure Neighbor Discovery messages if the packets carrying them
> include an IPv6 Fragmentation Header:" — §5, `rfc6980.txt:343-345`, and
> "o  Router Advertisement" — §5, `rfc6980.txt:353`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with prefix P, A = 1, and a nonzero
  Router Lifetime in a packet with a Fragment header. Host A forms no address from P, and it
  sends no off-link packet to that router.

### RFC6980-FRAG-6

**A node silently ignores a Redirect in a packet with a Fragment header.**

> "Nodes MUST silently ignore the following Neighbor Discovery and
> SEcure Neighbor Discovery messages if the packets carrying them
> include an IPv6 Fragmentation Header:" — §5, `rfc6980.txt:343-345`, and
> "o  Redirect" — §5, `rfc6980.txt:355`

- Strength: must. Class: end-to-end.
- Check idea: the default router of host A sends it a Redirect for destination D, with a
  better first hop T, in a packet with a Fragment header. Host A still sends its packets to D
  through the default router.

## Out of scope in this catalog

- **The sections outside §5**: §1 to §4 (introduction, background, rationale) and §6 to §9.
- **SEND**, RFC 3971: out of the in-scope set.

Within §5, these parts have no entry, with the reason:

All the parts left out concern SEcure Neighbor Discovery (SEND, RFC 3971), which is not in
the in-scope set:

- Certification Path Solicitation in the list of messages that a node must not fragment,
  `rfc6980.txt:325`. The rule itself is RFC6980-FRAG-1, for the five Neighbor Discovery
  messages.
- The SHOULD NOT on fragmentation of the Certification Path Advertisement,
  `rfc6980.txt:327-330`.
- Certification Path Solicitation in the list of messages that a node must ignore when
  fragmented, `rfc6980.txt:357`. The rule itself is RFC6980-FRAG-2 to RFC6980-FRAG-6.
- The SHOULD on normal processing of a fragmented Certification Path Advertisement,
  `rfc6980.txt:359-362`.
- The SHOULD NOT on keys that make Certification Path Advertisements too large and so
  fragmented, `rfc6980.txt:364-365`.
