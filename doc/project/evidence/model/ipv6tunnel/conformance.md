# IPv6 tunneling — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ipv6tunnel/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names RFC 2473, found with
`grep -rn '2473' src/ doc/src/users-guide/ WHATSNEW`, restricted to `.ned`/`.cc`/`.h`/`.msg`/`.rst`
and without the generated `*_m.cc`/`*_m.h` files. Four of the seven places are outside
`src/inet/networklayer/ipv6tunneling/`, the directory this protocol's own module lives in:
three in the generic IPv6 module, and one in Mobile IPv6's own copy of the tunnel-management
code, which the Mobile IPv6 pass reads for its own family and does not map here again.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2473 | [`Ipv6Tunnel.ned:12`](../../../../../src/inet/networklayer/ipv6tunneling/Ipv6Tunnel.ned) | "Worker module of ~Ipv6TunnelInterface: performs IPv6-in-IPv6 (RFC 2473) encapsulation." |
| RFC 2473 | [`Ipv6TunnelInterface.ned:13`](../../../../../src/inet/networklayer/ipv6tunneling/Ipv6TunnelInterface.ned) | "A virtual network interface that tunnels IPv6 traffic in IPv6 (RFC 2473)." The NED documentation comment, which the module reference publishes. |
| RFC 2473 | [`Ipv6Tunnel.h:18`](../../../../../src/inet/networklayer/ipv6tunneling/Ipv6Tunnel.h) | "A virtual network interface performing IPv6-in-IPv6 (RFC 2473) encapsulation." The class-level documentation comment. |
| RFC 2473 | [`Ipv6Tunnel.cc:50`](../../../../../src/inet/networklayer/ipv6tunneling/Ipv6Tunnel.cc) | `addresses->setSrcAddress(source); // the tunnel entry point (RFC 2473)`. |
| RFC 2473 | [`Ipv6.ned:50`](../../../../../src/inet/networklayer/ipv6/Ipv6.ned) | "IPv6-in-IPv6 tunnels (RFC 2473) are likewise plain ~Ipv6TunnelInterface network interfaces plus routes, not a special path in ~Ipv6." The netfilter-hooks section of the main `Ipv6` module's own documentation comment. |
| RFC 2473 | [`Ipv6RoutingTable.cc:409`](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.cc) | A comment inside the XML-triggered tunnel setup: "the interface performs the IPv6-in-IPv6 encapsulation, RFC 2473", next to the call to `createTunnelNetworkInterface`. |
| RFC 2473 | [`Ipv6.cc:847`](../../../../../src/inet/networklayer/ipv6/Ipv6.cc) | "Generic IPv6-in-IPv6 decapsulation (RFC 2473): the decapsulated inner datagram is re-processed as if received from the network..." — the decapsulation code path inside the main `Ipv6` module's packet handler. |
| RFC 2473 | [`ch-ipv6.rst:166`](../../../../../doc/src/users-guide/ch-ipv6.rst) | "IPv6-in-IPv6 tunneling (RFC 2473) is modeled as an ordinary virtual network interface, :ned:`Ipv6TunnelInterface`." The users-guide section "IPv6 Tunneling". |

Mobile IPv6 keeps its own copy of the same mechanism: `Mipv6.h:75` and `Mipv6.cc:2104` both say
"IP tunnel management (RFC 2473), moved here from the former Ipv6Tunneling module." This is the
claim the [Mobile IPv6 survey](../mipv6/conformance.md) found in its own files and left for this
document to map, per the brief that opened this wave; it is not repeated in
`model/mipv6/conformance.md`'s own claims table, which is scoped to the Mobile IPv6 family.

Mapped onto the standards map, [`standards.md`](../../protocol/ipv6tunnel/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 2473 | yes, `base` | **yes**, in eight places across three code areas | The generic `Ipv6TunnelInterface`/`Ipv6Tunnel` pair, the generic `Ipv6` module (both its NED documentation and its decapsulation code), Mobile IPv6's own copy, and the users guide. No stale citation: every mention names RFC 2473 by its only number, and the register gives it none to be stale against. |

**The finding of the survey.** IPv6 tunneling has the cleanest register entry of the three
protocols in this wave — no obsoletion, no update, nothing to date — and its claim matches: eight
citations, every one to RFC 2473 by number, none to a superseded document. The second finding is
architectural, not a citation gap: `Ipv6Tunnel.h`'s own design note (see the facts below) says
"the IPv6 core needs no tunneling-specific knowledge," and the `Ipv6TunnelInterface` pair is
indeed a plain, generic network interface — but the generic `Ipv6` module carries two places of
its own that know the mechanism is a tunnel and name RFC 2473 for it (`Ipv6.ned:50` and
`Ipv6.cc:847`), plus a third in `Ipv6RoutingTable.cc:409` for the XML-triggered tunnel it can
create on demand. The claim is real in all three places; it does not come only from the
`Ipv6TunnelInterface` pair being "a plain interface" — the core itself carries a documented,
RFC-cited statement of what a tunnel interface is for, even though it needs no separate protocol
module to act on it.

For the level 2 pass, three facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. `Ipv6Tunnel.h:20-27` states the design directly: a packet routed to the tunnel interface is
   handed to the IPv6 layer's own service interface with an `L3AddressReq` for the tunnel
   endpoints, so ordinary IPv6 encapsulation does the work; "this is just the normal
   'encapsulate an upper-layer payload' path, where the payload happens to be an IPv6 datagram."
   A level 2 check of the outer-header fields (RFC 2473 §3.1) is therefore a check of the
   generic IPv6 encapsulation path with a tunnel interface as the destination, not of code
   private to this module. The checks document should say this plainly, so a reader does not
   expect tunnel-specific field-set code that does not exist.
2. `Ipv6.cc:847-849` re-injects a decapsulated datagram through the same pre-routing hooks an
   ordinary received datagram passes through ("seen by the netfilter pre-routing hooks --
   normally"). A level 2 or 3 check of nested encapsulation (RFC 2473 §4) needs to confirm
   whether this re-injection enforces any limit on how many times a datagram can be
   re-decapsulated, since no nesting counter was found by a plain read of `Ipv6Tunnel.cc`
   (59 lines) or of the `Ipv6.cc` decapsulation site itself.
3. `grep -n 'TODO\|FIXME'` on `Ipv6Tunnel.cc`, `Ipv6Tunnel.h` and `Ipv6TunnelInterface.ned`
   returns nothing: no half-written mechanism and no stated refusal in this protocol's own
   files. Whatever a level 2 pass finds missing (the encapsulation limit option of RFC 2473
   §4.1.1, the tunnel ICMP messages of §8.1) will be an absence to characterize from scratch,
   not a comment to read.
