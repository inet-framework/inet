# RFC 8931 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8931-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 8931 in the selected 6LoWPAN family.
Source: [rfc8931.txt](rfc8931.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc8931.txt>. The publication version and family
relationships are pinned in the linked standards map.

The catalog is a scoped selection, not an exhaustive inventory or a claim of test
coverage. Its scope exclusions are listed at the end. No test-depth level is claimed.
Statements and check ideas are independent of any simulation implementation.

Quotes retain the wording and line breaks of the cached text; leading indentation
is omitted; excerpts may start or end within a line. Line references count physical
newline-delimited lines, including page
headers. Strength records the source keyword, including qualified permissions;
`description` denotes a format or procedure stated without a requirement keyword.
Each check idea is a proposed observation, not a test result. Conditional rules apply
only to the roles, modes and prerequisites named in their source paragraphs.

## Index

| ID | Statement |
| --- | --- |
| [RFC8931-FRAG-1](#rfc8931-frag-1) | Headers outside the first recoverable fragment are not compressed. |
| [RFC8931-FRAG-2](#rfc8931-frag-2) | Fragment sizes fit the path minimum MTU and the first fragment reserves slack for recompression. |
| [RFC8931-FRAG-3](#rfc8931-frag-3) | Recoverable-fragment sizes and offsets refer to the compressed datagram. |
| [RFC8931-FRAG-4](#rfc8931-frag-4) | Sequence zero identifies the first fragment and its offset field carries datagram size. |
| [RFC8931-FWD-1](#rfc8931-fwd-1) | Changing first-fragment size adjusts the encoded datagram size. |
| [RFC8931-FWD-2](#rfc8931-fwd-2) | The first-fragment size delta is stored and applied to subsequent offsets. |
| [RFC8931-ACK-1](#rfc8931-ack-1) | An acknowledgment-requested fragment triggers an RFRAG-ACK of received fragments. |
| [RFC8931-ACK-2](#rfc8931-ack-2) | ARQ use requires X on the last fragment. |
| [RFC8931-ARQ-1](#rfc8931-arq-1) | ARQ uses a retransmission timeout. |
| [RFC8931-ECN-1](#rfc8931-ecn-1) | The receiver echoes a received ECN mark at most once in the next acknowledgment. |
| [RFC8931-ARQ-2](#rfc8931-arq-2) | All fragments are sent once before retries, and lost fragments are retried oldest first. |
| [RFC8931-FWD-3](#rfc8931-fwd-3) | The first fragment contains the entire IPv6 header for routing. |
| [RFC8931-FWD-4](#rfc8931-fwd-4) | Retrying a datagram with smaller fragments aborts the original attempt and uses a new tag. |
| [RFC8931-ACK-3](#rfc8931-ack-3) | An acknowledgment without reverse-path state is silently dropped. |
| [RFC8931-ABORT-1](#rfc8931-abort-1) | A later fragment without matching VRB elicits an abort acknowledgment. |
| [RFC8931-ABORT-2](#rfc8931-abort-2) | A NULL bitmap aborts transmission, except for the permitted first-fragment alternate-hop retry. |
| [RFC8931-PARAM-1](#rfc8931-param-1) | Window size is between 1 and 32 inclusive. |
| [RFC8931-PARAM-2](#rfc8931-param-2) | Configured protocol parameter bounds are respected. |

## Checkable statements

### RFC8931-FRAG-1

**Headers outside the first recoverable fragment are not compressed.**

> To be consistent with Section 2 of [RFC6282], for the fragmentation
> mechanism described in Section 5.3 of [RFC4944], any header that
> cannot fit within the first fragment MUST NOT be compressed when
> using the fragmentation mechanism described in this specification.

— §3, `rfc8931.txt:266-269`.

- Strength: must not. Class: encoding.
- Check idea: Use a long header chain and verify headers outside the first fragment remain uncompressed.

### RFC8931-FRAG-2

**Fragment sizes fit the path minimum MTU and the first fragment reserves slack for recompression.**

> This specification cannot allow that refragmentation operation since
> the fragments are recovered end to end based on a sequence number.
> The Fragment_Size MUST be tailored to fit the minimal MTU along the
> path, and the first fragment that contains a 6LoWPAN compressed
> header MUST have enough slack to enable a less-efficient compression
> in the next hops to still fit within the Link MTU.

— §4.1, `rfc8931.txt:287-292`.

- Strength: must. Class: wire.
- Check idea: Forward across a path that expands a compressed address and verify the first fragment still fits.

### RFC8931-FRAG-3

**Recoverable-fragment sizes and offsets refer to the compressed datagram.**

> In this specification, if the packet is compressed, the size and
> offset of the fragments are expressed with respect to the compressed
> form of the packet, as opposed to the uncompressed (native) form.

— §5.1, `rfc8931.txt:407-409`.

- Strength: description. Class: encoding.
- Check idea: Compare encoded positions with compressed bytes and distinguish them from RFC 4944 uncompressed offsets.

### RFC8931-FRAG-4

**Sequence zero identifies the first fragment and its offset field carries datagram size.**

> The first fragment is recognized by a Sequence of 0; it carries its
> Fragment_Size and the Datagram_Size of the compressed packet before
> it is fragmented, whereas the other fragments carry their
> Fragment_Size and Fragment_Offset.  The last fragment for a datagram
> is recognized when its Fragment_Offset and its Fragment_Size add up
> to the stored Datagram_Size of the packet identified by the sender
> link-layer address and the Datagram_Tag.

— §5.1, `rfc8931.txt:422-428`.

- Strength: description. Class: encoding.
- Check idea: Decode first and later fragments and check the field's conditional interpretation and last-fragment detection.

### RFC8931-FWD-1

**Changing first-fragment size adjusts the encoded datagram size.**

> The compression of the hop limit, of the source and destination
> addresses in the IPv6 header, and of the Routing Header, which are
> all in the first fragment, may change en route in a route-over mesh
> LLN.  If the size of the first fragment is modified, then the
> intermediate node MUST adapt the Datagram_Size, encoded in the
> Fragment_Size field, to reflect that difference.

— §4.4, `rfc8931.txt:353-358`.

- Strength: must. Class: encoding.
- Check idea: Force recompression to change the first fragment length and inspect the forwarded size field.

### RFC8931-FWD-2

**The first-fragment size delta is stored and applied to subsequent offsets.**

> The intermediate node MUST also save the difference of Datagram_Size
> of the first fragment in the VRB and add it to the Fragment_Offset of
> all the subsequent fragments that it forwards for that datagram.  In
> the case of a Source Routing Header 6LoWPAN Routing Header (SRH-
> 6LoRH) [RFC8138] being consumed and thus reduced, that difference is
> negative, meaning that the Fragment_Offset is decremented by the
> number of bytes that were consumed.

— §4.4, `rfc8931.txt:360-366`.

- Strength: must. Class: wire.
- Check idea: Expand the first fragment and verify the same offset adjustment on every later fragment.

### RFC8931-ACK-1

**An acknowledgment-requested fragment triggers an RFRAG-ACK of received fragments.**

> The fragmenting endpoint may set the Ack-Request flag on any fragment
> to perform congestion control by limiting the number of outstanding
> fragments, which are the fragments that have been sent but for which
> reception or loss was not positively confirmed by the reassembling
> endpoint.  The maximum number of outstanding fragments is controlled
> by the Window-Size.  It is configurable and may vary in case of ECN
> notification.  When the endpoint that reassembles the packets at the
> 6LoWPAN level receives a fragment with the Ack-Request flag set, it
> MUST send an RFRAG-ACK back to the originator to confirm reception of
> all the fragments it has received so far.

— §6, `rfc8931.txt:601-610`.

- Strength: must. Class: wire.
- Check idea: Lose selected fragments and deliver an X-set fragment; check the returned bitmap.

### RFC8931-ACK-2

**ARQ use requires X on the last fragment.**

> The Ack-Request ("X") set in an RFRAG marks the end of a window.
> This flag MUST be set on the last fragment if the fragmenting
> endpoint wishes to perform an automatic repeat request (ARQ) process
> for the datagram, and it MAY be set in any intermediate fragment for
> the purpose of congestion control.

— §6, `rfc8931.txt:612-616`.

- Strength: must; may. Class: wire.
- Check idea: Enable ARQ and inspect the final fragment; allow optional intermediate requests.

### RFC8931-ARQ-1

**ARQ uses a retransmission timeout.**

> This ARQ process MUST be protected by a Retransmission Timeout (RTO)
> timer, and the fragment that carries the "X" flag MAY be retried upon
> a timeout for a configurable number of times (see Section 7.1) with
> an exponential backoff.  Upon exhaustion of the retries, the
> fragmenting endpoint may either abort the transmission of the
> datagram or resend the first fragment with an "X" flag set in order
> to establish a new path for the datagram and obtain the list of
> fragments that were received over the old path in the acknowledgment
> bitmap.  When the fragmenting endpoint knows that an underlying link-
> layer mechanism protects the fragments, it may refrain from using the
> RFRAG Acknowledgment mechanism and never set the Ack-Request bit.

— §6, `rfc8931.txt:618-628`.

- Strength: must; may. Class: internal.
- Check idea: Lose an acknowledgment and check timeout protection and the configured limit on retries of the X-set fragment.

### RFC8931-ECN-1

**The receiver echoes a received ECN mark at most once in the next acknowledgment.**

> The RFRAG Acknowledgment carries an ECN indication for congestion
> control (see Appendix C).  The reassembling endpoint of a fragment
> with the "E" (ECN) flag set MUST echo that information at most once
> by setting the "E" (ECN) flag in the next RFRAG-ACK.

— §6, `rfc8931.txt:637-640`.

- Strength: must. Class: wire.
- Check idea: Deliver a marked fragment and inspect the next and following ACK ECN flags.

### RFC8931-ARQ-2

**All fragments are sent once before retries, and lost fragments are retried oldest first.**

> Fragments MUST be sent in a round-robin fashion: the sender MUST send
> all the fragments for a first time before it retries any lost
> fragment; lost fragments MUST be retried in sequence, oldest first.
> This mechanism enables the receiver to acknowledge fragments that
> were delayed in the network before they are retried.

— §6, `rfc8931.txt:680-684`.

- Strength: must. Class: wire.
- Check idea: Lose multiple fragments and verify initial-pass completion and retry order.

### RFC8931-FWD-3

**The first fragment contains the entire IPv6 header for routing.**

> The IPv6 header MUST be placed in the first fragment in full to
> enable the routing decision.  The first fragment is routed and
> creates an LSP from the fragmenting endpoint to the reassembling
> endpoint.  The next fragments are label switched along that LSP.  As
> a consequence, the next fragments can only follow the path that was
> set up by the first fragment; they cannot follow an alternate route.
> The Datagram_Tag is used to carry the label, which is swapped in each
> hop.

— §6.1, `rfc8931.txt:702-709`.

- Strength: must. Class: encoding.
- Check idea: Check the first fragment of a routed datagram contains the full header in its applicable encoded form.

### RFC8931-FWD-4

**Retrying a datagram with smaller fragments aborts the original attempt and uses a new tag.**

> If the first fragment is too large for the path MTU, it will
> repeatedly fail and never establish an LSP.  In that case, the
> fragmenting endpoint MAY retry the same datagram with a smaller
> Fragment_Size, in which case it MUST abort the original attempt and
> use a new Datagram_Tag for the new attempt.

— §6.1, `rfc8931.txt:711-715`.

- Strength: must; may. Class: wire.
- Check idea: Trigger the whole-datagram smaller-size retry and inspect abort signaling and the new tag; distinguish individual-fragment splitting.

### RFC8931-ACK-3

**An acknowledgment without reverse-path state is silently dropped.**

> If the reverse LSP is not found, the router MUST silently drop the
> RFRAG-ACK message.

— §6.2, `rfc8931.txt:787-788`.

- Strength: must. Class: wire.
- Check idea: Inject an RFRAG-ACK with no matching reverse state and check that it is not forwarded.

### RFC8931-ABORT-1

**A later fragment without matching VRB elicits an abort acknowledgment.**

> If the VRB for the tuple is not found, the router builds an RFRAG-ACK
> to abort the transmission of the packet.  The resulting message has
> the following information:
>
> *  The source and destination link-layer addresses are swapped from
> those found in the fragment, and the same interface is used
>
> *  The Datagram_Tag is set to the Datagram_Tag found in the fragment
>
> *  A NULL bitmap is used to signal the abort condition

— §6.1.2, `rfc8931.txt:753-762`.

- Strength: description. Class: error-signal.
- Check idea: Inject a nonzero-sequence fragment without state and check the NULL bitmap, reflected tag and reverse link addresses.

### RFC8931-ABORT-2

**A NULL bitmap aborts transmission, except for the permitted first-fragment alternate-hop retry.**

> The RFRAG-ACK is forwarded all the way back to the source of the
> packet and cleans up all resources on the path.  Upon an
> acknowledgment with a NULL bitmap, the fragmenting endpoint MUST
> abort the transmission of the fragmented datagram with one exception:
> in the particular case of the first fragment, it MAY decide to retry
> via an alternate next hop instead.

— §6.3, `rfc8931.txt:844-849`.

- Strength: must; may. Class: wire.
- Check idea: Return a NULL bitmap after a later fragment and check cessation; exercise the first-fragment exception separately.

### RFC8931-PARAM-1

**Window size is between 1 and 32 inclusive.**

> Window_Size:  The Window_Size MUST be at least 1 and less than 33.

— §7.1, `rfc8931.txt:947-947`.

- Strength: must. Class: internal.
- Check idea: Exercise both valid bounds and reject or constrain configurations outside them.

### RFC8931-PARAM-2

**Configured protocol parameter bounds are respected.**

> The management system SHOULD be capable of providing the parameters
> listed in this section, and an implementation MUST abide by those
> parameters and, in particular, never exceed the minimum and maximum
> configured boundaries.

— §7.1, `rfc8931.txt:894-897`.

- Strength: must. Class: internal.
- Check idea: Set distinct retry, timer and size bounds and verify runtime adaptation never exceeds them.

## Source interpretation note

RFC 8931 §4.4 (quoted in [RFC8931-FWD-1](#rfc8931-fwd-1)) says the
datagram size is encoded in `Fragment_Size`, whereas §5.1 (quoted in
[RFC8931-FRAG-4](#rfc8931-frag-4)) assigns the first fragment’s datagram
size to `Fragment_Offset`. The quotes preserve this textual inconsistency.
Resolve the applicable errata before deriving an assertion about the wire field
from §4.4; its required size adjustment and the §5.1 field definition are separate
statements here.

## Areas outside this selection

This catalog selects recoverable fragmentation, size adjustment, ACK/ARQ behavior, aborts and selected parameter bounds. Complete dispatch and bitmap bit layouts (§5), reverse-path construction and completed-datagram grace timers (§6), diverse paths (§6.4), all parameter algorithms (§7), security prerequisites (§8), registry allocations (§9) and the illustrative congestion/window algorithms in the appendices are not exhaustively cataloged. Timing checks require explicit link and topology assumptions; these entries do not establish congestion-control completeness.
