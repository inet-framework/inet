# Partial HT reception: parent and commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-29 · **Seal:** none · **Owns:** — · **Stands on:** [Test guide](ampdu-partial-reception.md)

This comparison checks copied PHY outcomes at the production Rx drop. The same [fixture](common/WifiHtAmpduPartialReception.test) must fail on the parent and pass after separate range outcomes reach Rx. The [guide](ampdu-partial-reception.md) explains the production path and its current MAC rejection boundary.

## Inputs and expected difference

| Input | Revision |
| --- | --- |
| Parent source | `bad67fef81824773e9eca138b755a01e63d7442c` |
| Behavior source | `bda41c85ad64ad35064d4771917f1f641ba5117a` |
| Behavior commit subject | `ieee80211: add+change: retain partial HT A-MPDU PHY outcomes` |

Each aggregate contains MPDUs of 100, 101, and 102 octets. Four loss patterns fail one MPDU position or all four-octet delimiters. The parent samples the aggregate as one packet and supplies no copied range indication. After the correction, Rx must receive intact bytes and six exact outcomes. Rx must reject the container before standalone frame checks. A subsequent valid CTS must set NAV for 50 microseconds.

The fixture reads the new indication through descriptors. It therefore compiles on the parent without that class. The assertions require the indication and every field. Both revisions receive identical fixture bytes. Their fixture SHA-256 is `270f1f4be6907fed36c995719d5b3248b21cc0f6202d4b88d18c71ff635b011c`.

The four loss patterns combine with both GIs, both byte representations, and both reception-part modes. These combinations produce 32 runs with seed zero. Each probability equals zero or one, so this claim requires no seed campaign. These source revisions precede the fixture amendments. The amendments must preserve each production tree and the shared test library.

## Observed results

Both debug builds and both fixture compilations pass. The parent reaches the production Rx drop but supplies no copied indication. The observer fails its `indication != nullptr` assertion. After the correction, all 32 cases retain the required outcomes and pass the subsequent CTS control. The missing indication therefore causes a runtime regression failure without a dependency on its new C++ class.

| Revision | Build exit | Runner exit | Observed simulation runs |
| --- | ---: | ---: | --- |
| Parent | 0 | 1 | One runtime error. The 31 later runs are skipped. |
| Behavior commit | 0 | 0 | All 32 cases pass. No case is skipped. |

Cmdenv stops after the first error. The executed parent case selects first-MPDU loss, long GI, typed bytes, and whole-signal reception. This comparison supplies no executed parent result for the other 31 combinations. The corrected commit executes every combination.

## Commands and evidence

The comparison directory is `/tmp/ampdu-protocol-parent-20261029/checkout`. Each execution installs the same fixture at its original path after source checkout. A fresh debug build precedes each filtered invocation. The temporary descriptor preserves the checked-in protocol settings and points to this directory. It omits an unsupported coverage-report setting.

```sh
record=/tmp/ampdu-protocol-parent-20261029
export MAKEFLAGS='--jobserver-style=pipe LN=ln\ -sf'
make MODE=debug -j8
opp_run_protocol_tests --load "$record/inet-compatible.opp" -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/wifi$' -f 'WifiHtAmpduPartialReception\.test$'
```

The `partial-parent/` and `partial-current/` artifact directories contain build logs, runner logs, native results, compressed simulation output, and input hashes. Each `evidence.json` records the exact revision, production tree, command, and exit status. The fixture expects PASS on both revisions. Its parent failure must remain visible without an expected-failure declaration.

This comparison proves copied PHY facts and current MAC rejection. It does not prove member delivery, implicit Block Ack, or correlated FEC behavior. The guide supplies the sequence diagram; this record adds no separate interaction.
