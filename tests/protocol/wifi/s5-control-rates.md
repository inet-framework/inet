# S5-P8: Prepare legal Compressed control rates

This document belongs to **ieee80211: add: select legal Compressed control rates**. It prepares the selector and response-airtime contract before production activation.

Before this commit, QoS rate selection supports the Basic response contract. Compressed operation needs legal non-HT BAR modes and airtime for a 32-byte response. After this commit, selection uses the applicable local, BSS, and peer rate sets. Invalid configured overrides fail explicitly.

IEEE Std 802.11-2024 Clause 10.6.6.5.2 defines the response-rate basis. The selector uses an eligible BSS basic rate, with the applicable mandatory-rate fallback. The normative corpus identifier is `ieee80211-2024:clause:10.6.6.5.2`, physical PDF pages 1945–1947. The model has an explicit fallback for unknown BSS state. That model choice is separate from the standard's known-rate requirements.

## Direct check and protocol boundary

The [rate fixture](../../unit/Ieee80211CompressedBlockAckRate_1.test) supplies independent peer rate sets and checks both response directions. It also checks initial and later BAR selection, invalid overrides, and Basic compatibility. The prediction and response queries must select compatible modes.

| Decision | Input that controls it | Required check |
| --- | --- | --- |
| Initial BAR | Applicable BSS basic rates and local support. | The selected mode is legal and non-HT. |
| Later BAR | Applicable peer operational rates and local support. | Different peers can select different legal modes. |
| Compressed response | Received mode and its non-HT reference rate. | Prediction and response agree on 32-byte airtime. |

No diagram is necessary here. The table identifies each input and observation without an event-order dependency.

HCF still rejects Compressed selection at this commit. A new protocol exchange cannot pass until activation. Direct helper calls establish selection rules, not their production use. A missing parent API is not a protocol failure.

## Integration and limits

No new protocol test is assigned to this preparation boundary. The later activation test observes a real non-HT BAR and a non-HT 32-byte response. That exchange supplies integration evidence; it does not replace the selector's peer-set and invalid-override cases.
