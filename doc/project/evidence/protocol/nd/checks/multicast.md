# ND — English check procedures: multicast groups

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md), [rfc4862/catalog.md](../../../standard/rfc4862/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

A node joins a group on a link with Multicast Listener Discovery: an MLD version 1 Report
(ICMPv6 type 131) or an MLD version 2 Report (type 143) that names the group. The report is the
wire trace of the join. MLD never reports the all-nodes address, so a join of ff02::1 leaves no
trace, and the checks below do not judge it. In the checks below, every node runs MLD, as RFC 4861
§7.2.1 assumes.

## Groups joined before Duplicate Address Detection

Checks: **RFC4862-DAD-11**, **RFC4861-AR-4** (must), **RFC4861-AR-6** (description); covers
**RFC4862-DAD-16** (description).

### Requirement

RFC 4862 §5.4.2: before it sends a Neighbor Solicitation for a tentative address, the interface
joins the solicited-node multicast address of that address. RFC 4861 §7.2.1: when an interface
becomes enabled, the node joins the solicited-node multicast address of each of its addresses,
with MLD.

### Scenario constants

- The link, with the default of every variable. Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the MLD Reports and the Neighbor Solicitations of A on L1.

### Expected observations

1. On L1, from A, a Neighbor Solicitation from the unspecified address with the link-local
   address of A as its target. This confirms the stimulus.
2. On L1, from A, before that solicitation, an MLD Report that names the solicited-node multicast
   address of that target (RFC4862-DAD-11, RFC4861-AR-4, AR-6).

### Notes

- RFC 4862 §5.4.2 lets the node delay the report by a random time (RFC4862-DAD-14, DAD-16); the
  check asks only that the report leaves before the solicitation.

## All-routers group of a router

Checks: **RFC4861-ADV-3** (must).

### Requirement

RFC 4861 §6.2.2: a router joins the all-routers multicast address, ff02::2, on each advertising
interface.

### Scenario constants

- The link, with the default of every variable. Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the MLD Reports and the Router Advertisements of R on L1.

### Expected observations

1. On L1, from R, a Router Advertisement. This confirms the stimulus: the interface advertises.
2. On L1, from R, an MLD Report that names ff02::2 (RFC4861-ADV-3).
