# DHCP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around DHCP for IPv4, records which document governs each contested clause, and pins the
exact set that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-11.

## Target level

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set below holds the base document, the companion that defines the options
without which no DHCP message is legal, and the one later document that corrects an edge
rule of the base text.

DHCP has no host-requirements document of its own. RFC 1122 predates it and states nothing
about DHCP. RFC 2131 is itself the host specification: it states the client behavior and
the server behavior in two long sections, with the keywords of section 1.4. The document
that plays the part RFC 1122 plays for UDP is RFC 6842, which changes a `MUST NOT` of
RFC 2131 into a `MUST` after ten years of operational experience.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 2131, RFC 2132 | the four-message exchange and the options it needs; no DHCP message is legal without the `DHCP message type` option of RFC 2132 §9.6 |
| level 3, Edge | RFC 6842 | in scope now. It reverses the rule that a server must not echo the `client identifier` option, and it adds a discard rule for the client |
| level 4, Dynamics | nothing new | in scope already. The timers of DHCP — T1, T2, the lease, and the randomized exponential backoff — are all in RFC 2131 §4.1 and §4.4.5. Level 4 for DHCP is a matter of tolerances, not of documents |
| level 5, Complete | RFC 951, RFC 1542, RFC 3396, RFC 4361 | the BOOTP base, the relay agent, long options, and the DUID form of the client identifier |

What the pass actually reached is not recorded here. It is in
[`model/dhcp/coverage.md`](../../model/dhcp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 2131 | Dynamic Host Configuration Protocol | March 1997 | Draft Standard | `base`; obsoletes RFC 1541 | [`standard/rfc2131/`](../../standard/rfc2131/rfc2131.txt), 2026-09-11 |
| RFC 2132 | DHCP Options and BOOTP Vendor Extensions | March 1997 | Draft Standard | `companion`; obsoletes RFC 1533. RFC 2131 §3 delegates the whole option set to it | [`standard/rfc2132/`](../../standard/rfc2132/rfc2132.txt), 2026-09-11 |
| RFC 6842 | Client Identifier Option in DHCP Server Replies | January 2013 | Proposed Standard | `updates` RFC 2131 | [`standard/rfc6842/`](../../standard/rfc6842/rfc6842.txt), 2026-09-11 |
| RFC 951 | Bootstrap Protocol | September 1985 | Draft Standard | `companion`; RFC 2131 §3 takes the message format from it | no |
| RFC 1542 | Clarifications and Extensions for the Bootstrap Protocol | October 1993 | Draft Standard | `companion`; `updates` RFC 951. RFC 2131 §4.1 delegates the meaning of the BROADCAST bit and all relay agent behavior to it | no |
| RFC 3396 | Encoding Long Options in DHCPv4 | November 2002 | Proposed Standard | `updates` RFC 2131 and RFC 2132 | no |
| RFC 4361 | Node-specific Client Identifiers for DHCPv4 | February 2006 | Proposed Standard | `updates` RFC 2131 and RFC 2132 | no |
| RFC 5494 | IANA Allocation Guidelines for the ARP | April 2009 | Proposed Standard | `updates` RFC 2131 and RFC 2132 | no |
| RFC 3442, RFC 3942, RFC 4833 | classless static route; reclassifying option codes; timezone options | 2002 to 2007 | Proposed Standard | `updates` RFC 2132 | no |

Source of the cached text:

- `rfc2131.txt` in [`evidence/standard/rfc2131/`](../../standard/rfc2131/) —
  <https://www.rfc-editor.org/rfc/rfc2131.txt>, downloaded 2026-09-11.
- `rfc2132.txt` in [`evidence/standard/rfc2132/`](../../standard/rfc2132/) —
  <https://www.rfc-editor.org/rfc/rfc2132.txt>, downloaded 2026-09-11.
- `rfc6842.txt` in [`evidence/standard/rfc6842/`](../../standard/rfc6842/) —
  <https://www.rfc-editor.org/rfc/rfc6842.txt>, downloaded 2026-09-11.

Each of the three documents gets its own catalog file. No other protocol in this tree uses
them, so no catalog here is shared, unlike the RFC 792 and RFC 1122 catalogs that IPv4,
UDP and TCP share.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The `client identifier` option in a server reply | RFC 2131 table 3, `rfc2131.txt:1584`: `Client identifier` is `MUST NOT` in DHCPOFFER and DHCPACK and `MAY` in DHCPNAK | RFC 6842 §3, `rfc6842.txt:158-160` and `rfc6842.txt:175-180`: when the client sent one, the server `MUST` return it unaltered in all three; when the client sent none, `MUST NOT` | RFC 6842 | yes, from level 3 |
| What a client does with a foreign `client identifier` | RFC 2131 states nothing; the client matches only on `xid` | RFC 6842 §3, `rfc6842.txt:183-186`: the client `MUST` compare and `MUST` discard in silence when the two differ | RFC 6842 | yes, from level 3 |
| The meaning of the BROADCAST bit | RFC 2131 §4.1, `rfc2131.txt:1362-1385`, states the client rule and the server rule | RFC 1542 discusses what follows from the bit | RFC 2131 for the bit itself | the RFC 2131 half only; RFC 1542 is out of scope |
| Options longer than 255 octets | RFC 2131 §4.1, `rfc2131.txt:1309-1314`: the client concatenates the values of several instances of one option | RFC 3396 defines the splitting and the reassembly exactly | RFC 3396 | no; level 5 |
| The form of the `client identifier` | RFC 2132 §9.14, `rfc2132.txt:1650-1656`: a hardware type and a hardware address, or type 0 | RFC 4361 replaces this with a DUID plus an IAID | RFC 4361 | no; level 5 |

The first two rows are the reason RFC 6842 is in the in-scope set at level 3. The first one
is a true reversal: a statement that RFC 2131 writes as a prohibition became a requirement.
The catalog keeps the RFC 2131 entry and its ID, and the entry carries `Overridden by`;
see [RFC2131-OFF-5](../../standard/rfc2131/catalog.md#rfc2131-off-5) and
[RFC2131-ACK-5](../../standard/rfc2131/catalog.md#rfc2131-ack-5), which point at
[RFC6842-CLID-1](../../standard/rfc6842/catalog.md#rfc6842-clid-1).

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 2131 | March 1997, Draft Standard, no revision of the body since | [`standard/rfc2131/catalog.md`](../../standard/rfc2131/catalog.md) |
| RFC 2132 | March 1997, Draft Standard; §2 and §9 in full, plus the two RFC 1497 extensions that §3 of RFC 2131 needs | [`standard/rfc2132/catalog.md`](../../standard/rfc2132/catalog.md) |
| RFC 6842 | January 2013, Proposed Standard | [`standard/rfc6842/catalog.md`](../../standard/rfc6842/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 951 | Level 5. RFC 2131 §3 restates the message format that it takes from RFC 951, so every field in scope has an RFC 2131 quote of its own. Nothing is lost until the BOOTP interworking of RFC 1534 comes in scope. |
| RFC 1542 | Level 5. Almost all of it states the behavior of a BOOTP relay agent, which is a third node type beside the client and the server. A test network without a relay agent cannot observe one statement of it. |
| RFC 3396 | Level 5. A long option is an encoding matter, and it needs an option whose value passes 255 octets. |
| RFC 4361 | Level 5. It changes what a client puts inside the `client identifier` option, not what the protocol does with the option. |
| RFC 5494 | It changes an IANA registry (the `htype` values), not a behavior. |
| RFC 3442, RFC 3942, RFC 4833 | Level 5. Each one defines or reclassifies one configuration option. The options that carry a parameter are the level 5 area of the RFC 2132 catalog. |

The two most important exclusions are RFC 1542 and the per-parameter half of RFC 2132.
Both are named again in the closing section of the catalogs, so that a later pass finds
them without reading this document.
