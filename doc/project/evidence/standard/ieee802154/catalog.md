# IEEE 802.15.4 — selected base-standard statements

> **Kind:** what · **Status:** draft · **Seal:** none · **Owns:** IEEE802154-ADDRESS-*, IEEE802154-WIRE-*, IEEE802154-ACCESS-*, IEEE802154-SEQUENCE-*, IEEE802154-RECEIVE-*, IEEE802154-ACK-*, IEEE802154-SECURITY-*, IEEE802154-PHY-*, IEEE802154-SERVICE-*, IEEE802154-PIB-*, IEEE802154-SCAN-* · **Stands on:** [source.md](source.md)

Edition: **IEEE Std 802.15.4-2024**, without amendments. Entries below are a bounded extraction,
not an exhaustive mandatory-statement inventory. Source excerpts retain normative wording;
conditions prevent an excerpt from being applied outside its full clause. All cited clauses,
tables and figures are normative; explanatory notes are not promoted to requirements.
Physical PDF pages are one-based. Canonical locators refer to the pinned local corpus.
Model applicability and execution evidence live in the [model ledger](../../model/ieee802154/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [IEEE802154-ACCESS-3](#ieee802154-access-3) | Interframe spacing depends on the preceding MPDU length. |
| [IEEE802154-SERVICE-9](#ieee802154-service-9) | Unsupported or out-of-range MLME request parameters report INVALID_PARAMETER. |
| [IEEE802154-SERVICE-8](#ieee802154-service-8) | Incoming security errors generate a communication-status indication. |
| [IEEE802154-ADDRESS-1](#ieee802154-address-1) | Device extended identity uses EUI-64. |
| [IEEE802154-ADDRESS-2](#ieee802154-address-2) | Address octets are transmitted from rightmost to leftmost. |
| [IEEE802154-WIRE-1](#ieee802154-wire-1) | Legacy same-PAN dual addressing omits the source PAN. |
| [IEEE802154-WIRE-2](#ieee802154-wire-2) | Legacy different-PAN dual addressing includes both PAN IDs. |
| [IEEE802154-WIRE-3](#ieee802154-wire-3) | Legacy single addressing includes its PAN without compression. |
| [IEEE802154-WIRE-4](#ieee802154-wire-4) | Immediate ACKs have the Figure 7-15 layout. |
| [IEEE802154-WIRE-5](#ieee802154-wire-5) | A data payload preserves the supplied octets. |
| [IEEE802154-WIRE-6](#ieee802154-wire-6) | LegacyTx selects version 1 data. |
| [IEEE802154-ACCESS-1](#ieee802154-access-1) | Initialize attempt-local backoff state. |
| [IEEE802154-ACCESS-2](#ieee802154-access-2) | Nonperiodic-beacon transmission follows successful unslotted access. |
| [IEEE802154-SEQUENCE-1](#ieee802154-sequence-1) | Sequence allocation is device-wide. |
| [IEEE802154-SEQUENCE-2](#ieee802154-sequence-2) | Sequence counters wrap at their representable maximum. |
| [IEEE802154-RECEIVE-1](#ieee802154-receive-1) | An incorrect FCS causes discard. |
| [IEEE802154-RECEIVE-2](#ieee802154-receive-2) | A present destination PAN must match or be broadcast. |
| [IEEE802154-RECEIVE-3](#ieee802154-receive-3) | Valid legacy unicast AR frames require immediate ACK. |
| [IEEE802154-RECEIVE-4](#ieee802154-receive-4) | Data delivery requires successful incoming security processing. |
| [IEEE802154-ACK-1](#ieee802154-ack-1) | Broadcast and group-addressed frames do not request ACK. |
| [IEEE802154-ACK-2](#ieee802154-ack-2) | AR-clear frames receive no ACK. |
| [IEEE802154-ACK-3](#ieee802154-ack-3) | Outside-CAP ACK start is measured from the received last symbol. |
| [IEEE802154-ACK-4](#ieee802154-ack-4) | Direct retransmissions retain the DSN. |
| [IEEE802154-ACK-5](#ieee802154-ack-5) | Failed indirect transmission waits for a new poll. |
| [IEEE802154-SECURITY-1](#ieee802154-security-1) | Security level zero returns the outgoing frame unchanged. |
| [IEEE802154-SECURITY-2](#ieee802154-security-2) | A nonzero outgoing security request fails when security is disabled. |
| [IEEE802154-SECURITY-3](#ieee802154-security-3) | Incoming secured version zero reports unsupported legacy security. |
| [IEEE802154-SECURITY-4](#ieee802154-security-4) | Incoming nonlegacy secured frames fail when security is disabled. |
| [IEEE802154-SECURITY-5](#ieee802154-security-5) | Unsecured input succeeds when security is disabled. |
| [IEEE802154-PHY-1](#ieee802154-phy-1) | CCA Mode 1 detects energy above its threshold. |
| [IEEE802154-PHY-2](#ieee802154-phy-2) | A CCA request during PPDU reception reports busy. |
| [IEEE802154-PHY-3](#ieee802154-phy-3) | CCA observes for phyCcaDuration. |
| [IEEE802154-PHY-4](#ieee802154-phy-4) | ED averages over the selected measurement interval. |
| [IEEE802154-PHY-5](#ieee802154-phy-5) | LQI uses at least eight values. |
| [IEEE802154-PHY-6](#ieee802154-phy-6) | O-QPSK preamble is eight zero symbols. |
| [IEEE802154-PHY-7](#ieee802154-phy-7) | PHR length counts the complete PSDU. |
| [IEEE802154-PHY-8](#ieee802154-phy-8) | 2450 MHz O-QPSK operates at 250 kbit/s. |
| [IEEE802154-SERVICE-1](#ieee802154-service-1) | Oversize MPDUs fail before transmission. |
| [IEEE802154-PIB-1](#ieee802154-pib-1) | Dagger-marked MAC attributes are read-only to the upper layer. |
| [IEEE802154-PIB-2](#ieee802154-pib-2) | Unknown PIB reads return unsupported attribute. |
| [IEEE802154-PIB-3](#ieee802154-pib-3) | Reset can preserve or restore MAC PIB values. |
| [IEEE802154-SCAN-1](#ieee802154-scan-1) | All devices support passive scan. |
| [IEEE802154-SCAN-2](#ieee802154-scan-2) | Scan visits channels in ascending order. |
| [IEEE802154-WIRE-7](#ieee802154-wire-7) | Legacy frames cannot suppress their sequence number. |
| [IEEE802154-WIRE-8](#ieee802154-wire-8) | Legacy frames cannot advertise IEs. |
| [IEEE802154-WIRE-9](#ieee802154-wire-9) | Destination addressing mode must not be reserved. |
| [IEEE802154-WIRE-10](#ieee802154-wire-10) | Legacy source omission implies a present destination. |
| [IEEE802154-WIRE-11](#ieee802154-wire-11) | Legacy destination omission implies a present source. |
| [IEEE802154-WIRE-12](#ieee802154-wire-12) | Security control and auxiliary-header presence are related. |
| [IEEE802154-WIRE-13](#ieee802154-wire-13) | FCS covers MAC header and payload. |
| [IEEE802154-WIRE-14](#ieee802154-wire-14) | Two-octet FCS uses the specified polynomial division. |
| [IEEE802154-WIRE-15](#ieee802154-wire-15) | Immediate ACK FCF contains only its permitted fields. |
| [IEEE802154-WIRE-16](#ieee802154-wire-16) | Frame Pending is clear outside its specified mechanisms. |
| [IEEE802154-RECEIVE-5](#ieee802154-receive-5) | Reserved frame types fail filtering. |
| [IEEE802154-RECEIVE-6](#ieee802154-receive-6) | Reserved frame versions fail filtering. |
| [IEEE802154-RECEIVE-7](#ieee802154-receive-7) | Short destination acceptance includes local and broadcast addresses. |
| [IEEE802154-RECEIVE-8](#ieee802154-receive-8) | Extended destination acceptance includes enabled group addresses. |
| [IEEE802154-RECEIVE-9](#ieee802154-receive-9) | Implicit broadcast is an explicit receive predicate. |
| [IEEE802154-RECEIVE-10](#ieee802154-receive-10) | PAN coordinators accept matching source-only data. |
| [IEEE802154-RECEIVE-11](#ieee802154-receive-11) | An omitted source PAN can be inferred from destination PAN. |
| [IEEE802154-RECEIVE-12](#ieee802154-receive-12) | Idle receiver control is restored after a transceiver task. |
| [IEEE802154-RECEIVE-13](#ieee802154-receive-13) | Promiscuous entry enables reception. |
| [IEEE802154-RECEIVE-14](#ieee802154-receive-14) | Promiscuous indications contain MAC header plus payload. |
| [IEEE802154-RECEIVE-15](#ieee802154-receive-15) | Promiscuous exit restores the idle receiver policy. |
| [IEEE802154-PHY-9](#ieee802154-phy-9) | TX-to-RX readiness is bounded by turnaround. |
| [IEEE802154-PHY-10](#ieee802154-phy-10) | RX-to-TX turnaround is bounded. |
| [IEEE802154-PHY-11](#ieee802154-phy-11) | ED zero identifies sufficiently low power. |
| [IEEE802154-PHY-12](#ieee802154-phy-12) | ED spans at least 40 dB with linear decibel mapping. |
| [IEEE802154-PHY-13](#ieee802154-phy-13) | LQI is measured for every received packet. |
| [IEEE802154-PHY-14](#ieee802154-phy-14) | 2450 MHz channels use the specified center frequencies. |
| [IEEE802154-PHY-15](#ieee802154-phy-15) | Supported PHY channels include those allowed in the operating region. |
| [IEEE802154-PIB-4](#ieee802154-pib-4) | PHY PIB read-only markers restrict upper writes. |
| [IEEE802154-PIB-5](#ieee802154-pib-5) | Read-only writes report READ_ONLY. |
| [IEEE802154-PIB-6](#ieee802154-pib-6) | Unknown writes report UNSUPPORTED_ATTRIBUTE. |
| [IEEE802154-PIB-7](#ieee802154-pib-7) | Out-of-range writes report INVALID_PARAMETER. |
| [IEEE802154-PIB-8](#ieee802154-pib-8) | Invalid hierarchical indices have a distinct status. |
| [IEEE802154-PIB-9](#ieee802154-pib-9) | Reset success is reported on completion. |
| [IEEE802154-SERVICE-2](#ieee802154-service-2) | Legacy data cannot omit both addresses. |
| [IEEE802154-SERVICE-3](#ieee802154-service-3) | Direct retry exhaustion reports NO_ACK. |
| [IEEE802154-SERVICE-4](#ieee802154-service-4) | Direct access exhaustion reports CHANNEL_ACCESS_FAILURE. |
| [IEEE802154-SERVICE-5](#ieee802154-service-5) | Indications expose received DSN when present. |
| [IEEE802154-SERVICE-6](#ieee802154-service-6) | AckSent indicates an ACK actually sent. |
| [IEEE802154-SERVICE-7](#ieee802154-service-7) | O-QPSK service DataRate uses the default selector. |
| [IEEE802154-WIRE-17](#ieee802154-wire-17) | Reserved fields are zero on transmission and ignored on reception. |
| [IEEE802154-RECEIVE-16](#ieee802154-receive-16) | Promiscuous mode accepts received frames for monitor delivery. |

## IEEE802154-ACCESS-3

**Interframe spacing depends on the preceding MPDU length.**

- Source: `ieee802154-2024:clause:6.3.1`, physical PDF pp. 62–63; `ieee802154-2024@304032:305918`; Figure 6-1 on p. 63; aMaxSifsFrameSize in Table 8-35 on p. 152 (`ieee802154-2024@658233:659380`).
- Source excerpt: “Frames (i.e., MPDUs) of up to aMaxSifsFrameSize shall be followed by a short interfame space (SIFS) period of a duration of at least max(macSifsPeriod, aTurnaroundTime).”
- Strength: `shall`; observation class: `wire`.
- Condition: Successive transmissions; the same clause requires max(macLifsPeriod, aTurnaroundTime) for longer MPDUs. aMaxSifsFrameSize is 18 octets. The acknowledged exchange also obeys the AIFS lower bound and Figure 6-1's size-dependent interval after ACK.
- Check idea: Exercise complete 18- and 19-octet MPDUs, with and without acknowledgment, and observe the next actual transmission boundary. Use MPDU length including FCS, not MSDU length or PHY overhead.

## IEEE802154-SERVICE-9

**Unsupported or out-of-range MLME request parameters report INVALID_PARAMETER.**

- Source: `ieee802154-2024:clause:8.2.2`, physical PDF pp. 112–114; `ieee802154-2024@489336:494944`.
- Source excerpt: “If any parameter in the request primitive is not supported or is out of range, the MAC sublayer will issue the corresponding confirm primitive with a Status of INVALID_PARAMETER.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME request parameter support/range validation; preserve more specific primitive error predicates such as absent or read-only PIB attributes.
- Check idea: Set a present writable attribute to a valid but unsupported capability value and observe INVALID_PARAMETER, separately from absent-attribute, read-only, invalid-index and out-of-range cases.

## IEEE802154-SERVICE-8

**Incoming security errors generate a communication-status indication.**

- Source: `ieee802154-2024:table:8-5`, continuation of 8.2.4.4, physical PDF pp. 117–118; `ieee802154-2024@508417:512979`.
- Source excerpt: “The MLME-COMM-STATUS.indication primitive is generated by the MLME and issued to its next higher layer either following a transmission instigated through a response primitive or on receipt of a frame that generates an error in its security processing, as described in 9.2.4.”
- Strength: `description`; observation class: `error-signal`.
- Condition: A received frame generates an incoming-security error; the alternative response-primitive transmission branch has its own procedure predicates.
- Check idea: Observe the upper communication-status indication and its error separately from acknowledgment and ordinary data delivery. Early-return security outputs remain invalid unless explicitly set by 9.2.4.

## IEEE802154-ADDRESS-1

**Device extended identity uses EUI-64.**

- Source: `ieee802154-2024:clause:7.1`, physical PDF pp. 78–78; `ieee802154-2024@359535:359817`.
- Source excerpt: “A device’s extended address shall be an extended unique identifier (EUI-64), as defined by IEEE Std 802”
- Strength: `shall`; observation class: `encoding`.
- Condition: Device extended address.
- Check idea: Encode identities that differ only in the high 16 bits and verify they remain distinct.

## IEEE802154-ADDRESS-2

**Address octets are transmitted from rightmost to leftmost.**

- Source: `ieee802154-2024:clause:4.5.1`, physical PDF pp. 50–51; `ieee802154-2024@263734:264671`.
- Source excerpt: “For every address field, the bit transmission order shall be performed from the right most octet (RMO) to the left most octet (LMO) in the field, and inside an octet from the LSB to the MSB.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Every address field.
- Check idea: Check AC-DE-48-23-45-67-89-01 against octets 01 89 67 45 23 48 DE AC.

## IEEE802154-WIRE-1

**Legacy same-PAN dual addressing omits the source PAN.**

- Source: `ieee802154-2024:clause:7.2.2.6`, physical PDF pp. 80–80; `ieee802154-2024@367029:368560`.
- Source excerpt: “If the PAN IDs are identical, the PAN ID Compression field shall be set to one, and the Source PAN ID field shall be omitted from the transmitted frame.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Frame version 0 or 1; both addresses present and PAN IDs equal.
- Check idea: Enumerate short/extended address pairs and verify compression and exact offsets.

## IEEE802154-WIRE-2

**Legacy different-PAN dual addressing includes both PAN IDs.**

- Source: `ieee802154-2024:clause:7.2.2.6`, physical PDF pp. 80–80; `ieee802154-2024@367029:368560`.
- Source excerpt: “If the PAN IDs are different, the PAN ID Compression field shall be set to zero, and both Destination PAN ID field and Source PAN ID fields shall be included in the transmitted frame.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Frame version 0 or 1; both addresses present and PAN IDs different.
- Check idea: Use distinct PAN values and independently verify both two-octet fields.

## IEEE802154-WIRE-3

**Legacy single addressing includes its PAN without compression.**

- Source: `ieee802154-2024:clause:7.2.2.6`, physical PDF pp. 80–80; `ieee802154-2024@367029:368560`.
- Source excerpt: “If only either the destination or the source addressing information is present, the PAN ID Compression field shall be set to zero, and the PAN ID field of the single address shall be included in the transmitted frame.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Frame version 0 or 1; exactly one address present.
- Check idea: Exercise each direction with short and extended addresses.

## IEEE802154-WIRE-4

**Immediate ACKs have the Figure 7-15 layout.**

- Source: `ieee802154-2024:clause:7.3.3`, physical PDF pp. 91–91; `ieee802154-2024@403772:404594`.
- Source excerpt: “The Imm-Ack frame shall be formatted as illustrated in Figure 7-15.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Immediate ACK.
- Check idea: Verify FCF, DSN and the PHY-selected FCS with no address fields; inspect Figure 7-15 and continuation after Figure 7-16.

## IEEE802154-WIRE-5

**A data payload preserves the supplied octets.**

- Source: `ieee802154-2024:clause:7.3.2.3`, physical PDF pp. 91–91; `ieee802154-2024@403577:403772`.
- Source excerpt: “The payload of a Data frame shall contain the sequence of octets that the next higher layer has requested the MAC sublayer to transmit.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Data frame.
- Check idea: Send octets containing zeros and apparent header patterns and compare the received MSDU byte-for-byte.

## IEEE802154-WIRE-6

**LegacyTx selects version 1 data.**

- Source: `ieee802154-2024:table:8-30`, physical PDF pp. 144–146; `ieee802154-2024@625153:628609`, `ieee802154-2024@628609:632178`.
- Source excerpt: “If LegacyTx of TxOptions is TRUE, the Data frame sent shall use the Frame Version field set to 0b01 and use the IEEE Std 802.15.4-2006 format.”
- Strength: `shall`; observation class: `wire`.
- Condition: LegacyTx true; data transmission.
- Check idea: Observe the transmitted FCF version independently of the request representation.

## IEEE802154-ACCESS-1

**Initialize attempt-local backoff state.**

- Source: `ieee802154-2024:clause:6.3.2.1`, physical PDF pp. 63–64; `ieee802154-2024@306402:310168`.
- Source excerpt: “BE shall be initialized to the value of macMinBe.”
- Strength: `shall`; observation class: `internal`.
- Condition: New CSMA-CA attempt, including a direct retransmission.
- Check idea: Verify NB=0 and BE=macMinBe at each attempt, then follow Figure 6-2 on idle and busy CCA results.

## IEEE802154-ACCESS-2

**Nonperiodic-beacon transmission follows successful unslotted access.**

- Source: `ieee802154-2024:clause:6.6.1`, physical PDF pp. 69–70; `ieee802154-2024@325413:329415`.
- Source excerpt: “If the frame is to be transmitted on a PAN not using periodic beacons, the frame shall be transmitted following the successful application of the unslotted version of the CSMA-CA algorithm, as described in 6.3.2.”
- Strength: `shall`; observation class: `wire`.
- Condition: Data transmission in a non-beacon PAN.
- Check idea: Keep the medium busy through the limit and verify no data transmission; release it before exhaustion and observe transmission.

## IEEE802154-SEQUENCE-1

**Sequence allocation is device-wide.**

- Source: `ieee802154-2024:clause:6.6.1`, physical PDF pp. 69–70; `ieee802154-2024@325413:329415`.
- Source excerpt: “Each device shall generate exactly one Sequence Number regardless of the number of unique devices with which it wishes to communicate.”
- Strength: `shall`; observation class: `wire`.
- Condition: Generated frames using the same sequence space.
- Check idea: Alternate destinations and verify a single progressing DSN sequence.

## IEEE802154-SEQUENCE-2

**Sequence counters wrap at their representable maximum.**

- Source: `ieee802154-2024:clause:6.6.1`, physical PDF pp. 69–70; `ieee802154-2024@325413:329415`.
- Source excerpt: “The sequence numbers stored in the MAC PIB attributes shall roll over to zero after reaching the maximum value representable.”
- Strength: `shall`; observation class: `wire`.
- Condition: Sequence counter reaches its maximum.
- Check idea: Drive DSN through 254, 255, 0, 1 over a byte boundary and observe continued delivery.

## IEEE802154-RECEIVE-1

**An incorrect FCS causes discard.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “The MAC sublayer shall discard all received frames that do not contain a correct value in their frame check sequence (FCS) field in the MFR, as described in 7.2.11.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Received frames with an MFR.
- Check idea: Inject a one-bit corruption without repairing FCS; confirm the bytes arrived and that no MSDU or ACK is emitted.

## IEEE802154-RECEIVE-2

**A present destination PAN must match or be broadcast.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “If a destination PAN ID is included in the frame, it shall match macPanId or shall be the broadcast PAN ID.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Not scanning; destination PAN present.
- Check idea: Compare local, broadcast and foreign PAN frames with valid FCS and otherwise identical fields.

## IEEE802154-RECEIVE-3

**Valid legacy unicast AR frames require immediate ACK.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “the MAC sublayer shall send an Imm-Ack frame.”
- Strength: `shall`; observation class: `wire`.
- Condition: Valid nonbroadcast Data/MAC command, version 0 or 1, AR set; full predicate in 6.6.2.
- Check idea: Compare normal delivery and a well-formed secured frame rejected by incoming security; check ACK independently of delivery.

## IEEE802154-RECEIVE-4

**Data delivery requires successful incoming security processing.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “If the valid frame is a Data frame or Multipurpose frame and the status from the incoming frame security procedure is SUCCESS, the MAC sublayer shall pass the MAC service data unit (MSDU) to the next higher layer.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Valid data or multipurpose frame.
- Check idea: Verify successful unsecured delivery and suppression of payload delivery on security failure.

## IEEE802154-ACK-1

**Broadcast and group-addressed frames do not request ACK.**

- Source: `ieee802154-2024:clause:6.6.3.1`, physical PDF pp. 72–72; `ieee802154-2024@338546:339023`.
- Source excerpt: “any frame that is broadcast or has a group address as the extended destination address, as defined in IEEE Std 802, shall be sent with its AR field set to indicate no acknowledgment requested.”
- Strength: `shall`; observation class: `wire`.
- Condition: Broadcast/group destination.
- Check idea: Observe AR=0 and no resulting ACK for a broadcast data request.

## IEEE802154-ACK-2

**AR-clear frames receive no ACK.**

- Source: `ieee802154-2024:clause:6.6.3.2`, physical PDF pp. 72–72; `ieee802154-2024@339023:340044`.
- Source excerpt: “shall not be acknowledged by its target.”
- Strength: `shall`; observation class: `wire`.
- Condition: Frame AR=0; full predicate in 6.6.3.2.
- Check idea: Send addressed AR-clear data, observe delivery and successful transmission completion without an ACK.

## IEEE802154-ACK-3

**Outside-CAP ACK start is measured from the received last symbol.**

- Source: `ieee802154-2024:clause:6.6.3.3`, physical PDF pp. 72–73; `ieee802154-2024@340121:342816`.
- Source excerpt: “The transmission of an Ack frame outside the CAP shall commence AIFS after the reception of the last symbol of the Data frame or MAC command.”
- Strength: `shall`; observation class: `wire`.
- Condition: ACK outside CAP.
- Check idea: For O-QPSK use AIFS=macSifsPeriod and measure from received PPDU end to ACK PPDU start.

## IEEE802154-ACK-4

**Direct retransmissions retain the DSN.**

- Source: `ieee802154-2024:clause:6.6.3.4`, physical PDF pp. 73–73; `ieee802154-2024@343219:345787`.
- Source excerpt: “The retransmitted frame shall contain the same DSN as was used in the original transmission.”
- Strength: `shall`; observation class: `wire`.
- Condition: Failed direct acknowledged transmission.
- Check idea: Suppress ACKs, inspect every retry, and verify at most macMaxFrameRetries additional transmissions.

## IEEE802154-ACK-5

**Failed indirect transmission waits for a new poll.**

- Source: `ieee802154-2024:clause:6.6.3.4`, physical PDF pp. 73–73; `ieee802154-2024@343219:345787`.
- Source excerpt: “If a single transmission attempt has failed and the transmission was indirect, the coordinator shall not retransmit the frame.”
- Strength: `shall`; observation class: `wire`.
- Condition: Indirect delivery attempt fails.
- Check idea: Verify no autonomous retry, then request again and verify the same pending frame/DSN.

## IEEE802154-SECURITY-1

**Security level zero returns the outgoing frame unchanged.**

- Source: `ieee802154-2024:clause:9.2.2`, physical PDF pp. 160–162; `ieee802154-2024@697713:703495`.
- Source excerpt: “If the SecurityLevel parameter is zero, the procedure shall set the secured frame to be the frame to be secured and return with a Status of SUCCESS.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Outgoing SecurityLevel=0.
- Check idea: Observe unchanged unsecured frame and successful policy result.

## IEEE802154-SECURITY-2

**A nonzero outgoing security request fails when security is disabled.**

- Source: `ieee802154-2024:clause:9.2.2`, physical PDF pp. 160–162; `ieee802154-2024@697713:703495`.
- Source excerpt: “If macSecurityEnabled is set to FALSE, the procedure shall return with a Status of UNSUPPORTED_SECURITY.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Outgoing SecurityLevel is nonzero; step (a) did not return.
- Check idea: Request unavailable security, observe UNSUPPORTED_SECURITY and no frame transmission.

## IEEE802154-SECURITY-3

**Incoming secured version zero reports unsupported legacy security.**

- Source: `ieee802154-2024:clause:9.2.4`, physical PDF pp. 163–163; `ieee802154-2024@707731:710227`.
- Source excerpt: “If the Frame Version field of the frame to be unsecured is set to zero, the procedure shall return with a Status of UNSUPPORTED_LEGACY.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Security Enabled=1, Frame Version=0.
- Check idea: Inject a valid addressed legacy secured frame; distinguish status from version-1 UNSUPPORTED_SECURITY.

## IEEE802154-SECURITY-4

**Incoming nonlegacy secured frames fail when security is disabled.**

- Source: `ieee802154-2024:clause:9.2.4`, physical PDF pp. 163–163; `ieee802154-2024@707731:710227`.
- Source excerpt: “If macSecurityEnabled is set to FALSE, the procedure shall return with a Status of UNSUPPORTED_SECURITY.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Security Enabled=1, version nonzero, security disabled.
- Check idea: Check the status before auxiliary-header/key lookup and prevent payload delivery.

## IEEE802154-SECURITY-5

**Unsecured input succeeds when security is disabled.**

- Source: `ieee802154-2024:clause:9.2.5`, physical PDF pp. 165–166; `ieee802154-2024@718207:722316`.
- Source excerpt: “If macSecurityEnabled is set to FALSE, the procedure shall set the validated frame to be the frame to be validated and return with a Status of SUCCESS.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Security Enabled=0 and macSecurityEnabled=false.
- Check idea: Verify that no key or device table is needed for accepted unsecured data.

## IEEE802154-PHY-1

**CCA Mode 1 detects energy above its threshold.**

- Source: `ieee802154-2024:clause:11.2.8`, physical PDF pp. 596–597; `ieee802154-2024@2373831:2378773`.
- Source excerpt: “CCA shall report a busy medium upon detecting any energy above the ED threshold.”
- Strength: `shall`; observation class: `internal`.
- Condition: CCA Mode 1.
- Check idea: Inject energy without a decodable frame; sweep power on both sides of the configured threshold.

## IEEE802154-PHY-2

**A CCA request during PPDU reception reports busy.**

- Source: `ieee802154-2024:clause:11.2.8`, physical PDF pp. 596–597; `ieee802154-2024@2373831:2378773`.
- Source excerpt: “For any of the CCA modes, if a request to perform CCA is received by the PHY during reception of a PPDU, CCA shall report a busy medium.”
- Strength: `shall`; observation class: `internal`.
- Condition: Reception interval from detected SFD through decoded PHR octet count.
- Check idea: Request CCA inside that interval and immediately outside it; distinguish reception from generic carrier state.

## IEEE802154-PHY-3

**CCA observes for phyCcaDuration.**

- Source: `ieee802154-2024:clause:11.2.8`, physical PDF pp. 596–597; `ieee802154-2024@2373831:2378773`.
- Source excerpt: “The CCA detection time shall be equal to phyCcaDuration, as defined in Table 12-2.”
- Strength: `shall`; observation class: `internal`.
- Condition: CCA operation.
- Check idea: Measure actual observation start to completion and test a nonintegral number of symbols.

## IEEE802154-PHY-4

**ED averages over the selected measurement interval.**

- Source: `ieee802154-2024:clause:11.2.6`, physical PDF pp. 596–596; `ieee802154-2024@2372030:2373030`.
- Source excerpt: “The ED measurement time, to average over, shall be equal to eight symbol periods, unless the PHY specifies shorter phyCcaDuration time, in which case it is used instead.”
- Strength: `shall`; observation class: `internal`.
- Condition: Receiver ED.
- Check idea: Apply changing power within the interval and verify an average, not an instantaneous reception-state result.

## IEEE802154-PHY-5

**LQI uses at least eight values.**

- Source: `ieee802154-2024:clause:11.2.7`, physical PDF pp. 596–596; `ieee802154-2024@2373030:2373831`.
- Source excerpt: “At least eight unique values of LQI shall be used.”
- Strength: `shall`; observation class: `internal`.
- Condition: Received packet LQI.
- Check idea: Vary received signal quality and verify per-packet LQI with at least eight representable output levels.

## IEEE802154-PHY-6

**O-QPSK preamble is eight zero symbols.**

- Source: `ieee802154-2024:clause:13.1.2.2`, physical PDF pp. 614–614; `ieee802154-2024@2450789:2450993`.
- Source excerpt: “The length of the Preamble field for the O-QPSK PHYs shall be 8 symbols (i.e., 4 octets), and the bits in the Preamble field shall be binary zeros.”
- Strength: `shall`; observation class: `encoding`.
- Condition: O-QPSK PPDU.
- Check idea: Check SHR duration and preamble bytes independently of the MAC length.

## IEEE802154-PHY-7

**PHR length counts the complete PSDU.**

- Source: `ieee802154-2024:clause:13.1.3.2`, physical PDF pp. 615–615; `ieee802154-2024@2452100:2452253`.
- Source excerpt: “The Frame Length field specifies the total number of octets contained in the PSDU (i.e., PHY payload).”
- Strength: `description`; observation class: `encoding`.
- Condition: O-QPSK PHR.
- Check idea: Compare PHR length to MAC header plus payload plus FCS at minimum and maximum sizes.

## IEEE802154-PHY-8

**2450 MHz O-QPSK operates at 250 kbit/s.**

- Source: `ieee802154-2024:clause:13.2.2`, physical PDF pp. 615–615; `ieee802154-2024@2452853:2453503`.
- Source excerpt: “The data rate of the O-QPSK PHY shall be 250 kb/s when operating in the 2450 MHz, 915 MHz, 780 MHz or 2380 MHz bands”
- Strength: `shall`; observation class: `wire`.
- Condition: 2450 MHz O-QPSK.
- Check idea: Use 4 bits/symbol from 13.2.4 to derive 16 microseconds/symbol and compare PPDU duration.

## IEEE802154-SERVICE-1

**Oversize MPDUs fail before transmission.**

- Source: `ieee802154-2024:table:8-31`, physical PDF pp. 146–148; `ieee802154-2024@633380:637006`, `ieee802154-2024@637006:643870`.
- Source excerpt: “If the length of the frame exceeds phyMaxPacketSize (e.g., due to the additional overhead required for security processing or additional IEs), the MAC sublayer shall discard the frame and issue the MCPS- DATA.confirm primitive with a Status of FRAME_TOO_LONG.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Generated MPDU exceeds the selected PHY maximum.
- Check idea: Compare requests at the exact payload capacity and one octet above it; inspect status and absence of transmission.

## IEEE802154-PIB-1

**Dagger-marked MAC attributes are read-only to the upper layer.**

- Source: `ieee802154-2024:clause:8.4.3.1`, physical PDF pp. 152–152; `ieee802154-2024@659416:660132`.
- Source excerpt: “Attributes marked with a dagger (†) are read-only attributes (i.e., attribute can only be set by the MAC sublayer)”
- Strength: `description`; observation class: `internal`.
- Condition: MAC PIB attribute marked with dagger.
- Check idea: Attempt MLME-SET and verify READ_ONLY without mutation; GET still reports the value.

## IEEE802154-PIB-2

**Unknown PIB reads return unsupported attribute.**

- Source: `ieee802154-2024:table:8-9`, physical PDF pp. 120–121; `ieee802154-2024@522591:524416`.
- Source excerpt: “If the identifier of the PIB attribute is not found, the primitive returns with a Status of UNSUPPORTED_ATTRIBUTE.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME-GET for an absent attribute.
- Check idea: Verify UNSUPPORTED_ATTRIBUTE and do not consume the invalid returned value.

## IEEE802154-PIB-3

**Reset can preserve or restore MAC PIB values.**

- Source: `ieee802154-2024:table:8-12`, physical PDF pp. 123–123; `ieee802154-2024@530664:531472`.
- Source excerpt: “If FALSE, the MAC sublayer is reset, but all MAC PIB attributes retain their values prior to the generation of the MLME-RESET.request primitive.”
- Strength: `description`; observation class: `internal`.
- Condition: SetDefaultPib false; compare with true as defined in the same table.
- Check idea: Set nondefault attributes, reset in both modes, then read them back; check completion and PHY reset.

## IEEE802154-SCAN-1

**All devices support passive scan.**

- Source: `ieee802154-2024:clause:6.4.1.1`, physical PDF pp. 64–65; `ieee802154-2024@310297:311546`.
- Source excerpt: “All devices shall be capable of performing passive scan across a specified set of channels.”
- Strength: `shall`; observation class: `wire`.
- Condition: All devices, including static non-beacon operation.
- Check idea: Request a passive scan across multiple channels and observe discovery without active scan requests.

## IEEE802154-SCAN-2

**Scan visits channels in ascending order.**

- Source: `ieee802154-2024:clause:6.4.1.1`, physical PDF pp. 64–65; `ieee802154-2024@310297:311546`.
- Source excerpt: “Channels are scanned in order from the lowest channel number to the highest.”
- Strength: `description`; observation class: `internal`.
- Condition: Channel scan.
- Check idea: Submit an unordered channel set and observe ascending visits and terminal results.

## IEEE802154-WIRE-7

**Legacy frames cannot suppress their sequence number.**

- Source: `ieee802154-2024:clause:7.2.2.7`, physical PDF pp. 81–81; `ieee802154-2024@371560:371926`.
- Source excerpt: “If the Frame Version field is 0b00 or 0b01, the Sequence Number Suppression field shall be zero.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Frame version 0 or 1.
- Check idea: Check the bit and one-octet DSN presence; do not use version-2 offsets for a malformed legacy combination.

## IEEE802154-WIRE-8

**Legacy frames cannot advertise IEs.**

- Source: `ieee802154-2024:clause:7.2.2.8`, physical PDF pp. 81–81; `ieee802154-2024@371926:372182`.
- Source excerpt: “If the Frame Version field is 0b00 or 0b01, the IE Present field shall be zero.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Frame version 0 or 1.
- Check idea: Verify transmitted bit zero and distinguish unsupported enhanced frames from invalid legacy IE flags.

## IEEE802154-WIRE-9

**Destination addressing mode must not be reserved.**

- Source: `ieee802154-2024:clause:7.2.2.9`, physical PDF pp. 81–81; `ieee802154-2024@372182:372371`.
- Source excerpt: “The Destination Addressing Mode field shall be set to one of the non-reserved values listed in Table 7-3.”
- Strength: `shall`; observation class: `encoding`.
- Condition: General FCF destination mode.
- Check idea: Enumerate mode 00, 10 and 11; do not treat reserved 01 as a two- or eight-octet address.

## IEEE802154-WIRE-10

**Legacy source omission implies a present destination.**

- Source: `ieee802154-2024:clause:7.2.2.11`, physical PDF pp. 82–82; `ieee802154-2024@375465:375983`.
- Source excerpt: “the Destination Addressing Mode field shall be nonzero,”
- Strength: `shall`; observation class: `encoding`.
- Condition: Source mode zero in version-0/1 data or MAC command; full predicate in 7.2.2.11.
- Check idea: Generate destination-only traffic from a PAN coordinator; reject local legacy data requests omitting both addresses.

## IEEE802154-WIRE-11

**Legacy destination omission implies a present source.**

- Source: `ieee802154-2024:table:7-3`, physical PDF pp. 81–82; `ieee802154-2024@372371:373776`.
- Source excerpt: “the Source Addressing Mode field shall be nonzero,”
- Strength: `shall`; observation class: `encoding`.
- Condition: Destination mode zero in version-0/1 data or MAC command; continuation of 7.2.2.9 after Table 7-3.
- Check idea: Generate source-only traffic to the PAN coordinator; verify source PAN field and no destination fields.

## IEEE802154-WIRE-12

**Security control and auxiliary-header presence are related.**

- Source: `ieee802154-2024:clause:7.2.2.3`, physical PDF pp. 79–80; `ieee802154-2024@365417:365949`.
- Source excerpt: “The Auxiliary Security Header field of the MHR shall be present only if the Security Enabled field is set to one.”
- Strength: `shall`; observation class: `encoding`.
- Condition: General MAC header.
- Check idea: Ensure unsecured legacy payload is not parsed as an auxiliary security header.

## IEEE802154-WIRE-13

**FCS covers MAC header and payload.**

- Source: `ieee802154-2024:clause:7.2.11`, physical PDF pp. 84–85; `ieee802154-2024@380293:384026`.
- Source excerpt: “The FCS is calculated over the MHR and MAC payload parts of the frame; these parts together are referred to as the calculation field.”
- Strength: `description`; observation class: `encoding`.
- Condition: Frame has an FCS.
- Check idea: Change one MHR bit and one payload bit separately; PHY preamble and PHR are outside CRC coverage.

## IEEE802154-WIRE-14

**Two-octet FCS uses the specified polynomial division.**

- Source: `ieee802154-2024:clause:7.2.11`, physical PDF pp. 84–85; `ieee802154-2024@380293:384026`.
- Source excerpt: “The 2-octet FCS shall be calculated for transmission using the following algorithm:”
- Strength: `shall`; observation class: `encoding`.
- Condition: Two-octet FCS; full polynomial/bit order in 7.2.11.
- Check idea: Divide x^16 times the transmitted-bit polynomial by x^16+x^12+x^5+1. Verify the source ACK example 02 00 6A yields E4 79.

## IEEE802154-WIRE-15

**Immediate ACK FCF contains only its permitted fields.**

- Source: `ieee802154-2024:figure:7-16`, physical PDF pp. 91–92; `ieee802154-2024@406235:409068`.
- Source excerpt: “In an Imm-Ack frame, all other fields in the Frame Control field shall be set to zero.”
- Strength: `shall`; observation class: `encoding`.
- Condition: Immediate ACK; frame type, pending and AR are described in the preceding 7.3.3 continuation.
- Check idea: For ordinary data outside CSL, verify FCF 0x0002, no addressing/security/compression and AR zero; copy received DSN.

## IEEE802154-WIRE-16

**Frame Pending is clear outside its specified mechanisms.**

- Source: `ieee802154-2024:clause:7.2.2.4`, physical PDF pp. 80–80; `ieee802154-2024@365949:366572`.
- Source excerpt: “At all other times, the frame pending bit shall be set to zero on transmission and ignored on reception.”
- Strength: `shall`; observation class: `wire`.
- Condition: Not indirect, LE CSL or applicable TSCH pending behavior.
- Check idea: Transmit static direct data with pending clear; do not start a polling exchange merely from an irrelevant received pending bit.

## IEEE802154-RECEIVE-5

**Reserved frame types fail filtering.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “The Frame Type field shall not contain a reserved frame type.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Not scanning; apply Table 7-1 classifications.
- Check idea: Inject type 100 with valid FCS and verify no normal delivery or ACK; distinguish other unsupported but assigned types.

## IEEE802154-RECEIVE-6

**Reserved frame versions fail filtering.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “The Frame Version field shall not contain a reserved value.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Not scanning; interpret version within its frame-type format.
- Check idea: Inject legacy-layout data with version 3 and verify rejection; do not apply this bit location to formats without the field.

## IEEE802154-RECEIVE-7

**Short destination acceptance includes local and broadcast addresses.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “A short destination address is included in the frame, and it matches either macShortAddress or the broadcast address.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Short addressing, non-scan validity predicate 6.6.2(d)(1).
- Check idea: Compare local, 0xFFFF and foreign short destinations with matching PAN and correct FCS.

## IEEE802154-RECEIVE-8

**Extended destination acceptance includes enabled group addresses.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “An extended destination address is included in the frame and matches either macExtendedAddress or, if macGroupRxMode is set to TRUE, a 64-bit group address, as defined in IEEE Std 802.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Extended addressing, predicate 6.6.2(d)(2).
- Check idea: Test exact EUI-64 equality, a high-bit mismatch and group reception with the feature disabled/enabled.

## IEEE802154-RECEIVE-9

**Implicit broadcast is an explicit receive predicate.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “The Destination Address field and the Destination PAN ID field are not included in the frame. and macImplicitBroadcast is TRUE.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Predicate 6.6.2(d)(3); quoted punctuation follows the source.
- Check idea: Use a structurally legal source-only legacy frame; compare implicit-broadcast false and true independently of PAN-coordinator acceptance.

## IEEE802154-RECEIVE-10

**PAN coordinators accept matching source-only data.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “The device is the PAN coordinator, only source addressing fields are included in a Data frame or MAC command and the source PAN ID matches macPanId.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Predicate 6.6.2(d)(4).
- Check idea: Send identical source-only legacy data to coordinator and noncoordinator receivers, with implicit broadcast disabled; vary source PAN.

## IEEE802154-RECEIVE-11

**An omitted source PAN can be inferred from destination PAN.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “If the Source PAN ID field is not included in the frame and the Destination PAN ID field is included in the frame, the MAC sublayer shall use the value of the Destination PAN ID field as the source PAN ID.”
- Strength: `shall`; observation class: `internal`.
- Condition: Source PAN absent and destination PAN present.
- Check idea: Verify effective source PAN in filtering/security context separately from whether an indication field is wire-present/valid.

## IEEE802154-RECEIVE-12

**Idle receiver control is restored after a transceiver task.**

- Source: `ieee802154-2024:clause:6.6.2`, physical PDF pp. 70–72; `ieee802154-2024@329415:338486`.
- Source excerpt: “On completion of each transceiver task, the MAC sublayer shall request that the PHY enables or disables its receiver, depending on the values of macBeaconOrder and macRxOnWhenIdle.”
- Strength: `shall`; observation class: `internal`.
- Condition: Transceiver task completion; BO=15 makes idle policy relevant at all times.
- Check idea: With idle reception false, transmit with ACK requested and observe receiver enabled for the ACK then restored; repeat with true.

## IEEE802154-RECEIVE-13

**Promiscuous entry enables reception.**

- Source: `ieee802154-2024:clause:10.23.1`, physical PDF pp. 379–380; `ieee802154-2024@1491164:1492266`.
- Source excerpt: “If the MLME is requested to set macPromiscuousMode to TRUE, the MLME shall then request that the PHY enable its receiver.”
- Strength: `shall`; observation class: `internal`.
- Condition: Optional promiscuous mode is supported and enabled.
- Check idea: Enter with macRxOnWhenIdle=false and observe the receiver-enable request.

## IEEE802154-RECEIVE-14

**Promiscuous indications contain MAC header plus payload.**

- Source: `ieee802154-2024:clause:10.23.1`, physical PDF pp. 379–380; `ieee802154-2024@1491164:1492266`.
- Source excerpt: “The Msdu parameter shall contain the MHR concatenated with the MAC payload, as illustrated in Figure 7-1.”
- Strength: `shall`; observation class: `end-to-end`.
- Condition: Promiscuous indication; only Msdu, MpduLinkQuality, Timestamp and Rssi are valid, as the preceding sentence specifies.
- Check idea: Verify raw MHR+payload excludes FCS; do not interpret DSN/address/AckSent parameters as valid ordinary indications.

## IEEE802154-RECEIVE-15

**Promiscuous exit restores the idle receiver policy.**

- Source: `ieee802154-2024:clause:10.23.1`, physical PDF pp. 379–380; `ieee802154-2024@1491164:1492266`.
- Source excerpt: “If the MLME is requested to set macPromiscuousMode to FALSE, the MLME shall request that the PHY set its receiver to the state specified by macRxOnWhenIdle.”
- Strength: `shall`; observation class: `internal`.
- Condition: Optional promiscuous mode is disabled.
- Check idea: Exit under both idle policies and verify receiver state without changing the stored idle attribute.

## IEEE802154-PHY-9

**TX-to-RX readiness is bounded by turnaround.**

- Source: `ieee802154-2024:clause:11.2.2`, physical PDF pp. 594–595; `ieee802154-2024@2368024:2368734`.
- Source excerpt: “The TX-to-RX turnaround time shall be less than or equal to aTurnaroundTime, as defined in Table 12-1.”
- Strength: `shall`; observation class: `wire`.
- Condition: At the air interface from last transmitted part/chip to readiness for the next received first part/chip.
- Check idea: For O-QPSK bound readiness by 12 symbols; do not add turnaround again after an already inclusive ACK wait.

## IEEE802154-PHY-10

**RX-to-TX turnaround is bounded.**

- Source: `ieee802154-2024:clause:11.2.3`, physical PDF pp. 595–595; `ieee802154-2024@2368734:2369197`.
- Source excerpt: “The RX-to-TX turnaround time shall be less than or equal to aTurnaroundTime, as defined in Table 12-1.”
- Strength: `shall`; observation class: `wire`.
- Condition: Air-interface turnaround as defined in 11.2.3.
- Check idea: Verify selected O-QPSK transition supports the 12-symbol immediate-ACK start rule; distinguish request completion from PPDU start.

## IEEE802154-PHY-11

**ED zero identifies sufficiently low power.**

- Source: `ieee802154-2024:clause:11.2.6`, physical PDF pp. 596–596; `ieee802154-2024@2372030:2373030`.
- Source excerpt: “The minimum ED value (zero) shall indicate received power less than 10 dB above the lowest specified receiver sensitivity, in dBm, for the PHY.”
- Strength: `shall`; observation class: `internal`.
- Condition: Receiver ED.
- Check idea: Declare the ED transfer function and verify that every zero result denotes power below the lowest specified PHY sensitivity plus 10 dB; do not infer an exact universal zero/nonzero threshold.

## IEEE802154-PHY-12

**ED spans at least 40 dB with linear decibel mapping.**

- Source: `ieee802154-2024:clause:11.2.6`, physical PDF pp. 596–596; `ieee802154-2024@2372030:2373030`.
- Source excerpt: “The range of received power spanned by the ED values shall be at least 40 dB.”
- Strength: `shall`; observation class: `internal`.
- Condition: Receiver ED; the next sentence also requires linear dB mapping with ±6 dB accuracy.
- Check idea: Sweep at least 40 dB; compare the declared transfer curve, tolerance and saturation without assuming linear watts.

## IEEE802154-PHY-13

**LQI is measured for every received packet.**

- Source: `ieee802154-2024:clause:11.2.7`, physical PDF pp. 596–596; `ieee802154-2024@2373030:2373831`.
- Source excerpt: “The LQI measurement shall be performed for each received packet.”
- Strength: `shall`; observation class: `internal`.
- Condition: Packet reception.
- Check idea: Send multiple distinct packets at different qualities and verify each indication carries that packet's measurement.

## IEEE802154-PHY-14

**2450 MHz channels use the specified center frequencies.**

- Source: `ieee802154-2024:clause:11.1.3.3`, physical PDF pp. 566–566; `ieee802154-2024@2259908:2260971`.
- Source excerpt: “fc = 2405 + 5 (k – 11) in megahertz, for k = 11, 12, …, 26”
- Strength: `description`; observation class: `internal`.
- Condition: Selected 2450 MHz O-QPSK band; not the excluded SUN/LECIM/MSK channel assignments.
- Check idea: Check every channel 11–26; endpoints are 2405 and 2480 MHz.

## IEEE802154-PHY-15

**Supported PHY channels include those allowed in the operating region.**

- Source: `ieee802154-2024:clause:11.1.3.1`, physical PDF pp. 565–566; `ieee802154-2024@2258178:2259440`.
- Source excerpt: “For each PHY supported, a compliant device shall support all channels allowed by regulations for the region in which the device operates.”
- Strength: `shall`; observation class: `internal`.
- Condition: PHY/channel support; subsequent HRP/LRP exceptions are outside O-QPSK.
- Check idea: Declare the operating channel set; do not infer regulatory permission from the model's 16-channel capability.

## IEEE802154-PIB-4

**PHY PIB read-only markers restrict upper writes.**

- Source: `ieee802154-2024:clause:12.3.1`, physical PDF pp. 601–601; `ieee802154-2024@2388544:2389020`.
- Source excerpt: “Attributes marked with a dagger (†) are read-only attributes (i.e., attribute can only be set by the PHY),”
- Strength: `description`; observation class: `internal`.
- Condition: PHY PIB attributes marked dagger.
- Check idea: Check PHY-owned read-only values through management services; keep their authority in the PHY.

## IEEE802154-PIB-5

**Read-only writes report READ_ONLY.**

- Source: `ieee802154-2024:table:8-11`, physical PDF pp. 122–122; `ieee802154-2024@527680:529776`.
- Source excerpt: “READ_ONLY: The PibAttribute parameter specifies an attribute that is a read-only attribute.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME-SET selecting a read-only attribute.
- Check idea: Try to overwrite the native extended address through MLME-SET; verify status and unchanged identity.

## IEEE802154-PIB-6

**Unknown writes report UNSUPPORTED_ATTRIBUTE.**

- Source: `ieee802154-2024:table:8-11`, physical PDF pp. 122–122; `ieee802154-2024@527680:529776`.
- Source excerpt: “UNSUPPORTED_ATTRIBUTE: The PibAttribute parameter specifies an attribute that was not found in the database.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME-SET selecting an absent attribute.
- Check idea: Verify no silent insertion or fallback parameter mutation.

## IEEE802154-PIB-7

**Out-of-range writes report INVALID_PARAMETER.**

- Source: `ieee802154-2024:table:8-11`, physical PDF pp. 122–122; `ieee802154-2024@527680:529776`.
- Source excerpt: “INVALID_PARAMETER: The PibAttributeValue parameter specifies a value that is out of the valid range for the given attribute.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME-SET range failure.
- Check idea: Probe range endpoints and neighbors and verify the previous value remains observable.

## IEEE802154-PIB-8

**Invalid hierarchical indices have a distinct status.**

- Source: `ieee802154-2024:table:8-11`, physical PDF pp. 122–122; `ieee802154-2024@527680:529776`.
- Source excerpt: “INVALID_INDEX: The index inside the hierarchical values in PibAttribute is out of range.”
- Strength: `description`; observation class: `error-signal`.
- Condition: Supported hierarchical PIB attribute with out-of-range index.
- Check idea: Distinguish an absent attribute from a present attribute with an invalid element index.

## IEEE802154-PIB-9

**Reset success is reported on completion.**

- Source: `ieee802154-2024:table:8-13`, physical PDF pp. 123–123; `ieee802154-2024@531952:532329`.
- Source excerpt: “The Status parameter is set to SUCCESS on completion of the reset procedure.”
- Strength: `description`; observation class: `error-signal`.
- Condition: MLME-RESET.
- Check idea: Check successful confirm follows completed reset, rather than initial request acceptance.

## IEEE802154-SERVICE-2

**Legacy data cannot omit both addresses.**

- Source: `ieee802154-2024:table:8-31`, physical PDF pp. 146–148; `ieee802154-2024@633380:637006`, `ieee802154-2024@637006:643870`.
- Source excerpt: “the Status shall be set to INVALID_ADDRESS.”
- Strength: `shall`; observation class: `error-signal`.
- Condition: Generated version-0/1 data with both SrcAddrMode and DstAddrMode NONE; full sentence in Table 8-31 continuation.
- Check idea: Submit the invalid address combination and verify no frame transmission and correct confirm handle.

## IEEE802154-SERVICE-3

**Direct retry exhaustion reports NO_ACK.**

- Source: `ieee802154-2024:table:8-31`, physical PDF pp. 146–148; `ieee802154-2024@633380:637006`, `ieee802154-2024@637006:643870`.
- Source excerpt: “it will discard the MSDU and issue the MCPS-DATA.confirm primitive with a Status of NO_ACK.”
- Strength: `description`; observation class: `error-signal`.
- Condition: Direct request with no acknowledgment after macMaxFrameRetries retransmissions.
- Check idea: Drop ACKs while observing actual transmissions and distinguish this outcome from busy-channel access failure.

## IEEE802154-SERVICE-4

**Direct access exhaustion reports CHANNEL_ACCESS_FAILURE.**

- Source: `ieee802154-2024:table:8-31`, physical PDF pp. 146–148; `ieee802154-2024@633380:637006`, `ieee802154-2024@637006:643870`.
- Source excerpt: “the MAC sublayer will discard the MSDU, and the Status will be set to CHANNEL_ACCESS_FAILURE.”
- Strength: `description`; observation class: `error-signal`.
- Condition: Direct request; CSMA-CA failed due to channel conditions.
- Check idea: Keep CCA busy through exhaustion and verify no frame and exactly the associated failed confirm.

## IEEE802154-SERVICE-5

**Indications expose received DSN when present.**

- Source: `ieee802154-2024:table:8-32`, physical PDF pp. 149–150; `ieee802154-2024@646089:651055`, `ieee802154-2024@651055:653714`.
- Source excerpt: “The DSN of the received Data frame if one was present.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Ordinary data indication; Dsn row, not promiscuous mode.
- Check idea: Match the indicated DSN to serialized received octets through wrap.

## IEEE802154-SERVICE-6

**AckSent indicates an ACK actually sent.**

- Source: `ieee802154-2024:table:8-32`, physical PDF pp. 149–150; `ieee802154-2024@646089:651055`, `ieee802154-2024@651055:653714`.
- Source excerpt: “TRUE if the received frame requested an acknowledgment that has been sent, FALSE otherwise.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Ordinary MCPS-DATA.indication AckSent field.
- Check idea: Delay or interrupt ACK transmission and verify the indication does not equate AR=1 with completed ACK emission.

## IEEE802154-SERVICE-7

**O-QPSK service DataRate uses the default selector.**

- Source: `ieee802154-2024:table:8-29`, physical PDF pp. 142–143; `ieee802154-2024@617271:620339`, `ieee802154-2024@620339:623550`.
- Source excerpt: “For all other PHYs, the parameter is set to zero.”
- Strength: `description`; observation class: `internal`.
- Condition: DataRate selector for a PHY not enumerated in the preceding list, including ordinary O-QPSK.
- Check idea: Use DataRate=0 for 250 kbit/s O-QPSK; do not place 250000 into the selector field.

## IEEE802154-WIRE-17

**Reserved fields are zero on transmission and ignored on reception.**

- Source: `ieee802154-2024:clause:4.6`, physical PDF pp. 51–52; `ieee802154-2024@267001:267498`.
- Source excerpt: “Each bit within any Reserved field shall be set to zero on transmission and shall be ignored on reception.”
- Strength: `shall`; observation class: `encoding`.
- Condition: A field designated Reserved; distinguish from specific frame-type/version rejection rules in 6.6.2.
- Check idea: Set FCF reserved bit 7 on an otherwise valid received frame and repair FCS; verify identical filtering, ACK and payload behavior.

## IEEE802154-RECEIVE-16

**Promiscuous mode accepts received frames for monitor delivery.**

- Source: `ieee802154-2024:table:10-114`, physical PDF pp. 380–380; `ieee802154-2024@1492433:1493180`.
- Source excerpt: “A value of TRUE indicates that the MAC sublayer accepts all frames received from the PHY.”
- Strength: `description`; observation class: `end-to-end`.
- Condition: Optional macPromiscuousMode=true, with 10.23.1 requiring correctly received frames and processing per 6.6.2.
- Check idea: Compare foreign-address monitoring with ordinary filtering; keep ACK eligibility and monitor acceptance distinct and record the interpretation of the cross-reference.

## PIB field domains

These are source-table data for the validation/access/reset checks above, not a selected-model
attribute list. The MAC source is `ieee802154-2024:table:8-36`, PDF pp. 152–156, locators
`ieee802154-2024@660132:662629`, `@662629:668924`, `@668924:674671`,
`@674671:680445`, `@680445:682982`. The dagger/asterisk meanings come from 8.4.3.1:
read-only to the upper layer, and optional, respectively. A dash is unspecified, not zero.

| Attribute | Type | Range | Default | Marker |
|---|---|---|---|---|
| `macAoaEnable` | Boolean | TRUE, FALSE | FALSE | — |
| `macAutoRequest` | Boolean | TRUE, FALSE | TRUE | — |
| `macBattLifeExt` | Boolean | TRUE, FALSE | FALSE | — |
| `macBattLifeExtPeriods` | Integer | 6–41 | Dependent on currently selected PHY | — |
| `macBeaconOrder` | Integer | 0–15 | 15 | `*` optional |
| `macBeaconPayload` | Set of octets | — | NULL | `*` optional |
| `macBsn` | Integer | 0x00–0xff | Random value from within the range | `*` optional |
| `macCoordExtendedAddress` | IEEE address | An extended IEEE address | — | — |
| `macCoordShortAddress` | Integer | 0x0000–0xffff | 0xffff | — |
| `macDsn` | Integer | 0x00–0xff | Random value from within the range | — |
| `macExtendedAddress` | IEEE address | Device specific | — | `†` read-only |
| `macFcsType` | Integer | 0–1 | 0 | — |
| `macGroupRxMode` | Boolean | TRUE, FALSE | FALSE | — |
| `macGtsPermit` | Boolean | TRUE, FALSE | TRUE | `*` optional |
| `macImplicitBroadcast` | Boolean | TRUE, FALSE | FALSE | — |
| `macLifsPeriod` | Integer | As defined in 11.1.4 | PHY dependent | `†` read-only |
| `macMaxBe` | Integer | 3–8 | 5 | — |
| `macMaxCsmaBackoffs` | Integer | 0–5 | 4 | — |
| `macMaxFrameRetries` | Integer | 0–7 | 3 | — |
| `macNotifyAllBeacons` | Boolean | TRUE, FALSE | FALSE | — |
| `macMinBe` | Integer | 0–`macMaxBe` | 3 | — |
| `macPanId` | Integer | 0x0000–0xffff | 0xffff | — |
| `macResponseWaitTime` | Integer | 2–64 | 32 | — |
| `macRxOnWhenIdle` | Boolean | TRUE, FALSE | FALSE | — |
| `macSecurityEnabled` | Boolean | TRUE, FALSE | FALSE | — |
| `macShortAddress` | Integer | 0x0000–0xffff | 0xffff | — |
| `macSifsPeriod` | Integer | As defined in 11.1.4 | PHY dependent | `†` read-only |
| `macSyncSymbolOffset` | Integer | 0x000–0x100 for the 2.4 GHz band; 0x000–0x400 for the 868 MHz and 915 MHz bands, and the SUN FSK and SUN OFDM PHYs | — | `†` read-only |
| `macTimestampSupported` | Boolean | TRUE, FALSE | — | `†` read-only |
| `macTransactionPersistenceTime` | Integer | 0x0000–0xffff | 0x01f4 | `*` optional |
| `macUnitBackoffPeriod` | Integer | PHY specific | `aTurnaroundTime + ceil(phyCcaDuration / symbolDuration)` in symbols | — |


The PHY source is `ieee802154-2024:table:12-2`, PDF pp. 601–603, locators
`ieee802154-2024@2389231:2391190` and `@2391190:2396656`; read-only access is defined
by 12.3.1. **Table 12-2 has no default column**; the column below records that absence rather
than supplying defaults from a particular implementation.

| Attribute | Type | Range | Default | Marker |
|---|---|---|---|---|
| `phyBroadcastTxPower` | Signed integer | — | Not specified in Table 12-2 | — |
| `phyCcaDuration` | Integer | 1–1000000 | Not specified in Table 12-2 | — |
| `phyCcaMode` | Enumeration | MODE_1, MODE_2, MODE_3A, MODE_3B, MODE_4, MODE_5, MODE_6 | Not specified in Table 12-2 | — |
| `phyCurrentChannelInfo` | Channel Information structure as defined in 11.1.3.1 | PHY dependent as defined in 11.1.3.1 | Not specified in Table 12-2 | — |
| `phyCcaEdThreshold` | Implementation dependent | Implementation dependent | Not specified in Table 12-2 | — |
| `phyMaxPacketSize` | Integer | 16–4095 | Not specified in Table 12-2 | — |
| `phyMaxTxPower` | Signed integer | — | Not specified in Table 12-2 | `†` read-only |
| `phyPeersTxPower` | List of parameters as defined in Table 12-3 | — | Not specified in Table 12-2 | — |
| `phyRanging` | Boolean | TRUE, FALSE | Not specified in Table 12-2 | `†` read-only |
| `phyRxRmarkerOffset` | Integer | 0x00000000–0xffffffff | Not specified in Table 12-2 | — |
| `phyTxPower` | Signed integer | — | Not specified in Table 12-2 | — |
| `phyTxRmarkerOffset` | Integer | 0x00000000–0xffffffff | Not specified in Table 12-2 | — |
| `phyUnicastTxPower` | Signed integer | — | Not specified in Table 12-2 | — |


Qualifying conditions from the table descriptions remain part of these domains:

- `macFcsType` is valid only for LECIM, TVWS, SUN and HRP UWB in HPRF mode; its zero
  default must not select a four-octet FCS for O-QPSK.
- `macUnitBackoffPeriod` uses the CCA duration rounded upward to whole symbols before adding
  turnaround. This is a default expression; the table does not say that every PHY attribute
  write automatically overwrites an explicitly configured MAC backoff period.
- `phyMaxPacketSize` is 127 octets for ordinary O-QPSK; other PHY-specific descriptions
  override the generic 16–4095 range. The attribute is not dagger-marked in Table 12-2.
- `phyCcaDuration` is in microseconds; absent a PHY-specific recommendation, eight symbols
  are recommended. This recommendation is not a universal fixed default.
- `phyBroadcastTxPower` and `phyUnicastTxPower` do not exceed `phyTxPower`, which does not
  exceed `phyMaxTxPower`. `phyPeersTxPower` uses the element structure in Table 12-3.
- `macMinBe` depends on `macMaxBe`; a successful change cannot leave the pair outside the
  tabulated domains. The source does not prescribe a transactional multi-attribute API.
- `macPanId=0xffff` means not associated. Coordinator short addresses are chosen before PAN
  start; other devices receive their short addresses during association. Static setup does not
  establish that the association procedure has been implemented.

Other PHY-specific tables and optional-mode PIB tables are outside this data inventory; absence
here is not evidence that their attributes are optional or inapplicable to another selected mode.

### Functional-organization attributes

Table 8-37, physical PDF pp. 156–157,
`ieee802154-2024@683532:690780`, defines these additional Boolean attributes. Dagger marks
upper-layer read-only access. An unspecified default is not an implicit FALSE.

| Attribute | Type | Range | Default | Access marker |
| --- | --- | --- | --- | --- |
| `macDsmeCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macDsmeEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macDaCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macDaEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macExtendedDsmeCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macExtendedDsmeEnabled` | Boolean | TRUE, FALSE | FALSE | — |
| `macHoppingCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macHoppingEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macLeCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macLeEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macLeHsEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macMetricsCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macMetricsEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macRccnCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macRccnEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macSrmCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macSrmEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macTrleCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macTrleEnabled` | Boolean | TRUE, FALSE | Unspecified | — |
| `macTrleRelayingMode` | Boolean | TRUE, FALSE | Unspecified | — |
| `macTschCapable` | Boolean | TRUE, FALSE | Unspecified | † |
| `macTschEnabled` | Boolean | TRUE, FALSE | Unspecified | — |

## Extraction boundary

This catalog extracts selected legacy data/ACK, unslotted access, unsecured-policy, O-QPSK,
PIB and passive-scan statements. It does not exhaust these clauses: additional PHY/optional-mode
PIB tables and references for later enabled features require further audit. The bounded M1
reference closure is recorded in the [dependency ledger](../../model/ieee802154/dependencies.md).
Concrete local service error precedence and reset semantics remain implementation contracts for
packages 1a–1b. The field domains above cover Tables 8-36, 8-37 and 12-2, not every PIB table in the standard.
Beacon scheduling, indirect delivery beyond the retry distinction, association, security transforms,
enhanced formats, scheduled access, other PHYs and amendment requirements are outside this
extraction. Their absence does not assert that they are optional for a particular device role.

The group-address definition imported by 6.6.2 is recorded separately in the
[IEEE Std 802 address catalog](../ieee802/catalog.md); it is not a statement owned by this document.
