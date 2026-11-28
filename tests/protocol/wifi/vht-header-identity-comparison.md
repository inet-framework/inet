# VHT header identity: parent and commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-29 · **Seal:** none · **Owns:** — · **Stands on:** [Test guide](vht-header-identity.md)

This comparison checks the header class at production transmission start. The same [fixture](common/WifiVhtHeaderIdentity.test) must fail on the parent and pass after the VHT factory correction. The [guide](vht-header-identity.md) explains the production path and its limits.

## Inputs and expected difference

| Input | Revision |
| --- | --- |
| Parent source | `c99358b1ba9fa881e4961bdbc43697c31ce1f3b7` |
| Behavior source | `c3be2077d0edaf630735f151dfbbc94ce09f6097` |
| Behavior commit subject | `ieee80211: fix: create VHT headers for VHT signal modes` |

Both executions use a 100-octet ordinary PSDU, VHT20 MCS 0, typed transmission, and seed zero. Long GI uses one stream. Short GI uses two streams. On the parent, the factory returns an HT header. After the correction, the factory returns a VHT header. The fixture requires that class before it emits its success event.

Both revisions receive identical fixture bytes. Their fixture SHA-256 is `40b018e42279105dcd1d563afe4853e1fcfff2229b215f7ccb3479f4ce9bcb88`. These source revisions precede the fixture-document amendments. The amendments must preserve each production tree and the shared test library.

## Observed results

Both debug builds and both fixture compilations pass. The parent fixture fails during production transmission. The VHT class assertion detects the HT object. The behavior fixture passes both GI runs. The parent failure therefore comes from the changed factory contract, not a missing API or build error.

| Revision | Build exit | Runner exit | Observed simulation runs |
| --- | ---: | ---: | --- |
| Parent | 0 | 1 | One runtime error. One later run is skipped. |
| Behavior commit | 0 | 0 | Both runs pass. No run is skipped. |

The parent error includes `dynamicPtrCast<const Ieee80211VhtPhyHeader>(header) != nullptr`. Cmdenv stops after the first error. This comparison therefore establishes the parent failure in the long-GI case. It supplies no executed parent result for short GI.

## Commands and evidence

The comparison directory is `/tmp/ampdu-protocol-parent-20261029/checkout`. Each execution installs the same fixture at its original path after source checkout. A fresh debug build precedes each filtered invocation. The temporary descriptor preserves the checked-in protocol settings and points to this directory. It omits an unsupported coverage-report setting.

```sh
record=/tmp/ampdu-protocol-parent-20261029
export MAKEFLAGS='--jobserver-style=pipe LN=ln\ -sf'
make MODE=debug -j8
opp_run_protocol_tests --load "$record/inet-compatible.opp" -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/wifi$' -f 'WifiVhtHeaderIdentity\.test$'
```

The `vht-parent/` and `vht-current/` artifact directories contain build logs, runner logs, native results, compressed simulation output, and input hashes. Each `evidence.json` records the exact revision, production tree, command, and exit status. The fixture expects PASS on both revisions. Its parent failure must remain visible without an expected-failure declaration.

This comparison proves header class selection for these two modes. It does not prove VHT serialization, peer reception, or MAC delivery. The guide supplies the sequence diagram; this record adds no separate interaction.
