# S5-P7: Retain receipt evidence independently of delivery

This document belongs to **ieee80211: add: retain explicit Compressed receipt state**. It prepares complete-MPDU receipt records before production Compressed activation.

Before this commit, the Basic fragment bitmap supplies the recipient record. Delivery cleanup cannot provide the separate persistent receipt truth needed by Compressed feedback. After this commit, the agreement selects a Compressed record with explicit receipts. Positive receipts survive delivery, and absent positions remain zero.

```mermaid
flowchart LR
    D[Receive a complete MPDU] --> R[Record positive receipt]
    D --> B[Store for ordered delivery]
    B --> U[Deliver the complete unit]
    U -. leaves receipt unchanged .-> R
    R --> F[Build the later Compressed bitmap]
```

## Direct check and protocol boundary

The [receipt fixture](../../unit/Ieee80211CompressedBlockAckRecord_1.test) checks starts at 100 and 4094. It records receipts, performs delivery cleanup, advances the window, and builds a response. It requires zero for every absent position. Sequence wrap must not create a positive receipt.

For example, delivery removes the buffered unit for sequence 4094. The receipt record still reports that sequence until window rules remove its active bitmap position. A later response can therefore retain acknowledgment truth without a second delivery.

HCF still rejects Compressed selection in this commit. The fixture supplies agreement state directly and does not prove negotiation or physical interframe recognition. Its new API is absent in the parent, so a parent compilation failure would not establish a protocol defect.

## Integration and limits

No new protocol test is assigned to this preparation boundary. Production Compressed activation owns the peer exchange. Its protocol fixture requires a real response with positive receipt evidence. The direct record fixture remains necessary for delivery cleanup, holes, and sequence wrap. A successful response alone cannot prove all those record boundaries.
