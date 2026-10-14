# R1: management publishes legacy rate state

> **Kind:** report · **Status:** snapshot 2026-10-14 · **Seal:** none · **Owns:** — · **Stands on:** [testing rules](../../../rule/testing.md), [R1 plan](../../../../../plan/pending/ieee80211-compressed-blockack-replacement.md#r1-publish-production-rate-state)

R1 populates the existing MIB records through full and simplified management. The new production fixture passes. All 23 existing controls pass on both R1 and its parent. The same new fixture fails on the parent because local operational rates remain unknown. The comparison establishes the missing publication path and the result of its repair.

## Source and scope

The verified source baseline and comparison base are `1cdb362a2a4b97f564566487079d9d156a5d213d`. The implementation follows R1 in plan revision 25. The maintainer's instruction, "Execute R1 from the Compressed Block Ack plan", authorizes this implementation. The verification changes no recorded expectation. A separate instruction authorizes the commits, push, and upstream PR.

The R1 checkout is `/home/user/omnetpp_ws/inet-ieee80211-compressed-blockack-r1`. The parent checkout is `/tmp/inet-r1-parent`, with detached HEAD at the pinned baseline. The parent contains an exact copy of the new fixture for the comparison. Its production source remains unchanged. The planning commit `63fecafefbe72716fda761ca7793d476b754dffb` retains the four linked plans and preserves the baseline's production and test source. The local `AGENTS.md` edit remains outside the commits.

The source changes reside in [management](../../../../../src/inet/linklayer/ieee80211/mgmt/) and the [MIB](../../../../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.h). The MIB remains the owner of committed rate records. The implementation adds one temporary peer record to the AP's existing pending association state. That record preserves accepted request rates until the response exchange completes. No new rate store, parameter, signal, or negotiation mechanism exists. The cleanup verification below covers the added MIB operations.

## Behavior and implementation contract

Before the first source edit, the implementation contract identified management as the writer of local, BSS, and peer rate state. Source inspection confirmed the existing setters, management completion paths, stop paths, and absence of an S3b subscriber. The proposed fixture used those production paths without direct MIB rate injection. The source protection check found no sealed target.

The management base publishes local legacy rates after a mode-set update. AP management publishes BSS rates from the same elements that it advertises. Full AP management retains accepted request rates until the final successful response receives an ACK. It clears the pending transaction before the rate notification. Retry failure, a stale completion, and an intermediate fragment ACK cannot replace committed peer rates.

Full STA management commits BSS and AP rates from the successful response. It uses the MIB's combined setter to install both records before one notification. Valid associated-AP Beacons refresh those records. Unrelated advertisements, associated-AP Probe Responses, and refused attempts for another AP preserve active rate state. Invalid rate values cannot replace committed records.

Simplified management installs equivalent records during its existing static association operation. It uses the AP's advertised BSS rates and each endpoint's local operational rates. AP peer records keep basic rates unknown because request frames do not define BSS basic rates. A Beacon with no basic flags supplies a known empty basic set. That state differs from absent rate information.

Peer departure removes the AP's peer record and preserves its BSS record. STA association loss clears the STA's old BSS and AP records. Existing stop paths clear the corresponding BSS and peer records. The local catalog record remains available. R1 adds no recovery guarantee after stop and start.

For example, the fixture uses `g(mixed)`, which supplies twelve legacy rates. Real management emits eight rates in Supported Rates and four in Extended Supported Rates. After the AP/STA exchange, both endpoints expose all twelve operational rates. A controlled associated-AP Beacon changes the rates to 6, 24, and 54 Mbps with no basic flags. The STA publishes both changed records before one notification. A second identical Beacon emits no notification and causes no rate-control reset or additional rate statistic.

The [developer guide](../../../../src/developers-guide/ch-80211.rst#the-mib) explains these records and their lifetime. `WHATSNEW` describes the effect for external models that query the MIB after notifications.

## Standard evidence

The following normative sources define the rate-element meaning and the existing AP completion boundary. The local corpus supplied the text. PDF inspection was unnecessary.

| Canonical node | Physical PDF pages | Use |
| --- | --- | --- |
| `ieee80211-2024:clause:9.4.2.3` and `ieee80211-2024:figure:9-210` | 936 | Operational rates, basic flags in BSS advertisements, and ignored basic bits in request frames. |
| `ieee80211-2024:clause:9.4.2.11` | 951 | Additional operational rates in Extended Supported Rates. |
| `ieee80211-2024:clause:11.3.5.3` | 2528–2531 | The AP's existing successful response-exchange boundary. |

The corpus resolves the reference from Clause 9.4.2.3 to Figure 9-210. Global corpus lint reports unrelated extraction ambiguities. The selected nodes resolve to explicit canonical identifiers. The zero-count representation for abstract management inputs is a model choice; the wire serializer still requires a nonempty Supported Rates element.

## Verification

Both checkouts use OMNeT++ 6.4.0aipre2, Cmdenv, and debug INET libraries. The following command builds the library from each checkout root:

```sh
make MODE=debug -j12
```

The final build exits with status 0 in both checkouts. The parent reuses objects only for identical source and generated inputs. Its build recompiles changed source and dependent objects before the parent tests. `parent-artifact-reuse.json` records 4,906 identical tracked source files and the eight source paths that require replacement objects.

The following commands run from each checkout root. The parent shell first executes `source setenv -q` to select its own tools and library.

```sh
inet_run_module_tests -m debug -f 'Ieee80211MgmtRatePublication_1.test' --no-concurrent

inet_run_module_tests -m debug -f 'Ieee80211(Mgmt(Ap|Sta)(BeaconUpdate|Discovery|Lifecycle|Deauthentication|Disassociation|SimplifiedInitialization|ReassociationSnapshot|Timeout|QueueDrop|HcfRtsTimeout|HcfQueueDrop|MalformedHtCap|UnavailableChannel|ChannelChange|GenericRadio)|VhtHtSupport)_1.test' --no-concurrent

inet_run_unit_tests -m debug -f 'Ieee80211(ResponseModeSelection|NonHtReferenceRate|SupportedRates|BlockAckActionWire)_1.test' --no-concurrent

inet_run_protocol_tests -m debug -w wifi -f '(N_BlockAck|Ac_BlockAck)\.test$' --no-concurrent
```

| Selection | Parent | R1 | Runner exit status, parent / R1 |
| --- | --- | --- | --- |
| New publication fixture | 1 FAIL: local operational rates remain unknown. | 1 PASS. | 1 / 0 |
| Existing management and support-disabled controls | 17 PASS. | 17 PASS. | 0 / 0 |
| Rate selection and wire controls | 4 PASS. | 4 PASS. | 0 / 0 |
| Basic Block Ack protocol controls in `n` and `ac` | 2 PASS. | 2 PASS. | 0 / 0 |

The new [fixture](../../../../../tests/module/Ieee80211MgmtRatePublication_1.test) uses `General`, run 0, seed set 0, and a 620 ms limit. It first drives a real agent, management, MAC, and radio exchange. Later controlled inputs call production management handlers and the existing AP completion callback. Its assertions compare published rates and basic flags with the actual management elements. Its listener checks that paired STA records are complete when the MIB emits the notification. A built-in Aarf rate-control query confirms retained state and no additional data-rate notification after rate-only refresh.

The existing cases retain their checked-in parameter matrices. Each checkout attempts 41 simulation runs across 24 selected test files. The RTS timeout fixture executes twelve runs with seed sets 0, 1, and 2. The VHT/HT fixture executes seven scenarios with seed set 0 and Block Ack disabled. All other selected cases use seed set 0. `runs.json` records each configuration, run, scenario, seed set, and stdout path. The parent publication run stops at initialization; the other parent runs complete.

The parent failure is the intentional regression comparison. It does not alter the fixture's PASS expectation. The assertion fails at `staMib->getLocalRateSet().operational.known`. This direct failure identifies the missing publication path; runner classification alone does not establish the cause.

The following focused gates run from the R1 checkout:

```sh
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mgmt
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mgmt
doc/project/enforcement/check-source-seals.sh --diff
git diff --check
```

Architecture, source protection, and whitespace checks pass with exit status 0. The naming check exits with status 1 and reports 26 candidates in unchanged MSG files. The parent check produces identical output. R1 adds no naming candidate and changes no NED or MSG declaration.

Raw artifacts reside in the Git-ignored directory `audit/ieee80211-compressed-blockack-r1/`. It contains final build logs, runner logs, gate logs, the parent reuse record, and the run manifest. The `feature/` and `parent/` directories preserve generated stdout, stderr, and INI files for each selected case. `source-digests.json` records SHA-256 values for the eight changed source files and the new fixture. The artifact names identify the command group and comparison side.

## Publication checks

The publication pass builds fresh debug and release INET libraries with `make MODE=debug -j12` and `make MODE=release -j12`. Both commands exit with status 0. It repeats the four focused debug commands above. All 24 selected test files pass across the same 41 runs.

Two Basic Block Ack fingerprint cases also match their stored expectations. The command runs from `tests/fingerprint` in the R1 checkout:

```sh
./fingerprinttest -d -m 'HCFfragblockack|MacQosWithBlockAck' -f tplx -f '~tNl' -f '~tND'
```

`HCFfragblockack` uses run 0 and a 1 s limit. `MacQosWithBlockAck` uses run 0 and a 10 s limit. Both cases preserve the `tplx`, `~tNl`, and `~tND` fingerprints. The runner exits with status 0. No fingerprint baseline changes.

The publication pass adds the following repository-wide gates from the checkout root:

```sh
doc/project/enforcement/check-architecture.sh
doc/project/enforcement/check-naming.sh --base upstream/master
doc/project/enforcement/check-interfaces.sh
doc/project/enforcement/check-seals.sh
doc/project/enforcement/check-source-seals.sh --base upstream/master
doc/project/enforcement/check-pull-request.sh --base upstream/master --description-file /tmp/inet-r1-pr.md
```

The full architecture, naming, and interface gates return status 1 for inherited findings. The pinned parent produces the same findings; comparison ignores output order. The interface gate identifies 15 classes with existing contract-purity violations. R1 changes none of those classes. Document seals, source protection, and the PR series checks pass. The focused management architecture check and whitespace check also pass.

The `publication/` artifact directory contains the build, test, fingerprint, and gate logs from this pass. The PR description includes three diagrams for association publication, rate-only Beacon refresh, and record cleanup. The planning commit changes no production behavior. The R1 commit supplies the publication behavior and its direct evidence.

## Cleanup notification verification

Association loss now clears the STA's association snapshot, Beacon timer, peer HT state, peer rates, and BSS rates before the cleanup notification. A subscribed callback sees one complete transition. Local state remains available. A single-peer cleanup also preserves unrelated peers.

The comparison source is published R1 commit `697d2934bbeefd20d97f99ce80f080dbf2e3f8b5`. Its separate HT, peer-rate, and BSS-rate removal calls emit synchronous notifications. The first callback can see an unassociated STA with old rate records. The expanded production fixture fails at 0.5 s, event 394, because BSS rates remain known inside that callback. This run uses `General`, run 0, and seed set 0.

Management first resets its association snapshot and cancels the Beacon timer. The MIB operation `clearBssAndPeerState()` clears the selected peer's HT and rate state and BSS rates before one notification. Stop uses `clearBssAndAllPeerStates()` to clear all peer HT and rate state and BSS rates before one notification. Empty cleanup emits no notification. Neither operation changes local rates, local HT capabilities, or local HT operation. Each operation completes all mutations before notification, so a callback can install replacement records without later deletion by that operation.

Successful association replacement uses the same full STA cleanup path. Simplified STA stop completes local cleanup before AP peer cleanup. The AP removes its station entry before its existing `releaseAssociationId()` operation removes peer HT and rate state together. AP BSS rates remain available when one STA departs. Standalone MIB removal methods retain their independent notifications.

The expanded [production fixture](../../../../../tests/module/Ieee80211MgmtRatePublication_1.test) checks state inside subscribed callbacks for disassociation, association replacement, full STA stop, and simplified STA stop. Full management uses `g(mixed)` and twelve legacy rates. Simplified management uses `n(mixed-2.4Ghz)` to supply real peer HT state. STA stop starts at 600 ms; AP stop starts at 620 ms after the asynchronous STA operations complete. The fixture uses a 640 ms limit. It injects no MIB rate records directly.

The new [unit fixture](../../../../../tests/unit/Ieee80211MibBssTeardown_1.test) checks both MIB operations across all combinations of BSS, peer HT, and peer-rate state. It checks notification counts, empty cleanup, local-state retention, unrelated peers, and callback-installed replacement records. The existing peer teardown fixture checks the standalone removal contracts. Both unit fixtures use seed set 0.

The correction builds a fresh debug library with `make MODE=debug -j12`. The build exits with status 0. The following focused commands run from the checkout root and exit with status 0:

```sh
inet_run_module_tests -m debug -f 'Ieee80211MgmtRatePublication_1.test' --no-concurrent
inet_run_unit_tests -m debug -f 'Ieee80211Mib(BssTeardown|PeerTeardown)_1.test' --no-concurrent
inet_run_module_tests -m debug -f 'Ieee80211(MgmtSta(Lifecycle|Disassociation|Deauthentication|SimplifiedInitialization|BeaconUpdate)|MgmtApLifecycle|VhtHtSupport)_1.test' --no-concurrent
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
git diff --check
```

All ten selected test files pass. The VHT/HT fixture retains its seven scenarios. The other cases use one run each. All runs use seed set 0. The scoped architecture and whitespace checks pass. Raw reproduction, build, and test logs reside under `audit/ieee80211-compressed-blockack-r1/teardown/`.

These checks establish complete cleanup at the tested callback boundaries. They do not establish arbitrary management actions from callbacks or lifecycle-subscriber integration. The combined cleanup operations preserve callback-installed MIB records; the fixture does not test a new association initiated from a callback.

The final correction pass also builds the release library with `make MODE=release -j12`; the command exits with status 0. It repeats the publication selections with the expanded module fixture and both MIB teardown unit fixtures. All 18 module, six unit, and two protocol files pass across 43 runs. Both Basic Block Ack fingerprint cases match all three selected ingredients. No stored expectation changes.

The final module filter combines the two earlier module selections into one command:

```sh
inet_run_module_tests -m debug -f 'Ieee80211(MgmtRatePublication|Mgmt(Ap|Sta)(BeaconUpdate|Discovery|Lifecycle|Deauthentication|Disassociation|SimplifiedInitialization|ReassociationSnapshot|Timeout|QueueDrop|HcfRtsTimeout|HcfQueueDrop|MalformedHtCap|UnavailableChannel|ChannelChange|GenericRadio)|VhtHtSupport)_1.test' --no-concurrent
inet_run_unit_tests -m debug -f 'Ieee80211(ResponseModeSelection|NonHtReferenceRate|SupportedRates|BlockAckActionWire|MibBssTeardown|MibPeerTeardown)_1.test' --no-concurrent
```

Protocol and fingerprint commands retain the publication selections above. The final logs use the names `modules-final.log`, `units-final.log`, `protocol-final.log`, and `fingerprint-final.log` under `teardown/`. Both build logs also reside there. The final source snapshot appears in `teardown/source-digests.json`.

The final repository-wide architecture, naming, and interface checks report the same findings as the pinned parent. The comparison ignores output order. Source protection and document seals pass. The PR description contains four diagrams, including the cleanup notification sequence. The correction belongs to the R1 implementation commit; the planning commit remains unchanged.

## Association and HT notification verification

Management now commits peer HT state and rate records before one MIB notification. AP management clears the pending response transaction first. STA management completes its association snapshot and schedules its Beacon timer first. Valid associated-AP Beacons refresh the timer before publication. Simplified management uses the same commit operation at each endpoint. Each notification describes one MIB's completed transition; the two endpoints retain separate commit boundaries.

The comparison source is published commit `0376d9cfd19b9ee49049c0e6adb0d9872e825dde`. Separate HT and rate calls let a callback observe new HT state without the accepted peer rates. The AP also retains its pending transaction during the first callback. The initial full-management fixture fails on that library at 0.022234059166 s, event 160, in the STA callback. A separate run checks the AP callback and fails at 0.022278069027 s, event 170. Both callbacks fail the assertion that the peer rate record exists and contains known supported and operational rates.

The MIB operation `installBssAndPeerState()` validates both rate records before mutation. It commits the selected peer's HT state, BSS rates, and peer rates before notification. A null capabilities argument removes HT state in the same transition. Unchanged accepted state emits no notification. Private update helpers share the existing state logic with standalone setters.

Standalone setters retain their notification behavior. The operation adds no signal, parameter, event, or rate store.

The final [module fixture](../../../../../tests/module/Ieee80211MgmtStateCommit_1.test) drives a real HT association with `n(mixed-2.4Ghz)`. It observes both MIBs during callbacks. Controlled requests pass through the AP handler and final-ACK completion path. Controlled Beacons pass through the STA handler. These inputs change HT capabilities and legacy rates together, repeat identical state, and remove HT state. The fixture also checks each simplified endpoint's callback after the production stop and static association operations.

The module fixture uses `General`, runs 0 and 1, seed set 0, and a 501 ms limit. Run 0 checks both full-management endpoints. Run 1 checks AP callbacks independently. The fixture checks the pending AP transaction and scheduled STA Beacon timer. It writes no MIB record directly. Controlled frame copies preserve chunk immutability after packet insertion.

The new [unit fixture](../../../../../tests/unit/Ieee80211MibStateCommit_1.test) checks combined changes, HT-only changes, rate-only changes, HT removal, duplicate publication, and unknown peer rates. It checks rejection before mutation for invalid rates and unsupported local HT state. It also checks local-rate retention and replacement records installed by a callback. An unknown peer record remains a non-null entry whose sets have `known=false`; absence and unknown information remain distinct.

The direct commands run from the checkout root after a fresh debug build:

```sh
make MODE=debug -j12
inet_run_module_tests -m debug -f 'Ieee80211MgmtStateCommit_1.test' --no-concurrent
inet_run_unit_tests -m debug -f 'Ieee80211MibStateCommit_1.test' --no-concurrent
```

The AP-only reproduction runs the generated test executable from `tests/module/work/Ieee80211MgmtStateCommit_1`:

```sh
./Ieee80211MgmtStateCommit_1_dbg -u Cmdenv -r 1
```

Raw artifacts reside under `audit/ieee80211-compressed-blockack-r1/association/`. The files `before-sta.out`, `before-sta.err`, `before-ap.out`, and `before-ap.err` contain direct callback failure evidence. The final fixture adds simplified association checks to the initial full-management reproduction. The source snapshot appears in `source-digests.json`.

Both final debug and release builds exit with status 0. The new module fixture passes both runs; the new unit fixture passes. The final pass also repeats the previous 18 module, six unit, and two protocol files. All 28 selected files pass across 46 simulation runs. Both Basic Block Ack fingerprint cases match the three selected ingredients. No stored expectation changes.

The final unit selection adds the new state commit fixture:

```sh
inet_run_unit_tests -m debug -f 'Ieee80211(ResponseModeSelection|NonHtReferenceRate|SupportedRates|BlockAckActionWire|MibBssTeardown|MibPeerTeardown|MibStateCommit)_1.test' --no-concurrent
make MODE=release -j12
```

The earlier final module, protocol, and fingerprint commands retain their selections. The new module fixture runs through its direct command above. The final logs are `after.log`, `modules-final.log`, `units-final.log`, `protocol-final.log`, and `fingerprint-final.log`. The build logs are `build-debug.log` and `build-release-final.log`.

The scoped IEEE 802.11 architecture check passes. The full architecture, naming, and interface checks report the same findings as the pinned parent. Source protection, document seals, and whitespace checks pass. The correction retains the planning commit and belongs to the R1 source commit. The PR's four diagrams now show combined HT and rate publication and completed cleanup.

## AP cleanup and membership-selector verification

Full AP departure now completes the station downgrade and required pending-transaction cleanup before the MIB removes peer records and emits. Disassociation, Deauthentication, authentication reset, and acknowledged refusal follow that order. Full AP stop clears management bookkeeping before `clearAccessPointState()` clears the MIB's station/AID tables, peer state, and BSS rates. This operation emits once if HT or rate state changes. Local rate records remain available.

The comparison source is published commit `df320692fb69a6ba4175b8189d606ff6fb423bac`. Diagnostic copies of the generated production fixtures establish three failures at 0.5 seconds, event 394. Departure exposes `ASSOCIATED` after peer rates disappear. AP stop exposes known BSS rates after all peer records disappear. A wire-decoded HT membership selector enters the operational legacy set as 63.5 Mbps. Raw failure evidence resides under `audit/pull-request/pr1329/verification/`.

The expanded publication fixture checks the full AP's station status, cleared pending transaction, absent peer records, and preserved BSS rates inside departure callbacks. It checks Disassociation, Deauthentication, and authentication reset with a pending response present. Its AP stop callback checks empty station/AID tables and unknown BSS rates. The HT fixture adds the acknowledged-refusal callback check. These fixtures write no MIB records directly.

The HT fixture also serializes and decodes each of the six selectors in IEEE Std 802.11-2024, Table 9-131. It puts each selector in both the primary and extended rate element. The production Beacon handler retains the actual 6 Mbps rate and emits no extra notification for unchanged accepted information. The assertions check supported, operational, and basic legacy sets. Selector-based join checks remain unmodeled; this correction supplies accurate legacy rate records without a new negotiation mechanism.

The unit teardown fixture adds AP cleanup checks for station/AID tables, pending AID reservations, peer records, BSS rates, local-state retention, and repeated empty cleanup. The standalone association-ID operation continues to preserve BSS rates.

The series retains planning commit `63fecafefbe72716fda761ca7793d476b754dffb`. A separate preparation commit adds the combined MIB operations and their unit fixtures. Management activation follows it. Both commits use the same fixed production base; the split absorbs no upstream change. The unrelated local `AGENTS.md` edit remains outside the series.

Raw correction and per-commit verification logs reside under `audit/ieee80211-compressed-blockack-r1/corrections/`. The production fixtures retain their configuration matrices, run numbers, seed set 0, and time limits. No recorded expectation changes.

Preparation commit `404b919808dc040afa2a1624f9d42ee91852e689` passes fresh debug and release builds. Its three selected MIB unit fixtures pass after the debug build completes. The effective production source and selected unit definitions match that candidate exactly. The empty comparison appears in `preparation-final-source.diff`. The planning commit remains unchanged.

The final activation source also passes fresh debug and release builds. All 19 module, seven unit, and two protocol files pass across 46 runs. Both Basic Block Ack fingerprint controls retain all three selected ingredients. The three audit diagnostics now exit with status 0 against the corrected library. Their added assertions check the specific departure, AP stop, and selector conditions; they do not alter production source.

The final commands run from the checkout root after the matching build:

```sh
make MODE=debug -j12
make MODE=release -j12
inet_run_module_tests -m debug -f 'Ieee80211(MgmtRatePublication|MgmtStateCommit|Mgmt(Ap|Sta)(BeaconUpdate|Discovery|Lifecycle|Deauthentication|Disassociation|SimplifiedInitialization|ReassociationSnapshot|Timeout|QueueDrop|HcfRtsTimeout|HcfQueueDrop|MalformedHtCap|UnavailableChannel|ChannelChange|GenericRadio)|VhtHtSupport)_1.test' --no-concurrent
inet_run_unit_tests -m debug -f 'Ieee80211(ResponseModeSelection|NonHtReferenceRate|SupportedRates|BlockAckActionWire|MibBssTeardown|MibPeerTeardown|MibStateCommit)_1.test' --no-concurrent
inet_run_protocol_tests -m debug -w wifi -f '(N_BlockAck|Ac_BlockAck)\.test$' --no-concurrent
```

The preparation unit selector is `Ieee80211Mib(BssTeardown|PeerTeardown|StateCommit)_1.test`. Preparation logs use the `preparation-` prefix. Final build logs use the `final-build-` prefix. The final runner logs are `modules-final.log`, `units-final.log`, `protocol-final.log`, and `fingerprint-final.log`. The fingerprint command retains the earlier selection from `tests/fingerprint`.

The corrected diagnostic logs use the `repro-corrected-` prefix under `audit/pull-request/pr1329/verification/`. The departure diagnostic stops its temporary AP subscription after the checked departure. The stop diagnostic ends after the checked production stop operation. These controls keep their assertions limited to the transition under review.

The scoped architecture check, source protection, document seals, and whitespace checks pass. Full architecture, naming, and interface findings match the pinned parent. The naming comparison omits the different base-ref label and ignores output order. The correction adds no exception or recorded expectation. The final source and fixture hashes appear in `corrections/source-digests.json`.

## Limits and next integration check

The self-audit checks ownership, packet lifetime, callback boundaries, terminal paths, and source protection against the implementation contract. The retained response chunk remains valid after packet deletion. The source adds no event, frame format, or rate-selector change. The controls establish their existing assertions; they do not establish every simulation trajectory or arbitrary external listener behavior.

The pinned parent contains no S3b lifecycle subscriber. C2 must repeat the subscriber interaction after S3b and R1 both exist. That check must preserve unchanged Basic and Compressed agreements after rate-only publication. R1 supplies no Compressed exchange, new MCS publication, default activation, or lifetime change. The release build establishes compilation; behavioral verification uses debug mode.
