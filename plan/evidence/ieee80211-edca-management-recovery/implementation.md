# EDCA management recovery verification

Validated on 2026-09-23 against base `9ccb5c8e597086871c435155d67e5ad548bd4c4e`.
Commands ran from the repository root unless stated otherwise. Simulation tests
used the freshly built debug library. Raw logs, per-run inventory, pre-fix fixture
and author self-audit are local artifacts, not repository inputs. They are retained
in the Git-local directory returned by
`git rev-parse --git-path verification-evidence/ieee80211-edca-management-recovery`.
Artifact names below are relative to that directory.

## Build and direct regression evidence

- `make MODE=debug -j$(nproc)` passed before and after the production change
  (exit 0; `prefix-build.log`, `debug-build.log`).
- `make MODE=release -j$(nproc)` passed (exit 0; `release-build.log`).
- Before the fix, `inet_run_module_tests -m debug -f 'Ieee80211HcfInternalCollision_1\.test$'`
  failed on the intended assertion: an injected VO management collision changed
  BE CW from 31 to 63 (exit 1; `prefix-collision.out`, `prefix-collision.err`,
  fixture `prefix-collision.test.txt`). An earlier stale-library link failure was
  resolved by rebuilding before this behavioral observation.

The following debug commands cover six module fixtures and **40 passing runs**:

```sh
inet_run_module_tests -m debug -f 'Ieee80211(HcfInternalCollision|HcfManagementRecovery|MgmtApHcfRtsTimeout)_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211HcfManagementRecovery_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211MgmtAp(HcfQueueDrop|QueueDrop|Timeout)_1\.test$'
```

The combined invocation exited 1: collision and all 12 RTS-timeout cases passed,
while the new ACK fixture failed during setup. After correcting an unspecified
synthetic sequence number and a runtime write to a non-mutable tester parameter,
the standalone ACK campaign exited 0 with all 24 cases passing. The three controls
also passed (exit 0). Logs: `focused-tests.log`, `management-test.log`,
`controls.log`; individual outputs: `Ieee80211*.out` and `Ieee80211*.err`.

| Fixture / General configuration | Runs and seeds | Claim checked |
| --- | --- | --- |
| HcfInternalCollision | Run/seed 0 | VO CW growth, VI saturation, BK retry limit one; all four CW/counter snapshots, cleanup and exactly-once terminal callbacks. Injected dispatch only, not arbitration. |
| MgmtApHcfRtsTimeout | Runs 0–11; seeds 0/1/2 × retry limits 1/2 × CWmax 3/15 | Actual missing CTS, local CW signal source, exact RTS/drop/retry-limit/callback counts and association cleanup. |
| HcfManagementRecovery | Runs 0–23; seeds 0/1/2 × short/long frames × retry limits 1/2 × CWmax 3/15 | Missing ACK then success or discard, followed by a fresh frame; CTS, management/QoS multicast isolation and local retry cleanup. BE starts above CWmin to detect wrong resets. |
| MgmtApHcfQueueDrop, MgmtApQueueDrop, MgmtApTimeout | Run/seed 0 each | HCF/DCF queue-drop and non-QoS DCF timeout controls. |

## Fingerprint evidence

From `tests/fingerprint`:

```sh
./fingerprinttest -d -m '/showcases/wireless/qos/' -f tplx -f '~tNl' -f '~tND'
./fingerprinttest -d -m 'MacQosWithBlockAck' -f tplx -f '~tNl' -f '~tND'
```

The first command passed both before and after (exit 0): `Qos` and `NonQos`,
run/seed 0, 10 seconds. Their baselines stayed unchanged. Artifacts:
`prefix-fingerprint.log`, `fingerprint.log`.

The user approved updating the `tests/fingerprint/examples.csv` row for
`/examples/wireless/qos/`, `MacQosWithBlockAck`, run/seed 0, 10 seconds, after CI
reported a mismatch. The second command passed the one selected configuration
with the updated baselines (exit 0; `blockack-fingerprint.log`):

| Ingredient | Previous | Updated |
| --- | --- | --- |
| tplx | 6f59-bd7d | 9d64-cc8f |
| ~tNl | c3ce-3389 | 6da9-a463 |
| ~tND | 9e1e-7c17 | d5b1-8768 |

ADDBA management recovery also uses the corrected EDCAF owner, explaining why
this scenario can change while the showcase controls stay stable. No
parent/current first-divergence trace was collected. `tyf` was neither updated
nor validated by this invocation; statistical baselines remain unchanged.

## Gates and limits

The scoped IEEE 802.11 architecture gate, document-seal check, final one-commit
commit/classification checks, source-seal guard and whitespace checks passed.
Changed source paths are unsealed and no new exception is introduced. The global
architecture, naming and interface checks exited 1 for pre-existing findings;
the earlier empty-range commit check also failed as a tool issue. Local gate
logs retain these findings (`architecture*.log`, `naming*.log`, `interfaces.log`,
`commits.log`, `classification.log`, `source-seals.log`, `doc-seals.log`). The
old empty-range logs are not results for the final one-commit range.

All 14 HCF management recovery calls select the affected EDCAF. The author
self-audit found no new changed-contract violation; no independent review is
claimed. Source/tests were not changed by the evidence cleanup, so behavioral
checks were not repeated for it.

Separate QoS/non-QoS counter models, older threshold/retry rules, sequence-key
limitations and lifecycle reset gaps remain. RTS failures increment the short
frame map, but length-based terminal cleanup can select the long map and leave
the short entry. ACK/data-failure cleanup is covered; this repair does not claim
to fix RTS-map cleanup, add non-QoS HCF data support, or implement complete
IEEE 802.11-2024 recovery semantics.
