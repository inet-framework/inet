# IPsec — English check procedures: sequence numbers

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4302/catalog.md](../../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../../standard/rfc4303/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Sequence numbers of an SA

Checks: **RFC4301-SADI-2**, **RFC4302-SEQ-2**, **RFC4303-SEQ-2** (must), **RFC4302-SEQ-1**,
**SEQ-8**, **OSEQ-1**, **OSEQ-2**, **RFC4303-SEQ-1**, **SEQ-7**, **OSEQ-1** (description); covers
**RFC4302-SEQ-6** (must), **OSEQ-9**, **RFC4303-OSEQ-2**, **OSEQ-10** (description), **SEQ-6**
(must (generation and verification capability); description (the receiver's discretion not to act
on it)).

### Requirement

RFC 4302 §2.5 and §3.3.2, RFC 4303 §2.2 and §3.3.3: the sender keeps a counter for each SA. The
counter is zero when the SA is created, and the sender increments it before each packet, so the
first packet of an SA carries Sequence Number 1, the next 2, and so on. Each SA counts on its
own. The sender does this also when the receiver does not check the numbers.

### Scenario constants

- The link, IPv4. Flows 2000 and 2001 run from A to B from 1 second; flow 2002 runs from
  3 seconds. Each flow sends five datagrams, one each second.
- ESP SA 101 carries flow 2000 and ESP SA 102 carries flow 2002. AH SA 103 carries flow 2001.
  The SPDs of A and B name the three SAs.

### Procedure

1. Build the link, and configure the SPDs and the three SAs.
2. Start the flows at A.
3. Observe the ESP and AH packets of A on L1, in order, for each SPI.

### Expected observations

1. On L1, from A to B, five ESP packets with SPI 101, five with SPI 102 and five AH packets with
   SPI 103. This confirms the stimulus.
2. On each SA, each packet carries the Sequence Number of the packet before it plus one
   (RFC4302-SEQ-1, SEQ-2, OSEQ-2, RFC4303-SEQ-1, SEQ-2).
3. The first packet of SA 102, which starts two seconds later, carries the same Sequence Number
   as the first packet of SA 101: each SA counts on its own (RFC4301-SADI-2).
4. The first packet of SA 101, of SA 102 and of SA 103 carries Sequence Number 1
   (RFC4302-SEQ-8, OSEQ-1, RFC4303-SEQ-7, OSEQ-1).
