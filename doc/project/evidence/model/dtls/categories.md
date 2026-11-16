# DTLS verification categories

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/dtls/checks.md), [test-anatomy.md](../../../design/test-anatomy.md)

The selected checks combine local tests, protocol exchanges and runnable examples. Execution status belongs to [results.md](results.md); category selection alone establishes no behavior.

| Observation | Category | Reason |
| --- | --- | --- |
| Encoded header/body octets, length rejection, reconstruction arithmetic and modeled protection overhead | Unit | These are deterministic local transformations. Independent byte expectations distinguish layout checks from a serializer merely agreeing with itself. |
| Directional processing service, capacity, cancellation and lifecycle ownership | Module | These require the simulation clock and module lifecycle, without substituting a fake peer handshake. |
| Handshake ordering, cookies, negotiation, amplification limits and peer independence | Protocol | A driver calls the production socket API and observes UDP traffic on the link and endpoint callbacks. |
| Fragmentation, selective ACKs and recovery timing | Protocol with interception | Deterministic packet loss, duplication or reordering supplies conditions ordinary senders cannot reliably produce. |
| Application datagrams, replay, invalid records, KeyUpdate and close | Protocol with interception where needed | Assertions concern production endpoint decisions and their observable consequences. Crafted records exercise rejection paths. |
| Example initialization and execution over IPv4/IPv6, with loss and processing costs | Simulation | These demonstrate usable configurations. A successful example cannot replace assertions for a standards claim. |
| First recorded scalar results for the six new examples | Statistical regression | These establish references for later trajectory comparisons, independently of protocol correctness assertions. |

Packet captures and signals are observations, not authentication evidence. EncryptedChunk payload retention cannot establish real ciphertext interoperability. No statistical performance claim is made. The new examples nevertheless require first-record scalar regression baselines under the current upstream workflow; those detect later trajectory changes and do not establish a standards or performance claim. No existing fingerprint is changed by this work.
