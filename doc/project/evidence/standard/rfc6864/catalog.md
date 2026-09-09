# RFC 6864 (IPv4 identification field) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6864-*` · **Stands on:** [standards.md](../../protocol/ipv4/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 6864, which updates RFC 791 and RFC 1122. The catalog comes from the
RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc6864.txt` — Updated Specification of the IPv4 ID Field, February 2013. Downloaded
  2026-09-09 from <https://www.rfc-editor.org/rfc/rfc6864.txt>.

RFC 6864 narrows the meaning of the identification field to fragmentation and reassembly,
relaxes the uniqueness rule for datagrams that cannot be fragmented, and keeps it for the
rest. Its §4 holds the requirements, each marked with `>>`. The document defines two kinds
of datagram, and every entry below depends on the distinction:

> "Atomic datagrams are datagrams not yet fragmented and for which further fragmentation
> has been inhibited." — §4, `rfc6864.txt:320-321`
> "Non-atomic datagrams are datagrams either that already have been fragmented or for which
> fragmentation remains possible." — `rfc6864.txt:323-324`
> "Atomic datagrams: (DF==1)&&(MF==0)&&(frag_offset==0)" — `rfc6864.txt:331`
> "Non-atomic datagrams: (DF==0)||(MF==1)||(frag_offset>0)" — `rfc6864.txt:333`

Quotes are verbatim. A reference such as `rfc6864.txt:354` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv4/coverage.md`](../../model/ipv4/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC6864-ID-1](#rfc6864-id-1) | The identification field serves fragmentation and reassembly only. |
| [RFC6864-ID-2](#rfc6864-id-2) | A source may set any identification on an atomic datagram. |
| [RFC6864-ID-3](#rfc6864-id-3) | Every device ignores the identification of an atomic datagram. |
| [RFC6864-ID-4](#rfc6864-id-4) | A copy of a non-atomic datagram does not reuse its identification. |
| [RFC6864-ID-5](#rfc6864-id-5) | Non-atomic identifications are unique per tuple within one MDL. |
| [RFC6864-ID-6](#rfc6864-id-6) | A datagram with DF set is not fragmented. |
| [RFC6864-ID-7](#rfc6864-id-7) | A transit device does not clear DF. |

The conventions of an entry — strength, class, and the `Overridden by` field — are the
ones of [`rfc1122/catalog.md`](../rfc1122/catalog.md#how-to-read-an-entry). A **Governs**
line names the RFC 791 or RFC 1122 entry that this entry replaces.

## Identification

### RFC6864-ID-1

**The identification field serves fragmentation and reassembly only.**

> ">> The IPv4 ID field MUST NOT be used for purposes other than fragmentation and
> reassembly." — §4.1, `rfc6864.txt:354-355`

- Strength: must not. Class: internal (a rule about what a node reads the field for).
- Note: a protocol test cannot see what a node does not do with a field. The observable
  consequence is [RFC6864-ID-3](#rfc6864-id-3).

### RFC6864-ID-2

**A source may set the identification of an atomic datagram to any value.**

> ">> Originating sources MAY set the IPv4 ID field of atomic datagrams to any value." —
> §4.1, `rfc6864.txt:367-368`

- Strength: may. Class: wire.
- Note: a source that keeps unique values on atomic datagrams over-satisfies the rule; a
  source that repeats a value on them is within the rule.

### RFC6864-ID-3

**Every device that examines an IPv4 header ignores the identification of an atomic
datagram.**

> ">> All devices that examine IPv4 headers MUST ignore the IPv4 ID field of atomic
> datagrams." — §4.1, `rfc6864.txt:375-376`

- Strength: must. Class: end-to-end.
- Check idea: deliver two atomic datagrams (DF set, unfragmented) with the same source,
  destination, protocol and identification, close together. The destination delivers both.

### RFC6864-ID-4

**A copy of an earlier non-atomic datagram does not reuse its identification.**

> ">> The IPv4 ID of non-atomic datagrams MUST NOT be reused when sending a copy of an
> earlier non-atomic datagram." — §4.2, `rfc6864.txt:421-422`

- Strength: must not. Class: wire.
- Governs: [RFC1122-ID-1](../rfc1122/catalog.md#rfc1122-id-1).
- Check idea: force a retransmission of a fragmentable datagram (a TCP segment); the copy
  carries a different identification. This needs a transport that retransmits, so it is
  a candidate for the TCP set as much as for this one.

### RFC6864-ID-5

**A source of non-atomic datagrams does not repeat an identification within one maximum
datagram lifetime for a given source, destination and protocol.**

> ">> Sources emitting non-atomic datagrams MUST NOT repeat IPv4 ID values within one MDL
> for a given source address/destination address/protocol tuple." — §4.3,
> `rfc6864.txt:438-440`

- Strength: must not. Class: wire.
- Governs: [RFC791-ID-1](../rfc791/catalog.md#rfc791-id-1). The rule is the RFC 791 rule
  restricted to non-atomic datagrams and bounded by the MDL.
- Check idea: two fragmentable datagrams of one flow, sent close together, carry two
  different identifications. The full property over one MDL is a count over many datagrams,
  which is level 4 work.

### RFC6864-ID-6

**A datagram whose DF bit is set is not fragmented.**

> ">> IPv4 datagrams whose DF=1 MUST NOT be fragmented." — §4.3, `rfc6864.txt:458`

- Strength: must not. Class: wire.
- Governs: [RFC791-FRAG-5](../rfc791/catalog.md#rfc791-frag-5), which states the same
  rule as prose.

### RFC6864-ID-7

**A device that forwards a datagram does not clear the DF bit.**

> ">> IPv4 datagram transit devices MUST NOT clear the DF bit." — §4.3,
> `rfc6864.txt:460`

- Strength: must not. Class: wire.
- Check idea: a datagram with DF set that fits every link carries DF set on the link after
  the gateway.

## Out of scope in this catalog

§5, the impact discussion, and §6.3, the update to IP-in-IP tunnels (RFC 2003). The
document has no other requirements.
