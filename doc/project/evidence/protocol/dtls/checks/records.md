# DTLS record checks

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md)

## Framing

Place two complete records in one datagram. Include a final record without its length field in a separate variant. The receiver must consume the exact declared records and never complete an incomplete record using another datagram. Exercise both supported truncated sequence widths and reject unsupported header forms.

## Replay and invalid content

After setup, deliver records with distinguishable bodies. Duplicate a recent record, reorder unseen records within the receive window, and deliver a record older than the window. Keep the sequence values inside the unambiguous reconstruction range. Require once-only delivery for accepted identities and no replay-window update from failed deprotection. An input outside the supported reconstruction/authentication conditions cannot establish authenticated rejection without cryptographic evidence.

## Application and closure

Lose one application datagram and deliver the next. The next must not wait for recovery of the lost application data, and no retransmission should be initiated for that loss. Application data received before peer Finished cannot be delivered. For closure, lose the closure alert and check local completion independently of peer observation. No alert retransmission is permitted.
