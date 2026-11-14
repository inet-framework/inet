# DSDV paper extension execution record

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [plan](../../../../../plan/pending/dsdv-paper-extensions.md)

The extension branch starts at `dc594f8b2e0b2cd6c7ef23f484af356143202e00`. The user authorized implementation and publication on 2026-11-14. The base implementation is pending upstream in PR #1444; this record concerns the additional features only.

| Feature | Build | Direct tests | Parent comparison |
| --- | --- | --- | --- |
| C1 incremental fairness | debug PASS | dissemination unit and fairness protocol PASS | Source comparison only, runtime NOT_RUN |
| C2 MAC feedback | NOT_RUN | NOT_RUN | NOT_RUN |
| C3 adaptive full dumps | NOT_RUN | NOT_RUN | NOT_RUN |
| C4 base-station routes | NOT_RUN | NOT_RUN | New profile cannot run unchanged on the parent |

## C1: Incremental fairness

The authoring contract was checked against `DsdvTable::prepare`, `suspend` and its adapter caller before editing: the table owns the cursor, required entries keep priority, only submitted optional entries advance it, and a full dump/session resets it. The staged C1 production snapshot was built before its direct tests. Later C2 source edits are not part of this result.

From the checkout root on 2026-11-14, debug mode:

```sh
make -j8 MODE=debug
opp_run_unit_tests --load . -p inet -m debug --no-build -f 'DsdvDissemination\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'DsdvIncrementalFairness\.test$'
doc/project/enforcement/check-architecture.sh src/inet/routing/dsdv
```

Build, scoped architecture and both selected tests exited 0, with one unit and one protocol case executed. The protocol fixture uses General/run/seed 0 and observes real UDP/radio delivery of optional destinations 4, 5, 6, required-only capacity and full-dump reset. The unit case additionally checks wraparound, candidate removal and uniqueness.

Raw build/unit logs: `/tmp/dsdv-extensions-c1-build.log`, `/tmp/dsdv-extensions-c1-unit.log`; final protocol log: `/tmp/dsdv-c1-protocol-final.log`. Generated protocol output is `tests/protocol/dsdv/work/DsdvIncrementalFairness/test.out`; later protocol runs may replace generated work. The first fixture attempts exposed missing outgoing-interface metadata and an incomplete include; the corrected fixture reaches the intended routing assertions.

The parent's address-order traversal is a source-grounded explanation of the expected fairness failure, not an executed parent comparison. No fingerprint baseline has been changed. Release compilation and final integration checks remain pending.
