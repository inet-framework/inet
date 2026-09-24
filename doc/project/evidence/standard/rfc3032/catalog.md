# RFC 3032 (MPLS Label Stack Encoding) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC3032-*` · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 3032, MPLS Label Stack Encoding, of
January 2001, a document of the in-scope set of MPLS. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc3032.txt`](../../../../../../standards/RFC/rfc3032.txt) —
  MPLS Label Stack Encoding, January 2001. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc3032.txt>.

The architecture is in [`rfc3031/catalog.md`](../rfc3031/catalog.md); RFC 3443 and RFC 5462
update this document, and have catalogs of their own:
[`rfc3443/catalog.md`](../rfc3443/catalog.md) and [`rfc5462/catalog.md`](../rfc5462/catalog.md).
The in-scope sections are §2 to §5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mpls/standards.md). The features these statements build are in
[`features.md`](../../protocol/mpls/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mpls/coverage.md`](../../model/mpls/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc3032.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC3032-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 3032 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them (`must` for
  REQUIRED), and `description` for a normative sentence with no keyword. Where the text writes a
  keyword in lower case, the entry records the word and says so, for example `must (lower
  case)`. An entry with two keywords names what each one covers. A step of a numbered procedure
  that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of a header on a link, or
  the presence or absence of a packet), `end-to-end` (what an implementation accepts, delivers
  or discards, seen in what it sends or delivers afterwards), `error-signal` (an ICMP error
  report), `internal` (state or a database inside an implementation, an audit log), `encoding`
  (the exact bit layout, sizes and alignment).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC3032-ENC-1](#rfc3032-enc-1) | A label stack entry is 4 octets long. |
| [RFC3032-ENC-2](#rfc3032-enc-2) | A label stack entry holds, in bit order, the Label field, the Exp field, the S bit, and the TTL field. |
| [RFC3032-ENC-3](#rfc3032-enc-3) | The Label field is 20 bits wide and carries the label value. |
| [RFC3032-ENC-4](#rfc3032-enc-4) | The Exp field is 3 bits wide and is reserved for experimental use. |
| [RFC3032-ENC-5](#rfc3032-enc-5) | The S bit is 1 bit wide. |
| [RFC3032-ENC-6](#rfc3032-enc-6) | The TTL field is 8 bits wide and encodes a time-to-live value. |
| [RFC3032-ENC-7](#rfc3032-enc-7) | The label stack entries sit after the data link layer headers and before the network layer header. |
| [RFC3032-ENC-8](#rfc3032-enc-8) | In the label stack, the top entry appears earliest in the packet and the bottom entry appears latest. |
| [RFC3032-ENC-9](#rfc3032-enc-9) | The network layer packet immediately follows the label stack entry that has the S bit set. |
| [RFC3032-ENC-10](#rfc3032-enc-10) | The S bit is one in the last entry of the label stack and zero in every other entry. |
| [RFC3032-ENC-11](#rfc3032-enc-11) | A label value of 0 is the IPv4 Explicit NULL Label: legal only at the bottom of the stack, and it requires the label to be popped and forwarding to continue on the IPv4 header. |
| [RFC3032-ENC-12](#rfc3032-enc-12) | A label value of 1 is the Router Alert Label: legal anywhere in the stack except the bottom, and a packet that carries it at the top is delivered to a local software module, with forwarding decided by the label beneath it. |
| [RFC3032-ENC-13](#rfc3032-enc-13) | If an LSR forwards a packet further after local processing of the Router Alert Label, it pushes the Router Alert Label back onto the stack first. |
| [RFC3032-ENC-14](#rfc3032-enc-14) | A label value of 2 is the IPv6 Explicit NULL Label: legal only at the bottom of the stack, and it requires the label to be popped and forwarding to continue on the IPv6 header. |
| [RFC3032-ENC-15](#rfc3032-enc-15) | A label value of 3 is the Implicit NULL Label: when an LSR would otherwise replace the top label with it, the LSR pops the stack instead. |
| [RFC3032-ENC-16](#rfc3032-enc-16) | Label values 4 through 15 are reserved. |
| [RFC3032-NLP-1](#rfc3032-nlp-1) | After the last label is popped from a packet, further processing of the packet is based on the network layer header. |
| [RFC3032-NLP-2](#rfc3032-nlp-2) | The LSR that pops the last label of a packet must be able to identify the packet's network layer protocol. |
| [RFC3032-NLP-3](#rfc3032-nlp-3) | Because the label stack has no field that names the network layer protocol, the protocol's identity must be inferable from the value of the bottom label, together with the network layer header if needed. |
| [RFC3032-NLP-4](#rfc3032-nlp-4) | When an LSR pushes the first label onto a network layer packet, it uses a label value that is used only for one network layer, or only for a set of network layers whose packets can be told apart by their header. |
| [RFC3032-NLP-5](#rfc3032-nlp-5) | When an LSR replaces a label during a packet's transit, the new label value must meet the same network-layer-identifiability criterion as the original label. |
| [RFC3032-NLP-6](#rfc3032-nlp-6) | If intermediate LSRs are to generate protocol-specific error messages for labeled packets, every label in the stack, not only the bottom one, must meet the network-layer-identifiability criterion. |
| [RFC3032-NLP-7](#rfc3032-nlp-7) | An LSR silently discards a packet it cannot forward when either the network layer protocol cannot be identified, or no protocol-dependent rule covers the error. |
| [RFC3032-ICMP-1](#rfc3032-icmp-1) | An LSR generates an ICMP message for a labeled IP packet only if it can determine that the packet is an IP packet. |
| [RFC3032-ICMP-2](#rfc3032-icmp-2) | An LSR generates an ICMP message for a labeled IP packet only if it can route to the packet's IP source address. |
| [RFC3032-ICMP-3](#rfc3032-icmp-3) | To send an ICMP message toward the source of a packet whose source address is not routable directly, an LSR may copy the label stack from the original packet onto the ICMP message and label switch it. |
| [RFC3032-ICMP-4](#rfc3032-icmp-4) | When an LSR copies a label stack from the original packet onto an ICMP message, it copies the label values exactly. |
| [RFC3032-ICMP-5](#rfc3032-icmp-5) | When an LSR copies a label stack from the original packet onto an ICMP message, it sets the copied entries' TTL values to the TTL value placed in the ICMP message's own IP header. |
| [RFC3032-ICMP-6](#rfc3032-icmp-6) | The TTL value placed in a label-switched ICMP message is long enough for the message's circuitous route to the source. |
| [RFC3032-TTL-1](#rfc3032-ttl-1) | The incoming TTL of a labeled packet is the value of the TTL field of the top label stack entry when the packet is received. |
| [RFC3032-TTL-2](#rfc3032-ttl-2) | The outgoing TTL of a labeled packet is the larger of one less than the incoming TTL, or zero. |
| [RFC3032-TTL-3](#rfc3032-ttl-3) | If a labeled packet's outgoing TTL is zero, the LSR does not forward it further, whether labeled or with the label stack stripped off. |
| [RFC3032-TTL-4](#rfc3032-ttl-4) | Depending on the label value, an LSR may simply discard a packet with an outgoing TTL of zero, or pass it to the network layer for error processing. |
| [RFC3032-TTL-5](#rfc3032-ttl-5) | When an LSR forwards a labeled packet, it sets the TTL field of the top label stack entry to the outgoing TTL value. |
| [RFC3032-TTL-6](#rfc3032-ttl-6) | The outgoing TTL depends only on the incoming TTL, not on how many labels are pushed or popped before forwarding. |
| [RFC3032-TTL-7](#rfc3032-ttl-7) | The TTL field of a label stack entry that is not at the top of the stack carries no significant value. |
| [RFC3032-TTL-8](#rfc3032-ttl-8) | The IP TTL field is the IPv4 TTL field, or the IPv6 Hop Limit field, whichever applies. |
| [RFC3032-TTL-9](#rfc3032-ttl-9) | When an IP packet is first labeled, the TTL field of the new label stack entry is set to the value of the IP TTL field. |
| [RFC3032-TTL-10](#rfc3032-ttl-10) | When a label is popped and the resulting label stack is empty, the IP TTL field is replaced with the outgoing TTL value, and for IPv4 the header checksum is updated accordingly. |
| [RFC3032-FRAG-1](#rfc3032-frag-1) | Pushing an additional label onto a packet grows its frame payload by 4 octets per label pushed. |
| [RFC3032-FRAG-2](#rfc3032-frag-2) | An LSR that can receive an unlabeled IP datagram, add a label stack to it, and forward the result, supports a configuration parameter, the Maximum Initially Labeled IP Datagram Size, settable to a non-negative value. |
| [RFC3032-FRAG-3](#rfc3032-frag-3) | When the Maximum Initially Labeled IP Datagram Size parameter is set to zero, it has no effect. |
| [RFC3032-FRAG-4](#rfc3032-frag-4) | When the Maximum Initially Labeled IP Datagram Size parameter is set to a positive value, an LSR fragments an oversized unlabeled datagram without the DF bit set before labeling it, keeping each fragment no larger than the parameter, and labels and forwards each fragment. |
| [RFC3032-FRAG-5](#rfc3032-frag-5) | Setting the Maximum Initially Labeled IP Datagram Size parameter to a non-zero value everywhere it applies removes fragmentation of previously labeled datagrams, though it may add unnecessary fragmentation of initially labeled datagrams. |
| [RFC3032-FRAG-6](#rfc3032-frag-6) | The Maximum Initially Labeled IP Datagram Size parameter does not change the processing of datagrams with the DF bit set, so Path MTU Discovery's result is unaffected by its setting. |
| [RFC3032-FRAG-7](#rfc3032-frag-7) | A labeled IP datagram larger than the Conventional Maximum Frame Payload Size of the outgoing data link may be considered too big. |
| [RFC3032-FRAG-8](#rfc3032-frag-8) | A labeled IP datagram larger than the True Maximum Frame Payload Size of the outgoing data link is always too big. |
| [RFC3032-FRAG-9](#rfc3032-frag-9) | A labeled IP datagram that is not too big is transmitted without fragmentation. |
| [RFC3032-FRAG-10](#rfc3032-frag-10) | An LSR may silently discard a too-big labeled IPv4 datagram whose DF bit is not set. |
| [RFC3032-FRAG-11](#rfc3032-frag-11) | When an LSR does not discard a too-big labeled IPv4 datagram without the DF bit set, it fragments it, keeping each fragment at least N bytes below the Effective Maximum Frame Payload Size, where N is the label stack's byte length. |
| [RFC3032-FRAG-12](#rfc3032-frag-12) | The LSR prepends each fragment of a too-big labeled IPv4 datagram without the DF bit set with the same label header the original datagram would have carried, and forwards the fragments. |
| [RFC3032-FRAG-13](#rfc3032-frag-13) | A too-big labeled IPv4 datagram with the DF bit set is not forwarded. |
| [RFC3032-FRAG-14](#rfc3032-frag-14) | When an LSR does not forward a too-big labeled IPv4 datagram with the DF bit set, it creates an ICMP Destination Unreachable message with code Fragmentation Required and DF Set, and a Next-Hop MTU field equal to the Effective Maximum Frame Payload Size minus N. |
| [RFC3032-FRAG-15](#rfc3032-frag-15) | The LSR transmits the ICMP Destination Unreachable message to the source of a discarded too-big labeled IPv4 datagram, if possible. |
| [RFC3032-FRAG-16](#rfc3032-frag-16) | To process a too-big labeled IPv6 datagram that is larger than 1280 bytes, or has no fragment header, the LSR creates an ICMP Packet Too Big message with a Next-Hop MTU field equal to the Effective Maximum Frame Payload Size minus N. |
| [RFC3032-FRAG-17](#rfc3032-frag-17) | The LSR transmits the ICMP Packet Too Big message to the source, if possible, and discards the too-big labeled IPv6 datagram. |
| [RFC3032-FRAG-18](#rfc3032-frag-18) | When a too-big labeled IPv6 datagram is not larger than 1280 octets and has a fragment header, the LSR fragments it, keeping each fragment at least N bytes below the Effective Maximum Frame Payload Size. |
| [RFC3032-FRAG-19](#rfc3032-frag-19) | The LSR prepends each fragment of such an IPv6 datagram with the same label header the original datagram would have carried, forwards the fragments, and leaves reassembly to the destination host. |
| [RFC3032-FRAG-20](#rfc3032-frag-20) | When an LSR at the transmitting end of a tunnel cannot forward an ICMP message to a packet's source from inside the tunnel, but can otherwise receive packets destined through it, that LSR determines the tunnel's MTU as a whole, for example by running Path MTU Discovery through the tunnel. |
| [RFC3032-FRAG-21](#rfc3032-frag-21) | When a packet with the DF bit set exceeds the tunnel MTU, the transmitting end of the tunnel sends an ICMP Destination Unreachable message, coded Fragmentation Required and DF Set, with the Next-Hop MTU field set as for a too-big labeled IPv4 datagram. |
| [RFC3032-PPP-1](#rfc3032-ppp-1) | PPP sends MPLS Control Protocol packets to enable the transmission of labeled packets after LCP establishes the link. |
| [RFC3032-PPP-2](#rfc3032-ppp-2) | PPP does not exchange MPLS Control Protocol packets before it reaches the Network-Layer Protocol phase. |
| [RFC3032-PPP-3](#rfc3032-ppp-3) | An implementation silently discards an MPLS Control Protocol packet it receives before PPP reaches the Network-Layer Protocol phase. |
| [RFC3032-PPP-4](#rfc3032-ppp-4) | An MPLS Control Protocol packet may carry any frame format modification negotiated during Link Establishment. |
| [RFC3032-PPP-5](#rfc3032-ppp-5) | The PPP Information field carries exactly one MPLS Control Protocol packet, and the PPP Protocol field holds hex 8281 for it. |
| [RFC3032-PPP-6](#rfc3032-ppp-6) | An MPLS Control Protocol packet uses only Code values 1 through 7. |
| [RFC3032-PPP-7](#rfc3032-ppp-7) | An implementation treats an MPLS Control Protocol packet with any other Code as unrecognized and answers it with a Code-Reject. |
| [RFC3032-PPP-8](#rfc3032-ppp-8) | An implementation waits for Authentication and Link Quality Determination to finish before it times out a Configure-Ack wait. |
| [RFC3032-PPP-9](#rfc3032-ppp-9) | An implementation gives up waiting for a response only after user intervention or after a configurable amount of time. |
| [RFC3032-PPP-10](#rfc3032-ppp-10) | The MPLS Control Protocol defines no configuration option types. |
| [RFC3032-PPP-11](#rfc3032-ppp-11) | PPP sends no labeled packet until it reaches the Network-Layer Protocol phase and the MPLS Control Protocol reaches the Opened state. |
| [RFC3032-PPP-12](#rfc3032-ppp-12) | The PPP Information field carries exactly one labeled packet. |
| [RFC3032-PPP-13](#rfc3032-ppp-13) | The PPP Protocol field holds hex 0281 to mark a labeled packet as an MPLS unicast packet. |
| [RFC3032-PPP-14](#rfc3032-ppp-14) | The PPP Protocol field holds hex 0283 to mark a labeled packet as an MPLS multicast packet. |
| [RFC3032-PPP-15](#rfc3032-ppp-15) | The maximum length of a labeled packet sent over a PPP link equals the maximum length of the Information field of a PPP encapsulated packet. |
| [RFC3032-PPP-16](#rfc3032-ppp-16) | The PPP Information field carries the label stack entries in the format section 2 defines. |
| [RFC3032-LAN-1](#rfc3032-lan-1) | Each frame carries exactly one labeled packet. |
| [RFC3032-LAN-2](#rfc3032-lan-2) | The label stack entries immediately precede the network layer header and follow any data link layer headers, including an 802.1Q header. |
| [RFC3032-LAN-3](#rfc3032-lan-3) | The ethertype value 8847 hex marks a frame as carrying an MPLS unicast packet. |
| [RFC3032-LAN-4](#rfc3032-lan-4) | The ethertype value 8848 hex marks a frame as carrying an MPLS multicast packet. |
| [RFC3032-LAN-5](#rfc3032-lan-5) | Both the MPLS ethertype values can appear with either the ethernet encapsulation or the 802.3 LLC/SNAP encapsulation. |

## Encoding the label stack

### RFC3032-ENC-1

**A label stack entry is 4 octets long.**

> "Each label stack entry is represented by 4 octets." — §2.1, `rfc3032.txt:149`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry on a link and check that it occupies exactly 4
  octets.

### RFC3032-ENC-2

**A label stack entry holds, in bit order, the Label field, the Exp field, the S bit, and the TTL field.**

> "|                Label                  | Exp |S|       TTL     |" — Figure 1, `rfc3032.txt:155`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry on a link and check that its bits appear in this
  order: Label, Exp, S, TTL.

### RFC3032-ENC-3

**The Label field is 20 bits wide and carries the label value.**

> "Label:  Label Value, 20 bits" and "This 20-bit field carries the actual value of the
> Label." — Figure 1, `rfc3032.txt:158`, `rfc3032.txt:200`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry and check that its Label field occupies 20 bits and
  carries the label value.

### RFC3032-ENC-4

**The Exp field is 3 bits wide and is reserved for experimental use.**

> "Exp:    Experimental Use, 3 bits" and "This three-bit field is reserved for experimental
> use." — Figure 1, `rfc3032.txt:159`, `rfc3032.txt:196`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry and check that its Exp field occupies 3 bits.

### RFC3032-ENC-5

**The S bit is 1 bit wide.**

> "S:      Bottom of Stack, 1 bit" — Figure 1, `rfc3032.txt:160`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry and check that its S field occupies exactly 1 bit.

### RFC3032-ENC-6

**The TTL field is 8 bits wide and encodes a time-to-live value.**

> "TTL:    Time to Live, 8 bits" and "This eight-bit field is used to encode a time-to-live
> value." — Figure 1, `rfc3032.txt:161`, `rfc3032.txt:191`.

- Strength: description. Class: encoding.
- Check idea: capture a label stack entry and check that its TTL field occupies 8 bits.

### RFC3032-ENC-7

**The label stack entries sit after the data link layer headers and before the network layer header.**

> "The label stack entries appear AFTER the data link layer headers, but
>    BEFORE any network layer headers." — §2.1, `rfc3032.txt:175-176`.

- Strength: description. Class: wire.
- Check idea: capture a labeled frame on a link and check that the label stack entries sit
  after the data link layer headers and before the network layer header.

### RFC3032-ENC-8

**In the label stack, the top entry appears earliest in the packet and the bottom entry appears latest.**

> "The top of the label stack appears
>    earliest in the packet, and the bottom appears latest." — §2.1, `rfc3032.txt:176-177`.

- Strength: description. Class: wire.
- Check idea: capture a labeled packet with two or more label stack entries and check that the
  top entry appears before the bottom entry in the packet.

### RFC3032-ENC-9

**The network layer packet immediately follows the label stack entry that has the S bit set.**

> "The network
>    layer packet immediately follows the label stack entry which has the
>    S bit set." — §2.1, `rfc3032.txt:177-179`.

- Strength: description. Class: wire.
- Check idea: capture a labeled packet and check that the network layer header starts
  immediately after the label stack entry whose S bit is set.

### RFC3032-ENC-10

**The S bit is one in the last entry of the label stack and zero in every other entry.**

> "This bit is set to one for the last entry in the label stack
>          (i.e., for the bottom of the stack), and zero for all other
>          label stack entries." — §2.1, `rfc3032.txt:185-187`.

- Strength: description. Class: wire.
- Check idea: capture a labeled packet with several label stack entries and check that the S
  bit is one only in the bottom entry and zero in every entry above it.

### RFC3032-ENC-11

**A label value of 0 is the IPv4 Explicit NULL Label: legal only at the bottom of the stack, and it requires the label to be popped and forwarding to continue on the IPv4 header.**

> "A value of 0 represents the "IPv4 Explicit NULL Label".
>              This label value is only legal at the bottom of the label
>              stack.  It indicates that the label stack must be popped,
>              and the forwarding of the packet must then be based on the
>              IPv4 header." — §2.1, `rfc3032.txt:233-237`.

- Strength: must (lower case). Class: wire.
- Check idea: send a packet with label value 0 at the bottom of the stack and check that the
  receiving LSR pops the label and forwards the packet on its IPv4 header.

### RFC3032-ENC-12

**A label value of 1 is the Router Alert Label: legal anywhere in the stack except the bottom, and a packet that carries it at the top is delivered to a local software module, with forwarding decided by the label beneath it.**

> "A value of 1 represents the "Router Alert Label".  This
>              label value is legal anywhere in the label stack except at
>              the bottom.  When a received packet contains this label
>              value at the top of the label stack, it is delivered to a
>              local software module for processing.  The actual
>              forwarding of the packet is determined by the label
>              beneath it in the stack." — §2.1, `rfc3032.txt:239-245`.

- Strength: description. Class: end-to-end.
- Check idea: send a packet with label value 1 at the top of the stack, not at the bottom, and
  check that the LSR delivers it to a local module while forwarding is decided by the label
  beneath it.

### RFC3032-ENC-13

**If an LSR forwards a packet further after local processing of the Router Alert Label, it pushes the Router Alert Label back onto the stack first.**

> "if the packet is
>              forwarded further, the Router Alert Label should be pushed
>              back onto the label stack before forwarding." — §2.1, `rfc3032.txt:245-247`.

- Strength: should (lower case). Class: wire.
- Check idea: send a packet with the Router Alert Label at the top, have the LSR forward it
  further after local processing, and check that the outgoing packet still carries the Router
  Alert Label on the stack.

### RFC3032-ENC-14

**A label value of 2 is the IPv6 Explicit NULL Label: legal only at the bottom of the stack, and it requires the label to be popped and forwarding to continue on the IPv6 header.**

> "A value of 2 represents the "IPv6 Explicit NULL Label".
>              This label value is only legal at the bottom of the label
>              stack.  It indicates that the label stack must be popped,
>              and the forwarding of the packet must then be based on the
>              IPv6 header." — §2.1, `rfc3032.txt:253-257`.

- Strength: must (lower case). Class: wire.
- Check idea: send a packet with label value 2 at the bottom of the stack and check that the
  receiving LSR pops the label and forwards the packet on its IPv6 header.

### RFC3032-ENC-15

**A label value of 3 is the Implicit NULL Label: when an LSR would otherwise replace the top label with it, the LSR pops the stack instead.**

> "A value of 3 represents the "Implicit NULL Label".  This
>              is a label that an LSR may assign and distribute, but
>              which never actually appears in the encapsulation.  When
>              an LSR would otherwise replace the label at the top of the
>              stack with a new label, but the new label is "Implicit
>              NULL", the LSR will pop the stack instead of doing the
>              replacement." — §2.1, `rfc3032.txt:259-265`.

- Strength: may (lower case). Class: internal.
- Check idea: assign the Implicit NULL Label as the next label for a packet and check that the
  LSR pops the top label stack entry instead of replacing it.

### RFC3032-ENC-16

**Label values 4 through 15 are reserved.**

> "Values 4-15 are reserved." — §2.1, `rfc3032.txt:269`.

- Strength: description. Class: wire.
- Check idea: capture label stack entries on a link and check that none of them carries a
  label value from 4 through 15, other than as later assigned.

## Determining the network layer protocol

### RFC3032-NLP-1

**After the last label is popped from a packet, further processing of the packet is based on the network layer header.**

> "When the last label is popped from a packet's label stack (resulting
>    in the stack being emptied), further processing of the packet is
>    based on the packet's network layer header." — §2.2, `rfc3032.txt:273-275`.

- Strength: description. Class: end-to-end.
- Check idea: send a packet whose last label is popped at an LSR and check that the LSR then
  processes the packet using its network layer header.

### RFC3032-NLP-2

**The LSR that pops the last label of a packet must be able to identify the packet's network layer protocol.**

> "The LSR which pops the
>    last label off the stack must therefore be able to identify the
>    packet's network layer protocol." — §2.2, `rfc3032.txt:275-277`.

- Strength: must (lower case). Class: internal.
- Check idea: send a labeled packet whose last label is popped at an LSR and check that the LSR
  determines the packet's network layer protocol.

### RFC3032-NLP-3

**Because the label stack has no field that names the network layer protocol, the protocol's identity must be inferable from the value of the bottom label, together with the network layer header if needed.**

> "the label stack does not
>    contain any field which explicitly identifies the network layer
>    protocol.  This means that the identity of the network layer protocol
>    must be inferable from the value of the label which is popped from
>    the bottom of the stack, possibly along with the contents of the
>    network layer header itself." — §2.2, `rfc3032.txt:277-278`, `rfc3032.txt:287-290`.

- Strength: must (lower case). Class: internal.
- Check idea: pop the bottom label of a packet at an LSR and check that the LSR determines the
  network layer protocol from the label value alone, or from the label value together with the
  network layer header.

### RFC3032-NLP-4

**When an LSR pushes the first label onto a network layer packet, it uses a label value that is used only for one network layer, or only for a set of network layers whose packets can be told apart by their header.**

> "Therefore, when the first label is pushed onto a network layer
>    packet, either the label must be one which is used ONLY for packets
>    of a particular network layer, or the label must be one which is used
>    ONLY for a specified set of network layer protocols, where packets of
>    the specified network layers can be distinguished by inspection of
>    the network layer header." — §2.2, `rfc3032.txt:292-297`.

- Strength: must (lower case). Class: internal.
- Check idea: have an LSR push the first label onto an unlabeled network layer packet and check
  that the assigned label value is dedicated to one network layer, or to a set of network
  layers whose packets can be told apart by their header.

### RFC3032-NLP-5

**When an LSR replaces a label during a packet's transit, the new label value must meet the same network-layer-identifiability criterion as the original label.**

> "Furthermore, whenever that label is
>    replaced by another label value during a packet's transit, the new
>    value must also be one which meets the same criteria." — §2.2, `rfc3032.txt:297-299`.

- Strength: must (lower case). Class: internal.
- Check idea: have an LSR replace a label during a packet's transit and check that the new
  label value still meets the network-layer-identifiability criterion the original label met.

### RFC3032-NLP-6

**If intermediate LSRs are to generate protocol-specific error messages for labeled packets, every label in the stack, not only the bottom one, must meet the network-layer-identifiability criterion.**

> "The only means the
>    intermediate LSR has for identifying the network layer is inspection
>    of the top label and the network layer header.  So if intermediate
>    nodes are to be able to generate protocol-specific error messages for
>    labeled packets, all labels in the stack must meet the criteria
>    specified above for labels which appear at the bottom of the stack." — §2.2, `rfc3032.txt:310-315`.

- Strength: must (lower case). Class: internal.
- Check idea: configure a network where intermediate LSRs generate protocol-specific error
  messages for labeled packets and check that every label in the stack, not only the bottom
  one, meets the network-layer-identifiability criterion.

### RFC3032-NLP-7

**An LSR silently discards a packet it cannot forward when either the network layer protocol cannot be identified, or no protocol-dependent rule covers the error.**

> "If a packet cannot be forwarded for some reason (e.g., it exceeds the
>    data link MTU), and either its network layer protocol cannot be
>    identified, or there are no specified protocol-dependent rules for
>    handling the error condition, then the packet MUST be silently
>    discarded." — §2.2, `rfc3032.txt:317-321`.

- Strength: must. Class: end-to-end.
- Check idea: send an undeliverable labeled packet whose network layer protocol cannot be
  identified, or for which no protocol-dependent error rule exists, and check that the LSR
  discards it and sends no error message.

## ICMP messages for labeled IP packets

### RFC3032-ICMP-1

**An LSR generates an ICMP message for a labeled IP packet only if it can determine that the packet is an IP packet.**

> "it must be possible for that LSR to determine that a particular
>         labeled packet is an IP packet" — §2.3, `rfc3032.txt:330-331`.

- Strength: must (lower case). Class: error-signal.
- Check idea: present an undeliverable labeled IP packet to an LSR and check that it generates
  an ICMP message only when it can first determine that the packet is an IP packet.

### RFC3032-ICMP-2

**An LSR generates an ICMP message for a labeled IP packet only if it can route to the packet's IP source address.**

> "it must be possible for that LSR to route to the packet's IP
>         source address." — §2.3, `rfc3032.txt:333-334`.

- Strength: must (lower case). Class: error-signal.
- Check idea: present an undeliverable labeled IP packet to an LSR whose source address it can,
  and separately cannot, route to, and check that it generates the ICMP message only in the
  first case.

### RFC3032-ICMP-3

**To send an ICMP message toward the source of a packet whose source address is not routable directly, an LSR may copy the label stack from the original packet onto the ICMP message and label switch it.**

> "one can copy the label stack from the original packet to
>    the ICMP message, and then label switch the ICMP message." — §2.3.2, `rfc3032.txt:389-390`.

- Strength: may (lower case). Class: error-signal.
- Check idea: have an LSR generate an ICMP message for a packet whose source address it cannot
  route to directly, and check that the LSR copies the original packet's label stack onto the
  ICMP message and label switches it.

### RFC3032-ICMP-4

**When an LSR copies a label stack from the original packet onto an ICMP message, it copies the label values exactly.**

> "the label values must be copied exactly" — §2.3.2, `rfc3032.txt:411`.

- Strength: must (lower case). Class: error-signal.
- Check idea: have an LSR build an ICMP message by copying the original packet's label stack
  and check that every copied label value matches the original packet's label value exactly.

### RFC3032-ICMP-5

**When an LSR copies a label stack from the original packet onto an ICMP message, it sets the copied entries' TTL values to the TTL value placed in the ICMP message's own IP header.**

> "the TTL values
>    in the label stack should be set to the TTL value that is placed in
>    the IP header of the ICMP message." — §2.3.2, `rfc3032.txt:411-413`.

- Strength: should (lower case). Class: error-signal.
- Check idea: have an LSR build an ICMP message by copying the original packet's label stack
  and check that each copied entry's TTL field equals the TTL value in the ICMP message's own
  IP header.

### RFC3032-ICMP-6

**The TTL value placed in a label-switched ICMP message is long enough for the message's circuitous route to the source.**

> "This TTL value should be long
>    enough to allow the circuitous route that the ICMP message will need
>    to follow." — §2.3.2, `rfc3032.txt:413-415`.

- Strength: should (lower case). Class: wire.
- Check idea: label switch an ICMP message over a circuitous route to the source and check that
  its TTL value is large enough for the message to reach the source.

## Processing the Time to Live field

### RFC3032-TTL-1

**The incoming TTL of a labeled packet is the value of the TTL field of the top label stack entry when the packet is received.**

> "The "incoming TTL" of a labeled packet is defined to be the value of
>    the TTL field of the top label stack entry when the packet is
>    received." — §2.4.1, `rfc3032.txt:428-430`.

- Strength: description. Class: encoding.
- Check idea: capture a labeled packet as it arrives at an LSR and check that its incoming TTL
  equals the TTL field of its top label stack entry.

### RFC3032-TTL-2

**The outgoing TTL of a labeled packet is the larger of one less than the incoming TTL, or zero.**

> "The "outgoing TTL" of a labeled packet is defined to be the larger
>    of:
>
>       a) one less than the incoming TTL,
>       b) zero." — §2.4.1, `rfc3032.txt:432-436`.

- Strength: description. Class: encoding.
- Check idea: present an LSR with a labeled packet of a known incoming TTL and check that the
  outgoing TTL equals the incoming TTL minus one, or zero if that would be negative.

### RFC3032-TTL-3

**If a labeled packet's outgoing TTL is zero, the LSR does not forward it further, whether labeled or with the label stack stripped off.**

> "If the outgoing TTL of a labeled packet is 0, then the labeled packet
>    MUST NOT be further forwarded; nor may the label stack be stripped
>    off and the packet forwarded as an unlabeled packet." — §2.4.2, `rfc3032.txt:440-442`.

- Strength: must not. Class: end-to-end.
- Check idea: send a labeled packet whose outgoing TTL computes to zero and check that the LSR
  neither forwards it as a labeled packet nor strips the label stack and forwards it unlabeled.

### RFC3032-TTL-4

**Depending on the label value, an LSR may simply discard a packet with an outgoing TTL of zero, or pass it to the network layer for error processing.**

> "Depending on the label value in the label stack entry, the packet MAY
>    be simply discarded, or it may be passed to the appropriate
>    "ordinary" network layer for error processing (e.g., for the
>    generation of an ICMP error message, see section 2.3)." — §2.4.2, `rfc3032.txt:455-458`.

- Strength: may. Class: end-to-end.
- Check idea: send a labeled packet whose outgoing TTL computes to zero and check that the LSR
  either discards it silently or passes it to the network layer for error processing.

### RFC3032-TTL-5

**When an LSR forwards a labeled packet, it sets the TTL field of the top label stack entry to the outgoing TTL value.**

> "When a labeled packet is forwarded, the TTL field of the label stack
>    entry at the top of the label stack MUST be set to the outgoing TTL
>    value." — §2.4.2, `rfc3032.txt:460-462`.

- Strength: must. Class: wire.
- Check idea: capture a labeled packet as an LSR forwards it and check that the top label stack
  entry's TTL field equals the outgoing TTL computed from the incoming TTL.

### RFC3032-TTL-6

**The outgoing TTL depends only on the incoming TTL, not on how many labels are pushed or popped before forwarding.**

> "the outgoing TTL value is a function solely of the incoming
>    TTL value, and is independent of whether any labels are pushed or
>    popped before forwarding." — §2.4.2, `rfc3032.txt:464-466`.

- Strength: description. Class: wire.
- Check idea: forward the same packet through two paths that push or pop a different number of
  labels and check that the outgoing top TTL value is the same in both cases.

### RFC3032-TTL-7

**The TTL field of a label stack entry that is not at the top of the stack carries no significant value.**

> "There is no significance to the value of
>    the TTL field in any label stack entry which is not at the top of the
>    stack." — §2.4.2, `rfc3032.txt:466-468`.

- Strength: description. Class: wire.
- Check idea: capture a labeled packet with more than one label stack entry and check that a
  receiver's processing does not depend on the TTL field of any entry other than the top one.

### RFC3032-TTL-8

**The IP TTL field is the IPv4 TTL field, or the IPv6 Hop Limit field, whichever applies.**

> "We define the "IP TTL" field to be the value of the IPv4 TTL field,
>    or the value of the IPv6 Hop Limit field, whichever is applicable." — §2.4.3, `rfc3032.txt:472-473`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv4 packet and an IPv6 packet at an LSR and check that the IP TTL
  field is read from the IPv4 TTL field or the IPv6 Hop Limit field respectively.

### RFC3032-TTL-9

**When an IP packet is first labeled, the TTL field of the new label stack entry is set to the value of the IP TTL field.**

> "When an IP packet is first labeled, the TTL field of the label stack
>    entry MUST BE set to the value of the IP TTL field." — §2.4.3, `rfc3032.txt:475-476`.

- Strength: must. Class: wire.
- Check idea: label an unlabeled IP packet at an LSR and check that the new label stack entry's
  TTL field equals the packet's IP TTL field.

### RFC3032-TTL-10

**When a label is popped and the resulting label stack is empty, the IP TTL field is replaced with the outgoing TTL value, and for IPv4 the header checksum is updated accordingly.**

> "When a label is popped, and the resulting label stack is empty, then
>    the value of the IP TTL field SHOULD BE replaced with the outgoing
>    TTL value, as defined above.  In IPv4 this also requires modification
>    of the IP header checksum." — §2.4.3, `rfc3032.txt:480-483`.

- Strength: should. Class: wire.
- Check idea: pop the last label of a packet at an LSR and check that the IP TTL field becomes
  the outgoing TTL value, and, for IPv4, that the header checksum matches the new TTL value.

## Fragmentation and path MTU discovery

### RFC3032-FRAG-1

**Pushing an additional label onto a packet grows its frame payload by 4 octets per label pushed.**

> "In label switching, a packet may grow
>    in size if additional labels get pushed on.  Thus if one receives a
>    labeled packet with a 1500-byte frame payload, and pushes on an
>    additional label, one needs to forward it as frame with a 1504-byte
>    payload." — §3, `rfc3032.txt:530-534`.

- Strength: may (lower case). Class: wire.
- Check idea: push one additional label onto a labeled packet at an LSR and check that the
  outgoing frame payload is 4 octets larger than the incoming frame payload.

### RFC3032-FRAG-2

**An LSR that can receive an unlabeled IP datagram, add a label stack to it, and forward the result, supports a configuration parameter, the Maximum Initially Labeled IP Datagram Size, settable to a non-negative value.**

> "Every LSR which is capable of
>
>       a) receiving an unlabeled IP datagram,
>       b) adding a label stack to the datagram, and
>       c) forwarding the resulting labeled packet,
>
>    SHOULD support a configuration parameter known as the "Maximum
>    Initially Labeled IP Datagram Size", which can be set to a non-
>    negative value." — §3.2, `rfc3032.txt:647-655`.

- Strength: should. Class: internal.
- Check idea: check that an LSR which receives unlabeled IP datagrams and labels them before
  forwarding offers a configuration parameter for the Maximum Initially Labeled IP Datagram
  Size, settable to a non-negative value.

### RFC3032-FRAG-3

**When the Maximum Initially Labeled IP Datagram Size parameter is set to zero, it has no effect.**

> "If this configuration parameter is set to zero, it has no effect." — §3.2, `rfc3032.txt:657`.

- Strength: description. Class: internal.
- Check idea: set the Maximum Initially Labeled IP Datagram Size parameter to zero at an LSR
  and check that it does not change how the LSR labels and forwards datagrams.

### RFC3032-FRAG-4

**When the Maximum Initially Labeled IP Datagram Size parameter is set to a positive value, an LSR fragments an oversized unlabeled datagram without the DF bit set before labeling it, keeping each fragment no larger than the parameter, and labels and forwards each fragment.**

> "If it is set to a positive value, it is used in the following way.
>    If:
>
>       a) an unlabeled IP datagram is received, and
>       b) that datagram does not have the DF bit set in its IP header,
>          and
>       c) that datagram needs to be labeled before being forwarded, and
>       d) the size of the datagram (before labeling) exceeds the value of
>          the parameter,
>    then
>       a) the datagram must be broken into fragments, each of whose size
>          is no greater than the value of the parameter, and
>
>       b) each fragment must be labeled and then forwarded." — §3.2, `rfc3032.txt:659-671`, `rfc3032.txt:679`.

- Strength: must (lower case). Class: wire.
- Check idea: set the Maximum Initially Labeled IP Datagram Size parameter to a positive value
  at an LSR, send it an unlabeled datagram without the DF bit set that needs labeling and
  exceeds the parameter, and check that every resulting fragment is no larger than the
  parameter, is labeled, and is forwarded.

### RFC3032-FRAG-5

**Setting the Maximum Initially Labeled IP Datagram Size parameter to a non-zero value everywhere it applies removes fragmentation of previously labeled datagrams, though it may add unnecessary fragmentation of initially labeled datagrams.**

> "setting this parameter to a non-zero value allows one
>    to eliminate all fragmentation of Previously Labeled IP Datagrams,
>    but it may cause some unnecessary fragmentation of Initially Labeled
>    IP Datagrams." — §3.2, `rfc3032.txt:688-691`.

- Strength: may (lower case). Class: wire.
- Check idea: configure the Maximum Initially Labeled IP Datagram Size parameter to a non-zero
  value network-wide and check that no previously labeled datagram is fragmented further,
  while an initially labeled datagram may be fragmented that would not otherwise need it.

### RFC3032-FRAG-6

**The Maximum Initially Labeled IP Datagram Size parameter does not change the processing of datagrams with the DF bit set, so Path MTU Discovery's result is unaffected by its setting.**

> "the setting of this parameter does not affect the
>    processing of IP datagrams that have the DF bit set; hence the result
>    of Path MTU discovery is unaffected by the setting of this parameter." — §3.2, `rfc3032.txt:693-695`.

- Strength: description. Class: end-to-end.
- Check idea: run Path MTU Discovery through an LSR under two different settings of the Maximum
  Initially Labeled IP Datagram Size parameter and check that the discovered MTU is the same
  in both cases.

### RFC3032-FRAG-7

**A labeled IP datagram larger than the Conventional Maximum Frame Payload Size of the outgoing data link may be considered too big.**

> "A labeled IP datagram whose size exceeds the Conventional Maximum
>    Frame Payload Size of the data link over which it is to be forwarded
>    MAY be considered to be "too big"." — §3.3, `rfc3032.txt:699-701`.

- Strength: may. Class: internal.
- Check idea: forward a labeled IP datagram larger than the outgoing link's Conventional Maximum
  Frame Payload Size and check that the LSR is permitted to treat it as too big.

### RFC3032-FRAG-8

**A labeled IP datagram larger than the True Maximum Frame Payload Size of the outgoing data link is always too big.**

> "A labeled IP datagram whose size exceeds the True Maximum Frame
>    Payload Size of the data link over which it is to be forwarded MUST
>    be considered to be "too big"." — §3.3, `rfc3032.txt:703-705`.

- Strength: must. Class: internal.
- Check idea: forward a labeled IP datagram larger than the outgoing link's True Maximum Frame
  Payload Size and check that the LSR always treats it as too big.

### RFC3032-FRAG-9

**A labeled IP datagram that is not too big is transmitted without fragmentation.**

> "A labeled IP datagram which is not "too big" MUST be transmitted
>    without fragmentation." — §3.3, `rfc3032.txt:707-708`.

- Strength: must. Class: wire.
- Check idea: forward a labeled IP datagram that is not too big for the outgoing link and check
  that it is transmitted in one piece.

### RFC3032-FRAG-10

**An LSR may silently discard a too-big labeled IPv4 datagram whose DF bit is not set.**

> "If a labeled IPv4 datagram is "too big", and the DF bit is not set in
>    its IP header, then the LSR MAY silently discard the datagram." — §3.4, `rfc3032.txt:712-713`.

- Strength: may. Class: end-to-end.
- Check idea: send a too-big labeled IPv4 datagram without the DF bit set and check that the LSR
  is permitted to discard it silently.

### RFC3032-FRAG-11

**When an LSR does not discard a too-big labeled IPv4 datagram without the DF bit set, it fragments it, keeping each fragment at least N bytes below the Effective Maximum Frame Payload Size, where N is the label stack's byte length.**

> "then it MUST
>    execute the following algorithm:" and "convert it into fragments, each of which MUST be at least N
>          bytes less than the Effective Maximum Frame Payload Size." — §3.4, `rfc3032.txt:721-722`, `rfc3032.txt:741-742`.

- Strength: must. Class: wire.
- Check idea: send a too-big labeled IPv4 datagram without the DF bit set to an LSR that does
  not discard it, and check that every resulting fragment is at least N bytes below the
  Effective Maximum Frame Payload Size of the outgoing link.

### RFC3032-FRAG-12

**The LSR prepends each fragment of a too-big labeled IPv4 datagram without the DF bit set with the same label header the original datagram would have carried, and forwards the fragments.**

> "Prepend each fragment with the same label header that would
>          have been on the original datagram had fragmentation not
>          been necessary." and "Forward the fragments" — §3.4, `rfc3032.txt:744-746`, `rfc3032.txt:748`.

- Strength: description. Class: wire.
- Check idea: fragment a too-big labeled IPv4 datagram without the DF bit set and check that
  each fragment carries the same label header the original datagram would have carried, and
  that the LSR forwards every fragment.

### RFC3032-FRAG-13

**A too-big labeled IPv4 datagram with the DF bit set is not forwarded.**

> "the datagram MUST NOT be forwarded" — §3.4, `rfc3032.txt:753`.

- Strength: must not. Class: end-to-end.
- Check idea: send a too-big labeled IPv4 datagram with the DF bit set to an LSR and check that
  it never forwards the datagram.

### RFC3032-FRAG-14

**When an LSR does not forward a too-big labeled IPv4 datagram with the DF bit set, it creates an ICMP Destination Unreachable message with code Fragmentation Required and DF Set, and a Next-Hop MTU field equal to the Effective Maximum Frame Payload Size minus N.**

> "Create an ICMP Destination Unreachable Message:
>
>              i. set its Code field [3] to "Fragmentation Required and DF
>                 Set",
>
>             ii. set its Next-Hop MTU field [4] to the difference between
>                 the Effective Maximum Frame Payload Size and the value
>                 of N" — §3.4, `rfc3032.txt:755-762`.

- Strength: description. Class: error-signal.
- Check idea: send a too-big labeled IPv4 datagram with the DF bit set and check that the LSR's
  ICMP Destination Unreachable message carries code Fragmentation Required and DF Set and a
  Next-Hop MTU field equal to the Effective Maximum Frame Payload Size minus N.

### RFC3032-FRAG-15

**The LSR transmits the ICMP Destination Unreachable message to the source of a discarded too-big labeled IPv4 datagram, if possible.**

> "If possible, transmit the ICMP Destination Unreachable
>             Message to the source of the of the discarded datagram." — §3.4, `rfc3032.txt:764-765`.

- Strength: description. Class: error-signal.
- Check idea: send a too-big labeled IPv4 datagram with the DF bit set over a path where the
  LSR can route to its source, and check that the LSR transmits the ICMP Destination
  Unreachable message there.

### RFC3032-FRAG-16

**To process a too-big labeled IPv6 datagram that is larger than 1280 bytes, or has no fragment header, the LSR creates an ICMP Packet Too Big message with a Next-Hop MTU field equal to the Effective Maximum Frame Payload Size minus N.**

> "To process a labeled IPv6 datagram which is too big, an LSR MUST
>    execute the following algorithm:" and "Create an ICMP Packet Too Big Message, and set its Next-Hop
>          MTU field to the difference between the Effective Maximum
>          Frame Payload Size and the value of N" — §3.5, `rfc3032.txt:769-770`, `rfc3032.txt:791-793`.

- Strength: must. Class: error-signal.
- Check idea: send a too-big labeled IPv6 datagram that is larger than 1280 bytes, or that has
  no fragment header, and check that the LSR's ICMP Packet Too Big message carries a Next-Hop
  MTU field equal to the Effective Maximum Frame Payload Size minus N.

### RFC3032-FRAG-17

**The LSR transmits the ICMP Packet Too Big message to the source, if possible, and discards the too-big labeled IPv6 datagram.**

> "If possible, transmit the ICMP Packet Too Big Message to the
>          source of the datagram." and "discard the labeled IPv6 datagram." — §3.5, `rfc3032.txt:795-796`, `rfc3032.txt:798`.

- Strength: description. Class: end-to-end.
- Check idea: send a too-big labeled IPv6 datagram that is larger than 1280 bytes, or that has
  no fragment header, over a path where the LSR can route to its source, and check that the
  LSR sends the ICMP Packet Too Big message there and discards the datagram.

### RFC3032-FRAG-18

**When a too-big labeled IPv6 datagram is not larger than 1280 octets and has a fragment header, the LSR fragments it, keeping each fragment at least N bytes below the Effective Maximum Frame Payload Size.**

> "Convert it into fragments, each of which MUST be at least N
>          bytes less than the Effective Maximum Frame Payload Size." — §3.5, `rfc3032.txt:803-804`.

- Strength: must. Class: wire.
- Check idea: send a too-big labeled IPv6 datagram that is not larger than 1280 octets and
  carries a fragment header, and check that every resulting fragment is at least N bytes below
  the Effective Maximum Frame Payload Size of the outgoing link.

### RFC3032-FRAG-19

**The LSR prepends each fragment of such an IPv6 datagram with the same label header the original datagram would have carried, forwards the fragments, and leaves reassembly to the destination host.**

> "Prepend each fragment with the same label header that would
>          have been on the original datagram had fragmentation not
>          been necessary." and "Forward the fragments." and "Reassembly of the fragments will be done at the destination
>          host." — §3.5, `rfc3032.txt:806-808`, `rfc3032.txt:810`, `rfc3032.txt:812-813`.

- Strength: description. Class: wire.
- Check idea: fragment a too-big labeled IPv6 datagram that carries a fragment header and check
  that each fragment carries the same label header the original datagram would have carried,
  that the LSR forwards every fragment, and that no LSR along the path reassembles them.

### RFC3032-FRAG-20

**When an LSR at the transmitting end of a tunnel cannot forward an ICMP message to a packet's source from inside the tunnel, but can otherwise receive packets destined through it, that LSR determines the tunnel's MTU as a whole, for example by running Path MTU Discovery through the tunnel.**

> "The LSR at the transmitting end of the tunnel MUST be able to
>       determine the MTU of the tunnel as a whole.  It MAY do this by
>       sending packets through the tunnel to the tunnel's receiving
>       endpoint, and performing Path MTU Discovery with those packets." — §3.6, `rfc3032.txt:853-856`.

- Strength: must (determining the tunnel MTU); may (the method used). Class: internal.
- Check idea: set up a tunnel whose transmitting-end LSR cannot forward ICMP messages to a
  packet's source from inside the tunnel, and check that the LSR still determines the tunnel's
  MTU as a whole, for instance by running Path MTU Discovery through the tunnel.

### RFC3032-FRAG-21

**When a packet with the DF bit set exceeds the tunnel MTU, the transmitting end of the tunnel sends an ICMP Destination Unreachable message, coded Fragmentation Required and DF Set, with the Next-Hop MTU field set as for a too-big labeled IPv4 datagram.**

> "Any time the transmitting endpoint of the tunnel needs to send
>       a packet into the tunnel, and that packet has the DF bit set,
>       and it exceeds the tunnel MTU, the transmitting endpoint of the
>       tunnel MUST send the ICMP Destination Unreachable message to
>       the source, with code "Fragmentation Required and DF Set", and
>       the Next-Hop MTU Field set as described above." — §3.6, `rfc3032.txt:858-863`.

- Strength: must. Class: error-signal.
- Check idea: send a packet with the DF bit set into a tunnel whose MTU it exceeds, and check
  that the transmitting-end LSR sends an ICMP Destination Unreachable message to the source,
  coded Fragmentation Required and DF Set, with the Next-Hop MTU field set accordingly.

## Labeled packets over PPP

### RFC3032-PPP-1

**PPP sends MPLS Control Protocol packets to enable the transmission of labeled packets after LCP establishes the link.**

> "After the link has been established and optional
> facilities have been negotiated as needed by the LCP, PPP must send
> "MPLS Control Protocol" packets to enable the transmission of labeled
> packets." — §4.1, `rfc3032.txt:890-893`

- Strength: must (lower case). Class: wire.
- Check idea: bring up LCP on a PPP link, then check that the sending side sends MPLS Control Protocol packets before it sends any labeled packet.

### RFC3032-PPP-2

**PPP does not exchange MPLS Control Protocol packets before it reaches the Network-Layer Protocol phase.**

> "MPLSCP
> packets may not be exchanged until PPP has reached the Network-Layer
> Protocol phase." — §4.2, `rfc3032.txt:912-914`

- Strength: must not. Class: wire.
- Check idea: attempt to send an MPLS Control Protocol packet before PPP reaches the Network-Layer Protocol phase, and check that no such packet appears on the link.

### RFC3032-PPP-3

**An implementation silently discards an MPLS Control Protocol packet it receives before PPP reaches the Network-Layer Protocol phase.**

> "MPLSCP packets received before this phase is reached
> should be silently discarded." — §4.2, `rfc3032.txt:914-915`

- Strength: should (lower case). Class: end-to-end.
- Check idea: send an MPLS Control Protocol packet to a PPP peer before PPP reaches the Network-Layer Protocol phase, and check that the peer sends no response and takes no other action.

### RFC3032-PPP-4

**An MPLS Control Protocol packet may carry any frame format modification negotiated during Link Establishment.**

> "The packet may utilize any modifications to the basic frame
> format which have been negotiated during the Link Establishment
> phase." — §4.2, `rfc3032.txt:922-924`

- Strength: may (lower case). Class: wire.
- Check idea: negotiate a frame format modification during Link Establishment, then send an MPLS Control Protocol packet, and check that it carries the negotiated modification.

### RFC3032-PPP-5

**The PPP Information field carries exactly one MPLS Control Protocol packet, and the PPP Protocol field holds hex 8281 for it.**

> "Exactly one MPLSCP packet is encapsulated in the PPP
> Information field, where the PPP Protocol field indicates type
> hex 8281 (MPLS)." — §4.2, `rfc3032.txt:928-930`

- Strength: description. Class: wire.
- Check idea: capture a PPP frame that carries an MPLS Control Protocol packet, and check that the Information field holds exactly one such packet and the Protocol field holds hex 8281.

### RFC3032-PPP-6

**An MPLS Control Protocol packet uses only Code values 1 through 7.**

> "Only Codes 1 through 7 (Configure-Request, Configure-Ack,
> Configure-Nak, Configure-Reject, Terminate-Request, Terminate-
> Ack and Code-Reject) are used." — §4.2, `rfc3032.txt:934-936`

- Strength: description. Class: wire.
- Check idea: capture every MPLS Control Protocol packet exchanged on a PPP link, and check that the Code field always holds a value from 1 through 7.

### RFC3032-PPP-7

**An implementation treats an MPLS Control Protocol packet with any other Code as unrecognized and answers it with a Code-Reject.**

> "Other Codes should be treated
> as unrecognized and should result in Code-Rejects." — §4.2, `rfc3032.txt:936-937`

- Strength: should (lower case). Class: wire.
- Check idea: send an MPLS Control Protocol packet whose Code field holds a value outside 1 through 7, and check that the receiver answers with a Code-Reject.

### RFC3032-PPP-8

**An implementation waits for Authentication and Link Quality Determination to finish before it times out a Configure-Ack wait.**

> "An implementation should be
> prepared to wait for Authentication and Link Quality
> Determination to finish before timing out waiting for a
> Configure-Ack or other response." — §4.2, `rfc3032.txt:942-945`

- Strength: should (lower case). Class: internal.
- Check idea: start Authentication and Link Quality Determination and delay their completion, and check that the implementation does not time out a Configure-Ack wait before they finish.

### RFC3032-PPP-9

**An implementation gives up waiting for a response only after user intervention or after a configurable amount of time.**

> "It is suggested that an
> implementation give up only after user intervention or a
> configurable amount of time." — §4.2, `rfc3032.txt:945-947`

- Strength: description. Class: internal.
- Check idea: leave a Configure-Ack or other response unanswered, and check that the implementation gives up only after user intervention or after a configurable amount of time, not sooner.

### RFC3032-PPP-10

**The MPLS Control Protocol defines no configuration option types.**

> "Configuration Option Types" — §4.2, `rfc3032.txt:959`; and
> "None." — `rfc3032.txt:961`

- Strength: description. Class: wire.
- Check idea: capture an MPLS Control Protocol Configure-Request on a PPP link, and check that it carries no configuration options.

### RFC3032-PPP-11

**PPP sends no labeled packet until it reaches the Network-Layer Protocol phase and the MPLS Control Protocol reaches the Opened state.**

> "Before any labeled packets may be communicated, PPP must reach the
> Network-Layer Protocol phase, and the MPLS Control Protocol must
> reach the Opened state." — §4.3, `rfc3032.txt:965-967`

- Strength: must (lower case). Class: wire.
- Check idea: attempt to send a labeled packet over a PPP link before PPP reaches the Network-Layer Protocol phase and before the MPLS Control Protocol reaches the Opened state, and check that no labeled packet appears on the link.

### RFC3032-PPP-12

**The PPP Information field carries exactly one labeled packet.**

> "Exactly one labeled packet is encapsulated in the PPP Information
> field" — §4.3, `rfc3032.txt:969-970`

- Strength: description. Class: encoding.
- Check idea: capture a PPP frame that carries a labeled packet, and check that the Information field holds exactly one labeled packet.

### RFC3032-PPP-13

**The PPP Protocol field holds hex 0281 to mark a labeled packet as an MPLS unicast packet.**

> "the PPP Protocol field indicates either type hex 0281
> (MPLS Unicast) or type hex 0283 (MPLS Multicast)." — §4.3, `rfc3032.txt:970-971`

- Strength: description. Class: wire.
- Check idea: send an MPLS unicast labeled packet over a PPP link, and check that the Protocol field holds hex 0281.

### RFC3032-PPP-14

**The PPP Protocol field holds hex 0283 to mark a labeled packet as an MPLS multicast packet.**

> "the PPP Protocol field indicates either type hex 0281
> (MPLS Unicast) or type hex 0283 (MPLS Multicast)." — §4.3, `rfc3032.txt:970-971`

- Strength: description. Class: wire.
- Check idea: send an MPLS multicast labeled packet over a PPP link, and check that the Protocol field holds hex 0283.

### RFC3032-PPP-15

**The maximum length of a labeled packet sent over a PPP link equals the maximum length of the Information field of a PPP encapsulated packet.**

> "The maximum length
> of a labeled packet transmitted over a PPP link is the same as the
> maximum length of the Information field of a PPP encapsulated packet." — §4.3, `rfc3032.txt:971-973`

- Strength: description. Class: wire.
- Check idea: measure the maximum length of the Information field of a PPP encapsulated packet, and check that the maximum length of a labeled packet sent over the same link equals it.

### RFC3032-PPP-16

**The PPP Information field carries the label stack entries in the format section 2 defines.**

> "The format of the Information field itself is as defined in section
> 2." — §4.3, `rfc3032.txt:975-976`

- Strength: description. Class: encoding.
- Check idea: capture the Information field of a PPP frame carrying a labeled packet, and check that its label stack entries follow the label stack entry format.
## Labeled packets over LAN media

### RFC3032-LAN-1

**Each frame carries exactly one labeled packet.**

> "Exactly one labeled packet is carried in each frame." — §5, `rfc3032.txt:989`.

- Strength: description. Class: wire.
- Check idea: capture a frame carrying labeled packets on a LAN link and check that it carries
  exactly one labeled packet.

### RFC3032-LAN-2

**The label stack entries immediately precede the network layer header and follow any data link layer headers, including an 802.1Q header.**

> "The label stack entries immediately precede the network layer header,
>    and follow any data link layer headers, including, e.g., any 802.1Q
>    headers that may exist." — §5, `rfc3032.txt:991-993`.

- Strength: may (lower case). Class: wire.
- Check idea: capture a labeled frame on a LAN link, including one with an 802.1Q header, and
  check that the label stack entries sit immediately before the network layer header and after
  every data link layer header.

### RFC3032-LAN-3

**The ethertype value 8847 hex marks a frame as carrying an MPLS unicast packet.**

> "The ethertype value 8847 hex is used to indicate that a frame is
>    carrying an MPLS unicast packet." — §5, `rfc3032.txt:995-996`.

- Strength: description. Class: wire.
- Check idea: capture a frame carrying an MPLS unicast packet on a LAN link and check that its
  ethertype field carries the value 8847 hex.

### RFC3032-LAN-4

**The ethertype value 8848 hex marks a frame as carrying an MPLS multicast packet.**

> "The ethertype value 8848 hex is used to indicate that a frame is
>    carrying an MPLS multicast packet." — §5, `rfc3032.txt:998-999`.

- Strength: description. Class: wire.
- Check idea: capture a frame carrying an MPLS multicast packet on a LAN link and check that
  its ethertype field carries the value 8848 hex.

### RFC3032-LAN-5

**Both the MPLS ethertype values can appear with either the ethernet encapsulation or the 802.3 LLC/SNAP encapsulation.**

> "These ethertype values can be used with either the ethernet
>    encapsulation or the 802.3 LLC/SNAP encapsulation to carry labeled
>    packets." — §5, `rfc3032.txt:1001-1003`.

- Strength: description. Class: wire.
- Check idea: capture labeled frames on a LAN link using the ethernet encapsulation and,
  separately, the 802.3 LLC/SNAP encapsulation, and check that both carry the MPLS ethertype
  values correctly.


## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§2 to §5, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 (introduction) and §6 to §11 (IANA,
  security, intellectual property, addresses, references).

Within the in-scope sections, these parts have no entry, with the reason:

In §2, §3 and §5:

- `rfc3032.txt:202-217`: the description of what a successful label lookup yields (next hop,
  stack operation, outgoing encapsulation) restates the generic NHLFE-style lookup outcome that
  belongs to RFC 3031's NHLFE definition, out of this draft's assigned areas.
- `rfc3032.txt:248-251`, `rfc3032.txt:265-267`: informative asides (the analogy with the IP
  Router Alert Option; the reason the Implicit NULL Label needs a reserved value in the Label
  Distribution Protocol) state no forwarding behavior of their own.
- `rfc3032.txt:299-302`: restates, as a negative consequence, the requirement RFC3032-NLP-2,
  RFC3032-NLP-4 and RFC3032-NLP-5 already state; no independent checkable fact.
- `rfc3032.txt:304-309`: informative rationale and example for why intermediate LSRs generally
  do not need to identify a packet's network layer protocol.
- `rfc3032.txt:343-346`: cross-reference back to §2.2 and a restatement that ICMP generation is
  not always possible; no new rule.
- `rfc3032.txt:348-376` (§2.3.1): a routing-engineering example (default-route injection into an
  IGP, ASBRs, BGP) for making ICMP messages routable through a transit domain; states nothing
  about the forwarding of a labeled packet, and is scoped to routing/label-distribution
  protocols outside this catalog.
- `rfc3032.txt:380-387`, `rfc3032.txt:392-409`, `rfc3032.txt:417-422`: scene-setting for the
  private-address tunneling case, the elaboration of how a label-switched ICMP message
  eventually reaches an unlabeled hop that can route to the source, a usefulness example (Time
  Exceeded / Destination Unreachable messages), and a closing note on routing-loop risk and
  implementation throttling of ICMP generation; all informative, no independent LSR requirement.
- `rfc3032.txt:443`: "The packet's lifetime in the network is considered to have expired" is
  explanatory of RFC3032-TTL-3, not a separate checkable fact.
- `rfc3032.txt:477-478`: a parenthetical assumption that any IPv4/IPv6 TTL decrement needed as
  part of ordinary IP processing has already happened before labeling; not an independent
  requirement.
- `rfc3032.txt:485-488`: an administrative note that a network may prefer to decrement the IPv4
  TTL by one per MPLS domain rather than by the number of LSP hops; states no requirement,
  either scheme is left to local policy.
- `rfc3032.txt:490-518` (§2.4.4): TTL handling when translating to or from an encapsulation other
  than the PPP/LAN encoding this document specifies (e.g., LC-ATM) defers entirely to the
  procedures of that other encapsulation's own specification; no rule of this document to
  catalog.
- `rfc3032.txt:522-529`: scene-setting restating, in general terms, the possibility that a
  labeled or unlabeled packet is too large for its output link; superseded by RFC3032-FRAG-1
  and the §3.3 rules.
- `rfc3032.txt:536-576`: informative background on typical (non-Path-MTU-Discovery) host
  behavior and datagram sizes; motivates the section but imposes no LSR requirement.
- `rfc3032.txt:578-644` (§3.1 Terminology): pure definitions of Frame Payload, Conventional/True/
  Effective Maximum Frame Payload Size, and Initially/Previously Labeled IP Datagram; the terms
  are used as defined in the entries above, no independent behavior follows from the
  definitions themselves.
- `rfc3032.txt:681-686`: a worked numeric example illustrating RFC3032-FRAG-4; no new rule.
- `rfc3032.txt:715-718`: a rationale note on when discarding is "sensible"; not a requirement.
- `rfc3032.txt:817-834` (part of §3.6): informative discussion of how Path MTU Discovery's
  outcome relates to the rules above, and a cross-reference to §2.3; restates consequences of
  entries already cataloged, states no new requirement.
- `rfc3032.txt:1004`: "The procedure for choosing which of these two encapsulations to use is
  beyond the scope of this document" explicitly disclaims scope; not a requirement.

In §4:

- `rfc3032.txt:865-887` — introduction to §4 and the three-component description of PPP itself (encapsulation, LCP, the family of NCPs). Background on PPP in general, not a statement about labels, a label stack, or an LSR.
- `rfc3032.txt:888-890` — "each end of the PPP link must first send LCP packets to configure and test the data link." Generic PPP/LCP link-establishment procedure, a different protocol's behavior, not specific to MPLS.
- `rfc3032.txt:893-894` — "Once the "MPLS Control Protocol" has reached the Opened state, labeled packets can be sent over the link." An early, less precise restatement; the precise gate condition is RFC3032-PPP-11 (§4.3, `rfc3032.txt:965-967`).
- `rfc3032.txt:902-906` — link teardown conditions (explicit LCP or MPLS Control Protocol packets, an inactivity timer, or administrator intervention). Generic PPP link-state behavior, not related to labels or a label stack entry.
- `rfc3032.txt:908-912` — the §4.2 heading and "The MPLS Control Protocol (MPLSCP) is responsible for enabling and disabling the use of label switching on a PPP link. It uses the same packet exchange mechanism as the Link Control Protocol (LCP)." A pure definition of what MPLSCP is; no behavior beyond what RFC3032-PPP-2 and neighboring entries already state.
- `rfc3032.txt:917-918` — "The MPLS Control Protocol is exactly the same as the Link Control Protocol [6] with the following exceptions:" A lead-in sentence to the numbered list; the list items are cataloged individually as RFC3032-PPP-4 through RFC3032-PPP-10.
- `rfc3032.txt:941-942` — "MPLSCP packets may not be exchanged until PPP has reached the Network-Layer Protocol phase." (repeated under item 4, Timeouts). Verbatim repeat of RFC3032-PPP-2 (§4.2, `rfc3032.txt:912-914`).
- `rfc3032.txt:978-981` — "Note that two codepoints are defined for labeled packets; one for multicast and one for unicast. Once the MPLSCP has reached the Opened state, both label switched multicasts and label switched unicasts can be sent over the PPP link." An informative "Note" restating the Protocol-field codepoints (RFC3032-PPP-13, RFC3032-PPP-14) and the send-gate condition (RFC3032-PPP-11); no new fact.
- `rfc3032.txt:983-985` — "4.4. Label Switching Control Protocol Configuration Options / There are no configuration options." Verbatim repeat of RFC3032-PPP-10 (§4.2 item 5, `rfc3032.txt:959-961`).
