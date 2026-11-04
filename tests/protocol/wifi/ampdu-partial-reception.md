# Preserve partial PHY outcomes before member extraction

This document belongs to PHY P2, **ieee80211: add+change: retain partial HT A-MPDU PHY outcomes**. Its protocol test is [WifiHtAmpduPartialReception.test](common/WifiHtAmpduPartialReception.test). The [PHY plan](../../../plan/pending/ieee80211-ampdu-phy-reception.md) defines the receiver contract and later MAC work.

The PHY prerequisite retains cached outcomes in chunk flags. Before MAC integration, Rx rejects the complete aggregate before standalone frame checks. After MAC integration, a separate Rx method extracts members and checks their FCS. The fixture therefore observes the envelope before extraction. This change preserves the PHY assertions and the independent CTS control.

## Requirement and model boundary

IEEE Std 802.11-2024 Clause 9.7.1 and Table 9-659 define A-MPDU subframes and delimiter fields. Their canonical nodes are `ieee80211-2024:clause:9.7.1` and `ieee80211-2024:table:9-659`. Physical PDF pages 1840–1842 cover the relevant layout. Figure 19-6 defines the HT-SIG Aggregation field on physical PDF pages 3432–3434.

These requirements define the represented aggregate. Independent chunk error probabilities are an explicit INET model abstraction. The standard does not require the fixture's chosen synthetic loss pattern. That pattern makes the chunk error contract deterministic. It supplies no error-accuracy or goodput claim.

## Setup, stimulus, and assertions

The fixture uses a production scalar transmitter, medium, receiver, MAC, and Rx. The recipient uses HCF in HT mixed mode at 20 MHz. A test error model inherits NIST support but fixes common-field success. Its chunk probability function returns failure for the configured bit length. Four configurations select lengths of 100, 101, 102, and 4 octets. Typed input retains separate chunk errors at those positions. Raw byte input has one DATA outcome and fails throughout.

At 1 ms, the driver submits members of 100, 101, and 102 octets. Three alignment octets follow the 101-octet member. The complete PSDU contains 318 octets. The driver selects HT MCS 0 and the run's GI before production radio encapsulation.

An Rx subclass emits an observation signal before it calls production aggregate reception. The observer checks the intact PSDU length and clear container bit-error flag at that signal. The indication must report a successful header and complete content. Chunk flags supply the error outcomes at offsets 0, 4, 104, 108, 212, and 216 octets. For typed input, each selected chunk must be incorrect exactly when its length equals the configured failed length. Raw input marks the complete DATA view incorrect.

The observer reads common indication fields through OMNeT++ descriptors. A missing indication fails at runtime. The observer changes no protocol state. The amended fixture requires the aggregate Rx method from MAC integration. It supplies no execution result for that method's parent.

Production Rx must accept no member because these zero-filled members have no valid MAC FCS. At 2 ms, the driver submits an independent CTS with computed FCS and a 50-microsecond Duration field. The observer requires one 50-microsecond NAV update after the aggregate observation. This control checks ordinary reception after the aggregate.

The protocol tester requires `testHtAmpduPartial` followed by `testHtAmpduControl`. These events occur only after their assertions pass. A missing aggregate, wrong outcome, or missing control result cannot pass through an empty observation.

The four loss patterns combine with both GIs, typed and raw-byte transmission, and whole-signal and separate-part reception. These combinations produce 32 runs. Every run uses seed zero and ends at 4 ms. The deterministic probabilities require no extra seeds. The existing module fixture separately retains dimensional-radio, DCF, malformed-input, and cached-query controls.

```mermaid
sequenceDiagram
    participant Driver
    participant Tx as Production transmitter
    participant Receiver as Production receiver
    participant Rx
    participant Tester as Protocol tester
    Driver->>Tx: Three-member HT aggregate
    Tx->>Receiver: One PPDU through the medium
    Receiver->>Receiver: Retain cached chunk outcomes
    Receiver->>Rx: Intact content with chunk error flags
    Rx->>Tester: Observation after envelope assertions
    Rx->>Rx: Extract members; reject invalid MAC content
    Driver->>Tx: Independent CTS with computed FCS
    Tx->>Receiver: Ordinary PPDU
    Receiver->>Rx: Decoded CTS
    Rx->>Tester: Verified NAV update
```

The amended fixture preserves the prerequisite's PHY assertions at the new aggregate Rx entry. Its parent result remains unverified. The comparison below describes an earlier fixture and source range.

## Verification and limits

Use the current debug library before the selected protocol run:

```sh
make MODE=debug -j8
opp_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiHtAmpduPartialReception\.test$' --no-concurrent
```

The installed runner can require the temporary compatible descriptor described in the [execution record](../../../plan/pending/ieee80211-ampdu-phy-execution.md). The expected result is 32 successful runs and `PROTOCOLTEST wifi_ht_ampdu_partial_reception: PASS`. The [comparison record](ampdu-partial-reception-comparison.md) reports historical executions of an earlier fixture.

This fixture checks PHY outcomes before member extraction and checks rejection of invalid MAC content. It does not prove valid member acceptance, upper delivery, implicit Block Ack, or correlated FEC errors. The diagram describes the test sequence. Diagram rendering remains unverified.
