# Implementation plan: current HT support in VHT stations

Checkout at review: `/home/user/omnetpp_ws/inet` at `a61363aba5e9492f0c98a24905e050f0cf36966a`.

Plan revision: 10, 2026-10-13.

Scope authority: the user requests the current VHT implementation to match the current INET HT implementation's capabilities.

Status: local implementation, verification, and planned commits complete. The user's “Execute plan ieee80211-vht-ht-support.md” authorizes this work on 13 October 2026. The user's “Try now the planned commits” authorizes their creation after Git access changes. The [execution record](ieee80211-vht-ht-support-execution.md) contains the results and commit boundaries. The plan remains under `pending` until the change lands.

Evidence at review: current source inspection, configuration and test inspection, and retrieval of IEEE clauses from the local corpus. No build or simulation ran. The plan and its companion summary were untracked at review.

The design and review sections below preserve revision 10 at its review commit. Their proposed behavior and verification requirements now have separate execution evidence.

Current HCF consumes response estimates for single-protection forecasts and continuation decisions. The source review confirms that the prediction defect also affects the existing `n(mixed-2.4Ghz)` catalog. The response repair therefore precedes HT activation in a separate commit. The plan also specifies an explicit role enum, rate-control width checks, and documentation of equal-rate adaptation steps.

## 1. Intended behavior and boundary

An INET interface with `opMode = "ac"` will retain VHT support and gain the HT support that INET already implements. HT means high throughput, introduced by IEEE 802.11n. VHT means very high throughput, introduced by IEEE 802.11ac. The reference for implementation scope is the current `n(mixed-2.4Ghz)` profile and its production consumers. The VHT interface will use the corresponding 5 GHz HT modes, because its operating band differs from that profile.

The change will provide real HT modes, local HT capabilities, HT management elements, existing HT peer checks, and actual HT transmission and reception. It will retain the current HT implementation's limits. In particular, the current packet-level PHY advertises only 20 MHz HT support. A catalog entry for 40 MHz will not establish operational 40 MHz support.

**Proposed example:** Two `ac` interfaces use a supported 5 GHz channel and one antenna each. Full management exchanges the existing HT elements and commits HT peer state. The configuration selects a supported 20 MHz HT data mode. The originator sends an HT frame, and the peer receives and delivers it. The local HT advertisement contains MCS 0–7 and excludes 40 MHz support, just as the current one-antenna HT configuration does. A separate case selects an existing 20 MHz VHT mode and checks transmission, reception, and upper-protocol delivery.

The supported radio path uses `Ieee80211Transmitter` and `Ieee80211Receiver`. Derived components must satisfy those contracts. HT activation enters the MAC's existing concrete-type checks. Incompatible replacement components fail initialization, as they do for current HT. This change adds no generic radio capability interface.

MCS means modulation and coding scheme. A PHY mode identifies a PHY format and its transmission parameters. Station capability and PHY format remain separate concepts. A VHT mode keeps its VHT format, duration calculation, and modulation class. It does not acquire an HT MCS index or inherit HT frame semantics through a class change.

This plan does not promise automatic VHT-to-HT fallback from a VHT rate request. The current VHT implementation lacks peer VHT negotiation; this revision preserves that limitation. Tests select HT explicitly when they establish the added HT path.

## 2. Standard motivation and retained limitations

IEEE Std 802.11-2024, Clause 11.38.1, requires VHT stations to implement HT. Clause 21.1.1 also requires VHT stations to transmit and receive the mandatory 5 GHz HT PHY formats. These clauses explain why an INET VHT interface needs HT capability. They do not make this limited change a conformance implementation.

The standard's requirements and this change's limits are distinct:

| IEEE requirement | Current evidence and boundary |
| --- | --- |
| Clause 11.38.1 requires 20/40 MHz HT capability. | INET's HT capability queries permit only 20 MHz. This change retains that declared limitation. |
| Clause 11.38.1 relates HT MCS support to VHT receive maps. | This change derives HT support from the existing catalog, antenna limit, and radio queries. It adds no VHT receive-map negotiation. |
| Clause 21.1.1 requires 20, 40, and 80 MHz VHT operation. | The existing catalog includes wider VHT modes. `Ac_PhyMode` checks an 80 MHz VHT mode request at the MAC output. This assertion establishes neither PHY transmission nor reception. |
| Clause 4.3.13 identifies HT stations as QoS stations. | This change retains INET's explicit QoS configuration. |
| Clause 10.6.6.1 requires a non-HT PPDU for ordinary ACKs when none of its listed exceptions applies. | The proposed default HT ACK retains a known INET violation. It is a compatibility expectation, not a standards requirement. |
| Clause 10.6.6.1(a)(2) requires an HT response to an HT RTS. | Unknown advertised Tx capability does not permit a forced non-HT CTS. This plan preserves actual response selection in that case. |
| Clause 10.6.6.1 requires non-HT control formats unless a listed condition applies. | The plain VHT RTS with HT CTS retains an INET format violation. The HT BAR with HT Block Ack retains another. An HT BAR alone does not permit an HT Block Ack response. |
| Clauses 10.6.6.5.2 and 10.6.11 derive the primary non-HT response rate from the data frame's mode and basic rates. | VHT MCS 0 has a 6 Mbps reference rate. The inherited 12 Mbps fallback in Section 4 does not implement that calculation. |
| Table 19-6 specifies 4 μs symbols for long GI and 3.6 μs symbols for short GI. | Current HT duration calculation uses 4 μs for both. This change retains that defect and makes no short-GI airtime conformance claim. |
| Clauses 10.23.2.8 and 10.23.2.9 constrain TXOP continuation through TXNAV and the TXOP limit. | Current HCF consumes response estimates for both checks. The proposed continuation fixture isolates the TXOP limit with an explicit test control for TXNAV. |

The current VHT path lacks peer VHT negotiation. `Ac_VhtIe` retains its expected failure for the absent VHT Operation element. `Ac_BlockAck` checks the existing Basic Block Ack path. It does not establish Compressed Block Ack support. This task includes Basic Block Ack response prediction, but preserves agreement procedures, wire formats, and aggregation behavior. It makes no additional A-MPDU support claim.

The [HT bitrate calculation](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.cc) recognizes short GI. Its data duration method nevertheless uses the long-GI symbol interval from [Ieee80211HtMode.h](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h). The [transmitter](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.cc) uses that duration for actual transmission. For the proposed MCS 0 fixture, short and long GI therefore produce identical model airtime. Agreement between predicted and actual durations does not establish standards compliance. This baseline defect remains outside the required integration repair.

Use [the local base standard](../../standards/80211ax-2024.pdf) for these citations. Its filename does not identify its actual revision. The document ID is `ieee80211-2024`. The evidence below comes from normative clauses in the local corpus.

| Canonical corpus node | Physical PDF pages | Use in this plan |
| --- | --- | --- |
| `ieee80211-2024:clause:11.38.1` | 2883–2884 | The VHT-to-HT relationship and the wider capability requirements. |
| `ieee80211-2024:clause:21.1.1` | 3551–3552 | Actual HT PHY support in VHT stations and wider VHT requirements. |
| `ieee80211-2024:clause:4.3.13` | 354 | The standard's HT and QoS relationship. |
| `ieee80211-2024:clause:19.1.1` | 3401–3402 | Band-specific HT and legacy PHY behavior. |
| `ieee80211-2024:clause:17.3.8.4.2` | 3370–3371 | Channel-number conversion, Equation (17-27). |
| `ieee80211-2024:table:E-4` | 5657–5664 | Normative Annex E: class 115 supplies the 20 MHz channels; classes 116 and 117 supply valid 40 MHz pairs. |
| `ieee80211-2024:clause:11.15.2` | 2630–2631 | A 20 MHz-only STA can operate in a 20/40 MHz BSS. |
| `ieee80211-2024:table:21-22` | 3636–3637 | Distinct primary and wider-channel centers; the wider-channel model remains outside scope. |
| `ieee80211-2024:clause:10.6.6.1` | 1942–1943 | Conditions that permit HT or VHT control responses; ordinary ACKs require non-HT PPDUs. |
| `ieee80211-2024:clause:10.6.6.5.2` | 1945–1947 | The primary response rate and modulation class. |
| `ieee80211-2024:clause:10.6.6.5.4` | 1950 | An alternate response rate must preserve the primary response duration and satisfy the other stated conditions. |
| `ieee80211-2024:clause:10.6.11` | 1962–1963 | Conversion from an MCS to a non-HT basic rate. |
| `ieee80211-2024:table:10-10` | 1963 | BPSK with code rate 1/2 maps to a 6 Mbps non-HT reference rate. |
| `ieee80211-2024:clause:10.17` | 1984–1987 | Short-GI HT transmission requires the intended receiver's advertised support for that width, subject to the clause's other conditions. |
| `ieee80211-2024:table:19-6` | 3421–3422 | Long- and short-GI symbol intervals; current HT duration calculation retains a known defect. |
| `ieee80211-2024:clause:10.23.2.8` | 2010–2012 | Continuation requires the exchange duration to be strictly less than TXNAV. |
| `ieee80211-2024:clause:10.23.2.9` | 2012–2014 | TXOP containment includes the interval before the next exchange. |

The current review retrieved every node in this table and confirmed its physical page range. The inspected references from Clauses 11.38.1, 21.1.1, and 10.6.6.1 resolve. The corpus has unrelated lint findings. Direct PDF inspection was unnecessary for these claims.

## 3. Current behavior and required integration

| Owner | Current source behavior | Proposed change |
| --- | --- | --- |
| [Mode catalog](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.cc) | `ac` contains VHT and legacy OFDM modes. It has no HT entries and leaves `htOperationSupported` false. | Add the current HT catalog's combinations for the 5 GHz band. Enable its existing HT capability flag. |
| [MAC](../../src/inet/linklayer/ieee80211/mac/Ieee80211Mac.cc) and [MIB](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.cc) | The MAC queries radio HT support. The MIB derives capability from HT entries, supported widths, and the antenna limit. | Let `ac` enter this existing path. Preserve its ownership, initialization stages, and capability restrictions. |
| [Management](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.cc) | The existing HT capability controls HT element insertion. Association commits and removes HT peer state. | Use the same path for `ac`, with valid 5 GHz channel conversion. |
| [Receiver](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.cc) | Reception requires the transmitted mode to occur in its mode set. Its HT width query permits only 20 MHz. | Added HT entries permit HT reception. Preserve the current 20 MHz advertised and negotiated HT capability. |
| [Transmitter](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.cc) | It validates requested mode membership and antenna capacity. Its HT width query permits only 20 MHz. | Added HT entries permit HT transmission through the existing mechanism. |
| [Peer selector](../../src/inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.cc) | HT candidates use committed HT peer capabilities. VHT candidates bypass those checks. | Added HT candidates use the same checks. Preserve the current VHT selection path. |
| [QoS response selector](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) and [DCF response selector](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelection.cc) | Both use the transmitter address from the frame that requires a response. Sender prediction therefore queries its own address or an unset address. | Add an explicit caller role to ACK, CTS, and Basic Block Ack queries. Select the peer and capability direction for that role. |
| [Band representation](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h) | Existing 5 GHz arithmetic bands lack standard channel-number conversion. The base HT40 check validates channel separation but not permitted pairs. | Supply a mapped 5 GHz band through the existing enumerated-band representation. Restrict its HT40 pairs to 36/40 and 44/48. |

The [HT catalog test](../../tests/unit/Ieee80211HtModeSet_1.test) currently asserts that `ac` lacks HT support. The [HT association test](../../tests/module/Ieee80211HtAssociation_1.test) verifies a 20 MHz-only advertisement despite 40 MHz catalog entries. These are the direct reference points for the changed flag and the preserved radio limit.

## 4. Proposed design

### Add the existing HT combinations

Add HT MCS 0–31 at 20 and 40 MHz, with long and short guard intervals, to `ac`. These are the combinations in the current HT catalog. Construct them through `Ieee80211HtCompliantModes` with `BAND_5GHZ` and the mixed HT preamble. Use the existing compliant mode objects; do not create duplicate objects with equivalent parameters.

Preserve the current HT mandatory-entry rule: only long-GI, 20 MHz MCS 0–7 are mandatory in the inherited HT subset. Do not introduce a new role-dependent mandatory policy. Keep the existing Basic HT-MCS Set derivation. That preserves current HT behavior without a separate basic-set redesign.

Keep every existing VHT and legacy OFDM entry, its flags, and its relative order. Append the HT subset after the existing entries before the constructor's stable bitrate sort. For an exact equal-rate lookup, the existing VHT entry retains precedence over a new HT entry with the requested constraints. This guarantee covers forward fixed-rate lookup. Backward mandatory-mode searches have the different result described below. Keep `PhyType::VHT` and the existing VHT reference mode, which supplies the timing anchor.

Reuse a private construction helper only if it removes actual duplicate HT construction. Do not add a public capability hierarchy, a format-selection parameter, or a new HT-only 5 GHz profile for this task. The existing HT profile remains the comparison reference; the added production path can use two `ac` interfaces.

### Reuse capability and management state

The MIB will derive HT capabilities from actual HT entries through its current method. For a one-antenna interface, it must expose MCS 0–7. For additional antennas, it must expose the same supported groups as the current HT implementation. HT support stops at four spatial streams even if the VHT catalog or antenna configuration permits more. A spatial stream is an independent stream that a multiple-antenna PHY carries.

Preserve the current 20 MHz advertised and negotiated HT capability. Do not force the HT 40 MHz advertisement bit to satisfy the broader standard requirement. Preserve short-GI capability only for the operationally supported HT widths. GI means guard interval, the interval between PHY symbols.

The radio's HT width queries supply the MIB capability assembly. Existing peer checks apply that capability to negotiated HT traffic. These queries do not constitute a universal transmitter rejection of wider HT modes. For example, a direct request for a cataloged 40 MHz HT mode encounters no separate width rejection in `Ieee80211Transmitter::createTransmission()`. This inherited behavior establishes neither correct 40 MHz operation nor an additional requirement to reject such requests. This task adds no transmitter width guard.

Ordinary unicast rate control passes through an existing width check. [AARF](../../src/inet/linklayer/ieee80211/mac/ratecontrol/AarfRateControl.cc) and [Onoe](../../src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.cc) return a catalog candidate through `getRate()`. Both [RateSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelection.cc) and [QosRateSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) pass that candidate to `getPeerCompatibleMode()` before they return the data mode. The MAC supplies the selected mode to the transmitter through `Ieee80211ModeReq`.

The [shared HT helper](../../src/inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.cc) checks the negotiated width set and HT Operation width. With valid 20 MHz-only state, it replaces a 40 MHz HT candidate with a compatible HT mode or legacy fallback. With absent or invalid peer state, an HT candidate falls back to legacy. These results assume the normal MIB connection and production capability assembly. Catalog traversal therefore establishes no 40 MHz transmission defect through this path, in either the existing `n` configuration or the proposed `ac` configuration. Section 6 requires direct checks of this path; direct PHY requests remain outside that claim.

Use existing HT management elements, serializers, association transactions, and teardown methods. Full management must commit HT peer state at the same successful exchange boundary as the current HT implementation. Simplified management must install the same HT state through its existing abstraction, without an on-air negotiation claim. Do not add VHT capability records, VHT management elements, timers, events, or lifecycle state.

### Provide channel numbers for the existing HT management path

HT Operation carries a standard channel number. INET's internal channel index is a separate value. The current arithmetic bands inherit conversion methods that throw, so HT activation would otherwise introduce an initialization failure.

Add `5 GHz (mapped 20 MHz)` through `Ieee80211EnumeratedBand` and the existing band lookup. Use this subset from operating class 115 in Table E-4. Equation (17-27) gives each center as `5000 + 5 * channel` MHz.

| Internal index | Standard channel number | Center frequency |
| --- | --- | --- |
| 0 | 36 | 5180 MHz |
| 1 | 40 | 5200 MHz |
| 2 | 44 | 5220 MHz |
| 3 | 48 | 5240 MHz |

The subset is an INET scope choice, not a complete channel or regulatory-domain model. Record it in the band declaration, tests, and NED descriptions. Add the name to the radio, transmitter, and receiver band enums. Use the existing `Ieee80211Band.h` and `.cc` catalog. No new band class or conversion interface is necessary.

The mapped band must reject the adjacent pair 40/44 for HT40 operation. The inherited `isHt40OperationSupported()` accepts any mapped pair with the expected 20 MHz separation. That test would accept 40/44 in either direction. Table E-4 classes 116 and 117 permit 36/40 and 44/48 within this subset. Channel separation alone therefore cannot establish a valid pair.

Extend the enumerated-band constructor with an optional list of HT40 pairs, expressed as standard channel numbers in ascending order. The mapped band supplies `{36, 40}` and `{44, 48}`. Override the existing virtual `isHt40OperationSupported()` in `Ieee80211EnumeratedBand`. Retain the base offset, bounds, and frequency checks first. For a nonempty list, require the normalized pair to occur in that list. An empty list preserves the existing geometric check for existing bands, including 2.4 GHz.

This list is fixed catalog data, with no new configuration or regulatory policy interface. It restricts this band's existing validation method; it does not enable 40 MHz transmission. [Ieee80211MgmtSta](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) uses that method to validate received HT Operation elements. A 20 MHz-only STA can receive such elements from a 20/40 MHz BSS under Clause 11.15.2. Thus, the local 20 MHz limit does not remove this receive-path requirement.

Keep the current arithmetic band definitions and their frequencies unchanged. HT activation makes AP initialization require channel conversion even when the configured data format remains VHT. An unmapped band therefore fails this HT management contract. This also affects simplified management through the shared AP base. Do not round a nonstandard center frequency to an invented channel number.

Migrate these three configuration sources in the same commit as HT activation:

| Configuration | Current band and effective initial center | Required migration and retained claim |
| --- | --- | --- |
| [Shared `ac` protocol configuration](../../tests/protocol/wifi/ini/_ac.ini) | `5 GHz (80 MHz)`, index 0, 5040 MHz. | Set the mapped band. Set radio `channelNumber = 0`. Set configured `centerFrequency = 5.18GHz`. Preserve `Ac_PhyMode`'s 292.5 Mbps configuration and its assertion of an 80 MHz VHT mode request. |
| [`lan80211ac` example](../../examples/wireless/lan80211ac/omnetpp.ini) | `5 GHz (20 MHz)`, index 0, 5010 MHz. | Set the same mapped band, index, and configured center. Preserve 693.3 Mbps and eight antennas. Check initialization; do not infer successful delivery from initialization. |
| [`Ac_160mhz.test`](../../tests/protocol/wifi/11ac/Ac_160mhz.test) | Its explicit `5 GHz (160 MHz)` override selects index 0 at 5080 MHz. | Remove the band override. Remove its obsolete precedence comment. Inherit the mapped band, index 0, and 5180 MHz center from `_ac.ini`. Preserve the configured rate of 650 Mbps, width of 160 MHz, and one-stream constraint. Preserve the assertion of a 160 MHz VHT mode request. |

The `Ac_160mhz.test` assignment precedes the `_ac.ini` include and takes precedence over the shared band assignment. A shared-file migration alone leaves this test on an unmapped band. Its AP would fail initialization after HT activation.

The old `centerFrequency = 5GHz` settings do not establish the effective centers: radio channel configuration supplies those centers. The migration deliberately changes the effective frequencies. It does not preserve propagation results or fingerprints automatically. Check for additional inherited `ac` configurations before implementation. Apply the same migration wherever HT activation introduces this conversion failure.

The mapped band's name identifies its 20 MHz channel centers; it does not constrain the selected VHT mode's channel width. The existing flat radio uses one channel center. The migrated 80 MHz and 160 MHz configurations retain that approximation at 5180 MHz. Table 21-22 illustrates an 80 MHz center at 5210 MHz with primary channel center 5180 MHz. Its 160 MHz example uses center 5250 MHz and primary channel center 5280 MHz. These examples distinguish the primary channel center from the wider channel center.

This plan adds neither that primary/secondary relationship nor a claim of wider-channel reception. The new HT and VHT delivery controls use 20 MHz.

The wider VHT controls observe `Ieee80211ModeReq` at the MAC's `packetSentToLower` simulation signal. [MacProtocolBase::sendDown()](../../src/inet/linklayer/base/MacProtocolBase.cc) emits that notification before it sends the frame to the radio. [WifiTestSupport::isVht()](../../tests/protocol/wifi/WifiTestSupport.h) checks the requested format, channel width, and a minimum spatial stream count. Its argument `1` means at least one stream, not exactly one stream. The tests do not directly assert PHY transmission or the configured data rate. Preserve these checks as evidence of mode selection; use the new 20 MHz fixtures for actual transmission, reception, and delivery.

### Preserve catalog selection and resolve response prediction

Use existing HT peer filtering when the selected candidate is an HT mode. Keep VHT candidate handling unchanged. Do not infer that HT peer state proves VHT support or add automatic cross-generation negotiation.

Check fixed-rate lookup, mandatory defaults, and adaptive traversal after catalog expansion. The existing relative order resolves exact HT/VHT rate ties in favor of the existing VHT entry. Adaptive traversal may encounter the new HT modes; that is an intended consequence of the expanded mode set. A new format preference policy is unnecessary for this scope.

The catalog's `getFasterMode()` and `getSlowerMode()` methods select adjacent entries, even when their data rates are equal. For VHT MCS 0 at 20 MHz, one stream, and long GI, the proposed next entry is the equivalent HT MCS 0. Both modes supply 6.5 Mbps. A reverse step from that HT entry returns the VHT entry. Add these two assertions to `Ieee80211HtModeSet_1.test`. They check catalog traversal, not rate-control convergence or automatic VHT fallback.

Document the adaptation effect in the HT activation commit's WHATSNEW entry. AARF and Onoe advance through adjacent entries after their respective thresholds or intervals. Added equal-rate entries can therefore change the adaptation path without an immediate increase in nominal data rate. Equal nominal data rates do not establish equal airtime or throughput. The magnitude of the adaptation effect remains unmeasured. The plan requires no performance claim, new format preference, or deduplication policy.

Any later performance comparison must record the actual transmitted mode and the controller's raw candidate separately. The controller's data-rate notification describes its candidate before the peer filter applies. That notification alone cannot establish the transmitted rate or throughput.

**Proposed compatibility decision:** Retain the catalog's response-candidate search, subject to the prediction repair below. Do not require preservation of an algorithm that produces an incorrect exchange estimate. [ACK selection](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) searches backward for a mandatory mode when the data frame's mode is nonmandatory. The backward search encounters a new HT entry before an equal-rate VHT entry. The DCF selector uses the same catalog operation.

For example, existing 20 MHz VHT MCS 0 with two streams and short GI supplies approximately 14.444 Mbps. Its default ACK currently uses the mandatory 13 Mbps VHT mode. After expansion, the backward search supplies mandatory HT MCS 1 at 13 Mbps. At the recipient, compatible committed HT state retains that candidate. Without that state, the existing recipient check selects legacy OFDM at 12 Mbps.

These response formats are retained INET limitations. For an ordinary DATA/ACK exchange without the exceptions in Clause 10.6.6.1, the standard requires a non-HT ACK. VHT MCS 0 uses BPSK with code rate 1/2, so Table 10-10 gives a 6 Mbps reference rate. With the current basic rates of 6, 12, and 24 Mbps, the primary ACK rate is 6 Mbps. The 12 Mbps fallback uses a different calculation; it is not the normative primary rate. Clause 10.6.6.5.4 also requires an alternate rate to preserve the primary response duration.

**Required integration repair:** Sender prediction and recipient response must use the appropriate direction of the same exchange. Consider associated `ac` stations A and B with compatible HT state. A sends the 14.444 Mbps VHT data frame to B. Both response selectors use the data frame's transmitter address, A, for the peer check. B finds its peer record for A and selects the 13 Mbps HT ACK. A queries its own address, finds no peer record, and predicts a 12 Mbps OFDM ACK.

The defect also exists before HT activation in `ac`. With associated `n(mixed-2.4Ghz)` stations, HT MCS 1 at 20 MHz and long GI supplies a mandatory 13 Mbps response candidate. The recipient can retain that HT candidate, while the sender's self-address lookup selects the 12 Mbps ERP-OFDM fallback. This example requires compatible peer state at the recipient. If both endpoints lack usable peer state, both can select legacy responses. The separate prerequisite commit uses the `n` example as its direct reproduction.

The mismatch changes the sender's estimate of the ACK duration. [OriginatorProtectionMechanism](../../src/inet/linklayer/ieee80211/mac/protectionmechanism/OriginatorProtectionMechanism.cc) and [SingleProtectionMechanism](../../src/inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.cc) use that estimate for Duration/ID. [Hcf](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) also uses it for exchange duration. The [QoS ACK policy](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.cc) and [DCF ACK policy](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorAckPolicy.cc) use the predicted response mode for timeout calculation. Before expansion, the VHT candidate bypasses HT peer checks. The added HT candidate therefore exposes this defect for existing VHT traffic.

Current HCF also uses response estimates to forecast a queued successor and decide whether a TXOP can continue. A transmission opportunity (TXOP) is a time interval in which a QoS station has the right to start frame exchange sequences. TXNAV is the transmit-side reservation timer. With single protection and Block Ack disabled, HCF compares the complete exchange cost against TXNAV and the TXOP time left. An underestimated response can change the forecast or admission decision. Successful completion of an isolated exchange does not test those boundaries.

The same defect affects CTS prediction. With `controlFrameBitrate` at approximately 14.444 Mbps, a QoS originator can send RTS in the same nonmandatory VHT mode. The expanded catalog supplies a 13 Mbps HT CTS candidate. An associated recipient can retain that candidate while the sender predicts legacy OFDM at 12 Mbps. RTS Duration/ID and CTS timeout calculations consume that prediction.

This VHT-RTS/HT-CTS example retains INET format violations. The current RTS builder supplies no HT Control field. Clause 10.6.6.1(e) requires that field with MRQ set for its mandatory VHT response condition. Clause 10.6.6.1(a)(2) instead requires an HT response to an HT RTS. The proposed VHT RTS fixture does not exercise that requirement. The direct CTS fixture uses an HT RTS input for that separate check.

The caller role cannot depend on a comparison between the frame's transmitter address and the local address. [RtsProcedure::buildRtsFrame()](../../src/inet/linklayer/ieee80211/mac/originator/RtsProcedure.cc) initially sets only the receiver address. [Tx](../../src/inet/linklayer/ieee80211/mac/Tx.cc) supplies the transmitter address after the originator calculates RTS Duration/ID. An address-based role test therefore fails at a real prediction call.

### Make the response query role explicit

Add the required argument `ResponseQueryRole role` to each response-mode query, with no default value. Define `enum class ResponseQueryRole { PREDICT, RESPOND };` once in [Ieee80211Defs.h](../../src/inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h). Both rate-selection contracts include that shared definition. The named values make each caller's intent explicit without new runtime configuration or state.

The affected contracts are [IRateSelection](../../src/inet/linklayer/ieee80211/mac/contract/IRateSelection.h) and [IQosRateSelection](../../src/inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h). Both declare ACK and CTS queries. The QoS contract also declares the Basic Block Ack query.

The frame argument always identifies the frame that requires a response. Its receiver address must be available for sender prediction. Its transmitter address must be available for a response to a received frame. The role determines the peer lookup and the direction in the existing negotiated state:

| Query role | Argument | Peer record | Capability direction | Receiver of the response |
| --- | --- | --- | --- | --- |
| The sender predicts the peer's response to its outgoing frame. | `ResponseQueryRole::PREDICT` | Frame receiver address | `localRxPeerTx` | The local station |
| The recipient selects its response to a received frame. | `ResponseQueryRole::RESPOND` | Frame transmitter address | `localTxPeerRx` | The peer station |

Update `RateSelection.h/.cc` and `QosRateSelection.h/.cc` with these signatures and semantics. Preserve each selector's response-candidate calculation and configured response-rate override. Apply the directional check after either source supplies the candidate. Preserve mode request and mode indication tags as sources of the mode for the frame that requires a response. Neither tags nor address values determine the query role.

Reuse [Ieee80211PeerModeSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.cc) for the compatibility calculation. Add `selectPeerCompatibleResponseMode()` there, with the existing selection inputs and the explicit role. Share its internal candidate scan, tie rules, width check, GI check, and legacy fallback with the current helper. The shared calculation takes the selected directional capability and the existing HT Operation state. Ordinary data selection retains `selectPeerCompatibleMode()` and its current `localTxPeerRx` behavior.

Null and non-HT candidates retain current behavior. An HT candidate requires valid peer state and a usable capability in the selected direction. The helper retains a compatible candidate or selects the best compatible HT mode at or below its data rate. If no such mode exists, the existing legacy fallback applies. The helper adds no MIB fields, copied peer records, timers, or response-mode cache.

Unknown Tx capability limits prediction. The current [HT negotiation helper](../../src/inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h) can mark `localRxPeerTx` valid while its exact Tx MCS bitmap remains unknown. This occurs when `txMcsSetDefined` is false or `txRxMcsSetNotEqual` is true in the peer advertisement. The predictor cannot establish an HT candidate from that bitmap, so the existing legacy fallback applies. The actual responder still uses its local Tx knowledge through `localTxPeerRx` and can select HT.

Preserve that distinction and the unknown capability state. Do not reject association or force the actual responder to use legacy solely because its advertised Tx bitmap is unknown. Such a rule would introduce a separate response policy. It would also conflict with Clause 10.6.6.1(a)(2) when an HT RTS requires an HT CTS. The prediction can therefore differ from the actual response in this retained limitation. A legacy prediction in this case is not a proved upper bound on response duration.

Exact prediction requires these conditions:

- Both stations use identical catalogs and response configuration.
- Both stations have mutually consistent peer advertisements and HT Operation state.
- The responder advertises a defined Tx set equal to its Rx set, so the predictor knows its exact Tx bitmap.
- The selected capability remains stable across the exchange.

Current built-in capability assembly supplies that Tx and Rx advertisement. These conditions define the response agreement that this plan tests.

Unknown responder Tx capability, independent response overrides, and one-sided or stale peer state remain outside the exact-prediction guarantee. This change adds no remote override discovery, capability inference, or response-duration bound for those cases. With absent or invalid peer state, each query retains the existing legacy fallback independently. Full prediction support for unknown Tx capability requires a separate design if that use case enters scope.

All production consumers must supply the role explicitly:

| Consumer | Required role and use |
| --- | --- |
| `OriginatorProtectionMechanism` and `SingleProtectionMechanism` | Use `ResponseQueryRole::PREDICT` for ACK, CTS, and Basic Block Ack duration estimates wherever those response types apply. |
| `OriginatorAckPolicy`, `OriginatorQosAckPolicy`, `RtsPolicy`, and `QosRtsPolicy` | Use `ResponseQueryRole::PREDICT` for response modes that supply timeout calculations. |
| `Hcf::estimateZeroLimitExchange()` | Use `ResponseQueryRole::PREDICT` for each response estimate. |
| `Hcf::computeImmediateResponseDuration()` | Use `ResponseQueryRole::PREDICT` for the ACK estimate in single-protection exchange costs. |
| `Hcf::forecastSingleProtectionSuccessor()` | Use `ResponseQueryRole::PREDICT` for the CTS estimate for a queued successor. Its ACK estimate uses `computeImmediateResponseDuration()`. |
| `Hcf::transmitFrame()` | Use `ResponseQueryRole::PREDICT` for the CTS estimate in the single-protection branch. Its DATA/ACK cost uses `computeImmediateResponseDuration()`. |
| `Dcf::transmitControlResponseFrame()` and `Hcf::transmitControlResponseFrame()` | Use `ResponseQueryRole::RESPOND` for actual response selection. |
| `RecipientAckPolicy`, `RecipientQosAckPolicy`, `CtsPolicy`, and `QosCtsPolicy` | Use `ResponseQueryRole::RESPOND` for response duration calculations. |

Update the test selectors in these fixtures:

- `Ieee80211ResponseTimeout_1.test`.
- `Ieee80211HcfMultipleProtection_1.test`.
- `Ieee80211HcfSingleProtection_1.test`.
- `Ieee80211SingleProtectionGroup_1.test`.
- `Ieee80211Txnav_1.test`.

Their overrides must accept the role. Methods that call another selector must forward it. Update direct response calls in `Ieee80211MgmtStaDiscovery_1.test` to use `ResponseQueryRole::RESPOND`. The changed signatures also require external selector implementations and callers to supply the role. Missing overrides or call arguments break compilation because the contract supplies no default.

Document that source compatibility change in the prerequisite commit's migration guidance and WHATSNEW entry. State which role each external caller must supply and how selector overrides accept and forward the argument. Do not retain an ambiguous overload. The shared selectors also serve current HT configurations, so their sender estimates can change after the repair. Include those configurations in regression selection; the repair is not conditional on `opMode = "ac"`.

Basic Block Ack needs the same role because its response query shares the faulty peer lookup. Include its duration and timeout consumers in the repair. Preserve its candidate rule, agreement state, and wire format. Direct production-query checks cover this contract; no new Block Ack network fixture is necessary.

The proposed associated examples retain 13 Mbps HT ACK and CTS responses with the same modes in sender estimates. Those results retain the response-policy limitations stated above. Tests must compare predicted and actual response durations. Tests must check the applicable Duration/ID reservation. Tests must establish successful exchange completion. Both selector classes need direct checks through their production methods.

These are source-derived findings and proposed results, not executed evidence. The review established no runtime timeout or packet loss. The fastest mandatory mode remains the existing 78 Mbps VHT entry; the added mandatory HT subset stops at 65 Mbps.

Existing Beacon, multicast, QoS, and Block Ack limitations remain outside this change, except for the specified response-query repair. General control-response conformance work also remains outside scope. The prediction mismatch above is a required integration repair. Other repairs require a concrete regression or integration failure caused by the added HT path. Record each repair through its cause and direct test.

For example, use a supported legacy management rate in the HT delivery fixture, as existing rate configuration permits. That isolates the new HT data path from the known default management-rate limitation. The fixture does not establish corrected Beacon rate selection. Keep a separate response case without an ACK-rate override. Otherwise, the fixture would hide the catalog's effect on default responses.

## 5. Implementation steps

Each step includes its directly related tests and compatibility documentation. Step 1b contains the response-query contract, implementations, and callers in one buildable prerequisite commit. It needs neither HT activation in `ac` nor the channel migration. This separation follows [PR-SPLIT-PREPARE](../../doc/project/rule/pull-request.md#pr-split-prepare). The prerequisite can remain in the same PR. Step 2 keeps HT activation and its required configuration migration together.

| Step | Owner and change | Expected result and verification |
| --- | --- | --- |
| 1. Add channel conversion required by HT management. | Extend `Ieee80211Band.h` and `.cc` with the mapped band and permitted HT40 pairs. Update `Ieee80211Radio.ned`, `Ieee80211Transmitter.ned`, and `Ieee80211Receiver.ned`. Extend `Ieee80211Band_1.test` and `Ieee80211MgmtStaDiscovery_1.test`. | Unit checks verify channel conversions and permitted pairs. Receive-path checks reject 40/44 while valid pairs remain acceptable. The step 2 fixture verifies HT Operation channel conversion for `ac`. |
| 1b. Repair response prediction for existing HT. | Add the shared role enum, response-query signatures, directional helper, callers, and test overrides from Section 4. Add the `n` cases in `Ieee80211ResponseModeSelection_1.test`. Include migration guidance and WHATSNEW for the source break. | The `n` reproduction detects the old sender/recipient mismatch without catalog expansion. Direct checks cover both selectors, all response types, GI direction, and retained prediction limits. Existing HT module controls and affected callers retain their assertions. Record this commit's fingerprint effects separately. |
| 2. Enable current HT support after the prerequisite repair. | Add the inherited HT combinations and catalog flag. Migrate `_ac.ini`, `lan80211ac/omnetpp.ini`, and `Ac_160mhz.test` in this commit. Add the `ac` cases and width checks from Section 6. Include the adaptation and channel changes in WHATSNEW. | Catalog checks prove parity. Module checks prove management state, peer restrictions, HT delivery, and VHT delivery. ACK and CTS predictions agree with actual responses and exchange reservations. A bounded continuation case checks HCF's forecast and TXOP admission. Width checks exercise the production data selectors and AARF transmission path. |
| 3. Complete compatibility verification. | Check the final focused controls and user documentation. Confirm that each behavior commit includes its own migration guidance and WHATSNEW entry. | Existing HT controls and all eleven affected expected-PASS protocol controls retain their assertions. Documentation states the channel requirement, changed centers, response effects, adaptation steps, radio requirements, evidence boundaries, and retained standards violations. |

No separate channel-model project, VHT negotiation project, or Block Ack project is a prerequisite. No source change is proposed under sealed `common/packet/`.

## 6. Verification and acceptance

Use seed 0 for each production scenario. Use run 0 within each named configuration unless parameter variants require additional runs. Derive the new fixtures from existing HT tests where their production paths fit. Do not assert that the existing 2.4 GHz and new 5 GHz PHY timing values are identical. Compare HT capability and behavior under equivalent settings, while preserving band-specific mode timing.

| Check | Required observation |
| --- | --- |
| Extend `Ieee80211HtModeSet_1.test`. | `ac` supports HT and contains all 128 inherited HT combinations. Existing VHT entries, legacy rates, and reference mode remain intact. |
| Extend [Ieee80211Band_1.test](../../tests/unit/Ieee80211Band_1.test). | Each of the four triples matches the table above, including its frequency. Both conversions agree. Indices -1 and 4, and absent standard channel 52, fail. Both directions of 36/40 and 44/48 pass HT40 validation; both directions of 40/44 fail. Existing band checks retain their assertions. |
| Extend [Ieee80211MgmtStaDiscovery_1.test](../../tests/module/Ieee80211MgmtStaDiscovery_1.test). | A STA with the mapped band and 20 MHz capability rejects received HT Operation with pair 40/44. Valid 36/40 and 44/48 elements pass the same validation path. Serialized discovery input reaches production `storeAPInfo()` through the fixture's existing helper. This does not establish 40 MHz PHY operation. |
| Add `Ieee80211VhtHtSupport_1.test`. | Full management between two `ac` interfaces emits existing HT elements and commits HT peer state. An explicitly selected 20 MHz HT data mode reaches actual peer delivery. |
| Add a simplified-management variant to that fixture. | The existing no-air association path installs HT peer state for `ac`. Existing teardown removes it. |
| Add focused capability assembly checks. | Call the production MIB assembly with `ac`, operational width 20 MHz, and stream limits 1–4 and 8. Check MCS 0–7, 0–15, 0–23, 0–31, and 0–31 respectively. A module case checks actual antenna-to-limit propagation. |
| Check operational limits. | HT advertisement contains 20 MHz support and short GI at 20 MHz. It excludes 40 MHz and short GI at 40 MHz with the current PHY. |
| Extend `Ieee80211PeerModeSelection_1.test` through the production data selectors. | Each catalog candidate enters both selectors through a controlled rate-control input. Valid 20 MHz peer state excludes wider HT output. Absent peer state converts HT candidates to legacy. Each catalog includes all 128 HT combinations after activation. |
| Extend `Ieee80211HtAntennaRateControl_1.test`. | AARF supplies at least one raw 40 MHz HT candidate. Every actual HT data transmission from that station uses 20 MHz. Existing association, antenna-limit, and delivery assertions remain active. |
| Check fixed selection and catalog traversal. | Fixed-rate ties retain VHT. A unique supported HT rate selects HT. The fastest mandatory mode remains VHT at 78 Mbps. The specified forward and reverse steps connect VHT MCS 0 and HT MCS 0 at 6.5 Mbps. |
| Check asymmetric HT peer restrictions. | A two-antenna sender requests HT MCS 8 against a one-antenna peer. The production selector supplies HT MCS 1 at 13 Mbps, one stream, 20 MHz, and long GI. The peer receives and delivers the data. |
| Check default responses and sender prediction. | After the required repair, the associated 14.444 Mbps VHT case receives a 13 Mbps HT ACK without an ACK-rate override. The sender predicts the actual ACK duration. The applicable Duration/ID reservation covers the response, and the exchange completes successfully. Direct checks exercise both selectors. A recipient without peer HT state retains the 12 Mbps legacy fallback as an INET limitation. |
| Reproduce the existing `n` response defect before HT activation. | Production selectors receive an HT MCS 1 input at 13 Mbps, 20 MHz, and long GI. With compatible peer state, both caller roles select the same HT response and duration. The old sender path instead selects 12 Mbps ERP-OFDM. This direct-query test does not establish a complete exchange. |
| Check CTS prediction through a complete exchange. | A QoS sender uses approximately 14.444 Mbps VHT RTS with the default CTS rate. The actual CTS uses 13 Mbps HT. Prediction, RTS Duration/ID, and the CTS timeout consumer use that response mode. The exchange completes without a spurious RTS retry. |
| Check a queued successor at the TXOP boundary. | Production HCF uses the HT ACK duration for its forecast and admission decision. It defers a successor that fits only with the incorrect legacy estimate. The emitted reservation covers only the first exchange. The fixture isolates the TXOP limit through a TXNAV control during admission. |
| Add `Ieee80211ResponseModeSelection_1.test`. | Production ACK and CTS queries agree across caller roles under the stated conditions. The QoS Basic Block Ack query has the same checks. Asymmetric GI advertisements distinguish the directions. Unknown Tx capability checks preserve association and actual response behavior while prediction retains its stated limitation. Absent peer state exercises legacy fallback. |
| Retain HT and VHT controls. | Existing HT association, peer selection, and antenna-rate-control cases retain their assertions. A 13 Mbps, 20 MHz, one-stream VHT case proves transmission, reception, and upper-protocol delivery after HT activation. |
| Check migration. | All three migrated configuration sources produce runs with mapped index 0 and effective center 5180 MHz. The existing 80 MHz and 160 MHz VHT tests retain their assertions of mode selection at the MAC output. |

### Verify the prerequisite without HT activation in `ac`

Introduce `Ieee80211ResponseModeSelection_1.test` in step 1b with the existing `n(mixed-2.4Ghz)` catalog. Give stations A and B compatible 20 MHz capabilities, MCS 0–7, and mutually consistent peer state. Leave the ACK-rate override unset. Supply HT MCS 1 with long GI as the outgoing data mode.

Use a request tag for A's prediction. Use an indication tag for B's response query. Set the data frame's transmitter address to A. Set its receiver address to B.

Run the production ACK query in both selectors with `PREDICT` at A and `RESPOND` at B. Require HT MCS 1 at 13 Mbps and equal response durations. Before the repair, A's query selects 12 Mbps ERP-OFDM, so the equality assertion fails. This is a source-derived expected failure, not an executed result. The query test establishes response selection; it does not establish a complete exchange or corrected control-format conformance.

The same prerequisite fixture includes the directional GI, unknown Tx capability, and absent-peer cases below using band-appropriate `n` modes. It checks ACK and CTS in both selectors, plus Basic Block Ack in the QoS selector. Step 2 adds the corresponding `ac` checks and the VHT default-candidate cases after catalog expansion. Run the existing HT association and AARF module controls at the prerequisite commit. Keep the seven new `ac` network scenarios in step 2.

### Verify the rate-control width restriction

Extend `Ieee80211PeerModeSelection_1.test` with a deterministic sweep through the production data-selection entry points. Use both `RateSelection` and `QosRateSelection`, with catalogs `n(mixed-2.4Ghz)` and expanded `ac`. Keep a valid MIB connection in every case. Assemble local capabilities for one antenna and 20 MHz through the production MIB path. Use the production negotiation helper for the case with peer state.

For each selector and catalog, check these two states:

- A valid peer record permits HT MCS 0–7 at 20 MHz, with HT Operation width 20 MHz.
- No peer record exists for the unicast receiver; the local MIB remains present.

Disable fixed and per-receiver data-rate overrides in this fixture. Supply each catalog entry through a test-local rate controller's `getRate()` method. Require the production data selector to request that candidate. Keep the selector and peer filter in production code.

Require 128 HT candidates per catalog and peer-state case after activation. Require 64 of those candidates to use 40 MHz. A zero-candidate pass is invalid.

With valid peer state, every selected HT mode must use 20 MHz. With absent peer state, every HT input must produce a non-HT fallback. VHT candidates retain their current path; this test makes no 20 MHz restriction claim for VHT. Preserve the existing helper assertions for invalid state and HT Operation width. This sweep tests candidate filtering and caller integration, without a simulation of rate-control convergence.

Extend the existing AARF module fixture with width observations for raw candidates and actual PHY data transmissions. Use its existing `n` configuration, run 0, and seed 0. Require at least one raw 40 MHz HT candidate to prevent a vacuous pass. Require every actual HT data transmission from that station to use 20 MHz. Preserve its association, antenna-limit, and delivery assertions. A MAC mode request alone does not establish actual PHY transmission.

The sweep covers both catalogs and both selectors; the AARF case checks the production transmission path. No new module fixture or full network sweep through every AARF/Onoe combination is necessary. These checks establish the stated unicast path, not a universal rejection of direct PHY mode requests.

### Verify HT activation and response integration

A candidate HT fixture rate is approximately 7.222 Mbps at 20 MHz with one spatial stream and short GI. The current VHT catalog's one-stream 20 MHz entries use long GI. Confirm unique lookup after the new entries exist before the fixture relies on this rate. Inspect the actual selected modulation class and received frame; application delivery alone does not prove HT transmission.

This fixture proves HT selection and delivery within the current model. It does not establish compliant short-GI airtime or performance. Section 2 explains the retained duration defect. The directional response checks must inspect the GI parameter because duration alone cannot distinguish these MCS 0 variants.

Use explicit management and response rates where necessary to isolate inherited limitations. Record those settings in the fixture's claim. The default ACK case leaves its ACK-rate override unset. The default CTS case leaves its CTS-rate override unset.

In the default-response case, use the same data frame for sender prediction and recipient response checks. Inspect each selector through its production method with the appropriate station state. Record the predicted duration before transmission. Record the actual ACK mode at reception. Check the applicable reservation through the production protection mechanism. Require acknowledgment at the originator without a spurious retry.

Receiver delivery alone cannot establish successful acknowledgment.

Add one TXOP continuation scenario to the same module fixture. Use two associated `ac` stations with two antennas and compatible HT state. Select the same approximately 14.444 Mbps VHT data mode as the default ACK case. Leave the ACK-rate override unset. Apply these configuration requirements:

- Enable QoS.
- Select single protection.
- Set a positive TXOP limit.
- Disable Block Ack.
- Disable RTS for these data frames.

For this scenario (run 6), use these radio medium settings:

```ini
*.radioMedium.propagation.typename = "ConstantTimePropagation"
*.radioMedium.propagation.propagationTime = 0s
```

[HCF's reservation forecast](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) excludes propagation delay. With nonzero delay, HCF can reserve the successor but refuse it after the ACK arrives. Zero delay aligns the forecast with the observed ACK completion time. This scenario makes no claim about nonzero propagation delay.

Queue two unicast data frames before HCF starts the first exchange. Let `C_ht` denote the successor's DATA duration, one SIFS, and its 13 Mbps HT ACK duration. Let `C_legacy` denote the same cost with the incorrect 12 Mbps OFDM ACK. Let `R` denote the TXOP time left when the first ACK completes. Choose payload lengths and the TXOP limit so that `SIFS + C_legacy < R < SIFS + C_ht`. Assert this inequality from the selected modes and observed exchange times.

Reuse the test control for TXNAV from [Ieee80211HcfSingleProtection_1.test](../../tests/module/Ieee80211HcfSingleProtection_1.test). Supply TXNAV greater than `C_ht` only while HCF evaluates admission. This test control isolates the TXOP limit; it does not establish correct TXNAV updates. Record the actual emitted Duration/ID without this control. Keep response selection and the HCF decision in production code.

Require the first frame's reservation to cover only its own ACK exchange. Require HCF to refuse the successor in that TXOP. Require later transmission and acknowledgment after a new channel grant, without a retry caused by this refusal. Record predicted and actual ACK modes, response durations, TXOP times, reservation fields, and admission results. With the incorrect legacy estimate, the successor fits the test's TXOP budget and produces the wrong forecast or admission result. No additional test framework is necessary.

Put direct response checks in the focused `Ieee80211ResponseModeSelection_1.test` unit fixture. Introduce its `n` cases in step 1b. Extend it with `ac` cases in step 2. Instantiate both production selectors with explicit station state.

Use test subclasses only to provide initialization inputs where necessary. Do not replace production selection with a copied algorithm or a mock selector. Supply a mode request for prediction. Supply a mode indication for the received frame.

The directional case uses stations A and B with MCS 0–7, 20 MHz capability, and defined, equal Tx and Rx sets. A advertises `shortGi20 = true`; B advertises `shortGi20 = false`. Set the same approximately 7.222 Mbps HT response candidate at both stations through the relevant response override. Use identical catalogs and stable negotiated state. This case tests direction; the separate default-response cases test candidate selection without an override.

| Exchange | Required response | Sender prediction | Recipient selection |
| --- | --- | --- | --- |
| A sends to B; B responds to A. | HT MCS 0, short GI, approximately 7.222 Mbps | A uses `localRxPeerTx`, whose receiver is A. | B uses `localTxPeerRx`, whose receiver is A. |
| B sends to A; A responds to B. | HT MCS 0, long GI, 6.5 Mbps | B uses `localRxPeerTx`, whose receiver is B. | A uses `localTxPeerRx`, whose receiver is B. |

Run these assertions for ACK and CTS in both selectors, plus Basic Block Ack in the QoS selector. Check mode parameters and duration, not data rate alone. Leave the transmitter address unset in the sender's RTS and BAR query inputs. Set the actual sender address in the corresponding received frames. These cases reject both a wrong capability direction and a role guess from the transmitter address.

For these direct CTS checks, use 20 MHz HT MCS 0 with long GI for the RTS in both directions. Supply that mode through the sender's request tag and the recipient's indication tag. Repeat that HT RTS input in the unknown-Tx-capability cases below.

Clause 10.17 permits short-GI HT transmission only when the intended receiver advertises that capability, subject to its other conditions. The long-GI RTS fits both stations' advertised reception capabilities. Keep the short-GI response candidate unchanged. These assertions exercise the HT response requirement in Clause 10.6.6.1(a)(2) and the directional GI restriction. They do not establish full RTS/CTS conformance.

For each response query, test a responder advertisement with undefined Tx MCS support. Repeat with defined, unequal Tx and Rx MCS sets. Supply the responder's private Tx map with MCS 0–7. Preserve valid negotiated state at both endpoints. Use A as the response receiver, with its short-GI capability and the approximately 7.222 Mbps HT response candidate.

Prediction selects legacy 6 Mbps while the capable responder retains HT MCS 0 with short GI at approximately 7.222 Mbps. Assert that the query changes no negotiated state. This case records the retained limitation; it does not establish response agreement.

Also check absent peer state at each endpoint and unchanged non-HT candidates. Unknown Tx capability must remain distinct from invalid association state. The HT RTS case must not acquire a forced legacy response through this repair.

Retain a direct default-candidate check for the 14.444 Mbps VHT input in both selectors. It must produce the same 13 Mbps HT ACK or CTS mode for prediction and actual response. For Basic Block Ack, use an HT BAR at approximately 7.222 Mbps with no Block Ack override. Both roles must retain its compatible HT candidate. Existing `Ac_BlockAck` remains the complete-exchange compatibility control.

These default candidates test retained INET behavior. The plain VHT RTS and HT CTS do not satisfy Clause 10.6.6.1's control-format conditions. An HT BAR alone also does not permit an HT Block Ack response under that clause. The fixture preserves these format limitations separately from the response-prediction repair.

The CTS network case uses two associated `ac` interfaces with two antennas and compatible HT state. Set the QoS sender's `controlFrameBitrate` to approximately 14.444 Mbps. Set its RTS threshold to zero. Confirm that the RTS uses 20 MHz VHT MCS 0, two streams, and short GI. Use 13 Mbps VHT data to keep its default ACK candidate unchanged.

Compare predicted and actual CTS mode and duration. Check RTS Duration/ID. Require successful data acknowledgment without a spurious retry.

Extend `Ieee80211ResponseTimeout_1.test` so its test selector asserts that each timeout policy requests the prediction role. Retain its existing delay assertions. The production-query fixture establishes which mode that role selects; the timeout fixture establishes how the policy uses that mode. Together with the CTS network case, these checks cover the source-derived failure without a separate timeout framework.

For the asymmetric case, set `dataFrameBitrate = 13Mbps`, `dataFrameBandwidth = 20MHz`, and `dataFrameNumSpatialStreams = 2` on the sender's selector. Those constraints identify HT MCS 8 with long GI in the expanded catalog. Check that candidate before transmission. After full association, inspect the emitted mode and received frame to prove the change to one-stream HT MCS 1. Repeat the selector assertion for the QoS and non-QoS paths through focused tests. One complete network case is sufficient.

Keep capability matrix checks in a focused unit fixture that calls production MIB assembly. Use `Ieee80211MibPeerTeardown_1.test`'s construction pattern. Name that new fixture `Ieee80211VhtHtCapabilities_1.test`. Do not create five complete network runs solely for stream-count arithmetic. Use one scenario parameter in `Ieee80211VhtHtSupport_1.test`, with these seven values in order:

1. Run 0 checks full management and HT delivery.
2. Run 1 checks simplified management and teardown.
3. Run 2 checks asymmetric HT peer restrictions.
4. Run 3 checks VHT delivery.
5. Run 4 checks default ACK responses, sender prediction, the exchange reservation, and successful acknowledgment.
6. Run 5 checks default CTS responses, sender prediction, RTS Duration/ID, and successful completion without a spurious RTS retry.
7. Run 6 checks the queued successor's forecast, reservation, and TXOP admission with default HT ACKs and the stated TXNAV control.

Set `%extraargs: -r 0..6` in the fixture. Use seed 0 for every run. Reuse the parameter-variant mechanism in [Ieee80211RecipientNav_1.test](../../tests/module/Ieee80211RecipientNav_1.test); no custom test driver is necessary. Record the scenario names and run numbers in the fixture description. Confirm that the standard module-test command executes all seven runs.

All acceptance conditions are required:

1. `ac` contains actual HT modes and enters existing HT capability assembly.
2. Its local HT capability reflects the same antenna and PHY limits as current HT.
3. Full and simplified management expose HT state through their existing paths.
4. An HT transmission between `ac` interfaces reaches the receiver and upper protocol through production code.
5. Existing 20 MHz VHT data reaches the peer and upper protocol after HT activation.
6. Existing VHT modes and format calculations remain available. Wider VHT controls retain their assertions of mode selection; those assertions do not establish PHY transmission or duration calculations.
7. Antenna and GI asymmetry checks produce their specified modes. Under the stated conditions, default ACK and CTS cases establish consistent prediction, reservation, actual responses, and successful exchange completion. Basic Block Ack queries use the same explicit role contract. The queued successor case establishes the specified forecast, reservation, and TXOP admission decision through production HCF.
8. Migrated configurations initialize with the declared channel mapping and effective centers. Received HT Operation elements cannot authorize pair 40/44 in the mapped band.
9. No new VHT negotiation or wider-channel capability is claimed.
10. The response repair passes its `n` reproduction before HT activation in `ac`. Its commit includes the enum contract, all callers, migration guidance, and separate regression evidence.
11. The candidate sweep excludes wider HT output through both data selectors. The AARF fixture observes actual 20 MHz HT transmission despite raw 40 MHz candidates. These checks retain their explicit unicast and MIB-state limits.
12. WHATSNEW explains the additional equal-rate adaptation steps without an unmeasured throughput or convergence claim.

### Planned commands

Run these commands after implementation, from the repository root in the configured INET environment. The new tests do not yet exist. Confirm that each selector includes its intended cases before execution.

At step 1b, build in debug mode. Run these focused checks before HT activation:

```sh
make -j$(nproc) MODE=debug
bin/inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet|HtCapabilities|HtMgmtElements|PeerModeSelection|ResponseModeSelection|SupportedRates|NonHtReferenceRate|MibPeerTeardown|ResponseTimeout)_1\.test'
bin/inet_run_module_tests -m debug -f 'Ieee80211(HtAssociation|HtAntennaRateControl|MgmtStaDiscovery|HcfMultipleProtection|HcfSingleProtection|SingleProtectionGroup|Txnav)_1\.test'
```

The prerequisite unit command selects eight existing fixtures and the proposed response fixture. The module command selects seven existing fixtures. The response fixture contains its `n` cases at this commit; the new `ac` cases follow in step 2. Select affected fingerprint configurations from this commit's changed paths. Preserve the resulting comparison separately from the later HT activation comparison.

After HT activation, run the complete selection:

```sh
make -j$(nproc) MODE=debug
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
bin/inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet|Band|HtCapabilities|VhtHtCapabilities|HtMgmtElements|PeerModeSelection|ResponseModeSelection|SupportedRates|NonHtReferenceRate|MibPeerTeardown|ResponseTimeout)_1\.test'
bin/inet_run_module_tests -m debug -f 'Ieee80211(HtAssociation|HtAntennaRateControl|VhtHtSupport|MgmtStaDiscovery|HcfMultipleProtection|HcfSingleProtection|SingleProtectionGroup|Txnav)_1\.test'
bin/inet_run_protocol_tests -p inet -m debug -w wifi -f '(11ac/Ac_(PhyMode|VhtIe|BlockAck|256qam|4ss|160mhz)|common/Wifi(Association|AuthOpen|Beacon|BeaconInterval|DataDelivery|Probe))\.test'
```

The protocol selector includes twelve consumers of the changed `_ac.ini`. Five expected-PASS controls reside under `11ac/`; six reside under `common/`. The other selected case, `Ac_VhtIe`, has an expected FAIL result. Preserve their actual assertions. Their descriptions do not establish additional conformance claims.

The common controls are:

- [WifiAssociation.test](../../tests/protocol/wifi/common/WifiAssociation.test).
- [WifiAuthOpen.test](../../tests/protocol/wifi/common/WifiAuthOpen.test).
- [WifiBeacon.test](../../tests/protocol/wifi/common/WifiBeacon.test).
- [WifiBeaconInterval.test](../../tests/protocol/wifi/common/WifiBeaconInterval.test).
- [WifiDataDelivery.test](../../tests/protocol/wifi/common/WifiDataDelivery.test).
- [WifiProbe.test](../../tests/protocol/wifi/common/WifiProbe.test).

These controls cover discovery, management exchanges, association, and delivery under the shared `ac` configuration. The unit selector includes nine existing tests and two proposed tests. The module selector includes seven existing tests and the proposed fixture with seven runs. The existing cases include all test callers and overrides affected by the response-query signature at the recorded review commit. Confirm that inventory against the implementation base before execution.

`HtCapabilities`, `HtMgmtElements`, `SupportedRates`, `NonHtReferenceRate`, and `ResponseTimeout` all reside under `tests/unit` with the `Ieee80211` prefix. The verified unit selector retains all five fixtures. `Ieee80211ResponseTimeout_1.test` checks timeout calculations with a test selector. It does not replace the new direct checks of production response selection. `Ieee80211HcfMultipleProtection_1.test` must forward the explicit role through its selector wrapper without a change to its protection assertions.

The three added module controls must also forward the role and retain their existing assertions. They use `opMode = "a"` and explicit legacy ACK rates. They check source compatibility and existing protection behavior after the signature change. They do not replace run 6's HT response and continuation checks.

Check example initialization from `examples/wireless/lan80211ac` in the configured debug environment:

```sh
inet --debug -u Cmdenv -c Ping1 -r 0 --sim-time-limit=0s
```

The explicit `--debug` flag selects the newly built debug artifacts. Plain `inet` can prefer an existing release library when `MODE` is unset. The zero-duration run checks initialization only. Confirm mapped index 0 and effective center 5180 MHz from the radio state or log.

`Ac_VhtIe` retains its existing expected failure because this scope adds no VHT elements. The runner's `FAIL` classification does not establish the cause. For example, an initialization error can produce the same classification. Inspect the failure reason before you report it as the known VHT-element limitation. Confirm that the AP emitted a Beacon without VHT Operation through the test output or a focused frame observation.

If that evidence is absent, report the failure cause as unverified. A different failure does not validate the expected limitation. Report each positive control only at its actual observation boundary. The new 20 MHz fixtures establish PHY transmission and peer delivery. No new test framework is required.

`Ac_BlockAck` remains a regression check for its actual assertions, not proof of Compressed Block Ack. Do not change either expectation solely to accommodate this plan. Report a zero-case selection as not run.

Select affected fingerprint configurations for each behavior commit from its changed paths and configurations. Compare each commit against its parent. Record the response repair's effects separately from HT activation and the channel migration. Explain each changed trajectory before any proposed baseline update. A required baseline update belongs with its responsible source commit. Follow the [gate procedure](../../doc/project/guide/run-the-gates.md) for final build and publication checks.

## 7. Review status before implementation and retained limits

This revision follows the user's explicit implementation-parity scope. It separates the existing HT response defect into a prerequisite commit with an explicit role enum and an `n` reproduction. The HCF consumers and affected test selectors remain in the implementation inventory. The continuation scenario checks the TXOP consumer after HT activation. At review, no source change or runtime result established the repair. The execution record now supplies that evidence.

The mapped band now has explicit HT40 pair validation and a receive-path test. The plan corrects Table 21-22's 160 MHz example and distinguishes mode requests from actual PHY transmission. It retains all eleven affected expected-PASS protocol controls, plus evidence requirements for the known expected failure. The response design uses existing MIB state and selection code; the band design uses existing catalog and validation interfaces.

The 20 MHz limit describes advertised and negotiated HT capability. Source inspection traces the same restriction through ordinary unicast rate control. The plan adds a candidate sweep and an actual transmission check, but no universal transmitter guard. Equal-rate catalog steps require compatibility documentation; their performance effect remains unmeasured. Reports of expected failures require evidence of the failure cause.

The CTS and Basic Block Ack fixtures retain explicit control-format limitations. A separate HT RTS input tests the mandatory HT CTS condition. The short-GI fixture proves mode selection and delivery within the existing duration model. It does not establish short-GI airtime compliance.

This work remains outside scope:

- Operational 40 MHz HT support and a primary/secondary channel model.
- VHT capability negotiation, new VHT elements, and automatic VHT fallback.
- Stricter QoS defaults, a new HT-only 5 GHz profile, and a role-dependent basic-set redesign.
- Exact response prediction for unknown responder Tx capability, independent response overrides, or inconsistent peer state.
- Repair of the inherited HT short-GI duration defect or general control-format conformance.

The added channel mapping is necessary for the existing HT management path to operate in the VHT band. The response-prediction mismatch requires the preceding HT repair, as Section 4 explains. Other repairs require a demonstrated failure caused by HT activation. They do not authorize unrelated conformance work. Refresh the source comparison if the implementation base changes.

Follow the [plan procedure](../../doc/project/guide/write-an-implementation-plan.md), [state ownership design](../../doc/project/design/ieee80211-model-architecture.md), and [minimal-design rule](../../doc/project/rule/quality.md#qr-design-minimal). The plan procedure remains a draft. Revision 10 changed the proposal at review. The separate execution record describes the later implementation, checks, and commits. No baseline update or publication occurs during execution.
