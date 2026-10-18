# Implicit Block Ack execution record

Execution date: 2026-10-18. A1 through A7 are complete on the approved base, within the profile and verification limits below. The implementation evidence describes the source tree before commit. The user subsequently authorizes commit, push, and PR creation. A0 remains open because prerequisite merge confirmation is absent.

## A2–A7 authorization and scope

The user's request `Execute stages A2-A7` authorizes the remaining stages on this worktree after A1. This request extends implementation beyond the earlier A1-only exception. The base remains `56b75c959af104b5db931bef022bdfe5578eead9`, with the recorded A1 changes. The prerequisite merge status remains unverified. This execution preserves the separate `AGENTS.md` change. Commits, publication, and baseline changes require their own authority.

The root agent owns production source changes. Separate read-only investigations trace transmit contracts and mandatory receive release. A test specialist owns the new A2 boundary fixture. The existing plan defines each stage's behavior and acceptance conditions. Stage completion requires the corresponding direct evidence.

### A2 implementation contract

Rx must reject a delivered PHY failure container before a standalone MAC header or trailer peek. The current guard checks only the decoded Aggregation property. A failed common header supplies no accepted Aggregation property. Rx will also check common header success and complete content from the typed PHY indication. This guard requires no range draw, shared chunk change, or new event.

The existing real-radio fixture supplies copied long-GI and short-GI outcomes, all four decision parts, actual failure paths, and receive metadata. The new boundary fixture will prove refusal before the FCS predicate, including short malformed containers. Ordinary standalone reception retains its current path. A4 will add the separate aggregate entry and member validation. Source inspection and the seal registry validate this bounded contract before the edit.

### A3 implementation contract

The data service will prepare independent queued previews beside borrowed canonical members. The new HT policy will select one peer, transmitter, and TID within supplied window, byte, mode, and spacing limits. The existing Basic policy will retain its disabled behavior. HCF will reject aggregate opt-in with a rate-control submodule before DATA attempts. The new policy requires HT mixed format, 20 MHz, and long GI.

Preparation will assign consecutive provisional numbers without a counter update. Batch validation will check every queued source before the first extraction. Commitment will reuse the existing extraction, region-tag, lifetime, and allocation owners for each new member. Canonical members retain their numbers and deadlines. The in-progress queue will expose eligible canonical members without extraction. HCF will supply retries before numbered unsent members, followed by queued candidates.

Batch refusal will retain sources and sequence state. The preparation fixture will check this boundary through production queues and commitment. It will also check sequence wrap, independent deadlines, count and byte limits, and numbered unsent members. Existing collision recovery remains a single-frame path. A5 will connect this preparation to actual aggregate submission. The inspected owners, interface implementations, and unsealed paths validate this contract before source edits.

### A5 implementation contract

HCF will retain one prepared batch beside the canonical members in the in-progress queue. The frame sequence will use a distinct aggregate transmit step and an implicit Block Ack receive step. This distinction prevents an envelope from entry into ordinary MAC-header paths. Each batch will retain one actual DATA mode through size reduction, RTS prediction, admission, and submission. HCF will supply transmitted retries first, followed by numbered unsent members, each in cyclic sequence order.

Tx will use its existing IFS timer for aggregate requests. At expiry, HCF will validate current agreement identity, member eligibility, deadlines, mode compatibility, and complete exchange cost. HCF will commit queued previews only after those checks. Tx will finalize each member before envelope construction. Actual submission will record every member's allocation index with the agreement owner.

Cancellation will clear the Tx request before HCF deletes its exchange context. Refusal will produce no DATA attempt, DATA outcome, probe, or queue extraction. Zero IFS will use the same submission callback. Ordinary Tx callers will retain their current contract. No new timer or simulation event is necessary.

The shared response selector will accept an actual eliciting mode instead of a fabricated BAR. Explicit and implicit Compressed responses will use the same legal non-HT rate rule. The recipient procedure will construct the bitmap directly from current receipt state. The implicit response will follow SIFS and use the aggregate's common Duration value. HCF will retain the existing receive-start timer and reception-completion mechanism.

The acknowledgment owner will attach all submitted members to the implicit wait. A valid matched bitmap will complete one successful response exchange. Positive bits will acknowledge only transmitted members. Missing bits will retain retry eligibility and independent deadlines. A missing or invalid response will produce one contention failure and arm one pending probe on the current agreement.

The BAR policy will select pending probes through existing EDCA classification. Submission will consume the probe flag. Refusal and member expiry will retain the flag until transfer completion or agreement retirement. Probe outcomes will produce no second DATA attempt or member outcome. Aggregate RTS failure will update contention once without member retry increments or disposal.

The proposed exchange fixture uses production association, queues, radios, and both DATA hops through an AP. It will assert member FCS, shared wire fields, actual aggregate modes, matched responses, and upper-layer delivery. Its bounded matrix varies PHY representation, NIST/Yans, protection, and TXOP limit at seed zero. Separate focused cases must prove cancellation, partial receipt, failed responses, probe transitions, and recovery under continuous traffic. A build or healthy exchange cannot establish those failure paths.

Current source inspection identifies each owner and its terminal paths. The WLAN sources remain outside the packet subsystem seal. The new exchange fixture exists but remains unrun. These facts validate the contract before the A5 production edit. A4 retains sole production write ownership until its stable handoff.

### Required A5 scope correction

The infrastructure fixture initially showed source aggregates but individual AP frames. DS forwarding copies the incoming transmitter address into its outgoing header. Tx replaces that address only at submission. The HT policy therefore refused the AP's queued candidates before that replacement.

An initial correction assigned the local transmitter address in DS. Fingerprint comparison exposed an ordinary A-MSDU effect: its builder derives Address3 from that transmitter field. The correction therefore changed ordinary wire bytes. The final implementation removes the DS edit.

Aggregate preparation now assigns the local transmitter address only to its independent queued preview. HCF supplies that address through the existing batch limits. Tx assigns the same address before FCS calculation.

The queued source retains its identity, owner, header, sequence state, and deadline until commitment. Retained canonical members still face the existing transmitter check. This correction does not normalize a pre-existing numbered, unsent AP member with another transmitter address. Such a member can use standalone service.

The preparation fixture adds a forwarded-source case. The infrastructure exchange checks aggregate transmission across both hops. Fresh fingerprint checks verify ordinary behavior after the DS edit disappears.

### Required cyclic arithmetic correction

The physical reorder fixture reaches sequence wrap with a two-slot window. The shared cyclic helper rejects an intermediate arithmetic value of minus one. Its operators already reject an unspecified source before that calculation. Thus, valid arithmetic such as `0 - 1` cannot return sequence 4095. Window advancement can fail through the same helper.

The unsealed target is `mac/common/Ieee80211Defs.h`. The correction removes only the internal rejection of a valid arithmetic intermediate. Constructors and operators retain their checks for an unspecified sequence object. A new unit fixture will check every sequence against bounded positive and negative offsets. The real-radio wrap cases will verify receipt and reorder advancement. These facts validate the expanded contract before the edit.

## A2–A7 result and verification scope

HCF now transmits independent HT MPDUs in one PPDU after acceptance of a Compressed Block Ack agreement. Each member retains its sequence number, FCS, allocation index, lifetime, and retry state. The recipient validates correct members, updates receipt state, applies mandatory release, and returns one Compressed response after SIFS. The originator retries only unacknowledged members.

For example, an aggregate can contain three members with sequence numbers 10, 11, and 12. If member 11 fails, the recipient records 10 and 12. The originator retains 11 with its original deadline. A later aggregate can combine that retry with new members. This example describes the implemented mechanism; the physical loss fixtures supply direct checks.

The transmit policy supports HT mixed format, 20 MHz, one retained long-GI DATA mode, and fixed rates. The receiver also accepts advertised HT20 short-GI aggregates. Basic and disabled MPDU policies retain their historical behavior. Adaptive aggregate rates, HT40 transmit aggregation, VHT aggregates, Multi-TID Block Ack, and correlated FEC errors remain outside this profile.

The originator marks one probe requirement after an unsuccessful implicit response. BAR selection checks that requirement before further DATA, subject to access-category ownership and legal admission. Actual BAR submission consumes the requirement. Probe failure returns to DATA retry without a second DATA outcome. A BAR received without an agreement uses the existing DELBA path. Continuous traffic can therefore retire a stale agreement without queue drain.

### Required admission correction

The example exposed a reservation endpoint beyond the positive TXOP deadline. At event 2845, the remaining reservation was 2.831966644 ms. The remaining TXOP was 2.831933288 ms. Fractional propagation delay and Duration encoding produced the 33.356 ns difference. The new preparation path bypassed the legacy admission guard against that condition.

HCF now refuses the next prepared exchange when the reservation endpoint exceeds the deadline. The explicit initial singleton-overrun exception remains valid. Refusal occurs before queue commitment or a DATA attempt. The admission fixture adds a fractional-propagation case and an equal-endpoint control. The example retains its physical propagation delay.

### Interface contracts

The response-rate method and actual-submission callback are pure virtual interface declarations. Every in-tree implementation supplies a concrete override. DCF requires no submission transition, so its concrete callback does nothing. Custom rate selectors must implement implicit response selection or throw an explicit capability diagnostic. The migration guide lists these obligations.

### Final campaign

The final campaign uses the unchanged base and uncommitted source described above. Its artifacts reside in `out/verification/ampdu-a2-a7/`. This ignored directory retains logs across tool-session replacement. Earlier `/tmp/ampdu-*` logs became unavailable after such a replacement. Earlier results remain historical observations; the final campaign repeats the affected checks against the fresh library.

Each runner first executes `source ./setenv -q`. Each runner uses `MAKEFLAGS=--jobserver-style=pipe MPLCONFIGDIR=/tmp/ampdu-matplotlib`. Debug compilation precedes all behavioral checks. The unit, module, and protocol commands below use serial fixture execution. Independent categories use separate generated directories.

```sh
make MODE=debug -j8
make MODE=release -j8

inet_run_unit_tests -m debug -f 'Ieee80211(Ampdu.*|SequenceNumberCyclic|BlockAck.*|CompressedBlockAck.*|ResponseTimeout|TxNavChoice|TxopProcedure|HtCapabilities|HtMgmtElements|HtModeSet|HtModeCache)_1\.test$' --no-concurrent

inet_run_module_tests -m debug -f 'Ieee80211(Ampdu(Admission|Capabilities|Exchange|Preparation|Recovery|ResponseWindow|RxBoundary|TxStart|PhyReception)|CompressedBlockAck.*|BlockAck.*|Edca.*Lifetime.*|HtAssociation|MgmtStaSimplifiedInitialization|VhtHtSupport|HcfMultipleProtection|HcfSingleProtection|SingleProtectionGroup|Txnav)_1\.test$' --no-concurrent

inet_run_module_tests -m debug -f 'Ieee80211AmpduReordering_1\.test$' --no-concurrent

inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/(N_AmpduImplicitBa|N_Ampdu|N_BlockAck|Ac_BlockAck|Ac_BlockAckFallback)\.test$' --no-concurrent
```

The unit selection checks response validation, sequence wrap, batch construction, probe selection, and ordinary timing controls. The module selection checks production negotiation, physical reception, actual submission, admission, retry, stale-agreement recovery, and historical compatibility. The separate reorder campaign preserves its original 160 physical cases and adds 16 negative RIFS cases. Seed zero controls the feature fixtures except the PHY matrix, which defines seeds zero, one, and two.

The real infrastructure exchange varies scalar and dimensional reception, NIST and Yans, single and multiple protection, and zero or positive TXOP limits. Its 16 main configurations require exactly 60 upper deliveries across both AP hops. Three additional cases force full, partial, or failed probe recovery with a healthy agreement. Fresh DATA remains queued at the probe. The fixture checks later DATA progress, original deadlines, retry order, mixed batches, and one outcome per submitted member.

The admission matrix covers both bands, MCS 0/7, complete-PSDU RTS thresholds, spacing, fixed-mode reduction, and HT duration boundaries. It also checks positive-limit singleton overrun, zero-limit behavior, post-CTS cancellation, and new, retained, or mixed-batch RTS failure. RTS failures preserve member state without a DATA attempt. The final two cases check reservation overshoot and equality with actual propagation delay.

The response-window fixture uses a real outstanding receive timer. A new aggregate overlaps the timeout. An addressed aggregate receives one response after SIFS; an overheard aggregate resolves the previous wait without a local response. Long and short GI cases use actual reception. The response uses non-HT 6 Mbps when BSS basic-rate state is unknown.

The recovery fixture separately checks absent-agreement behavior. Continuous cases retain fresh queued DATA with inactivity values zero and 200 ms. They require BAR, recipient DELBA, agreement retirement, later ADDBA, and further aggregates. Finite cases lose the probes and require lifetime completion. These fixtures distinguish seeded agreements from the infrastructure fixture's production management exchange.

The example provides scalar, dimensional, and zero-TXOP configurations. The user guide explains opt-in and recovery. The migration guide lists each changed extension role. WHATSNEW states the supported profile and its limits. These artifacts establish no throughput or complete-amendment conformance claim.

The user separately approves removal of the historical `N_Ampdu.test` expected-failure marker on 2026-10-18. The final focused campaign proves aggregate observation, implicit response, and exactly 60 application deliveries before that removal. The VHT marker and stored fingerprints remain unchanged. No commit or publication forms part of this execution.

### Corrected evidence boundaries

The first protocol output check matched a test name without its verdict. Raw output showed a failed protocol assertion despite the runner's PASS. Shared INI defaults also preceded the intended traffic overrides. The corrected fixtures place their overrides first and require complete PASS records.

UDP supplies the supported upper-delivery signal. The sink's actual finish counter independently proves application receipt. The deterministic zero-propagation fixture disables only the diagnostic for simultaneous transmission starts; the medium still models interference.

An all-failed aggregate exposed an ownership warning. Rx retained ownership of the envelope after its method returned. MAC now takes the envelope and returned members before local disposal or HCF handoff. The strict reorder checker requires all 176 successful runs without warnings or errors.

The first ownership build failed inside Clang with invalid-UTF-8 diagnostics for built-in definitions. The compiler failure's cause remains unknown. A retry with `CCACHE_DISABLE=1` compiled the same source successfully. Subsequent final source builds retain that setting. `debug-ownership-build.log` and `debug-ownership-build-retry.log` preserve both results.

Positive RIFS remains unverified because a consistent supported configuration cannot reach it. HCF compares the physical gap plus the preceding signal extension with 2 microseconds. HT at 2.4 GHz adds a 6 microsecond extension. The `ac` mode admits HT at 5 GHz, but HCF excludes RIFS for that VHT mode set. The fixture preserves all reachable SIFS cases and adds 16 negative RIFS cases. It does not introduce a mixed, inconsistent mode configuration.

### Mechanical checks and review

Both scoped WLAN architecture checks pass. The changed-file NED/MSG naming check passes. The source-seal check passes without a sealed source edit. The document-seal index remains consistent. The separate `AGENTS.md` change remains outside this feature work. The final preservation check uses its recorded SHA-256.

The interface checker exits 1 for two unchanged bodies in `IFrameSequence.h`. `AV-CONTRACT-02` records these as open deviations in `doc/project/audit/architecture-exceptions.md`. The new interface methods contain no default bodies. Independent review confirms that each in-tree implementation supplies its required override. These facts do not imply a clean project-wide interface gate.

The complete document-link check reports 163 broken links to absent standards files in unchanged documents. The scoped check uses the canonical checker with an explicit inventory of four changed Markdown documents. It reports zero broken links. The C++ checker cannot run because this build supplies no `compile_commands.json`; it exits 2. Independent source review and debug/release compilation remain separate evidence. No C++ checker pass follows from them.

The final narrow source review resolves the preview address, ownership, interface, and protocol-observation defects. The earlier integrated review covers response windows, singleton overrun, probe counters, and cyclic sequence arithmetic. Neither review supplies runtime evidence by itself. No new production-source finding remains open in those review scopes.

The artifacts use these names:

| Check | Result | Artifact under `out/verification/ampdu-a2-a7/` |
| --- | --- | --- |
| Final debug compilation. | Exit 0. | `debug-preview-build.log` |
| Final release compilation. | Exit 0. | `release-final-build.log` |
| Unit campaign. | 17 fixtures pass. | `units.log` |
| Module campaign. | 45 fixtures pass. | `modules.log` |
| Final preview and infrastructure exchange. | Both fixtures pass; preparation includes 12 cases and two AARF rejection controls. | `preview-integration.log` |
| Physical reorder and history. | 176 runs pass without warnings or errors. | `reordering-final176.log`, `.out`, and `.err` |
| Focused Wi-Fi protocol campaign. | All five fixtures pass before marker removal. | `protocol-final-focused.log` |
| Complete Wi-Fi inventory after setup recovery. | 77 distinct fixtures: 48 passes and 29 expected failures. No unresolved error or unexpected runtime failure remains. | `protocol-combined-results.json`, `protocol-full.log`, `protocol-space-retry.log`, `protocol-matcher-retry.log` |
| Ordinary fingerprints. | All 23 configurations pass; exit 0. | `fingerprints-final.log`, `fingerprints-final-raw.log` |
| Graphical fingerprints. | All 20 match the parent; 18 stored expectations differ on both revisions. The runner exits 1. | `fingerprints-final-tyf.log`, `final-tyf-parent-comparison.json` |
| Three example configurations. | All reach the simulation limit without error. | `example-final-General.log`, `example-final-ZeroTxop.log`, `example-final-Dimensional.log` |
| MAC and PHY architecture. | Both pass. | `architecture-mac.log`, `architecture-phy.log` |
| Changed-file names and source seals. | Both pass. | `naming-diff.log`, `source-seals.log` |
| Document seals and scoped links. | Both pass. | `document-seals.log`, `document-links-scoped.log` |
| Interface checker. | Two known, unchanged deviations; exit 1. | `interfaces.log` |
| Complete document links. | 163 pre-existing missing standards links; exit 1. | `document-links.log` |
| C++ checker. | No compilation database; no analysis; exit 2. | `cpp.log` |

The module campaign spans the final corrections. The final preview and exchange rerun replaces its earlier evidence for the changed preparation path. The strict reorder run replaces the earlier run with an ownership warning. Other affected fixtures use the fresh library after their relevant source correction. The final protocol and fingerprint campaigns start after the last production edit.

### Final regression campaigns

The focused protocol campaign passes all five fixtures. The corrected implicit fixture runs both normal receipt and deterministic partial loss. Each run requires 60 UDP deliveries, 60 sink receipts, and its complete protocol PASS record. The historical HT fixture retains aggregate observation and adds receipt completion. The approved marker removal applies only to that HT fixture. The complete Wi-Fi campaign uses this command:

```sh
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' --no-concurrent
```

The full campaign selects 77 test files. Its log is `protocol-full.log`. Existing expected failures remain explicit. The runner's preview mode previously raised a selection error before any simulation. That preview supplies no executed-test evidence. The actual campaign supplies the executed inventory.

The full invocation reports 40 passes, 22 expected failures, and 15 setup errors after filesystem exhaustion. Its log also shows a matcher write error for `Ac_MultiTidBa`. Thus only 61 fixtures supply complete observations before recovery: 40 passes and 21 expected failures. The invocation exits 1. No unexpected runtime failure precedes that setup failure. The HT marker metadata still reflects its earlier declaration in that invocation; its observed PASS remains valid.

The recovery removes duplicate generated matcher output and compresses raw simulation logs without source changes. Compressed logs remain beside their original paths as `test.out.gz`. This action restores 4.7 GiB of free space. A filtered retry selects the 15 setup failures and the approved HT marker control. It reports nine passes and seven expected failures, with exit 0. Its log is `protocol-space-retry.log`.

A separate `Ac_MultiTidBa` retry resolves the matcher write error. Its log is `protocol-matcher-retry.log`. The original full-invocation errors remain visible in the record. These filtered retries replace incomplete evidence; they do not change source or expectations.

The matcher retry reports its expected failure and exits 1 under the runner's all-failure aggregation rule. The final combined inventory contains all 77 distinct fixtures: 48 passes and 29 expected failures. Every final observation matches its current declared expectation. `protocol-combined-results.json` identifies the authoritative artifact for each fixture. No unresolved setup error or unexpected runtime failure remains. This result combines the full invocation with the bounded retries; it is not a single successful full-suite invocation.

```sh
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/(Ac_LdpcStbcCap|Ac_Ldpc|Ac_4ss|Ac_OpModeNotification|Ac_PhyMode|Ac_MuMimo|Ac_VhtIe|Ac_Ampdu|Ac_256qam|Legacy_RtsCts|Legacy_PhyMode|Legacy_DataAck|Legacy_Pcf|Legacy_Fragmentation|Legacy_Nav|N_Ampdu)\.test$' --no-concurrent
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/Ac_MultiTidBa\.test$' --no-concurrent
```

The ordinary fingerprint campaign passes all 23 selected configurations without an expectation change. It uses the final debug library and these exact commands from the repository root:

```sh
cd tests/fingerprint
PYTHONUNBUFFERED=1 ./fingerprinttest -d -t 4 -m '(showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)|examples/(wireless/qos|adhoc/qos))' -f 'tplx' -f '~tNl' -f '~tND'
```

`fingerprints-final.log` records the 23 passes and exit 0. `fingerprints-final-raw.log` preserves the exact simulator commands and output. The CSV rows define configurations and limits. The scalar attributes record seed sets zero and one. The passing byte fingerprints replace the earlier four differences from the removed DS edit.

Graphical fingerprints use a separate campaign with the same selector and `-f 'tyf'`. The legacy wrapper supplies fake-GUI probability 0.001. Parent controls use that same value. The newer Python API's probability 0.1 would define different inputs. These checks do not establish visual rendering correctness.

The parent control uses revision `56b75c959af104b5db931bef022bdfe5578eead9` and its existing debug library. No parent production source is newer than that library. The control uses the same current NED inputs, commands, configurations, and seeds as the feature. It changes no parent file. `fingerprint-control/README.md` states the library hashes and input identities. The control covers all 20 graphical configurations separately.

The final graphical campaign records two baseline passes and 18 baseline mismatches; its runner exits 1. Every final actual value equals its corresponding parent value. `compare-final-tyf.py` verifies all 20 rows against the isolated parent results. Its output is `final-tyf-parent-comparison.log`; its detailed rows are `final-tyf-parent-comparison.json`. This comparison establishes no new graphical fingerprint change under the recorded inputs. It does not repair the pre-existing baseline differences.

### Stage closure

| Stage | Closure evidence |
| --- | --- |
| A2. | PHY refusal stops before standalone MAC/FCS peeks. The physical matrix retains all 468 runs and copied outcome checks. |
| A3. | Twelve preparation cases and two AARF rejection controls prove refusal, commitment, retained members, limits, and forwarded previews. |
| A4. | All 176 physical reorder/history cases pass without warnings or errors. They cover reachable mandatory release and the recorded negative RIFS boundary. |
| A5. | Admission passes 57 runs. Actual-start and response-window fixtures pass. Infrastructure exchanges verify computed member FCS, implicit responses, selective retry, and probe policy. |
| A6. | Production management establishes both AP hops. Healthy and absent-agreement recovery pass. Historical module controls and the complete protocol inventory retain their declared outcomes. |
| A7. | The example, user guide, migration guide, release note, and execution records exist. Debug/release compilation, ordinary fingerprints, and parent graphical comparisons complete the declared verification. |

The known interface deviations, absent C++ compilation database, missing standards links, and positive-RIFS coverage limit remain explicit above. No publication gate receives an inferred pass. The plan remains under `pending` because A0 is open. The approved execution exception closes A1–A7 on this base. The later publication request authorizes commit, push, and PR creation. Merge approval remains separate.

## A1 scope and authorization

The user authorizes A1 on base `56b75c959af104b5db931bef022bdfe5578eead9` before the prerequisite PRs merge. This exception applies to A1 only. The active branch is `work/ieee80211-ampdu-implicit-blockack-a1-a7`. The [feature plan](ieee80211-ampdu-implicit-blockack.md) defines A1. The [PHY execution record](ieee80211-ampdu-phy-execution.md) supplies the prerequisite evidence.

GitHub queries failed, so this execution does not confirm merge status. No commit, push, or baseline change forms part of this task.

## Validated implementation contract

The MIB owns local HT capabilities and negotiated receiver limits. Before A1, management conversion omitted minimum MPDU start spacing. The serializer already preserved its encoded bits. A1 adds the encoded value to both capability directions and management conversion. Existing peer removal removes the complete capability state. The default remains zero.

The aggregate builder owns delimiter insertion, alignment, and content references. Before A1, its region-tag destination preceded the member content. A1 corrects that offset and uses the source Packet's front offset. A new construction API retains the source Packets on success and refusal. The existing consuming API retains its ownership contract. No queue, sequence, lifetime, retry, or acknowledgment state changes through construction.

The HT mode owns the conversion from receiver spacing to octet distance. It also owns the L-SIG length and PPDU duration checks. The builder uses these calculations with the supplied peer byte limit. It adds zero-length delimiters between members when alignment alone cannot satisfy spacing. The final member has no alignment padding or extra delimiter.

Empty batches and size-limit violations produce refusal before allocation or source disposal. Invalid API parameters produce an error.

For example, two 30-octet members at 65 Mbps with encoded spacing 7 require 130 octets between their starts. Alignment and zero-length delimiters increase that distance to 132 octets. The builder retains both source Packets. The resulting aggregate ends immediately after the second member.

The focused checks cover all spacing encodings and exponents, asymmetric negotiation, management bytes, and cleanup. Existing full and simplified association fixtures verify their production paths. New construction checks cover alignment, zero-length delimiters, byte limits, source retention, region tags, and singleton identity. The existing PHY wire fixture owns HT-SIG golden bytes and independent decode. The PHY reception fixture owns error-model compatibility. These checks do not establish aggregate selection or implicit responses, which belong to later stages.

Source inspection confirms one implementation of the aggregate interface. No target falls under the packet subsystem seal. The contract passes the pre-write checks. The implementation adds no event, timer, callback, retained protocol state, or lifecycle operation.

## Result and limits

The MIB and management preserve the receiver spacing value in both directions. The new aggregate API retains source Packets and refuses excessive member, PSDU, peer, or duration values. The builder copies region tags to member content positions. HT modes supply octet distance, L-SIG length, and separate duration predicates. No HT-SIG field, serializer, or bounded parser required a change; A1 consumes the implemented P1 representation.

The new construction fixture checks MCS 0 and 7, four alignment residues, and all eight spacing encodings. It checks the 65535-octet PSDU boundary at MCS 15, where airtime permits that length. This calculation test does not expand the feature's transmit profile. It separately checks the 4095-octet member boundary and source retention after refusal. Deaggregation restores each member's region-tag offsets. Fresh bytes preserve singleton HT-SIG identity.

The capability fixture checks all 32 combinations of length exponent and spacing through production simplified management. It passes the negotiated transmit limits to the builder. Existing fixtures check asymmetric values through full association and simplified restart, shutdown, and crash. Peer cleanup removes the complete old state. The PHY reception fixture retains its absent-role, incomplete-coverage, absent-model, and compatible-model checks.

A1 adds no aggregate selection, DATA commitment, implicit response, or selective retry. The existing disabled invocation and default aggregate policy remain in place. A3 owns the production selection of members. Later stages retain their MAC reception and exchange requirements. No fingerprint, performance, or complete feature claim follows from these checks. Release compilation and publication gates remain outside this task.

## Verification

All commands ran from the active checkout. Builds use debug mode. Each test command first uses `source ./setenv -q`. This selects the active checkout instead of the inherited parent environment. Each runner also uses the environment assignments below:

```sh
MAKEFLAGS=--jobserver-style=pipe MPLCONFIGDIR=/tmp/a1-matplotlib
```

The assignments precede each runner command on the same shell line. They select pipe-based make coordination and a writable Matplotlib directory. The initial inherited environment caused a read-only filesystem error before any test ran. The corrected environment resolved that setup error.

| Check | Exact command after the environment setup | Result and artifact |
| --- | --- | --- |
| Base library before source edits. | `make MODE=debug -j8` | Exit 0; `/tmp/a1-build-base.log`. |
| Base unit controls. | `inet_run_unit_tests -m debug -f 'Ieee80211(HtCapabilities\|HtMgmtElements\|AmpduWire\|CompressedBlockAckRecord)_1\.test$' --no-concurrent` | Four fixtures pass, exit 0; `/tmp/a1-base-unit.log`. |
| Base ordinary PHY and Normal Ack controls. | `inet_run_module_tests -m debug -f 'Ieee80211(VhtHtSupport\|CompressedBlockAckMigration)_1\.test$' --no-concurrent` | Two fixtures pass across 13 runs, exit 0; `/tmp/a1-base-module.log`. |
| A1 library. | `make MODE=debug -j8` | Exit 0; `/tmp/a1-build.log`. |
| Unit selection. | `inet_run_unit_tests -m debug -f 'Ieee80211(AmpduCapabilityEncoding\|AmpduConstruction\|AmpduWire\|MpduDelimiterWire\|MpduSubframeHeaderSerializer\|HtCapabilities\|HtMgmtElements\|HtModeSet\|HtModeCache\|CompressedBlockAckRecord)_1\.test$' --no-concurrent` | Eight existing fixtures pass. Two new fixtures require test corrections; exit 1. `/tmp/a1-unit.log`. |
| Corrected new unit fixtures. | `inet_run_unit_tests -m debug -f 'Ieee80211Ampdu(CapabilityEncoding\|Construction)_1\.test$' --no-concurrent` | Both fixtures pass, exit 0; `/tmp/a1-new-unit.log`. |
| Module selection. | `inet_run_module_tests -m debug -f 'Ieee80211(AmpduCapabilities\|HtAssociation\|MgmtStaSimplifiedInitialization\|VhtHtSupport\|CompressedBlockAckMigration\|BlockAckAssociationReset\|CompressedBlockAckExchange)_1\.test$' --no-concurrent` | Six existing fixtures pass across 47 runs. The new fixture requires a test correction; exit 1. `/tmp/a1-module.log`. |
| Corrected capability fixture and PHY controls. | `inet_run_module_tests -m debug -f 'Ieee80211Ampdu(Capabilities\|PhyReception)_1\.test$' --no-concurrent` | Both fixtures pass, exit 0; `/tmp/a1-capabilities-phy.log`. Capability coverage uses 32 runs with seed 0. PHY coverage uses 468 defined configurations with its seeds 0, 1, and 2. |

The new unit fixtures required an explicit Packet include and a valid HT factory call. The new module fixture required an explicit global class name. Its first corrected run completed all 32 simulations but lacked the required output marker. A direct completion message resolved that fixture failure. `/tmp/a1-capabilities-module.log` records the intermediate marker failure. Production source remained unchanged through these test corrections.

The final evidence covers ten distinct unit fixtures and eight distinct module fixtures. No final fixture retains an unexpected failure. The table preserves earlier runner failures rather than treating their aggregate exit status as success. The source base has no production diff before the base build; `/tmp/a1-base-source-diff.log` records that check.

Module fixtures use configuration `General`. All module runs use seed 0 except the PHY fixture, which explicitly varies seeds 0, 1, and 2. The exact run ranges are:

| Fixture | Runs |
| --- | --- |
| `Ieee80211VhtHtSupport_1.test` | 0–6 |
| `Ieee80211CompressedBlockAckMigration_1.test` | 0–5 |
| `Ieee80211CompressedBlockAckExchange_1.test` | 0–15 |
| `Ieee80211BlockAckAssociationReset_1.test` | 0–15 |
| `Ieee80211HtAssociation_1.test` | 0 |
| `Ieee80211MgmtStaSimplifiedInitialization_1.test` | 0 |
| `Ieee80211AmpduCapabilities_1.test` | 0–31 |
| `Ieee80211AmpduPhyReception_1.test` | 0–467 |

Both scoped architecture commands pass with exit 0:

```sh
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
```

Their logs are `/tmp/a1-architecture-mac.log` and `/tmp/a1-architecture-phy.log`. `doc/project/enforcement/check-source-seals.sh --diff` passes with exit 0. Its log is `/tmp/a1-source-seals.log`. `python3 doc/project/enforcement/check-ned-msg-naming.py --diff --scope src/inet/linklayer/ieee80211` passes with exit 0. Its log is `/tmp/a1-naming-diff.log`.

The complete WLAN naming scan uses `doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211`. It exits 1 with 32 candidates, all in unchanged source files. `/tmp/a1-naming.log` preserves that result. This task introduces no candidate in that scan. The execution does not claim that the complete subtree satisfies every naming rule.

The self-audit confirms the interface implementation, ownership paths, source offsets, byte limits, and mode calculations against the validated contract. No implementation scope expansion occurred. `git diff --check` passes. A byte comparison confirms preservation of the separate `AGENTS.md` change.

## Standard evidence

The implementation uses normative IEEE Std 802.11-2024 requirements. The corpus supplies these canonical nodes:

| Requirement | Canonical node | Physical PDF pages |
| --- | --- | --- |
| Receiver length and spacing encodings. | `ieee80211-2024:table:9-225` | 1126 |
| Octet distance and zero-length delimiters. | `ieee80211-2024:clause:10.12.3` | 1977–1979 |
| Mixed-format L-SIG length calculation. | `ieee80211-2024:clause:10.27.4` | 2091–2092 |
| HT PSDU limit, PPDU time limit, and signal extension. | `ieee80211-2024:table:19-25` | 3493–3494 |

The corpus required a rebuild for its current format. Its linter reports 76 ambiguous headings and 425 unresolved references across the corpus. Exact node retrieval identifies the requirements above. The spacing clause resolves its reference to Table 9-225 through exact table retrieval.

PDF inspection confirms the ceiling operators in Equation (10-17) on physical page 2091. The image is `/tmp/a1-duration-standard.png`. The raw retrieval and lint artifacts use the `/tmp/a1-` prefix.
