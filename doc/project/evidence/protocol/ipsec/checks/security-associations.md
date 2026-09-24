# IPsec — English check procedures: security associations

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../../standard/rfc4303/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## One SA for each direction

Checks: **RFC4301-SA-1** (must), **SA-2**, **SA-5** (description); covers **RFC4301-SA-3**,
**SAD-1**, **RFC4303-SPI-1**, **SPI-3** (description), **RFC4301-SADI-1** (must).

### Requirement

RFC 4301 §4.1: an SA is simplex. Two hosts that protect traffic in both directions need a pair
of SAs, one for each direction, each with its own SPI.

### Scenario constants

- The link, IPv4. Flow 2000 runs from A to B, and B sends each datagram back to its sender, from
  port 2000 to port 1000 of A.
- The SPD of A: flow 2000 outbound PROTECT with ESP SA 101; its answers inbound PROTECT with
  ESP SA 201. The SPD of B: the same two entries, with the directions swapped.

### Procedure

1. Build the link, and configure the SPDs, SA 101 from A to B and SA 201 from B to A.
2. Start flow 2000 at A.
3. Observe the packets of A and of B on L1, and count the answers that A delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101. This confirms the stimulus.
2. On L1, from B to A, ESP packets with SPI 201, and none with SPI 101: the answers use the SA
   of their own direction (RFC4301-SA-2, SA-5).
3. A delivers the five answers on port 1000 (RFC4301-SA-1).

## Inbound SA lookup

Checks: **RFC4301-SA-6**, **SA-18**, **SAD-3**, **IN-14**, **IN-15**, **IN-17**,
**RFC4302-SPI-2**, **SPI-15**, **ISA-1**, **ISA-2**, **RFC4303-SPI-13**, **ISA-1**, **ISA-2**
(description), **RFC4301-SA-13**, **RFC4302-SPI-10**, **RFC4303-SPI-10** (must (lower case)),
**RFC4302-SPI-5**, **ISA-6**, **RFC4303-SPI-4**, **ISA-6** (must); covers **RFC4301-SA-7**,
**RFC4302-SPI-3** (may (lower case)), **RFC4301-SA-14**, **SA-15**, **SA-16**, **SA-17**,
**IN-6**, **IN-8**, **IN-20**, **RFC4302-SPI-1**, **SPI-11**, **SPI-12**, **SPI-13**, **SPI-14**,
**ISA-4**, **ISA-5**, **RFC4303-SPI-11**, **SPI-12**, **ISA-5** (description), **RFC4301-SA-20**
(may (the search method), must (the externally visible behavior)), **RFC4302-SPI-4** (must (lower
case)), **SPI-16** (may (the choice of method); must (the equivalent behavior)),
**RFC4303-SPI-14** (may; must), **ISA-4** (should (lower case)).

### Requirement

RFC 4301 §4.1 and §5.2, RFC 4302 §2.4 and §3.4.2, RFC 4303 §2.1 and §3.4.2: the receiver of an
AH or ESP packet finds its SA in the SAD by the SPI, or by the SPI and the protocol. A packet for
which no valid SA exists is discarded.

### Scenario constants

- The link, IPv4. Five flows run from A to B:
  - flow 2000 on ESP SA 101 and flow 2001 on ESP SA 102;
  - flow 2002 on AH SA 103;
  - flow 2008 on ESP SA 104, and flow 2009 on AH SA 105;
  - flow 2010, from port 1010 of A to port 2000 of B, on an AH SA with SPI 101.
- B has the inbound SAs 101 and 102 (ESP, for local port 2000 and 2001) and 103 (AH, for local
  port 2002), each with an inbound PROTECT entry. B has no SA 104 and no SA 105, and no AH SA
  with SPI 101.

### Procedure

1. Build the link, and configure the SPDs and the SAs at A and at B.
2. Start the flows at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers on each port.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101, 102 and 104, and AH packets with SPI 103, 105
   and 101. This confirms the stimulus.
2. B delivers the datagrams of flows 2000 and 2001, each on its own port: the SPI selects the SA
   (RFC4301-SA-6, SA-13, SAD-3, IN-14, IN-15, RFC4303-SPI-4, SPI-10, ISA-1, ISA-2).
3. B delivers the datagrams of flow 2002 (RFC4302-SPI-2, SPI-5, SPI-10, ISA-1, ISA-2).
4. B delivers none of flow 2008: no SA 104 exists (RFC4301-SA-18, IN-17, RFC4303-SPI-13,
   ISA-6).
5. B delivers none of flow 2009: no SA 105 exists (RFC4302-SPI-15, ISA-6).
6. B delivers only the five datagrams of flow 2000 on port 2000, and none of flow 2010: the only
   SA with SPI 101 is an ESP SA, and it is not valid for an AH packet (RFC4302-ISA-6).

## Inbound selector check

Checks: **RFC4301-IN-31**, **IN-32**, **IN-33** (must), **SAD-6** (description); covers
**RFC4301-SAD-5** (must), **IN-39** (description).

### Requirement

RFC 4301 §4.4.2 and §5.2, step 4: after AH or ESP processing, the receiver matches the packet
against the selectors of the SA it arrived on. A packet whose headers do not agree with those
selectors is discarded.

### Scenario constants

- The link, IPv4. Flows 2000 and 3000 run from A to B.
- The SPD of A: UDP to remote ports 2000 and 3000, PROTECT with ESP SA 101. A therefore sends
  both flows on SA 101.
- The SPD of B: UDP to local port 2000, inbound PROTECT with ESP SA 101, so the selectors of
  SA 101 at B hold local port 2000 only. B has an application on port 3000 too.

### Procedure

1. Build the link, and configure the SPDs and SA 101.
2. Start the two flows at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers on each port.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 for both flows: ten packets in all. This
   confirms the stimulus.
2. B delivers the five datagrams of flow 2000 (RFC4301-IN-31).
3. B delivers none of flow 3000: port 3000 does not agree with the selectors of SA 101
   (RFC4301-IN-32, IN-33, SAD-6).

## A PROTECT entry without an SA

Checks: **RFC4301-OUT-10** (description); covers **RFC4301-OUT-8**, **OUT-9** (description).

### Requirement

RFC 4301 §5.1: a packet that matches a PROTECT entry leaves only on an SA. When no SA exists and
none can be created, the packet is discarded. A host without key management cannot create one.

### Scenario constants

- The link, IPv4. Flows 2000 and 2001 run from A to B.
- The SPD of A: flow 2000 PROTECT with ESP, but no SA exists for it; flow 2001 BYPASS. The SPD of
  B: flows 2000 and 2001 inbound BYPASS, so that B would deliver flow 2000 if it came in clear.

### Procedure

1. Build the link, and configure the SPDs. Configure no SA.
2. Start the two flows at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, plain UDP datagrams of flow 2001. This confirms that A sends.
2. On L1, no packet of flow 2000, protected or in clear (RFC4301-OUT-10).
3. B delivers none of flow 2000 (RFC4301-OUT-10).

## The lifetime of an SA

Checks: **RFC4301-SADI-10** (must (lower case)), **SADI-11** (must); covers **RFC4301-SADI-14**,
**SADI-15** (should).

### Requirement

RFC 4301 §4.4.2.1: each SA has a lifetime, a time, a byte count, or both, after which the SA is
replaced or ends. An implementation supports both kinds. Without key management no replacement
comes, so the SA ends, and a packet for it is discarded.

### Scenario constants

- The link, IPv4. Flows 2000 and 2001 run from A to B, one datagram each second from 1 to 9
  seconds. Observation lasts 12 seconds.
- ESP SA 101 carries flow 2000 and has a hard lifetime of 5 seconds. ESP SA 102 carries flow
  2001 and has a hard lifetime of 500 octets, the bytes of four datagrams of flow 2001.
- The SPDs of A and B: flow 2000 PROTECT with SA 101, flow 2001 PROTECT with SA 102.

### Procedure

1. Build the link, and configure the SPDs and the two SAs with their lifetimes.
2. Start the two flows at A.
3. Observe the packets of A on L1.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 and 102 in the first seconds. This confirms the
   stimulus.
2. On L1, no ESP packet with SPI 101 after 6 seconds, and no plain datagram of flow 2000:
   the time lifetime of SA 101 has ended (RFC4301-SADI-10, SADI-11).
3. On L1, at most four ESP packets with SPI 102, and no plain datagram of flow 2001: the byte
   lifetime of SA 102 has ended (RFC4301-SADI-10, SADI-11).

## Parallel SAs for classes of traffic

Checks: **RFC4301-SA-25**, **SA-26** (must); covers **RFC4301-SA-24** (should), **SA-28** (must
not), **SADI-19**, **SADI-20** (description).

### Requirement

RFC 4301 §4.1 and §4.4.2.1: a sender can keep more than one SA with the same selectors, for
example one for each class of traffic, and chooses among them by the DSCP values of each SA.
The receiver processes the packets of every such SA the same way.

### Scenario constants

- The link, IPv4. Two applications of A send to port 2000 of B: one with DSCP 0, from port 1000,
  and one with DSCP 46, from port 1001.
- The SPD of A: UDP to remote port 2000, PROTECT with ESP SA 101 for DSCP 0 and ESP SA 102 for
  DSCP 46. The SPD of B: UDP to local port 2000, inbound PROTECT with SA 101 and SA 102.

### Procedure

1. Build the link, and configure the SPDs and the two SAs.
2. Start the two applications at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with DSCP 0 and with DSCP 46. This confirms the stimulus.
2. Every packet with DSCP 46 carries SPI 102, and every packet with DSCP 0 carries SPI 101
   (RFC4301-SA-25).
3. B delivers the ten datagrams (RFC4301-SA-26).
