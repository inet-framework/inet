# RFC 5494 (ARP number spaces) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC5494-*` · **Stands on:** [standards.md](../../protocol/arp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 5494, IANA Allocation Guidelines for the Address Resolution Protocol
of April 2009, which updates RFC 826. The catalog comes from the RFC text only. It contains
no simulation model names and no code references.

Source, cached in this folder:

- `rfc5494.txt` — IANA Allocation Guidelines for the Address Resolution Protocol (ARP),
  April 2009. Downloaded 2026-09-11 from <https://www.rfc-editor.org/rfc/rfc5494.txt>.

The document is four pages of substance and it asks nothing of a host. It gives IANA the
rules for allocating new values in three fields of the ARP packet, and it allocates six
numbers itself: two hardware space values and two opcodes for experiments, and the two
boundary values of both spaces as reserved. The catalog is therefore short, and every entry
is `description` — the document carries no RFC 2119 keyword about the behaviour of an
implementation.

What the document does give a test is a **defined** value outside the two opcodes of
RFC 826. Before RFC 5494 a packet with opcode 24 was an unassigned number and a test with
one could be read as a test with a corruption. After RFC 5494 it is an opcode reserved for
experiments, so a host that is not part of the experiment meets a well-formed packet whose
opcode it does not handle. What the host then does is the rule of RFC 826; see
[RFC826-RECV-10](../rfc826/catalog.md#rfc826-recv-10).

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/arp/standards.md). The features these statements build are
in [`features.md`](../../protocol/arp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`arp/coverage.md`](../../model/arp/coverage.md).

Quotes are verbatim. A reference such as `rfc5494.txt:181-183` points to a line of the
cached file in this folder.

## Index

| ID | Statement |
| --- | --- |
| [RFC5494-NUM-1](#rfc5494-num-1) | In both the hardware space and the opcode, 0 and 65535 are reserved. |
| [RFC5494-NUM-2](#rfc5494-num-2) | Two hardware space values are allocated for experiments: 36 and 256. |
| [RFC5494-NUM-3](#rfc5494-num-3) | Two opcodes are allocated for experiments: 24 and 25. |
| [RFC5494-NUM-4](#rfc5494-num-4) | The protocol space of ARP shares the Ethertype space. |
| [RFC5494-PROC-1](#rfc5494-proc-1) | New values in the three spaces follow named allocation policies. |

## How to read an entry

The conventions of an entry — strength, class, and the `Overridden by` field — are the ones
of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry).

## The number spaces

### RFC5494-NUM-1

**In both the hardware space and the opcode, 0 and 65535 are reserved.**

> "In addition, for both ar$hrd and ar$op, the values 0 and 65535 are marked as reserved.
> This means that they are not available for allocation." — §3, `rfc5494.txt:181-183`

- Strength: description. Class: encoding.
- Governs: the field widths of [RFC826-FMT-1](../rfc826/catalog.md#rfc826-fmt-1). RFC 826
  gives each field 16 bits and names one value in each; this entry closes the two ends of
  both ranges.
- Check idea: no host ever sends either value in either field. A check reads the two fields
  of every ARP packet a host puts on the link.

### RFC5494-NUM-2

**Two hardware space values are allocated for experiments: 36 and 256.**

> "Two new ar$hrd values are allocated for experimental purposes: HW_EXP1 (36) and HW_EXP2
> (256). Note that these two new values were purposely chosen so that one would be below 256
> and the other would be above 255, and so that there would be different values in the least
> and most significant octets." — §3, `rfc5494.txt:162-166`

- Strength: description. Class: wire.
- Check idea: the value gives a packet a hardware space that a host on an Ethernet link
  does not have, and therefore a defined input for the first question of the reception
  algorithm of RFC 826, [RFC826-RECV-2](../rfc826/catalog.md#rfc826-recv-2).

### RFC5494-NUM-3

**Two opcodes are allocated for experiments: 24 and 25.**

> "Two new values for the ar$op are allocated for experimental purposes: OP_EXP1 (24) and
> OP_EXP2 (25)." — §3, `rfc5494.txt:175-176`

- Strength: description. Class: wire.
- Check idea: the value gives a packet an opcode that is neither a request nor a reply, and
  therefore a defined input for the opcode question of the reception algorithm of RFC 826,
  [RFC826-RECV-10](../rfc826/catalog.md#rfc826-recv-10). A host that is not part of the
  experiment answers nothing and keeps working.

### RFC5494-NUM-4

**The protocol space of ARP shares the Ethertype space.**

> "ar$pro (16 bits) Protocol address space / These numbers share the Ethertype space. The
> Ethertype space is administered as described in [RFC5342]." — §2, `rfc5494.txt:145-148`

- Strength: description. Class: encoding.
- Governs: [RFC826-GEN-1](../rfc826/catalog.md#rfc826-gen-1), which says the field "may no
  longer correspond to the Ethernet type field" on other hardware. RFC 5494 settles the
  question for the whole space: the numbers are Ethertype numbers whatever the hardware is.
- Check idea: the protocol space field of a packet that resolves an IPv4 address carries
  the Ethertype of IPv4.

### RFC5494-PROC-1

**New values in the three spaces follow named allocation policies.**

> "Requests for ar$hrd values below 256 or for a batch of more than one new value are made
> through Expert Review [RFC5226]." — §2, `rfc5494.txt:125-126`

> "Requests for new ar$op values are made through IETF Review or IESG Approval [RFC5226]."
> — §2, `rfc5494.txt:152-153`

- Strength: description. Class: internal; the statement binds IANA and not an
  implementation.
- Overrides: the Notes of RFC 826, `rfc826.txt:68-75`, which asked that requests for
  hardware space values go to the author of the document by network mail. That procedure
  ended with this document.
- Check idea: none. No behaviour of a host follows from an allocation policy, and no test
  can observe one.

## Out of scope in this catalog

The whole of §4, the security considerations, `rfc5494.txt:185-195`. Its first sentence says
that the document changes no security property. The rest warns a person who runs an
experiment to evaluate the side effects before the experiment leaves a network that the
person controls. Both are advice to a reader, not behaviour of a host.

The introduction, `rfc5494.txt:63-105`, lists the eleven other documents that take numbers
from these spaces, among them BOOTP, DHCP and the ARP variants for ATM and Fibre Channel.
The list matters for the reach of the update and states nothing about ARP itself.

One thing this document does **not** say is worth recording, because a reader looks for it:
it states no rule for what a host does with a value it does not know, in any of the three
fields. That rule lives in the reception algorithm of RFC 826, and every check of an
unknown value therefore targets an RFC 826 entry and cites this document only for the value
it used.
