# Wi-Fi protocol test suite

Protocol tests for INET's IEEE 802.11 MAC/PHY paths through HT and VHT, including
management, data exchanges, and optional-feature probes. Tests have different evidence
strengths: a passing presence check is not full standards conformance, and a failure can
identify a model defect, a scenario prerequisite, or a missing observation.

## Run the suite

From the repository root, with the normal OMNeT++/INET environment sourced:

```sh
make MODE=debug -j$(nproc)
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/N_BlockAck\.test$'
```

The integrated runner generates and compiles each selected `.test` under `work/<name>/`
and links the shared protocol-test support. The test filter matches the full `.test` path.
Do not run overlapping invocations that share generated directories. See the
[protocol authoring guide](../lib/AUTHORING.md) for the test format.

`WifiTestSupport.h` supplies typed management-body, action, element, and PHY-mode
observations. Shared topology/configuration inputs live under `ned/` and `ini/`;
individual tests may add local scenario fixtures.

## Source inventory

The suite contains **75 tests: 45 default PASS expectations and 30 declared FAIL expectations**.
These are source declarations, not a fresh full-suite result.

| Directory | Tests | Expected PASS | Expected FAIL |
| --- | ---: | ---: | ---: |
| `common/` | 19 | 10 | 9 |
| `legacy/` | 6 | 5 | 1 |
| `11b/` | 6 | 4 | 2 |
| `11a/` | 4 | 4 | 0 |
| `11g/` | 7 | 6 | 1 |
| `11n/` | 18 | 11 | 7 |
| `11ac/` | 15 | 5 | 10 |
| **Total** | **75** | **45** | **30** |

The [audit report](../../../doc/project/evidence/model/wifi/results.md) owns the observed
results, failure classifications, repair dependencies, and verification records. It includes
six repaired behavior checks: HT Capabilities, HT Operation, the modeled dummy four-message
authentication exchange, deauthentication, reassociation, and deterministic retransmission.
The authentication check does not establish WEP cryptography.

Of the 30 expected failures, 17 are retained unsupported-feature probes or explicit limitations;
13 are deferred repairs declared by maintainer decision. Five of those 13 retain explicit
`TESTABILITY GAP` assertions. `Ac_Ldpc` checks the VHT coding-observation gap separately;
the historical `Ac_LdpcStbcCap` filename now covers STBC only.

## Outcome semantics

The test asserts its intended successful observation and `%contains` checks the
`PROTOCOLTEST <name>: PASS` line. The `%# expected-result: FAIL` declaration changes how
the runner classifies that observed verdict; it does not turn a failing assertion into PASS.

- Actual FAIL with expected FAIL is reported as **FAIL (expected)**.
- Actual FAIL with expected PASS is reported as **FAIL (unexpected)**.
- Actual PASS with expected FAIL is reported as **PASS (unexpected)** and calls for review
  of the declaration and evidence.

The integrated runner can exit 1 for an all-FAIL selection even when every individual failure
is expected. Inspect individual verdicts as well as process status. The audit report records
this behavior for the focused deferred-repair runs.

A future implementation alone cannot make an unconditional testability-gap assertion pass:
it must also supply the missing observation and a faithful assertion. Expected failures do not
establish support or complete standards coverage.

## Adding or repairing a test

Use a unique `.test` basename, select the named test with `*.tester.testName`, and provide
explicit stimulus, prerequisites, and a bounded observation window. Assert the intended
successful behavior; do not replace the `%contains` PASS line with FAIL to disguise a gap.
Choose a unit or module test when an algorithm or internal state is the actual claim.

Follow the [test derivation and failure-classification procedure](../../../doc/project/guide/derive-tests-from-a-standard.md)
and the [expectation-change procedure](../../../doc/project/guide/change-a-baseline.md).
Record limitations next to any authorized expected-failure declaration and keep the audit
report consistent with changes to the source inventory.
