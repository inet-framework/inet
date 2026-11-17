# IEEE 802.15.4 — M1 applicability audit

> **Kind:** report · **Status:** snapshot 2026-09-19 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md), [catalog.md](../../standard/ieee802154/catalog.md)

Latest audit baseline: `5f3845f055` (initial pass: `fd6f800222`). This is the continuing extraction for step 0 of the
[implementation plan](../../../../../plan/pending/802154-implementation-plan.md).
**The bounded M1 engineering applicability gate passed independent review on 2026-09-19.**
The extracted statements and English procedures close the selected engineering audit, not every
mandatory obligation of a complete IEEE device profile. Deferred profile debt remains explicit.
No simulation behavior, source implementation or fingerprints changed in this pass.

## Selected engineering behavior

| Dimension | M1 selection | Effect on the claim |
| --- | --- | --- |
| Device roles | Statically configured device and PAN-coordinator endpoints | PAN-coordinator receive predicates remain applicable even without start/association procedures. A forwarding coordinator role is not claimed. |
| PAN mode | No periodic beacons; no superframe scheduling; direct data exchanges | Unslotted access applies. No CAP/GTS scheduling claim. |
| PHY | 2450 MHz O-QPSK, page 0, channels 11–26, 250 kbit/s; CCA Mode 1 | The intended PHY selection needs complete channel/PIB/measurement extraction before closure. |
| Addressing | Native short and EUI-64 identities; source-only, destination-only and dual-address legacy layouts | Static assignment is a simulation setup procedure, not evidence of standard association/address allocation. |
| Data service | MCPS request/confirm/indication; acknowledged and unacknowledged direct data; opaque MSDU, optional single receiver-configured protocol | No 6LoWPAN or IPv6 interoperability claim. |
| Management | Reset/get/set sufficient for selected attributes | Start, scan, association and other managed services remain separately owed or conditional. |
| Security | `macSecurityEnabled=false`; outgoing level zero; explicit rejection outcomes | Incoming secured version 0 and version 1 have different statuses. ACK eligibility is independent of security success for valid legacy input. |
| Sequence policy | One device-wide DSN; modulo-256 fresh allocation; stable retry DSN | No default duplicate suppression or exactly-once delivery claim. |
| Optional modes | No periodic beacon, GTS, enhanced frame/IE, scheduled access, ranging or security transform | Conditional requirements may be excluded only after checking their predicates; absence from the current catalog is not exclusion evidence. |

## Frame support decision

This is the proposed replacement's support boundary, not the current model's capability.

| Input or output | Transmit | Structural interpretation | Operational processing |
| --- | --- | --- | --- |
| Unsecured Data version 1 | Yes, `LegacyTx=true` | Legacy presence rules | Direct data service |
| Unsecured Data version 0 | No new-data generation initially | Legacy presence rules | Receive legacy data |
| Immediate ACK | Generate version 0 with the applicable Frame Pending value; 7.3.3 continuation sets other FCF fields to zero | Address-free legacy layout; accept legal versions 0/1 | Match DSN and outstanding exchange context |
| Valid secured legacy Data | No | Retain enough validated framing to apply filters and ACK decision; never expose protected bytes as plaintext MSDU | Version 0: `UNSUPPORTED_LEGACY`; version 1: `UNSUPPORTED_SECURITY`; preserve required ACK behavior |
| Version 2 Data/ACK | No | Identify unsupported version before applying legacy offsets | No legacy ACK fallback; enhanced behavior belongs to E1 |
| Beacon and command | No in the static data subset | Type discrimination; full codecs follow managed services | Deferred selected-device obligations remain visible |
| Reserved version/type/address mode or truncated required common framing | Never generate | Distinguish malformed from legal-but-unsupported; do not assert on external input | No ordinary data delivery; exact malformed-frame outcome needs the remaining format extraction |

Version 1 transmission follows the `LegacyTx` requirement in the continuation of Table 8-30.
Version 0 reception does not import the 2003 security procedure. The immediate-ACK continuation
is in corpus node `ieee802154-2024:figure:7-16`, physical PDF pp. 91–92; reading only the parent
7.3.3 node would miss it. The plan's “reject unsupported combinations” must mean controlled
policy rejection, not premature removal of framing needed for a required ACK.

## Receive decision table

Rows describe the selected legacy data path. Apply structural bounds and FCS validation before
ordinary filtering; apply 6.6.2 validity predicates before ACK and security decisions. A separate
capture observer is not an MCPS data indication. Normative promiscuous mode is defined by 10.23.1
and Table 10-114 (physical PDF pp. 379–380). It enables reception and passes MHR+MAC payload,
with Msdu, MpduLinkQuality, Timestamp and Rssi as the allowed valid-field set; disabling it
restores macRxOnWhenIdle. M1 selects optional Data timestamping off: Timestamp remains invalid.
This reconciles the allowed-field list with macTimestampSupported=false; it does not invent a
valid timestamp from internal simulation time.

The selected interpretation separates promiscuous monitor acceptance from ordinary 6.6.2 validity:
correctly received, structurally supported foreign-address frames can be monitored but do not gain
ACK eligibility. Valid local legacy AR frames retain their ordinary ACK obligations. Protected
bytes may be monitored as raw MHR+payload without asserting successful security or plaintext
MSDU delivery. This reconciles the table's receive-all wording with the clause's 6.6.2 reference;
the source does not explicitly prescribe this two-decision architecture or blanket ACK suppression.
Malformed/unsupported parsing boundaries remain explicit; incorrect FCS is discarded in both modes.

| Cause or accepted case | Capture visibility | Immediate ACK | Duplicate state | DSN in MCPS indication | MSDU delivery | Outcome |
| --- | --- | --- | --- | --- | --- | --- |
| Truncated common framing or invalid selected format | Raw observation only if safe | No (structural safety policy; not a standardized status claim) | No suppression cache in M1 | No | No | Malformed classification; no invented IEEE status |
| Incorrect FCS | May remain visible to forensic capture | No | None | No | No | FCS discard |
| Reserved type/version or failing PAN/address filter | May remain visible to capture | No | None | No | No | Filter discard |
| Valid local legacy unsecured unicast, AR=1 | Yes | Yes, received DSN | None | Yes | Yes | Successful indication |
| Same, AR=0 | Yes | No | None | Yes | Yes | Successful indication |
| Valid broadcast-addressed unsecured data | Yes | No | None | Yes | Yes | Successful indication; transmitted AR must be zero |
| Broadcast PAN but local unicast address, AR=1 | Yes | Yes | None | Yes | Yes | PAN broadcast does not imply address broadcast |
| Valid legacy secured unicast, AR=1 | Protected bytes may be observed | Yes under 6.6.2 | None | No data indication | No | Version 0 `UNSUPPORTED_LEGACY`; version 1 `UNSUPPORTED_SECURITY` |
| Valid ACK, matching outstanding exchange | Yes | No | None | Not an MSDU indication | No | Transaction success only within its expected interval |
| Wrong/late ACK | Yes | No | None | No | No | Must not complete an unrelated transaction |
| Unsupported version-2 traffic | Raw observation | No immediate-ACK fallback | None | No | No | Unsupported capability; no enhanced-processing claim |

Additional selected receive decisions:

| Predicate or mode | Normal MAC result | Promiscuous monitor result |
| --- | --- | --- |
| Source-only legacy Data/MAC command, source PAN matches, receiver is PAN coordinator, implicit broadcast false | Accept through 6.6.2(d)(4); apply ordinary AR/nonbroadcast rule | MHR+payload, only the four specified fields valid |
| Same frame at ordinary device with implicit broadcast false | Filter discard; no ACK | Monitor if structurally supported and FCS-correct |
| No destination address or destination PAN, implicit broadcast true | Accept through 6.6.2(d)(3); Table 8-36 describes broadcast treatment, so no ACK | Monitor with the same four-field validity rule |
| Extended group destination, group reception enabled, conforming AR=0 | Accept through 6.6.2(d)(2); no ACK requested | Monitor under the same field-validity rule |
| Injected nonbroadcast group destination, group reception enabled, AR=1, otherwise valid legacy data | Accept through ordinary filtering and ACK; sender violates the group AR rule | Ordinary ACK eligibility is retained; monitoring does not add eligibility |
| Foreign PAN/address frame, otherwise correctly received | No normal indication or ACK | Monitor MHR+payload; no ACK gained merely by monitoring |
| Correct local secured legacy AR frame | ACK eligible, security error and no plaintext MSDU delivery | Raw protected MHR+payload under the documented interpretation; AckSent is not a valid monitor parameter |

For an injected nonbroadcast group frame with AR=1, apply ordinary legacy receive filtering
and generate an immediate ACK if accepted. Clause 7.2.2.5 (physical p. 80,
`ieee802154-2024@366572:367029`) ties AR to 6.6.2 filtering; the latter excludes broadcast
from immediate ACK, not all groups. Clause 6.6.3.1 requires transmitters to clear AR for groups
but adds no receive-filter predicate. M1 never generates group AR=1; this receive decision does
not declare the injected transmitter conformant. All-ones broadcast retains its ACK exclusion.

The [imported IEEE Std 802 definitions](../../standard/ieee802/catalog.md) now close the
group-address source dependency. The native 64-bit address representation must not silently
substitute MacAddress group tests or call group addresses EUI-64 device identities. Ordinary
MCPS-DATA.indication AckSent is true only after an ACK has been sent (Table 8-32), not on AR
inspection or ACK scheduling. For PAN compression, distinguish the effective source PAN inferred
by 6.6.2 from the wire-presence validity of the SrcPanId indication field. Lifecycle and packet
ownership outcomes are implementation-contract obligations in step 1, not extra IEEE statements.

## Interpretations and prerequisites

| Item | Evidence and disposition | Gate affected |
| --- | --- | --- |
| IFS wording and figure | 6.3.1 (PDF p. 62) gives an “at least AIFS” lower bound after ACK; Figure 6-1 (p. 63) shows size-dependent max(SIFS/LIFS, turnaround). A size-dependent interval satisfies the textual lower bound too. Use both constraints; do not incorrectly replace the longer interval with AIFS. Record exact owner and start event in 1c. | 1c, 4–6 |
| ACK expected time | 6.6.3.4 says “within the expected time”. Searches for `macAckWaitDuration` and `AckWait` returned no corpus matches. A numeric timeout, propagation allowance and simultaneous-event rule remain to be justified from timing clauses/model assumptions; do not call a copied older-edition constant a 2024 requirement. | 1c, 6 |
| Promiscuous indication | 10.23.1 and Table 10-114 supply normative receive-all/format/receiver-control rules. Monitor acceptance and ordinary ACK eligibility are separated by the documented interpretation above; blanket ACK suppression is not a sourced 2024 rule. | 1a, 1b, 6 |
| Incoming secured legacy ACK | 6.6.2 ACK eligibility precedes the security outcome controlling delivery. 9.2.4(a)/(b) returns before key lookup when legacy/disabled. Preserve framing and ACK decision even when security is unsupported. | 2, 6 |
| Invalid security outputs | 9.2.4 declares outputs invalid until explicitly set; early failure cannot justify reading uninitialized security descriptors. Define indication validity explicitly. | 1a, 6 |
| Legacy encoding rules | 7.2.2.6 selects compression based on equal/different PAN IDs; Table 7-3 includes source-only coordinator semantics. Version-2 Table 7-2 is not a legacy oracle. | 2 |
| Native-address framework | `NetworkInterface` keeps generic `MacAddress`, but has attached `InterfaceProtocolData`. This establishes a candidate extension point, not a proven compatible interface. | 1d |
| Energy observations | Mode 1 requires energy detection without a decoded frame. Public medium listening/noise APIs can observe undecodable energy. Timed observation, averaging and cache retention still require the production fixture described in [results.md](results.md#phy-feasibility-inspection). | 4 |

## PIB selection and service boundaries

Concrete package-1b access, mutation, startup and reset choices are in the
[selected MAC PIB contract](pib.md).

The complete source rows of Tables 8-36, 8-37 and 12-2 are now recorded in the
[catalog field domains](../../standard/ieee802154/catalog.md#pib-field-domains), including
31 base MAC attributes, 22 functional-organization flags and 13 generic PHY attributes. The following is the M1 implementation selection;
it does not change source access markers or turn deferred capability into a normative exclusion.

| Attribute group | M1 responsibility / later dependency |
| --- | --- |
| macExtendedAddress, macShortAddress, macPanId, macCoordExtendedAddress, macCoordShortAddress | Native identity/configured PAN context. Honor read-only extended identity and distinguish unknown coordinator context from a fabricated zero address. Static setup is outside association-service evidence. |
| macDsn, macMinBe, macMaxBe, macMaxCsmaBackoffs, macMaxFrameRetries | Device-wide sequence and direct-access/retry state; validate cross-attribute BE domains. |
| macSifsPeriod, macLifsPeriod, macUnitBackoffPeriod | PHY-derived timing and default backoff calculation. SIFS/LIFS are upper-layer read-only; backoff period is not dagger-marked. Automatic recomputation after PHY mutation is a step-1b decision, not implied by the default expression. |
| macRxOnWhenIdle, macImplicitBroadcast | Idle-receiver and receive-filter behavior. TRUE/FALSE transitions need direct tests, including ACK wait while idle reception is false. |
| macGroupRxMode | Use the pinned IEEE Std 802-2024 group definition and the C-WIRE vectors. Default false; enabled group reception must use the full-width predicate, not the generic 48-bit address helper. |
| macSecurityEnabled | Disabled profile only; enabling unsupported security returns INVALID_PARAMETER under 8.2.2. Preserve the previous value; do not claim the writable attribute is read-only. |
| macSyncSymbolOffset, macTimestampSupported | M1 reports read-only macTimestampSupported=false. Typed MCPS timestamps are absent/invalid, not zero-valued measurements. Internal PHY event times remain required. Enabling optional timestamp support later requires the 6.5.3 symbol boundary, width and rollover rules. |
| macBeaconOrder | BO=15 context for M1; periodic values require later beacon-mode support. Optional attribute in the source does not erase the non-beacon receive-policy predicate. |
| macAutoRequest, macBeaconPayload, macBsn, macNotifyAllBeacons | Deferred scan/beacon services. Passive scan remains owed; BeaconNotify support and auto-request effects must be audited with its procedure. |
| macResponseWaitTime, macTransactionPersistenceTime | Deferred managed/indirect exchanges. These are not direct ACK-timeout substitutes. |
| macBattLifeExt, macBattLifeExtPeriods, macGtsPermit | Deferred beacon/GTS mechanisms; their absence must remain explicit in capability reporting. |
| macAoaEnable | Unsupported ranging/angle-of-arrival feature, outside selected O-QPSK behavior. |
| macFcsType | Not applicable to ordinary O-QPSK per the attribute description. Do not use its generic default 0 to select a four-octet O-QPSK FCS. |
| phyCcaDuration, phyCcaMode, phyCcaEdThreshold, phyCurrentChannelInfo | PHY-owned timed CCA/channel configuration, Mode 1 selected. Duration is in microseconds; thresholds are implementation-dependent. |
| phyMaxPacketSize | O-QPSK fixes the applicable value to 127, but the table does not mark the attribute read-only. Distinguish invalid PHY-specific values from READ_ONLY access. |
| phyMaxTxPower, phyTxPower, phyBroadcastTxPower, phyUnicastTxPower, phyPeersTxPower | PHY-owned power constraints. Table 12-3 supplies SHORT/EXTENDED deviceAddrMode, corresponding deviceAddress and signed TxPower; no default is given. Validate peer values against the applicable power constraints; specify lookup/mutation semantics in 1b/1c. |
| phyRanging, phyRxRmarkerOffset, phyTxRmarkerOffset | Ranging unsupported in M1. Preserve the read-only capability marker; offsets do not establish ranging support. |
| Functional flags (Table 8-37) | All optional capability flags report false in M1; corresponding enabled flags and LE handshake/TRLE relaying flags remain false. Respect dagger-marked read-only capabilities; reject true on present writable unsupported flags with INVALID_PARAMETER. |
| macPromiscuousMode (Table 10-114) | Optional feature selected for M1; use the documented monitor/ordinary-acceptance separation above. Default false; exit restores the stored idle policy. |

Source MLME-SET outcomes are READ_ONLY, UNSUPPORTED_ATTRIBUTE, INVALID_INDEX and
INVALID_PARAMETER, with SUCCESS indicating the value was written. Validate these separately
from implementation-only cancellation/busy/refusal results. A local service API may distinguish
such results without falsely claiming that every one is a standard MLME status.

MCPS-DATA confirm fields also have validity predicates: Timestamp is valid only on SUCCESS;
NumBackoffs is undefined on failure; ranging results are invalid if unsupported/disabled.
The indication's SrcPanId is valid only when a source PAN is included, whereas 6.6.2 supplies an
effective source PAN for internal processing when compression omitted it. Preserve both facts
with presence/validity metadata. DataRate=0 selects ordinary O-QPSK; it is not a bitrate value.

The full service continuation contains a source spelling discrepancy outside the selected ranging
behavior: Table 8-31 lists UNSUPPORTED_RANGING, while its prose says RANGING_NOT_SUPPORTED.
Record both for a future ranging contract; do not invent an alias and call it an exact standard
status. Likewise, its Rssi type is printed Boolean and supplies no numeric range. The selected service
contract represents ACK RSSI as an optional unsigned eight-bit value: both the numeric
interpretation and the width are local API choices, not a correction claimed on behalf of the
standard. The described zero/unavailable sentinel maps to absence. ACK metadata requires an
actually received ACK. Normal receive RSSI remains the separate eight-bit field of Table 8-32.
Normal indication DstPanId remains valid when inferred from the receiver's PAN, unlike SrcPanId's
wire-presence predicate. Independent bounded review confirmed these distinctions for step 1a.

## Selected channel descriptor

Clause 11.1.3.1 (physical PDF pp. 565–566,
`ieee802154-2024@2258178:2259440`) defines a PHY-specific channel information structure,
not a universal numeric channel or fixed field layout. Its listed fields are examples; its
requirement is enough information to identify all channel radio parameters. Table 12-2 makes
phyCurrentChannelInfo this structure for following transmissions and receptions.

For M1, the model contract selects O-QPSK, the 2450 MHz band and channel 11–26 as a single
validated descriptor. The selected mode fixes 250 kbit/s and the channel-frequency mapping already
cataloged in IEEE802154-PHY-14. Any legacy page-0 setting is a compatibility input to this
selection, not a substitute for the 2024 channel structure. Reject a descriptor naming another
PHY/band before applying a numerically overlapping channel index. This is the chosen model API
shape; the source does not prescribe its C++ representation.

Step 1b/1c must define when a validated descriptor takes effect relative to in-flight CCA,
transmission and reception. The source's “following” wording does not by itself define atomic
mutation or cancellation. The existing C-PHY channel sweep covers the selected frequency map;
the paired service fixture must additionally test mismatched PHY/band and rejected mutation
leaving the previous descriptor intact. This closes the source lookup, not the implementation
contract or runtime evidence.

## Mandatory-service and PICS reconciliation

Clause 6.1 (physical PDF p. 62, `ieee802154-2024@302462:302610`) declares clause 6
features mandatory. Its channel-access, PAN maintenance, synchronization and exchange descendants
must be classified individually; a heading-only extraction does not close their requirements.
Tables 8-1 (pp. 111–112, `@484842:489336`) and 8-26 (p. 136, `@583267:583941`)
provide the service inventory. Asterisks mark optional primitives; operation predicates still come
from the referenced procedures.

| Service boundary | M1 disposition / remaining debt |
| --- | --- |
| MCPS-DATA request/confirm/indication | Selected; direct data contracts and field-validity checks in 1a/6. |
| MLME-GET, SET and RESET request/confirm | Selected; attribute domains and status rules extracted, mutation semantics owed in 1b. |
| MLME-COMM-STATUS indication | Selected for incoming security errors; valid secured version 0 gives UNSUPPORTED_LEGACY and version 1 with disabled security gives UNSUPPORTED_SECURITY. Response-primitive transmission outcomes remain with their managed services. |
| MLME-START request/confirm | Unstarred; static configured endpoints do not establish this service. Managed start remains later debt; classify non-beacon prerequisites before M2. |
| MLME-BEACON request/confirm/indication and BEACON-REQUEST indication | Unstarred; audit procedure predicates with passive scan and coordinator response. Neither a blanket M1 obligation nor a justified exclusion follows from the table alone. |
| MLME-SCAN request/confirm | Starred interface; passive-scan capability remains mandatory under 6.4.1.1. Retain M2 debt. |
| Other starred services | Their optional interface marker is not a substitute for checking dependencies of selected behaviors. Association, indirect transfer and optional mode procedures remain separately gated. |

Annex E is explicitly **informative** (`ieee802154-2024:clause:E`, p. 941,
`@3502812:3502940`). Its PICS tables are a completeness cross-check, not normative overrides.
E.7.3 (p. 945, `@3511511:3512657`) marks extended addressing mandatory and short/enhanced
support optional. E.7.5.1 (pp. 951–954, `@3534457:3546133`) marks data service, access,
validation, acknowledged delivery, unsecured mode and passive scan mandatory. E.7.5.2
(pp. 954–957, `@3546133:3555819`) marks data, ACK and command format mandatory, while
individual commands have their own predicates. Command-format debt needs explicit treatment;
marking every command optional would miss this distinction.

E.7.4.2 (pp. 945–946, `@3512820:3515260`) labels generic ED/LQI optional, but selected
O-QPSK clauses 13.3.12/13.3.13 require them. Preserve those PHY-specific requirements.
The PICS M/O labels do not alter the normative catalog or establish a full-device claim.

## Clause-6 applicability inventory

The structural walk found 28 descendant clause nodes and 14 table/figure nodes. The following
classifies the remaining procedure groups; it does not claim every referenced statement has an
executable check. In particular, a mandatory capability deferred from the engineering subset
remains profile debt.

| Clause / source | Predicate and M1 disposition | Delivery / evidence boundary |
| --- | --- | --- |
| 6.1–6.2; p. 62 | Mandatory-feature scope, broadcast constants and symbol-unit conventions apply | Existing catalog plus the pinned IEEE Std 802-2024 address definitions |
| 6.3.1–6.3.2.1, Figures 6-1/6-2; pp. 62–64 | IFS and unslotted access apply | C-ACCESS/C-ACK and 1c timing contract; algorithm figures previously inspected |
| 6.4.1.1; pp. 64–65, `@310297:311546` | Passive scan capability is mandatory; during a scan suspend applicable beacon transmission and accept only relevant frames | Deferred to M2 step 8; optional ED/active/orphan modes are not inferred as mandatory |
| 6.4.1.2 and Figure 6-3 continuation; pp. 65–66, `@311546:316861` | Passive scan receives beacons without extracting their pending data; channel changes, receiver duration, descriptor storage, notification and termination rules apply when scanning | Extend C-SCAN before step 8 with macAutoRequest true/false, capacity and security-error variants; static M1 does not satisfy this debt |
| 6.4.2.1–6.4.2.2; pp. 66–67, `@316908:320648` | Starting a non-superframe PAN uses MLME-START. Prior reset, ED/active scan and distinct PAN selection are recommendations (“should”) in this procedure | Managed start is deferred; configured M1 PAN context is not MLME-START evidence. Do not promote recommendations into mandatory active-scan support |
| 6.5.1; p. 67, `@320751:321108` | Beacon synchronization uses decoded beacons; non-beacon synchronization uses coordinator polling | M1 claims direct transfer only, not synchronization/polling. Preserve M2 indirect/poll debt |
| 6.5.2 and Table 6-1; pp. 67–68, `@321108:324096` | Enhanced-beacon response/filter and requested-IE rules are triggered by enhanced requests and selected attributes | Enhanced requests/IE processing are excluded from M1's legacy operational formats; classify with E1 and the selected optional mechanisms before enabling them |
| 6.5.3; pp. 68–69, `@324096:325354` | Exported timestamps use the specified symbol boundary, units, width/precision and rollover | Optional Data timestamp capability is explicitly false in M1, with invalid typed MCPS metadata. Internal PHY timing remains required; enabling the service later requires these representation rules |
| 6.6.1–6.6.3.4; pp. 69–73 | Direct generation, filtering, security routing, ACK and retry rules apply; indirect retry branch remains deferred | C-WIRE/C-RECEIVE/C-SECURITY/C-ACK; expected-time deadline still needs 1c model assumptions |
| 6.6.4–6.6.5 and Figures 6-7–6-10; pp. 73–75 | ATI-end admission and clock-drift guard time apply to allocated transmission intervals | No periodic-beacon, GTS, TSCH or DSME ATI is selected in M1. Do not use this guard-time rule as the direct ACK-timeout formula |
| 6.6.6 and Figures 6-11–6-13; pp. 75–77, `@352845:359503` | Success, lost data and lost ACK have distinct observable paths; both direct-loss cases retry up to the same limit | C-ACK must inject data loss as well as ACK loss; indirect queue retention/expiry belongs to step 7 |

The Figure 6-3 continuation adds concrete M2 obligations: at least one PAN descriptor can be
stored; macAutoRequest=true returns stored descriptors and terminates at descriptor capacity
or after all available channels have been scanned, while false emits per-beacon notifications
and scans every channel. Nonempty beacon payloads also cause
notifications. Protected beacon information is retained with its security status even on security
error. This is scan behavior, not permission to deliver failed-security direct data as plaintext.
The full descriptor/SCAN/BEACON-NOTIFY contracts remain an M2 audit dependency.

The corpus omits some references, including 8.2.8.3 in 6.4.1.1, 10.8 in 6.5.1,
10.29.1.5 in 6.5.3 and the 11 distinct IE targets in Table 6-1. Direct lookup resolved those
targets; their existence is not evidence of implementation or recursive audit completion.

## PHY measurement boundary

The selected 2450 MHz O-QPSK profile brings all applicable clause 13.3 RF requirements, not
just packet timing. Clauses 13.3.1–13.3.13 occupy physical PDF pp. 618–621,
`ieee802154-2024@2462791:2469312`; Table 11-30 (p. 594, `@2365464:2366669`)
provides sensitivity-test conditions. Sensitivity is −85 dBm or better at PER below 1%, with
20-octet random PSDUs and no interference. This is not established by setting a receiver threshold.
PSD mask, interference rejection, EVM, frequency/symbol tolerance, actual transmit power and
maximum input level need independent PHY validation. They remain outside the M1 packet-level
engineering evidence claim and owed for any corresponding PHY-conformance claim.

Table 12-3 (p. 603, `@2396656:2397655`) closes the peer-power field extraction.
ED averaging is eight symbol periods or the shorter PHY-specified CCA duration (11.2.6);
CCA Mode 1's energy decision and ED's quantitative average are distinct observations.
The [feasibility inspection](results.md#phy-feasibility-inspection) identifies suitable extension
points and the missing timed contract. A Boolean listening decision does not prove ED calibration.

## Native-address compatibility decision

This matrix selects the bounded M1 integration boundary from existing public extension points.
It is a design decision for step 1d, not evidence that the replacement composition works.
Paths below are relative to `src/inet/`; the production fixture remains required.

| Consumer / boundary | Existing contract | M1 decision |
| --- | --- | --- |
| Interface identity | `networklayer/common/NetworkInterface.h`: addProtocolData/getProtocolData and integer interface ID; generic MAC accessors use MacAddress | Store native identity/PAN/short context in protocol-owned attached data; route by interface ID. Prove full-width identity and notification behavior. |
| Upper destination / lower source | `linklayer/common/MacAddressTag.msg` stores MacAddress only; Packet supports protocol-owned tags | Define native request/indication tags or typed primitives. Reject generic 48-bit address requests at the native boundary; no truncating adapter. |
| Queue transport / classification | Selectable IPacketQueue; `queueing/contract/IPacketClassifierFunction.h` accepts Packet | Use the chosen queue unchanged. If classification is needed, register a protocol-local classifier using native tags. Prove tag preservation and duplication. |
| Address filters | ReceiveAtMacAddress/SendToMacAddress and PacketFilter conversions use MacAddress | Exclude these native-address selectors from M1; implement native MAC filtering locally. Generic expression support is unproven, and PacketFilter is sealed. |
| Neighbor discovery | ARP and IPv6 ND resolve to MacAddress; ND reads generic interface identity | Exclude existing ARP/ND from native M1. Configured payload transfer does not imply IPv4/IPv6 support. |
| Forwarding / lookup | IMacForwardingTable and L3AddressResolver MAC lookup use MacAddress | Exclude Ethernet bridging and generic MAC lookup for native M1. Native configuration resolution is protocol-local. |
| Configuration | `NetworkInterface.cc` interprets a parent interface parameter named address as MacAddress during interface configuration | Use distinct native parameter ownership/parser; do not pass EUI-64 into inherited address handling. Prove initialization ordering and full-width parse/format. |
| Display | NetworkInterface %m is generic MAC; str() includes attached protocol data | Display native identity through attached data or protocol-local presentation. No claim that %m becomes EUI-64. |
| Payload / dispatch | ProtocolTag, registration callbacks and MessageDispatcher interface/protocol routing do not require native address values | One configured upper protocol per interface; receive adapter supplies it. Unknown capture payloads remain opaque. Mixed upper protocols require a later discriminator. |
| Medium address optimization | RadioMedium macAddressFilter inspects MacAddressInd/generic interface identity | Keep macAddressFilter=false for native M1 and perform native filtering in MAC. Never count this optimization as native filtering proof. |

Step 1d must exercise two EUI-64 identities with identical low 48 bits through the real interface,
queue and service adapter, plus a negative destination case. Observe upper destination, attached
identity, serialized addresses, lower source and configured payload dispatch. Include unsupported
generic requests, duplication and configuration/display round trips. These decisions need no
widening of MacAddress or planned modification to sealed common/packet source.

## Malformed input and unsupported capabilities

This is the bounded M1 policy. Local rejection reasons classify external input; they are not
invented IEEE service statuses. Selected operational input is legacy Data and immediate ACK;
managed command/beacon procedures remain deferred as declared in the frame-support matrix.

| Input / request | Selected outcome | Authority and evidence |
| --- | --- | --- |
| Too short for required common FCF, DSN, addressing or FCS fields | Safe discard; no ACK or ordinary delivery, no fabricated security status | Local bounds policy using clause-7 widths; C-WIRE truncation variants |
| Bad FCS, reserved type/version or ordinary PAN/address filter failure | Discard before ordinary security processing or ACK | 6.6.2; C-RECEIVE |
| Legacy sequence suppression/IE flag set, reserved address mode or contradictory address presence | Local malformed/unsupported-encoding discard; never guess enhanced offsets | Clause-7 generation constraints plus declared receive safety policy; C-WIRE |
| Reserved FCF bit 7 set, otherwise acceptable | Ignore that reserved field; continue normal processing | 4.6; C-WIRE/RECEIVE, distinct from reserved type/version values |
| Common framing safe, selected legacy Data accepted, security bit set | Preserve the protected remainder as opaque; perform 9.2.4(a)/(b) early return | Version 0 gives COMM-STATUS UNSUPPORTED_LEGACY; version 1 gives UNSUPPORTED_SECURITY. C-SECURITY |
| Same input, AR=1 and nonbroadcast | Apply required ACK independently of the security error | 6.6.2/7.2.2.5; C-RECEIVE/ACK |
| Assigned but nonselected version/type | Local unsupported-format discard; no legacy fallback or fabricated security result | Declared M1 engineering boundary, not a reserved-frame standards claim |
| Outgoing nonzero security request while disabled | Associated DATA.confirm UNSUPPORTED_SECURITY; no transmit | 9.2.2 and data-service rules; C-SECURITY |
| SET of absent attribute / source-marked read-only attribute / invalid hierarchical index | UNSUPPORTED_ATTRIBUTE / READ_ONLY / INVALID_INDEX respectively | Table 8-11; C-SERVICE |
| SET with out-of-range parameter or present writable attribute's unsupported value | INVALID_PARAMETER; previous state retained | 8.2.2/Table 8-11 plus local failure atomicity; SERVICE-9/C-SERVICE |
| SET of supported valid value | Apply the declared mutation procedure and confirm SUCCESS after writing | Table 8-11; 1b mutation semantics and C-SERVICE |

For example, setting supported macSecurityEnabled to true or selecting an unimplemented CCA
mode returns INVALID_PARAMETER, rather than falsely classifying the whole attribute as absent
or read-only. This applies to the selected implementation's support declaration; it does not
change the source's allowable values. Startup configuration rejects the same unsupported values
before simulation initialization succeeds; that configuration error is not an MLME confirm.

The secured-input boundary is **common framing safety**, not certification of the opaque
security remainder. In 9.2.4, outputs are invalid until set, and (a)/(b) return before auxiliary
header parsing in (c). M1 need not parse keys, MICs or security-control fields to obtain those
prescribed failures, and must not expose those outputs as valid. Fully validating the protected
remainder would add 9.4 child-clause dependencies and belongs with operational security support.
A short opaque remainder alone must not be converted into a fabricated security success.

Multiple simultaneous request defects, operation-busy refusal, cancellation and lifecycle results
need explicit local precedence/ownership in 1a/1b. The standard-derived fixtures isolate defects;
no unspecified precedence is inferred from this table's row order. Reserved encoding, malformed
input, unsupported profile and standardized failures remain separate observable classifications.

## Timing inputs and implementation handoff

The normative inputs are the selected 16 µs O-QPSK symbol, two-octet FCS, 127-octet PSDU
limit, six-octet PHY overhead, turnaround bounded by 12 symbols, outside-CAP ACK start after
AIFS=macSifsPeriod, and configured CCA/ED intervals. These are already mapped to C-PHY and
C-ACK. Clause 6.6.3.4 specifies an expected ACK interval without supplying a numeric universal
ACK timeout; defining that model interval is a contract task, not an undiscovered constant.

| Event / interval | Selected owner and observation boundary | Implementation gate |
| --- | --- | --- |
| Request acceptance | Service provider validates and accepts ownership; acceptance is not a radio event | 1a/1c: identity, refusal and cancellation contracts |
| PPDU start/end and receive completion | PHY provider reports actual air-interface boundaries, including SHR/PHR and FCS; PSDU completion is PPDU end for the selected O-QPSK format | 1c/4: timed provider and production observations |
| RX/TX readiness and turnaround | PHY provider owns radio transition and reports readiness; MAC must not add a second turnaround to an inclusive interval | 1c/4: transition and lifecycle tests |
| CCA/ED observation | PHY provider owns actual observation start/end and units. Setup before the observation is separate from phyCcaDuration or ED averaging time | 1c/4: pulse ending before completion, quantitative average and cancellation |
| Backoff | Access provider owns one attempt's symbol-rounded backoff and counters; PHY owns each CCA measurement | 5: access attempt tests |
| Immediate ACK start | Exchange engine schedules from received PPDU end plus AIFS; PHY must be ready at that target | 6: actual ACK start, not enqueue time |
| Interframe spacing | Exchange engine applies the selected IFS constraints; readiness intervals may overlap and are not blindly summed | 1c/6: source Figure 6-1 plus 6.3.1 |
| Direct ACK deadline and retries | Transaction owner starts the deadline at transmitted PPDU end and owns the retry decision | 1c/6: bound assumptions, lost data, lost ACK and simultaneous-event tests |

A candidate bounded M1 deadline is transmit PPDU end + 2·Dmax + 192 µs + 352 µs, where
Dmax bounds each propagation leg, 192 µs is the selected AIFS and 352 µs is the five-octet
immediate ACK plus PHY overhead. This is a **derived model contract**, not a quoted IEEE
constant. Its validity requires the selected propagation model, bounded geometry/motion,
receiver ACK timing and transmitter readiness. No generic medium API supplies Dmax automatically.
Step 1c must validate those assumptions for the selected composition, choose an explicit bound,
and define completion-at-deadline ordering before the deadline becomes a test oracle. A late
completion must never acquire success solely because its callback ran before a delayed timer.
This arithmetic does not justify adding IFS or a second turnaround to the ACK duration.

## Evidence admission and later gates

| Artifact / claim | Audit admission | Remaining gate |
| --- | --- | --- |
| Source ACK octets 02 00 6A E4 79 | Admitted as an independently specified CRC/example vector, checked by separate polynomial arithmetic | Step 2/3 must exercise the production serializer against these octets |
| Companion address vectors | Admitted for group/broadcast classification and octet-order arithmetic | Step 1a/1d/2 must exercise values, real interface identity and serialization |
| Existing ieee802154.pcap | Not admitted as a sole standards oracle: mixed versions, bad/unknown FCS and incomplete provenance are recorded | Step 3 must select admitted frames or obtain independent provenance; do not repair bytes to manufacture evidence |
| Native consumer compatibility matrix | Accepted as the chosen bounded integration design | Step 1d production fixture remains required |
| PHY listening/noise APIs | Architectural feasibility only | Step 4 timed CCA/ED fixture and independent PHY validation |

Capture interoperability evidence and working integration fixtures are implementation gates.
Their absence does not create another normative-source lookup task in step 0; the existing
capture's admission decision is complete and its evidence limits remain explicit.

## Step-0 gate verdict and implementation handoff

**PASS — bounded M1 engineering subset, 2026-09-19, baseline 5f3845f055 plus this audit diff.**
Independent review confirmed the selected predicates, malformed-input policy, unsupported-value
statuses, timestamp decision, timing inputs, evidence admission and source-reference dispositions.
The [dependency record](dependencies.md) preserves the stopping predicates, selected-target
expansion, manual roots and the unresolved non-protocol parser record. No necessary applicability
question remains open for this bounded selection; enabling another capability reopens its branch.

There are 87 catalog statements (85 base-standard and two companion address definitions), nine
English procedures, and explicit planned observations/delivery packages. All executable checks
remain NOT_RUN. This verdict does not complete M1 implementation, establish PHY validation,
authorize a conformance claim, or erase mandatory passive-scan and managed-service debt.

Package 1a may now define native values, typed MCPS/minimal MLME contracts, distinct IEEE and
local outcomes, ownership and cancellation. Packages 1b/1c still owe mutation/reset semantics,
validated propagation bounds and event ordering; 1d owes the native-interface fixture; steps 2–6
owe operational evidence. These are implementation gates with assigned owners, not unresolved
normative-source lookups. The legacy default remains unchanged until the stated cutover gate.
