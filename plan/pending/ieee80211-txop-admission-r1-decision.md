# Basic RTS exhaustion: decision for C5

The user accepted the lifetime interpretation on 2026-10-19: "Retain the MSDU until lifetime expiry." This decision satisfies R1 of [the plan](ieee80211-txop-admission-remaining-paths.md#r1-basic-rts-exhaustion-under-an-accepted-agreement). B1 already supplies the legal failing baseline. C5 can now activate Basic admission.

IEEE Std 802.11-2016 supplies both relevant rules. Clause 10.22.2.11.1 requires the MSDU short counter and QSRC to increase after CTS failure. Its general exhaustion rule discards the MSDU at the applicable retry limit. Clause 10.24.3 instead limits MSDUs that use Block Ack by lifetime. It also permits Normal Ack under an agreement. The original PDF supplies these normative clauses on physical pages 1391–1392 and 1419–1421. The corpus does not index this edition.

The accepted interpretation applies the specific Block Ack lifetime rule to MSDUs under an accepted Basic agreement. Associated RTS failures still increase the MSDU short counter and QSRC. Short-limit exhaustion alone does not discard that MSDU. This rule also applies before the first local DATA transmission. It applies after a local DATA transmission and after an internal collision.

The alternative applies the RTS short limit independently of the Block Ack DATA lifetime rule. It discards the protected MSDU when that short count reaches the limit. It applies before and after local DATA transmission. Under this alternative, internal-collision exhaustion also discards the MSDU. This interpretation retains the current legacy disposal result but needs explicit acceptance too.

Both choices use the same ownership boundary. After admission succeeds, the data service commits the ordinary MSDU or fragment group before RTS. The committed-frame owner retains the allocation identity and absolute deadline across CTS failure. The recovery owner retains both 2016 MSDU counters. Neither choice permits fragmentation under an HT-immediate agreement.

| Condition | Accepted lifetime interpretation | Rejected independent RTS limit |
| --- | --- | --- |
| RTS short count reaches its limit before local DATA. | Retain all fragments and the MSDU record until success, lifetime expiry, or agreement disposal. | Dispose of all fragments, ACK entries, lifetime entries, and the MSDU record. |
| RTS short count reaches its limit after local DATA. | Retain the fixed transmitted bodies and lengths until another terminal condition. | Dispose of the same complete group. |
| An internal collision reaches the short limit. | Retain the ordinary source or its fragment group under the lifetime rule. | Dispose of the same complete group. |
| Lifetime expires. | Dispose of the complete group and retire its record once. | Perform the same group cleanup if the group remains. |
| Basic DATA acknowledgment fails. | Apply the Block Ack lifetime rule. | Apply the same DATA lifetime rule. |

The two named RTS exhaustion configurations will assert the accepted choice. The internal-collision case will assert the same disposal decision. Each case will check group ownership, absolute deadlines, ACK entries, and AC recovery. A separate peer and TID will remain intact. Admission refusal will change none of these counters.

The C5 implementation uses commitment before RTS after complete admission. HCF selects the 2016 contract for enabled Basic support, including fallback without an agreement. The recovery owner retains the counters across fragment replacement. An accepted agreement suppresses retry-limit disposal. Without an accepted agreement, the short and long limits still control disposal. The modern path uses IEEE Std 802.11-2024 and does not depend on this decision.
