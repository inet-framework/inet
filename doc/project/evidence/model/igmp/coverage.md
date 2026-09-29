# IGMP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc9776/catalog.md](../../standard/rfc9776/catalog.md), [rfc2236/catalog.md](../../standard/rfc2236/catalog.md), [features.md](../../protocol/igmp/features.md), [results.md](results.md)

The single place that holds the changing state of the IGMP workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-29 17:29 +0200
- INET: branch `master`, commit `24675c3a37`, tree clean
- Trees: src `8b4f86968e`, tests/protocol `1f1d62beca`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/igmp$'`
- Suite: 42 tests, 24 PASS, 17 FAIL (unexpected), 1 FAIL (expected), so the suite reports FAIL
- Target level: 2

The analysis of every failure is in [`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a crafted message at level 3, the state of a node at level 4, a version 1 or an SSM-aware system at level 5 |
| `no check` | a permission that no observation can fail, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so.

| Status | RFC 9776 | RFC 2236 | Together |
| --- | --- | --- | --- |
| `selected` | 128 | 22 | 150 |
| `covered` | 76 | 34 | 110 |
| `owed` | 37 | 12 | 49 |
| `later` | 50 | 25 | 75 |
| `no check` | 2 | 1 | 3 |
| all | 293 | 94 | 387 |

### RFC 9776

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC9776-GEN-1](../../standard/rfc9776/catalog.md#rfc9776-gen-1) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-GEN-2](../../standard/rfc9776/catalog.md#rfc9776-gen-2) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-GEN-3](../../standard/rfc9776/catalog.md#rfc9776-gen-3) | selected | [Router Alert and precedence on every message](../../protocol/igmp/checks/message-format.md#router-alert-and-precedence-on-every-message) | `Rfc9776Precedence` | FAIL | fails: [gap 2](results.md#gap-2-defect--a-report-leaves-with-precedence-0) |
| [RFC9776-GEN-4](../../standard/rfc9776/catalog.md#rfc9776-gen-4) | selected | [Router Alert and precedence on every message](../../protocol/igmp/checks/message-format.md#router-alert-and-precedence-on-every-message) | `Rfc9776RouterAlert` | PASS | — |
| [RFC9776-GEN-5](../../standard/rfc9776/catalog.md#rfc9776-gen-5) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-GEN-6](../../standard/rfc9776/catalog.md#rfc9776-gen-6) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-GEN-7](../../standard/rfc9776/catalog.md#rfc9776-gen-7) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-GEN-8](../../standard/rfc9776/catalog.md#rfc9776-gen-8) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-GEN-9](../../standard/rfc9776/catalog.md#rfc9776-gen-9) | selected | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | `Rfc9776LeaveV2Mode` | PASS | — |
| [RFC9776-GEN-10](../../standard/rfc9776/catalog.md#rfc9776-gen-10) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-QRY-1](../../standard/rfc9776/catalog.md#rfc9776-qry-1) | covered | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-2](../../standard/rfc9776/catalog.md#rfc9776-qry-2) | covered | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-3](../../standard/rfc9776/catalog.md#rfc9776-qry-3) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-4](../../standard/rfc9776/catalog.md#rfc9776-qry-4) | owed | — | — | — | a later level 2 pass: a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 |
| [RFC9776-QRY-5](../../standard/rfc9776/catalog.md#rfc9776-qry-5) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-6](../../standard/rfc9776/catalog.md#rfc9776-qry-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-QRY-7](../../standard/rfc9776/catalog.md#rfc9776-qry-7) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-8](../../standard/rfc9776/catalog.md#rfc9776-qry-8) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-QRY-9](../../standard/rfc9776/catalog.md#rfc9776-qry-9) | covered | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-10](../../standard/rfc9776/catalog.md#rfc9776-qry-10) | owed | — | — | — | a later level 2 pass: a second router that hears a Query with the S flag set |
| [RFC9776-QRY-11](../../standard/rfc9776/catalog.md#rfc9776-qry-11) | owed | — | — | — | a later level 2 pass: a second router that hears a Query with the S flag set |
| [RFC9776-QRY-12](../../standard/rfc9776/catalog.md#rfc9776-qry-12) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9776-QRY-13](../../standard/rfc9776/catalog.md#rfc9776-qry-13) | owed | — | — | — | a later level 2 pass: a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 |
| [RFC9776-QRY-14](../../standard/rfc9776/catalog.md#rfc9776-qry-14) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9776-QRY-15](../../standard/rfc9776/catalog.md#rfc9776-qry-15) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9776-QRY-16](../../standard/rfc9776/catalog.md#rfc9776-qry-16) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9776-QRY-17](../../standard/rfc9776/catalog.md#rfc9776-qry-17) | owed | — | — | — | a later level 2 pass: a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 |
| [RFC9776-QRY-18](../../standard/rfc9776/catalog.md#rfc9776-qry-18) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9776-QRY-19](../../standard/rfc9776/catalog.md#rfc9776-qry-19) | covered | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-20](../../standard/rfc9776/catalog.md#rfc9776-qry-20) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-21](../../standard/rfc9776/catalog.md#rfc9776-qry-21) | owed | — | — | — | a later level 2 pass: many sources or many groups, beyond the size of one message |
| [RFC9776-QRY-22](../../standard/rfc9776/catalog.md#rfc9776-qry-22) | covered | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-23](../../standard/rfc9776/catalog.md#rfc9776-qry-23) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-QRY-24](../../standard/rfc9776/catalog.md#rfc9776-qry-24) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-25](../../standard/rfc9776/catalog.md#rfc9776-qry-25) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-26](../../standard/rfc9776/catalog.md#rfc9776-qry-26) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-QRY-27](../../standard/rfc9776/catalog.md#rfc9776-qry-27) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | PASS | its observation held before the failure |
| [RFC9776-QRY-28](../../standard/rfc9776/catalog.md#rfc9776-qry-28) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-QRY-29](../../standard/rfc9776/catalog.md#rfc9776-qry-29) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-QRY-30](../../standard/rfc9776/catalog.md#rfc9776-qry-30) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-1](../../standard/rfc9776/catalog.md#rfc9776-rep-1) | covered | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-2](../../standard/rfc9776/catalog.md#rfc9776-rep-2) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-3](../../standard/rfc9776/catalog.md#rfc9776-rep-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-4](../../standard/rfc9776/catalog.md#rfc9776-rep-4) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-5](../../standard/rfc9776/catalog.md#rfc9776-rep-5) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-6](../../standard/rfc9776/catalog.md#rfc9776-rep-6) | covered | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-7](../../standard/rfc9776/catalog.md#rfc9776-rep-7) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-8](../../standard/rfc9776/catalog.md#rfc9776-rep-8) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-REP-9](../../standard/rfc9776/catalog.md#rfc9776-rep-9) | covered | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-10](../../standard/rfc9776/catalog.md#rfc9776-rep-10) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-REP-11](../../standard/rfc9776/catalog.md#rfc9776-rep-11) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-REP-12](../../standard/rfc9776/catalog.md#rfc9776-rep-12) | covered | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-REP-13](../../standard/rfc9776/catalog.md#rfc9776-rep-13) | covered | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-14](../../standard/rfc9776/catalog.md#rfc9776-rep-14) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-15](../../standard/rfc9776/catalog.md#rfc9776-rep-15) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-16](../../standard/rfc9776/catalog.md#rfc9776-rep-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-17](../../standard/rfc9776/catalog.md#rfc9776-rep-17) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-18](../../standard/rfc9776/catalog.md#rfc9776-rep-18) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-REP-19](../../standard/rfc9776/catalog.md#rfc9776-rep-19) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-REP-20](../../standard/rfc9776/catalog.md#rfc9776-rep-20) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-REP-21](../../standard/rfc9776/catalog.md#rfc9776-rep-21) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-REP-22](../../standard/rfc9776/catalog.md#rfc9776-rep-22) | covered | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-REP-23](../../standard/rfc9776/catalog.md#rfc9776-rep-23) | selected | [Leave reported](../../protocol/igmp/checks/host-reports.md#leave-reported) | `Rfc9776LeaveReport` | PASS | — |
| [RFC9776-REP-24](../../standard/rfc9776/catalog.md#rfc9776-rep-24) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-REP-25](../../standard/rfc9776/catalog.md#rfc9776-rep-25) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-REP-26](../../standard/rfc9776/catalog.md#rfc9776-rep-26) | covered | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-REP-27](../../standard/rfc9776/catalog.md#rfc9776-rep-27) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-REP-28](../../standard/rfc9776/catalog.md#rfc9776-rep-28) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-REP-29](../../standard/rfc9776/catalog.md#rfc9776-rep-29) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9776-REP-30](../../standard/rfc9776/catalog.md#rfc9776-rep-30) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-31](../../standard/rfc9776/catalog.md#rfc9776-rep-31) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-32](../../standard/rfc9776/catalog.md#rfc9776-rep-32) | owed | — | — | — | a later level 2 pass: a system that reports before it has an address |
| [RFC9776-REP-33](../../standard/rfc9776/catalog.md#rfc9776-rep-33) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-34](../../standard/rfc9776/catalog.md#rfc9776-rep-34) | selected | [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | `Rfc9776ReportEncapsulation` | PASS | — |
| [RFC9776-REP-35](../../standard/rfc9776/catalog.md#rfc9776-rep-35) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-REP-36](../../standard/rfc9776/catalog.md#rfc9776-rep-36) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-REP-37](../../standard/rfc9776/catalog.md#rfc9776-rep-37) | owed | — | — | — | a later level 2 pass: many sources or many groups, beyond the size of one message |
| [RFC9776-REP-38](../../standard/rfc9776/catalog.md#rfc9776-rep-38) | owed | — | — | — | a later level 2 pass: many sources or many groups, beyond the size of one message |
| [RFC9776-REP-39](../../standard/rfc9776/catalog.md#rfc9776-rep-39) | owed | — | — | — | a later level 2 pass: many sources or many groups, beyond the size of one message |
| [RFC9776-HOST-1](../../standard/rfc9776/catalog.md#rfc9776-host-1) | covered | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-HOST-2](../../standard/rfc9776/catalog.md#rfc9776-host-2) | later | — | — | — | level 4: state inside the node |
| [RFC9776-HOST-3](../../standard/rfc9776/catalog.md#rfc9776-host-3) | covered | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-HOST-4](../../standard/rfc9776/catalog.md#rfc9776-host-4) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-HOST-5](../../standard/rfc9776/catalog.md#rfc9776-host-5) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-HOST-6](../../standard/rfc9776/catalog.md#rfc9776-host-6) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-HOST-7](../../standard/rfc9776/catalog.md#rfc9776-host-7) | covered | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776GeneralQueryNoState` | PASS | the host keeps INCLUDE({}) for the group that it left; the record for it is gap 8 |
| [RFC9776-HOST-8](../../standard/rfc9776/catalog.md#rfc9776-host-8) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-HOST-9](../../standard/rfc9776/catalog.md#rfc9776-host-9) | selected | [Change inside EXCLUDE mode](../../protocol/igmp/checks/host-reports.md#change-inside-exclude-mode) | `Rfc9776ExcludeChange` | PASS | — |
| [RFC9776-HOST-10](../../standard/rfc9776/catalog.md#rfc9776-host-10) | selected | [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | `Rfc9776JoinReport` | PASS | — |
| [RFC9776-HOST-11](../../standard/rfc9776/catalog.md#rfc9776-host-11) | selected | [Leave reported](../../protocol/igmp/checks/host-reports.md#leave-reported) | `Rfc9776LeaveReport` | PASS | — |
| [RFC9776-HOST-12](../../standard/rfc9776/catalog.md#rfc9776-host-12) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-HOST-13](../../standard/rfc9776/catalog.md#rfc9776-host-13) | selected | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeatedCount` | PASS | — |
| [RFC9776-HOST-14](../../standard/rfc9776/catalog.md#rfc9776-host-14) | selected | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | `Rfc9776ChangeDuringRepetitions` | PASS | the Report after the second change leaves at once |
| [RFC9776-HOST-15](../../standard/rfc9776/catalog.md#rfc9776-host-15) | selected | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | `Rfc9776ChangeDuringRepetitions` | FAIL | fails: [gap 5](results.md#gap-5-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9776-HOST-16](../../standard/rfc9776/catalog.md#rfc9776-host-16) | selected | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | `Rfc9776ChangeDuringRepetitions` | FAIL | fails: [gap 5](results.md#gap-5-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9776-HOST-17](../../standard/rfc9776/catalog.md#rfc9776-host-17) | selected | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | `Rfc9776ChangeDuringRepetitionsTail` | PASS | — |
| [RFC9776-HOST-18](../../standard/rfc9776/catalog.md#rfc9776-host-18) | selected | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeated` | FAIL | fails: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9776-HOST-19](../../standard/rfc9776/catalog.md#rfc9776-host-19) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9776-HOST-20](../../standard/rfc9776/catalog.md#rfc9776-host-20) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9776-HOST-21](../../standard/rfc9776/catalog.md#rfc9776-host-21) | selected | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeated` | FAIL | not reached: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) keeps the test from the observation |
| [RFC9776-HOST-22](../../standard/rfc9776/catalog.md#rfc9776-host-22) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-HOST-23](../../standard/rfc9776/catalog.md#rfc9776-host-23) | selected | [Leave reported](../../protocol/igmp/checks/host-reports.md#leave-reported) | `Rfc9776LeaveReport` | PASS | — |
| [RFC9776-HOST-24](../../standard/rfc9776/catalog.md#rfc9776-host-24) | selected | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeated` | FAIL | not reached: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) keeps the test from the observation |
| [RFC9776-HOST-25](../../standard/rfc9776/catalog.md#rfc9776-host-25) | selected | [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | `Rfc9776ChangeDuringRepetitions` | FAIL | fails: [gap 5](results.md#gap-5-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9776-HOST-26](../../standard/rfc9776/catalog.md#rfc9776-host-26) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-HOST-27](../../standard/rfc9776/catalog.md#rfc9776-host-27) | selected | [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | `Rfc9776SourceListChange` | PASS | — |
| [RFC9776-HQRY-1](../../standard/rfc9776/catalog.md#rfc9776-hqry-1) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-2](../../standard/rfc9776/catalog.md#rfc9776-hqry-2) | covered | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-3](../../standard/rfc9776/catalog.md#rfc9776-hqry-3) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-4](../../standard/rfc9776/catalog.md#rfc9776-hqry-4) | covered | [Response to a Group-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-specific-query) | `Rfc9776GroupSpecificResponse` | PASS | — |
| [RFC9776-HQRY-5](../../standard/rfc9776/catalog.md#rfc9776-hqry-5) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-6](../../standard/rfc9776/catalog.md#rfc9776-hqry-6) | selected | [Response to a Group-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-specific-query) | `Rfc9776GroupSpecificResponse` | PASS | — |
| [RFC9776-HQRY-7](../../standard/rfc9776/catalog.md#rfc9776-hqry-7) | selected | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | PASS | its observation held before the failure |
| [RFC9776-HQRY-8](../../standard/rfc9776/catalog.md#rfc9776-hqry-8) | owed | — | — | — | a later level 2 pass: two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host |
| [RFC9776-HQRY-9](../../standard/rfc9776/catalog.md#rfc9776-hqry-9) | covered | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | PASS | its observation held before the failure |
| [RFC9776-HQRY-10](../../standard/rfc9776/catalog.md#rfc9776-hqry-10) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryNoState` | FAIL | fails: [gap 8](results.md#gap-8-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out); observation 4 of `Rfc9776GeneralQueryResponse` holds |
| [RFC9776-HQRY-11](../../standard/rfc9776/catalog.md#rfc9776-hqry-11) | covered | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-12](../../standard/rfc9776/catalog.md#rfc9776-hqry-12) | selected | [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | `Rfc9776GeneralQueryResponse` | PASS | — |
| [RFC9776-HQRY-13](../../standard/rfc9776/catalog.md#rfc9776-hqry-13) | selected | [Response to a Group-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-specific-query) | `Rfc9776GroupSpecificResponse` | PASS | — |
| [RFC9776-HQRY-14](../../standard/rfc9776/catalog.md#rfc9776-hqry-14) | selected | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | PASS | its observation held before the failure |
| [RFC9776-HQRY-15](../../standard/rfc9776/catalog.md#rfc9776-hqry-15) | selected | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | PASS | its observation held before the failure |
| [RFC9776-HQRY-16](../../standard/rfc9776/catalog.md#rfc9776-hqry-16) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-HQRY-17](../../standard/rfc9776/catalog.md#rfc9776-hqry-17) | selected | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | FAIL | fails: [gap 8](results.md#gap-8-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out) |
| [RFC9776-HQRY-18](../../standard/rfc9776/catalog.md#rfc9776-hqry-18) | covered | [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | `Rfc9776GroupSourceResponse` | PASS | its observation held before the failure |
| [RFC9776-RQ-1](../../standard/rfc9776/catalog.md#rfc9776-rq-1) | covered | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC9776-RQ-2](../../standard/rfc9776/catalog.md#rfc9776-rq-2) | covered | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-RQ-3](../../standard/rfc9776/catalog.md#rfc9776-rq-3) | later | — | — | — | level 4: state inside the node |
| [RFC9776-RQ-4](../../standard/rfc9776/catalog.md#rfc9776-rq-4) | later | — | — | — | level 4: state inside the node |
| [RFC9776-RQ-5](../../standard/rfc9776/catalog.md#rfc9776-rq-5) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-RQ-6](../../standard/rfc9776/catalog.md#rfc9776-rq-6) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC9776-RQ-7](../../standard/rfc9776/catalog.md#rfc9776-rq-7) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQ-8](../../standard/rfc9776/catalog.md#rfc9776-rq-8) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQ-9](../../standard/rfc9776/catalog.md#rfc9776-rq-9) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | PASS | its observation held before the failure |
| [RFC9776-RQ-10](../../standard/rfc9776/catalog.md#rfc9776-rq-10) | covered | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | PASS | its observation held before the failure |
| [RFC9776-RST-1](../../standard/rfc9776/catalog.md#rfc9776-rst-1) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-2](../../standard/rfc9776/catalog.md#rfc9776-rst-2) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-3](../../standard/rfc9776/catalog.md#rfc9776-rst-3) | covered | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-RST-4](../../standard/rfc9776/catalog.md#rfc9776-rst-4) | covered | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-RST-5](../../standard/rfc9776/catalog.md#rfc9776-rst-5) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RST-6](../../standard/rfc9776/catalog.md#rfc9776-rst-6) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-7](../../standard/rfc9776/catalog.md#rfc9776-rst-7) | covered | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | its observation held before the failure |
| [RFC9776-RST-8](../../standard/rfc9776/catalog.md#rfc9776-rst-8) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-9](../../standard/rfc9776/catalog.md#rfc9776-rst-9) | covered | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | its observation held before the failure |
| [RFC9776-RST-10](../../standard/rfc9776/catalog.md#rfc9776-rst-10) | selected | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | the router deletes the record when its own interval, 260 s, ends |
| [RFC9776-RST-11](../../standard/rfc9776/catalog.md#rfc9776-rst-11) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-12](../../standard/rfc9776/catalog.md#rfc9776-rst-12) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-13](../../standard/rfc9776/catalog.md#rfc9776-rst-13) | covered | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RST-14](../../standard/rfc9776/catalog.md#rfc9776-rst-14) | covered | [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | `Rfc9776SourceBlockedOnlyMember` | FAIL | not reached: [gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) keeps the test from the observation |
| [RFC9776-RST-15](../../standard/rfc9776/catalog.md#rfc9776-rst-15) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RST-16](../../standard/rfc9776/catalog.md#rfc9776-rst-16) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RST-17](../../standard/rfc9776/catalog.md#rfc9776-rst-17) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-FWD-1](../../standard/rfc9776/catalog.md#rfc9776-fwd-1) | covered | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-FWD-2](../../standard/rfc9776/catalog.md#rfc9776-fwd-2) | no check | — | — | — | a permission that no observation can fail |
| [RFC9776-FWD-3](../../standard/rfc9776/catalog.md#rfc9776-fwd-3) | selected | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-FWD-4](../../standard/rfc9776/catalog.md#rfc9776-fwd-4) | covered | [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | `Rfc9776SourceBlockedOnlyMember` | FAIL | not reached: [gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) keeps the test from the observation |
| [RFC9776-FWD-5](../../standard/rfc9776/catalog.md#rfc9776-fwd-5) | selected | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificBlocking` | FAIL | fails: [gap 12](results.md#gap-12-defect--the-forwarding-asks-for-a-listener-of-the-group-not-of-the-source) |
| [RFC9776-FWD-6](../../standard/rfc9776/catalog.md#rfc9776-fwd-6) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-FWD-7](../../standard/rfc9776/catalog.md#rfc9776-fwd-7) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-FWD-8](../../standard/rfc9776/catalog.md#rfc9776-fwd-8) | selected | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-RREP-1](../../standard/rfc9776/catalog.md#rfc9776-rrep-1) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-RREP-2](../../standard/rfc9776/catalog.md#rfc9776-rrep-2) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-RREP-3](../../standard/rfc9776/catalog.md#rfc9776-rrep-3) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-RREP-4](../../standard/rfc9776/catalog.md#rfc9776-rrep-4) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-RREP-5](../../standard/rfc9776/catalog.md#rfc9776-rrep-5) | covered | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | its observation held before the failure |
| [RFC9776-RREP-6](../../standard/rfc9776/catalog.md#rfc9776-rrep-6) | covered | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | its observation held before the failure |
| [RFC9776-RREP-7](../../standard/rfc9776/catalog.md#rfc9776-rrep-7) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RREP-8](../../standard/rfc9776/catalog.md#rfc9776-rrep-8) | owed | — | — | — | a later level 2 pass: a run longer than the Group Membership Interval with an INCLUDE-mode member that answers the Queries |
| [RFC9776-RREP-9](../../standard/rfc9776/catalog.md#rfc9776-rrep-9) | selected | [A router that comes up after a join](../../protocol/igmp/checks/router-state.md#a-router-that-comes-up-after-a-join) | `Rfc9776RouterAfterJoin` | PASS | — |
| [RFC9776-RREP-10](../../standard/rfc9776/catalog.md#rfc9776-rrep-10) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RREP-11](../../standard/rfc9776/catalog.md#rfc9776-rrep-11) | selected | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | FAIL | fails: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9776-RREP-12](../../standard/rfc9776/catalog.md#rfc9776-rrep-12) | selected | [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | `Rfc9776SourceBlockedOnlyMember` | PASS | its observation held before the failure |
| [RFC9776-RREP-13](../../standard/rfc9776/catalog.md#rfc9776-rrep-13) | covered | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | FAIL | fails: [gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9776-RREP-14](../../standard/rfc9776/catalog.md#rfc9776-rrep-14) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | FAIL | not reached: [gap 10](results.md#gap-10-defect--a-report-cancels-the-retransmissions-of-the-queries) keeps the test from the observation |
| [RFC9776-RREP-15](../../standard/rfc9776/catalog.md#rfc9776-rrep-15) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RREP-16](../../standard/rfc9776/catalog.md#rfc9776-rrep-16) | selected | [Leave with another member](../../protocol/igmp/checks/router-state.md#leave-with-another-member) | `Rfc9776LeaveWithAnotherMember` | PASS | — |
| [RFC9776-RREP-17](../../standard/rfc9776/catalog.md#rfc9776-rrep-17) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RREP-18](../../standard/rfc9776/catalog.md#rfc9776-rrep-18) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RREP-19](../../standard/rfc9776/catalog.md#rfc9776-rrep-19) | owed | — | — | — | a later level 2 pass: two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host |
| [RFC9776-RREP-20](../../standard/rfc9776/catalog.md#rfc9776-rrep-20) | selected | [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | `Rfc9776SourceSpecificForwarding` | PASS | — |
| [RFC9776-RREP-21](../../standard/rfc9776/catalog.md#rfc9776-rrep-21) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | PASS | its observation held before the failure |
| [RFC9776-RREP-22](../../standard/rfc9776/catalog.md#rfc9776-rrep-22) | selected | [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | `Rfc9776ForwardingAfterJoin` | PASS | — |
| [RFC9776-RREP-23](../../standard/rfc9776/catalog.md#rfc9776-rrep-23) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-RREP-24](../../standard/rfc9776/catalog.md#rfc9776-rrep-24) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RREP-25](../../standard/rfc9776/catalog.md#rfc9776-rrep-25) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RREP-26](../../standard/rfc9776/catalog.md#rfc9776-rrep-26) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-RREP-27](../../standard/rfc9776/catalog.md#rfc9776-rrep-27) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RSW-1](../../standard/rfc9776/catalog.md#rfc9776-rsw-1) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RSW-2](../../standard/rfc9776/catalog.md#rfc9776-rsw-2) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQRY-1](../../standard/rfc9776/catalog.md#rfc9776-rqry-1) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQRY-2](../../standard/rfc9776/catalog.md#rfc9776-rqry-2) | selected | [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | `Rfc9776SourceBlockedOnlyMember` | FAIL | fails: [gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9776-RQRY-3](../../standard/rfc9776/catalog.md#rfc9776-rqry-3) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQRY-4](../../standard/rfc9776/catalog.md#rfc9776-rqry-4) | owed | — | — | — | a later level 2 pass: a second router that hears a Query with the S flag set |
| [RFC9776-RQRY-5](../../standard/rfc9776/catalog.md#rfc9776-rqry-5) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC9776-RQRY-6](../../standard/rfc9776/catalog.md#rfc9776-rqry-6) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC9776-RQRY-7](../../standard/rfc9776/catalog.md#rfc9776-rqry-7) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC9776-RQRY-8](../../standard/rfc9776/catalog.md#rfc9776-rqry-8) | selected | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | `Rfc9776QuerierV2Router` | FAIL | fails: [gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) |
| [RFC9776-RQRY-9](../../standard/rfc9776/catalog.md#rfc9776-rqry-9) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQRY-10](../../standard/rfc9776/catalog.md#rfc9776-rqry-10) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-RQRY-11](../../standard/rfc9776/catalog.md#rfc9776-rqry-11) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuerySFlag` | FAIL | fails: [gap 9](results.md#gap-9-defect--the-s-flag-of-a-group-specific-query-comes-from-the-timer-before-it-is-lowered) |
| [RFC9776-RQRY-12](../../standard/rfc9776/catalog.md#rfc9776-rqry-12) | covered | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | FAIL | fails: [gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9776-RQRY-13](../../standard/rfc9776/catalog.md#rfc9776-rqry-13) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | FAIL | fails: [gap 10](results.md#gap-10-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9776-RQRY-14](../../standard/rfc9776/catalog.md#rfc9776-rqry-14) | selected | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceQuerySFlag` | PASS | the S flag of the first Query holds; no second Query leaves, [gap 10](results.md#gap-10-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9776-RQRY-15](../../standard/rfc9776/catalog.md#rfc9776-rqry-15) | covered | [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | `Rfc9776SourceBlockedOtherMember` | PASS | its observation held before the failure |
| [RFC9776-VER-1](../../standard/rfc9776/catalog.md#rfc9776-ver-1) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-VER-2](../../standard/rfc9776/catalog.md#rfc9776-ver-2) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-VER-3](../../standard/rfc9776/catalog.md#rfc9776-ver-3) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-VER-4](../../standard/rfc9776/catalog.md#rfc9776-ver-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-COMPH-1](../../standard/rfc9776/catalog.md#rfc9776-comph-1) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-2](../../standard/rfc9776/catalog.md#rfc9776-comph-2) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-3](../../standard/rfc9776/catalog.md#rfc9776-comph-3) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-4](../../standard/rfc9776/catalog.md#rfc9776-comph-4) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-5](../../standard/rfc9776/catalog.md#rfc9776-comph-5) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-6](../../standard/rfc9776/catalog.md#rfc9776-comph-6) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-7](../../standard/rfc9776/catalog.md#rfc9776-comph-7) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-8](../../standard/rfc9776/catalog.md#rfc9776-comph-8) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-9](../../standard/rfc9776/catalog.md#rfc9776-comph-9) | selected | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776HostBackToV3` | PASS | — |
| [RFC9776-COMPH-10](../../standard/rfc9776/catalog.md#rfc9776-comph-10) | covered | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776HostBackToV3` | PASS | — |
| [RFC9776-COMPH-11](../../standard/rfc9776/catalog.md#rfc9776-comph-11) | later | — | — | — | level 3: needs a crafted message |
| [RFC9776-COMPH-12](../../standard/rfc9776/catalog.md#rfc9776-comph-12) | covered | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776HostBackToV3` | PASS | — |
| [RFC9776-COMPH-13](../../standard/rfc9776/catalog.md#rfc9776-comph-13) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-14](../../standard/rfc9776/catalog.md#rfc9776-comph-14) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-15](../../standard/rfc9776/catalog.md#rfc9776-comph-15) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-16](../../standard/rfc9776/catalog.md#rfc9776-comph-16) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-17](../../standard/rfc9776/catalog.md#rfc9776-comph-17) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-18](../../standard/rfc9776/catalog.md#rfc9776-comph-18) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-19](../../standard/rfc9776/catalog.md#rfc9776-comph-19) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-20](../../standard/rfc9776/catalog.md#rfc9776-comph-20) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPH-21](../../standard/rfc9776/catalog.md#rfc9776-comph-21) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-COMPH-22](../../standard/rfc9776/catalog.md#rfc9776-comph-22) | selected | [A mode change cancels the pending reports](../../protocol/igmp/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | `Rfc9776ModeChangeCancels` | FAIL | fails: [gap 7](results.md#gap-7-defect--the-igmpv2-mode-keeps-the-pending-igmpv3-retransmissions) |
| [RFC9776-COMPH-23](../../standard/rfc9776/catalog.md#rfc9776-comph-23) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPH-24](../../standard/rfc9776/catalog.md#rfc9776-comph-24) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPH-25](../../standard/rfc9776/catalog.md#rfc9776-comph-25) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPH-26](../../standard/rfc9776/catalog.md#rfc9776-comph-26) | no check | — | — | — | a permission that no observation can fail |
| [RFC9776-COMPH-27](../../standard/rfc9776/catalog.md#rfc9776-comph-27) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPR-1](../../standard/rfc9776/catalog.md#rfc9776-compr-1) | selected | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | `Rfc9776QuerierV2Router` | FAIL | fails: [gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) |
| [RFC9776-COMPR-2](../../standard/rfc9776/catalog.md#rfc9776-compr-2) | covered | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | `Rfc9776QuerierV2Router` | FAIL | fails: [gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) |
| [RFC9776-COMPR-3](../../standard/rfc9776/catalog.md#rfc9776-compr-3) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-4](../../standard/rfc9776/catalog.md#rfc9776-compr-4) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-5](../../standard/rfc9776/catalog.md#rfc9776-compr-5) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-6](../../standard/rfc9776/catalog.md#rfc9776-compr-6) | selected | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | `Rfc9776QuerierV2Router` | FAIL | fails: [gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) |
| [RFC9776-COMPR-7](../../standard/rfc9776/catalog.md#rfc9776-compr-7) | later | — | — | — | level 4: state inside the node |
| [RFC9776-COMPR-8](../../standard/rfc9776/catalog.md#rfc9776-compr-8) | selected | [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | `Rfc9776QuerierV2Router` | FAIL | not reached: [gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router) keeps the test from the observation |
| [RFC9776-COMPR-9](../../standard/rfc9776/catalog.md#rfc9776-compr-9) | later | — | — | — | level 4: state inside the node |
| [RFC9776-COMPR-10](../../standard/rfc9776/catalog.md#rfc9776-compr-10) | later | — | — | — | level 4: state inside the node |
| [RFC9776-COMPR-11](../../standard/rfc9776/catalog.md#rfc9776-compr-11) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPR-12](../../standard/rfc9776/catalog.md#rfc9776-compr-12) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9776-COMPR-13](../../standard/rfc9776/catalog.md#rfc9776-compr-13) | selected | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-14](../../standard/rfc9776/catalog.md#rfc9776-compr-14) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-15](../../standard/rfc9776/catalog.md#rfc9776-compr-15) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-16](../../standard/rfc9776/catalog.md#rfc9776-compr-16) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-17](../../standard/rfc9776/catalog.md#rfc9776-compr-17) | selected | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-18](../../standard/rfc9776/catalog.md#rfc9776-compr-18) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-19](../../standard/rfc9776/catalog.md#rfc9776-compr-19) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-20](../../standard/rfc9776/catalog.md#rfc9776-compr-20) | owed | — | — | — | a later level 2 pass: the end of the IGMPv2 mode of a group after the Older Host Present Interval |
| [RFC9776-COMPR-21](../../standard/rfc9776/catalog.md#rfc9776-compr-21) | owed | — | — | — | a later level 2 pass: the end of the IGMPv2 mode of a group after the Older Host Present Interval |
| [RFC9776-COMPR-22](../../standard/rfc9776/catalog.md#rfc9776-compr-22) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-23](../../standard/rfc9776/catalog.md#rfc9776-compr-23) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-24](../../standard/rfc9776/catalog.md#rfc9776-compr-24) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-25](../../standard/rfc9776/catalog.md#rfc9776-compr-25) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-26](../../standard/rfc9776/catalog.md#rfc9776-compr-26) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-27](../../standard/rfc9776/catalog.md#rfc9776-compr-27) | selected | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-28](../../standard/rfc9776/catalog.md#rfc9776-compr-28) | selected | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-COMPR-29](../../standard/rfc9776/catalog.md#rfc9776-compr-29) | selected | [A BLOCK record for a group in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#a-block-record-for-a-group-in-igmpv2-mode) | `Rfc9776BlockInV2Mode` | PASS | — |
| [RFC9776-COMPR-30](../../standard/rfc9776/catalog.md#rfc9776-compr-30) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9776-COMPR-31](../../standard/rfc9776/catalog.md#rfc9776-compr-31) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-32](../../standard/rfc9776/catalog.md#rfc9776-compr-32) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-COMPR-33](../../standard/rfc9776/catalog.md#rfc9776-compr-33) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC9776-TIMER-1](../../standard/rfc9776/catalog.md#rfc9776-timer-1) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9776-TIMER-2](../../standard/rfc9776/catalog.md#rfc9776-timer-2) | covered | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeatedCount` | PASS | — |
| [RFC9776-TIMER-3](../../standard/rfc9776/catalog.md#rfc9776-timer-3) | selected | [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | `Rfc9776QueryRobustness` | FAIL | fails: [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9776-TIMER-4](../../standard/rfc9776/catalog.md#rfc9776-timer-4) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC9776-TIMER-5](../../standard/rfc9776/catalog.md#rfc9776-timer-5) | selected | [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | `Rfc9776QueryEncapsulation` | PASS | its observation held before the failure |
| [RFC9776-TIMER-6](../../standard/rfc9776/catalog.md#rfc9776-timer-6) | selected | [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | `Rfc9776QueryTimerRelations` | PASS | — |
| [RFC9776-TIMER-7](../../standard/rfc9776/catalog.md#rfc9776-timer-7) | covered | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | PASS | the router waits its own interval, 260 s; the value is TIMER-8 |
| [RFC9776-TIMER-8](../../standard/rfc9776/catalog.md#rfc9776-timer-8) | selected | [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | `Rfc9776MembershipTimeout` | FAIL | fails: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9776-TIMER-9](../../standard/rfc9776/catalog.md#rfc9776-timer-9) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC9776-TIMER-10](../../standard/rfc9776/catalog.md#rfc9776-timer-10) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC9776-TIMER-11](../../standard/rfc9776/catalog.md#rfc9776-timer-11) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC9776-TIMER-12](../../standard/rfc9776/catalog.md#rfc9776-timer-12) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC9776-TIMER-13](../../standard/rfc9776/catalog.md#rfc9776-timer-13) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-TIMER-14](../../standard/rfc9776/catalog.md#rfc9776-timer-14) | owed | — | — | — | a later level 2 pass: a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 |
| [RFC9776-TIMER-15](../../standard/rfc9776/catalog.md#rfc9776-timer-15) | owed | — | — | — | a later level 2 pass: a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 |
| [RFC9776-TIMER-16](../../standard/rfc9776/catalog.md#rfc9776-timer-16) | selected | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-TIMER-17](../../standard/rfc9776/catalog.md#rfc9776-timer-17) | covered | [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | `Rfc9776LastMemberQuery` | PASS | — |
| [RFC9776-TIMER-18](../../standard/rfc9776/catalog.md#rfc9776-timer-18) | selected | [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | `Rfc9776JoinRepeated` | FAIL | fails: [gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9776-TIMER-19](../../standard/rfc9776/catalog.md#rfc9776-timer-19) | covered | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776HostBackToV3` | PASS | — |
| [RFC9776-TIMER-20](../../standard/rfc9776/catalog.md#rfc9776-timer-20) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC9776-TIMER-21](../../standard/rfc9776/catalog.md#rfc9776-timer-21) | covered | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776HostBackToV3` | PASS | — |
| [RFC9776-TIMER-22](../../standard/rfc9776/catalog.md#rfc9776-timer-22) | selected | [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | `Rfc9776OlderQuerierInterval` | FAIL | fails: [gap 4](results.md#gap-4-defect--the-igmpv2-mode-of-a-host-ends-after-the-other-querier-present-interval) |
| [RFC9776-TIMER-23](../../standard/rfc9776/catalog.md#rfc9776-timer-23) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-TIMER-24](../../standard/rfc9776/catalog.md#rfc9776-timer-24) | covered | [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | `Rfc9776RouterV2Member` | PASS | — |
| [RFC9776-TIMER-25](../../standard/rfc9776/catalog.md#rfc9776-timer-25) | owed | — | — | — | a later level 2 pass: the end of the IGMPv2 mode of a group after the Older Host Present Interval |
| [RFC9776-TIMER-26](../../standard/rfc9776/catalog.md#rfc9776-timer-26) | selected | [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | `Rfc9776QueryTimerRelations` | PASS | — |

### RFC 2236

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC2236-HOST-1](../../standard/rfc2236/catalog.md#rfc2236-host-1) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-2](../../standard/rfc2236/catalog.md#rfc2236-host-2) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-3](../../standard/rfc2236/catalog.md#rfc2236-host-3) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-4](../../standard/rfc2236/catalog.md#rfc2236-host-4) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-5](../../standard/rfc2236/catalog.md#rfc2236-host-5) | covered | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | `Rfc9776LeaveV2Mode` | PASS | — |
| [RFC2236-HOST-6](../../standard/rfc2236/catalog.md#rfc2236-host-6) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-7](../../standard/rfc2236/catalog.md#rfc2236-host-7) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-HOST-8](../../standard/rfc2236/catalog.md#rfc2236-host-8) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-HOST-9](../../standard/rfc2236/catalog.md#rfc2236-host-9) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-10](../../standard/rfc2236/catalog.md#rfc2236-host-10) | covered | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) keeps the test from the observation |
| [RFC2236-HOST-11](../../standard/rfc2236/catalog.md#rfc2236-host-11) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-HOST-12](../../standard/rfc2236/catalog.md#rfc2236-host-12) | covered | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) keeps the test from the observation |
| [RFC2236-HOST-13](../../standard/rfc2236/catalog.md#rfc2236-host-13) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-14](../../standard/rfc2236/catalog.md#rfc2236-host-14) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-HOST-15](../../standard/rfc2236/catalog.md#rfc2236-host-15) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-16](../../standard/rfc2236/catalog.md#rfc2236-host-16) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-17](../../standard/rfc2236/catalog.md#rfc2236-host-17) | no check | — | — | — | a permission that no observation can fail |
| [RFC2236-HOST-18](../../standard/rfc2236/catalog.md#rfc2236-host-18) | selected | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | `Rfc9776LeaveV2Mode` | PASS | — |
| [RFC2236-HOST-19](../../standard/rfc2236/catalog.md#rfc2236-host-19) | covered | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) keeps the test from the observation |
| [RFC2236-HOST-20](../../standard/rfc2236/catalog.md#rfc2236-host-20) | covered | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) keeps the test from the observation |
| [RFC2236-HOST-21](../../standard/rfc2236/catalog.md#rfc2236-host-21) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2236-HOST-22](../../standard/rfc2236/catalog.md#rfc2236-host-22) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2ModeRepeat` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2236-HOST-23](../../standard/rfc2236/catalog.md#rfc2236-host-23) | owed | — | — | — | a later level 2 pass: two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host |
| [RFC2236-HOST-24](../../standard/rfc2236/catalog.md#rfc2236-host-24) | covered | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | not reached: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) keeps the test from the observation |
| [RFC2236-HOST-25](../../standard/rfc2236/catalog.md#rfc2236-host-25) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-26](../../standard/rfc2236/catalog.md#rfc2236-host-26) | covered | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | `Rfc9776LeaveV2Mode` | PASS | — |
| [RFC2236-HOST-27](../../standard/rfc2236/catalog.md#rfc2236-host-27) | selected | [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | `Rfc9776LeaveV2Mode` | PASS | — |
| [RFC2236-HOST-28](../../standard/rfc2236/catalog.md#rfc2236-host-28) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2236-HOST-29](../../standard/rfc2236/catalog.md#rfc2236-host-29) | selected | [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | `Rfc2236ReportSuppression` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2236-HOST-30](../../standard/rfc2236/catalog.md#rfc2236-host-30) | owed | — | — | — | a later level 2 pass: two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host |
| [RFC2236-HOST-31](../../standard/rfc2236/catalog.md#rfc2236-host-31) | selected | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | FAIL | fails: [gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2236-HOST-32](../../standard/rfc2236/catalog.md#rfc2236-host-32) | covered | [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | `Rfc9776HostV2Mode` | PASS | its observation held before the failure |
| [RFC2236-HOST-33](../../standard/rfc2236/catalog.md#rfc2236-host-33) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-34](../../standard/rfc2236/catalog.md#rfc2236-host-34) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-35](../../standard/rfc2236/catalog.md#rfc2236-host-35) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-36](../../standard/rfc2236/catalog.md#rfc2236-host-36) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-37](../../standard/rfc2236/catalog.md#rfc2236-host-37) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-38](../../standard/rfc2236/catalog.md#rfc2236-host-38) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-39](../../standard/rfc2236/catalog.md#rfc2236-host-39) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-HOST-40](../../standard/rfc2236/catalog.md#rfc2236-host-40) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-1](../../standard/rfc2236/catalog.md#rfc2236-router-1) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-2](../../standard/rfc2236/catalog.md#rfc2236-router-2) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-3](../../standard/rfc2236/catalog.md#rfc2236-router-3) | covered | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-4](../../standard/rfc2236/catalog.md#rfc2236-router-4) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-5](../../standard/rfc2236/catalog.md#rfc2236-router-5) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-6](../../standard/rfc2236/catalog.md#rfc2236-router-6) | covered | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-7](../../standard/rfc2236/catalog.md#rfc2236-router-7) | covered | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-8](../../standard/rfc2236/catalog.md#rfc2236-router-8) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-9](../../standard/rfc2236/catalog.md#rfc2236-router-9) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-10](../../standard/rfc2236/catalog.md#rfc2236-router-10) | selected | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-11](../../standard/rfc2236/catalog.md#rfc2236-router-11) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-12](../../standard/rfc2236/catalog.md#rfc2236-router-12) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-13](../../standard/rfc2236/catalog.md#rfc2236-router-13) | selected | [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | `Rfc9776QuerierElection` | PASS | — |
| [RFC2236-ROUTER-14](../../standard/rfc2236/catalog.md#rfc2236-router-14) | covered | [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9776StartupQueries` | PASS | — |
| [RFC2236-ROUTER-15](../../standard/rfc2236/catalog.md#rfc2236-router-15) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-16](../../standard/rfc2236/catalog.md#rfc2236-router-16) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-17](../../standard/rfc2236/catalog.md#rfc2236-router-17) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-18](../../standard/rfc2236/catalog.md#rfc2236-router-18) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-19](../../standard/rfc2236/catalog.md#rfc2236-router-19) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-20](../../standard/rfc2236/catalog.md#rfc2236-router-20) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-ROUTER-21](../../standard/rfc2236/catalog.md#rfc2236-router-21) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-22](../../standard/rfc2236/catalog.md#rfc2236-router-22) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-23](../../standard/rfc2236/catalog.md#rfc2236-router-23) | later | — | — | — | level 3: needs a crafted message |
| [RFC2236-ROUTER-24](../../standard/rfc2236/catalog.md#rfc2236-router-24) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-25](../../standard/rfc2236/catalog.md#rfc2236-router-25) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-26](../../standard/rfc2236/catalog.md#rfc2236-router-26) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-27](../../standard/rfc2236/catalog.md#rfc2236-router-27) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-28](../../standard/rfc2236/catalog.md#rfc2236-router-28) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-29](../../standard/rfc2236/catalog.md#rfc2236-router-29) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-30](../../standard/rfc2236/catalog.md#rfc2236-router-30) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-31](../../standard/rfc2236/catalog.md#rfc2236-router-31) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-32](../../standard/rfc2236/catalog.md#rfc2236-router-32) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-33](../../standard/rfc2236/catalog.md#rfc2236-router-33) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-34](../../standard/rfc2236/catalog.md#rfc2236-router-34) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-35](../../standard/rfc2236/catalog.md#rfc2236-router-35) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-36](../../standard/rfc2236/catalog.md#rfc2236-router-36) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-37](../../standard/rfc2236/catalog.md#rfc2236-router-37) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-38](../../standard/rfc2236/catalog.md#rfc2236-router-38) | covered | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-39](../../standard/rfc2236/catalog.md#rfc2236-router-39) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-40](../../standard/rfc2236/catalog.md#rfc2236-router-40) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-41](../../standard/rfc2236/catalog.md#rfc2236-router-41) | owed | — | — | — | a later level 2 pass: two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host |
| [RFC2236-ROUTER-42](../../standard/rfc2236/catalog.md#rfc2236-router-42) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-43](../../standard/rfc2236/catalog.md#rfc2236-router-43) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-44](../../standard/rfc2236/catalog.md#rfc2236-router-44) | selected | [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | `Rfc2236LeaveAtV2Router` | PASS | — |
| [RFC2236-ROUTER-45](../../standard/rfc2236/catalog.md#rfc2236-router-45) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-46](../../standard/rfc2236/catalog.md#rfc2236-router-46) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-47](../../standard/rfc2236/catalog.md#rfc2236-router-47) | later | — | — | — | level 5: a version 1 system, RFC 1112 |
| [RFC2236-ROUTER-48](../../standard/rfc2236/catalog.md#rfc2236-router-48) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-49](../../standard/rfc2236/catalog.md#rfc2236-router-49) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-50](../../standard/rfc2236/catalog.md#rfc2236-router-50) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-51](../../standard/rfc2236/catalog.md#rfc2236-router-51) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-52](../../standard/rfc2236/catalog.md#rfc2236-router-52) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-53](../../standard/rfc2236/catalog.md#rfc2236-router-53) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |
| [RFC2236-ROUTER-54](../../standard/rfc2236/catalog.md#rfc2236-router-54) | owed | — | — | — | a later level 2 pass: an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine |

## The coverage debt: the checks this pass owes

Forty-nine statements that the model claims have no check yet. Each needs a normal exchange
only, in a larger mockup or with a second path of messages, so each is level 2 work for a later
pass. The closing list of
[`checks.md`](../../protocol/igmp/checks.md#statements-this-pass-wrote-no-check-for) holds the
same needs in the words of the standard.

| What the check needs | Statements |
| --- | --- |
| a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a Query Interval above 127 seconds, for the floating-point codes, and a Robustness Variable above 7 | RFC9776-QRY-4, QRY-13, QRY-17, TIMER-14, TIMER-15 |
| a second router that hears a Query with the S flag set | RFC9776-QRY-10, QRY-11, RQRY-4 |
| systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier | RFC9776-QRY-14, QRY-18, TIMER-1 |
| many sources or many groups, beyond the size of one message | RFC9776-QRY-21, REP-37, REP-38, REP-39 |
| a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions | RFC9776-REP-29, HOST-19, HOST-20 |
| a system that reports before it has an address | RFC9776-REP-32 |
| two specific Queries for one group within one Max Response Time, or two changes within one query period, and a second IGMPv2 member that answers the Query after a Leave, or a second Query within the delay of a host | RFC9776-HQRY-8, RREP-19, RFC2236-HOST-23, HOST-30, ROUTER-41 |
| a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router | RFC9776-HQRY-16, RST-5, RST-15, RST-16, RST-17, FWD-6, FWD-7, RREP-10, RREP-24, RREP-25, RREP-26, COMPR-30 |
| a run longer than the Group Membership Interval with an INCLUDE-mode member that answers the Queries | RFC9776-RREP-8 |
| the end of the IGMPv2 mode of a group after the Older Host Present Interval | RFC9776-COMPR-20, COMPR-21, TIMER-25 |
| an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine | RFC2236-ROUTER-29, ROUTER-37, ROUTER-48, ROUTER-49, ROUTER-50, ROUTER-51, ROUTER-52, ROUTER-53, ROUTER-54 |

Two groups of them will fail when their checks exist, from what the code shows: the forwarding
in EXCLUDE mode with blocked sources, RFC9776-FWD-6 and FWD-7, because the forwarding asks for
the group only ([gap 12](results.md#gap-12-defect--the-forwarding-asks-for-a-listener-of-the-group-not-of-the-source)),
and a non-querier that adopts the QRV and the QQIC, RFC9776-QRY-14, QRY-18 and TIMER-1, because
the Queries carry zero in both fields ([gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic)).

## Feature support

The rule of the guide, per feature: `supported` when every core check ran and passed,
`partial` when at least one passed and at least one failed or has no check, `not supported`
when every core check that ran failed, `untested` when no core check exists.

| Feature | Level | Support | Core statements that fail or have no check |
| --- | --- | --- | --- |
| [IGMP-F-MESSAGE-FORMAT](../../protocol/igmp/features.md#igmp-f-message-format) | mandatory | supported | — |
| [IGMP-F-QUERY-FORMAT](../../protocol/igmp/features.md#igmp-f-query-format) | mandatory | partial | RFC9776-QRY-12, QRY-15, QRY-16 fail ([gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic)); RFC9776-QRY-4, QRY-13, QRY-17 are `owed` |
| [IGMP-F-REPORT-FORMAT](../../protocol/igmp/features.md#igmp-f-report-format) | mandatory | supported | — |
| [IGMP-F-MESSAGE-VALIDATION](../../protocol/igmp/features.md#igmp-f-message-validation) | mandatory | untested | RFC9776-GEN-10, QRY-6, QRY-23, QRY-30, REP-3, REP-5, REP-15, REP-16, REP-30, REP-33, REP-36, VER-4, RFC2236-HOST-7, HOST-11, ROUTER-20, ROUTER-23 are `later` |
| [IGMP-F-STATE-CHANGE-REPORT](../../protocol/igmp/features.md#igmp-f-state-change-report) | mandatory | supported | — |
| [IGMP-F-REPORT-RETRANSMISSION](../../protocol/igmp/features.md#igmp-f-report-retransmission) | mandatory | partial | RFC9776-HOST-16, HOST-18, HOST-25 fail ([gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents), [gap 5](results.md#gap-5-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge)); RFC9776-HOST-21, HOST-24 are not reached ([gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents)) |
| [IGMP-F-QUERY-RESPONSE](../../protocol/igmp/features.md#igmp-f-query-response) | mandatory | partial | RFC9776-HQRY-10, HQRY-17 fail ([gap 8](results.md#gap-8-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out)); RFC9776-HQRY-16 is `owed` |
| [IGMP-F-GENERAL-QUERY](../../protocol/igmp/features.md#igmp-f-general-query) | mandatory | supported | — |
| [IGMP-F-QUERIER-ELECTION](../../protocol/igmp/features.md#igmp-f-querier-election) | mandatory | supported | — |
| [IGMP-F-GROUP-MEMBERSHIP](../../protocol/igmp/features.md#igmp-f-group-membership) | mandatory | partial | RFC9776-RREP-11, TIMER-8 fail ([gap 3](results.md#gap-3-defect--two-default-intervals-have-the-values-of-older-documents)); RFC9776-RST-14 is not reached ([gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers)); RFC9776-RREP-8, RREP-10 are `owed` |
| [IGMP-F-FORWARDING](../../protocol/igmp/features.md#igmp-f-forwarding) | mandatory | partial | RFC9776-FWD-5 fails ([gap 12](results.md#gap-12-defect--the-forwarding-asks-for-a-listener-of-the-group-not-of-the-source)); RFC9776-FWD-4 is not reached ([gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers)); RFC9776-FWD-6, FWD-7 are `owed` |
| [IGMP-F-STATE-CHANGE-PROCESSING](../../protocol/igmp/features.md#igmp-f-state-change-processing) | mandatory | partial | RFC9776-RREP-24, RREP-25, RREP-26 are `owed`; RFC9776-RREP-23 is `later` |
| [IGMP-F-SPECIFIC-QUERIES](../../protocol/igmp/features.md#igmp-f-specific-queries) | mandatory | partial | RFC9776-RQRY-11, RQRY-13 fail ([gap 9](results.md#gap-9-defect--the-s-flag-of-a-group-specific-query-comes-from-the-timer-before-it-is-lowered), [gap 10](results.md#gap-10-defect--a-report-cancels-the-retransmissions-of-the-queries)) |
| [IGMP-F-QUERY-TIMER-UPDATES](../../protocol/igmp/features.md#igmp-f-query-timer-updates) | mandatory | partial | RFC9776-RQRY-2 fails ([gap 11](results.md#gap-11-defect--a-group-and-source-specific-query-does-not-lower-the-source-timers)); RFC9776-RQRY-4, QRY-10 are `owed` |
| [IGMP-F-HOST-COMPATIBILITY](../../protocol/igmp/features.md#igmp-f-host-compatibility) | mandatory | partial | RFC9776-COMPH-22, TIMER-22 fail ([gap 4](results.md#gap-4-defect--the-igmpv2-mode-of-a-host-ends-after-the-other-querier-present-interval), [gap 7](results.md#gap-7-defect--the-igmpv2-mode-keeps-the-pending-igmpv3-retransmissions)); RFC9776-VER-1, COMPH-5, COMPH-8, COMPH-11, COMPH-18, COMPH-20 are `later` |
| [IGMP-F-VERSION-2-HOST](../../protocol/igmp/features.md#igmp-f-version-2-host) | mandatory | partial | RFC2236-HOST-21, HOST-22, HOST-28, HOST-29, HOST-31 fail ([gap 6](results.md#gap-6-defect--the-igmpv2-mode-of-a-host-answers-at-once-and-sends-one-report)); RFC2236-HOST-30 is `owed` |
| [IGMP-F-ROUTER-COMPATIBILITY](../../protocol/igmp/features.md#igmp-f-router-compatibility) | mandatory | partial | RFC9776-RQRY-8, COMPR-1 fail ([gap 13](results.md#gap-13-missing-feature--no-igmpv2-querier-mode-in-an-igmpv3-router)); RFC9776-COMPR-20, COMPR-30, TIMER-25 are `owed`; RFC9776-GEN-7, COMPR-16, COMPR-19, COMPR-31, COMPR-32, COMPR-33 are `later` |
| [IGMP-F-VERSION-2-ROUTER](../../protocol/igmp/features.md#igmp-f-version-2-router) | mandatory | partial | RFC2236-ROUTER-37, ROUTER-41 are `owed`; RFC2236-ROUTER-36, ROUTER-40, ROUTER-42, ROUTER-45, ROUTER-46, ROUTER-47 are `later` |
| [IGMP-F-SSM-AWARE](../../protocol/igmp/features.md#igmp-f-ssm-aware) | optional | untested | RFC9776-REP-21, REP-25, RREP-1, RREP-2, COMPH-27 are `later` |
| [IGMP-F-TIMER-CONFIGURATION](../../protocol/igmp/features.md#igmp-f-timer-configuration) | mandatory | partial | RFC9776-TIMER-3 fails ([gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic)); RFC9776-TIMER-1 is `owed` |

Five features are supported, thirteen partial, none not supported, and two untested. Each of
the thirteen partial features has at least one core check that passes; eleven of them also have
a core check that fails. The two untested features are IGMP-F-MESSAGE-VALIDATION, whose checks
all need a crafted message, and the optional IGMP-F-SSM-AWARE, which is level 5.

## Achieved level

**Level 2, reached.** Target: level 2, from
[`standards.md`](../../protocol/igmp/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold:

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of RFC 9776 §4 to §8 and of the host and router state diagrams of RFC 2236 §6 and §7, 387 entries; the feature map has 20 features and places every entry |
| Every mandatory feature has a core check that ran and has a verdict | holds for the normal path | 18 of the 19 mandatory features have core checks that ran: 42 tests, 24 PASS, 18 FAIL |

The one mandatory feature without a core check has no normal path, so the criterion does not
reach it at this level, and the ledger says so rather than count it: IGMP-F-MESSAGE-VALIDATION
is the silent discard of a message that fails a validity check, and every such message is a
crafted one, which is level 3.

A level is a measure of how deeply the pass looked, not of how well the model did. Eighteen
tests fail, and thirteen gaps of the model stand behind them; the level holds because each of
those checks ran.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 49 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes) |
| 3, Edge | not started | the catalog half holds; the checks need crafted messages: the validation of every message, the unknown types, an IGMPv2 Group-Specific Query at a host in IGMPv3 mode |
| 4, Dynamics | partial | the delays and intervals with a bound have a check with a stated window, but the random halves and the state inside a node (RFC9776-HOST-2, RQ-3, RQ-4) have no test |
| 5, Complete | not started | RFC 1112, which no module implements, and RFC 4604, the SSM-aware behavior |

Per feature, the level reached is level 2 for the eighteen features with a core check that ran,
level 1 for IGMP-F-MESSAGE-VALIDATION, whose checks are level 3, and level 1 for
IGMP-F-SSM-AWARE, whose checks are level 5.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 9776 and RFC 2236 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 3376, claimed at `Igmpv3.ned:13`); two RFC 9776 formula changes found relative to the model's defaults (Group Membership Interval, Older Version Querier Present Interval); the unrecognized-message-type rule went from a lower-case "should" to a formal MUST between RFC 2236 and RFC 9776, against code that already crashes on the case |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 9776 (293) and RFC 2236 (94); 20 features; 32 checks; 42 tests; the conformance matrix | 24 PASS, 18 FAIL, one of them declared expected; thirteen gaps of the model, twelve defects and one missing feature; 49 statements owed |

The read record of pass 1 named commit `e360ca980e` of the wave 0 branch; its trees, src
`16dc528e10` and tests/protocol `6f0a6bdb05`, identify the code it read.

## Out of scope

The areas that the catalogs leave out, and why, are at the end of each catalog:
[RFC 9776](../../standard/rfc9776/catalog.md#out-of-scope-in-this-catalog),
[RFC 2236](../../standard/rfc2236/catalog.md#out-of-scope-in-this-catalog). The documents
outside the in-scope set are in [`standards.md`](../../protocol/igmp/standards.md#in-scope-set).
