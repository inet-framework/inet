# RIP — English check procedures: the contents of a response

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## More than 25 routes (RIP version 2)

Checks: **RFC2453-GEN-3**, **MSG-7** (description), **RFC2453-TABLE-2** (must).

### Requirement

RFC 2453 §3.6 and §3.10.2: a response holds between 1 and 25 entries; when there are more
routes, the router sends the current response and starts a new one, and there is no limit to
the number of messages. §3.5: each route holds a subnet mask, so two routes with the same
network number and different masks are two routes.

### Scenario constants

- The pair with many routes. The marker of R1 is netA.
- The table of R1 holds netA, L1, netB (learned from R2), the 30 routes `10.100.k.0/24`, and
  the two routes `10.200.0.0/16` and `10.200.0.0/24`: 35 routes.
- A periodic update of R1 is the set of messages that R1 sends to the multicast group on L1
  within 1 second.
- Observation starts 60 seconds after the start and lasts 40 seconds.

### Size or value arithmetic

35 routes, at most 25 in one message, need at least two messages. On L1, split horizon may
leave out netB, so the update holds 34 or 35 entries: still at least two messages.

### Procedure

1. Build the pair with many routes, and let both routers start.
2. From 60 seconds on, observe the next periodic update of R1 on L1, and collect every
   message of it.

### Expected observations

1. On L1, from R1, after 60 seconds, a message to the multicast group. Record its instant: the
   periodic update starts. This confirms the stimulus.
2. Every message of R1 on L1 to the multicast group within 1 second of observation 1 holds
   between 1 and 25 entries (RFC2453-MSG-7, GEN-3).
3. There are at least two such messages (RFC2453-GEN-3: a new message for the routes after the
   25th).
4. Together, those messages hold netA and every one of the 30 routes `10.100.k.0/24`, each
   with subnet mask 255.255.255.0 (RFC2453-GEN-3: no route is lost at the split).
5. Together, they hold two entries for `10.200.0.0`, one with subnet mask 255.255.0.0 and one
   with 255.255.255.0 (RFC2453-TABLE-2).

### Notes

- The 1 second that bounds the periodic update is not a figure of the standard. The messages
  of one update leave together, and the next update is 25 seconds away at least, so any
  bound between the two separates them.

## Messages limited by the MTU (RIPng)

Checks: **RFC2080-GEN-4**, **RFC2080-MSG-11** (description).

### Requirement

RFC 2080 §2.1 and §2.5.2: the MTU of the medium limits the size of a message; the number of
entries in one message follows from the MTU, the size of the headers and the size of an
entry. When a message has no more space, the router sends it and starts a new one.

### Scenario constants

- The pair with many routes, on IPv6. The marker of R1 is netA.
- The table of R1 holds netA, L1, netB, and the 80 prefixes `2001:db8:100:k::/64`: 83 routes.
- The MTU of every link is 1500 octets.
- Observation starts 60 seconds after the start and lasts 40 seconds.

### Size or value arithmetic

The formula of §2.1: (1500 − 40 octets of IPv6 header − 8 octets of UDP header − 4 octets of
RIPng header) / 20 octets for each entry = 72.4, so at most 72 entries in one message. 83
routes, or 82 when split horizon leaves out netB, need at least two messages.

### Procedure

1. Build the pair with many routes on IPv6, and let both routers start.
2. From 60 seconds on, observe the next periodic update of R1 on L1, and collect every
   message of it.

### Expected observations

1. On L1, from R1, after 60 seconds, a message to FF02::9. Record its instant: the periodic
   update starts. This confirms the stimulus.
2. Every message of R1 on L1 to FF02::9 within 1 second of observation 1 holds at most 72
   entries (RFC2080-MSG-11, GEN-4).
3. Together, those messages hold netA and every one of the 80 prefixes, each with prefix
   length 64 (RFC2080-GEN-4: no route is lost at the split).

### Notes

- A router that splits at fewer entries than the MTU allows — at 25, like RIP version 2 —
  passes the check. RFC 2080 limits the size of a message; it does not ask for full messages.
