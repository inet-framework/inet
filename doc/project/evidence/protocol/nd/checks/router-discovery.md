# ND — English check procedures: router discovery

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Router Solicitation of a host that comes up

Checks: **RFC4861-HOST-44**, **HOST-45**, **RS-3**, **RS-4**, **RS-5**, **RS-6**, **RS-7**
(description), **RS-9**, **HOST-50** (must), **RS-11** (must not), **RS-12**, **HOST-46**,
**HOST-51** (should); covers **RFC4861-RS-2** (encoding), **OPT-11** (description), **HOST-43**
(may, lower case).

### Requirement

RFC 4861 §4.1 and §6.3.7: a host sends at least one Router Solicitation to the all-routers
address, from an address of the interface or from the unspecified address. A solicitation from
the unspecified address carries no Source Link-Layer Address option; one from an assigned address
should carry it. A Router Solicitation has hop limit 255, type 133, code 0 and zero in its
Reserved field. After a valid Router Advertisement with a non-zero Router Lifetime, the host stops
its solicitations.

### Scenario constants

- The link. Observation lasts 30 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the Router Solicitations of A and the Router Advertisements of R on L1.

### Expected observations

1. On L1, from A, a Router Solicitation: ICMPv6 type 133 (RFC4861-RS-6, HOST-51). This confirms
   the stimulus.
2. Its IPv6 destination is ff02::2, the all-routers address (RFC4861-RS-4, HOST-44).
3. Its IPv6 source is the unspecified address or an address of the interface of A
   (RFC4861-RS-3, HOST-45).
4. It carries no Source Link-Layer Address option when its source is the unspecified address
   (RFC4861-RS-11), and one with the link-layer address of A otherwise (RFC4861-RS-12, HOST-46).
5. On L1, from R, a Router Advertisement with a non-zero Router Lifetime. From that instant to
   the end of the window, no Router Solicitation leaves A (RFC4861-HOST-50).
6. The solicitation of observation 1 has hop limit 255, code 0 and zero in its Reserved field
   (RFC4861-RS-5, RS-7, RS-9).

### Notes

- Observation 5 accepts a Router Advertisement to any destination on L1: the host must stop
  after one that reaches it, and a multicast one reaches every host.

## Router Solicitations on a link without a router

Checks: **RFC4861-HOST-42** (should), **HOST-52**, **RFC4862-GLOB-4** (description).

### Requirement

RFC 4861 §6.3.7: a host sends up to MAX_RTR_SOLICITATIONS (3) Router Solicitations, at least
RTR_SOLICITATION_INTERVAL (4 seconds) apart; when none gets an answer, it concludes that the link
has no router and sends no more. RFC 4862 §5.5.2 uses the same conclusion.

### Scenario constants

- The link without a router. Observation lasts 40 seconds from the start.

### Procedure

1. Build the link without a router, and let A and B come up.
2. Observe the Router Solicitations of A on L1.

### Expected observations

1. On L1, from A, a first Router Solicitation. Record its instant. This confirms the stimulus.
2. A second Router Solicitation, at least 4 seconds after the first (RFC4861-HOST-42).
3. A third Router Solicitation, at least 4 seconds after the second (RFC4861-HOST-42).
4. No fourth Router Solicitation in the 20 seconds after the third (RFC4861-HOST-42, HOST-52,
   RFC4862-GLOB-4).

## Router Advertisement header and addressing

Checks: **RFC4861-RA-3**, **RA-13** (must), **RA-1**, **RA-4**, **RA-5**, **RA-6**, **RA-7**,
**ADV-4** (description), **RVAL-11** (must, lower case), **ADV-21** (must not); covers
**RFC4862-GLOB-3** (description).

### Requirement

RFC 4861 §4.2 and §6.1.2: a router sends a Router Advertisement from the link-local address of
the interface, to the all-nodes address or to the source of the solicitation that caused it, with
hop limit 255, type 134, code 0 and zero in its 6-bit Reserved field. §6.2.4 and §6.2.6: it sends
them periodically and in answer to solicitations, on each advertising interface. §6.2.1: a host
never sends one.

### Scenario constants

- The link. Observation lasts 30 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the Router Advertisements on L1.

### Expected observations

1. On L1, from R, a Router Advertisement: ICMPv6 type 134 (RFC4861-RA-1, RA-6, ADV-4). This
   confirms the stimulus.
2. Its IPv6 source is the link-local address of R on L1 (RFC4861-RA-3, RVAL-11).
3. Its IPv6 destination is ff02::1, or the source of a Router Solicitation that R received
   (RFC4861-RA-4).
4. Every Router Advertisement of R in the window leaves from the link-local address of R on L1
   (RFC4861-RA-3).
5. No Router Advertisement leaves A or B (RFC4861-ADV-21).
6. The advertisement of observation 1 has hop limit 255, code 0 and zero in its Reserved field
   (RFC4861-RA-5, RA-7, RA-13).

## Router Advertisement fields

Checks: **RFC4861-ADV-5**, **ADV-6**, **ADV-7**, **ADV-8**, **ADV-9**, **ADV-11**, **RA-19**,
**RA-21**, **RA-23**, **OPT-8**, **OPT-9**, **OPT-45**, **OPT-46**, **RCFG-7**, **RCFG-8**,
**RCFG-11** (description), **OPT-47**, **RCFG-10**, **RCFG-13** (must), **RCFG-12** (should, lower
case); covers **RFC4861-RA-2**, **OPT-3**, **OPT-5**, **OPT-7**, **OPT-10**, **OPT-44**
(encoding), **ADV-10**, **RCFG-9** (description), **RA-26** (may), **OPT-1**, **OPT-2** (may,
should, lower case).

### Requirement

RFC 4861 §6.2.3: each field of a Router Advertisement carries a router variable of the interface
— Router Lifetime from AdvDefaultLifetime, the M and O flags from AdvManagedFlag and
AdvOtherConfigFlag, Cur Hop Limit from AdvCurHopLimit, Reachable Time and Retrans Timer from
AdvReachableTime and AdvRetransTimer, the MTU option from AdvLinkMTU when that is not zero — and
the Source Link-Layer Address option carries the link-layer address of the interface. §4.2:
Reachable Time and Retrans Timer count milliseconds. §4.6: the option types, lengths and reserved
fields. §6.2.1 gives the default of each variable.

### Scenario constants

- The link. R sets AdvReachableTime to 30000 milliseconds and AdvRetransTimer to 1000
  milliseconds on L1, and keeps the default of every other variable: MaxRtrAdvInterval 600
  seconds, so AdvDefaultLifetime 1800 seconds; AdvManagedFlag and AdvOtherConfigFlag FALSE;
  AdvLinkMTU 0; AdvCurHopLimit the value of the Assigned Numbers registry, 64.
- Observation lasts 30 seconds from the start.

### Procedure

1. Build the link with the router variables above, and let R, A and B come up.
2. Observe the first Router Advertisement of R on L1.

### Expected observations

1. On L1, from R, a Router Advertisement. This confirms the stimulus.
2. A Source Link-Layer Address option, when present, has type 1 and length 1 and holds the
   link-layer address of R on L1 (RFC4861-RA-23, OPT-8, OPT-9).
3. The M and O flags are clear (RFC4861-ADV-6, RCFG-7, RCFG-8), and Router Lifetime is 1800
   (RFC4861-ADV-5, RCFG-13).
4. An MTU option, when present, has type 5, length 1, zero in its Reserved field and MTU 1500
   (RFC4861-ADV-11, OPT-45 to OPT-47).
5. Reachable Time is 30000 and Retrans Timer is 1000, both in milliseconds (RFC4861-ADV-8,
   ADV-9, RA-19, RA-21, RCFG-10, RCFG-11).
6. Cur Hop Limit is 64 (RFC4861-ADV-7, RCFG-12).

### Notes

- Observation 2 accepts the absence of the Source Link-Layer Address option, because
  RFC4861-RA-24 lets a router leave it out.
- Observation 4 accepts an MTU option or its absence. With AdvLinkMTU 0 the router sends none
  (RFC4861-ADV-11, RCFG-9), but RFC4861-RA-26 lets a router send one on any link, and the check
  cannot tell a router that set AdvLinkMTU to 1500 from one that did not.
- Observations 5 and 6 come last, so that a value of the wrong unit or the wrong default does not
  keep the options from a verdict.

## Prefix Information option

Checks: **RFC4861-ADV-12**, **ADV-13**, **ADV-14**, **ADV-15**, **ADV-16**, **OPT-15**,
**OPT-16**, **RCFG-14**, **RCFG-16**, **RCFG-18**, **RCFG-19**, **RCFG-22** (description),
**OPT-22**, **OPT-29**, **OPT-32** (must), **OPT-28**, **RCFG-21** (must not), **RA-27**
(should), **OPT-34**, **RCFG-15** (should not); covers **RFC4861-OPT-14**, **OPT-17**, **OPT-31**
(encoding), **OPT-4** (description), **RCFG-17**, **RCFG-20** (may).

### Requirement

RFC 4861 §6.2.3: a Router Advertisement carries a Prefix Information option for each prefix of
AdvPrefixList, which holds the on-link prefixes of the interface by default, with the L flag,
the A flag, the Valid Lifetime and the Preferred Lifetime of the prefix; the link-local prefix is
not advertised. §4.6.2: the type is 3, the length 4, both Reserved fields and the bits after the
prefix length are zero, and the Preferred Lifetime is not greater than the Valid Lifetime.
§6.2.1: AdvOnLinkFlag and AdvAutonomousFlag are TRUE, AdvValidLifetime is 2592000 seconds and
AdvPreferredLifetime is 604800 seconds by default.

### Scenario constants

- The link, with the default of every router variable. The only on-link prefix of R on L1 other
  than the link-local prefix is 2001:db8:1::/64.
- Observation lasts 30 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the first Router Advertisement of R on L1.

### Expected observations

1. On L1, from R, a Router Advertisement. This confirms the stimulus.
2. It holds a Prefix Information option for 2001:db8:1::/64, with type 3, length 4 and prefix
   length 64 (RFC4861-ADV-12, RA-27, RCFG-14, OPT-15, OPT-16).
3. It holds no Prefix Information option for fe80::/64 (RFC4861-OPT-34, RCFG-15).
4. In the option for 2001:db8:1::/64, the L and the A flag are set (RFC4861-ADV-13, ADV-15,
   RCFG-18, RCFG-22).
5. In that option, zero is in both Reserved fields and in the bits of the prefix after the first
   64 (RFC4861-OPT-22, OPT-29, OPT-32).
6. In that option, the Preferred Lifetime is not greater than the Valid Lifetime
   (RFC4861-OPT-28, RCFG-21).
7. In that option, the Valid Lifetime is 2592000 and the Preferred Lifetime is 604800
   (RFC4861-ADV-14, ADV-16, RCFG-16, RCFG-19).

### Notes

- A router may let the lifetimes decrement in real time (RFC4861-RCFG-17, RCFG-20). The default
  is a fixed value, and the check observes the first advertisement only, so observation 7 holds
  in both cases for a router that keeps the defaults.

## Router Advertisement in answer to a solicitation

Checks: **RFC4861-ADV-29**, **RS-1**, **ADV-34**, **ADV-35** (description), **ADV-32**,
**ADV-33** (must), **ADV-19** (should); covers **RFC4861-ADV-30** (may).

### Requirement

RFC 4861 §6.2.6: a router answers a valid Router Solicitation on an advertising interface with a
Router Advertisement, after a random delay of up to MAX_RA_DELAY_TIME (0.5 seconds), and never
sends two multicast advertisements closer than MIN_DELAY_BETWEEN_RAS (3 seconds); an answer that
the second rule holds back goes MIN_DELAY_BETWEEN_RAS plus the random delay after the last
multicast advertisement. §6.2.3: a solicited advertisement holds all the options.

### Scenario constants

- The link. Host B comes up at 10 seconds, when A has finished its solicitations and R has
  answered them.
- Observation lasts 30 seconds from the start.

### Size or value arithmetic

The answer comes within 0.5 seconds of the solicitation or, when R sent a multicast advertisement
less than 3 seconds before the solicitation, within 3 + 0.5 seconds of that advertisement: at
most 3.5 seconds after the solicitation in every case.

### Procedure

1. Build the link, with B down until 10 seconds.
2. Observe the Router Solicitations of B and the Router Advertisements of R on L1.

### Expected observations

1. On L1, from B, after 10 seconds, a Router Solicitation. Record its instant. This confirms the
   stimulus.
2. On L1, from R, within 3.5 seconds of it, a Router Advertisement to ff02::1 or to the source of
   the solicitation of B (RFC4861-ADV-29, RS-1, ADV-32, ADV-34, ADV-35).
3. That advertisement holds the Source Link-Layer Address option and the Prefix Information option
   for 2001:db8:1::/64 (RFC4861-ADV-19).
4. No two multicast Router Advertisements of R in the window are less than 3 seconds apart
   (RFC4861-ADV-33).

## Unsolicited Router Advertisements

Checks: **RFC4861-ADV-22**, **ADV-31** (description), **ADV-23** (should), **ADV-36** (must not),
**RCFG-5**, **RCFG-6** (must).

### Requirement

RFC 4861 §6.2.4: after each multicast Router Advertisement the router sets its timer to a random
value between MinRtrAdvInterval and MaxRtrAdvInterval; for the first MAX_INITIAL_RTR_ADVERTISEMENTS
(3) advertisements of an interface, the interval is at most MAX_INITIAL_RTR_ADVERT_INTERVAL (16
seconds). Only answers to solicitations come more often than MinRtrAdvInterval. §6.2.1: the
default MaxRtrAdvInterval is 600 seconds, and the default MinRtrAdvInterval is 0.33 ×
MaxRtrAdvInterval.

### Scenario constants

- The link, with the default MaxRtrAdvInterval of 600 seconds, so MinRtrAdvInterval is 0.33 × 600
  = 198 seconds.
- Observation lasts 1900 seconds from the start: room for the three first advertisements, one
  more interval that the text leaves open, and two full intervals after it.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the multicast Router Advertisements of R on L1, and record the instant of each one.

### Expected observations

1. On L1, from R, a first multicast Router Advertisement. This confirms the stimulus.
2. The second comes at most 16 seconds after the first, and the third at most 16 seconds after
   the second (RFC4861-ADV-23).
3. The fifth comes 198 to 600 seconds after the fourth, and the sixth 198 to 600 seconds after
   the fifth (RFC4861-ADV-22, ADV-36, RCFG-5, RCFG-6).

### Notes

- The text caps the timer for "the first few advertisements"; whether the cap also holds for the
  interval after the third one is open, so the check does not judge the interval from the third
  to the fourth.
- A multicast answer to a solicitation also restarts the timer (RFC4861-ADV-31), so the check
  counts every multicast advertisement, solicited or not. The hosts solicit only in the first
  seconds, so the fourth to the sixth advertisements are unsolicited.

## The default router

Checks: **RFC4861-HOST-12**, **HOST-39**, **RA-15** (description).

### Requirement

RFC 4861 §6.3.4: an advertisement with a non-zero Router Lifetime from a new router puts that
router into the Default Router List. §6.3.6: a host sends a packet for an off-link destination
to a router of that list.

### Scenario constants

- The path. At 10 seconds, host A sends one ICMPv6 echo request to the global address of host C.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the path, and let R, A and C come up.
2. At 10 seconds, let A send an echo request to C.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with a non-zero Router Lifetime (RFC4861-RA-15). This
   confirms the stimulus.
2. On L1, from A, the echo request to C, with the link-layer address of R on L1 as its Ethernet
   destination (RFC4861-HOST-12, HOST-39).

## A router with Router Lifetime zero

Checks: **RFC4861-RA-17** (should not); covers **RFC4861-ADV-17** (description).

### Requirement

RFC 4861 §4.2: a router that advertises Router Lifetime zero is not a default router, and a host
does not put it into its Default Router List. §6.2.3: a router that advertises but does not want
to be a default router sends Router Lifetime zero.

### Scenario constants

- The two routers. R2 sets AdvDefaultLifetime to zero on L1; R1 keeps the default.
- At 10 seconds, host A sends one ICMPv6 echo request to the global address of host C, behind R2.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the two routers, and let R1, R2, A and C come up.
2. At 10 seconds, let A send an echo request to C.
3. Observe L1.

### Expected observations

1. On L1, from R2, a Router Advertisement with Router Lifetime zero (covers RFC4861-ADV-17). This
   confirms the stimulus.
2. On L1, from A, the first echo request to C, with the link-layer address of R1 as its Ethernet
   destination and not the one of R2 (RFC4861-RA-17).

### Notes

- Only the first packet counts: R1 may send A a Redirect to R2, and a later packet that goes to
  R2 after the Redirect is correct. The check of the Redirect is in
  [`redirect.md`](redirect.md#redirect-from-the-first-hop-router).
