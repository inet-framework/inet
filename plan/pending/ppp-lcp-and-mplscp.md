# PPP link control and the MPLS Control Protocol

**Status:** not started. Written 2026-09-25 on `topic/standards-tests-mpls-level2-fixes`, as the
plan for gap 7 of the MPLS results; the user decided that large missing features get a plan and
no code in the repair branches.

## Goal

The PPP model sends a frame of any protocol from the first moment. RFC 3032 §4 wants the MPLS
Control Protocol (MPLSCP, PPP Protocol 8281 hex) in the Opened state before the first labeled
packet, and MPLSCP runs in the Network-Layer Protocol phase, after LCP has opened the link (RFC
1661). So the repair has two parts: LCP first, then a network control protocol for MPLS on the
same automaton. The test that shows it is `Rfc3032PppMplscp` of `tests/protocol/mpls`, which
declares an expected FAIL today.

## Why it is large

- `Ppp.cc` (about 400 lines) has no state for the link. LCP needs the option negotiation
  automaton of RFC 1661 §4 (Configure-Request, -Ack, -Nak, -Reject, Terminate-Request, -Ack,
  Code-Reject, Protocol-Reject, Echo-Request, -Reply), its timers and counters, and the phases
  Link Dead, Establish, Authenticate, Network, Terminate.
- Every network layer protocol on a PPP link then needs its own control protocol before it may
  send: IPCP for IPv4 (RFC 1332), IPV6CP for IPv6 (RFC 5072), and MPLSCP for MPLS. Without
  IPCP, an LCP that gates the Network-Layer Protocol phase would stop IPv4 on every PPP link.
- The start of every PPP link gains an exchange of control frames and a delay, so the
  fingerprints and the statistical results of every simulation with a PPP link move.

## Steps

1. [ ] **The automaton** — one implementation of the RFC 1661 option negotiation, used by LCP and
   by each network control protocol, with the default timer and counter values of §4.6.
2. [ ] **LCP** — the Maximum-Receive-Unit and the Magic-Number options; the other options are
   rejected. The MRU becomes the MTU of the interface.
3. [ ] **IPCP** — no options (the address comes from the configurator), so IPv4 keeps working.
4. [ ] **MPLSCP** — no options (RFC 3032 §4.2, RFC3032-PPP-10); `Mpls` sends a labeled packet only
   on an interface whose MPLSCP is Opened (RFC3032-PPP-11).
5. [ ] **A parameter to switch the control protocols off**, default on, so that a study that does
   not care about link setup can keep its old trajectories.
6. [ ] **Tests** — `Rfc3032PppMplscp` passes and its declaration goes; the level 3 statements of
   RFC 3032 §4.2 (a packet before the Network-Layer Protocol phase, an unknown Code) get checks.
7. [ ] **Baselines** — every PPP fingerprint and statistical result, attributed to the commit that
   moves it.

## Open questions

- Whether IPV6CP belongs to the same series: IPv6 on a PPP link has the same gate.
- Whether the default of the switch in step 5 should be off at first, so that the baselines of
  the users of PPP do not move in the release that brings LCP.
