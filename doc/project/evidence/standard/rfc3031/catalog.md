# RFC 3031 (Multiprotocol Label Switching Architecture) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC3031-*` · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 3031, Multiprotocol Label Switching Architecture, of
January 2001, a document of the in-scope set of MPLS. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc3031.txt`](../../../../../../standards/RFC/rfc3031.txt) —
  Multiprotocol Label Switching Architecture, January 2001. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc3031.txt>.

The other documents of the in-scope set have catalogs of their own:
[`rfc3032/catalog.md`](../rfc3032/catalog.md) (the label stack encoding),
[`rfc3443/catalog.md`](../rfc3443/catalog.md) (TTL processing) and
[`rfc5462/catalog.md`](../rfc5462/catalog.md) (the Traffic Class field).

The in-scope sections are §3.1, §3.3, §3.9 to §3.13, §3.15, §3.16, §3.18, §3.22, §3.23 and
§4.1.5. Most of RFC 3031 defines terms; an entry catalogs what an LSR does with them.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mpls/standards.md). The features these statements build are in
[`features.md`](../../protocol/mpls/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mpls/coverage.md`](../../model/mpls/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc3031.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC3031-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 3031 uses the keywords of RFC 2119 in capitals;
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
| [RFC3031-LBL-1](#rfc3031-lbl-1) | Rd must not bind one label to two different FECs from two upstream peers, unless Rd can always tell which peer sent an incoming packet. |
| [RFC3031-LBL-2](#rfc3031-lbl-2) | Each LSR makes sure it can uniquely read its incoming labels. |
| [RFC3031-LBL-3](#rfc3031-lbl-3) | A label never carries an encoded form of the destination address of the packet it marks. |
| [RFC3031-LBL-4](#rfc3031-lbl-4) | The entity that encodes a label and the entity that decodes it must agree on the encoding technique. |
| [RFC3031-STK-1](#rfc3031-stk-1) | A labeled packet's label stack works as a last-in, first-out stack. |
| [RFC3031-STK-2](#rfc3031-stk-2) | An LSR bases its handling of a labeled packet only on the top label, no matter the level of hierarchy. |
| [RFC3031-NHLFE-1](#rfc3031-nhlfe-1) | An NHLFE holds the packet's next hop. |
| [RFC3031-NHLFE-2](#rfc3031-nhlfe-2) | One NHLFE operation replaces the top label with a new label. |
| [RFC3031-NHLFE-3](#rfc3031-nhlfe-3) | One NHLFE operation pops the label stack. |
| [RFC3031-NHLFE-4](#rfc3031-nhlfe-4) | One NHLFE operation replaces the top label and then pushes one or more new labels. |
| [RFC3031-NHLFE-5](#rfc3031-nhlfe-5) | An NHLFE may hold the data link encapsulation to use when it sends the packet. |
| [RFC3031-NHLFE-6](#rfc3031-nhlfe-6) | An NHLFE may hold the way to encode the label stack when it sends the packet. |
| [RFC3031-NHLFE-7](#rfc3031-nhlfe-7) | When the NHLFE's next hop is the LSR itself, the LSR pops the top label and makes a new forwarding decision on what remains. |
| [RFC3031-ILM-1](#rfc3031-ilm-1) | The ILM maps each incoming label to a set of NHLFEs, and an LSR uses it to forward packets that arrive labeled. |
| [RFC3031-ILM-2](#rfc3031-ilm-2) | When the ILM maps a label to more than one NHLFE, the LSR picks exactly one before it forwards the packet. |
| [RFC3031-FTN-1](#rfc3031-ftn-1) | The FTN maps each FEC to a set of NHLFEs, and an LSR uses it to forward unlabeled packets that get a label before they leave. |
| [RFC3031-FTN-2](#rfc3031-ftn-2) | When the FTN maps a FEC to more than one NHLFE, the LSR picks exactly one before it forwards the packet. |
| [RFC3031-SWAP-1](#rfc3031-swap-1) | To forward a labeled packet, an LSR looks up the top label in the ILM, gets an NHLFE, and applies its label stack operation before it sends the packet on. |
| [RFC3031-SWAP-2](#rfc3031-swap-2) | To forward an unlabeled packet, an LSR gets the FEC from the network layer header, looks it up in the FTN, gets an NHLFE, and applies its label stack operation before it sends the packet on. |
| [RFC3031-SWAP-3](#rfc3031-swap-3) | Popping the label stack is not a valid operation for a packet that arrives unlabeled. |
| [RFC3031-SWAP-4](#rfc3031-swap-4) | When an LSR does label swapping, it always takes the next hop from the NHLFE, which the network layer routing algorithm might not have picked. |
| [RFC3031-LSP-1](#rfc3031-lsp-1) | The LSP Ingress is the LSR that pushes a label onto a packet, so that the packet gets a label stack of depth m. |
| [RFC3031-LSP-2](#rfc3031-lsp-2) | Every intermediate LSR of a level m LSP gets the packet with a label stack of depth m. |
| [RFC3031-LSP-3](#rfc3031-lsp-3) | Between the LSP Ingress and the last LSR before the egress, the packet's label stack never has a depth below m. |
| [RFC3031-LSP-4](#rfc3031-lsp-4) | Every intermediate LSR of a level m LSP forwards the packet by MPLS, using the top label as an index into an ILM. |
| [RFC3031-LSP-5](#rfc3031-lsp-5) | A non-MPLS system between two LSRs of a level m LSP does not base its forwarding decision on the level m label or the network layer header. |
| [RFC3031-LSP-6](#rfc3031-lsp-6) | The LSP Egress is the LSR where the forwarding decision uses a label below level m, or non-MPLS forwarding, ending the level m LSP. |
| [RFC3031-LSP-7](#rfc3031-lsp-7) | When an LSR pushes a label onto an already labeled packet, it makes sure the new label's FEC has an LSP Egress equal to the LSR that assigned the label now second in the stack. |
| [RFC3031-PHP-1](#rfc3031-php-1) | The penultimate LSR of an LSP may pop the label stack instead of the LSP Egress. |
| [RFC3031-PHP-2](#rfc3031-php-2) | When penultimate hop popping is used, the penultimate LSR sees that it is the penultimate hop, pops the stack, and forwards the packet using the label that was on top. |
| [RFC3031-PHP-3](#rfc3031-php-3) | When the LSP Egress gets the packet after penultimate hop popping, it looks up the label now on top, or, if only one label was carried, it sees the plain network layer packet. |
| [RFC3031-PHP-4](#rfc3031-php-4) | When penultimate hop popping is used, the LSP Egress need not be an LSR. |
| [RFC3031-PHP-5](#rfc3031-php-5) | The penultimate LSR pops the label stack only when the egress node asks for it, or when the next node does not support MPLS. |
| [RFC3031-PHP-6](#rfc3031-php-6) | An LSR that can pop the label stack must do penultimate hop popping when its downstream label distribution peer asks for it. |
| [RFC3031-PHP-7](#rfc3031-php-7) | Initial label distribution negotiations must let each LSR learn whether its neighboring LSRs can pop the label stack. |
| [RFC3031-PHP-8](#rfc3031-php-8) | An LSR must not ask a label distribution peer to pop the label stack unless the LSR itself can pop the stack. |
| [RFC3031-INV-1](#rfc3031-inv-1) | An LSR discards a labeled packet that arrives with an invalid incoming label, unless it can tell that forwarding it unlabeled is harmless. |
| [RFC3031-NOL-1](#rfc3031-nol-1) | When the ILM has no NHLFE for a valid incoming label, an LSR discards the packet unless it can rule out harm. |
| [RFC3031-TTL-1](#rfc3031-ttl-1) | A packet on an LSP should emerge with the TTL value it would have had without label switching. |
| [RFC3031-TTL-2](#rfc3031-ttl-2) | A packet on a hierarchy of LSPs should have its TTL reflect the total number of LSR-hops it crossed. |
| [RFC3031-TTL-3](#rfc3031-ttl-3) | A shim label encoding must have a TTL field. |
| [RFC3031-TTL-4](#rfc3031-ttl-4) | The shim TTL field should start out at the value of the network layer TTL field. |
| [RFC3031-TTL-5](#rfc3031-ttl-5) | The shim TTL field should decrement at each LSR-hop. |
| [RFC3031-TTL-6](#rfc3031-ttl-6) | The shim TTL field should copy back into the network layer TTL field when the packet leaves its LSP. |
| [RFC3031-TTL-7](#rfc3031-ttl-7) | A packet that leaves a non-TTL LSP segment should get a TTL that reflects the LSR-hops it crossed. |
| [RFC3031-TTL-8](#rfc3031-ttl-8) | An LSR at the ingress to a non-TTL LSP segment must not label switch a packet whose TTL would expire in that segment. |
| [RFC3031-NULL-1](#rfc3031-null-1) | When the next hop LSR has bound Implicit NULL to the address prefix, the upstream LSR pops the label stack instead of a swap, then forwards the packet. |
| [RFC3031-NULL-2](#rfc3031-null-2) | Rd binds Implicit NULL for prefix X to Ru only when Rd would give Ru an ordinary label binding for X, Ru can support Implicit NULL by popping the stack, and Rd is the LSP Egress, not a proxy egress, for X. |
| [RFC3031-NULL-3](#rfc3031-null-3) | When the penultimate LSR of an LSP for prefix X is the LSP Proxy Egress, it acts as if the LSP Egress had distributed an Implicit NULL binding for X. |

## Labels and labeled packets

### RFC3031-LBL-1

**Rd must not bind one label to two different FECs from two upstream peers, unless Rd can always tell which peer sent an incoming packet.**

> "Rd MUST NOT agree with Ru1 to bind L to FEC F1, while also agreeing with some other LSR Ru2 to bind L to a different FEC F2, UNLESS Rd can always tell, when it receives a packet with incoming label L, whether the label was put on the packet by Ru1 or whether it was put on by Ru2." — §3.1, `rfc3031.txt:537-541`.

- Strength: must not. Class: internal.
- Check idea: bind label L to FEC F1 from Ru1 and to FEC F2 from Ru2 at Rd, and check that Rd
  rejects this unless it can always tell which peer put label L on an arriving packet.

### RFC3031-LBL-2

**Each LSR makes sure it can uniquely read its incoming labels.**

> "It is the responsibility of each LSR to ensure that it can uniquely interpret its incoming labels." — §3.1, `rfc3031.txt:543-544`.

- Strength: description. Class: internal.
- Check idea: assign incoming labels to an LSR and check that it can always resolve each one to
  a single meaning.

### RFC3031-LBL-3

**A label never carries an encoded form of the destination address of the packet it marks.**

> "the label is never an encoding of that address." — §3.1, `rfc3031.txt:512-513`.

- Strength: description. Class: internal.
- Check idea: assign a FEC to a packet by its network layer destination address, and check that
  the label value used is not a function of that address.

### RFC3031-LBL-4

**The entity that encodes a label and the entity that decodes it must agree on the encoding technique.**

> "The particular encoding technique to be used must be agreed to by both the entity which encodes the label and the entity which decodes the label." — §3.3, `rfc3031.txt:573-575`.

- Strength: must (lower case). Class: internal.
- Check idea: set the encoding technique for a label at the entity that encodes it and at the
  entity that decodes it, and check that both sides use the same technique.

## The label stack

### RFC3031-STK-1

**A labeled packet's label stack works as a last-in, first-out stack.**

> "a labeled packet carries a number of labels, organized as a last-in, first-out stack." — §3.9, `rfc3031.txt:692-693`.

- Strength: description. Class: internal.
- Check idea: push two labels onto a packet in order, then pop the stack, and check that the
  labels come off in reverse order.

### RFC3031-STK-2

**An LSR bases its handling of a labeled packet only on the top label, no matter the level of hierarchy.**

> "Although, as we shall see, MPLS supports a hierarchy, the processing of a labeled packet is completely independent of the level of hierarchy.  The processing is always based on the top label, without regard for the possibility that some number of other labels may have been "above it" in the past, or that some number of other labels may be below it at present." — §3.9, `rfc3031.txt:695-700`.

- Strength: may (lower case). Class: internal.
- Check idea: send a labeled packet with labels below the top one through an LSR, and check that
  its forwarding decision uses only the top label.

## The Next Hop Label Forwarding Entry

### RFC3031-NHLFE-1

**An NHLFE holds the packet's next hop.**

> "The "Next Hop Label Forwarding Entry" (NHLFE) is used when forwarding a labeled packet.  It contains the following information:" and "the packet's next hop" — §3.10, `rfc3031.txt:715-716`, `rfc3031.txt:718`.

- Strength: description. Class: internal.
- Check idea: look up a label in the ILM to get an NHLFE, and check that the NHLFE gives a next
  hop for the packet.

### RFC3031-NHLFE-2

**One NHLFE operation replaces the top label with a new label.**

> "the operation to perform on the packet's label stack; this is one of the following operations:" and "replace the label at the top of the label stack with a specified new label" — §3.10, `rfc3031.txt:720-721`, `rfc3031.txt:723-724`.

- Strength: description. Class: internal.
- Check idea: set an NHLFE's operation to replace the top label, forward a packet through it,
  and check that the top label changes to the new value.

### RFC3031-NHLFE-3

**One NHLFE operation pops the label stack.**

> "pop the label stack" — §3.10, `rfc3031.txt:726`.

- Strength: description. Class: internal.
- Check idea: set an NHLFE's operation to pop the stack, forward a packet through it, and check
  that the top label is removed.

### RFC3031-NHLFE-4

**One NHLFE operation replaces the top label and then pushes one or more new labels.**

> "replace the label at the top of the label stack with a specified new label, and then push one or more specified new labels onto the label stack." — §3.10, `rfc3031.txt:735-737`.

- Strength: description. Class: internal.
- Check idea: set an NHLFE's operation to replace the top label and push further labels, forward
  a packet through it, and check the depth and values of the resulting stack.

### RFC3031-NHLFE-5

**An NHLFE may hold the data link encapsulation to use when it sends the packet.**

> "the data link encapsulation to use when transmitting the packet" — §3.10, `rfc3031.txt:741`.

- Strength: may. Class: internal.
- Check idea: set a data link encapsulation in an NHLFE, forward a packet through it, and check
  that the packet leaves with that encapsulation.

### RFC3031-NHLFE-6

**An NHLFE may hold the way to encode the label stack when it sends the packet.**

> "the way to encode the label stack when transmitting the packet" — §3.10, `rfc3031.txt:743`.

- Strength: may. Class: internal.
- Check idea: set a label stack encoding method in an NHLFE, forward a packet through it, and
  check that the packet leaves with that encoding.

### RFC3031-NHLFE-7

**When the NHLFE's next hop is the LSR itself, the LSR pops the top label and makes a new forwarding decision on what remains.**

> "Note that at a given LSR, the packet's "next hop" might be that LSR itself.  In this case, the LSR would need to pop the top level label, and then "forward" the resulting packet to itself.  It would then make another forwarding decision, based on what remains after the label stacked is popped." and "If the packet's "next hop" is the current LSR, then the label stack operation MUST be to "pop the stack"." — §3.10, `rfc3031.txt:748-753`, `rfc3031.txt:758-759`.

- Strength: must. Class: internal.
- Check idea: set an NHLFE's next hop to the LSR itself, forward a labeled packet through it,
  and check that the LSR pops the top label and then forwards the remainder by a new decision.

## The Incoming Label Map

### RFC3031-ILM-1

**The ILM maps each incoming label to a set of NHLFEs, and an LSR uses it to forward packets that arrive labeled.**

> "The "Incoming Label Map" (ILM) maps each incoming label to a set of NHLFEs.  It is used when forwarding packets that arrive as labeled packets." — §3.11, `rfc3031.txt:763-765`.

- Strength: description. Class: internal.
- Check idea: give an LSR a labeled packet and check that it looks up the incoming label in the
  ILM to get a set of NHLFEs.

### RFC3031-ILM-2

**When the ILM maps a label to more than one NHLFE, the LSR picks exactly one before it forwards the packet.**

> "If the ILM maps a particular label to a set of NHLFEs that contains more than one element, exactly one element of the set must be chosen before the packet is forwarded." — §3.11, `rfc3031.txt:767-769`.

- Strength: must (lower case). Class: internal.
- Check idea: map an incoming label to two NHLFEs in the ILM, forward a packet with that label,
  and check that the LSR picks exactly one NHLFE.

## The FEC-to-NHLFE map

### RFC3031-FTN-1

**The FTN maps each FEC to a set of NHLFEs, and an LSR uses it to forward unlabeled packets that get a label before they leave.**

> "The "FEC-to-NHLFE" (FTN) maps each FEC to a set of NHLFEs.  It is used when forwarding packets that arrive unlabeled, but which are to be labeled before being forwarded." — §3.12, `rfc3031.txt:777-779`.

- Strength: description. Class: internal.
- Check idea: give an LSR an unlabeled packet of a FEC held in its FTN, and check that it looks
  up the FEC in the FTN to get a set of NHLFEs.

### RFC3031-FTN-2

**When the FTN maps a FEC to more than one NHLFE, the LSR picks exactly one before it forwards the packet.**

> "If the FTN maps a particular label to a set of NHLFEs that contains more than one element, exactly one element of the set must be chosen before the packet is forwarded." — §3.12, `rfc3031.txt:791-793`.

- Strength: must (lower case). Class: internal.
- Check idea: map a FEC to two NHLFEs in the FTN, forward an unlabeled packet of that FEC, and
  check that the LSR picks exactly one NHLFE.

## Label swapping

### RFC3031-SWAP-1

**To forward a labeled packet, an LSR looks up the top label in the ILM, gets an NHLFE, and applies its label stack operation before it sends the packet on.**

> "In order to forward a labeled packet, a LSR examines the label at the top of the label stack.  It uses the ILM to map this label to an NHLFE.  Using the information in the NHLFE, it determines where to forward the packet, and performs an operation on the packet's label stack.  It then encodes the new label stack into the packet, and forwards the result." — §3.13, `rfc3031.txt:804-809`.

- Strength: description. Class: internal.
- Check idea: send a labeled packet through an LSR and check that it looks up the top label in
  the ILM, then applies the NHLFE's label stack operation before it forwards the packet.

### RFC3031-SWAP-2

**To forward an unlabeled packet, an LSR gets the FEC from the network layer header, looks it up in the FTN, gets an NHLFE, and applies its label stack operation before it sends the packet on.**

> "In order to forward an unlabeled packet, a LSR analyzes the network layer header, to determine the packet's FEC.  It then uses the FTN to map this to an NHLFE.  Using the information in the NHLFE, it determines where to forward the packet, and performs an operation on the packet's label stack." and "It then encodes the new label stack into the packet, and forwards the result." — §3.13, `rfc3031.txt:811-815`, `rfc3031.txt:816-817`.

- Strength: description. Class: internal.
- Check idea: send an unlabeled packet through an LSR and check that it derives the FEC from the
  network layer header, looks it up in the FTN, and applies the NHLFE's label stack operation.

### RFC3031-SWAP-3

**Popping the label stack is not a valid operation for a packet that arrives unlabeled.**

> "(Popping the label stack would, of course, be illegal in this case.)" — §3.13, `rfc3031.txt:815-816`.

- Strength: description. Class: internal.
- Check idea: check that no NHLFE reached through the FTN specifies a pop operation, since an
  unlabeled packet has no label stack to pop yet.

### RFC3031-SWAP-4

**When an LSR does label swapping, it always takes the next hop from the NHLFE, which the network layer routing algorithm might not have picked.**

> "IT IS IMPORTANT TO NOTE THAT WHEN LABEL SWAPPING IS IN USE, THE NEXT HOP IS ALWAYS TAKEN FROM THE NHLFE; THIS MAY IN SOME CASES BE DIFFERENT FROM WHAT THE NEXT HOP WOULD BE IF MPLS WERE NOT IN USE." — §3.13, `rfc3031.txt:819-821`.

- Strength: description. Class: internal.
- Check idea: set an NHLFE's next hop to differ from the network layer route, forward a labeled
  packet, and check that the LSR sends it to the NHLFE's next hop.

## LSP, LSP ingress, LSP egress

### RFC3031-LSP-1

**The LSP Ingress is the LSR that pushes a label onto a packet, so that the packet gets a label stack of depth m.**

> "R1, the "LSP Ingress", is an LSR which pushes a label onto P's label stack, resulting in a label stack of depth m" — §3.15, `rfc3031.txt:903-904`.

- Strength: description. Class: internal.
- Check idea: send an unlabeled packet into an LSP and check that its LSP Ingress pushes a label
  onto it, giving it a label stack of depth m.

### RFC3031-LSP-2

**Every intermediate LSR of a level m LSP gets the packet with a label stack of depth m.**

> "For all i, 1<i<n, P has a label stack of depth m when received by LSR Ri" — §3.15, `rfc3031.txt:906-907`.

- Strength: description. Class: internal.
- Check idea: send a packet along a level m LSP and check that each intermediate LSR gets it
  with a label stack of depth m.

### RFC3031-LSP-3

**Between the LSP Ingress and the last LSR before the egress, the packet's label stack never has a depth below m.**

> "At no time during P's transit from R1 to R[n-1] does its label stack ever have a depth of less than m" — §3.15, `rfc3031.txt:909-910`.

- Strength: description. Class: internal.
- Check idea: send a packet along a level m LSP and check that its label stack depth never drops
  below m before the last LSR.

### RFC3031-LSP-4

**Every intermediate LSR of a level m LSP forwards the packet by MPLS, using the top label as an index into an ILM.**

> "Ri transmits P to R[i+1] by means of MPLS, i.e., by using the label at the top of the label stack (the level m label) as an index into an ILM" — §3.15, `rfc3031.txt:912-914`.

- Strength: description. Class: internal.
- Check idea: at an intermediate LSR of a level m LSP, check that it forwards the packet by
  looking up the level m label in an ILM.

### RFC3031-LSP-5

**A non-MPLS system between two LSRs of a level m LSP does not base its forwarding decision on the level m label or the network layer header.**

> "if a system S receives and forwards P after P is transmitted by Ri but before P is received by R[i+1] (e.g., Ri and R[i+1] might be connected via a switched data link subnetwork, and S might be one of the data link switches), then S's forwarding decision is not based on the level m label, or on the network layer header.  This may be because: a) the decision is not based on the label stack or the network layer header at all; b) the decision is based on a label stack on which additional labels have been pushed (i.e., on a level m+k label, where k>0)." — §3.15, `rfc3031.txt:916-928`.

- Strength: may (lower case). Class: internal.
- Check idea: insert a non-MPLS switch between two LSRs of a level m LSP and check that its
  forwarding decision does not use the level m label or the network layer header.

### RFC3031-LSP-6

**The LSP Egress is the LSR where the forwarding decision uses a label below level m, or non-MPLS forwarding, ending the level m LSP.**

> "which ends (at an "LSP Egress") when a forwarding decision is made by label Switching on a level m-k label, where k>0, or when a forwarding decision is made by "ordinary", non-MPLS forwarding procedures." — §3.15, `rfc3031.txt:939-942`.

- Strength: description. Class: internal.
- Check idea: at the LSR where a level m LSP ends, check that its forwarding decision uses a
  label of level less than m, or plain non-MPLS forwarding.

### RFC3031-LSP-7

**When an LSR pushes a label onto an already labeled packet, it makes sure the new label's FEC has an LSP Egress equal to the LSR that assigned the label now second in the stack.**

> "whenever an LSR pushes a label onto an already labeled packet, it needs to make sure that the new label corresponds to a FEC whose LSP Egress is the LSR that assigned the label which is now second in the stack." — §3.15, `rfc3031.txt:944-947`.

- Strength: description. Class: internal.
- Check idea: at an LSR that pushes a label onto an already labeled packet, check that the new
  label's FEC has an LSP Egress equal to the LSR that assigned the label now second in the stack.

## Penultimate hop popping

### RFC3031-PHP-1

**The penultimate LSR of an LSP may pop the label stack instead of the LSP Egress.**

> "P may be transmitted from R[n-1] to Rn with a label stack of depth m-1.  That is, the label stack may be popped at the penultimate LSR of the LSP, rather than at the LSP Egress." — §3.16, `rfc3031.txt:974-977`.

- Strength: may (lower case). Class: internal.
- Check idea: configure the penultimate LSR of an LSP to pop the label stack, and check that the
  packet reaches the LSP Egress with a label stack of depth m-1 already.

### RFC3031-PHP-2

**When penultimate hop popping is used, the penultimate LSR sees that it is the penultimate hop, pops the stack, and forwards the packet using the label that was on top.**

> "If, on the other hand, penultimate hop popping is used, then when the penultimate hop looks up the label, it determines: -  that it is the penultimate hop, and -  who the next hop is." and "The penultimate node then pops the stack, and forwards the packet based on the information gained by looking up the label that was previously at the top of the stack." — §3.16, `rfc3031.txt:997-1006`.

- Strength: description. Class: internal.
- Check idea: enable penultimate hop popping at the penultimate LSR of an LSP, send it a labeled
  packet, and check that it pops the top label and forwards using the information that label held.

### RFC3031-PHP-3

**When the LSP Egress gets the packet after penultimate hop popping, it looks up the label now on top, or, if only one label was carried, it sees the plain network layer packet.**

> "When the LSP egress receives the" and "packet, the label which is now at the top of the stack will be the label which it needs to look up in order to make its own forwarding decision.  Or, if the packet was only carrying a single label, the LSP egress will simply see the network layer packet, which is just what it needs to see in order to make its forwarding decision." — §3.16, `rfc3031.txt:1006`, `rfc3031.txt:1015-1019`.

- Strength: description. Class: internal.
- Check idea: after penultimate hop popping, deliver a packet to the LSP Egress and check that
  it forwards on the next remaining label, or on the plain network layer packet if none remains.

### RFC3031-PHP-4

**When penultimate hop popping is used, the LSP Egress need not be an LSR.**

> "In fact, when penultimate hop popping is done, the LSP Egress need not even be an LSR." — §3.16, `rfc3031.txt:1034-1035`.

- Strength: description. Class: internal.
- Check idea: set up penultimate hop popping so the LSP Egress is a plain host, not an LSR, and
  check that it still gets a correctly forwarded network layer packet.

### RFC3031-PHP-5

**The penultimate LSR pops the label stack only when the egress node asks for it, or when the next node does not support MPLS.**

> "Therefore the penultimate node pops the label stack only if this is specifically requested by the egress node, OR if the next node in the LSP does not support MPLS." — §3.16, `rfc3031.txt:1040-1042`.

- Strength: description. Class: internal.
- Check idea: check that a penultimate LSR pops the stack only after a request from the egress
  node, or when the LSP's next node does not support MPLS, and not otherwise.

### RFC3031-PHP-6

**An LSR that can pop the label stack must do penultimate hop popping when its downstream label distribution peer asks for it.**

> "An LSR which is capable of popping the label stack at all MUST do penultimate hop popping when so requested by its downstream label distribution peer." — §3.16, `rfc3031.txt:1046-1048`.

- Strength: must. Class: internal.
- Check idea: ask an LSR that is capable of popping the label stack to do penultimate hop
  popping, and check that it complies.

### RFC3031-PHP-7

**Initial label distribution negotiations must let each LSR learn whether its neighboring LSRs can pop the label stack.**

> "Initial label distribution protocol negotiations MUST allow each LSR to determine whether its neighboring LSRS are capable of popping the label stack." — §3.16, `rfc3031.txt:1050-1052`.

- Strength: must. Class: internal.
- Check idea: run the initial label distribution negotiation between two LSRs and check that
  each one learns whether its neighbor can pop the label stack.

### RFC3031-PHP-8

**An LSR must not ask a label distribution peer to pop the label stack unless the LSR itself can pop the stack.**

> "A LSR MUST NOT request a label distribution peer to pop the label stack unless it is capable of doing so." — §3.16, `rfc3031.txt:1052-1053`.

- Strength: must not. Class: internal.
- Check idea: check that an LSR requests penultimate hop popping from a peer only when the
  requesting LSR itself can pop the label stack.

## Invalid incoming labels

### RFC3031-INV-1

**An LSR discards a labeled packet that arrives with an invalid incoming label, unless it can tell that forwarding it unlabeled is harmless.**

> "Therefore, when a labeled packet is received with an invalid incoming label, it MUST be discarded, UNLESS it is determined by some means (not within the scope of the current document) that forwarding it unlabeled cannot cause any harm." — §3.18, `rfc3031.txt:1099-1102`.

- Strength: must. Class: end-to-end.
- Check idea: send an LSR a labeled packet whose incoming label has no ILM binding, and check
  that the LSR discards the packet.

## Lack of outgoing label

### RFC3031-NOL-1

**When the ILM has no NHLFE for a valid incoming label, an LSR discards the packet unless it can rule out harm.**

> "Unless it can be determined (through some means outside the scope of this document) that neither of these situations obtains, the only safe procedure is to discard the packet." — §3.22, `rfc3031.txt:1314-1316`.

- Strength: description. Class: end-to-end.
- Check idea: send an LSR a valid labeled packet whose incoming label has no NHLFE in the ILM,
  and check that the LSR discards the packet.

## Time-to-Live

### RFC3031-TTL-1

**A packet on an LSP should emerge with the TTL value it would have had without label switching.**

> "When a packet travels along an LSP, it SHOULD emerge with the same TTL value that it would have had if it had traversed the same sequence of routers without having been label switched." — §3.23, `rfc3031.txt:1334-1336`.

- Strength: should. Class: wire.
- Check idea: send a packet across an LSP and, separately, across the same routers without label
  switching, and check that both arrive with the same TTL.

### RFC3031-TTL-2

**A packet on a hierarchy of LSPs should have its TTL reflect the total number of LSR-hops it crossed.**

> "If the packet travels along a hierarchy of LSPs, the total number of LSR- hops traversed SHOULD be reflected in its TTL value when it emerges from the hierarchy of LSPs." — §3.23, `rfc3031.txt:1336-1339`.

- Strength: should. Class: wire.
- Check idea: send a packet across a hierarchy of LSPs and check that its TTL, on emerging,
  reflects the total LSR-hops crossed.

### RFC3031-TTL-3

**A shim label encoding must have a TTL field.**

> "If the label values are encoded in a "shim" that sits between the data link and network layer headers, then this shim MUST have a TTL field" — §3.23, `rfc3031.txt:1356-1358`.

- Strength: must. Class: encoding.
- Check idea: capture a shim label stack entry on the link and check that it carries a TTL field.

### RFC3031-TTL-4

**The shim TTL field should start out at the value of the network layer TTL field.**

> "that SHOULD be initially loaded from the network layer header" and "TTL field" — §3.23, `rfc3031.txt:1358`, `rfc3031.txt:1359`.

- Strength: should. Class: wire.
- Check idea: push a label onto a packet and check that the shim TTL field starts at the network
  layer TTL value.

### RFC3031-TTL-5

**The shim TTL field should decrement at each LSR-hop.**

> "SHOULD be decremented at each LSR-hop" — §3.23, `rfc3031.txt:1359`.

- Strength: should. Class: wire.
- Check idea: send a packet across several LSR-hops of a shim-encoded LSP and check that the
  shim TTL field decreases at each hop.

### RFC3031-TTL-6

**The shim TTL field should copy back into the network layer TTL field when the packet leaves its LSP.**

> "and SHOULD be copied into the network layer header TTL field when the packet emerges from its LSP." — §3.23, `rfc3031.txt:1359-1361`.

- Strength: should. Class: wire.
- Check idea: pop the last label of a shim-encoded LSP and check that the network layer TTL
  field takes the shim TTL value.

### RFC3031-TTL-7

**A packet that leaves a non-TTL LSP segment should get a TTL that reflects the LSR-hops it crossed.**

> "When a packet emerges from a non-TTL LSP segment, it SHOULD however be given a TTL that reflects the number of LSR-hops it traversed." — §3.23, `rfc3031.txt:1371-1372`.

- Strength: should. Class: wire.
- Check idea: send a packet across a non-TTL LSP segment and check that its TTL, on leaving the
  segment, reflects the LSR-hops crossed.

### RFC3031-TTL-8

**An LSR at the ingress to a non-TTL LSP segment must not label switch a packet whose TTL would expire in that segment.**

> "In this case, the LSR at the ingress to the non-TTL LSP segment must not label switch the packet." — §3.23, `rfc3031.txt:1379-1381`.

- Strength: must not (lower case). Class: end-to-end.
- Check idea: give a packet a TTL too low to cross a non-TTL LSP segment, and check that the
  ingress LSR does not label switch it.

## The Implicit NULL label

### RFC3031-NULL-1

**When the next hop LSR has bound Implicit NULL to the address prefix, the upstream LSR pops the label stack instead of a swap, then forwards the packet.**

> "If LSR Ru, by consulting its ILM, sees that labeled packet P must be forwarded next to Rd, but that Rd has distributed a binding of Implicit NULL to the corresponding address prefix, then instead of replacing the value of the label on top of the label stack, Ru pops the label stack, and then forwards the resulting packet to Rd." — §4.1.5, `rfc3031.txt:2198-2203`.

- Strength: must (lower case). Class: internal.
- Check idea: bind Implicit NULL at Rd for an address prefix, send Ru a labeled packet for that
  prefix, and check that Ru pops the label stack instead of replacing the label.

### RFC3031-NULL-2

**Rd binds Implicit NULL for prefix X to Ru only when Rd would give Ru an ordinary label binding for X, Ru can support Implicit NULL by popping the stack, and Rd is the LSP Egress, not a proxy egress, for X.**

> "LSR Rd distributes a binding between Implicit NULL and an address prefix X to LSR Ru if and only if:" and "the rules of Section 4.1.2 indicate that Rd distributes to Ru a label binding for X, and" and "Rd knows that Ru can support the Implicit NULL label (i.e., that it can pop the label stack), and" and "Rd is an LSP Egress (not proxy egress) for X." — §4.1.5, `rfc3031.txt:2205-2214`.

- Strength: description. Class: internal.
- Check idea: configure Rd as the LSP Egress for prefix X with Ru capable of popping the stack,
  and check that Rd advertises Implicit NULL only in that case, not when Rd is a proxy egress or
  Ru cannot pop the stack.

### RFC3031-NULL-3

**When the penultimate LSR of an LSP for prefix X is the LSP Proxy Egress, it acts as if the LSP Egress had distributed an Implicit NULL binding for X.**

> "If the penultimate LSR in an LSP for address prefix X is an LSP Proxy Egress, it acts just as if the LSP Egress had distributed a binding of Implicit NULL for X." — §4.1.5, `rfc3031.txt:2229-2231`.

- Strength: description. Class: internal.
- Check idea: set up an LSP whose egress does not support MPLS, so its penultimate LSR is the
  LSP Proxy Egress, and check that this LSR pops the label stack as if Implicit NULL had been
  advertised.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§3.1, §3.3, §3.9 to §3.13, §3.15, §3.16, §3.18, §3.22, §3.23 and §4.1.5, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set.** §1 and §2 (introduction, terminology), §3.2,
  §3.4 to §3.8, §3.14, §3.17, §3.19 to §3.21 and §3.24 to §3.30 (label distribution, aggregation,
  route selection, loop control, encodings, merging, tunnels, multicast), and §4 and §5 apart from
  §4.1.5. See [`standards.md`](../../protocol/mpls/standards.md#target-level) for the level at
  which each one enters.

Within the in-scope sections, these parts have no entry, with the reason:

In §3.1 to §4.1.5:

- `rfc3031.txt:489-493`: section 3 heading and introduction, no checkable content.
- `rfc3031.txt:496-499`: pure definition of the term "label"; no LSR behavior follows from it.
- `rfc3031.txt:511-512`: background statement that a FEC is commonly assigned by destination
  address; descriptive context, not a rule.
- `rfc3031.txt:515-521`: illustrative example of two LSRs agreeing on a label/FEC binding; an
  example, not a requirement.
- `rfc3031.txt:523-530`: terminology clarification about the scope of a binding and of "packets
  being sent"; clarifies wording, states no requirement.
- `rfc3031.txt:567-569`: pure definition of the term "labeled packet".
- `rfc3031.txt:570-573`: description of where a label may be encoded (dedicated header vs. an
  existing field); background on implementation options, not a specific rule.
- `rfc3031.txt:688-689`: section heading.
- `rfc3031.txt:702-703`: notational convention that an unlabeled packet is a label stack of depth
  0; no independent behavior follows.
- `rfc3031.txt:705-708`: naming convention for level 1..m labels; pure definition.
- `rfc3031.txt:710-711`: forward reference to section 3.27; no content.
- `rfc3031.txt:713-714`: section heading.
- `rfc3031.txt:745-746`: NHLFE item (f), "any other information needed to properly dispose of
  the packet"; too unspecific to test independently.
- `rfc3031.txt:755-756`: consequence/implication of NHLFE-7, already covered by that entry.
- `rfc3031.txt:761-762`, `rfc3031.txt:775-776`: section headings.
- `rfc3031.txt:769-773`, `rfc3031.txt:793-797`: "the procedures for choosing an element... are
  beyond the scope of this document", and the load-balancing rationale that follows; explicitly
  out of scope, and informative rationale.
- `rfc3031.txt:799-802`: section heading and definition of the term "label swapping".
- `rfc3031.txt:887-894`: definition of "Label Switched Path (LSP) of level m" as a sequence of
  routers; pure lead-in definition (the properties that follow are catalogued individually).
- `rfc3031.txt:930-938`: "in other words" restatement of the LSP Ingress and intermediate-LSR
  rules already catalogued as LSP-1 and LSP-4; no new content.
- `rfc3031.txt:959-969`: definition of "the LSP for a particular FEC F" and the "LSP tree";
  terminology/illustration, no LSR behavior demanded.
- `rfc3031.txt:971`: section heading.
- `rfc3031.txt:979-996`: rationale for penultimate hop popping (architectural appropriateness,
  the two-lookup problem without it); explanatory, not a rule.
- `rfc3031.txt:1020-1032`: summary and fastpath/implementation-simplification rationale; informative
  for implementers, not a protocol rule.
- `rfc3031.txt:1037-1039`, `rfc3031.txt:1043-1044`: hedging rationale on why PHP cannot be
  mandated universally, and a clarifying parenthetical; explanation around PHP-5, not a
  separate rule.
- `rfc3031.txt:1055-1059`: consequence tied to the uniqueness/scoping rules of section 3.14,
  which is outside this range; restatement, no new rule.
- `rfc3031.txt:1084`: section heading.
- `rfc3031.txt:1086-1097`: rhetorical question and rationale (the loop risk of stripping an
  invalid label, and the "route not inferable from the header" possibility); explanatory
  lead-in to the MUST in INV-1.
- `rfc3031.txt:1295`: section heading.
- `rfc3031.txt:1297-1301`: scenario description of when the ILM lacks an NHLFE for a valid
  label; background context for NOL-1.
- `rfc3031.txt:1303-1312`: rhetorical setup and two rationale bullets on why stripping the label
  stack is unsafe; explanatory, not itself a rule.
- `rfc3031.txt:1318`: section heading.
- `rfc3031.txt:1320-1323`: background on conventional (non-MPLS) IP TTL behavior, for contrast;
  not an MPLS/LSR rule.
- `rfc3031.txt:1325-1332`: rationale framing the two TTL-related issues MPLS must deal with; sets
  up the rules that follow, states no rule itself.
- `rfc3031.txt:1351-1354`: framing sentence that TTL handling depends on the shim vs. L2-header
  case; transition, not itself checkable.
- `rfc3031.txt:1363-1369`: definition of "non-TTL LSP segment" and the physical consequence that
  such a segment cannot decrement TTL; definitional/consequential background for TTL-7 and
  TTL-8.
- `rfc3031.txt:1372-1375`: example mechanism ("in the unicast case, this can be achieved by...")
  for meeting the TTL-7 SHOULD; illustrative, not a separate rule.
- `rfc3031.txt:1381-1383`: "special procedures must be developed" for traceroute, with a "may be
  forwarded" example; too unspecific to test, and an example.
- `rfc3031.txt:2195`: section heading.
- `rfc3031.txt:2197-2198`: pure definition of the term "Implicit NULL label".
- `rfc3031.txt:2216-2223`: rationale restating the penultimate-hop-popping benefit (saves the
  LSP Egress a second label lookup); already covered under Penultimate hop popping.
- `rfc3031.txt:2225-2227`: restatement, via an ATM-switch example, of the capability condition
  already catalogued in NULL-2 (Implicit NULL is only distributed to LSRs that can pop the
  stack).
