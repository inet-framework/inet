# Group-frame Duration/ID under single protection

IEEE Std 802.11-2024, 9.2.5.2 a) 9) (p. 710), sets Duration/ID to zero for the final group frame
of a single-protection TXOP. Clause 10.23.2.8 (p. 2010) requires separate contention
for each group frame in a QoS IBSS.

The earlier single-protection fixes supply the two pieces this repair uses.
`c493ce2aa9` excludes the current group frame's ACK, and `6e812198d4` suppresses the
group successor when Block Ack is disabled. BD-25 applies the same successor decision in
the legacy path used when Block Ack is enabled.

## Test

[`WifiSingleProtectionGroupDuration.test`](common/WifiSingleProtectionGroupDuration.test)
uses one ad hoc QoS sender on a scalar radio medium. At 1 ms it submits a broadcast QoS DATA
packet to HCF. The 120-byte fragmentation threshold lets HCF extract two MPDUs from the packet
before the first transmission. The sender uses single protection, a 1 ms TXOP limit, and zero
contention-window bounds. A positive limit matters because the legacy zero-limit path already
omits the successor.

The test inspects the headers when the radio completes each transmission. It checks that the two
frames have the same sequence number and the expected fragment numbers and More Fragments bits,
and that HCF completes the first frame sequence before the second transmission completes.
Both transmitted Duration/ID fields must be zero.

The parent identifies the second in-progress MPDU as a successor when Block Ack support is
enabled. Single protection adds that MPDU's airtime and one SIFS to the first frame's Duration/ID.
HCF ends the group sequence and the second MPDU needs its own channel access. The fix omits the
successor from the duration calculation for group Data and Management frames.

Clause 10.2.7 (p. 1881) prohibits fragmentation of group traffic. The fixture uses current model
behavior to put a successor in the in-progress queue. It checks the transmitted field and sequence
boundaries. The existing `Ieee80211SingleProtectionGroup_1` module test covers group Management and
non-QoS Data, including the protection equation when a caller supplies a successor.

## Comparison

All runs use seed set 0. The parent is `5bcb5a27f5`. Its compiled source tree is identical to
`897bbb4be9`, which supplied the debug library for the parent run. The fixed run uses the
rebuilt model library from the group-duration branch.

| Run | Block Ack support | Parent Duration/ID, first and second | Fixed Duration/ID, first and second |
| --- | --- | --- | --- |
| 0 | Disabled | 0 µs, 0 µs | 0 µs, 0 µs |
| 1 | Basic enabled | 96 µs, 0 µs | 0 µs, 0 µs |
| 2 | Compressed enabled | 96 µs, 0 µs | 0 µs, 0 µs |

The parent regression exits with status 1 after both enabled cases reach the Duration/ID check.
The fixed debug and release runs pass all three cases. The Basic missing-data and Compressed Block Ack
protocol controls and the single-protection group module test pass in debug.

Run these commands from the INET checkout root after loading the OMNeT++ toolchain with `source .envrc`.

```sh
make MODE=debug -j8
make MODE=release -j8
opp_run_protocol_tests --load . -p inet -m debug --no-build \
  -w '^tests/protocol/wifi$' \
  -f '(WifiSingleProtectionGroupDuration|WifiBasicBlockAckMissingData|N_CompressedBa)\.test$'
opp_run_protocol_tests --load . -p inet -m release --no-build \
  -w '^tests/protocol/wifi$' -f 'WifiSingleProtectionGroupDuration\.test$'
opp_run_module_tests --load . -p inet -m debug --no-build \
  -f 'Ieee80211SingleProtectionGroup_1\.test$'
```

For the parent comparison, run the same generated debug fixture with the parent library in
`LD_LIBRARY_PATH`. Use `tests/protocol/wifi/work/WifiSingleProtectionGroupDuration` as the working
directory and set `INET_ROOT` to the checkout root.

```sh
./WifiSingleProtectionGroupDuration_dbg -u Cmdenv -f test.ini \
  -n ".:../../ned:../../../lib:$INET_ROOT/src" -r 0..2
```
