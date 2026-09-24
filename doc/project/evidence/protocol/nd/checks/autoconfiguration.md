# ND — English check procedures: address autoconfiguration

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4862/catalog.md](../../../standard/rfc4862/catalog.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Link-local address and its Duplicate Address Detection

Checks: **RFC4862-DAD-2** (must), **DAD-6**, **DAD-12**, **DAD-13**, **LL-5**, **CONF-3**,
**CONF-5** (description), **LL-2**, **CONF-4**, **DAD-9** (may, lower case), **RFC4861-NS-14**
(must not); covers **RFC4862-CONF-1** (description).

### Requirement

RFC 4862 §5.3: a node forms a link-local address from the prefix fe80::/64 and its interface
identifier when the interface becomes enabled. §5.4: before it assigns a unicast address, it sends
DupAddrDetectTransmits (1 by default) Neighbor Solicitations, RetransTimer (1 second by default)
apart, from the unspecified address to the solicited-node multicast address of the address, with
the address as target; the address stays tentative, and not assigned, until RetransTimer after
the last solicitation. RFC 4861 §4.3: a solicitation from the unspecified address carries no
Source Link-Layer Address option.

### Scenario constants

- The link, with the default of every variable. Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the Neighbor Solicitations of A and every packet of A on L1.

### Expected observations

1. On L1, from A, a Neighbor Solicitation from the unspecified address with target fe80:: plus
   the interface identifier of A (RFC4862-DAD-2, LL-2, LL-5). This confirms the stimulus.
2. Its IPv6 destination is the solicited-node multicast address of that target
   (RFC4862-DAD-13).
3. It carries no Source Link-Layer Address option (RFC4861-NS-14).
4. No second solicitation from the unspecified address with that target in the window
   (RFC4862-CONF-3, CONF-4, DAD-12).
5. No packet from A with that link-local address as its IPv6 source earlier than 1 second after
   the solicitation (RFC4862-DAD-6, DAD-9, CONF-5).

## Duplicate Address Detection of a router

Checks: **RFC4862-DAD-1**, **LL-1** (description).

### Requirement

RFC 4862 §5.3 and §5.4: a router also forms a link-local address, and it runs Duplicate Address
Detection on each of its addresses before it assigns it, a configured address as well.

### Scenario constants

- The link, with the default of every variable. Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the Neighbor Solicitations and the Router Advertisements of R on L1.

### Expected observations

1. On L1, from R, a Neighbor Solicitation from the unspecified address with the link-local
   address of R as its target (RFC4862-DAD-1, LL-1). This confirms the stimulus.
2. On L1, from R, a Neighbor Solicitation from the unspecified address with the global address of
   R on L1 as its target (RFC4862-DAD-1).
3. The first Router Advertisement of R leaves at least 1 second after the solicitation of
   observation 1 (RFC4862-DAD-1).

## Global address and its Duplicate Address Detection

Checks: **RFC4862-GLOB-1**, **GLOB-8**, **GLOB-11**, **RFC4861-OPT-21** (description),
**RFC4862-DAD-5** (should, should not, must not), **GLOB-2** (should, must).

### Requirement

RFC 4862 §5.5.3: for a new prefix with the Autonomous flag set and a non-zero Valid Lifetime, a
node forms a global address from the prefix and its interface identifier, and adds it with the
lifetimes of the option; the creation of global addresses is on by default. §5.4: the node tests
the uniqueness of the address first, also when its interface identifier is the one of the
link-local address.

### Scenario constants

- The link. At 10 seconds, host A sends one ICMPv6 echo request to the global address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A send the echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with a Prefix Information option for 2001:db8:1::/64
   with the A flag set. This confirms the stimulus.
2. On L1, from A, a Neighbor Solicitation from the unspecified address with target 2001:db8:1::
   plus the interface identifier of A (RFC4862-GLOB-8, DAD-5).
3. On L1, from A, the echo request to B, with 2001:db8:1:: plus the interface identifier of A as
   its IPv6 source (RFC4862-GLOB-1, GLOB-2, GLOB-11, RFC4861-OPT-21).

## Duplicate link-local address

Checks: **RFC4862-DAD-25** (description), **DAD-27** (must not), **DAD-29** (should),
**RFC4861-AR-38** (must), **NA-14** (must not); covers **RFC4862-DAD-7** (must, lower case).

### Requirement

RFC 4861 §7.2.4: a node answers a Neighbor Solicitation from the unspecified address for one of
its addresses with a Neighbor Advertisement to the all-nodes address, with the Solicited flag
clear. RFC 4862 §5.4.4 and §5.4.5: a valid advertisement for a tentative address shows that the
address is a duplicate; the node does not assign the address, and when the address is a
link-local address formed from the hardware address, the node stops IP on that interface.

### Scenario constants

- The link with a duplicate. Host B comes up at 10 seconds, when A has long assigned the
  link-local address that B also forms.
- Observation lasts 30 seconds from the start.

### Procedure

1. Build the link with a duplicate, with B down until 10 seconds.
2. Observe L1.

### Expected observations

1. On L1, from B, after 10 seconds, a Neighbor Solicitation from the unspecified address with the
   shared link-local address as its target. This confirms the stimulus.
2. On L1, from A, a Neighbor Advertisement with that target to ff02::1 (RFC4861-AR-38).
3. On L1, from B, after that advertisement, no packet with the shared link-local address as its
   IPv6 source (RFC4862-DAD-25, DAD-27).
4. On L1, from B, after that advertisement, no IPv6 packet at all (RFC4862-DAD-29).
5. The Solicited flag of the advertisement of observation 2 is clear (RFC4861-AR-38, NA-14).

### Notes

- A and B share the link-layer address, so observations 2 to 4 tell the two hosts apart by the
  link on which the frame leaves, as "from A" and "from B" say, and never by the address.

## Prefix with the Autonomous flag clear

Checks: **RFC4862-GLOB-5** (description).

### Requirement

RFC 4862 §5.5.3: a node silently ignores a Prefix Information option with the Autonomous flag
clear for address autoconfiguration.

### Scenario constants

- The link. R sets AdvAutonomousFlag to FALSE for 2001:db8:1::/64, and keeps AdvOnLinkFlag TRUE.
- At 10 seconds, host A sends one ICMPv6 echo request to the global address of R on L1.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. At 10 seconds, let A send the echo request to R.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with a Prefix Information option for 2001:db8:1::/64
   with the A flag clear. This confirms the stimulus.
2. No Neighbor Solicitation from A from the unspecified address with a target in
   2001:db8:1::/64 (RFC4862-GLOB-5).
3. No packet from A with an IPv6 source in 2001:db8:1::/64 (RFC4862-GLOB-5).

## Address after its valid lifetime

Checks: **RFC4862-GLOB-22** (description), **GLOB-23** (must not).

### Requirement

RFC 4862 §5.5.4: an address becomes invalid when its valid lifetime expires, and a node never
uses an invalid address as the source of a packet.

### Scenario constants

- As in [Prefix after its valid lifetime](on-link.md#prefix-after-its-valid-lifetime): R
  advertises 2001:db8:1::/64 with Valid Lifetime 30 seconds and Preferred Lifetime 20 seconds and
  stops at 15 seconds; host A sends an echo request to the global address of B every second from
  10 to 80 seconds.

### Size or value arithmetic

The global address of A is invalid at 15 + 30 = 45 seconds at the latest. The check judges from
50 seconds on.

### Procedure

1. As in [Prefix after its valid lifetime](on-link.md#prefix-after-its-valid-lifetime).

### Expected observations

1. On L1, from A, before 15 seconds, an echo request with 2001:db8:1:: plus the interface
   identifier of A as its IPv6 source. This confirms the stimulus.
2. After 50 seconds, no packet from A with an IPv6 source in 2001:db8:1::/64 (RFC4862-GLOB-22,
   GLOB-23).
