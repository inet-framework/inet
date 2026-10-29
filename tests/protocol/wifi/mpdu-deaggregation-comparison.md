# MPDU deaggregation: parent and commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-29 · **Seal:** none · **Owns:** — · **Stands on:** [Test guide](mpdu-deaggregation.md)

This comparison checks production extraction after a malformed delimiter. The same [fixture](common/WifiMpduDeaggregation.test) must fail on the parent and pass after bounded extraction replaces unchecked lengths. The [guide](mpdu-deaggregation.md) explains the input layout, ownership, and recovery path.

## Inputs and expected difference

| Input | Revision |
| --- | --- |
| Parent source | `dea51e29fc820034b109f57ab2b0b668b1909a88` |
| Behavior source | `adb0b5db0a1fdeeaf89e525aec98ae49a55db48c` |
| Behavior commit subject | `ieee80211: fix: recover bounded MPDUs during deaggregation` |

Four seed-zero runs combine complete or malformed input with typed chunks or reconstructed bytes. Each aggregate contains 232 octets. The malformed case declares a 2000-octet middle member where only 124 octets remain. The parent requests that unchecked slice. After the correction, the deaggregator returns the first and last complete members. Complete input must return all three members on both revisions.

Both revisions receive identical fixture bytes. Their fixture SHA-256 is `e10177decf8c3e81f78c40104731a151917cb56043a7be1e796822a280576c4f`. These source revisions precede the fixture-document amendments. The amendments must preserve each production tree and the shared test library.

## Observed results

Both debug builds and both fixture compilations pass. On the parent, the two complete-input controls pass. The typed malformed input fails during the unchecked extraction with `length is invalid`. After the correction, all four cases return the required members and pass. The controls separate malformed-input recovery from ordinary extraction.

| Revision | Build exit | Runner exit | Observed simulation runs |
| --- | ---: | ---: | --- |
| Parent | 0 | 1 | Two controls pass. One malformed case errors. One later case is skipped. |
| Behavior commit | 0 | 0 | All four cases pass. No case is skipped. |

Cmdenv stops after the first error. This comparison therefore supplies no executed parent result for raw-byte malformed input. The corrected commit passes both malformed representations.

## Commands and evidence

The comparison directory is `/tmp/ampdu-protocol-parent-20261029/checkout`. Each execution installs the same fixture at its original path after source checkout. A fresh debug build precedes each filtered invocation. The temporary descriptor preserves the checked-in protocol settings and points to this directory. It omits an unsupported coverage-report setting.

```sh
record=/tmp/ampdu-protocol-parent-20261029
export MAKEFLAGS='--jobserver-style=pipe LN=ln\ -sf'
make MODE=debug -j8
opp_run_protocol_tests --load "$record/inet-compatible.opp" -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/wifi$' -f 'WifiMpduDeaggregation\.test$'
```

The `deaggregation-parent/` and `deaggregation-current/` artifact directories contain build logs, runner logs, native results, compressed simulation output, and input hashes. Each `evidence.json` records the exact revision, production tree, command, and exit status. The fixture expects PASS on both revisions. Its parent failure must remain visible without an expected-failure declaration.

This comparison proves bounded production extraction for the selected layouts. It does not prove radio reception, member FCS acceptance, or MAC delivery. The guide supplies the sequence diagram; this record adds no separate interaction.
