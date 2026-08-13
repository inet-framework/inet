# Mobile IPv6 showcase — plan (working file, do not commit)

Converged 2026-08-13 (Phase 1 teach-back passed). Branch `topic/gy/mipv6-showcase`,
worktree `/home/user/inet-mipv6`. Existing WIP `mipv6handover` showcase in another
worktree is deliberately ignored — this is a from-scratch build.

## Claim (one sentence)

An ongoing session with a mobile node survives its move between IPv6 networks:
without Mobile IPv6 the node becomes unreachable the moment it leaves home; with it,
traffic keeps flowing — first through the home-agent tunnel, then on a shorter direct
path once route optimization completes, and back to plain IPv6 when the node returns
home.

## Decisions (user-confirmed)

- Path: `showcases/general/mipv6/`, network `Mipv6Showcase`, wired into
  `showcases/general/index.rst`.
- Detail levels: protocol ≈ 6, INET config ≈ 5, C++ = **0** (no implementation
  internals in the doc).
- Traffic: **ping only** (CN → MN home address, ~0.5 s interval, whole run).
  OPEN option: add a UDP stream / throughput view later.
- **One** correspondent node. OPEN option: second CN (per-CN independent route
  optimization, as in the example's RouteOptimizationTwoCNs).
- Movement has a **return-home third act** in all configs (dwell home → dash to
  foreign net, dwell → dash back home, settle). Exercises the recently fixed
  de-registration paths (`80ea4afcc7`, `a1773e03a8`).
- **Millisecond-scale backbone link delays** so the three routing modes separate
  visibly on the RTT chart; the delays give analytic RTT predictions per phase.

## Configs

1. `WithoutMipv6` — MN is a plain IPv6 wireless host (parametric node type in the
   showcase NED). Pings die abroad, resume after return home. The motivation config.
2. `BidirectionalTunneling` — `useRouteOptimization = false`. Outage, then tunneling
   plateau (IPv6-in-IPv6 via home agent, both directions).
3. `RouteOptimization` — default (on). Return-routability then BU to CN; RTT steps
   down to near-direct; lifetime-0 de-registration at HA + CN on return.

## Evidence (Results artifacts; capture only after Phase 2.7 clean verdict)

- Hero: RTT-over-time chart of CN's pings, phase-annotated (home / outage /
  tunneled / route-optimized / outage / home). WithoutMipv6 flatlines abroad.
- Video with MN status label (SSID + `mobilityStatus`) + mobility/transmission
  visualizers.
- Sequence chart: BU/BAck registration + the 4-message return-routability exchange.
- Qtenv packet dissections side by side: tunneled (double IPv6 header) vs
  route-optimized (type-2 routing header / home-address dest option).
- Module-interior screenshot of `Ipv6NetworkLayer` (mipv6 beside neighbourDiscovery,
  icmpv6) — user-confirmed item.
- Nice-to-have: HA binding-cache inspector while MN is away.

## Doc outline

Goals → About Mobile IPv6 → Mobile IPv6 in INET → The Model → 3 config sections
(literalinclude) → Results → Try It Yourself → Discussion.

Style constraints: full name + abbreviation in parentheses long past first use
(user rule, see memory `spell-out-abbreviations`); ini nearly comment-free (doc prose
explains; never literalinclude a comment); FIGURE/VIDEO RECIPE comments for every
artifact; numbered handover timeline (below) is the About section's backbone.

Handover timeline (doc backbone): 1. L2 re-association → 2. movement detection
(RS/RA, new prefix) → 3. CoA via SLAAC + DAD → 4. BU/BAck with HA (no security
handshake — pre-arranged trust; reachable again, tunneled) → 5. optional per-CN
return routability (HoTI/CoTI/HoT/CoT) + BU → direct path. Return home: recognize
home prefix → lifetime-0 BU to HA (and CNs) → unsolicited NA (override) reclaims
the address.

## Reader questions to answer in About section (from user Q&A — actual reader preview)

- What is SLAAC (no server, no announce step; DAD probe = NS, silence = keep).
- MN/CN/HA/HoA/CoA glossary up front; CN = just the peer, can be anywhere (third
  network); either side may initiate.
- Where the home agent lives (role on a router on the home link) and that it is
  idle while MN is at home.
- Proxy ND: HA answers NS with its own MAC (not IP), only while MN away; NS is an
  owner-summons ("the node that IS X: report your MAC"), not a crowd lookup; nothing
  enforces it → proxying and spoofing share the mechanism.
- No announce step in SLAAC vs unsolicited NA (override) in MIPv6 choreography —
  keep separate.
- "Access router" is the term (no "care router").
- Return routability is PURE security (no routing function); why RO needs no
  handshake toward HA (pre-arranged trust); RO = both directions bypass HA; apps
  keep seeing HoA (type-2 RH / HAO swap at network layer).
- RO on/off: capability + per-peer fallback; why off can be a feature (location
  privacy, signaling economy).
- De-registration = BU with lifetime 0; why explicit (HA keeps stealing traffic
  until expiry otherwise).
- Who needs to be in on it: MN + HA (+ participating CNs); foreign network fully
  oblivious (contrast PMIPv6 inversion — possible future showcase).

## Expected observables (falsifiable; verify in Phase 2.7)

Movement times are placeholders until movement.xml is written; final times get
pinned there and re-checked.

1. WithoutMipv6: loss ≈ 0 at home; exactly 0 echo replies between foreign arrival
   and return home; replies resume within seconds of home re-association.
2. BidirectionalTunneling: outage = L2 scan/assoc + movement detection + DAD (~1 s)
   + BU/BAck RTT (literature anchor: classic MIPv6 handover-latency decomposition;
   compute predicted total from chosen parameters). Tunneled RTT ≈ 2·(CN↔HA + HA↔MN
   path delays) + encapsulation overhead (40 B extra on the wifi leg) — exact number
   from link-delay arithmetic once delays fixed. HA↔backbone wire carries
   IPv6-in-IPv6.
3. RouteOptimization: exactly one RR exchange per CN, strictly after HA registration
   (HoTI needs the tunnel) and before first direct packet; afterwards HA link carries
   no ping traffic; RTT ≈ 2·(CN↔MN direct path).
4. Return home: lifetime-0 BUs visible at HA (+CN in RO config); HA binding cache
   empty after; RTT back at home baseline.
5. MN status label transitions home → away → (route-optimized) → home at matching
   times.

## Files to create (Phase 2, only after Phase 1.75 verdict + user go)

`Mipv6Showcase.ned`, `omnetpp.ini` (General + 3 configs), `movement.xml`,
`doc/index.rst`, `doc/media/*`; toctree line in `showcases/general/index.rst`.
Verification: run all 3 configs, Cmdenv, release build (built in this worktree).

## Base material

`examples/ipv6/mipv6` (topology + ini patterns: SLAAC-only hosts via
`assignAddressesToHosts = false`, channel-per-network wifi, EUI-64-predictable
hardcoded ping target, status display string) and `examples/ipv6/mipv6roaming`
(multi-network roaming patterns). Module tests exist: return_home,
route_optimization (+two CNs), binding_refresh, return_home_route_optimization.

## Phase 1.5 enrichment findings (2026-08-13)

- Movement/timing skeleton validated by `MIPv6_return_home_route_optimization.test`:
  TurtleMobility `wait 12s → moveto foreign 3s → wait 20s → moveto home 3s → wait 22s`
  (60 s total), CN pinging 0.5 s throughout. Adapt for movement.xml.
- `detectL2Movement` (default on): RS sent immediately on L2 association → handover
  works with realistic RA intervals (validated with 150–600 s RAs). Explain in the
  INET section as "how INET detects movement".
- Early-RO validated: RO triggers promptly when tunneled CN traffic arrives.
- Second-handover RR re-run validated (fresh CoTI per CoA).
- NEW mechanism found: **Binding Refresh Request (BRR)** — CN prompts MN to re-run
  return routability before the RO binding expires (`maxRrBindingLifeTime`, default
  presumably 420 s ⇒ won't fire within our ~80 s run; audit + maybe one doc sentence).
- Tests use `**.wlan*.mgmt.numAuthSteps = 4`; examples don't set it — check default
  at implementation time.

## Phase 1.75 checklists (written BEFORE reading the C++ — grading follows)

### A. RFC 6275 required mechanisms (reader-expectation reference)

MN: movement detection (+recognize home); CoA via SLAAC + DAD on visited link;
home registration BU (seq, lifetime) with retransmission/backoff until BAck;
binding refresh before expiry; reverse tunneling out / decap in; RR initiation
(HoTI via tunnel, CoTI direct, token combine → Kbm); BU-to-CN with authenticator;
HAO on direct MN→CN; type-2 RH processing on CN→MN; return-home de-registration
(lifetime-0 BU to HA + CNs, stop tunnel, unsolicited NA override reclaim); fresh
CoTI/RR per new CoA on repeat handover; respond to BRR; BAck status handling;
ICMPv6 error fallback (CN without MIPv6 → stay tunneled).

HA: accept registration, binding cache (lifetime, seq); proxy ND while binding
active (answer NS with own MAC, gratuitous NA on takeover); intercept + encap
CN→MN; decap reverse tunnel; process de-registration; expire bindings (soft
state); BAck with granted lifetime; (DAD-defend HoA on home link); (DHAAD:
likely unmodeled — expect gap, judge blocker axis); (mobile prefix discovery:
likely unmodeled).

CN: HoT/CoT with keygen tokens + nonces; verify BU authenticator; binding cache
w/ ~420 s max lifetime; insert type-2 RH to CoA; process HAO inbound (apps see
HoA); BRR before expiry; process de-registration.

Security modeling: IPsec MN↔HA not expected in a simulator (document openly);
RR token crypto may be simplified — acceptable if message sequence/timing real.

### B. Scenario-required capabilities

1. Parametric MN type: plain wireless IPv6 host for WithoutMipv6 (StandardHost6
   + numWlanInterfaces=1?) — verify exists & associates.
2. `useRouteOptimization` param exists AND is read/gates RO in C++.
3. ms-scale link delays don't break timers (BU retrans, RR, ND, PingApp).
4. PingApp records per-ping RTT vector + loss observable → hero chart.
5. Configurator+SLAAC split works (routers assigned, hosts SLAAC; routes to home
   prefix at CN; validated by examples).
6. `mobilityStatus` display string (validated by examples).
7. Serializers for Mobility Header, IPv6-in-IPv6, type-2 RH, HAO → Qtenv
   dissection + optional pcap.
8. TurtleMobility return-home script (validated by test).
9. Channel-per-network wifi + scanning agent (validated).
10. BU/BAck/RR event times visible in logs/elog for chart annotation.
11. Ipv6Tunneling used both directions.
12. BindingCache contents inspectable in Qtenv (nice-to-have screenshot).

## Phase 1.75 audit results (2026-08-13)

Graded vs C++ (code anchors in parens). IMPLEMENTED w/ test evidence: movement
detection (detectL2Movement), CoA SLAAC+DAD (dupAddrDetectTransmits default 1 ⇒
~1 s), BU/BAck registration + retransmission timers (createBUTimer), tunnels both
endpoints (Mipv6.cc:886 HA, :1143 MN on BAck; destroy on return/new CoA), RR
exchange + RO data paths (type-2 RH handler registered Mipv6.cc:122,
processType2RH/validateType2RH, HAO insert :2052 / processHoAOpt), fresh
CoTI/redo-RO on new CoA (:1283-1289), BRR loop (:2676 CN, :2691 MN), return-home
de-registration RFC 9.5.1 branch (lifetime-0/CoA==HoA, HA-validated), early-RO
trigger, useRouteOptimization gate (:1270), HA address learned from RA H+R flags,
mipv6RoCompleted signal (:1163), mobilityStatus display string.

SIMPLIFIED (non-blocking, note openly in INET section): RR token crypto (no
nonces — TODOs :982/:1546/:1567; "quick and dirty" auth compare :992; no token
expiry in BUL :1632/:1725); IPsec MN↔HA not modeled (sim convention); ICMPv6
error fallback is passive (errors discarded gracefully — :204; RO simply never
completes vs a non-MIPv6 peer, traffic stays tunneled).

MISSING (both axes N): HA on-link proxy ND / DAD-defense of HoA — zero proxy
code in mipv6/ (only PMIPv6 P-flag msg defs); interception works because the HA
IS the home-link router (sufficient for scenario + examples). Gratuitous-NA
reclaim on return home: sendGratuitousNa exists but default false → reclamation
via normal NUD convergence (validated by return-home test); keep default, doc
notes it. DHAAD + Mobile Prefix Discovery unmodeled (HA learned from RA instead).

Gap table: all gaps blocker-by-definition=N AND impacts-current-scope=N.
VERDICT: **Suitable as-is** (under the converged claim). Doc consequence: About
teaches proxy ND as protocol knowledge; "MIPv6 in INET" states the model's
actual interception (home-router role) + the simplifications above.

Scenario capabilities: all 12 confirmed (parametric plain host via
LinkLayerNodeBase.numWlanInterfaces; PingApp @statistic[rtt] vector; ms link
delays safe vs seconds-scale protocol timers; serializers present incl. recent
dest-opt length fix; BindingCache inspectability = verify at capture time).

## Phase 2.6 + 2.7 results (2026-08-13) — VERDICT: demonstrates the claim cleanly

All 3 configs exit 0; rtt vectors recorded. Home address pinned:
`2001:db8:0:1:8aa:ff:fe00:d` (home prefix 2001:db8:0:1::/64, wlan0 EUI-64).
Movement: home 0-15s, dash 15-18, foreign until 48, dash 48-51, home to 80.

Measured vs predicted (link-delay arithmetic; HA-BB 5ms, FR-BB 8ms, CN-BB 1ms):
- home baseline: predicted ~14ms → measured 13.87-14.14ms (identical across
  configs pre-divergence, same seed)
- tunneling plateau: ~40ms → 40.26-40.40ms (whole away phase)
- route-optimized: ~20ms → 20.08-20.22ms (20.2ms < 26ms HA-floor ⇒ arithmetic
  proof the HA is bypassed)
- WithoutMipv6: single dead gap 17.01→54.51s (exactly the away time), revives on
  re-entering home coverage; 75 vs 138/136 replies.
- outage out: 17.01→21.54s (~4.5s); back: ~50→53s.

Protocol-event timeline (RouteOptimization, verbose re-run):
- 20.04 BU#1 to HA (registration; right after CoA DAD) — unanswered
- 21.04 BU#2 retransmission (+1.0s = RFC initial retransmission interval) — BAck
- 20.55 CoTI at CN (direct path, needs no tunnel); HoTI at CN 21.56+21.57
  (duplicate = retransmission; HoTI needed the tunnel ⇒ arrives after BAck)
- 21.578 BU to CN → step-down 40.4→20.2ms between t=21.54 and t=22.02 replies
- Return: 52.82 lifetime-0 BUs to HA AND CN; "Deregistered binding" logged at
  both (52.824, 52.832); baseline restored from t=53.01

Figure-explanation notes (things visible in seqchart that MUST be explained or
filtered): BU initial retransmission (1s), duplicate HoTI, CoTI-before-HoTI
asymmetry (CoTI direct vs HoTI via not-yet-ready tunnel). Ping at 21.0 lost
(in-flight during registration) — fine at 0.5s granularity.

Artifact capture UNLOCKED. Verbose re-run overwrote RouteOptimization results
with identical (same-seed) data — deterministic, OK.

## Phase status

- Phase 0 discovery: done. Phase 1 shared understanding: CONVERGED (this file).
- Phase 1.5 enrichment: done (see findings).
- Phase 1.75 feasibility audit: done — verdict "Suitable as-is" AWAITING USER
  DECISION; no showcase files until user accepts.
