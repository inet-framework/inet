# RFC 8930 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8930-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 8930 in the selected 6LoWPAN family.
Source: [rfc8930.txt](rfc8930.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc8930.txt>. The publication version and family
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
| [RFC8930-FWD-1](#rfc8930-fwd-1) | First-fragment forwarding and state creation are atomic. |
| [RFC8930-FWD-2](#rfc8930-fwd-2) | Each forwarding hop rewrites the tag using its own namespace. |
| [RFC8930-FWD-3](#rfc8930-fwd-3) | Later fragments use source-address and tag state; missing state causes a drop. |
| [RFC8930-FWD-4](#rfc8930-fwd-4) | The first fragment is transmitted before later fragments. |
| [RFC8930-GAP-1](#rfc8930-gap-1) | Consecutive fragments are separated to allow progress beyond the next hop and interference domain. |
| [RFC8930-SEC-1](#rfc8930-sec-1) | A firewall should reassemble and inspect uncompressed IP packets before forwarding fragments. |
| [RFC8930-SEC-2](#rfc8930-sec-2) | A firewall drops a datagram whose overlapping fragments disagree. |
| [RFC8930-VRB-1](#rfc8930-vrb-1) | VRB allocation is capacity-bounded and old VRBs expire on a timer. |
| [RFC8930-TAG-1](#rfc8930-tag-1) | Tags should be assigned pseudorandomly to reduce prediction attacks. |

## Checkable statements

### RFC8930-FWD-1

**First-fragment forwarding and state creation are atomic.**

> A 6LoWPAN Fragment Forwarding technique makes the routing decision on
> the first fragment, which is always the one with the IPv6 address of
> the destination.  Upon receiving a first fragment, a forwarding node
> (e.g., node B in an A->B->C sequence) that does fragment forwarding
> MUST attempt to create a state and forward the fragment.  This is an
> atomic operation, and if the first fragment cannot be forwarded, then
> the state MUST be removed.

— §5, `rfc8930.txt:323-329`.

- Strength: must. Class: internal.
- Check idea: Make first-fragment forwarding fail and verify that no forwarding state remains.

### RFC8930-FWD-2

**Each forwarding hop rewrites the tag using its own namespace.**

> Since the Datagram_Tag is uniquely associated with the source link-
> layer address of the fragment, the forwarding node MUST assign a new
> Datagram_Tag from its own namespace for the next hop and rewrite the
> fragment header of each fragment with that Datagram_Tag.

— §5, `rfc8930.txt:331-334`.

- Strength: must. Class: wire.
- Check idea: Forward interleaved datagrams from different senders and verify consistent outgoing tags for each datagram.

### RFC8930-FWD-3

**Later fragments use source-address and tag state; missing state causes a drop.**

> When a forwarding node receives a fragment other than a first
> fragment, it MUST look up state based on the source link-layer
> address and the Datagram_Tag in the received fragment.  If no such
> state is found, the fragment MUST be dropped; otherwise, the fragment
> MUST be forwarded using the information in the state found.

— §5, `rfc8930.txt:336-340`.

- Strength: must. Class: wire.
- Check idea: Send a later fragment before state exists, then repeat with state installed and check drop versus forwarding.

### RFC8930-FWD-4

**The first fragment is transmitted before later fragments.**

> A node that has not received the first fragment cannot forward the
> next fragments.  This means that if node B receives a fragment, node
> A was in possession of the first fragment at some point.  To keep the
> operation simple and consistent with [RFC4944], the first fragment
> MUST always be sent first.  When that is done, if node B receives a
> fragment that is not the first and for which it has no state, then
> node B treats it as an error and refrains from creating a state or
> attempting to forward.  This also means that node A should perform
> all its possible retries on the first fragment before it attempts to
> send the next fragments, and that it should abort the datagram and
> release its state if it fails to send the first fragment.

— §5, `rfc8930.txt:365-375`.

- Strength: must. Class: wire.
- Check idea: Capture a fragmented transmission and verify the first-fragment ordering.

### RFC8930-GAP-1

**Consecutive fragments are separated to allow progress beyond the next hop and interference domain.**

> Fragment forwarding obviates some of the benefits of the 6LoWPAN
> header compression [RFC6282] in intermediate hops.  In return, the
> memory used to store the packet is distributed along the path, which
> limits the buffer-bloat effect.  Multiple fragments may progress
> simultaneously along the network as long as they do not interfere.
> An associated caveat is that on a half-duplex radio, if node A sends
> the next fragment at the same time as node B forwards the previous
> fragment to node C down the path, then node B will miss it.  If node
> C forwards the previous fragment to node D at the same time and on
> the same frequency as node A sends the next fragment to node B, this
> may result in a hidden terminal problem.  In that case, the
> transmission from node C interferes at node B with that from node A,
> unbeknownst to node A.  Consecutive fragments of a same datagram MUST
> be separated with an inter-frame gap that allows one fragment to
> progress beyond the next hop and beyond the interference domain
> before the next shows up.  This can be achieved by interleaving
> packets or fragments sent via different next-hop routers.

— §5, `rfc8930.txt:377-393`.

- Strength: must. Class: wire.
- Check idea: Use a controlled multihop interference topology and compare observed inter-frame spacing with the configured propagation allowance.

### RFC8930-SEC-1

**A firewall should reassemble and inspect uncompressed IP packets before forwarding fragments.**

> *  Overlapping fragment attacks are possible with 6LoWPAN fragments,
> but there is no known firewall operation that would work on
> 6LoWPAN fragments at the time of this writing, so the exposure is
> limited.  An implementation of a firewall SHOULD NOT forward
> fragments but instead should recompose the IP packet, check it in
> the uncompressed form, and then forward it again as fragments if
> necessary.  Overlapping fragments are acceptable as long as they
> contain the same payload.  The firewall MUST drop the whole packet
> if overlapping fragments are encountered that result in different
> data at the same offset.

— §7, `rfc8930.txt:462-471`.

- Strength: should. Class: end-to-end.
- Check idea: Exercise a firewall with fragmented allowed and denied packets and check decisions on the reconstructed packet.

### RFC8930-SEC-2

**A firewall drops a datagram whose overlapping fragments disagree.**

> *  Overlapping fragment attacks are possible with 6LoWPAN fragments,
> but there is no known firewall operation that would work on
> 6LoWPAN fragments at the time of this writing, so the exposure is
> limited.  An implementation of a firewall SHOULD NOT forward
> fragments but instead should recompose the IP packet, check it in
> the uncompressed form, and then forward it again as fragments if
> necessary.  Overlapping fragments are acceptable as long as they
> contain the same payload.  The firewall MUST drop the whole packet
> if overlapping fragments are encountered that result in different
> data at the same offset.

— §7, `rfc8930.txt:462-471`.

- Strength: must. Class: end-to-end.
- Check idea: Inject conflicting overlapping payload bytes through a firewall and verify the entire datagram is dropped.

### RFC8930-VRB-1

**VRB allocation is capacity-bounded and old VRBs expire on a timer.**

> *  Resource-exhaustion attacks are certainly possible and a sensitive
> issue in a constrained network.  An attacker can perform a DoS
> attack on a node implementing VRB by generating a large number of
> bogus first fragments without sending subsequent fragments.  This
> causes the VRB table to fill up.  When hop-by-hop reassembly is
> used, the same attack can be more damaging if the node allocates a
> full Datagram_Size for each bogus first fragment.  With the VRB,
> the attack can be performed remotely on all nodes along a path,
> but each node suffers a lesser hit.  This is because the VRB does
> not need to remember the full datagram as received so far but only
> possibly a few octets from the last fragment that could not fit in
> it.  An implementation MUST protect itself to keep the number of
> VRBs within capacity and to ensure that old VRBs are protected by
> a timer of a reasonable duration for the technology and destroyed
> upon timeout.

— §7, `rfc8930.txt:473-487`.

- Strength: must. Class: internal.
- Check idea: Exhaust incomplete-datagram state, verify bounded allocation, and verify recovery after expiry.

### RFC8930-TAG-1

**Tags should be assigned pseudorandomly to reduce prediction attacks.**

> *  Attacks based on predictable fragment identification values are
> also possible but can be avoided.  The Datagram_Tag SHOULD be
> assigned pseudorandomly in order to reduce the risk of such
> attacks.  A larger size of the Datagram_Tag makes the guessing
> more difficult and reduces the chances of an accidental reuse
> while the original packet is still in flight, at the expense of
> more space in each frame.  Nonetheless, some level of risk remains
> because an attacker that is able to authenticate to and send
> traffic on the network can guess a valid Datagram_Tag value, since
> there are only a limited number of possible values.

— §7, `rfc8930.txt:489-498`.

- Strength: should. Class: wire.
- Check idea: Inspect the assignment algorithm and a controlled sequence for pseudorandom generation; do not claim unpredictability from a short trace alone.

## Areas outside this selection

Forwarding rules (§5) and selected state/security constraints (§7) are selected. The illustrative VRB data structure (§6), performance discussion (§4), link-specific gap sizing and complete security mitigation design (§7) are outside the selection. A generic fragment forwarder is not automatically a firewall; the firewall entries apply only to that role.
