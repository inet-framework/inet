# Wi-Fi protocol tests — expectation audit

> **Kind:** report · **Status:** snapshot 2026-09-17 · **Seal:** none · **Owns:** — · **Stands on:** [failure classification](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected), [testing rules](../../../rule/testing.md)

Raw artifacts linked under `audit/` are local, Git-ignored evidence and are not included in the branch. The observations, classifications, and follow-up work are recorded below.

## Deferred-repair expectations

After the audit and rebase, the maintainer explicitly requested that the 12 failures from the
18-test focused run be declared expected: their repairs are nontrivial and deferred to future
contributions. This later decision supersedes the removal decision for those 12 tests only.
The six repaired passing tests retain their PASS expectations. The maintainer subsequently extended the same decision to the separately added `Ac_Ldpc`
test. The total deferred-repair scope is now 13 tests.

Each restored declaration includes its concrete repair dependency in the test description.
The scope is:

- `WifiQosDelayedBa`
- `N_ReverseDirection`
- `N_LdpcCap`
- `Ac_Ldpc`
- `Ac_80p80`
- `Ac_MuMimo`
- `G_ErpProtection`
- `B_Pbcc`
- `B_ShortPreamble`
- `N_Greenfield`
- `Ac_MultiTidBa`
- `Ac_Ampdu`
- `N_Ampdu`

This is an explicit maintainer-directed deferral, not evidence that the features work or that
all 13 tests establish model defects. In particular, `Ac_80p80`, `Ac_MuMimo`, `Ac_Ldpc`, `N_LdpcCap`, and
`N_ReverseDirection` retain their explicit testability-gap assertions. Declaring these five
expected is an explicit exception to the guide's normal treatment of untestable claims;
their diagnostic class is unchanged. `G_ErpProtection` still fails its association prerequisite.

The runner compares observed and expected verdicts and flags PASS against declared FAIL as
unexpected. For the five explicit testability-gap assertions, future code support alone cannot
produce PASS: the missing observation and faithful assertion must also be implemented.

The source inventory is now **30 expected-failure declarations**. Applying these expectations
to the historical full-suite observations gives **45 PASS, 30 expected FAIL, 0 unexpected FAIL**;
this is a reconciliation, not a new full-suite execution. The earlier run records remain historical evidence. The classification table below shows
the current expectations, including the deferred-repair declarations.

Historical verification on the rebased tree (`270014a45b`, dirty; superseded by the
committed-source verification below): `make MODE=debug -j$(nproc)`
succeeded, then the exact 12-test debug selection reported **12 FAIL (expected)**, with no
unexpected per-test verdicts. Run: 2026-09-17 17:10:51–17:11:36 +02:00; runner duration
43.28s. Its exact command was:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/(WifiQosDelayedBa|N_ReverseDirection|N_LdpcCap|Ac_80p80|Ac_MuMimo|G_ErpProtection|B_Pbcc|B_ShortPreamble|N_Greenfield|Ac_MultiTidBa|Ac_Ampdu|N_Ampdu)\.test$' --log-file /home/user/omnetpp_ws/inet/audit/protocol/wifi/deferred-expectations/runner.log
```

[Command](../../../../../audit/protocol/wifi/deferred-expectations/command.json),
[run metadata](../../../../../audit/protocol/wifi/deferred-expectations/run.json),
[output](../../../../../audit/protocol/wifi/deferred-expectations/output.log), and
[outcomes](../../../../../audit/protocol/wifi/deferred-expectations/outcomes.json) are local artifacts.
Build output is `/tmp/inet-wifi-deferred-build.log`. This run regenerates and compiles the selected
tests; their assertions and simulation configuration are unchanged by this declaration update.

The focused CLI exits **1**, despite every individual failure being expected: an all-FAIL selection
has aggregate FAIL, compared with aggregate expected PASS by `MultipleTaskResults`. This does
not invalidate the observed individual classifications. A direct result-object check also confirms
that observed PASS against expected FAIL has `expected == False`. The suite was not rerun in full.
`git diff --check` and the seal-index check passed.

The subsequent focused `Ac_Ldpc` verification reported **FAIL (expected)** in 2.858s.
Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/Ac_Ldpc\.test$' --log-file /tmp/inet-wifi-ac-ldpc-expected.log`.
Output: `/tmp/inet-wifi-ac-ldpc-expected-output.log`; CLI exit 1 has the same aggregate-result
behavior described above. The test retains its explicit missing-observation diagnostic.

## Reproducible verification of the deferred declarations

The review verification ran all **13 affected tests** at committed source
`4fb5592264d5469518de715f49b803da464bae67` (Git tree `dffbb6c14c269fd8cd88eb399a71671c1d15c825`).
There were no tracked changes when the build/run started; unrelated untracked plans were not
execution inputs. The subsequent review amendment changes only Markdown documentation and
adds the referenced plan; executable sources, test definitions and configuration are identical.
The input digest below provides an identity that survives the documentation-only amendment.

- Working directory: `/home/user/omnetpp_ws/inet`.
- Started: `2026-09-17T17:33:50.204175+02:00`; finished: `2026-09-17T17:34:23.165634+02:00`.
- Build: debug, `src/libINET_dbg.so`; `make MODE=debug -j$(nproc)` succeeded (exit 0).
- Environment: OMNeT++ 6.4.0aipre2, build `260526-7b9114f72c`; Ubuntu clang 21.1.8;
  Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64. Normal OMNeT++/INET setup sourced.
- Test configuration: `General`, seed-set 0, `WifiInfraNetwork`; unchanged per-test INI inputs.
- Outcome: **13 FAIL (expected), 0 unexpected verdicts**, runner duration 29.866s; CLI exit 1
  from the all-FAIL aggregation behavior described above. No generation/build/runtime errors.
- Logs: `/tmp/wifi-pr-review-verification/build.log`, `runner.log`, and `output.log`.
  These logs are supplementary local artifacts; the selector and source identity are recorded here.

Exact commands (from the repository root):

```sh
make MODE=debug -j$(nproc)
MPLCONFIGDIR=/tmp/inet-matplotlib inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/(WifiQosDelayedBa|N_ReverseDirection|N_LdpcCap|Ac_Ldpc|Ac_80p80|Ac_MuMimo|G_ErpProtection|B_Pbcc|B_ShortPreamble|N_Greenfield|Ac_MultiTidBa|Ac_Ampdu|N_Ampdu)\.test$' --log-file /tmp/wifi-pr-review-verification/runner.log
```

For reproduction in a fresh checkout, create `/tmp/wifi-pr-review-verification` before running.
The selector names each affected `.test` file; all 13 produced FAIL at their existing assertions,
including five explicit testability-gap diagnostics. This verification supersedes the earlier
separate 12-test and `Ac_Ldpc` runs for current declaration evidence.

The SHA-256 of tracked non-Markdown inputs under `src/`, `python/`, and `tests/protocol/` is
`fddd3407956b49a1624e560cd71a6c2802a6878242380991ee4f2aa3446da65e`. Verify it on the amended branch with:

```python
import hashlib
import pathlib
import subprocess

paths = subprocess.check_output(
    ["git", "ls-files", "src", "python", "tests/protocol"], text=True
).splitlines()
digest = hashlib.sha256()
for name in sorted(p for p in paths if not p.endswith(".md")):
    digest.update(name.encode() + b"\0" + pathlib.Path(name).read_bytes() + b"\0")
print(digest.hexdigest())
```

## Full-suite run before marker changes

- Run: 2026-09-17 14:43:26–14:45:51 +02:00 (Europe/Madrid), 2m 25s.
- INET: `master`, `bf7b4872bb3bc48a5b7d0bfc115812931392e939` **dirty**. The hash does not identify the tested tree; [changed paths](../../../../../audit/protocol/wifi/2026-09-17/changed-paths.txt) and [test patch](../../../../../audit/protocol/wifi/2026-09-17/test-source.diff) identify the edits.
- OMNeT++: 6.4.0aipre2, build `260526-7b9114f72c`; installation is not a Git checkout.
- Build: debug, INET library `src/libINET_dbg.so` (the successful pre-audit `make MODE=debug -j$(nproc)`); no compiled INET sources changed during this audit. The protocol runner rebuilt test executables and applicable support.
- Compiler: `Ubuntu clang version 21.1.8 (6ubuntu1)`; [full compiler/platform record](../../../../../audit/protocol/wifi/2026-09-17/run-record.json).
- Platform: Ubuntu 26.04.1 LTS; `Linux 7.0.0-31-generic x86_64`.
- Working directory: `/home/user/omnetpp_ws/inet`; configuration `General`, seed-set 0. The reassociation test uses `ReassociationNetwork`; other tests use `WifiInfraNetwork`.
- Full-suite command (with OMNeT++/INET sourced and `MPLCONFIGDIR=/tmp/inet-matplotlib`):

  ```sh
  inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
    --log-file /tmp/inet-wifi-audit-20260917/full.log
  ```

- Exit status: 1. [Runner output](../../../../../audit/protocol/wifi/2026-09-17/full-output.log); [per-test outcomes and failure excerpts](../../../../../audit/protocol/wifi/2026-09-17/outcomes.json).

## Scope and result

The original 74-test run reported 39 passes and 35 declared failures. This audit repairs the observations and triggers, classifies all 35 declarations, and adds one separate VHT LDPC test. It does not implement the missing optional features or claim standards conformance.

Observed full-suite artifacts before marker changes: **45 PASS, 30 FAIL (including 5 explicit testability gaps), 0 setup/runtime errors, 0 not recorded**, across 75 tests. The runner may label explicit testability exceptions as FAIL; the stage column below preserves the distinction.

**Historical removal pass (superseded above):** the user approved the exact 18-marker removal patch on 2026-09-17. Those declarations are removed; 17 remain. `Ac_Ldpc` is a new test with the default PASS expectation. Its explicit testability gap remains an unexpected failure.

## Verification after approval

The approved patch was applied and the exact 18 affected tests rerun in debug mode: **6 PASS, 12 unexpected FAIL**, exit 1, runner duration 2m 17.582s. All 18 outcomes match their pre-marker observations; the failures are now exposed by the runner. The run started `2026-09-17T15:14:41.718194+02:00` and finished `2026-09-17T15:17:04.853313+02:00` with the same environment and seed as the full-suite run.

[Exact command](../../../../../audit/protocol/wifi/2026-09-17/postapproval-command.json), [runner output](../../../../../audit/protocol/wifi/2026-09-17/postapproval-output.log), [per-test outcomes and excerpts](../../../../../audit/protocol/wifi/2026-09-17/postapproval-outcomes.json), and [tested source diff](../../../../../audit/protocol/wifi/2026-09-17/postapproval-test-source.diff) preserve this verification. An initial incorrectly anchored selector selected zero tests; its artifacts are preserved with the `empty-selection-` prefix and excluded from verification.

Combining the earlier full-suite observations with this focused rerun yields **45 passing tests, 17 expected failures, and 13 unexpected failures** across 75 tests. The extra unexpected failure is the separately added `Ac_Ldpc` test. This reconciliation is not a second full-suite execution. That historical source inventory contained 17 expected-failure declarations; the current inventory is 30.

## Repaired implemented behaviors

Typed management-body checks now observe HT Capabilities and HT Operation. A four-message dummy authentication exchange replaces the incorrect AP transaction-3 assertion. Test-local station agents trigger deauthentication and two-AP reassociation using production primitives. A test-local MAC suppresses one addressed ACK; production DCF retries the same sequence and fragment, and UDP delivers once.

Negative controls use each generated `./run` with `--cmdenv-log-level=off`:

| Test | Command-line override | Expected diagnostic |
| --- | --- | --- |
| Both HT IE tests | `'--**.wlan[*].opMode="g(mixed)"'` | Presence assertion fails on the legacy body. |
| Authentication | `'--*.ap.wlan[*].mgmt.numAuthSteps=2'` | No transaction 3; deadline fails. |
| Deauthentication/reassociation | `'--*.sta1.wlan[*].agent.typename="Ieee80211AgentSta"'` | Initial association completes but the untriggered operation is absent. |
| Retransmission | `'--*.sta1.wlan[*].mac.typename="Ieee80211Mac"'` | ACK suppression prerequisite is absent. |

Negative-control output is preserved in [audit/protocol/wifi/2026-09-17](../../../../../audit/protocol/wifi/2026-09-17). These diagnostics intentionally fail and are not suite failures. INET emits the test verdict on stdout; process success alone is not the verdict.

## Current classifications and expectations

The expectation column reflects the current source declarations. **FAIL (deferred repair)**
identifies the 13 maintainer-approved deferrals, including five explicit testability gaps;
**FAIL (retained limitation/probe)** identifies the other 17 declarations. **PASS (default)**
identifies the six repaired tests whose failure markers remain absent. This table covers those
36 audited tests; the other 39 tests retain their default PASS expectations.

A declaration does not change the diagnostic class or establish full feature support.
Retained presence probes do not validate complete procedures or their triggers. The original
[removal patch](../../../../../audit/protocol/wifi/2026-09-17/expectation-changes.patch) records
history only; the later deferral decision supersedes it for the 12 restored declarations.

| Source test | Observed / stage | Current expectation | Class | Evidence and follow-up |
| --- | --- | --- | --- | --- |
| [Ac_80p80](../../../../../tests/protocol/wifi/11ac/Ac_80p80.test) | FAIL / testability | FAIL (deferred repair) | Testability gap | No two-segment VHT channel representation is available. Previous predicate always returned false. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader.msg#L149). Follow-up: PHY channel owner: provide a two-segment observation before writing an executable 80+80 claim; do not use total bandwidth as proxy. |
| [Ac_Ampdu](../../../../../tests/protocol/wifi/11ac/Ac_Ampdu.test) | FAIL / assertion | FAIL (deferred repair) | Incomplete implementation | Same absent production aggregation integration as HT. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.cc#L17). Follow-up: Share the QoS integration fix with HT, then verify VHT mode and receive path separately. |
| [Ac_Beamforming](../../../../../tests/protocol/wifi/11ac/Ac_Beamforming.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No VHT sounding producer found. Probe checks NDP Announcement control subtype, not an invented action frame. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L74). Follow-up: Needs sounding trigger and feedback exchange; control subtype alone is not beamforming proof. |
| [Ac_GroupIdMgmt](../../../../../tests/protocol/wifi/11ac/Ac_GroupIdMgmt.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No VHT group assignment producer found; generic VHT action 1 is observable. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Needs MU group membership configuration and payload checks. |
| [Ac_Ldpc](../../../../../tests/protocol/wifi/11ac/Ac_Ldpc.test) | FAIL / testability | FAIL (deferred repair) | Testability gap | VHT transmit coding selection is not observable; split from the historical STBC test. [Owner/source](../../../../../tests/protocol/wifi/11ac/Ac_Ldpc.test). Follow-up: Coding/mode API owner: expose production code selection, distinguish capability negotiation from coding support, and replace the explicit gap assertion with a faithful observation. |
| [Ac_LdpcStbcCap](../../../../../tests/protocol/wifi/11ac/Ac_LdpcStbcCap.test) | FAIL / assertion | FAIL (retained limitation/probe) | Explicit limitation | VHT STBC is explicitly excluded; LDPC moved into a separate failing test. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h#L90). Follow-up: Historical filename retained; no combined STBC-or-LDPC claim remains. |
| [Ac_MuMimo](../../../../../tests/protocol/wifi/11ac/Ac_MuMimo.test) | FAIL / testability | FAIL (deferred repair) | Testability gap | VHT PHY header lacks group/user allocation observation; previous lookup named a nonexistent chunk. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader.msg#L149). Follow-up: PHY/MU owner: define observable PPDU users/group and scheduling stimulus; current explicit failure is not a model verdict. |
| [Ac_MultiTidBa](../../../../../tests/protocol/wifi/11ac/Ac_MultiTidBa.test) | FAIL / assertion | FAIL (deferred repair) | Incomplete implementation | Multi-TID declarations and rejection paths exist with bare unfinished declarations. Two flow/TID prerequisites are checked. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L477). Follow-up: Block Ack owner: implement agreement, request, response, and serializer paths; reproduce with both TIDs, not high rate alone. |
| [Ac_OpModeNotification](../../../../../tests/protocol/wifi/11ac/Ac_OpModeNotification.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No VHT operating-mode notification producer found; generic VHT action 2 is observable. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Needs runtime receive-width/NSS change stimulus. |
| [Ac_VhtIe](../../../../../tests/protocol/wifi/11ac/Ac_VhtIe.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | AP does not generate VHT Operation; opaque element representation can observe its presence. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc#L210). Follow-up: Tests Operation only; add a separate Capabilities check when VHT management is implemented. |
| [B_Pbcc](../../../../../tests/protocol/wifi/11b/B_Pbcc.test) | FAIL / assertion | FAIL (deferred repair) | Selection/testability gap | PBCC mode objects exist, but built-in b set includes only long-preamble CCK. Predicate now compares actual objects. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.cc#L40). Follow-up: PHY mode-selection owner: expose/test a supported PBCC selection path; do not equate object presence with coding validation. |
| [B_ShortPreamble](../../../../../tests/protocol/wifi/11b/B_ShortPreamble.test) | FAIL / assertion | FAIL (deferred repair) | Selection/testability gap | Short-preamble PHY objects exist but are absent from the built-in b mode set. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.cc#L66). Follow-up: PHY mode-selection owner: bind explicit preamble choice and verify transmit/receive support. |
| [G_ErpProtection](../../../../../tests/protocol/wifi/11g/G_ErpProtection.test) | FAIL / assertion | FAIL (deferred repair) | Setup gap | Scenario now contains an actual b-only STA and checks its association first; that prerequisite fails before protection is tested. [Owner/source](../../../../../tests/protocol/wifi/11g/G_ErpProtection.test). Follow-up: Management/rate compatibility owner: explain failed mixed-BSS association before judging ERP protection; current run cannot classify protection behavior. |
| [Legacy_Pcf](../../../../../tests/protocol/wifi/legacy/Legacy_Pcf.test) | FAIL / assertion | FAIL (retained limitation/probe) | Explicit omission probe | MAC explicitly excludes PCF. Predicate now checks CF-Poll data-subtype bits, not PS-Poll. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Mac.ned#L68). Follow-up: Needs PCF configuration and contention-free-period stimulus if implemented. |
| [N_2040Coex](../../../../../tests/protocol/wifi/11n/N_2040Coex.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No coexistence action/scanning producer found; generic public action 0 replaces misspelled nonexistent class. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Needs overlap/intolerance stimulus; HT channel negotiation alone does not implement coexistence management. |
| [N_Ampdu](../../../../../tests/protocol/wifi/11n/N_Ampdu.test) | FAIL / assertion | FAIL (deferred repair) | Incomplete implementation | Aggregate construction exists; originator call is commented out and default policy never aggregates. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc#L113). Follow-up: QoS data-service owner: define selection policy and integrate existing aggregate builder; prove production transmission. |
| [N_Greenfield](../../../../../tests/protocol/wifi/11n/N_Greenfield.test) | FAIL / assertion | FAIL (deferred repair) | Integration gap | Greenfield factory and timing branches exist; built-in modes are mixed format. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.cc#L326). Follow-up: PHY mode owner: inspect cache identity and selection before claiming greenfield support; add a focused PHY/module test. |
| [N_HtCapabilitiesIe](../../../../../tests/protocol/wifi/11n/N_HtCapabilitiesIe.test) | PASS / execution | PASS (default) | Repaired test | HT element stored in management body; repaired presence assertion passes. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc#L367). Follow-up: None |
| [N_HtOperationIe](../../../../../tests/protocol/wifi/11n/N_HtOperationIe.test) | PASS / execution | PASS (default) | Repaired test | AP populates HT Operation; repaired body assertion passes. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc#L218). Follow-up: None |
| [N_LdpcCap](../../../../../tests/protocol/wifi/11n/N_LdpcCap.test) | FAIL / testability | FAIL (deferred repair) | Untestable claim | Bare LDPC TODO and capability plumbing exist, but no transmit-code discriminator supports this assertion. Now reports explicit tooling gap. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtCode.h#L40). Follow-up: Coding/mode API owner: separate capability negotiation from FEC implementation; add an observable code choice and faithful PHY test. |
| [N_ReverseDirection](../../../../../tests/protocol/wifi/11n/N_ReverseDirection.test) | FAIL / testability | FAIL (deferred repair) | Testability gap | No HT Control RDG field exposed; previous lookup named a nonexistent chunk. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.cc#L231). Follow-up: MAC/frame tooling owner: add RDG representation and a trigger before measuring reverse-direction behavior. |
| [N_Smps](../../../../../tests/protocol/wifi/11n/N_Smps.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | SMPS IE bits are representable, but no SMPS action-generation procedure found. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame.msg#L137). Follow-up: Add power-mode change stimulus; distinguish IE encoding from dynamic action support. |
| [N_StbcCap](../../../../../tests/protocol/wifi/11n/N_StbcCap.test) | FAIL / assertion | FAIL (retained limitation/probe) | Explicit limitation | HT signal mode explicitly assumes STBC is not used. [Owner/source](../../../../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h#L99). Follow-up: No support claimed by this result. |
| [WifiAuthSharedKey](../../../../../tests/protocol/wifi/common/WifiAuthSharedKey.test) | PASS / execution | PASS (default) | Repaired test | Four dummy authentication transactions are modeled; no WEP cryptography claim. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc#L276). Follow-up: Legacy filename retained; description limits claim to modeled exchange. |
| [WifiCountryIe](../../../../../tests/protocol/wifi/common/WifiCountryIe.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | Opaque Country IE can be preserved, but AP beacon producer does not create one. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc#L210). Follow-up: Configure regulatory domain and check element content when implemented. |
| [WifiDeauth](../../../../../tests/protocol/wifi/common/WifiDeauth.test) | PASS / execution | PASS (default) | Repaired test | Primitive emits deauthentication and AP clears authentication state. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc#L528). Follow-up: None |
| [WifiDfsChannelSwitch](../../../../../tests/protocol/wifi/common/WifiDfsChannelSwitch.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No DFS/CSA producer found; generic spectrum action 4 is observable. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Add radar/channel-switch stimulus and CSA count/channel checks when implemented. |
| [WifiPmf](../../../../../tests/protocol/wifi/common/WifiPmf.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | protectedFrame exists but is unused by production management; predicate now reads it. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L126). Follow-up: Needs negotiated security and robust-management stimulus for a complete PMF check. |
| [WifiQosAddts](../../../../../tests/protocol/wifi/common/WifiQosAddts.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No ADDTS admission-control producer found; generic QoS action 0 is observable. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Needs a TSPEC admission request and response checks when implemented. |
| [WifiQosDelayedBa](../../../../../tests/protocol/wifi/common/WifiQosDelayedBa.test) | FAIL / assertion | FAIL (deferred repair) | Model defect | ADDBA accepts delayed policy, but first addressed ACK/BA after BAR is a BA; assertion requires ACK first. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosAckPolicy.cc#L49). Follow-up: Block Ack owner: implement delayed response/ACK and TXOP handling, or resolve the claimed negotiation contract explicitly. |
| [WifiQosUapsd](../../../../../tests/protocol/wifi/common/WifiQosUapsd.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | EOSP field is represented; no U-APSD buffering/trigger procedure found. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L214). Follow-up: Presence probe only; needs negotiated trigger/delivery ACs and buffered traffic for conformance. |
| [WifiReassociation](../../../../../tests/protocol/wifi/common/WifiReassociation.test) | PASS / execution | PASS (default) | Repaired test | Two-AP primitive-driven request/response/completion now observed. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc#L379). Follow-up: None |
| [WifiRetransmission](../../../../../tests/protocol/wifi/common/WifiRetransmission.test) | PASS / execution | PASS (default) | Repaired test | One addressed ACK suppressed; identical sequence/fragment retried; one UDP delivery. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.cc#L389). Follow-up: None |
| [WifiRrmMeasurement](../../../../../tests/protocol/wifi/common/WifiRrmMeasurement.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No RRM request-generation procedure found; generic category 5/action 0 now observable. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Add RRM configuration/request stimulus when feature is implemented; presence alone does not validate reports. |
| [WifiRsn4way](../../../../../tests/protocol/wifi/common/WifiRsn4way.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No RSN key-exchange producer found. Probe now observes SNAP EAPOL EtherType, not an invented EAPOL class. [Owner/source](../../../../../src/inet/linklayer/ieee8022/Ieee8022SnapHeader.msg#L46). Follow-up: Needs key management and EAPOL-Key parsing before asserting a four-way exchange. |
| [WifiTpc](../../../../../tests/protocol/wifi/common/WifiTpc.test) | FAIL / assertion | FAIL (retained limitation/probe) | Unimplemented feature probe | No TPC reporting producer found; generic spectrum action 3 is observed. [Owner/source](../../../../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg#L249). Follow-up: Add a TPC request and verify matching report when implemented. |

`Ac_Ldpc` is the new VHT coding-observation gap split from `Ac_LdpcStbcCap`; the historical combined filename now tests STBC only.

The source-search commands and matches supporting the bounded absence claims are recorded in [source-searches.json](../../../../../audit/protocol/wifi/2026-09-17/source-searches.json).

## Failure analysis and limitations

- Delayed Block Ack: the scenario explicitly requests and observes accepted delayed policy. A permissive ACK-only filter could skip the immediate BA and match an unrelated later ACK; the repaired test instead asserts that the first addressed ACK-or-BA after BAR is an ACK. Its failure identifies the recipient response contract. Legacy delayed-policy timing is not inferred from a fixed SIFS+PIFS constant.
- ERP protection: the new b-only station does not complete association in the mixed-BSS scenario. The failure occurs before the ERP assertion. Investigate that setup/compatibility path before calling protection behavior defective.
- A-MPDU and Multi-TID: partial implementations and bare unfinished declarations do not justify expected failures under the current project rule. The tests expose integration gaps; they do not identify a complete production repair by themselves.
- Five explicit observation-gap tests (`N_LdpcCap`, `Ac_Ldpc`, `Ac_80p80`, `N_ReverseDirection`, `Ac_MuMimo`) fail with `TESTABILITY GAP` diagnostics. They do not silently use `return false` or nonexistent chunk names as measurements. Future work must supply the observation and stimulus before these can become behavior tests.
- PBCC and short/greenfield preambles require explicit supported mode selection. Existing PHY objects are not proof of a reachable production path or a correct coding implementation.
- Unsupported-feature presence probes still need the negotiated state, traffic, management requests, or radio conditions listed per row before they can establish conformance. No catalog IDs or feature-support matrix is invented here.
- Removed ineffective sensitivity/SNIR/power assignments that followed `_base.ini`; the effective baseline settings remain 100mW, -85dBm and 4dB. Other unique test parameters remain explicit. Configuration corrections are limited to audited tests.
- Broad focused execution was interrupted after excessive two-flow Multi-TID trace volume. An overlapping diagnostic attempt is excluded from final evidence. The old generated directory was moved to `audit/protocol/wifi/2026-09-17/interrupted-multitid-work` to isolate lingering output; final suite generation uses a fresh Multi-TID directory. Final Multi-TID traffic is 2ms per flow with a bounded 1s response window.

## Standards and verification scope

The local IEEE 802.11-2024/802.11be-2024 corpus was rebuilt and linted. Lint reports 76 unresolved heading ambiguities and unresolved cross-references; it is not a clean global corpus. The inspected 802.11-2024 text includes 9.4.1.1–9.4.1.2 (authentication fields; PDF page 834), 9.3.1.19 (NDP Announcement; PDF page 737), 9.6.11.1 / Table 9-517 (SMPS action), and 9.6.22.1 / Table 9-605 (VHT actions). Generic action/category mappings and IE IDs were checked against the current text and source representation. No PDF visual inspection was needed for the text-only field checks. This is an implementation/test audit, not a completed standards-derived conformance campaign.

802.11-2024 reserves legacy WEP authentication algorithm 1; the repaired legacy authentication test explicitly validates INET’s dummy four-message exchange, not WEP or FILS conformance. Legacy delayed Block Ack/PCF obligations need the applicable older revision when extending these presence probes into complete standard-derived procedures.

Focused execution selected the 35 original failures plus `Ac_Ldpc`; its exact selector is in `focused-command.json`. Individual correction runs and negative controls established the six repaired behaviors and the delayed-policy defect. The final full run is integration evidence, not a substitute for those focused observations.

## Self-audit and remaining work

- Test-only change: no production C++, NED, MSG, generated message code, source seal, or existing passing test was modified. The shared helper is under the suite, not under the sealed packet subsystem.
- Stimulus subclasses reuse existing management primitives and MAC receiver entry points; they do not implement protocol decisions. Packet ownership is unchanged except for the single intentionally deleted ACK. No borrowed packets are retained. Inherited initialization stage counts cover the derived fixtures.
- Header/body access is typed; opaque-element scanning checks lengths before indexing. No ACK timing formula or PHY mode computation is duplicated. Deadlines are test observation windows.
- T4 WLAN source-path checklist is N/A for production-path changes because no WLAN source file changed. Relevant ownership, representation, timing, and observability concerns were checked for fixtures. General test-category and evidence requirements are reflected in the explicit distinction between behavior, presence probes, and tooling gaps.
- Checks: `git diff --check` passed; `doc/project/enforcement/check-seals.sh` passed (19 documents, index in step); `git apply --reverse --check` confirms the approved 18-marker patch is applied.
- Full release builds and pre-push gates are not claimed: this task prepares local test/evidence changes, not a push. `git diff --check` and the project seal-index check are recorded separately.
- The user accepted the exact 18-marker removal patch and reasons before application under [TR-BASELINE-DELIBERATE](../../../rule/testing.md#tr-baseline-deliberate). Production follow-ups remain the bounded tasks in the table; making every test green is not an acceptance criterion.
