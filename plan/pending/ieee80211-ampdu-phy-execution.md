# HT A-MPDU receiver integration

The branch uses master at `f5612791b773bc83f654dbfdefa26e243f756c86`. The preceding commits supply VHT identity, bounded ranges, production deaggregation, HT-SIG representation, and pure range probabilities.

P2 adds cache-owned reception decisions and copied range outcomes. Rx rejects aggregate containers before standalone FCS or header checks. The MAC validates advertised receive capabilities before its MIB update. The result owns no reception pointer. MAC member acceptance and implicit Block Ack remain pending.

The P2 module and protocol fixtures define expected checks for partial loss, cache reuse, capability rejection, malformed input, and the production Rx boundary. Only the final commit receives build and test execution. The final completion record will state observed results and their limits.
