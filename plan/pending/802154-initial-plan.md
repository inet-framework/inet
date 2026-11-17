# IEEE 802.15.4 in INET: Gap Analysis and Implementation Plan

## 1. Executive summary

INET does **not currently contain a standards-complete IEEE 802.15.4 implementation**. What it has is better characterized as:

> a generic unslotted CSMA/CA model with retransmissions and ACK-like behavior, configured with some IEEE 802.15.4 timing parameters, plus a simplified 2.4 GHz O-QPSK PHY and an old 802.15.4a-derived UWB-IR PHY.

This is consistent with INET's own documentation, which calls the implementation “basic” and says that `Ieee802154Mac` is a parameterized version of a generic CSMA/CA protocol with ACK support. ([INET Framework][1])

More importantly, the current source explicitly acknowledges that `Ieee802154MacHeader` is **not a real IEEE 802.15.4 MAC header**. The serializer uses a fixed data-frame format, has no proper short/EUI-64 addressing model, does not model PAN-ID compression, security, frame versions, beacon/command frames, etc. [Current Ieee802154MacHeader.msg](https://github.com/inet-framework/inet/blob/master/src/inet/linklayer/ieee802154/Ieee802154MacHeader.msg?utm_source=chatgpt.com)

Therefore I would **not extend the current monolithic FSM feature-by-feature**. The first task should be to establish a correct 802.15.4 packet model, service model, address model, and MAC architecture. New behavior can then be implemented incrementally.

The appropriate normative baseline today is **IEEE 802.15.4-2024**, which is the active revision and supersedes 802.15.4-2020. IEEE describes it as specifying the PHY and MAC for low-rate wireless networks, including multiple PHYs and precision-ranging modes. ([IEEE Standards Association][2])

The two already-published 2026 amendments should be treated separately:

* IEEE 802.15.4ae-2026 adds **Ascon cryptographic algorithms**. ([IEEE Standards Association][3])
* IEEE 802.15.4ac-2026 adds **privacy enhancements**, including randomized addresses and session-continuity mechanisms. ([IEEE Standards Association][4])

I recommend getting the 2024 base implementation sound before addressing either amendment.

---

# 2. What INET currently implements

The current implementation contains several useful pieces, but they cover only a small subset of the standard.

| Area                         | Current INET status                                                  |
| ---------------------------- | -------------------------------------------------------------------- |
| Non-beacon CSMA/CA           | Basic exponential backoff and CCA                                    |
| CCA                          | Checks whether INET's radio reception state is idle                  |
| Retransmissions              | Yes                                                                  |
| ACK-like behavior            | Yes, but not real standard ACK frame processing                      |
| Sequence number              | 8 bits on serialization, with internal duplicate tracking            |
| Broadcast                    | Yes                                                                  |
| PHY timing parameters        | Some 2.4 GHz values such as 320 µs backoff period, 192 µs turnaround |
| 2.4 GHz O-QPSK               | Simplified DSSS-OQPSK, 250 kbit/s model                              |
| Maximum PSDU/MTU assumptions | Based roughly on the classical 127-byte PSDU                         |
| PHY error model              | Generic modulation BER/SER calculation                               |
| UWB                          | Legacy 802.15.4a-derived UWB-IR implementation                       |
| Energy accounting            | Available through INET radio/energy infrastructure                   |

The narrowband model defaults to 2.45 GHz and 250 kbit/s. The INET documentation itself cautions that the narrowband radio is a partially parameterized generic radio whose parameters must be checked against the particular 802.15.4 PHY being simulated. ([INET Framework][1])

The MAC implementation is here: [Ieee802154Mac.cc](https://github.com/inet-framework/inet/blob/master/src/inet/linklayer/ieee802154/Ieee802154Mac.cc?utm_source=chatgpt.com). The narrowband configuration is here: [Ieee802154NarrowbandMac.ned](https://github.com/inet-framework/inet/blob/master/src/inet/linklayer/ieee802154/Ieee802154NarrowbandMac.ned?utm_source=chatgpt.com).

---

# 3. Problems in the existing subset that should be fixed before adding features

There are several correctness problems more fundamental than “missing features.”

## 3.1 The MAC frame format is not IEEE 802.15.4

`Ieee802154MacHeaderSerializer` hard-codes:

* Frame Type = Data
* destination addressing mode = extended
* source addressing mode = extended
* Destination PAN ID = `0xffff`

and the deserializer asserts that the entire Frame Control field is exactly `0xCC01`.

Consequently, a real beacon, ACK, MAC command, or differently addressed data frame cannot even be deserialized by the model.

[Current MAC serializer](https://github.com/inet-framework/inet/blob/master/src/inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.cc?utm_source=chatgpt.com)

## 3.2 Source PAN ID is being used as an EtherType

The serializer writes `networkProtocol` into the Source PAN ID field.

This is explicitly documented as a kludge in the code. IEEE 802.15.4 does not have an EtherType field in its MAC header.

This must disappear. Protocol discrimination belongs either:

* in the MAC payload, e.g. 6LoWPAN dispatch/headers; or
* in INET packet metadata when protocol context is known out-of-band.

It must not alter the IEEE 802.15.4 wire representation.

## 3.3 Addresses are the wrong size

INET uses its ordinary `MacAddress`, which is 48 bits, and serializes six bytes followed by two zero bytes.

IEEE 802.15.4 instead requires native handling of:

* no address;
* 16-bit short address;
* 64-bit extended address/EUI-64.

This is particularly important for a future standards-correct 6LoWPAN implementation.

## 3.4 Header size is internally inconsistent

`Ieee802154NarrowbandMac` assumes a 9-byte minimum MAC overhead/header, and outgoing chunks are assigned that length.

But the current serializer actually emits:

`2 FCF + 1 sequence + 2 destination PAN + 8 destination address + 2 source PAN + 8 source address = 23 bytes`.

So the abstract packet length used by simulation and the serialized representation can describe different packets.

This needs to be eliminated by making frame length a result of actual fields rather than a manually configured `headerLength` parameter.

## 3.5 ACKs are simulation objects, not IEEE ACK frames

ACK recognition is currently based partly on:

`packet->getName() == "CSMA-Ack"`

rather than the Frame Type field.

The generated ACK uses the same generic MAC-header class as data and does not correctly model the standard ACK format or enhanced ACK mechanisms.

ACK matching should instead be based on the actual protocol fields and transaction context.

## 3.6 Sequence-number wraparound appears broken

The transmitted sequence number is eventually serialized to eight bits, while `SeqNrParent` and `SeqNrChild` are unbounded integers.

The duplicate test essentially uses:

`SeqNr < ExpectedNr`.

After the on-wire sequence number wraps from 255 to 0, the receiver can therefore treat the wrapped sequence as old.

Duplicate detection needs proper modulo-256 sequence semantics rather than monotonically increasing host integers.

## 3.7 CCA is overly abstract

CCA currently essentially asks whether:

`radio->getReceptionState() == IDLE`.

IEEE 802.15.4 PHYs define PHY-specific CCA/energy-detection behavior. CCA should be a PHY service, with appropriate CCA mode, thresholds and timing, rather than a generic test of whether INET happens to be receiving something.

## 3.8 The UWB interface is not using the 802.15.4 MAC

An especially important detail: `Ieee802154UwbIrInterface` currently combines `Ieee802154UwbIrRadio` with the generic `AckingMac`, not `Ieee802154Mac`.

[Ieee802154UwbIrInterface.ned](https://github.com/inet-framework/inet/blob/master/src/inet/linklayer/ieee802154/Ieee802154UwbIrInterface.ned?utm_source=chatgpt.com)

Furthermore, its PHY code identifies itself as ported from the old IEEE 802.15.4a UWB-IR model. It contains old ranging mode constants, but this should not be confused with an implementation of the modern 802.15.4-2024 UWB/ranging system.

---

# 4. Major missing MAC functionality

## 4.1 Correct MAC frame model — essentially missing

The new model needs to represent at least:

| Feature                                      | Current status              |
| -------------------------------------------- | --------------------------- |
| Complete Frame Control field                 | Missing                     |
| Frame Type                                   | Effectively fixed to Data   |
| Security Enabled                             | Missing                     |
| Frame Pending                                | Missing                     |
| ACK Request                                  | Missing                     |
| PAN ID Compression                           | Missing                     |
| Sequence Number Suppression where applicable | Missing                     |
| IE Present                                   | Missing                     |
| Destination addressing mode                  | Fixed                       |
| Source addressing mode                       | Fixed                       |
| Frame Version                                | Missing                     |
| 16-bit addresses                             | Missing                     |
| 64-bit EUI-64 addresses                      | Incorrectly represented     |
| Destination PAN ID                           | Hard-coded                  |
| Source PAN ID                                | Misused for protocol number |
| Header Information Elements                  | Missing                     |
| Payload Information Elements                 | Missing                     |
| Auxiliary Security Header                    | Missing                     |
| Proper MAC FCS                               | Missing/inaccurate          |
| Variable frame/header length                 | Missing                     |

This is the highest-priority work because virtually every later 802.15.4 feature depends on it.

---

# 5. Missing MAC frame families

The current code effectively supports a simulation-specific Data/ACK distinction.

A proper implementation needs real support for the standard frame families, including at minimum:

**Data frames**, **ACK frames**, **Beacon frames**, and **MAC Command frames**, together with the modern frame-version/Information-Element machinery required by newer mechanisms.

Enhanced beacons and enhanced acknowledgments should be constructed using the common frame model rather than hard-coded special packet classes wherever possible.

---

# 6. Missing MAC management architecture

INET currently has essentially no IEEE 802.15.4 MLME.

A standards-oriented implementation needs a MAC PIB and the corresponding service primitives/concepts for such operations as:

* MAC data request/confirm/indication;
* PAN start/configuration;
* scan;
* association;
* disassociation;
* synchronization;
* polling;
* receiver control;
* PIB get/set;
* communication status and synchronization loss.

This does not mean OMNeT++ messages must mimic the standard API byte-for-byte. It means the model should preserve the standard's separation between the **MAC Common Part Sublayer data service**, **management entity**, and **PHY services**.

This separation is important because otherwise scanning, association, TSCH and security will all end up tangled inside the current MAC FSM.

For comparison, ns-3 already structures its 802.15.4 implementation around these service primitives and currently supports both slotted and unslotted CSMA/CA, beacon operation, scanning and association. ([NS-3 Network Simulator][5]) This would be a useful first interoperability/parity target for INET, although ns-3 itself still lacks important parts such as general indirect transfers, GTS and security. ([NS-3 Network Simulator][6])

---

# 7. Missing PAN formation, scanning and association

INET has no real notion of an IEEE 802.15.4 PAN.

Missing pieces include:

**PAN coordinator/device roles**, PAN ID configuration and conflict handling, channel/page selection, PAN descriptors, energy-detection scans, active scans, passive scans, orphan scanning/recovery, beacon requests, association request/response, short-address allocation, coordinator realignment, disassociation, and synchronization-loss processing.

At present nodes simply have an INET 48-bit MAC address and can exchange frames without going through any of this machinery.

---

# 8. Missing indirect transmission and polling

This is an important low-power feature.

The PAN coordinator must be capable of buffering data for sleeping devices and advertising pending traffic. The device can then poll the coordinator using the appropriate MAC command.

That requires:

* pending-transaction state;
* Frame Pending handling;
* Data Request command processing;
* expiration of pending transactions;
* beacon pending-address information;
* polling procedures.

None of this exists in the current MAC.

---

# 9. Missing beacon-enabled mode

INET only approximates non-beacon unslotted operation.

A major missing subsystem is beacon-enabled operation:

* periodic beacon generation;
* synchronization with coordinator beacons;
* Beacon Order and Superframe Order;
* superframe timing;
* active and inactive portions;
* CAP;
* CFP;
* slotted CSMA/CA;
* battery-life extension behavior;
* beacon-loss detection.

In beacon-enabled operation, the active superframe contains contention and contention-free regions, with slotted CSMA/CA used in the CAP and allocated GTSs in the CFP. ([Springer][7])

---

# 10. Missing Guaranteed Time Slots

The complete GTS machinery is absent:

* GTS Request commands;
* allocation/deallocation;
* GTS descriptors in beacons;
* CFP scheduling;
* GTS expiration;
* direction handling;
* coordinator scheduling state.

I would implement GTS **after** beacon-enabled mode rather than trying to add it simultaneously.

---

# 11. Missing standard security

The current implementation has no real IEEE 802.15.4 MAC security.

At minimum, the base implementation requires:

* Auxiliary Security Header;
* security levels;
* key identifier modes;
* key/device/security-level tables;
* frame counters;
* nonce construction;
* authentication/encryption;
* replay protection;
* incoming security processing;
* outgoing security processing;
* security-related PIB state.

Cryptographic operations should be separated from the MAC state machine behind an interface. That would make it possible to support both the existing AES-based mechanisms and the new Ascon algorithms without duplicating the MAC.

IEEE 802.15.4ae-2026 now adds Ascon to the MAC security specification, so Ascon should eventually become another crypto-suite implementation rather than another MAC implementation. ([IEEE Standards Association][8])

---

# 12. Missing Information Elements and enhanced frames

Information Elements are foundational for modern 802.15.4 operation and should not be postponed until TSCH implementation.

The frame model should support:

* ordered Header IEs;
* ordered Payload IEs;
* known typed IE subclasses;
* unknown/raw IE preservation.

This should follow the same general serialization principle used elsewhere in INET for TLV protocols: preserve ordering and unknown elements so that captured frames can round-trip.

Without an IE framework, implementing TSCH and various modern MAC extensions cleanly becomes difficult.

---

# 13. Missing TSCH

There is no 802.15.4-specific TSCH implementation in current `master`; repository searches for TSCH under the 802.15.4 implementation yield no such MAC functionality.

TSCH deserves its own channel-access/scheduling component and needs at least:

* Absolute Slot Number;
* slotframe definitions;
* link definitions;
* timeslot templates;
* shared/dedicated links;
* channel-offset handling;
* channel hopping;
* Enhanced Beacons;
* TSCH synchronization;
* guard times;
* clock drift/resynchronization;
* keep-alive handling;
* TSCH Information Elements;
* joining state.

The standard deliberately separates TSCH operation from higher-level scheduling policy: a scheduling function can decide which cells should exist while TSCH implements their execution. This distinction is useful architecturally in INET as well. TSCH has been part of IEEE 802.15.4 since the 2015 revision. ([IETF][9])

Thus I would expose an `ITschSchedule`-like API instead of embedding a particular 6TiSCH scheduler in the MAC.

---

# 14. Missing DSME and other advanced MAC modes

There is likewise no DSME implementation.

DSME would require a separate channel-access engine with:

* multi-superframe scheduling;
* DSME GTS allocation;
* multi-channel operation;
* channel hopping/adaptation;
* DSME command procedures;
* synchronization and schedule state.

TSCH and DSME should share the common packet, security, PIB, radio and management infrastructure, but **should not be implemented as a gigantic set of branches inside one CSMA FSM**.

---

# 15. PHY gaps

The PHY gap is almost as significant as the MAC gap.

The existing narrowband radio is essentially one parameterization centered on the familiar 2.4 GHz, 250 kbit/s O-QPSK PHY. INET's own documentation notes that the standard defines several alternative PHYs while the supplied narrowband implementation is only partially parameterized. ([INET Framework][1])

Missing or incomplete areas include:

**Standard PHY service interface:** proper PD-DATA and PLME-style CCA, ED, state control and attribute access.

**Channel/page model:** channel number, channel page, supported-channel sets and runtime switching.

**CCA semantics:** CCA modes and PHY-specific detection thresholds instead of just asking whether the generic radio is currently idle.

**ED:** standard Energy Detection operation and results.

**LQI:** generation and propagation of Link Quality Indication.

**PPDU representation:** explicit synchronization/header/data structure where useful, rather than only modelling transmission duration as `preamble + header + data`.

**2.4-GHz PHY accuracy:** validate DSSS/O-QPSK BER/PER behavior rather than relying solely on the generic APSK BER abstraction.

**Other standardized PHYs:** legacy sub-GHz modes and the much larger collection of later PHYs are not present as complete IEEE 802.15.4 PHY implementations.

The 2024 standard intentionally covers multiple PHYs across different regulatory regions and includes precision-ranging modes. ([IEEE Standards Association][2]) A complete implementation of every 2024 PHY would therefore be a major project in its own right.

---

# 16. UWB and ranging

INET's UWB-IR implementation is historically based on IEEE 802.15.4a. It contains some ranging-related mode definitions but is not a modern 802.15.4 UWB/ranging stack.

I would treat modern UWB as a separate project after the common MAC has been fixed.

The 2024 standard already includes precision-ranging modes, while the ongoing P802.15.4ab project is making substantial further changes to UWB PHY/MAC/ranging: new modulation/coding choices, frequencies, interference mitigation, high-integrity ranging, sensing and other mechanisms. ([IEEE Standards Association][2])

Trying to modernize this PHY as part of the first MAC effort would make the project unnecessarily large.

---

# 17. Recommended architecture

I would replace the current essentially monolithic design with approximately the following decomposition:

```text
Ieee802154Interface
 |
 +-- queue
 |
 +-- Ieee802154Mac
 |    |
 |    +-- Ieee802154MacPib
 |    +-- Ieee802154MacDataService
 |    +-- Ieee802154Management
 |    +-- Ieee802154ChannelAccess
 |    |     +-- UnslottedCsmaCa
 |    |     +-- SlottedCsmaCa
 |    |     +-- Tsch           [later]
 |    |     +-- Dsme           [later]
 |    |
 |    +-- Ieee802154Security   [optional]
 |
 +-- Ieee802154Radio
```

Packet representation should be independent of all of these:

```text
Ieee802154MacFrame
  FrameControl
  SequenceNumber?
  AddressingFields
  AuxiliarySecurityHeader?
  HeaderIE[]
  payload
  PayloadIE[]
  FCS

Ieee802154Address
  NONE
  SHORT_16
  EXTENDED_64
```

This gives each concern a clear owner and prevents TSCH, security, association and beacon timing from becoming additional states in one enormous FSM.

The `Ieee802154Address` type is particularly important. Extending INET's 48-bit `MacAddress` semantics to pretend that it is an EUI-64 would perpetuate the current problem.

---

# 18. Implementation sequence

I would implement the work as a chain of independently reviewable upstream PRs.

## Phase 0 — Standard inventory and executable coverage matrix

Before changing behavior, create a machine-readable 802.15.4 feature inventory based on IEEE 802.15.4-2024.

Each item should record:

```text
requirement / feature
standard clause
status: absent / partial / implemented / validated
source implementation
tests
notes
```

This is particularly suitable for the protocol-evidence infrastructure already being developed under `doc/project`.

**Deliverable:** authoritative scope matrix and a deliberately selected first profile.

The first profile should be:

> IEEE 802.15.4-2024-compatible 2.4 GHz O-QPSK + non-beacon unslotted MAC.

Do not initially claim full 802.15.4-2024.

---

## Phase 1 — Address and frame-model foundation

Introduce:

* `Ieee802154Address`;
* proper Frame Control model;
* variable address fields;
* PAN IDs;
* 16-bit and 64-bit addresses;
* frame versions;
* optional sequence number;
* ACK Request/Frame Pending/etc.;
* variable header length.

No channel-access behavioral changes yet.

Remove `networkProtocol` from the on-wire MAC header.

**Definition of done:** arbitrary legal combinations of the basic Frame Control/address fields can be represented without loss.

---

## Phase 2 — Wire-format serializer, dissector and FCS

Implement a serializer/deserializer that derives the layout from the FCF.

Add:

* correct PAN-ID-presence/compression rules;
* correct variable lengths;
* proper FCS trailer and CRC;
* malformed-frame handling;
* dissector support;
* PCAP recording support.

Use real packet captures as independent evidence.

**Definition of done:** selected real 802.15.4 captures deserialize → serialize byte-for-byte, and Wireshark successfully decodes INET-generated frames.

This is the point where the existing serializer kludges should disappear completely.

---

## Phase 3 — Standards-correct non-beacon data transfer

Replace the current data/ACK behavior with genuine 802.15.4 operation:

* MCPS data semantics;
* proper unslotted CSMA/CA;
* standard backoff state;
* CCA through a PHY service;
* ACK Request bit;
* immediate ACK generation;
* sequence matching;
* retransmission rules;
* modulo sequence-number duplicate handling;
* correct IFS/turnaround behavior;
* reception/filtering rules.

Static PAN IDs and addresses are acceptable at this stage.

**Milestone:** this should already be a useful, realistic PHY/MAC for manually configured 6LoWPAN networks.

---

## Phase 4 — PHY service model

Introduce explicit 802.15.4 PHY services/contracts for:

* CCA;
* ED;
* transceiver state;
* channel/page;
* data transfer;
* LQI and reception metadata.

Refactor the existing 2.4-GHz radio through those contracts.

Validate its PER/SNR behavior against published/reference curves rather than just asserting that the generic APSK equations are adequate.

---

## Phase 5 — PIB and management primitives

Add `Ieee802154MacPib` and structured management contracts.

Start with:

* PAN ID;
* short/extended addresses;
* coordinator state;
* channel/page state;
* CSMA parameters;
* retry parameters;
* beacon parameters;
* security-related placeholders.

The operational state should live in the relevant owner, while the PIB holds the standardized shared/configurable state.

---

## Phase 6 — MAC commands and scanning

Implement proper command frames and:

* ED scan;
* active scan;
* passive scan;
* orphan scan;
* beacon request;
* coordinator realignment where required;
* PAN descriptors.

This makes network discovery possible instead of relying entirely on configuration files.

---

## Phase 7 — Association and PAN lifecycle

Implement:

* PAN coordinator start;
* association request/response;
* capability information;
* short-address allocation;
* association state;
* disassociation;
* sync-loss handling.

At the end of this phase, a device should be able to discover and join a PAN without preconfigured peer information.

This is roughly the level at which INET would start exceeding its present “generic CSMA” character and become recognizably an IEEE 802.15.4 MAC. It would also bring the core bootstrap functionality closer to what ns-3 currently provides. ([NS-3 Network Simulator][10])

---

## Phase 8 — Indirect transmission and low-power polling

Add:

* pending transaction queues;
* Data Request;
* Frame Pending;
* pending-address beacon fields;
* transaction persistence;
* device polling;
* receiver enable/sleep behavior.

This phase is important for meaningful low-power WPAN simulations.

---

## Phase 9 — Beacon-enabled PANs

Implement:

* beacon generation and reception;
* synchronization;
* BO/SO timing;
* superframe scheduler;
* active/inactive intervals;
* slotted CSMA/CA;
* CAP.

Keep beacon scheduling separate from the generic MAC frame-processing state machine.

---

## Phase 10 — GTS / CFP

Build CFP and GTS on top of Phase 9:

* allocation requests;
* coordinator allocation table;
* beacon descriptors;
* transmit and receive GTSs;
* expiration/deallocation;
* CFP transmission scheduling.

This should be its own PR series because it can be tested independently once the beacon scheduler exists.

---

## Phase 11 — Information Elements and enhanced MAC machinery

Although the basic IE containers should exist earlier in the packet model, this phase implements the semantic handling necessary for modern modes:

* Header IE framework;
* Payload IE framework;
* enhanced beacons;
* enhanced acknowledgments;
* generic unknown-IE preservation.

This is the direct prerequisite for TSCH.

---

## Phase 12 — MAC security

Implement the security pipeline independently of channel access:

```text
outgoing MAC frame
    -> security processing
    -> serialization

wire frame
    -> FCS validation
    -> security processing
    -> normal MAC receive processing
```

First implement the base 802.15.4 security mechanisms.

Then add Ascon from IEEE 802.15.4ae-2026 as another suite. ([IEEE Standards Association][3])

---

## Phase 13 — TSCH

Implement TSCH as an alternative channel-access engine:

```text
TschMac
  ASN
  timeslot template
  slotframe[]
  link[]
  hopping sequence
  synchronization state
```

Provide a scheduling interface but **do not bake a 6TiSCH Scheduling Function into IEEE 802.15.4**.

A later INET 6TiSCH model could plug a scheduling function into this API.

---

## Phase 14 — DSME

Implement DSME independently, sharing:

* common MAC frames;
* IEs;
* PIB;
* security;
* PHY service;
* timing infrastructure.

This avoids duplicating fundamental protocol logic.

---

## Phase 15 — PHY expansion

After the 2.4-GHz PHY is solid, additional PHYs can be added one family at a time.

I would order them according to research/use demand rather than attempting to implement all PHY clauses simultaneously:

1. classical sub-GHz PHYs;
2. selected SUN PHYs;
3. additional narrowband PHYs;
4. modern UWB/ranging.

IEEE already has a proposed SUN extension that illustrates how broad this part of the standard has become: P802.15.4ad adds new bandwidths, modulation/coding schemes, long-range modes and MAC modifications. ([IEEE Standards Association][2])

---

## Phase 16 — 2026 privacy amendment

Only after the base MAC/address management is mature should IEEE 802.15.4ac-2026 be implemented:

* randomized addresses;
* session continuity;
* associated MAC exchanges/state.

The amendment was published on August 28, 2026. ([IEEE Standards Association][4])

---

# 19. Test strategy

This project should not rely primarily on INET fingerprint tests. Fingerprints are useful regression checks but cannot establish standards correctness.

For each implemented requirement I would use four complementary layers of verification.

### Constructive/unit tests

Check individual algorithms and data structures:

* FCF encode/decode;
* address-field presence matrix;
* PAN-ID compression;
* CRC;
* sequence wraparound;
* CSMA backoff boundaries;
* superframe calculations;
* TSCH ASN/channel calculation.

### Protocol tests

Script complete frame exchanges:

```text
DATA -> ACK
DATA -> ACK lost -> retransmission
CCA busy -> backoff
scan -> beacon -> association request -> association response
poll -> pending data
beacon sync -> CAP access
GTS allocation -> CFP data
```

These should verify exact order, timing and fields.

### Capture-based tests

Build a corpus of real IEEE 802.15.4 captures.

For supported frames:

```text
pcap
 -> INET deserialize
 -> INET serialize
 -> byte-identical pcap
```

Also compare decoded fields against Wireshark/tshark, because symmetric serializer mistakes can survive a round-trip test.

### Cross-model validation

Use ns-3 as an independent comparison for overlapping functionality such as:

* unslotted CSMA/CA;
* slotted CSMA/CA;
* beacon timing;
* scanning;
* association;
* PHY PER.

ns-3 should be treated as an independent implementation, not as the specification. Its documentation explicitly lists its own limitations. ([NS-3 Network Simulator][5])

---

# 20. Recommended first practical target

Trying to implement “all of IEEE 802.15.4-2024” immediately would be comparable to attempting a very broad IEEE 802.11 implementation in one development cycle.

I recommend defining **INET IEEE 802.15.4 Core** as the first deliverable:

```text
IEEE 802.15.4-2024
    2.4 GHz O-QPSK
    real MPDU/PPDU representation
    16/64-bit addressing
    PAN IDs
    correct FCS
    non-beacon PAN
    unslotted CSMA/CA
    immediate ACK
    retransmissions
    CCA / ED / LQI
    real command frames
    scanning
    PAN creation
    association/disassociation
    indirect data/polling
```

That target would already transform INET's model from a generic sensor-network CSMA approximation into a useful standards-oriented IEEE 802.15.4 implementation and would provide a strong foundation for **6LoWPAN + IPv6 + UDP + CoAP**.

A second target would add:

```text
beacon-enabled operation
slotted CSMA/CA
GTS
security
```

A third target would add:

```text
Information Elements
TSCH
DSME
additional PHYs
UWB/ranging
2026 amendments
```

This progression makes the project useful long before the enormous surface area of the complete current standard has been implemented.

---

# 21. Suggested upstream PR decomposition

For INET upstream, I would avoid one large “implement 802.15.4” PR. A sensible dependency chain is:

|  PR | Change                                   | Behavioral risk         |
| --: | ---------------------------------------- | ----------------------- |
|   1 | 802.15.4 requirements/coverage inventory | None                    |
|   2 | `Ieee802154Address`                      | Low                     |
|   3 | Real FCF + addressing packet classes     | Low                     |
|   4 | Serializer/deserializer + capture tests  | Medium                  |
|   5 | FCS/trailer + dissector/PCAP             | Medium                  |
|   6 | Correct non-beacon DATA/ACK exchange     | High                    |
|   7 | Correct unslotted CSMA/CA + PHY CCA API  | High                    |
|   8 | PHY ED/LQI/channel/page services         | Medium                  |
|   9 | MAC PIB and primitive API                | Medium                  |
|  10 | MAC command frame family                 | Medium                  |
|  11 | Scanning                                 | Medium                  |
|  12 | Association/disassociation               | High                    |
|  13 | Indirect transmission/polling            | High                    |
|  14 | Beacon/superframe infrastructure         | High                    |
|  15 | Slotted CSMA/CA                          | High                    |
|  16 | GTS                                      | High                    |
|  17 | Information Elements                     | Medium                  |
| 18+ | Security, TSCH, DSME, additional PHYs    | Separate feature series |

The crucial ordering is **wire representation → PHY/MAC contracts → correct basic data path → management → advanced modes**.

Implementing TSCH, GTS, security, or association before replacing the current fake MAC header would accumulate technical debt immediately.

---

# 22. Overall assessment

The good news is that INET is not starting from zero: the radio-medium infrastructure, packet framework, queues, energy framework, simulation lifecycle, basic CSMA machinery, serializers, PCAP support and testing infrastructure already exist.

But the amount of reusable **802.15.4 protocol logic itself** is relatively small.

The current `Ieee802154Mac` should therefore be regarded primarily as a historical/basic sensor-MAC implementation rather than as the foundation whose existing behavior defines the architecture of a future standards-complete implementation.

The most important first milestone is not TSCH, security, beaconing or association. It is:

> **Make an IEEE 802.15.4 frame in INET actually be an IEEE 802.15.4 frame.**

Once the packet format, address model, PHY service boundary and PIB/management architecture are sound, the remaining features become independently implementable and testable.

One aspect I would emphasize for your intended 802.15.4 + 6LoWPAN work is that **Phases 1–8 are the critical path**; TSCH/DSME and the large set of alternative PHYs can remain separate feature families. That gives you a reasonably bounded route to a useful standards-oriented stack rather than making “complete 802.15.4-2024” the first milestone.

[1]: https://inet.omnetpp.org/docs/users-guide/ch-802154.html "The 802.15.4 Model — INET 4.6.0 documentation"
[2]: https://standards.ieee.org/ieee/802.15.4/11041/ "IEEE SA - IEEE 802.15.4-2024"
[3]: https://standards.ieee.org/ieee/802.15.4ae/11836/?utm_source=chatgpt.com "IEEE SA - IEEE 802.15.4ae-2026"
[4]: https://standards.ieee.org/ieee/802.15.4ac/11272/?utm_source=chatgpt.com "IEEE SA - IEEE 802.15.4ac-2026"
[5]: https://www.nsnam.org/docs/release/3.43/models/html-new/lr-wpan.html "18. Low-Rate Wireless Personal Area Network (LR-WPAN) — Model Library"
[6]: https://www.nsnam.org/docs/models/html/lr-wpan.html?utm_source=chatgpt.com "18. IEEE 802.15.4: Low-Rate Wireless Personal Area Network (LR-WPAN) — Model Library"
[7]: https://link.springer.com/article/10.1007/s00607-024-01307-9?utm_source=chatgpt.com "MAC approaches to communication efficiency and reliability under dynamic network traffic in wireless body area networks: a review | Computing | Springer Nature Link"
[8]: https://standards.ieee.org/ieee/802.15.4ae/11836/ "IEEE SA - IEEE 802.15.4ae-2026"
[9]: https://www.ietf.org/archive/id/draft-ietf-raw-technologies-08.html?utm_source=chatgpt.com "Reliable and Available Wireless Technologies"
[10]: https://www.nsnam.org/docs/models/singlehtml/index.html?utm_source=chatgpt.com "Model Library"
