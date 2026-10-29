# HT-SIG bytes: parent and commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-29 · **Seal:** none · **Owns:** — · **Stands on:** [Test guide](ampdu-ht-sig-wire.md)

This comparison checks HT-SIG bytes at production transmission start. The same [fixture](common/WifiHtAmpduWire.test) must fail on the parent and pass after HT-SIG serialization exists. The [guide](ampdu-ht-sig-wire.md) explains the independent vector and protocol requirement.

## Inputs and expected difference

| Input | Revision |
| --- | --- |
| Parent source | `adb0b5db0a1fdeeaf89e525aec98ae49a55db48c` |
| Behavior source | `24448018e20838b49376e6aee458a52fb54c8fec` |
| Behavior commit subject | `ieee80211: add: preserve HT-SIG fields through byte conversion` |

Two seed-zero runs send a 104-octet singleton aggregate through typed or raw-byte transmission. Both select HT20 MCS 0 and long GI. Both must produce `00 68 00 0F B4 01`. The parent supplies no HT-SIG bytes. After the correction, production encapsulation and serialization preserve the singleton's Aggregation field and CRC.

The fixture reads new decoded fields through descriptors. It therefore compiles without the parent's missing getters. Missing fields or wrong bytes still fail during execution. Both revisions receive identical fixture bytes. Their fixture SHA-256 is `9f0893450f251b318b1ae99ce0bae26a1dedf345fd69f17fe7932815f3f71d82`.

These source revisions precede the fixture amendments. The amendments must preserve each production tree and the shared test library.

## Observed results

Both debug builds and both fixture compilations pass. The typed parent case requests HT-SIG bytes at transmission start. Its empty serializer triggers the serialization length check. After the correction, typed and raw-byte transmission both match the independent vector. Independent decode also passes every required field check.

| Revision | Build exit | Runner exit | Observed simulation runs |
| --- | ---: | ---: | --- |
| Parent | 0 | 1 | One runtime error. One later run is skipped. |
| Behavior commit | 0 | 0 | Both runs pass. No run is skipped. |

The parent error is `serialized length is incorrect: serialized=0 bit, chunk=48 bit`. This is a production serialization failure. It is not a compile failure from a missing getter. Cmdenv stops after that error, so the raw-byte parent case does not execute.

## Commands and evidence

The comparison directory is `/tmp/ampdu-protocol-parent-20261029/checkout`. Each execution installs the same fixture at its original path after source checkout. A fresh debug build precedes each filtered invocation. The temporary descriptor preserves the checked-in protocol settings and points to this directory. It omits an unsupported coverage-report setting.

```sh
record=/tmp/ampdu-protocol-parent-20261029
export MAKEFLAGS='--jobserver-style=pipe LN=ln\ -sf'
make MODE=debug -j8
opp_run_protocol_tests --load "$record/inet-compatible.opp" -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/wifi$' -f 'WifiHtAmpduWire\.test$'
```

The `wire-parent/` and `wire-current/` artifact directories contain build logs, runner logs, native results, compressed simulation output, and input hashes. Each `evidence.json` records the exact revision, production tree, command, and exit status. The fixture expects PASS on both revisions. Its parent failure must remain visible without an expected-failure declaration.

This comparison proves transmit representation and independent decode. It does not prove peer reception, member FCS acceptance, or implicit Block Ack. The guide supplies the sequence diagram; this record adds no separate interaction.
