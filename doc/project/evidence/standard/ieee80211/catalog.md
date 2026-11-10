# IEEE Std 802.11-2024 — selected mesh requirements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IEEE80211-2024-MESH-*` · **Stands on:** [Standards map](../../protocol/wifi/standards.md)

This catalog paraphrases the normative requirements selected for open mesh operation. It contains neither implementation verdicts nor component names. Each source is a canonical clause or table in `ieee80211-2024`; physical PDF pages locate the source independently of the corpus. A row can describe related conditions, but the [checks](../../protocol/wifi/checks.md) exercise those conditions separately. This is the selected mesh slice, not an exhaustive catalog of IEEE 802.11.

## Profile and peering

### IEEE80211-2024-MESH-DEPENDENCIES

Mesh operation requires QoS and excludes OCB and fast BSS transition. Infrastructure-dependent HCCA, traffic specification/streams, admission control, APSD and TDLS are not supported in an MBSS. HT mesh does not support STBC; DMG and S1G do not enable mesh operation. Mesh EPDImplemented=true requires EPDRequired=true.

Source: `ieee80211-2024:clause:14.2`, PDF 3192, source span `13447463:13448484`. Strength: normative capability constraints. Observation: configuration and advertised capabilities.

### IEEE80211-2024-MESH-PROFILE

All seven profile parameters must match: Mesh ID, path protocol, path metric, congestion control, synchronization, authentication and EPD support. A member does not change its profile while remaining in that mesh BSS. Leaving discards its session information; explicitly closing active peerings is recommended. Beacons, probe responses, Peering Open and Peering Confirm convey the profile. An MBSS has no common BSSID.

Source: `ieee80211-2024:clause:14.3.3`, PDF 3193; EPD indication: `ieee80211-2024:clause:11.51`, PDF 2930. Strength: shall, with should for explicit closure. Observation: wire and peer state.

For the selected mesh beacons, probe responses and peering actions, Address 3 is the transmitter address. A Probe Request instead uses the wildcard BSSID or the intended recipient's MAC address; a mesh recipient does not use that field to filter received Probe Requests. Sources: `ieee80211-2024:clause:9.3.3.1` addressing rule c)1)/c)4)v), continuation extracted under `ieee80211-2024:figure:9-128`, PDF 777–779; `ieee80211-2024:clause:9.2.4.3.4`, PDF 671–672. Strength: normative address construction and receive rule. Observation: emitted management headers.

### IEEE80211-2024-MESH-BASIC-SETS

Peering compatibility includes identical applicable configuration/basic sets: the basic rate set and, when used, the HT basic MCS set and VHT basic MCS/NSS set.

Sources: `ieee80211-2024:clause:14.3.4`, PDF 3193–3194; `ieee80211-2024:clause:14.3.7`, PDF 3195; `ieee80211-2024:clause:10.6.4`, PDF 1938–1939. Strength: shall within the selected generation. Observation: wire and accepted peer state.

HT/VHT BSS membership selectors denote required station features, rather than announcing enabled capability. A non-AP STA may omit them because its HT/VHT capability elements already indicate support. When present in the selected beacon/probe/mesh-peering rate elements, HT selector 127 encodes as `FF` and VHT selector 126 as `FE`; these are not legacy bitrates. Supported Rates holds at most eight combined rate/selector octets, with overflow carried by Extended Supported Rates under the same interpretation. Before joining, support all represented required features and do not attempt to join an unsupported or unrecognized requirement. Selector support does not establish basic-set content/equality, and selector-list equality is not the mesh configuration rule.

Sources: `ieee80211-2024:clause:9.4.2.3`/Figure 9-210, PDF 936, source span `4242772:4245635`; Table 9-131, `4246418:4249592`; `ieee80211-2024:clause:9.4.2.11`/Figure 9-233, PDF 951, `4303550:4304714`; `ieee80211-2024:clause:11.1.4.6`, PDF 2463–2464, `10385315:10390928`; non-AP definition 3.1, PDF 273, `1400243:1400360`; mesh basic-set distinction in 14.3.4, `13453075:13455363`. Strength: required-feature admission and encoding, with explicit non-AP omission permission. Observation: actual rate-element octets, enabled local features and independent basic-set checks.

### IEEE80211-2024-MESH-HT-VHT-CAPABILITIES

HT capabilities distinguish receive support, transmit MCS support, aggregation limits and spacing. VHT uses separate transmit and receive MCS/NSS maps. A VHT transmitter must restrict the tuple, NSS and bandwidth to the recipient's supported receive set, including applicable highest-rate and excluded-tuple restrictions. VHT support implies HT support: VHT mesh configuration advertises coherent HT receive MCS and channel-width support, and HT/VHT operation indicates the BSS bandwidth. Joining requires transmit and receive support for all basic VHT tuples.

Device capability and operating bandwidth are separate facts. The VHT mandatory baseline includes NSS1 MCS0–7 with long GI at 20/40/80 MHz; optional 160-MHz support carries that baseline when advertised. Short GI is optional. VHT20/40 reception uses the HT SGI support bits, while VHT80/160 uses the corresponding VHT bits. A local receive-support advertisement does not establish that local short-GI transmission is enabled. The selected profile uses uniform NSS capability across its advertised widths, while actual transmission remains limited by the current primary allocation and recipient support.

Sources: `ieee80211-2024:clause:21.1.1`, `ieee80211-2024:clause:21.5`, HT/VHT capability fields in `9.4.2.54.2` and `9.4.2.156.2`, and directional VHT transmission rules in `10.6.13.1`. Strength: mandatory baseline and conditional optional support. Observation: independent capability/operation fields and actual fixed tuple eligibility.

The selected guard interval also governs actual DATA symbol duration: long symbols are 4 microseconds and short symbols 3.6 microseconds. HT-mixed and VHT round the total short-data duration upward once to a multiple of 4 microseconds; HT-greenfield uses the raw short-data duration. Signal/training durations remain fixed. HT BCC uses one encoder unless the net PHY rate evaluated with 800 ns GI exceeds 300 Mb/s, when it uses two. The selected GI and coded gross rate do not change that encoder decision. VHT uses its own specified encoder tables.

Sources: `ieee80211-2024:clause:19.4.3`, Eq19-90/91/92, PDF 3492–3493, span `14595285:14598282`; `ieee80211-2024:clause:21.4.3`, Eq21-109/110/111, PDF 3661–3662, span `15278503:15283577`; `ieee80211-2024:clause:19.3.11.4`, PDF 3443, span `14379071:14379891`; Table 19-32, span `14612976:14614682`. Original PDF ceiling operators visually checked because text extraction omits them. Strength: normative duration and BCC rules. Observation: independent literal durations and actual transmitted signal occupancy.

Sources: `ieee80211-2024:clause:9.4.2.54.2`–`9.4.2.54.4`, PDF 1123–1126; `ieee80211-2024:clause:9.4.2.156.3`, PDF 1298–1299; `ieee80211-2024:clause:10.6.13.1`, PDF 1964–1965; `ieee80211-2024:clause:10.6.13.3`; `ieee80211-2024:clause:11.38.1`, PDF 2883–2884; `ieee80211-2024:clause:11.38.7`, PDF 2891. Strength: shall when selected. Observation: capability/operation bytes, committed directional support and actual PHY tuple.

Channelization uses nested primary 20/40/80 MHz allocations. Annex E explicitly lists primary channels 36/40/44/48 and 52/56/60/64, the corresponding 40-MHz pairs, 80-MHz center indices 42/58 and contiguous 160-MHz center index 50. Narrower VHT transmissions use the corresponding primary suballocation. With uniform NSS support across widths, modern 160-MHz VHT Operation uses channel-width value 1, primary-80 center segment 42 or 58 and full-width center segment 50; the actual contiguous PHY center index is 50, with its second index zero. Deprecated width-2 signaling and extended-NSS variants have different encodings. Channelization does not establish regulatory permission or satisfy required startup channel selection; the listed upper channels have DFS conditions.

Sources: `ieee80211-2024:table:E-4`, PDF 5660–5662, source span `23598736:23618211`; `ieee80211-2024:clause:19.3.15.4`, PDF 3470–3471, `14493286:14494355`; `ieee80211-2024:table:9-230`, PDF 1133–1135; `ieee80211-2024:table:9-316` and `9-317`, PDF 1302–1303, `5641088:5647720`; `ieee80211-2024:table:11-30`, PDF 2884–2886, `12207828:12217453`; `ieee80211-2024:clause:21.3.14`/Table 21-22, PDF 3636–3637; Annex E.1, PDF 5649–5650; `ieee80211-2024:clause:11.38.2`, PDF 2887. Strength: normative geometry/signaling with separately applicable regulatory/startup conditions. Observation: operation fields and actual occupied frequency intervals.

An ordinary non-HT ACK or immediate Block Ack responding to HT/VHT DATA uses the eliciting PPDU's channel bandwidth. At wider widths the non-HT duplicate format replicates legacy signaling over the occupied 20-MHz subchannels; this does not multiply the logical MAC payload or its bitrate. Response-rate selection uses the applicable basic rates and non-HT reference rate independently of occupied width. A separate primary-20 BAR can elicit a primary-20 Block Ack.

Sources: `ieee80211-2024:clause:10.6.6.1`, PDF 1942–1943; `ieee80211-2024:clause:10.6.6.5.2`, PDF 1945–1947; `ieee80211-2024:clause:10.6.6.6`, PDF 1951–1953; `ieee80211-2024:clause:10.6.11`; `ieee80211-2024:clause:21.3.10.12`, PDF 3632–3633, source span `15145634:15149637`. Strength: normative response format, width and rate selection. Observation: actual DATA/response PHY format, occupancy and timing.

EDCA contention uses primary-channel CCA. Wider TXOP admission additionally requires secondary groups idle for the applicable interval: PIFS for the selected 5-GHz operation, and DIFS for HT40 in 2.4 GHz. Fixed-bandwidth access can defer and reinvoke backoff at the current CW without a retry increment when that width is unavailable. A TXOP holder cannot widen beyond the preceding holder PPDU without the applicable wider duplicate/protection conditions. Unprotected wider operation has peer/neighborhood compatibility conditions; configuring a wide channel does not remove them.

Sources: `ieee80211-2024:clause:10.23.2.5`/Table 10-17, PDF 2004–2006, source span `8337919:8345797`; `ieee80211-2024:clause:11.15.9`, PDF 2639–2640, `11159725:11162860`; `ieee80211-2024:clause:10.23.2.8`, PDF 2010–2012, `8360328:8369680`; `ieee80211-2024:clause:10.27.3.5`, PDF 2090–2091, `8706010:8709721`. Strength: normative channel-access/protection conditions, with an allowed fixed-width choice. Observation: primary contention, secondary idle history, attempted transmissions and retry state.

### IEEE80211-2024-MESH-PEERING

Open mesh peering uses the peering management exchanges and state machine. Local and peer link identifiers distinguish relationship instances. An accepted Open assigns a unique peer AID, separately from link identity. An Open with no matching instance can create one; Confirm or Close with no matching instance is silently discarded. Group transmitter or receiver addresses fail instance matching. Before establishment, only the permitted discovery, peering and SAE traffic is allowed.

Sources: `ieee80211-2024:clause:14.4.1`, PDF 3196–3197; `ieee80211-2024:table:14-1`, PDF 3198; `ieee80211-2024:clause:14.4.4.2`, `ieee80211-2024:clause:14.4.5`, PDF 3199–3200. Strength: shall. Observation: exchanges and peer state.

### IEEE80211-2024-MESH-PEERING-FSM

The peering states are IDLE, OPN_SNT, OPN_RCVD, CNF_RCVD, ESTAB and HOLDING. In OPN_SNT an accepted Confirm cancels the retry timer, starts confirmation timing and enters CNF_RCVD; a subsequent accepted Open cancels confirmation timing, sends Confirm and establishes. In OPN_RCVD an accepted Confirm cancels retry timing and establishes. An established duplicate Open sends Confirm and retains establishment. Retry expiry resends Open while the count is below the limit; exhaustion sends Close and enters holding. In HOLDING an accepted Close cancels holding and returns to IDLE; accepted or rejected Open/Confirm sends Close without reviving the instance. Open, Confirm and Close validation applies before the associated accepted event.

Sources: `ieee80211-2024:clause:14.4.6.2`–`14.4.8.2`, PDF 3200–3202; `ieee80211-2024:clause:14.5.2`–`14.5.11`, PDF 3202–3210; timers in `ieee80211-2024:clause:14.5.4`, PDF 3204. Strength: shall. Observation: wire, state and timers.

The following transition checks use the event/action names of 14.5. An unlisted event is ignored in that state. TOR1 is retry expiry below the retry limit; TOR2 is expiry at the limit. TOC and TOH are confirmation and holding expiry. Rejection and cancellation carry the specified reason; establishment and Close-received cases use their prescribed reason codes.

| State | Event | Action and next state | Governing child |
| --- | --- | --- | --- |
| IDLE | ACTOPN | Set retry counter zero, send Open, start retry timer; OPN_SNT | 14.5.6 |
| IDLE | OPN_ACPT | Send Open and Confirm, start retry timer; OPN_RCVD | 14.5.6 |
| IDLE | REQ_RJCT | Send Close with rejection reason; remain IDLE | 14.5.6 |
| OPN_SNT | OPN_ACPT | Send Confirm, retain retry timer; OPN_RCVD | 14.5.7 |
| OPN_SNT | CNF_ACPT | Cancel retry, start confirm timer; CNF_RCVD | 14.5.7 |
| OPN_SNT | CNCL, CLS_ACPT, OPN_RJCT, CNF_RJCT | Cancel retry, send Close, start holding; HOLDING | 14.5.7 |
| OPN_SNT | TOR1 | Send Open, increment retry count, rearm retry; remain OPN_SNT | 14.5.7 |
| OPN_SNT | TOR2 | Send Close with MAX_RETRIES, start holding; HOLDING | 14.5.7 |
| CNF_RCVD | OPN_ACPT | Cancel confirm, send Confirm; ESTAB | 14.5.8 |
| CNF_RCVD | CNCL, CLS_ACPT, OPN_RJCT, CNF_RJCT | Cancel confirm, send applicable Close, start holding; HOLDING | 14.5.8 |
| CNF_RCVD | TOC | Send Close with CONFIRM_TIMEOUT, start holding; HOLDING | 14.5.8 |
| OPN_RCVD | OPN_ACPT | Send Confirm; remain OPN_RCVD | 14.5.9 |
| OPN_RCVD | CNF_ACPT | Cancel retry; ESTAB | 14.5.9 |
| OPN_RCVD | CNCL, CLS_ACPT, OPN_RJCT, CNF_RJCT | Cancel retry, send applicable Close, start holding; HOLDING | 14.5.9 |
| OPN_RCVD | TOR1 | Send Open, increment retry count, rearm retry; remain OPN_RCVD | 14.5.9 |
| OPN_RCVD | TOR2 | Send Close with MAX_RETRIES, start holding; HOLDING | 14.5.9 |
| ESTAB | OPN_ACPT | Send Confirm; remain ESTAB | 14.5.10 |
| ESTAB | CNCL, CLS_ACPT, OPN_RJCT, CNF_RJCT | Send applicable Close, start holding; HOLDING | 14.5.10 |
| HOLDING | CLS_ACPT | Cancel holding; IDLE | 14.5.11 |
| HOLDING | OPN_ACPT, CNF_ACPT, OPN_RJCT, CNF_RJCT | Send Close; remain HOLDING | 14.5.11 |
| HOLDING | TOH | IDLE | 14.5.11 |

## Wire format and forwarding

### IEEE80211-2024-MESH-WIRE

Mesh Control carries its flags, nonzero on-air Mesh TTL and mesh sequence number, followed by the address extensions selected by AE. A received group TTL of one can decrement to zero during local processing; that is distinct from an invalid received wire value of zero. HWMP Mesh Path Selection actions carry the RANN, PREQ, PREP and PERR elements with their specified field widths, element lengths, target counts and optional fields. Reserved transmit fields are zero; unknown optional information and reserved receive fields follow their specified receive semantics, rather than a blanket rejection rule.

Sources: `ieee80211-2024:clause:9.2.4.8.3`, PDF 707–708; `ieee80211-2024:clause:9.4.2.110`–`9.4.2.113`, PDF 1222–1227; `ieee80211-2024:clause:9.6.16.3`, PDF 1773. Strength: normative format. Observation: encoding.

An unsecured non-HT management Frame Body is at most 2304 octets, including the Action Category and action code. A 24-octet MAC header and four-octet FCS therefore permit at most 2332 octets for the complete selected Action MPDU, independently of a larger PHY length field. Multiple HWMP elements may share one Action when their intended Address 1 is the same; the complete body remains bounded.

Sources: `ieee80211-2024:clause:9.2.4.8.1`, PDF 702, source span `3232286:3233967`; `ieee80211-2024:table:9-34`, PDF 703, `3233967:3244113`; `ieee80211-2024:clause:9.3.3.1`, PDF 777, `3539210:3540794`; `ieee80211-2024:clause:14.11.7`, PDF 3231–3232, `13601356:13607065`. Strength: normative length and common-recipient constraints. Observation: complete Action length and Address 1.

### IEEE80211-2024-MESH-FORWARD

Forwarding preserves mesh endpoint identity while changing hop addresses. Mesh TTL bounds propagation and mesh sequence identity supports duplicate detection. Individually addressed transit traffic is forwarded toward the mesh destination; group-addressed traffic is delivered and propagated subject to the applicable forwarding and duplicate rules. Mesh sequence identity and per-hop MAC retransmission identity are different fields.

Sources: `ieee80211-2024:clause:10.37`, `ieee80211-2024:clause:10.37.2`, PDF 2144–2145; Mesh Control source above. Strength: shall. Observation: wire and end-to-end delivery.

### IEEE80211-2024-MESH-FORWARDING-STATE

Forwarding information includes destination, next hop, lifetime and precursors with their own expiry. A precursor refresh retains the maximum of its current and new expiry; expired precursors are removed. Only validated, unexpired forwarding information authorizes forwarding. Reception from a nonpeer is silently discarded. For received individual traffic, the transmitter must appear in the destination's precursor information before delivery or transit forwarding.

Sources: `ieee80211-2024:clause:10.37.2`, PDF 2144–2145, source span `8944289:8946709`; `ieee80211-2024:clause:10.37.3.2`, PDF 2146–2147, source span `8950174:8955238`. Strength: shall. Observation: precursor/path state, absence of unauthorized upper delivery or forwarding.

### IEEE80211-2024-MESH-FORWARDING-LIFETIMES

Transit individual data resets destination and source forwarding lifetimes to their initial values. The destination's precursor entry for the transmitter and the source's precursor entry for the next hop toward the destination retain at least the initial precursor lifetime. Decrement Mesh TTL and discard at zero before transit transmission. Final-destination delivery refreshes the source forwarding lifetime and applicable destination precursor lifetime without a transit TTL decrement. Thus a final-destination frame is not discarded solely because transit decrement would reach zero.

Source: `ieee80211-2024:clause:10.37.3.2`, PDF 2146–2147, source span `8950174:8955238`. Strength: shall. Observation: timer state, transit TTL and final delivery.

### IEEE80211-2024-MESH-GROUP

Native non-GCR group QoS DATA transmits No Ack policy1 (QoS Control B5=1, B6=0). Independently, a received group-addressed frame never receives an ACK or Block Ack, including when its received policy is0; the cited rule does not mandate discard solely for that field value. Sources: Table9-13, PDF677, span `3124619:3131039`; `ieee80211-2024:clause:10.3.2.11`, span `7922099:7924087`. Strength: transmitted policy and received response requirements. Observation: actual source and relay frame fields with ordinary group delivery.

A native source group frame uses Address 1=group, Address 2=transmitter, Address 3=mesh source and AE=0. Reception rejects nonpeer input and duplicates. Decrement TTL on reception: zero prevents propagation but still permits local upper delivery. Nonzero TTL permits propagation when forwarding is enabled, changing Address 2 to the forwarding station. Forwarding disabled prevents multihop propagation. Duplicate identity is mesh source and mesh sequence, independently of same-peer MAC duplicate detection.

Sources: `ieee80211-2024:clause:10.37.4.1`, PDF 2147–2148, source span `8955316:8957690`; `ieee80211-2024:clause:10.37.4.2`, PDF 2148, source span `8957690:8961133`; `ieee80211-2024:clause:10.37.6`, PDF 2150, source span `8967167:8968220`; `ieee80211-2024:clause:10.37.7`, PDF 2150, source span `8968220:8968877`. Strength: shall within enabled forwarding. Observation: addresses, duplicate identity, TTL and local/transit delivery.

### IEEE80211-2024-MESH-NO-PATH

A station lacking forwarding information follows the applicable discovery and no-path rules. Transit handling can discard, discover or report a path error to the transmitter under its specified conditions. A source unable to discover a path can use available mesh gates; when none is available it discards. A station advertising a root is not thereby a mesh gate.

Sources: `ieee80211-2024:clause:10.37.3.2`, PDF 2146–2147; `ieee80211-2024:clause:10.37.8`, PDF 2150–2152, source span `8968877:8972390`. Strength: conditional normative behavior. Observation: discovery, path error and no-path discard.

## Airtime and HWMP

### IEEE80211-2024-MESH-AIRTIME

The two metric identifiers use `(O/n + Bt/r) / (1 - ef)` with a nominal test frame body of 8192 bits. ID 1 fixes `n = 1` and encodes cost in 0.01 TU; ID 2 amortizes overhead over aggregation and encodes cost in 0.01 microseconds. Path accumulation starts at zero. The rate and error estimates concern the local outgoing link; TTL failures do not estimate link corruption. A PHY capable of rates above 54 Mb/s should use ID 2. Estimation algorithms remain local implementation choices.

Sources: `ieee80211-2024:clause:14.10.1`, `ieee80211-2024:clause:14.10.2`, PDF 3224–3225; `ieee80211-2024:table:14-4`, `ieee80211-2024:table:14-5`, PDF 3225–3226. Strength: shall for selected metric, should for high-rate selection. Observation: encoding and link/path state.

### IEEE80211-2024-MESH-METRIC-REPORT

On receiving a Link Metric Report element with Request=1, a station replies with a report. It may update its local link metric from a received report; the update algorithm is outside the standard's scope.

Source: `ieee80211-2024:clause:14.9.3`, PDF 3224. Strength: normative exchange. Observation: wire.

### IEEE80211-2024-MESH-SEQUENCE

A station increments its own HWMP sequence number before discovery. A target answering a PREQ updates its own number to one more than the circular maximum of its current number and the known requested target number. USN=1 makes the requested target-sequence field reserved, so its bytes do not supply a sequence baseline. Other destinations' sequence history changes from received protocol information, except the specified next-hop failure increment. Comparisons use circular modulo-2^32 arithmetic; an exact half-range pair does not supply a unique circular maximum under these rules.

Sources: `ieee80211-2024:clause:14.11.8.3`, PDF 3233–3234; Figure 9-605, PDF 1225, source span `5340357:5341739`. Strength: shall, with the stated half-range source ambiguity. Observation: wire and path state.

### IEEE80211-2024-MESH-PATH-UPDATE

PREQ establishes reverse information toward the originator; PREP establishes information toward its target. A fresher sequence number wins; equal sequence information replaces a path when its updated metric is better. Add the receiving station's outgoing cost toward the transmitter. A created direct-neighbor record has unknown sequence; refreshing it retains its sequence. Lifetime updates retain the later of the existing deadline and the received lifetime added to now. PREQ alone does not validate a new reverse path: PREP transmission validates the path toward its originator and PREP reception validates the path toward its target. Expiry or an unusable next hop invalidates the corresponding forwarding information.

Sources: `ieee80211-2024:clause:14.11.8.4`, `ieee80211-2024:table:14-9`, `ieee80211-2024:table:14-10`, PDF 3234–3235. Strength: shall. Observation: path state and wire.

### IEEE80211-2024-MESH-DISCOVERY

PREQ processing follows its target and flag cases; duplicate identity is not an unconditional discard rule. Processing can proceed when reverse information updates, or under the stated equal target sequence and previously unseen originator/discovery identity condition. PREP propagation and precursor changes require creation or update of target forwarding information; propagation requires remaining element TTL greater than one. Discovery retries wait at least twice the network diameter traversal time and each attempt uses a new Discovery ID and incremented HWMP sequence number.

Sources: `ieee80211-2024:clause:14.11.8.5`, PDF 3236; `ieee80211-2024:clause:14.11.9.4.3`, PDF 3245–3246; `ieee80211-2024:clause:14.11.10.4.3`, PDF 3249–3250. Strength: shall. Observation: wire, precursor/path state and timers.

### IEEE80211-2024-MESH-PREQ-CASES

Do not initiate a second ongoing discovery for the same target. Postpone an initiation until the target-specific PREQ minimum interval permits it. Initial on-demand PREQ uses group addressing, hop zero, configured network-diameter TTL, fresh Discovery ID, incremented originator sequence and metric zero; target sequence knowledge controls USN. Propagation increments hop count, decrements TTL and adds the local outgoing cost toward the transmitter, preserving originator sequence, Discovery ID and lifetime. An intermediate reply changes that target's TO to one in the propagated PREQ. In a multiple-target PREQ remove targets answered as self or proxy, retain other targets and set TO for intermediate replies; replying to one target does not terminate processing of all targets.

Sources: `ieee80211-2024:clause:14.11.9.3`, PDF 3236–3237, source span `13623746:13625185`; Tables 14-11 and 14-15–14-17, PDF 3237–3244, source spans `13625185:13629244`, `13640547:13648539`. Strength: shall within the corresponding case. Observation: control fields and per-target discovery state.

The receipt guard in 14.11.9.4.3 compares the target sequence in its second alternative. Propagation cases E1/E2/E3 in 14.11.9.3 compare the originator sequence and unseen originator/Discovery ID. These are separate guards on separate actions; the source wording must not be silently normalized to a single generic duplicate predicate.

### IEEE80211-2024-MESH-PREP-CASES

An intermediate reply requires TO=0, active forwarding information for the target and nonzero lifetime. With USN=0 its retained target sequence must be at least the requested sequence; USN=1 imposes no requested-sequence freshness floor because that field is reserved. It uses retained target sequence/metric but copies the triggering PREQ lifetime. A target reply uses metric zero and its updated target sequence; its lifetime also comes from the PREQ. Propagation increments hop count, decrements TTL and adds outgoing-to-transmitter cost. Propagating a PREP adds the next hop toward the originator to the target's precursors and the next hop toward the target to the originator's precursors, with the applicable forwarding lifetimes. Intermediate PREQ replies establish the corresponding precursor relationships.

Sources: `ieee80211-2024:clause:14.11.10.3`, PDF 3246–3247, source span `13657925:13659777`; Tables 14-18–14-20, PDF 3247–3249, source span `13659777:13666300`; `ieee80211-2024:clause:14.11.10.4.3`, PDF 3249–3250; `ieee80211-2024:clause:14.11.9.4.3`, PDF 3245–3246. Strength: shall within the corresponding case. Observation: wire and precursor state.

### IEEE80211-2024-MESH-PERR

PERR processing concerns destinations whose selected next hop is the transmitter. For NO_FORWARDING_INFORMATION with sequence zero, increment the retained destination sequence and invalidate. Apply the general stale-sequence rejection and the reason-specific predicates in 14.11.11.4.3(c): NO_FORWARDING_INFORMATION with a nonzero sequence is a separate condition from DESTINATION_UNREACHABLE with a sequence strictly greater than retained information. In particular, do not impose the latter's strict-newer condition on both reasons without resolving the source wording. Propagate the applicable destination information to affected precursors, with the specified sequence and element-TTL rules. Do not invalidate unrelated paths or accept a PERR merely because the destination appears in it.

Sources: `ieee80211-2024:clause:14.11.11.4.2`, `ieee80211-2024:clause:14.11.11.4.3`, PDF 3254. Strength: shall. Observation: wire, selected next hop and precursor state. The [standards map](../../protocol/wifi/standards.md#interpretation-and-extraction-limits) records the table's cross-reference mismatch.

### IEEE80211-2024-MESH-ROOTS

Proactive PREQ and proactive RANN are separate root mechanisms and coexist with on-demand discovery. RANN announces root reachability; a receiver may initiate an individually addressed root-confirmation PREQ/PREP to establish a bidirectional path. The announcement alone does not complete that exchange. RANN comparison accepts an equal sequence number with equal metric; reject an equal-sequence worse metric. Root-confirmation addressing follows its own rules.

Sources: `ieee80211-2024:clause:14.11.7`, `ieee80211-2024:table:14-14`, PDF 3231–3232, 3240–3241; `ieee80211-2024:clause:14.11.12.4.2`, PDF 3256–3257. Strength: shall in selected root mode. Observation: wire and path validity.

### IEEE80211-2024-MESH-ROOT-PREQ

Root modes two and three periodically originate PREQ at the root interval, targeting broadcast with TO=1, USN=1 and target sequence zero. They use hop zero, metric zero, incremented root sequence, new Discovery ID and active-path-to-root lifetime. Mode three sets Proactive PREP; mode two clears it. A receiver sends a proactive PREP when the flag is set, or when it needs a bidirectional root path under the flag-zero case. The proactive PREP targets the responding station itself, uses its current own sequence and metric zero, and copies the root originator fields and lifetime. Table 14-21 does not explicitly repeat Table 14-18's qualification about the target-response sequence update; the application of 14.11.8.3 to this broadcast-response case is ambiguous, rather than a stated prohibition on a preceding local update.

Sources: `ieee80211-2024:clause:14.11.4.2`, PDF 3229–3230, source span `13595166:13597902`; `ieee80211-2024:table:14-13`, PDF 3239–3240, source span `13632749:13636271`; `ieee80211-2024:table:14-21`, PDF 3249, source span `13666300:13667819`. Strength: shall within selected root mode and reply condition. Observation: fields and proactive paths.

The flag-zero condition is not restricted to the PREQ reception callback: local-source data can later require the bidirectional root path before DATA transmission. Informative S.7.3/S.7.5 describe that later trigger and the local Proactive PREP Sent state. Source: Case D of `ieee80211-2024:clause:14.11.10.3`; informative S.7.3/S.7.5, PDF 5912–5913, source spans `24495350:24496429`, `24497202:24498141`.

### IEEE80211-2024-MESH-RANN-PROPAGATION

Propagating a RANN requires validated forwarding information to the root, forwarding enabled, TTL greater than one, and the RANN sending-interval gate. Increment hop count, decrement TTL, add outgoing-to-transmitter metric and preserve the announcement's root sequence and Interval field. Acceptance updates the shared RANN-interval MIB scalar from the received field. Own RANN origination uses that current scalar for timing and advertisement. An accepted announcement lacking validated root forwarding information cannot immediately propagate; the retrieved procedure specifies no additional random forwarding delay.

Sources: `ieee80211-2024:clause:14.11.12.3`, `ieee80211-2024:table:14-26`, `ieee80211-2024:table:14-27`, PDF 3256, source span `13688771:13690742`; `ieee80211-2024:clause:14.11.12.4.3`, PDF 3257. Strength: shall within the propagation case. Observation: root path validation, timing and wire.

The RANN and root-confirmation minimum intervals constrain Action frames sent by the station across roots. Per-root histories do not create separate station-wide transmission allowances. Case D's periodic confirmation condition refers to the last locally originated PREQ to that root without restricting it to a previous individual confirmation; forwarding somebody else's PREQ is distinct. Sources: C.3 `dot11MeshHWMPrannInterval`, source span `22451916:22452489`; C.3 `dot11MeshHWMPconfirmationInterval`, `22454037:22455056`; Case D after Table 14-13, PDF 3240, `13632749:13636271`. Strength: minimum intervals and root-specific trigger. Observation: Action counts, root targets and timing.

## Synchronization

### IEEE80211-2024-MESH-TSF

Neighbor-offset synchronization compares timestamp references at their specified transmit and receive instants, accounting for elapsed receive time. Mesh timing does not use the infrastructure TSF reset procedure. Required TSF slowing is bounded to 0.04% per beacon period. Beacon ordering and scheduling use the mesh rules.

Sources: `ieee80211-2024:clause:11.1.3.1`, PDF 2428–2429; `ieee80211-2024:clause:11.1.3.9`, PDF 2442–2443; `ieee80211-2024:clause:14.14.2.2.2`, `ieee80211-2024:clause:14.14.2.2.3`, `ieee80211-2024:clause:14.14.3.1`, PDF 3268–3270. Strength: shall. Observation: timestamps, timing state and actual transmissions.

## HT/VHT and aggregation

### IEEE80211-2024-MESH-AMSDU

Each mesh A-MSDU subframe contains mesh destination, mesh source, Length and its own Mesh Control. Mesh subframe Length is little endian and counts the MSDU, excluding Mesh Control; padding is determined from the complete mesh subframe. Basic A-MSDU layout does not substitute for this format.

Sources: `ieee80211-2024:clause:9.3.2.2.3`, `ieee80211-2024:figure:9-124`, PDF 775. Strength: normative format. Observation: encoding and constituent delivery.

An HT-capable receiver accepts valid standalone Normal-Ack A-MSDUs even in non-HT PPDUs; local formation policy and ADDBA permission do not gate that mandatory reception. The HT advertised A-MSDU body tiers are 3839/7935 bytes. A non-HT OFDM PSDU is limited to 4095 bytes, independently of that advertisement. The HT A-MPDU member limit of 4095 bytes applies to nested transport, not standalone HT A-MSDUs. VHT uses its advertised MPDU limit. Each constituent MSDU is at most 2304 bytes. Selected HT-mixed and VHT PPDUs are limited to 5484 microseconds; response airtime is a separate exchange cost.

Sources: `ieee80211-2024:clause:10.11`, PDF 1971–1976, span `8200504:8219198`; Table 9-224, span `4976790:4983600`; Table 17-24, span `14152347:14155438`; `ieee80211-2024:clause:10.12.2`, span `8220478:8226847`; Table 9-34, span `3256895:3261950`; Table 21-28, span `15284295:15285984`. Strength: mandatory reception and transport limits. Observation: independently reconstructed bytes and actual PHY/exchange admission.

A zero TXOP limit permits one forwarding unit. A fresh A-MSDU must fit a positive exchange budget. The initial plain-MSDU overrun exception requires a Block Ack agreement. A genuine same-size MPDU retransmission, including an A-MSDU, may overrun only as the sole Data/Management frame in that TXOP, outside a multiple-MPDU aggregate and DL-MU-MIMO. Merely committing an identity or setting Retry does not establish prior transmission; PHY, peer and lifetime limits still apply.

Source: `ieee80211-2024:clause:10.23.2.9`, PDF 2012–2014, span `8369680:8378130`. Strength: conditional TXOP rules. Observation: actual first transmission, unchanged retry size, exchange budget and no-fit queue behavior.

### IEEE80211-2024-MESH-AMSDU-IDENTITY

Native mesh A-MSDU subframes share the same individual mesh source, including applicable outer source fields. Individual-address aggregates share the same individual mesh destination, including the applicable outer destination field; group-address aggregates use the outer group destination. The modulo-2^32 mesh sequence counter starts at zero and increments once for each new nonaggregated MSDU, A-MSDU or multihop MMPDU. Duplicate rejection concerns the entire received MSDU/A-MSDU/MMPDU identity, rather than treating sibling subframes of one A-MSDU as new duplicates. An existing A-MSDU expires when all its constituent MSDU lifetime timers have expired, permitting an older constituent to remain in that still-valid aggregate.

Sources: `ieee80211-2024:clause:10.11`, PDF 1971–1976, source span `8200504:8219198`; `ieee80211-2024:clause:9.2.4.8.3` continuation in Table 9-35, PDF 708, source span `3274376:3277715`; `ieee80211-2024:clause:10.37.3.1`, PDF 2145–2146, source span `8946794:8950174`; `ieee80211-2024:clause:10.37.6`, PDF 2150. Strength: shall. Observation: endpoints, sequence allocation, duplicate admission and aggregate expiry.

### IEEE80211-2024-MESH-AMPDU

Aggregated transport obeys the selected HT/VHT format, per-member MPDU integrity and delimiter rules. VHT single-MPDU transport still uses its required delimiter/EOF representation. Aggregate construction respects negotiated MPDU/A-MPDU limits, duration and spacing. Immediate Compressed Block Ack identifies received members; recovery preserves sequence identity and retransmits only eligible missing members within applicable retry and lifetime bounds.

Sources: `ieee80211-2024:clause:10.12`, `ieee80211-2024:clause:10.12.8`, PDF 1976–1982; `ieee80211-2024:clause:10.25`, PDF 2049 onward. Strength: shall when selected. Observation: bytes, PPDU/member state and upper delivery.

The four-octet delimiter's MPDU Length includes the member MAC header and FCS, excluding delimiter/padding. Its first word places EOF at bit 0, a reserved bit at 1, the two high length bits at 2–3 and the low twelve length bits at 4–15. HT reserves the high length bits and clears EOF. A VHT single payload member with EOF set is an S-MPDU; ordinary multiple-payload-member aggregates clear EOF. CRC-8 protects the preceding sixteen bits using the specified initialization/complement and polynomial, followed by signature `0x4e`. Actual received PHY format identifies this framing, rather than information supplied only by the transmitter's local software state.

Sources: `ieee80211-2024:clause:9.7.1`, Figures 9-1326–9-1331 and Table 9-659, PDF 1840–1842, source span `7656504:7666563`; `ieee80211-2024:clause:9.7.2`, PDF 1842–1843, `7666563:7668654`; `ieee80211-2024:clause:10.12.5`, PDF 1979–1980; `ieee80211-2024:clause:10.12.7`, PDF 1981–1982, `8242052:8244436`. Strength: normative delimiter and receive-format rules. Observation: independent encoded bytes and actual PHY receive format.

Delimiter CRC processing consumes protected bits B0 through B15 in transmission order, starting with all coefficients one, then complements the remainder. Its most significant polynomial coefficient is transmitted first as delimiter bit B16, unlike ordinary octet least-significant-bit-first ordering. A serialized CRC octet must reflect that coefficient-to-wire-bit mapping.

Sources: `ieee80211-2024:clause:9.7.2`/Figure 9-1332, PDF 1842–1843, source span `7666563:7668709`; `ieee80211-2024:clause:9.2.2`, PDF 657–658, `3049211:3052920`. Strength: normative CRC initialization, complementation and bit order. Observation: literal CRC octets independently derived from the original register diagram.

Reserved delimiter B1 is transmitted zero and ignored on reception; HT also ignores its reserved high-length bits B2/B3 and interprets only the twelve low length bits. CRC still protects the received sixteen bits, including reserved values. B0 is the defined EOF/Tag field in the 2024 format, not reserved: HT transmission sets it zero under10.12.7. For selected non-HE VHT, no EOF0 subframe may follow any EOF1 subframe, including an EOF1 payload S-MPDU. Valid zero-length EOF0 subframes may themselves end the VHT pre-EOF portion without a later payload/EOF1 delimiter; their bytes still count toward that portion's limit.

Sources: `ieee80211-2024:clause:9.2.2` continuation, PDF 659, span `3052920:3058243`; Table 9-659, span `7661283:7664971`; `ieee80211-2024:clause:10.12.6`, span `8237064:8242052`; `ieee80211-2024:clause:10.12.7`, PDF 1981, span `8242052:8244436`. Strength: reserved-field handling and transmit/order requirements. Observation: independent valid-CRC reserved-bit and delimiter-order fixtures. Receiver rejection of an HT EOF/Tag1 frame is the selected profile's validity policy rather than an additional explicit receiver-drop requirement.

### IEEE80211-2024-MESH-AGGREGATION-LIMITS

Respect the peer maximum aggregate length and the 4095-byte HT member limit. For VHT maximum aggregate exponent greater than three, the corresponding HT maximum exponent is three; otherwise they are equal. Honor the peer minimum MPDU spacing, adding zero-length delimiters as needed. VHT single-MPDU operation follows nonaggregate acknowledgment rules: Normal Ack can receive an immediate ACK without a Block Ack agreement, and Implicit BAR is forbidden.

Sources: `ieee80211-2024:clause:10.12.2`, PDF 1976–1977, source span `8220478:8226847`; `ieee80211-2024:clause:10.12.3`, PDF 1977–1978, source span `8226847:8229672`; `ieee80211-2024:clause:10.12.8`, PDF 1982, source span `8244436:8245258`. Strength: shall. Observation: negotiated limits, delimiters, timing and responses.

Minimum MPDU start distance is an integer octet distance at least tMMSS times the selected net PHY DATA rate divided by eight. Use the selected GI's rate, not coded gross rate, a long-GI reference or whole-PPDU throughput. Consecutive delimiter-start and MPDU-start distances are equal; include complete MPDU/FCS, alignment and the next delimiter. Add zero-length spacing delimiters in four-byte units until the ceiling is met. Source:10.12.3 Eq10-12, span `8226847:8229672`; Table 9-225 spacing codes, span `4984834:4986280`. Strength: minimum-distance rule. Observation: actual member start offsets and independent long/short-GI examples.

VHT timing distinguishes APEP_LENGTH from full PSDU_LENGTH. Construction submits its pre-padding APEP length for timing, obtains symbol capacity, then fills exactly PSDU_LENGTH with final-subframe alignment, zero-length EOF1 delimiters and trailing 0–3 octets. Alignment/trailing values are unspecified. These bytes must fit the already allocated raw DATA symbols; rounded whole-PPDU duration alone cannot prove that fit. The wire pre-EOF prefix structurally includes final-subframe alignment, while the procedural TX APEP snapshot can precede it; RX APEP is SIG-B Length multiplied by four.

Sources: `ieee80211-2024:clause:10.12.6`, span `8237064:8242052`; Figure 9-1327, span `7658257:7660513`; `ieee80211-2024:clause:21.4.3`, Eq21-111/112, span `15278503:15283577`; Table 21-1, PDF 3557, span `14829435:14834212`; `ieee80211-2024:clause:21.3.8.3.6`/Table 21-14, Eq21-46, span `15064458:15067777`. Original ceiling visually checked. Strength: normative construction/timing/parameter definitions. Observation: separate prefix/full extents, exact padding and raw symbol capacity.

The size wording has a boundary tension: 10.12.2/Table 9-313 binds capability to structural pre-EOF length, but construction and SIG-B rounding can turn a request of 8191 into a prefix/RX indication of 8192. No retrieved sentence expressly grants a capability-plus-alignment exception or requires rejection of that over-cap indication. The selected interpretation belongs in the model ledger, rather than being attributed as an unambiguous IEEE allowance. Table 9-313's exact size entry is PDF 1296, span `5610808:5615987`.

### IEEE80211-2024-MESH-BLOCK-ACK-RECOVERY

Match Block Ack transmitter, receiver and TID to the agreement. Applicable bitmap-one bits acknowledge corresponding unacknowledged members within the window; a later bitmap-zero does not undo a known acknowledgment. Do not alter members below the reported starting sequence in the specified retained interval. Do not retransmit an expired data frame; BAR can advance past an expired hole. Per-TID agreement inactivity resets on applicable data/BAR at the recipient and Block Ack at the originator; timeout causes DELBA with TIMEOUT. Sleep does not suspend these lifetimes. Block-Ack-policy data without an agreement is discarded and causes DELBA; a BAR without an agreement may be acknowledged and requires DELBA.

Sources: `ieee80211-2024:clause:10.25.6.7.1`, PDF 2068–2070, source span `8616401:8625055`; `ieee80211-2024:clause:10.25.6.8`, PDF 2071–2073; `ieee80211-2024:clause:11.5.4`, PDF 2566, source span `10834739:10836331`. Strength: shall, with may for optional BAR acknowledgment. Observation: agreement/window state and wire. PDF inspection is required for the originator pseudocode interrupted by Figure 10-42 in the text extraction.

## Mesh power save

### IEEE80211-2024-MESH-POWER-MODES

Individually addressed mesh Data or QoS Null signals active mode with PM=0, light sleep with PM=1/PSL=0 and deep sleep with PM=1/PSL=1. Lowering activity requires an acknowledged individual exchange. Higher activity may be indicated individually or through group traffic. A station sleeping for any peer uses deep sleep as its nonpeer mode; group/nonpeer signaling follows that derived mode. Each receiver updates from what it actually receives.

Sources: `ieee80211-2024:clause:14.15.3.1`–`14.15.3.3`, PDF 3279–3280. Strength: shall, with may for higher-activity mechanisms. Observation: wire and endpoint-local mode state.

### IEEE80211-2024-MESH-TIM

Every mesh beacon carries a TIM. Bits use the AIDs assigned by the reporting station during peering; AID 0 indicates buffered group traffic. DTIM beacons recur at DTIMPeriod. After its DTIM the station sends buffered group frames before individually addressed frames; More Data clears on the last group frame. Group Delivery Idle Time equals the default AC_VI TXOP limit for the PHY.

Sources: `ieee80211-2024:clause:14.15.4`, `ieee80211-2024:clause:14.15.5`, PDF 3280–3281; `ieee80211-2024:clause:9.4.2.5`. Strength: shall. Observation: TIM bytes, frame order and receiver awake state.

### IEEE80211-2024-MESH-AWAKE-WINDOW

An advertised mesh awake window begins at the actual end of the beacon or probe-response transmission. The element is mandatory in DTIM beacons and in sleeping-mode beacons indicating buffered peer traffic. Group transmission extends awake time by PostAwakeDuration, equal to the window duration, after the group frame. The transmitting station remains awake through all its group traffic and the final extension.

Source: `ieee80211-2024:clause:14.15.6`, PDF 3281. Strength: shall. Observation: advertised durations and actual radio timing.

### IEEE80211-2024-MESH-WAKE-DOZE

A nonpeer-deep-sleep station wakes before each own TBTT and stays through its beacon, awake window and group delivery. Light sleep also wakes before every corresponding peer TBTT; indicated individual traffic initiates a trigger with RSPI=1, and indicated group traffic holds it awake through completion or the specified idle time. Deep-sleep synchronization maintenance is a local choice. Doze requires all peer modes sleeping, no ongoing MPSP, no pending peering/authentication/passive-scan transaction, expired own awake window and completed DTIM group delivery.

Sources: `ieee80211-2024:clause:14.15.8.3`–`14.15.8.6`, PDF 3282–3283. Strength: shall for wake obligations, may for doze after all conditions. Observation: radio state and transactions.

### IEEE80211-2024-MESH-MPSP

A trigger's RSPI/EOSP combination establishes directional ownership as follows. A direction whose recipient is in active mode is excluded. Participants stay awake throughout service periods, which may span TXOPs. Successful termination requires the owner's terminating frame to be acknowledged; enqueue or an unconfirmed transmission is insufficient. A missing termination acknowledgment requires at least one retransmission subject to retry and lifetime limits; the missing-ACK cap is the minimum of MaxRetryLimit and MeshSTAMissingAckRetryLimit.

| RSPI | EOSP | Trigger transmitter owns | Trigger receiver owns |
| --- | --- | --- | --- |
| 0 | 0 | yes | no |
| 0 | 1 | no | no |
| 1 | 0 | yes | yes |
| 1 | 1 | no | yes |

Sources: `ieee80211-2024:clause:14.15.9.1`–`14.15.9.4`, `ieee80211-2024:table:14-33`, PDF 3283–3285. Strength: shall in the selected power-save mechanism. Observation: trigger/ACK exchanges and each endpoint's local directional state.
