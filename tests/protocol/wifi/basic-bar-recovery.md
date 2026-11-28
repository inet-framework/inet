# Retry an unanswered Basic Block Ack request

The access category advances its EDCA retry count whenever a transmitted Basic Block Ack Request (BAR) goes unanswered. It also advances the counter when a BAR loses internal contention. The DATA retry counters belong to the individual DATA frames. `Ieee80211BlockAckScheduling_1.test` checks both failures and the reset after a successful TXOP.

A lost reply can conceal reception of every DATA MPDU in the batch. The EDCAF retains the BAR's receiver, TID, starting sequence number and retry count between grants. HCF opens a later TXOP with that request. A retry is counted when a BAR has been sent and its reply times out. Expiry and agreement teardown invalidate the pending request. After the retry limit, the existing Block Ack failure path makes unresolved DATA eligible for recovery.

`WifiBlockAckDataRetry.test` uses an associated station and AP with a Basic agreement and a BAR threshold of one. A radio tap drops the first Block Ack, drops every Block Ack at retry limits zero through four, or clears the first reply bitmap. Each expected retry must open a later grant with the same request fields. The lost-first-reply case requires a successful bitmap and checks for duplicate DATA. The other cases require the original DATA sequence and TID with Retry set.

```mermaid
flowchart TD
    Data["Station sends Basic DATA and requests its bitmap"]
    Data --> One["Tap loses one reply"]
    Data --> All["Tap loses all replies"]
    Data --> Clear["Tap clears the received bitmap"]
    One --> Retry["Same BAR in a fresh grant and positive bitmap"]
    All --> Bound["Fresh grants until the selected limit"]
    Bound --> Recover["Retransmit DATA with its original sequence and TID"]
    Clear --> Recover
```

The parent source makes DATA eligible immediately after a missing reply. That path fails the next-BAR assertion when a positive retry limit is expected. The retry fixture does not model corruption at a second station or an association reset while a retry is pending.

`WifiBasicNormalAckFallback.test` clears the first Basic bitmap and switches the station’s DATA rate from 54 to 6 Mbps. The protected retry no longer fits the 500 µs TXOP. The station must send the original sequence number with Retry set and Normal Ack policy, then receive an ACK with AARF enabled. The earlier recovery code tried to read a missing failure counter when reporting this ACK to rate control.

```mermaid
flowchart LR
    Input["Clear the bitmap and lower the DATA rate"] --> Action["Retry the original sequence with Normal Ack"]
    Action --> Result["Receive ACK with rate control active"]
```

`WifiBasicBitmapRecoveryCleanup.test` injects one prior RTS failure for each of at least two outstanding MPDUs, then passes a partial bitmap to HCF. Earlier bitmap processing removed acknowledged DATA from the queue while retaining its RTS failure count. The fixture reads the counters immediately after processing and observes the retry of the outstanding sibling.

```mermaid
flowchart LR
    Input["One RTS failure per outstanding MPDU"] --> Action["Process a partial bitmap"]
    Action --> Result["Acknowledged counters read zero and the missing MPDU retains one prior failure"]
```
