# TXOP fingerprint differences after C5

The selected fingerprint campaign reports two behavior changes and one inherited graphical mismatch. No recorded baseline changes. The [raw result](c5-fingerprints.txt) contains every old and new value.

The comparison uses debug libraries. The base checkout has revision `d841dc77db7d0b141bf31e5e215a0de9bb281957`. Its library SHA-256 is `bc3595467d81df205051bc14f684b4e7ccae7c367c0efea7358fed1599efcaf7`. The final changed library SHA-256 is `1de36100640807565d8456334072201ff3f412a3e5fbfe28e0376a5e0fc57bf2`.

## First event differences

The paired trace commands use the same configuration, run zero, and a ten-millisecond limit. Only the library path and output paths differ. The [command record](c5-causal-commands.json) gives the exact arguments. The excerpt files retain the event records and relevant log messages.

| Configuration | First cause and observable result |
| --- | --- |
| Aggregation | Event 88 sends the fifth radio transmission at 0.00043623452 seconds. The base sends `UdpBasicAppData-2+UdpBasicAppData-3`, a 354-byte A-MSDU with 78 microseconds of airtime. The changed library sends `UdpBasicAppData-2`, a 192-byte ordinary MPDU with 54 microseconds of airtime. Event 90 first differs in time and module. |
| Fragmentation | Event 39 receives the ACK for `Data-0-frag0` at 0.00031761726 seconds. The base ends the sequence and releases the channel. The changed library sends `Data-0-frag1` after SIFS within the same zero-limit TXOP. Event 40 first differs in time and module. |
| NoAggregation | All 883 event times and module IDs match through ten milliseconds. The full wrapper comparison below also matches every fingerprint value. |

The Aggregation configuration selects non-HT `g(erp)`. HCF now requires local and received peer HT capabilities before A-MSDU preparation. The changed ordinary MPDU therefore implements the intended permission condition. This behavior change explains the different packet, timing, and graphical fingerprints.

The Fragmentation configuration uses Basic and a zero TXOP limit. The corrected sequence permits the fragment burst of one MSDU. It protects the next fragment through its Duration field. Basic DATA with Block Ack policy has no immediate ACK. The reservation therefore includes no extra response SIFS. The direct Basic fixture asserts this Duration under both protection classes.

These are first-cause observations. They explain why later trajectories differ. They do not enumerate every later packet or establish conformance of the showcase configurations.

## Inherited NoAggregation mismatch

The exact wrapper command uses a one-second limit, computed checksums and FCS, and the wrapper's fake-GUI options. Both libraries finish event 95921 with the same result:

```text
21a4-c882/tplx, ecf2-6e33/~tNl, 5d9e-8fdf/~tND, cb71-81f8/tyf
```

The recorded graphical expectation is `27f2-4a52/tyf`. The unchanged base also produces `cb71-81f8/tyf`. The mismatch is therefore inherited. The [base output](c5-noaggregation-base.txt), [changed output](c5-noaggregation-head.txt), and [command record](c5-noaggregation-commands.json) retain this comparison.

## Final changed values

| Configuration | Values that differ from the recorded CSV |
| --- | --- |
| NoAggregation | `cb71-81f8/tyf` |
| Aggregation | `b7d0-1c67/tplx;d8f8-2120/~tNl;b2ba-0919/~tND;7c83-f2e4/tyf` |
| Fragmentation | `9fd0-6388/tplx;22f5-8db8/~tNl;68cc-5669/~tND` |

The behavioral fixtures pass independently of these stored expectations. Baseline approval remains a separate action.
