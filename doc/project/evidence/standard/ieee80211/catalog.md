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

### IEEE80211-2024-MESH-HT-VHT-CAPABILITIES

HT capabilities distinguish receive support, transmit MCS support, aggregation limits and spacing. VHT uses separate transmit and receive MCS/NSS maps. A VHT transmitter must restrict the tuple, NSS and bandwidth to the recipient's supported receive set, including applicable highest-rate and excluded-tuple restrictions. VHT support implies HT support: VHT mesh configuration advertises coherent HT receive MCS and channel-width support, and HT/VHT operation indicates the BSS bandwidth. Joining requires transmit and receive support for all basic VHT tuples.

Sources: `ieee80211-2024:clause:9.4.2.54.2`–`9.4.2.54.4`, PDF 1123–1126; `ieee80211-2024:clause:9.4.2.156.3`, PDF 1298–1299; `ieee80211-2024:clause:10.6.13.1`, PDF 1964–1965; `ieee80211-2024:clause:10.6.13.3`; `ieee80211-2024:clause:11.38.1`, PDF 2883–2884; `ieee80211-2024:clause:11.38.7`, PDF 2891. Strength: shall when selected. Observation: capability/operation bytes, committed directional support and actual PHY tuple.

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

A station increments its own HWMP sequence number before discovery. A target answering a PREQ updates its own number to one more than the circular maximum of its current number and the requested target number. Other destinations' sequence history changes from received protocol information, except the specified next-hop failure increment. Comparisons use circular modulo-2^32 arithmetic.

Source: `ieee80211-2024:clause:14.11.8.3`, PDF 3233–3234. Strength: shall. Observation: wire and path state.

### IEEE80211-2024-MESH-PATH-UPDATE

PREQ establishes reverse information toward the originator; PREP establishes information toward its target. A fresher sequence number wins; equal sequence information replaces a path when its updated metric is better. Add the receiving station's outgoing cost toward the transmitter. PREP transmission validates the path toward its originator and PREP reception validates the path toward its target. Expiry or an unusable next hop invalidates the corresponding forwarding information.

Sources: `ieee80211-2024:clause:14.11.8.4`, `ieee80211-2024:table:14-9`, `ieee80211-2024:table:14-10`, PDF 3234–3235. Strength: shall. Observation: path state and wire.

### IEEE80211-2024-MESH-DISCOVERY

PREQ processing follows its target and flag cases; duplicate identity is not an unconditional discard rule. Processing can proceed when reverse information updates, or under the stated equal target sequence and previously unseen originator/discovery identity condition. PREP propagation and precursor changes require creation or update of target forwarding information; propagation requires remaining element TTL greater than one. Discovery retries wait at least twice the network diameter traversal time and each attempt uses a new Discovery ID and incremented HWMP sequence number.

Sources: `ieee80211-2024:clause:14.11.8.5`, PDF 3236; `ieee80211-2024:clause:14.11.9.4.3`, PDF 3245–3246; `ieee80211-2024:clause:14.11.10.4.3`, PDF 3249–3250. Strength: shall. Observation: wire, precursor/path state and timers.

### IEEE80211-2024-MESH-PREQ-CASES

Do not initiate a second ongoing discovery for the same target. Postpone an initiation until the target-specific PREQ minimum interval permits it. Initial on-demand PREQ uses group addressing, hop zero, configured network-diameter TTL, fresh Discovery ID, incremented originator sequence and metric zero; target sequence knowledge controls USN. Propagation increments hop count, decrements TTL and adds the local outgoing cost toward the transmitter, preserving originator sequence, Discovery ID and lifetime. An intermediate reply changes that target's TO to one in the propagated PREQ. In a multiple-target PREQ remove targets answered as self or proxy, retain other targets and set TO for intermediate replies; replying to one target does not terminate processing of all targets.

Sources: `ieee80211-2024:clause:14.11.9.3`, PDF 3236–3237, source span `13623746:13625185`; Tables 14-11 and 14-15–14-17, PDF 3237–3244, source spans `13625185:13629244`, `13640547:13648539`. Strength: shall within the corresponding case. Observation: control fields and per-target discovery state.

The receipt guard in 14.11.9.4.3 compares the target sequence in its second alternative. Propagation cases E1/E2/E3 in 14.11.9.3 compare the originator sequence and unseen originator/Discovery ID. These are separate guards on separate actions; the source wording must not be silently normalized to a single generic duplicate predicate.

### IEEE80211-2024-MESH-PREP-CASES

An intermediate reply requires TO=0, active forwarding information for the target, retained target sequence at least the requested sequence and nonzero lifetime. It uses retained target sequence/metric but copies the triggering PREQ lifetime. A target reply uses metric zero and its updated target sequence; its lifetime also comes from the PREQ. Propagation increments hop count, decrements TTL and adds outgoing-to-transmitter cost. Propagating a PREP adds the next hop toward the originator to the target's precursors and the next hop toward the target to the originator's precursors, with the applicable forwarding lifetimes. Intermediate PREQ replies establish the corresponding precursor relationships.

Sources: `ieee80211-2024:clause:14.11.10.3`, PDF 3246–3247, source span `13657925:13659777`; Tables 14-18–14-20, PDF 3247–3249, source span `13659777:13666300`; `ieee80211-2024:clause:14.11.10.4.3`, PDF 3249–3250; `ieee80211-2024:clause:14.11.9.4.3`, PDF 3245–3246. Strength: shall within the corresponding case. Observation: wire and precursor state.

### IEEE80211-2024-MESH-PERR

PERR processing concerns destinations whose selected next hop is the transmitter. For NO_FORWARDING_INFORMATION with sequence zero, increment the retained destination sequence and invalidate. Apply the general stale-sequence rejection and the reason-specific predicates in 14.11.11.4.3(c): NO_FORWARDING_INFORMATION with a nonzero sequence is a separate condition from DESTINATION_UNREACHABLE with a sequence strictly greater than retained information. In particular, do not impose the latter's strict-newer condition on both reasons without resolving the source wording. Propagate the applicable destination information to affected precursors, with the specified sequence and element-TTL rules. Do not invalidate unrelated paths or accept a PERR merely because the destination appears in it.

Sources: `ieee80211-2024:clause:14.11.11.4.2`, `ieee80211-2024:clause:14.11.11.4.3`, PDF 3254. Strength: shall. Observation: wire, selected next hop and precursor state. The [standards map](../../protocol/wifi/standards.md#interpretation-and-extraction-limits) records the table's cross-reference mismatch.

### IEEE80211-2024-MESH-ROOTS

Proactive PREQ and proactive RANN are separate root mechanisms and coexist with on-demand discovery. RANN announces root reachability; a receiver may initiate an individually addressed root-confirmation PREQ/PREP to establish a bidirectional path. The announcement alone does not complete that exchange. RANN comparison accepts an equal sequence number with equal metric; reject an equal-sequence worse metric. Root-confirmation addressing follows its own rules.

Sources: `ieee80211-2024:clause:14.11.7`, `ieee80211-2024:table:14-14`, PDF 3231–3232, 3240–3241; `ieee80211-2024:clause:14.11.12.4.2`, PDF 3256–3257. Strength: shall in selected root mode. Observation: wire and path validity.

### IEEE80211-2024-MESH-ROOT-PREQ

Root modes two and three periodically originate PREQ at the root interval, targeting broadcast with TO=1, USN=1 and target sequence zero. They use hop zero, metric zero, incremented root sequence, new Discovery ID and active-path-to-root lifetime. Mode three sets Proactive PREP; mode two clears it. A receiver sends a proactive PREP when the flag is set, or when it needs a bidirectional root path under the flag-zero case. The proactive PREP targets the responding station itself, uses its current own sequence and metric zero, and copies the root originator fields and lifetime. It is not an ordinary self-target reply that increments the target sequence.

Sources: `ieee80211-2024:clause:14.11.4.2`, PDF 3229–3230, source span `13595166:13597902`; `ieee80211-2024:table:14-13`, PDF 3239–3240, source span `13632749:13636271`; `ieee80211-2024:table:14-21`, PDF 3249, source span `13666300:13667819`. Strength: shall within selected root mode and reply condition. Observation: fields and proactive paths.

### IEEE80211-2024-MESH-RANN-PROPAGATION

Propagating a RANN requires validated forwarding information to the root, forwarding enabled, TTL greater than one, and at least the RANN interval since the previous RANN transmission. Increment hop count, decrement TTL, add outgoing-to-transmitter metric and preserve the announcement's interval and root sequence. Acceptance updates the RANN interval from the received field. An accepted announcement lacking validated root forwarding information cannot immediately propagate under this case.

Sources: `ieee80211-2024:clause:14.11.12.3`, `ieee80211-2024:table:14-26`, `ieee80211-2024:table:14-27`, PDF 3256, source span `13688771:13690742`; `ieee80211-2024:clause:14.11.12.4.3`, PDF 3257. Strength: shall within the propagation case. Observation: root path validation, timing and wire.

## Synchronization

### IEEE80211-2024-MESH-TSF

Neighbor-offset synchronization compares timestamp references at their specified transmit and receive instants, accounting for elapsed receive time. Mesh timing does not use the infrastructure TSF reset procedure. Required TSF slowing is bounded to 0.04% per beacon period. Beacon ordering and scheduling use the mesh rules.

Sources: `ieee80211-2024:clause:11.1.3.1`, PDF 2428–2429; `ieee80211-2024:clause:11.1.3.9`, PDF 2442–2443; `ieee80211-2024:clause:14.14.2.2.2`, `ieee80211-2024:clause:14.14.2.2.3`, `ieee80211-2024:clause:14.14.3.1`, PDF 3268–3270. Strength: shall. Observation: timestamps, timing state and actual transmissions.

## HT/VHT and aggregation

### IEEE80211-2024-MESH-AMSDU

Each mesh A-MSDU subframe contains mesh destination, mesh source, Length and its own Mesh Control. Mesh subframe Length is little endian and counts the MSDU, excluding Mesh Control; padding is determined from the complete mesh subframe. Basic A-MSDU layout does not substitute for this format.

Sources: `ieee80211-2024:clause:9.3.2.2.3`, `ieee80211-2024:figure:9-124`, PDF 775. Strength: normative format. Observation: encoding and constituent delivery.

### IEEE80211-2024-MESH-AMSDU-IDENTITY

Native mesh A-MSDU subframes share the same individual mesh source, including applicable outer source fields. Individual-address aggregates share the same individual mesh destination, including the applicable outer destination field; group-address aggregates use the outer group destination. The modulo-2^32 mesh sequence counter starts at zero and increments once for each new nonaggregated MSDU, A-MSDU or multihop MMPDU. Duplicate rejection concerns the entire received MSDU/A-MSDU/MMPDU identity, rather than treating sibling subframes of one A-MSDU as new duplicates. An existing A-MSDU expires when all its constituent MSDU lifetime timers have expired, permitting an older constituent to remain in that still-valid aggregate.

Sources: `ieee80211-2024:clause:10.11`, PDF 1971–1976, source span `8200504:8219198`; `ieee80211-2024:clause:9.2.4.8.3` continuation in Table 9-35, PDF 708, source span `3274376:3277715`; `ieee80211-2024:clause:10.37.3.1`, PDF 2145–2146, source span `8946794:8950174`; `ieee80211-2024:clause:10.37.6`, PDF 2150. Strength: shall. Observation: endpoints, sequence allocation, duplicate admission and aggregate expiry.

### IEEE80211-2024-MESH-AMPDU

Aggregated transport obeys the selected HT/VHT format, per-member MPDU integrity and delimiter rules. VHT single-MPDU transport still uses its required delimiter/EOF representation. Aggregate construction respects negotiated MPDU/A-MPDU limits, duration and spacing. Immediate Compressed Block Ack identifies received members; recovery preserves sequence identity and retransmits only eligible missing members within applicable retry and lifetime bounds.

Sources: `ieee80211-2024:clause:10.12`, `ieee80211-2024:clause:10.12.8`, PDF 1976–1982; `ieee80211-2024:clause:10.25`, PDF 2049 onward. Strength: shall when selected. Observation: bytes, PPDU/member state and upper delivery.

### IEEE80211-2024-MESH-AGGREGATION-LIMITS

Respect the peer maximum aggregate length and the 4095-byte HT member limit. For VHT maximum aggregate exponent greater than three, the corresponding HT maximum exponent is three; otherwise they are equal. Honor the peer minimum MPDU spacing, adding zero-length delimiters as needed. VHT single-MPDU operation follows nonaggregate acknowledgment rules: Normal Ack can receive an immediate ACK without a Block Ack agreement, and Implicit BAR is forbidden.

Sources: `ieee80211-2024:clause:10.12.2`, PDF 1976–1977, source span `8220478:8226847`; `ieee80211-2024:clause:10.12.3`, PDF 1977–1978, source span `8226847:8229672`; `ieee80211-2024:clause:10.12.8`, PDF 1982, source span `8244436:8245258`. Strength: shall. Observation: negotiated limits, delimiters, timing and responses.

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
