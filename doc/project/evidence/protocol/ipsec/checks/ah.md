# IPsec — English check procedures: the Authentication Header

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../../standard/rfc4302/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## AH format in transport mode

Checks: **RFC4302-FMT-1**, **RSV-2**, **SEQ-5**, **SEQ-7**, **ICV-4** (must), **FMT-2**,
**NH-2**, **LEN-1**, **ICV-1**, **LOC-2**, **LOC-3**, **LOC-7**, **OSA-1**, **IICV-2**
(description), **LEN-2**, **ICV-2** (must (lower case)), **LOC-5** (should (lower case)); covers
**RFC4302-FMT-3**, **NH-1**, **RSV-1**, **OICV-25**, **OICV-26**, **OICV-27**, **RFC4301-SA-39**
(description), **RFC4302-ICV-3**, **LOC-1** (may (lower case)), **OICV-28** (must not (lower
case)), **CONF-1**, **CONF-2** (must).

### Requirement

RFC 4302 §2 and §3.1.1: in transport mode, the AH header follows the IP header, and the IP
header names protocol 51. The AH header holds, in order, the Next Header, the Payload Length,
a zero Reserved field, the SPI, the Sequence Number and the ICV. The Payload Length is the
length of the AH header in 32-bit words, minus 2. The ICV field is a multiple of 32 bits; in
IPv6 it is padded so that the whole AH header is a multiple of 8 octets, with no more padding
than that. AH does not encrypt: the next layer header follows the AH header as it is.

### Scenario constants

- The link, once on IPv4 and once on IPv6. Flow 2003 runs from A to B on AH SA 103 with
  HMAC-SHA-256-128, a 16-octet ICV.
- The arithmetic. The fixed part of the AH header is 12 octets. In IPv4, 12 + 16 = 28 octets,
  seven 32-bit words: the Payload Length is 5, and the ICV field is 16 octets. In IPv6, 28 is not
  a multiple of 8, so the ICV field has 4 octets of padding: 32 octets, eight words, a Payload
  Length of 6.
- The SPDs of A and B name SA 103 for flow 2003.

### Procedure

1. Build the link, and configure the SPDs and SA 103.
2. Start flow 2003 at A.
3. Observe the packets of A on L1, and the datagrams that B delivers on port 2003.

### Expected observations

1. On L1, from A to B, packets whose IP header names protocol 51 (RFC4302-FMT-1). This confirms
   the stimulus.
2. B delivers the five datagrams, each with the 100 octets of payload that A sent
   (RFC4302-IICV-2).
3. The AH header follows the IP header directly, and the UDP header of the flow follows the AH
   header, not encrypted (RFC4302-LOC-2, LOC-3, LOC-5, LOC-7, OSA-1).
4. The AH header holds Next Header 17, UDP, a zero Reserved field, SPI 103 and a Sequence Number
   (RFC4302-NH-2, RSV-2, SEQ-5, SEQ-7).
5. The ICV field follows the Sequence Number, inside the AH header: 16 octets in IPv4, 20 in
   IPv6 (RFC4302-FMT-2, ICV-1, ICV-2, ICV-4, LEN-2).
6. The Payload Length is 5 in IPv4 and 6 in IPv6 (RFC4302-LEN-1).

## AH across a router

Checks: **RFC4302-OICV-1**, **OICV-13**, **OICV-19** (description), **OICV-5** (may (lower
case)); covers **RFC4302-OICV-2**, **OICV-3**, **OICV-6**, **OICV-7**, **OICV-17**, **IICV-1**,
**IICV-5**, **IICV-9**, **ALG-1** (description), **OICV-11** (should (lower case)), **IICV-6**
(may (lower case)).

### Requirement

RFC 4302 §3.3.3.1: the AH ICV covers the fields of the IP header that do not change in transit.
A field that a router changes — the IPv4 TTL and Header Checksum, the IPv6 Hop Limit — counts as
zero in the ICV. A packet that crosses a router is therefore still valid at the receiver.

### Scenario constants

- The path, once on IPv4 and once on IPv6. Flow 2003 runs from A to B on AH SA 103 with the
  default algorithm. R decrements the TTL or the Hop Limit of each packet by one.
- The SPDs of A and B name SA 103 for flow 2003.

### Procedure

1. Build the path, and configure the SPDs and SA 103.
2. Start flow 2003 at A.
3. Observe the AH packets of A on L1 and of R on L2, and the datagrams that B delivers.

### Expected observations

1. On L1, from A, AH packets with SPI 103, and on L2, from R, the same packets with a TTL or Hop
   Limit that is one lower. This confirms the stimulus.
2. B delivers the five datagrams: the ICV does not cover the fields that R changed
   (RFC4302-OICV-1, OICV-5, OICV-13, OICV-19).
