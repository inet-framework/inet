# INET Mobile IPv6 — implementation gaps report

Audited 2026-08-13 on `master` (`fa3c69f237`), as the Phase 1.75 feasibility gate of
the `showcases/general/mipv6` showcase. Verdict there: *suitable as-is* — none of
the gaps below blocks the showcase (no host ever sits on the home link in its
scenario, the correspondent node supports MIPv6, and crypto fidelity is not what a
behavior showcase demonstrates). They are recorded here as future implementation
work, roughly ordered by how likely a user is to hit them.

## 1. Home agent on-link proxy Neighbor Discovery (missing)

**RFC 6275 §10.4.1:** while a binding exists, the home agent must act as an ND proxy
for the mobile node's home address: answer Neighbor Solicitations for it with its own
link-layer address, send a multicast Neighbor Advertisement when the binding is
accepted (to steal the neighbors' cache entries), and perform DAD for the home
address on the home link.

**INET today:** no proxy code exists anywhere in `src/inet/networklayer/mipv6/`
(the only "proxy" matches are PMIPv6 P-flag message definitions in
`MobilityHeader.msg`). Interception works solely because the home agent is the home
network's *router*, so off-link traffic to the home prefix flows through it anyway.
The related "HA DAD problem" is worked around with a delayed-send hack
(`Mipv6.cc:645`, the `sendTime` parameter of `sendMobilityMessageToIPv6Module`).

**Concrete failure scenario:** put an ordinary host on the home link and let it ping
the mobile node while the mobile node is away. The host resolves the home address
on-link (Neighbor Solicitation), nobody answers, the ping blackholes — even though
the home agent has a perfectly valid binding one hop away.

**Fix sketch:** on accepting a home registration, register the home address as a
proxy target in `Ipv6NeighbourDiscovery` (answer NS for it with the HA's MAC, emit
the takeover NA); deregister on de-registration/expiry; run real DAD for the home
address instead of the delay hack. Moderate effort; the `sendUnsolicitedNa()`
helper and the RFC-4861 proxy passages quoted around
`Ipv6NeighbourDiscovery.cc:2000–2111` are the natural attachment points.

**Related quirk (verified empirically, round-2 review):** with `sendRedirects`
left on, the home agent emits one ICMPv6 Redirect per *reverse-tunneled reply* —
not for intercepted forward traffic (the pre-routing hook at `Mipv6.cc:2345-2348`
steers that into the tunnel before the Redirect check at `Ipv6.cc:528-538`).
After decapsulation, the inner HoA→CN packet is re-processed with the *physical*
arrival interface (`Ipv6.cc:845-857`) and forwarded back out of it, tripping the
RFC 4861 §8.2 condition; the Redirect is addressed, uselessly, to the mobile
node's home address. A real stack attributes decapsulated packets to the tunnel
interface (and §8.2 also requires the source to be an on-link neighbor, a test
INET's simplified check omits). Until fixed, scenarios should disable
`Ipv6.sendRedirects` on the home agent (the showcase does).

## 2. Return-home address reclaim via gratuitous NA (partial)

**RFC 6275 §11.5.5:** a returning mobile node reclaims its home address by sending
an unsolicited Neighbor Advertisement (override flag set) so neighbors flush the
home agent's MAC from their caches.

**INET today:** `Ipv6NeighbourDiscovery` has a general `sendGratuitousNa` parameter
(announce every DAD-completed address), but it defaults to `false`, and there is no
MIPv6-specific reclaim NA. Reclamation happens via ordinary Neighbor Unreachability
Detection convergence — validated to work by `MIPv6_return_home*.test`, but with a
short staleness window that the RFC choreography avoids.

**Fix sketch:** small — on the returning-home path, send the override NA for the
home address explicitly (or scope `sendGratuitousNa` semantics for the MIPv6 case).
Interacts with gap 1 (the HA must also *stop* proxying at the same moment).

## 3. Return-routability token crypto (schematic)

**RFC 6275 §5.2:** keygen tokens derive from a node key and rotating *nonces*; the
Binding Update to a correspondent carries an authenticator computed from
Kbm = SHA1(home token | care-of token); tokens expire (MAX_TOKEN_LIFETIME 210 s,
nonces rotate).

**INET today:** tokens are deterministic functions with the nonce hardcoded to 0
(`Mipv6.cc:982–983`, `:1546`, `:1567` — `// TODO nonce`), the authenticator check is
a direct token comparison marked "quick and dirty" (`:992`), and token expiry is not
tracked in the BindingUpdateList (disabled code at `:1632`, `:1725`). The *message
sequence, paths, sizes and timing* are correct — what's schematic is only the
cryptographic content.

**Impact:** none for behavior/performance studies; disqualifying only for security
research (which the model shouldn't be used for anyway). Fix is mechanical: nonce
arrays + indices in HoT/CoT, real Kbm derivation, expiry bookkeeping in the BUL.

## 4. No explicit fallback against a non-MIPv6 correspondent (simplified)

**RFC 6275 §11.3.5:** a mobile node that receives an ICMP Parameter Problem for a
Mobility Header should conclude the correspondent doesn't speak MIPv6 and refrain
from further route-optimization attempts with it.

**INET today:** ICMPv6 errors reaching `Mipv6` are discarded gracefully (crash fixed
in `1b3b13c6f5`; see the comment at `Mipv6.cc:204`) but drive no state change. Route
optimization toward a plain host simply never completes and traffic stays tunneled —
behaviorally correct, with only slow HoTI retransmission noise (every
`MAX_RR_BINDING_LIFETIME × 8` s) as the cost.

**Fix sketch:** small — mark the correspondent as non-MIPv6 in the BUL on Parameter
Problem receipt and suppress further HoTI/CoTI.

## 5. Dynamic Home Agent Address Discovery + Mobile Prefix Discovery (missing)

**RFC 6275 §10.5/§11.4:** DHAAD lets a mobile node discover home agents from afar
via anycast; Mobile Prefix Discovery keeps it informed of home-prefix changes while
away.

**INET today:** neither is modeled. The mobile node learns its home agent from the
home link's Router Advertisements (H-bit + router-address R-flag processing in
`Ipv6NeighbourDiscovery.cc:1284–1454`), which requires it to boot at home.
Scenarios that start abroad, renumber the home network, or run multiple home agents
are not expressible.

## 6. IPsec on mobile-node ↔ home-agent signaling (absent by convention)

RFC 6275 mandates IPsec ESP protection for BU/BAck with the home agent. INET (like
essentially every simulator) models the exchange without it. Listed for
completeness; not proposed as work.

## 7. Early reply traffic dropped instead of reverse-tunneled (simplified)

**RFC 6275 §11.3.1:** while a mobile node has no binding at a correspondent (route
optimization not yet completed, or not used), packets it sends to that
correspondent must be reverse-tunneled through the home agent.

**INET today:** the window is opened by the home agent itself: it delays its
first Binding Acknowledgement by a hardcoded 1 s (`Mipv6.cc:856–861`,
`sendTime = existingBinding ? 0 : 1`) as a stand-in for the DAD it should
perform on the home address (`// TODO solve the HA DAD problem in a different
way`, `Mipv6.cc:645`). Until the acknowledgement lands, the mobile node's
reverse tunnel is not up, and packets sourced from the home address — data
replies *and the first HoTI* — fall through to the last-resort
topological-correctness guard in `Ipv6.cc:588` ("Using HoA instead of CoA...
dropping datagram") and are silently discarded instead of being queued or
tunneled. A knock-on: the mobile node's 1 s retransmission timer fires just
before the delayed acknowledgement arrives, so every fresh registration takes
two Binding Updates, and the first acknowledgement is then rejected for its
stale sequence number (`Mipv6.cc:1253–1257`).

**Concrete failure scenario (observed in the showcase runs):** the
correspondent's pings arrive tunneled from t≈20.54 and reach the application,
but the replies to the first two (t=20.54, t=21.02) and the first Home Test
Init (t=20.54) are dropped at the mobile node; the reverse tunnel carries
traffic only from t≈21.5, after the *second* Binding Update's acknowledgement.
Costs ~1 s of extra outage per handover, visible in the RTT chart — though
note a compliant registration also waits ~1 s for real DAD, so only the
*dropping* (vs. queueing/tunneling), not the delay itself, deviates from the
RFC.

**Fix sketch:** two independent halves. (a) Home agent: perform real DAD for
the home address instead of the fixed BAck delay (folds into gap 1's proxy-ND
work). (b) Mobile node: packets with a home-address source and no usable
binding should be held or routed into the HA tunnel once it exists — the
`Ipv6.cc:588` guard should only ever fire for genuinely unroutable leftovers.

## 8. Care-of address put into service without DAD (incorrect)

**RFC 4862 §5.4 / RFC 6275 §11.5.1:** every unicast address a node autoconfigures
is tentative until duplicate address detection clears it, and a mobile node
arriving on a new link regenerates and probes its link-local address there. A
care-of address that collides with another node's must be detected as such, not
registered.

**INET today:** at a handover the mobile node's interface already holds a
link-local plus the home address, so
`Ipv6NeighbourDiscovery::processRAPrefixInfoForAddrAutoConf()` takes its
multi-address branch (`Ipv6NeighbourDiscovery.cc:2578–2594`): it marks *every*
address on the interface tentative — the home address included — runs DAD on the
link-local address alone, and defers the new care-of address to `dadGlobalList`.
When that DAD completes, `makeTentativeAddressPermanent()` assigns the care-of
address with `tentative = false` (`:911`) and then blanket-clears the tentative
flag on all addresses (`:920–925`), so the "start DAD for a tentative global
address" loop at `:967–975` — which is correct, and does run on the non-MIPv6
path — finds nothing left to do. The care-of address therefore never gets a
probe of its own. Both halves carry their own `// TODO improve this code...`
(`:921`, `:2581`).

Two consequences:

- A care-of address that duplicates one already in use on the visited link is
  never detected. The mobile node registers it with its home agent and the
  tunnel is built onto a contested address.
- The home address is marked tentative while the node is *abroad*, which is
  precisely the address Mobile IPv6 exists to hold stable. Nothing may be sourced
  from a tentative address, so this widens the window in gap 7 rather than
  causing a separate visible failure — but it is wrong in principle, and it is
  the home link, not the visited one, on which that address's uniqueness means
  anything.

**Concrete failure scenario:** give the foreign link a second host whose
interface identifier matches the mobile node's (in the showcase, another node
with MAC `0A-AA-00-00-00-0D`). Both end up with `2001:db8:0:3:8aa:ff:fe00:d`; a
compliant mobile node would fail DAD and form another care-of address, INET's
registers the colliding one.

**Observed in the showcase runs:** at the handover the log shows the two
existing addresses set tentative at t = 18.6255 s and exactly one completion —
`DAD completed for address fe80::8aa:ff:fe00:d` at t = 20.0437 s. There is no
completion line for the care-of address `2001:db8:0:3:8aa:ff:fe00:d` anywhere in
the run, and the *Binding Update* leaves in the same event as the link-local
completion.

**Fix sketch:** in `makeTentativeAddressPermanent()`, assign the `dadGlobalList`
address as tentative rather than permanent and let the existing `:967–975` loop
probe it, holding `initiateMipv6Protocol()` until that second DAD completes;
narrow both blanket loops to the addresses actually formed from the new prefix,
which is what the two TODOs ask for. Note this *lengthens* the handover outage
by another DAD interval — the RFC-compliant cost — so the showcase's latency
budget would need re-deriving afterwards.

### Verdict (2026-08-26): confirmed and fixed -- issue #1140, PR #1141

Run through the `inet-bug-report` pipeline. Reproduced at the audit hash `fa3c69f237` and
again on `origin/master` `7b0a6de5ce`, where `Ipv6NeighbourDiscovery.cc` is **byte-identical**
to the audit hash. No branch fix, no existing issue, no existing pull request.

What this report did not know:

- **It is not showcase-dependent.** The stock example `examples/ipv6/mipv6roaming -c Roaming`
  hands over three times and reproduces it with no showcase involved. Counting
  `DAD completed for address` for `MN[0]`: link-local 4, home address 1, care-of addresses
  `2001:db8:0:4:...` and `2001:db8:0:6:...` **0 each**.
- **The decisive citation is RFC 4862 Section 5.4's third bullet**, not the section opening.
  It names this implementation choice explicitly -- "implementations [...] that only perform
  Duplicate Address Detection for the link-local address and skip the test for the global
  address that uses the same interface identifier" -- and rules that "new implementations
  MUST NOT do that optimization."
- **The RFC 6275 citation in this section is wrong.** Section 11.5.1 is *Movement Detection*.
  Care-of address formation is **Section 11.5.3, "Forming New Care-of Addresses"**, which
  presupposes DAD is performed and only debates whether the initial delay may be skipped.
- **The fix needed a second half this report did not identify.** Performing the DAD is not
  enough: the registration was started when the *link-local* DAD completed, so the Binding
  Update still left 1.67 s before the care-of address was verified. MIPv6 sets that
  datagram's source address explicitly, and the RFC 4862 guard in
  `Ipv6::fragmentPostRouting()` only covers datagrams whose source Ipv6 chooses itself, so
  nothing held it back. The registration now waits for the care-of address's own DAD.
  `dadHasFailed()` already erases the `dadGlobalList` entry, so a colliding care-of address
  is never registered -- which is what the failure scenario above asked for.

The predicted cost is real and measured: each handover is one DAD interval longer
(+1.67 s and +1.36 s in `mipv6roaming`).

The two consequences listed above were split off and **not** fixed, each with its own
write-up (in `/home/user/inet/`):

- the home address marked tentative while abroad ->
  `bug-ipv6-home-address-tentative-while-abroad.md`. Not fixed because the current behaviour
  may be load-bearing: while that address is tentative, packets sourced from it are deferred
  rather than dropped by the `Ipv6.cc` guard, so narrowing the loops could widen gap 7's
  outage instead of shrinking it. Needs measurement first.
- the general tentative-source hole -> `bug-ipv6-tentative-source-guard-explicit-source.md`.

Pre-existing failures on unmodified master, out of scope and not to be re-diagnosed:
`MIPv6_tcp_handover.test`, `IPv6_packet_too_big.test`, and a stale `~tNlb` for
`ipv6/mipv6roaming` in `mipv6-refactoring.csv` (expected `c8dc-27c2`, actual `7ed8-bee3`)
that the re-recording in PR #1141 absorbs.

## 9. Minor bookkeeping (cleanups)

- `BindingUpdateList.cc:122/:145` — `remainingLifetime` fields never updated
  (`// TODO`), so BUL entries don't age visibly in the inspector.
- `Mipv6.cc:279` — `removeBinding()`/`resetCareOfToken()` interplay flagged
  `FIXME need revision` (assert-prone ordering).
- `Mipv6.cc:1194` — binding-expiry timers scheduled a fixed `PRE_BINDING_EXPIRY`
  early, with a TODO to do it properly.
- `BindingCache.cc` — the entry's info string reads "Home Registeration" (sic)
  and embeds a literal `\n`; both are visible in inspector screenshots
  (including the showcase's binding-cache figure).

---

*Showcase-side consequences (already handled in the showcase plan): the doc's
"Mobile IPv6 in INET" section states that interception relies on the home agent
being the home network's router, that return-routability crypto is schematic, and
that IPsec is not modeled. The About section teaches proxy ND as protocol
knowledge, which is accurate — the RFC mechanism exists; INET just doesn't model
the on-link case yet.*
