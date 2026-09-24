# IPsec — English check procedures: the Security Policy Database

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Outbound dispositions

Checks: **RFC4301-PROC-1**, **PROC-2**, **SPD-19**, **SPD-20**, **OUT-3** (must), **SPD-8**,
**SPD-9**, **SPD-10** (description), **SPD-11** (must (lower case)); covers **RFC4301-SPD-2**,
**OUT-1**, **OUT-2**, **OUT-6**, **OUT-13**, **SPD-32** (must), **SPD-7** (must (lower case)),
**SPD-12**, **SPD-16**, **SPDE-13**, **SPDE-21**, **OVW-1**, **OUT-7** (description), **SPD-14**
(must, should (lower case)), **PROC-3** (should (lower case)).

### Requirement

RFC 4301 §4.4.1 and §5.1: an outbound packet gets one of three dispositions from the SPD.
PROTECT applies the SA that the entry names, BYPASS lets the packet leave without IPsec, and
DISCARD drops it. A packet that no entry matches is discarded.

### Scenario constants

- The link, IPv4. Flows 2000, 2001, 2002 and 2003 run from A to B.
- The SPD of A: flow 2000 PROTECT with ESP SA 101; flow 2001 BYPASS; flow 2002 DISCARD. No
  entry names flow 2003, so only the final entry matches it.
- The SPD of B: flow 2000 inbound PROTECT with SA 101; flow 2001 inbound BYPASS.

### Procedure

1. Build the link, and configure the SPDs and SA 101 at A and at B.
2. Start the four flows at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers on each port.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101: flow 2000 leaves protected (RFC4301-SPD-11,
   OUT-3). This confirms the stimulus.
2. On L1, from A to B, plain UDP datagrams to port 2001: flow 2001 leaves without IPsec
   (RFC4301-SPD-10, SPD-19).
3. On L1, no packet of flow 2002, protected or not (RFC4301-SPD-9).
4. On L1, no packet of flow 2003, protected or not: a packet that no entry matches is discarded
   (RFC4301-PROC-2).
5. B delivers the five datagrams of flow 2000 and the five of flow 2001, and none of flows 2002
   and 2003 (RFC4301-PROC-1, SPD-8, SPD-20).

## Inbound dispositions

Checks: **RFC4301-IN-4**, **IN-13**, **IN-22**, **IN-25** (description), **SPD-13**, **SPD-18**
(must); covers **RFC4301-IN-1**, **IN-2**, **IN-10**, **IN-21**, **IN-23**, **IN-24**
(description), **IN-3** (should).

### Requirement

RFC 4301 §4.4.1 and §5.2: an inbound packet that is not IPsec-protected is matched against
SPD-I, which holds only BYPASS and DISCARD entries. A BYPASS entry delivers the packet, a
DISCARD entry drops it, and a packet that no entry of SPD-I matches is discarded. Traffic that
a PROTECT entry covers can arrive only on its SA, so the plain form of that traffic is
discarded too.

### Scenario constants

- The link, IPv4. Flows 2000, 2001, 2004 and 2005 run from A to B.
- The SPD of A: flows 2000, 2001, 2004 and 2005 BYPASS, so all four leave without IPsec.
- The SPD of B: flow 2000 inbound PROTECT with ESP SA 101; flow 2001 inbound BYPASS; flow 2004
  inbound DISCARD. No entry names flow 2005.

### Procedure

1. Build the link, and configure the SPDs, and SA 101 at B.
2. Start the four flows at A.
3. Observe the packets of A on L1, and count the datagrams that B delivers on each port.

### Expected observations

1. On L1, from A to B, plain UDP datagrams of the four flows. This confirms the stimulus.
2. B delivers the five datagrams of flow 2001: a BYPASS entry of SPD-I lets them in
   (RFC4301-IN-13, IN-22, SPD-18).
3. B delivers none of flow 2004: a DISCARD entry drops them (RFC4301-IN-22).
4. B delivers none of flow 2005: a packet that no entry of SPD-I matches is discarded
   (RFC4301-IN-4, IN-25, SPD-13).
5. B delivers none of flow 2000: its traffic must arrive on SA 101, and its plain form matches
   no entry of SPD-I (RFC4301-IN-25).

## The first matching entry decides

Checks: **RFC4301-SPD-6**, **SPD-35**, **SPD-42** (must); covers **RFC4301-SPD-15**
(description).

### Requirement

RFC 4301 §4.4.1: the entries of the SPD are ordered, the administrator sets the order, and
an ordered search takes the first entry that matches. Where two entries match one packet, the
earlier one decides.

### Scenario constants

- The link, IPv4. Flows 2006 and 2007 run from A to B.
- The SPD of A, in this order: remote port 2006, BYPASS; remote ports 2006 to 2007, DISCARD;
  remote port 2007, BYPASS.
- The SPD of B: remote ports 2006 to 2007, inbound BYPASS.

### Procedure

1. Build the link, and configure the SPDs.
2. Start the two flows at A.
3. Observe the packets of A on L1.

### Expected observations

1. On L1, from A to B, plain UDP datagrams to port 2006: the first entry matches before the
   second (RFC4301-SPD-6, SPD-42). This confirms the stimulus.
2. On L1, no packet of flow 2007: the second entry matches before the third
   (RFC4301-SPD-35, SPD-42).

## Selectors

Checks: **RFC4301-SEL-1**, **SEL-20**, **SPD-34**, **SPD-38** (must), **SEL-4**, **SEL-7**,
**SEL-12**, **SEL-18**, **SEL-19**, **SPD-26**, **SPDE-11** (description), **SPD-17** (should);
covers **RFC4301-SEL-2** (should (lower case)), **SEL-5**, **SEL-8**, **SPD-22**, **SPD-23**,
**SPDE-3**, **SPDE-12**, **NLP-2** (description), **SPDE-4** (may, must (lower case)).

### Requirement

RFC 4301 §4.4.1.1 and §4.4.1.2: an SPD entry selects traffic by a list of remote address
ranges, a list of local address ranges, the next layer protocol, ranges of local and remote
ports, and, for ICMP, a range of message types and a range of codes. A selector that the entry
does not name is ANY. Every implementation supports all these selectors.

### Scenario constants

- The link, IPv4.
- The SPD of A, in this order:
  1. remote address 10.0.1.11 to 10.0.1.11 (B only), local address 10.0.1.10, protocol UDP,
     remote ports 2000 to 2009: PROTECT with ESP SA 101;
  2. protocol UDP, local port 1010: BYPASS;
  3. protocol ICMP, type 8 to 8 (Echo Request), code 0 to 0: BYPASS;
  4. the final entry, which discards everything else.
- The traffic of A: flow 2005 (UDP from port 1000 to B port 2005); UDP from port 1010 to B port
  3000; UDP from port 1011 to B port 3001; UDP from port 1000 to C port 2005; one ICMP Echo
  Request to B each second.
- The SPD of B: flow 2005 inbound PROTECT with SA 101; UDP to ports 3000 and 3001 inbound
  BYPASS; ICMP inbound BYPASS and outbound BYPASS, so that B answers the Echo Requests. The SPD
  of C: everything inbound BYPASS.
- The SPD of A also has, before its final entry, ICMP type 0 (Echo Reply) inbound BYPASS.

### Procedure

1. Build the link with A, B and C, and configure the SPDs and SA 101.
2. Start the traffic at A.
3. Observe the packets of A on L1, and count the datagrams that B and C deliver.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 that carry flow 2005: the address, protocol and
   port selectors of entry 1 match (RFC4301-SEL-4, SEL-7, SEL-12, SPDE-11). This confirms the
   stimulus.
2. On L1, from A to B, plain UDP datagrams from port 1010 to port 3000: entry 2 matches on its
   local port alone, and its other selectors are ANY (RFC4301-SEL-12, SPD-26).
3. On L1, no packet from port 1011: no entry matches it, and the final entry discards it
   (RFC4301-SPD-17).
4. On L1, no packet from A to C: the remote address range of entry 1 does not hold C, and the
   final entry discards the datagram (RFC4301-SEL-4, SPD-17).
5. On L1, from A to B, plain ICMP Echo Requests: entry 3 matches on type 8 and code 0
   (RFC4301-SEL-18, SEL-19, SEL-20).
6. B delivers the datagrams of flow 2005 and those to port 3000, and C delivers none
   (RFC4301-SEL-1, SPD-34, SPD-38).
