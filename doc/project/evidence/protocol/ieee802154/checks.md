# IEEE 802.15.4 — selected English checks

> **Kind:** procedure · **Status:** draft · **Seal:** none · **Owns:** IEEE802154-C-* · **Stands on:** [features.md](features.md)

These procedures specify observations, not executable tests or verdicts. Each check verifies its
stimulus at the observed boundary; absence alone cannot establish correct rejection. Timing
observations require a declared resolution and uncertainty appropriate to the tested interface.
The scope notes identify additional clauses needed before a complete feature claim.

Common topology (O is a passive observer unless injection is explicitly requested):

```text
upper client U -> device A <---- radio link ----> device B -> upper observer
                            O                 energy source J
```

## Index

- [IEEE802154-C-WIRE](#ieee802154-c-wire) — Native address and legacy data/ACK encoding.
- [IEEE802154-C-ACCESS](#ieee802154-c-access) — Unslotted channel access.
- [IEEE802154-C-SEQUENCE](#ieee802154-c-sequence) — Device-wide sequence allocation and wrap.
- [IEEE802154-C-RECEIVE](#ieee802154-c-receive) — Filtering, immediate ACK eligibility and data indication.
- [IEEE802154-C-ACK](#ieee802154-c-ack) — Acknowledgment and direct/indirect retry distinction.
- [IEEE802154-C-SECURITY](#ieee802154-c-security) — Unsecured-profile security outcomes.
- [IEEE802154-C-PHY](#ieee802154-c-phy) — O-QPSK timing, CCA, ED and LQI.
- [IEEE802154-C-SERVICE](#ieee802154-c-service) — PIB access, reset and request capacity.
- [IEEE802154-C-SCAN](#ieee802154-c-scan) — Passive scan obligation and channel order.

## IEEE802154-C-WIRE

Imported definitions: [IEEE802-ADDRESS-1](../../standard/ieee802/catalog.md#ieee802-address-1), [IEEE802-ADDRESS-2](../../standard/ieee802/catalog.md#ieee802-address-2).

**Native address and legacy data/ACK encoding.**

Checks: [IEEE802154-ADDRESS-1](../../standard/ieee802154/catalog.md#ieee802154-address-1), [IEEE802154-ADDRESS-2](../../standard/ieee802154/catalog.md#ieee802154-address-2), [IEEE802154-WIRE-1](../../standard/ieee802154/catalog.md#ieee802154-wire-1), [IEEE802154-WIRE-2](../../standard/ieee802154/catalog.md#ieee802154-wire-2), [IEEE802154-WIRE-3](../../standard/ieee802154/catalog.md#ieee802154-wire-3), [IEEE802154-WIRE-4](../../standard/ieee802154/catalog.md#ieee802154-wire-4), [IEEE802154-WIRE-5](../../standard/ieee802154/catalog.md#ieee802154-wire-5), [IEEE802154-WIRE-6](../../standard/ieee802154/catalog.md#ieee802154-wire-6); strengths are retained in those entries.

Additional checked statements: [IEEE802154-WIRE-7](../../standard/ieee802154/catalog.md#ieee802154-wire-7), [IEEE802154-WIRE-8](../../standard/ieee802154/catalog.md#ieee802154-wire-8), [IEEE802154-WIRE-9](../../standard/ieee802154/catalog.md#ieee802154-wire-9), [IEEE802154-WIRE-10](../../standard/ieee802154/catalog.md#ieee802154-wire-10), [IEEE802154-WIRE-11](../../standard/ieee802154/catalog.md#ieee802154-wire-11), [IEEE802154-WIRE-12](../../standard/ieee802154/catalog.md#ieee802154-wire-12), [IEEE802154-WIRE-13](../../standard/ieee802154/catalog.md#ieee802154-wire-13), [IEEE802154-WIRE-14](../../standard/ieee802154/catalog.md#ieee802154-wire-14), [IEEE802154-WIRE-15](../../standard/ieee802154/catalog.md#ieee802154-wire-15), [IEEE802154-WIRE-16](../../standard/ieee802154/catalog.md#ieee802154-wire-16).

Mockup: Two devices A and B with native short and extended identities, an octet observer O, and an independent frame decoder. Configure one PAN and then two distinct PAN IDs.

Procedure:

1. Encode AC-DE-48-23-45-67-89-01 and AC-DE-48-23-45-67-89-02, then two addresses differing only in their high 16 bits.
2. For legacy versions, enumerate short/short, short/extended, extended/short and extended/extended pairs, with equal and unequal PAN IDs. Repeat with source-only and destination-only addressing.
3. Request a legacy data transfer containing 00 FF 01 02 80 as payload. Observe the frame and delivered bytes independently.
4. Supply an independently authored immediate-ACK frame and verify the address-free layout. Decode truncated input at each supported field boundary as a robustness companion, without treating that companion as a new normative status requirement.

Expected observations:

1. The observer sees the requested source/destination identities, including distinct high address bits; AC-DE-48-23-45-67-89-01 is encoded as 01 89 67 45 23 48 DE AC.
2. Equal-PAN dual addressing includes only the destination PAN; unequal-PAN dual addressing includes both PANs. Single addressing includes its own PAN with compression clear.
3. LegacyTx produces version 1 data and the indicated MSDU is exactly 00 FF 01 02 80.
4. The immediate ACK contains FCF and DSN followed by the selected FCS, without source or destination addresses.

Arithmetic and scope: For unsecured legacy frames, MHR length is 3 plus the present PAN fields (2 octets each) and address fields (2 or 8 each). Same-PAN short/short therefore has a 9-octet MHR; different-PAN extended/extended has a 23-octet MHR. With O-QPSK two-octet FCS the latter has at most 102 payload octets in a 127-octet PSDU. A symmetric codec round trip is insufficient; use independent octets. The variants below add the independent CRC example and legacy field restrictions; full malformed-input coverage remains necessary.

Additional procedure variants (each row includes its stimulus and required observation):

| Stimulus | Expected observation |
| --- | --- |
| Encode version-0/1 data, then inject a legacy frame with sequence suppression or IE Present set | Generated legacy fields remain zero and DSN occupies one octet; invalid legacy combinations are never parsed using a shifted version-2 layout. |
| Enumerate addressing modes 00, 10 and 11; inject reserved 01 | Mode widths are absent, 2 and 8 octets. Reserved mode does not consume a guessed address width. |
| Submit source-only and destination-only legacy data, then both absent | The first two include the corresponding PAN and imply the coordinator direction. Both absent fails with INVALID_ADDRESS before transmission. |
| Submit unsecured direct data outside indirect/CSL/TSCH operation | No auxiliary security header; Frame Pending is zero and does not invent a poll operation on reception. |
| Acknowledge ordinary data outside CSL | Immediate ACK FCF is 0x0002 and DSN matches the received frame; no security, address, sequence-suppression or IE fields are inserted. |
| Calculate FCS for the standard's ACK example, then mutate an MHR or payload bit independently | Input octets 02 00 6A yield FCS octets E4 79. Mutation changes the expected FCS; preamble/PHR are not covered. |

Independent FCS oracle: clause 7.2.11 gives the transmitted bit sequence
`010000000000000001010110` and remainder `0010011110011110`. Within each octet,
the first transmitted bit is the least significant bit, so these correspond to `02 00 6A`
and `E4 79`, respectively. Form the polynomial from the input bits in temporal order,
multiply by x^16 and divide modulo two by x^16+x^12+x^5+1. This derives the expected
bytes from the standard, independently of any implementation's CRC helper. The final MPDU
is `02 00 6A E4 79`, five octets, and its O-QPSK PPDU duration is 352 microseconds.
This one vector does not exhaust data-frame CRC/length checks.

Additional checked statement: [IEEE802154-WIRE-17](../../standard/ieee802154/catalog.md#ieee802154-wire-17).

Inject FCF bit 7 set with a recomputed valid FCS. Observe the bit in the stimulus and verify that normal receive/ACK behavior matches the bit-clear control. The reserved bit is not a reserved frame-type or version value.

Imported-address vectors (synthetic bit-pattern checks, not allocation claims): combine IEEE 802
8.2.2 conventional notation with IEEE 802.15.4 4.5.1 rightmost-octet-first transmission.

| Conventional 64-bit value | Serialized address octets | Classification |
| --- | --- | --- |
| 0011223344556677 | 77 66 55 44 33 22 11 00 | Individual, despite numeric bit 0 being one |
| 0111223344556676 | 76 66 55 44 33 22 11 01 | Group, not broadcast, despite numeric bit 0 being zero |
| FFFFFFFFFFFFFFFF | FF FF FF FF FF FF FF FF | Group and broadcast |

Thus the numeric group mask is 0x0100000000000000; on these serialized bytes it is byte 7,
bit 0. With conforming group AR=0, check reception with macGroupRxMode=false and true, including
extended broadcast. IEEE 802.15.4 6.6.2(d)(2) does not give extended broadcast the unconditional
short-broadcast acceptance alternative. For an injected nonbroadcast group AR=1 legacy frame passing ordinary filtering with group
reception enabled, verify the immediate ACK required by 7.2.2.5 and 6.6.2. This injection violates
the sender rule in 6.6.3.1; the implementation must not generate such a frame itself. Repeat
with group reception disabled (filter discard/no ACK) and all-ones broadcast (no ACK).

## IEEE802154-C-ACCESS

**Unslotted channel access.**

Checks: [IEEE802154-ACCESS-1](../../standard/ieee802154/catalog.md#ieee802154-access-1), [IEEE802154-ACCESS-2](../../standard/ieee802154/catalog.md#ieee802154-access-2); strengths are retained in those entries.

Mockup: A transmits to B in a nonperiodic-beacon PAN. An energy source J controls whether each CCA observes busy or idle; O observes CCA decisions, backoff state and actual transmissions.

Procedure:

1. Set macMinBe=0, a valid macMaxBe, and macMaxCsmaBackoffs=0. Keep J active across the CCA observation.
2. Repeat with macMaxCsmaBackoffs=1: make the first observation busy, then make the next idle. Repeat with both busy.
3. Use a nonzero initial exponent and force enough busy results to reach the exponent cap. Start a fresh attempt after termination.

Expected observations:

1. The first observed CCA in the limit-zero case is busy; NB becomes 1 and the attempt fails without a data transmission.
2. At limit one, NB=1 permits another iteration. The second idle CCA permits transmission; a second busy CCA makes NB=2 and fails.
3. Backoff values remain in the Figure 6-2 range, BE saturates at macMaxBe, and a new attempt restores NB=0 and BE=macMinBe.

Arithmetic and scope: Figure 6-2 uses NB > macMaxCsmaBackoffs, not equality. Thus persistently busy access permits limit+1 CCA observations. Measure backoff in macUnitBackoffPeriod, separately from CCA observation time. This is an algorithm check, not a claim about contention fairness.

## IEEE802154-C-SEQUENCE

**Device-wide sequence allocation and wrap.**

Checks: [IEEE802154-SEQUENCE-1](../../standard/ieee802154/catalog.md#ieee802154-sequence-1), [IEEE802154-SEQUENCE-2](../../standard/ieee802154/catalog.md#ieee802154-sequence-2); strengths are retained in those entries.

Mockup: A sends data alternately to B and C; O captures serialized data and immediate ACKs. All peers remain reachable.

Procedure:

1. Arrange or observe the DSN just before wrap; retain the actual initial value as stimulus evidence.
2. Send more than 256 fresh acknowledged MSDUs while alternating B and C. Include DSNs 254, 255, 0 and 1.
3. Lose an ACK at the wrap boundary and distinguish the resulting retry from the next fresh MSDU. Restart a peer and repeat fresh transfers.

Expected observations:

1. O confirms the intended DSN boundary on actual transmitted octets.
2. Fresh transmissions progress in one device-wide modulo-256 sequence, irrespective of destination.
3. The retry retains its original DSN; the next fresh frame advances once. Upper indications remain usable across wrap and expose the received DSN.

Arithmetic and scope: The DSN is not a universal duplicate identifier. These checks do not require a particular duplicate cache or promise exactly-once application delivery. The retry assertion is also covered by IEEE802154-C-ACK.

## IEEE802154-C-RECEIVE

**Filtering, immediate ACK eligibility and data indication.**

Checks: [IEEE802154-RECEIVE-1](../../standard/ieee802154/catalog.md#ieee802154-receive-1), [IEEE802154-RECEIVE-2](../../standard/ieee802154/catalog.md#ieee802154-receive-2), [IEEE802154-RECEIVE-3](../../standard/ieee802154/catalog.md#ieee802154-receive-3), [IEEE802154-RECEIVE-4](../../standard/ieee802154/catalog.md#ieee802154-receive-4); strengths are retained in those entries.

Additional checked statements: [IEEE802154-RECEIVE-5](../../standard/ieee802154/catalog.md#ieee802154-receive-5), [IEEE802154-RECEIVE-6](../../standard/ieee802154/catalog.md#ieee802154-receive-6), [IEEE802154-RECEIVE-7](../../standard/ieee802154/catalog.md#ieee802154-receive-7), [IEEE802154-RECEIVE-8](../../standard/ieee802154/catalog.md#ieee802154-receive-8), [IEEE802154-RECEIVE-9](../../standard/ieee802154/catalog.md#ieee802154-receive-9), [IEEE802154-RECEIVE-10](../../standard/ieee802154/catalog.md#ieee802154-receive-10), [IEEE802154-RECEIVE-11](../../standard/ieee802154/catalog.md#ieee802154-receive-11), [IEEE802154-RECEIVE-12](../../standard/ieee802154/catalog.md#ieee802154-receive-12), [IEEE802154-RECEIVE-13](../../standard/ieee802154/catalog.md#ieee802154-receive-13), [IEEE802154-RECEIVE-14](../../standard/ieee802154/catalog.md#ieee802154-receive-14), [IEEE802154-RECEIVE-15](../../standard/ieee802154/catalog.md#ieee802154-receive-15).

Mockup: A sends to B; O can inject exact frames and observe both the link and B's upper indications. Each injection is confirmed at the receive boundary.

Procedure:

1. Inject valid local-PAN unicast data with AR set, then change only destination PAN to foreign and broadcast values, repairing the FCS each time.
2. Corrupt a data bit without updating the FCS. Repeat with correct FCS as a positive control.
3. Inject a structurally valid version-1 secured frame, with security disabled at B, using the same local unicast addressing and AR.
4. Compare ACK production with payload delivery and security-error indication for every case.

Expected observations:

1. The original and broadcast-PAN/local-address controls reach B and produce the required ACK and unsecured payload indication.
2. The foreign-PAN frame is rejected. The incorrect-FCS frame is discarded and produces neither delivery nor immediate ACK.
3. The valid legacy secured frame remains ACK-eligible under 6.6.2, while its unsuccessful incoming security result prevents MSDU delivery.

Arithmetic and scope: A broadcast PAN ID alone does not make the destination address broadcast. Separate the two predicates. The variants below cover source/destination omission and coordinator predicates. Group-address definitions remain a dependency on IEEE Std 802. Promiscuous MAC indication follows 10.23, distinct from an external capture observer.

Additional procedure variants:

| Stimulus | Expected observation |
| --- | --- |
| Inject reserved type 100 or reserved Data-frame version 3, with correct FCS | Neither normal MSDU delivery nor ordinary ACK; explicitly confirm the offending fields in injected bytes. |
| Compare local, foreign and broadcast short destinations at a matching PAN | Local and broadcast pass the destination predicate; foreign fails. Broadcast receives no ACK. |
| Compare exact EUI-64 match and an address differing only in its upper 16 bits | Only the complete matching identity passes; no low-48-bit approximation. |
| Inject a valid source-only frame with macImplicitBroadcast=false at a device and PAN coordinator, then vary source PAN | Only the PAN coordinator accepts it through predicate (d)(4), and only for matching source PAN. |
| Enable macImplicitBroadcast and repeat source-only input | Predicate (d)(3) admits the destination-omitted frame; apply broadcast semantics for ACK eligibility. |
| Inject an extended group destination with group reception disabled and enabled | Acceptance follows macGroupRxMode; group traffic does not request ACK. Use the imported address definitions and [C-WIRE vectors](#ieee802154-c-wire). |
| Receive a compressed-PAN frame | Use destination PAN as effective source PAN internally; keep wire-presence validity distinct in upper metadata. |
| Complete an acknowledged transmit task with macRxOnWhenIdle=false, then true | Receive for the ACK as required, then restore the selected idle reception state; idle=false cannot disable a required ACK wait. |
| Enter and exit supported promiscuous mode while idle reception is false | Entry enables reception; exit restores false. Indication MSDU is MHR+MAC payload, excluding FCS; only Msdu, MpduLinkQuality, Timestamp and Rssi may be treated as valid. |

Promiscuous acceptance and normal ACK eligibility must be tested as separate observations under
10.23.1 and 6.6.2. Optional timestamp support must be declared; the allowed-field list does not
create a valid timestamp when that capability is absent. The four-field indication validity rule means a monitor cannot treat the
ordinary Dsn, SrcAddr or AckSent parameters as valid; it must inspect the raw MHR if needed.

Additional checked statement: [IEEE802154-RECEIVE-16](../../standard/ieee802154/catalog.md#ieee802154-receive-16).

In promiscuous mode inject correctly received foreign-address input and observe the raw MHR+payload indication separately from ACK behavior. Do not equate monitor delivery with a normal addressed-data acceptance decision; the source cross-reference interpretation must be stated.

## IEEE802154-C-ACK

Interframe spacing: [IEEE802154-ACCESS-3](../../standard/ieee802154/catalog.md#ieee802154-access-3).

**Acknowledgment and direct/indirect retry distinction.**

Checks: [IEEE802154-ACK-1](../../standard/ieee802154/catalog.md#ieee802154-ack-1), [IEEE802154-ACK-2](../../standard/ieee802154/catalog.md#ieee802154-ack-2), [IEEE802154-ACK-3](../../standard/ieee802154/catalog.md#ieee802154-ack-3), [IEEE802154-ACK-4](../../standard/ieee802154/catalog.md#ieee802154-ack-4), [IEEE802154-ACK-5](../../standard/ieee802154/catalog.md#ieee802154-ack-5); strengths are retained in those entries.

Mockup: A sends to B with observer O and selective ACK loss. For the indirect variant, coordinator A already holds a pending frame and B issues Data Requests.

Procedure:

1. Transmit broadcast and unicast AR-clear data. Confirm the outgoing AR bit and observe completion without ACK.
2. Transmit AR-set unicast outside CAP, observe the received data end and ACK start, and compare the ACK DSN with the data DSN.
3. Suppress every ACK for a direct transmission with macMaxFrameRetries=0 and then 2. Repeat by losing every Data frame before receiver delivery. Include wrong-DSN and late-ACK injections in separate runs.
4. For a pending indirect frame, suppress the data ACK after one poll. Observe silence before a new poll, then request again.

Expected observations:

1. Broadcast traffic has AR=0; AR-clear unicast is delivered without generating ACK or autonomous retransmissions.
2. For O-QPSK, ACK starts macSifsPeriod after the last received data symbol and carries the data DSN.
3. In both loss cases, direct transmission attempts number 1 and 3 respectively; every retry retains DSN, and exhaustion reports NO_ACK. Data loss produces no receiver data indication, whereas ACK loss can follow a successful receiver indication. Wrong/late ACKs do not establish success for an unrelated exchange.
4. Failed indirect delivery does not retry by itself. A new Data Request permits sending the retained frame with the same DSN.

Additional spacing variant: queue a second Data frame after complete 18-octet and 19-octet
MPDUs, first without requesting ACK, then with a successful ACK. Use the selected PHY's 12-symbol
SIFS/turnaround and 40-symbol LIFS. The next PPDU starts no earlier than 192 µs or 640 µs,
respectively, after the first PPDU ends in the unacknowledged case, or after ACK reception ends
at the sender in the acknowledged case (Figure 6-1). Keep backoff observable so a long random
wait does not hide a missing IFS guard. MAC length includes FCS and excludes SHR/PHR. This check
is distinct from the AIFS delay before the ACK itself and from the sender's ACK deadline.

Arithmetic and scope: At 250 kbit/s and 4 bits/symbol, one symbol is 16 microseconds; 12-symbol AIFS is 192 microseconds. Specify the sender's expected-time deadline and event ordering before implementing late-ACK tests: 6.6.3.4 does not itself provide a numeric timeout. The continuation text of Figures 6-11 and 6-12 distinguishes lost-data and lost-ACK outcomes. Full poll/release timing belongs to a separate indirect-service extraction.

## IEEE802154-C-SECURITY

Service routing: [IEEE802154-SERVICE-8](../../standard/ieee802154/catalog.md#ieee802154-service-8).

**Unsecured-profile security outcomes.**

Checks: [IEEE802154-SECURITY-1](../../standard/ieee802154/catalog.md#ieee802154-security-1), [IEEE802154-SECURITY-2](../../standard/ieee802154/catalog.md#ieee802154-security-2), [IEEE802154-SECURITY-3](../../standard/ieee802154/catalog.md#ieee802154-security-3), [IEEE802154-SECURITY-4](../../standard/ieee802154/catalog.md#ieee802154-security-4), [IEEE802154-SECURITY-5](../../standard/ieee802154/catalog.md#ieee802154-security-5); strengths are retained in those entries.

Mockup: A and B have macSecurityEnabled=false. O observes outgoing requests/results and injects structurally valid secured and unsecured legacy frames.

Procedure:

1. Submit SecurityLevel=0 and nonzero SecurityLevel requests at A.
2. Inject local addressed unsecured data at B.
3. Inject secured version-0 and secured version-1 frames at B, each otherwise valid and AR-set.

Expected observations:

1. SecurityLevel=0 returns the unchanged frame with SUCCESS from the security procedure, then continues normal access/transmission. It does not immediately complete the upper request successfully: channel-access failure or ACK exhaustion may still determine MCPS-DATA.confirm. A nonzero request with security disabled produces MCPS-DATA.confirm(UNSUPPORTED_SECURITY), associated with its MsduHandle, without transmission.
2. Unsecured reception returns SUCCESS without requiring key lookup.
3. Secured version 0 produces MLME-COMM-STATUS.indication(UNSUPPORTED_LEGACY); secured version 1 produces MLME-COMM-STATUS.indication(UNSUPPORTED_SECURITY). Neither supplies a valid plaintext MSDU. Required legacy ACK behavior is checked independently by IEEE802154-C-RECEIVE. Repeat with AR=0: the same security-error indication occurs without an ACK. Incorrect FCS and ordinary filter failures are discarded before this incoming-security path and do not establish a security-error indication.

Arithmetic and scope: Table 8-5 continuation routes incoming security errors to COMM-STATUS; Table 8-31 and 6.6.1 route outgoing request errors to DATA.confirm. Do not read output security parameters that the procedure has not initialized. In 9.2.4(a)/(b), Status is set before returning, but the auxiliary-header parsing in (c) has not occurred. Represent security/key/plaintext output validity explicitly; retain usable frame addressing separately. Malformed/truncated security representations need structural safety checks separately; these well-formed-input checks do not assign them invented standardized statuses.

## IEEE802154-C-PHY

**O-QPSK timing, CCA, ED and LQI.**

Checks: [IEEE802154-PHY-1](../../standard/ieee802154/catalog.md#ieee802154-phy-1), [IEEE802154-PHY-2](../../standard/ieee802154/catalog.md#ieee802154-phy-2), [IEEE802154-PHY-3](../../standard/ieee802154/catalog.md#ieee802154-phy-3), [IEEE802154-PHY-4](../../standard/ieee802154/catalog.md#ieee802154-phy-4), [IEEE802154-PHY-5](../../standard/ieee802154/catalog.md#ieee802154-phy-5), [IEEE802154-PHY-6](../../standard/ieee802154/catalog.md#ieee802154-phy-6), [IEEE802154-PHY-7](../../standard/ieee802154/catalog.md#ieee802154-phy-7), [IEEE802154-PHY-8](../../standard/ieee802154/catalog.md#ieee802154-phy-8); strengths are retained in those entries.

Additional checked statements: [IEEE802154-PHY-9](../../standard/ieee802154/catalog.md#ieee802154-phy-9), [IEEE802154-PHY-10](../../standard/ieee802154/catalog.md#ieee802154-phy-10), [IEEE802154-PHY-11](../../standard/ieee802154/catalog.md#ieee802154-phy-11), [IEEE802154-PHY-12](../../standard/ieee802154/catalog.md#ieee802154-phy-12), [IEEE802154-PHY-13](../../standard/ieee802154/catalog.md#ieee802154-phy-13), [IEEE802154-PHY-14](../../standard/ieee802154/catalog.md#ieee802154-phy-14), [IEEE802154-PHY-15](../../standard/ieee802154/catalog.md#ieee802154-phy-15).

Mockup: PHY A receives from B and energy source J on the selected 2450 MHz channel. O observes timed PHY operations and PPDU boundaries.

Procedure:

1. Generate a PPDU with a known PSDU length L, including FCS; inspect SHR, PHR and total duration.
2. With no decodable frame, apply J below and above the Mode-1 threshold throughout CCA. Repeat CCA during a confirmed PPDU reception.
3. Configure two CCA durations, including a value not divisible by 16 microseconds, and measure the observation interval.
4. Apply stepped power during an ED observation and then sweep packet quality over a range for LQI.

Expected observations:

1. O confirms the PSDU bytes and length L; preamble is 4 zero octets, PHR counts L, and total PPDU duration is (6+L)*32 microseconds for the selected PHY.
2. Energy above threshold makes Mode 1 busy even without a decodable frame; a request during the defined PPDU reception interval reports busy.
3. CCA duration equals the configured microseconds without silently rounding the observation itself to whole symbols.
4. ED uses the specified averaging interval and LQI is present for received packets with at least eight distinct values.

Arithmetic and scope: The 6-octet PHY overhead is preamble 4, SFD 1 and PHR 1. ED measurement time and CSMA backoff rounding are different rules. Use the additional ED range/mapping variants below and a declared PHY sensitivity reference before asserting numeric ED results. Waveform, sensitivity and PER conformance require separate validation.

Additional procedure variants:

| Stimulus | Expected observation |
| --- | --- |
| Switch between transmit and receive at a controlled PPDU boundary | Each air-interface turnaround is at most 12 O-QPSK symbols (192 microseconds); confirm the measurement endpoints, not just callback delay. |
| Sweep selected channel k from 11 through 26 | Center frequency is 2405+5(k−11) MHz; endpoints are 2405 and 2480 MHz. No channel from another PHY's numbering is silently selected. |
| Declare an ED transfer floor consistent with the lowest specified PHY sensitivity; sweep across that floor and at least 40 dB of range | Every zero result denotes power below the lowest specified PHY sensitivity plus 10 dB; that bound is not an exact universal cutoff. Check the declared floor, linear mapping in dB within ±6 dB, range and endpoint saturation. |
| Receive successive packets at distinct qualities | Each packet has its own LQI; at least eight distinct LQI outputs are reachable. |
| Declare the operating-region channel set | Supported channels cover the allowed selected-PHY set; a simulation configuration is not itself proof of legal regional radio operation. |

For a configured CCA duration of 129 microseconds, the CCA observation lasts 129 microseconds.
The default backoff unit uses ceiling(129/16)+12=21 symbols, or 336 microseconds, per
Table 8-36. Do not round the observation itself to 144 microseconds merely because its
contribution to the default backoff unit is rounded to nine symbols.

## IEEE802154-C-SERVICE

Parameter support: [IEEE802154-SERVICE-9](../../standard/ieee802154/catalog.md#ieee802154-service-9).

**PIB access, reset and request capacity.**

Checks: [IEEE802154-SERVICE-1](../../standard/ieee802154/catalog.md#ieee802154-service-1), [IEEE802154-PIB-1](../../standard/ieee802154/catalog.md#ieee802154-pib-1), [IEEE802154-PIB-2](../../standard/ieee802154/catalog.md#ieee802154-pib-2), [IEEE802154-PIB-3](../../standard/ieee802154/catalog.md#ieee802154-pib-3); strengths are retained in those entries.

Additional checked statements: [IEEE802154-PIB-4](../../standard/ieee802154/catalog.md#ieee802154-pib-4), [IEEE802154-PIB-5](../../standard/ieee802154/catalog.md#ieee802154-pib-5), [IEEE802154-PIB-6](../../standard/ieee802154/catalog.md#ieee802154-pib-6), [IEEE802154-PIB-7](../../standard/ieee802154/catalog.md#ieee802154-pib-7), [IEEE802154-PIB-8](../../standard/ieee802154/catalog.md#ieee802154-pib-8), [IEEE802154-PIB-9](../../standard/ieee802154/catalog.md#ieee802154-pib-9), [IEEE802154-SERVICE-2](../../standard/ieee802154/catalog.md#ieee802154-service-2), [IEEE802154-SERVICE-3](../../standard/ieee802154/catalog.md#ieee802154-service-3), [IEEE802154-SERVICE-4](../../standard/ieee802154/catalog.md#ieee802154-service-4), [IEEE802154-SERVICE-5](../../standard/ieee802154/catalog.md#ieee802154-service-5), [IEEE802154-SERVICE-6](../../standard/ieee802154/catalog.md#ieee802154-service-6), [IEEE802154-SERVICE-7](../../standard/ieee802154/catalog.md#ieee802154-service-7).

Mockup: An upper client U issues requests to MAC A; O observes returned statuses, PIB values and transmissions. Select a known read-only attribute and a writable attribute with documented bounds/default.

Procedure:

1. GET an absent attribute and SET a read-only attribute. Read the latter again.
2. Set a writable attribute to a nondefault valid value, reset with SetDefaultPib=false, read back, then reset with true and read back.
3. Send an MSDU at the selected addressing-dependent capacity and another one octet larger. Observe exact frame size and terminal statuses.

Expected observations:

1. Absent GET reports UNSUPPORTED_ATTRIBUTE with no valid value; read-only SET reports READ_ONLY without changing the attribute.
2. The false reset preserves MAC PIB values; true reset restores specified defaults; successful reset completes with SUCCESS.
3. The boundary frame fits the selected PSDU limit. The oversized request reports FRAME_TOO_LONG and produces no frame transmission.

Arithmetic and scope: For same-PAN short/short unsecured O-QPSK data, capacity is 127-9-2=116 octets. For different-PAN extended/extended it is 127-23-2=102. Full status precedence and validation need the service tables, not one generic failure result. Reset PHY details are implementation-dependent; preserve this distinction from MAC PIB defaults.

Additional procedure variants:

| Stimulus | Expected observation |
| --- | --- |
| Write a PHY-owned read-only attribute, an absent attribute, and an out-of-range scalar | Respectively READ_ONLY, UNSUPPORTED_ATTRIBUTE and INVALID_PARAMETER; failed writes do not change the last accepted value. |
| Select a present hierarchical attribute with an invalid index | INVALID_INDEX, distinct from unknown attribute. This variant is conditional on supporting that hierarchical attribute. |
| Submit legacy data with both address modes NONE | INVALID_ADDRESS, no transmission and the original MsduHandle in the confirm. |
| Suppress all direct ACKs, then separately hold the medium busy until access fails | NO_ACK after the retry limit versus CHANNEL_ACCESS_FAILURE without transmission; do not collapse the two results. |
| Observe ordinary data indications at DSN wrap and after ACK generation | Dsn equals the received field. AckSent reports an ACK that has been sent, not merely AR=1 or an ACK scheduled for later. |
| Request ordinary O-QPSK transmission | DataRate selector is zero; the PHY rate is 250 kbit/s. The selector is not a rate in bits/second. |
| Reset with both SetDefaultPib values | SUCCESS is reported only after reset completion; reset(false) retains MAC PIB values, while reset(true) applies defaults. |

For MLME-SET of a present writable attribute, also request a source-valid value that the tested
capability declaration does not support. Clause 8.2.2 requires INVALID_PARAMETER; do not
substitute READ_ONLY or UNSUPPORTED_ATTRIBUTE merely because that value is unsupported.
Check preservation of the previous value under the declared failure-atomicity contract.

Test each specified status using a request with only that defect. Where several defects coexist,
a precedence order requires an explicit source or implementation contract; this procedure does
not invent precedence by the order of rows above.

Use the [PIB field domains](../../standard/ieee802154/catalog.md#pib-field-domains) as the
parameterized validation fixture: for each supported writable bounded integer, probe both
endpoints and each adjacent out-of-range value; for Boolean/enumerated attributes, probe each
supported value and an invalid encoding. Test read-only markers separately from fixed PHY
values: O-QPSK's 127-octet maximum does not make `phyMaxPacketSize` dagger-marked.
Change `macMaxBe` below the current `macMinBe` and confirm that no successful operation leaves
an invalid pair. Test the declared update procedure rather than assuming implicit clamping.
Reset with a fixed simulation-independent random input source or an observable random draw for
random-default sequence attributes; do not demand that two legitimate reset draws differ.
The PHY table supplies no defaults, so expected PHY initialization values need an explicitly
declared configuration or applicable PHY clause, not inferred zero values.

## IEEE802154-C-SCAN

**Passive scan obligation and channel order.**

Checks: [IEEE802154-SCAN-1](../../standard/ieee802154/catalog.md#ieee802154-scan-1), [IEEE802154-SCAN-2](../../standard/ieee802154/catalog.md#ieee802154-scan-2); strengths are retained in those entries.

Mockup: Device A scans channels containing beacon sources B and C. O observes A's channel visits, transmissions and scan result.

Procedure:

1. Request passive scan with an unordered channel set containing both sources and an empty channel.
2. Observe the actual channel order and relevant received beacons, then inspect the terminal scan result.

Expected observations:

1. A performs passive scanning and does not replace it with active solicitation.
2. Visits proceed from lowest to highest selected channel; discovered descriptors are returned on completion.

Arithmetic and scope: This check covers the universal capability and order only. Duration, result limits, cancellation, invalid-channel handling and state restoration need the passive-scan and MLME-SCAN child clauses before full scan acceptance.

## Statements without a procedure

None in the current bounded catalog. This does not cover statements not yet extracted; see the
[catalog extraction boundary](../../standard/ieee802154/catalog.md#extraction-boundary).
