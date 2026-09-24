# IPsec — English check procedures: the Encapsulating Security Payload

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md), [rfc4303/catalog.md](../../../standard/rfc4303/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## ESP packet format in transport mode

Checks: **RFC4303-FMT-1**, **SEQ-5**, **PAY-4**, **PAY-5**, **CONF-1**, **CONF-2**,
**RFC4301-SA-37**, **OVW-2**, **CONF-1**, **CONF-2** (must), **RFC4303-FMT-2**, **FMT-3**,
**SPI-2**, **PAY-2**, **PADL-1**, **PADL-3**, **NH-1**, **ICV-3**, **ICV-4**, **LOC-2**,
**LOC-3**, **OSA-1**, **OENC-1**, **IICV-2**, **IICV-8**, **IICV-14**, **RFC4301-SA-34**,
**SA-35** (description), **RFC4303-LOC-6** (should (lower case)), **RFC4301-OVW-5** (must (ESP),
may (AH)); covers **RFC4303-PAY-1**, **PAY-3**, **FMT-7**, **FMT-13**, **FMT-14**, **FMT-15**,
**FMT-16**, **OENC-4**, **OENC-9**, **IICV-1**, **RFC4301-SA-38**, **SA-29**, **SA-30**
(description), **SA-50** (must), **DB-1** (description (internal structure), must (external
behavior)).

### Requirement

RFC 4303 §2 and §3.1.1, RFC 4301 §4.1: in transport mode, the ESP header follows the IP header
directly, and the IP header names protocol 50. The ESP packet holds, in order, the SPI, the
Sequence Number, the Payload Data with the IV of the cipher, the Padding, the Pad Length, the
Next Header and the ICV. The next layer header starts at an offset from the start of the ESP
header that is a multiple of 4 octets in IPv4 and of 8 octets in IPv6. The receiver decrypts
the payload and rebuilds the original datagram.

### Scenario constants

- The link, once on IPv4 and once on IPv6. Flow 2000 runs from A to B on ESP SA 101, with the
  default algorithms: a 16-octet IV and a 12-octet ICV.
- The SPDs of A and B name SA 101 for flow 2000.

### Procedure

1. Build the link, and configure the SPDs and SA 101.
2. Start flow 2000 at A.
3. Observe the packets of A on L1, and the datagrams that B delivers on port 2000.

### Expected observations

1. On L1, from A to B, packets whose IP header names protocol 50: the IPv4 Protocol field, or the
   IPv6 Next Header field of the base header (RFC4303-FMT-1). This confirms the stimulus.
2. B delivers the five datagrams, each with the 100 octets of payload that A sent
   (RFC4303-IICV-2, IICV-8, IICV-14, RFC4301-OVW-2, OVW-5, CONF-1, CONF-2, RFC4303-CONF-1,
   CONF-2).
3. The ESP header follows the IP header directly, and the UDP header of the flow comes after it,
   inside the ESP payload (RFC4303-LOC-2, LOC-3, LOC-6, OSA-1, OENC-1, RFC4301-SA-34, SA-35,
   SA-37).
4. The ESP header holds SPI 101 and a Sequence Number (RFC4303-SPI-2, SEQ-5).
5. The Payload Data begins with the 16-octet IV, so the UDP header starts 24 octets after the
   start of the ESP header, a multiple of 4 and of 8 (RFC4303-PAY-2, PAY-4, PAY-5).
6. The trailer, read with the keys of SA 101, holds a Pad Length that counts the padding octets
   before it, and Next Header 17, UDP (RFC4303-PADL-1, PADL-3, NH-1).
7. A 12-octet ICV closes the packet, after the trailer (RFC4303-FMT-2, FMT-3, ICV-3, ICV-4).

## ESP padding

Checks: **RFC4303-PAD-1**, **PAD-6**, **PAD-8** (description), **PAD-2** (must (lower case)),
**PAD-5**, **PAD-9** (must); covers **RFC4303-PAD-4** (may), **OENC-3**, **IICV-11**,
**RFC4301-FUNC-8** (description).

### Requirement

RFC 4303 §2.4: the sender pads the plaintext so that the Payload Data without the IV, the
Padding, the Pad Length and the Next Header fill a whole number of cipher blocks, and so that the
Pad Length and the Next Header end on a 4-octet boundary. When the cipher does not name the
padding contents, the padding octets are 1, 2, 3 and so on. Every implementation generates and
consumes padding.

### Scenario constants

- The link, IPv4. Two flows run from A to B:
  - flow 2000 on ESP SA 101, with AES-CBC (a 16-octet block) and HMAC-SHA-1-96, and 90 octets
    of payload in each datagram;
  - flow 2001 on ESP SA 102, with NULL encryption and HMAC-SHA-1-96, so the plaintext is on the
    link as it is, and 91 octets of payload in each datagram.
- The arithmetic of SA 101. The UDP datagram is 98 octets, and the Pad Length and the Next
  Header add 2: 100 octets. The next multiple of the 16-octet block is 112, so the Pad Length is
  12.
- The arithmetic of SA 102. The UDP datagram is 99 octets, and the two trailer fields add 2:
  101 octets. NULL encryption has no block, so the 4-octet boundary decides: the next multiple
  of 4 is 104, the Pad Length is 3, and the padding octets are 1, 2 and 3.

### Procedure

1. Build the link, and configure the SPDs and the two SAs.
2. Start the two flows at A.
3. Observe the ESP packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 and 102. This confirms the stimulus.
2. B delivers the five datagrams of each flow, with the length of payload that A sent: B
   consumes the padding (RFC4303-PAD-5).
3. On SA 101, the Pad Length is 12, and the Payload Data without the IV plus the padding and the
   two trailer fields is 112 octets, seven blocks (RFC4303-PAD-1, PAD-6).
4. On SA 102, the Pad Length is 3, and the Next Header field ends on a 4-octet boundary of the
   ESP packet (RFC4303-PAD-2, PAD-8).
5. On SA 102, the three padding octets are 1, 2 and 3 (RFC4303-PAD-9).

## ESP services

Checks: **RFC4303-ALG-5** (may), **ALG-6** (may (the integrity service being optional); may
(lower case, the algorithm being NULL)), **CONF-8** (description), **CONF-10** (must); covers
**RFC4303-ALG-3**, **ALG-7**, **ALG-9**, **CONF-9**, **FMT-5**, **ICV-1**, **RFC4301-OVW-6**,
**OVW-8** (description), **RFC4303-FMT-6**, **FMT-8** (may (lower case)), **RFC4301-OVW-9**
(should not).

### Requirement

RFC 4303 §3.2 and §5, RFC 4301 §3.2: an ESP SA gives confidentiality, integrity, or both. Its
encryption algorithm can be NULL, and so can its integrity algorithm, but not both. An
implementation supports NULL encryption; one that offers confidentiality without integrity
also supports NULL integrity. A combined algorithm gives both services at once.

### Scenario constants

- The link, IPv4. Three flows run from A to B:
  - flow 2000 on ESP SA 101: AES-CBC and NULL integrity, confidentiality only;
  - flow 2001 on ESP SA 102: NULL encryption and HMAC-SHA-1-96, integrity only;
  - flow 2002 on ESP SA 103: AES-GCM with a 16-octet ICV, a combined algorithm.
- The SPDs of A and B name the three SAs.

### Procedure

1. Build the link, and configure the SPDs and the three SAs.
2. Start the three flows at A.
3. Observe the ESP packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101, 102 and 103. This confirms the stimulus.
2. B delivers the five datagrams of each flow (RFC4303-ALG-5, ALG-6, CONF-8, CONF-10).
3. The packets of SA 101 end with the trailer and have no ICV (RFC4303-ALG-6).
4. The packets of SA 102 carry the UDP datagram without an IV, and end with a 12-octet ICV
   (RFC4303-ALG-5, CONF-8).
5. The packets of SA 103 end with a 16-octet ICV.

## An ESP SA without encryption and without integrity

Checks: **RFC4303-ALG-2** (must (selecting at least one service); must not (setting both
algorithms to NULL at the same time)), **CONF-11** (must not (both algorithms being NULL at the
same time); may (lower case, either one being NULL on its own)), **RFC4301-FUNC-9** (must not).

### Requirement

RFC 4301 §4.2 and RFC 4303 §3.2 and §5: an ESP SA never has NULL encryption and NULL integrity
at the same time. A compliant implementation does not let such an SA exist.

### Scenario constants

- The link, IPv4. Flows 2000 and 2001 run from A to B.
- The SPD of A: flow 2000 PROTECT with ESP SA 101, configured with NULL encryption and NULL
  integrity; flow 2001 BYPASS. The SPD of B: flow 2000 inbound PROTECT with SA 101, flow 2001
  inbound BYPASS.

### Procedure

1. Build the link, and configure the SPDs and SA 101.
2. Start the two flows at A.
3. Observe the packets of A on L1.

### Expected observations

1. On L1, from A to B, plain UDP datagrams of flow 2001. This confirms that A runs and sends.
2. On L1, no ESP packet with SPI 101, and no plain datagram of flow 2000: SA 101 does not exist
   (RFC4303-ALG-2, CONF-11, RFC4301-FUNC-9).

## Traffic flow confidentiality padding

Checks: **RFC4303-TFC-2** (should), **TFC-5** (must not); covers **RFC4303-TFC-3** (may (lower
case)), **TFC-4**, **TFC-6**, **IICV-17**, **FMT-11**, **PADL-2** (description).

### Requirement

RFC 4303 §2.7: a sender can add TFC padding after the Payload Data and before the Padding, when
the payload carries its own length, as a UDP datagram does. The sender does not change that
length field, the Pad Length does not count the TFC padding, and the receiver removes it.

### Scenario constants

- The link, IPv4. Flow 2000 runs from A to B, with 50 octets of payload in each datagram, on ESP
  SA 101 with NULL encryption and HMAC-SHA-1-96, so the plaintext is on the link as it is.
- A adds TFC padding of a random length from 0 to 100 octets to each packet of SA 101. The flow
  sends 20 datagrams, one each 0.2 seconds.

### Procedure

1. Build the link, and configure the SPDs and SA 101 with TFC padding at A.
2. Start flow 2000 at A.
3. Observe the ESP packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101. This confirms the stimulus.
2. B delivers the 20 datagrams, each with 50 octets of payload (RFC4303-TFC-2).
3. Some packets of SA 101 are longer than the shortest one: A adds TFC padding (RFC4303-TFC-2).
4. In every packet, the UDP Length field is 58, and the Pad Length is 3 or less
   (RFC4303-TFC-5).

## Dummy packets

Checks: **RFC4303-NH-2**, **NH-3**, **NH-4**, **NH-5** (must); covers **RFC4303-NH-6** (should).

### Requirement

RFC 4303 §2.6: a sender can send dummy packets on an SA, with Next Header 59, "no next header",
and every other field of the ESP header and trailer. The receiver discards a dummy packet and
reports no error.

### Scenario constants

- The link, IPv4. Flow 2000 runs from A to B on ESP SA 101 with NULL encryption and
  HMAC-SHA-1-96.
- A is configured to send dummy packets on SA 101, one each 0.5 seconds from 1 second.

### Procedure

1. Build the link, and configure the SPDs and SA 101 with dummy packets at A.
2. Start flow 2000 at A.
3. Observe the ESP packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 and Next Header 59, beside the packets of flow
   2000 (RFC4303-NH-2, NH-3).
2. Each dummy packet holds the SPI, the Sequence Number, the Pad Length, the Next Header and the
   ICV (RFC4303-NH-5).
3. B delivers the five datagrams of flow 2000 and nothing else, and sends no ICMP error to A
   (RFC4303-NH-4).
