# Extract bounded MPDUs from an A-MPDU

The production deaggregator must return complete members without an out-of-bounds request. Its [fixture](common/WifiMpduDeaggregation.test) covers typed chunks and reconstructed bytes. The [plan](../../../plan/pending/ieee80211-mpdu-deaggregation.md) assigns the shared traversal and production consumer.

The aggregate contains members of 100, 100, and 20 octets. Each member follows a four-octet delimiter. These members require no alignment padding. Complete input must return all three exact members.

The malformed case changes the middle declared length to 2000 octets. Only 124 octets remain after that delimiter. Before this commit, the deaggregator requests an out-of-bounds slice. After this commit, the shared traversal stops at that delimiter. The deaggregator returns only the first complete member. Delimiter recovery belongs to a separate PR.

The traversal pops delimiters and MPDUs from a bounded Packet view. It passes each MPDU chunk directly to the deaggregator callback. The deaggregator creates an output packet and copies its region tags. It deletes the input aggregate after traversal. The driver checks each output and deletes every output packet and the result vector.

Four seed-zero runs cover complete and malformed input in both representations. The driver emits the required test event only after all assertions and cleanup. Extraction does not establish PHY success, FCS acceptance, or MAC delivery.

## Verification and limits

Build and test execution are deferred. The expected focused command is:

```sh
opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiMpduDeaggregation\.test$' --no-concurrent
```

This command requires a fresh matching debug library. No execution result exists for the revised commits. The fixture does not cover radio reception, upper delivery, every malformed delimiter, or region tags. The unit fixture supplies additional alignment and bounds assertions.
