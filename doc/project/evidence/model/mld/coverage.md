# MLD — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc9777/catalog.md](../../standard/rfc9777/catalog.md), [rfc2710/catalog.md](../../standard/rfc2710/catalog.md), [features.md](../../protocol/mld/features.md), [results.md](results.md)

The single place that holds the changing state of the MLD workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from the run after the repairs of 2026-09-25 to 2026-09-28 (the level 2 run
of 2026-09-24 is in the [pass log](#pass-log)):

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-igmp-mld-level2-fixes`, commit `ddea7a391b`, tree clean
- Trees: src `e76d92f3af`, tests/protocol `36c0e0673d`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mld$'`
- Suite: 47 tests, 47 PASS, so the suite reports PASS
- Target level: 2

The analysis of every failure is in [`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a crafted message at level 3, the state of a node at level 4, an SSM-aware system at level 5 |
| `no check` | a permission that no observation can fail, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so.

| Status | RFC 9777 | RFC 2710 | Together |
| --- | --- | --- | --- |
| `selected` | 157 | 20 | 177 |
| `covered` | 68 | 34 | 102 |
| `owed` | 38 | 20 | 58 |
| `later` | 42 | 7 | 49 |
| `no check` | 2 | 0 | 2 |
| all | 307 | 81 | 388 |

### RFC 9777

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC9777-GEN-1](../../standard/rfc9777/catalog.md#rfc9777-gen-1) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-GEN-2](../../standard/rfc9777/catalog.md#rfc9777-gen-2) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportSource` | PASS | repaired, [gap 2](results.md#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| [RFC9777-GEN-3](../../standard/rfc9777/catalog.md#rfc9777-gen-3) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-GEN-4](../../standard/rfc9777/catalog.md#rfc9777-gen-4) | selected | [Router Alert on every message](../../protocol/mld/checks/message-format.md#router-alert-on-every-message) | `Rfc9777RouterAlert` | PASS | repaired, [gap 3](results.md#gap-3-defect--an-mld-message-carries-no-router-alert-option) |
| [RFC9777-GEN-5](../../standard/rfc9777/catalog.md#rfc9777-gen-5) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-GEN-6](../../standard/rfc9777/catalog.md#rfc9777-gen-6) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-GEN-7](../../standard/rfc9777/catalog.md#rfc9777-gen-7) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-GEN-8](../../standard/rfc9777/catalog.md#rfc9777-gen-8) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-1](../../standard/rfc9777/catalog.md#rfc9777-qry-1) | covered | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-2](../../standard/rfc9777/catalog.md#rfc9777-qry-2) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-3](../../standard/rfc9777/catalog.md#rfc9777-qry-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-4](../../standard/rfc9777/catalog.md#rfc9777-qry-4) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryChecksum` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-icmpv6-checksum-has-no-pseudo-header) |
| [RFC9777-QRY-5](../../standard/rfc9777/catalog.md#rfc9777-qry-5) | covered | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-6](../../standard/rfc9777/catalog.md#rfc9777-qry-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-7](../../standard/rfc9777/catalog.md#rfc9777-qry-7) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-8](../../standard/rfc9777/catalog.md#rfc9777-qry-8) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-9](../../standard/rfc9777/catalog.md#rfc9777-qry-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-10](../../standard/rfc9777/catalog.md#rfc9777-qry-10) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-11](../../standard/rfc9777/catalog.md#rfc9777-qry-11) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-QRY-12](../../standard/rfc9777/catalog.md#rfc9777-qry-12) | owed | — | — | — | a later level 2 pass: a second router that hears a Query with the S flag set |
| [RFC9777-QRY-13](../../standard/rfc9777/catalog.md#rfc9777-qry-13) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | repaired, [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9777-QRY-14](../../standard/rfc9777/catalog.md#rfc9777-qry-14) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9777-QRY-15](../../standard/rfc9777/catalog.md#rfc9777-qry-15) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | repaired, [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9777-QRY-16](../../standard/rfc9777/catalog.md#rfc9777-qry-16) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9777-QRY-17](../../standard/rfc9777/catalog.md#rfc9777-qry-17) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-18](../../standard/rfc9777/catalog.md#rfc9777-qry-18) | owed | — | — | — | a later level 2 pass: many sources or many addresses, beyond the size of one message |
| [RFC9777-QRY-19](../../standard/rfc9777/catalog.md#rfc9777-qry-19) | covered | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-20](../../standard/rfc9777/catalog.md#rfc9777-qry-20) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-21](../../standard/rfc9777/catalog.md#rfc9777-qry-21) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-22](../../standard/rfc9777/catalog.md#rfc9777-qry-22) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-23](../../standard/rfc9777/catalog.md#rfc9777-qry-23) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-24](../../standard/rfc9777/catalog.md#rfc9777-qry-24) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-QRY-25](../../standard/rfc9777/catalog.md#rfc9777-qry-25) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-QRY-26](../../standard/rfc9777/catalog.md#rfc9777-qry-26) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QuerySource` | PASS | repaired, [gap 2](results.md#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| [RFC9777-QRY-27](../../standard/rfc9777/catalog.md#rfc9777-qry-27) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-QRY-28](../../standard/rfc9777/catalog.md#rfc9777-qry-28) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-QRY-29](../../standard/rfc9777/catalog.md#rfc9777-qry-29) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-QRY-30](../../standard/rfc9777/catalog.md#rfc9777-qry-30) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-1](../../standard/rfc9777/catalog.md#rfc9777-rep-1) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-2](../../standard/rfc9777/catalog.md#rfc9777-rep-2) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-3](../../standard/rfc9777/catalog.md#rfc9777-rep-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-4](../../standard/rfc9777/catalog.md#rfc9777-rep-4) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportChecksum` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-icmpv6-checksum-has-no-pseudo-header) |
| [RFC9777-REP-5](../../standard/rfc9777/catalog.md#rfc9777-rep-5) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-6](../../standard/rfc9777/catalog.md#rfc9777-rep-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-7](../../standard/rfc9777/catalog.md#rfc9777-rep-7) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-8](../../standard/rfc9777/catalog.md#rfc9777-rep-8) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-9](../../standard/rfc9777/catalog.md#rfc9777-rep-9) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-REP-10](../../standard/rfc9777/catalog.md#rfc9777-rep-10) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-11](../../standard/rfc9777/catalog.md#rfc9777-rep-11) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-REP-12](../../standard/rfc9777/catalog.md#rfc9777-rep-12) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-REP-13](../../standard/rfc9777/catalog.md#rfc9777-rep-13) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-14](../../standard/rfc9777/catalog.md#rfc9777-rep-14) | covered | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-15](../../standard/rfc9777/catalog.md#rfc9777-rep-15) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-16](../../standard/rfc9777/catalog.md#rfc9777-rep-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-17](../../standard/rfc9777/catalog.md#rfc9777-rep-17) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-18](../../standard/rfc9777/catalog.md#rfc9777-rep-18) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-19](../../standard/rfc9777/catalog.md#rfc9777-rep-19) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-20](../../standard/rfc9777/catalog.md#rfc9777-rep-20) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-REP-21](../../standard/rfc9777/catalog.md#rfc9777-rep-21) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-REP-22](../../standard/rfc9777/catalog.md#rfc9777-rep-22) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryNoState` | PASS | repaired, [gap 11](results.md#gap-11-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out) |
| [RFC9777-REP-23](../../standard/rfc9777/catalog.md#rfc9777-rep-23) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-REP-24](../../standard/rfc9777/catalog.md#rfc9777-rep-24) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-REP-25](../../standard/rfc9777/catalog.md#rfc9777-rep-25) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-REP-26](../../standard/rfc9777/catalog.md#rfc9777-rep-26) | selected | [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | `Rfc9777StopReport` | PASS | — |
| [RFC9777-REP-27](../../standard/rfc9777/catalog.md#rfc9777-rep-27) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-REP-28](../../standard/rfc9777/catalog.md#rfc9777-rep-28) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-REP-29](../../standard/rfc9777/catalog.md#rfc9777-rep-29) | covered | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-REP-30](../../standard/rfc9777/catalog.md#rfc9777-rep-30) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-REP-31](../../standard/rfc9777/catalog.md#rfc9777-rep-31) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-REP-32](../../standard/rfc9777/catalog.md#rfc9777-rep-32) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9777-REP-33](../../standard/rfc9777/catalog.md#rfc9777-rep-33) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-34](../../standard/rfc9777/catalog.md#rfc9777-rep-34) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportSource` | PASS | repaired, [gap 2](results.md#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| [RFC9777-REP-35](../../standard/rfc9777/catalog.md#rfc9777-rep-35) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-36](../../standard/rfc9777/catalog.md#rfc9777-rep-36) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-37](../../standard/rfc9777/catalog.md#rfc9777-rep-37) | owed | — | — | — | a later level 2 pass: a node that reports before it has a link-local address |
| [RFC9777-REP-38](../../standard/rfc9777/catalog.md#rfc9777-rep-38) | owed | — | — | — | a later level 2 pass: a node that reports before it has a link-local address |
| [RFC9777-REP-39](../../standard/rfc9777/catalog.md#rfc9777-rep-39) | owed | — | — | — | a later level 2 pass: a node that reports before it has a link-local address |
| [RFC9777-REP-40](../../standard/rfc9777/catalog.md#rfc9777-rep-40) | selected | [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | `Rfc9777ReportEncapsulation` | PASS | — |
| [RFC9777-REP-41](../../standard/rfc9777/catalog.md#rfc9777-rep-41) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-REP-42](../../standard/rfc9777/catalog.md#rfc9777-rep-42) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-REP-43](../../standard/rfc9777/catalog.md#rfc9777-rep-43) | owed | — | — | — | a later level 2 pass: many sources or many addresses, beyond the size of one message |
| [RFC9777-REP-44](../../standard/rfc9777/catalog.md#rfc9777-rep-44) | owed | — | — | — | a later level 2 pass: many sources or many addresses, beyond the size of one message |
| [RFC9777-REP-45](../../standard/rfc9777/catalog.md#rfc9777-rep-45) | owed | — | — | — | a later level 2 pass: many sources or many addresses, beyond the size of one message |
| [RFC9777-LSN-1](../../standard/rfc9777/catalog.md#rfc9777-lsn-1) | covered | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-2](../../standard/rfc9777/catalog.md#rfc9777-lsn-2) | covered | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-3](../../standard/rfc9777/catalog.md#rfc9777-lsn-3) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777SolicitedNodeReport` | PASS | repaired, [gap 5](results.md#gap-5-defect--a-node-does-not-report-its-solicited-node-addresses); the half about ff02::1 holds in `Rfc9777StartReport` |
| [RFC9777-LSN-4](../../standard/rfc9777/catalog.md#rfc9777-lsn-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-LSN-5](../../standard/rfc9777/catalog.md#rfc9777-lsn-5) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-6](../../standard/rfc9777/catalog.md#rfc9777-lsn-6) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-7](../../standard/rfc9777/catalog.md#rfc9777-lsn-7) | covered | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-8](../../standard/rfc9777/catalog.md#rfc9777-lsn-8) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LSN-9](../../standard/rfc9777/catalog.md#rfc9777-lsn-9) | selected | [Change inside EXCLUDE mode](../../protocol/mld/checks/listener-reports.md#change-inside-exclude-mode) | `Rfc9777ExcludeChange` | PASS | — |
| [RFC9777-LSN-10](../../standard/rfc9777/catalog.md#rfc9777-lsn-10) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-11](../../standard/rfc9777/catalog.md#rfc9777-lsn-11) | selected | [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | `Rfc9777StopReport` | PASS | — |
| [RFC9777-LSN-12](../../standard/rfc9777/catalog.md#rfc9777-lsn-12) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LSN-13](../../standard/rfc9777/catalog.md#rfc9777-lsn-13) | selected | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeated` | PASS | repaired, [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-LSN-14](../../standard/rfc9777/catalog.md#rfc9777-lsn-14) | selected | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | `Rfc9777ChangeDuringRepetitions` | PASS | — |
| [RFC9777-LSN-15](../../standard/rfc9777/catalog.md#rfc9777-lsn-15) | selected | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | `Rfc9777ChangeDuringRepetitions` | PASS | repaired, [gap 8](results.md#gap-8-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9777-LSN-16](../../standard/rfc9777/catalog.md#rfc9777-lsn-16) | selected | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | `Rfc9777ChangeDuringRepetitions` | PASS | repaired, [gap 8](results.md#gap-8-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9777-LSN-17](../../standard/rfc9777/catalog.md#rfc9777-lsn-17) | selected | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | `Rfc9777ChangeDuringRepetitionsTail` | PASS | — |
| [RFC9777-LSN-18](../../standard/rfc9777/catalog.md#rfc9777-lsn-18) | selected | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeatedCount` | PASS | — |
| [RFC9777-LSN-19](../../standard/rfc9777/catalog.md#rfc9777-lsn-19) | covered | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeatedCount` | PASS | — |
| [RFC9777-LSN-20](../../standard/rfc9777/catalog.md#rfc9777-lsn-20) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9777-LSN-21](../../standard/rfc9777/catalog.md#rfc9777-lsn-21) | owed | — | — | — | a later level 2 pass: a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions |
| [RFC9777-LSN-22](../../standard/rfc9777/catalog.md#rfc9777-lsn-22) | selected | [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | `Rfc9777StopReport` | PASS | — |
| [RFC9777-LSN-23](../../standard/rfc9777/catalog.md#rfc9777-lsn-23) | selected | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-LSN-24](../../standard/rfc9777/catalog.md#rfc9777-lsn-24) | selected | [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | `Rfc9777ChangeDuringRepetitions` | PASS | repaired, [gap 8](results.md#gap-8-defect--a-second-change-replaces-the-pending-records-instead-of-a-merge) |
| [RFC9777-LSN-25](../../standard/rfc9777/catalog.md#rfc9777-lsn-25) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LQRY-1](../../standard/rfc9777/catalog.md#rfc9777-lqry-1) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-LQRY-2](../../standard/rfc9777/catalog.md#rfc9777-lqry-2) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LQRY-3](../../standard/rfc9777/catalog.md#rfc9777-lqry-3) | covered | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LQRY-4](../../standard/rfc9777/catalog.md#rfc9777-lqry-4) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LQRY-5](../../standard/rfc9777/catalog.md#rfc9777-lqry-5) | owed | — | — | — | a later level 2 pass: two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state |
| [RFC9777-LQRY-6](../../standard/rfc9777/catalog.md#rfc9777-lqry-6) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LQRY-7](../../standard/rfc9777/catalog.md#rfc9777-lqry-7) | selected | [Response to a Multicast Address Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-specific-query) | `Rfc9777AddressSpecificResponse` | PASS | — |
| [RFC9777-LQRY-8](../../standard/rfc9777/catalog.md#rfc9777-lqry-8) | owed | — | — | — | a later level 2 pass: two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state |
| [RFC9777-LQRY-9](../../standard/rfc9777/catalog.md#rfc9777-lqry-9) | owed | — | — | — | a later level 2 pass: two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state |
| [RFC9777-LTIM-1](../../standard/rfc9777/catalog.md#rfc9777-ltim-1) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryNoState` | PASS | repaired, [gap 11](results.md#gap-11-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out); observation 2 of `Rfc9777GeneralQueryResponse` holds |
| [RFC9777-LTIM-2](../../standard/rfc9777/catalog.md#rfc9777-ltim-2) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LTIM-3](../../standard/rfc9777/catalog.md#rfc9777-ltim-3) | covered | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LTIM-4](../../standard/rfc9777/catalog.md#rfc9777-ltim-4) | covered | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LTIM-5](../../standard/rfc9777/catalog.md#rfc9777-ltim-5) | selected | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-LTIM-6](../../standard/rfc9777/catalog.md#rfc9777-ltim-6) | selected | [Response to a Multicast Address Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-specific-query) | `Rfc9777AddressSpecificResponse` | PASS | — |
| [RFC9777-LTIM-7](../../standard/rfc9777/catalog.md#rfc9777-ltim-7) | selected | [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | `Rfc9777AddressSourceResponse` | PASS | — |
| [RFC9777-LTIM-8](../../standard/rfc9777/catalog.md#rfc9777-ltim-8) | selected | [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | `Rfc9777AddressSourceResponse` | PASS | — |
| [RFC9777-LTIM-9](../../standard/rfc9777/catalog.md#rfc9777-ltim-9) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-LTIM-10](../../standard/rfc9777/catalog.md#rfc9777-ltim-10) | selected | [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | `Rfc9777AddressSourceResponse` | PASS | repaired, [gap 11](results.md#gap-11-defect--an-answer-to-a-query-holds-a-record-that-the-standard-leaves-out) |
| [RFC9777-LTIM-11](../../standard/rfc9777/catalog.md#rfc9777-ltim-11) | covered | [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | `Rfc9777AddressSourceResponse` | PASS | — |
| [RFC9777-LTIM-12](../../standard/rfc9777/catalog.md#rfc9777-ltim-12) | selected | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeated` | PASS | reached since the repair of [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-LTIM-13](../../standard/rfc9777/catalog.md#rfc9777-ltim-13) | covered | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeatedCount` | PASS | — |
| [RFC9777-LTIM-14](../../standard/rfc9777/catalog.md#rfc9777-ltim-14) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LTIM-15](../../standard/rfc9777/catalog.md#rfc9777-ltim-15) | selected | [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | `Rfc9777StopReport` | PASS | — |
| [RFC9777-LTIM-16](../../standard/rfc9777/catalog.md#rfc9777-ltim-16) | selected | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeated` | PASS | reached since the repair of [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-LTIM-17](../../standard/rfc9777/catalog.md#rfc9777-ltim-17) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LTIM-18](../../standard/rfc9777/catalog.md#rfc9777-ltim-18) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-LTIM-19](../../standard/rfc9777/catalog.md#rfc9777-ltim-19) | selected | [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | `Rfc9777SourceListChange` | PASS | — |
| [RFC9777-RQ-1](../../standard/rfc9777/catalog.md#rfc9777-rq-1) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RQ-2](../../standard/rfc9777/catalog.md#rfc9777-rq-2) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RQ-3](../../standard/rfc9777/catalog.md#rfc9777-rq-3) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RQ-4](../../standard/rfc9777/catalog.md#rfc9777-rq-4) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RQ-5](../../standard/rfc9777/catalog.md#rfc9777-rq-5) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RQ-6](../../standard/rfc9777/catalog.md#rfc9777-rq-6) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RQ-7](../../standard/rfc9777/catalog.md#rfc9777-rq-7) | covered | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-RQ-8](../../standard/rfc9777/catalog.md#rfc9777-rq-8) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-RQ-9](../../standard/rfc9777/catalog.md#rfc9777-rq-9) | covered | [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | `Rfc9777GeneralQueryResponse` | PASS | — |
| [RFC9777-RQ-10](../../standard/rfc9777/catalog.md#rfc9777-rq-10) | covered | [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | `Rfc9777StartReport` | PASS | — |
| [RFC9777-RQ-11](../../standard/rfc9777/catalog.md#rfc9777-rq-11) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQ-12](../../standard/rfc9777/catalog.md#rfc9777-rq-12) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQ-13](../../standard/rfc9777/catalog.md#rfc9777-rq-13) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQ-14](../../standard/rfc9777/catalog.md#rfc9777-rq-14) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQ-15](../../standard/rfc9777/catalog.md#rfc9777-rq-15) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RQ-16](../../standard/rfc9777/catalog.md#rfc9777-rq-16) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RQ-17](../../standard/rfc9777/catalog.md#rfc9777-rq-17) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RST-1](../../standard/rfc9777/catalog.md#rfc9777-rst-1) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-2](../../standard/rfc9777/catalog.md#rfc9777-rst-2) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-3](../../standard/rfc9777/catalog.md#rfc9777-rst-3) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-4](../../standard/rfc9777/catalog.md#rfc9777-rst-4) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RST-5](../../standard/rfc9777/catalog.md#rfc9777-rst-5) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-6](../../standard/rfc9777/catalog.md#rfc9777-rst-6) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-7](../../standard/rfc9777/catalog.md#rfc9777-rst-7) | selected | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificBlocking` | PASS | repaired, [gap 15](results.md#gap-15-defect--the-forwarding-asks-for-a-listener-of-the-address-not-of-the-source); the forwarding of S1 holds in `Rfc9777SourceSpecificForwarding` |
| [RFC9777-RST-8](../../standard/rfc9777/catalog.md#rfc9777-rst-8) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RST-9](../../standard/rfc9777/catalog.md#rfc9777-rst-9) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RST-10](../../standard/rfc9777/catalog.md#rfc9777-rst-10) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RST-11](../../standard/rfc9777/catalog.md#rfc9777-rst-11) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RST-12](../../standard/rfc9777/catalog.md#rfc9777-rst-12) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-13](../../standard/rfc9777/catalog.md#rfc9777-rst-13) | covered | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-14](../../standard/rfc9777/catalog.md#rfc9777-rst-14) | covered | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-15](../../standard/rfc9777/catalog.md#rfc9777-rst-15) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-16](../../standard/rfc9777/catalog.md#rfc9777-rst-16) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RST-17](../../standard/rfc9777/catalog.md#rfc9777-rst-17) | covered | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-18](../../standard/rfc9777/catalog.md#rfc9777-rst-18) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-19](../../standard/rfc9777/catalog.md#rfc9777-rst-19) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RST-20](../../standard/rfc9777/catalog.md#rfc9777-rst-20) | covered | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RST-21](../../standard/rfc9777/catalog.md#rfc9777-rst-21) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RST-22](../../standard/rfc9777/catalog.md#rfc9777-rst-22) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RST-23](../../standard/rfc9777/catalog.md#rfc9777-rst-23) | covered | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RST-24](../../standard/rfc9777/catalog.md#rfc9777-rst-24) | owed | — | — | — | a later level 2 pass: a run longer than the Multicast Address Listening Interval with an INCLUDE-mode listener that answers the Queries |
| [RFC9777-RST-25](../../standard/rfc9777/catalog.md#rfc9777-rst-25) | selected | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | `Rfc9777SourceBlockedOnlyListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RST-26](../../standard/rfc9777/catalog.md#rfc9777-rst-26) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RST-27](../../standard/rfc9777/catalog.md#rfc9777-rst-27) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RST-28](../../standard/rfc9777/catalog.md#rfc9777-rst-28) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | reached since the repair of [gap 13](results.md#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9777-RST-29](../../standard/rfc9777/catalog.md#rfc9777-rst-29) | selected | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | `Rfc9777SourceBlockedOnlyListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RST-30](../../standard/rfc9777/catalog.md#rfc9777-rst-30) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-31](../../standard/rfc9777/catalog.md#rfc9777-rst-31) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-32](../../standard/rfc9777/catalog.md#rfc9777-rst-32) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-33](../../standard/rfc9777/catalog.md#rfc9777-rst-33) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-34](../../standard/rfc9777/catalog.md#rfc9777-rst-34) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-35](../../standard/rfc9777/catalog.md#rfc9777-rst-35) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RST-36](../../standard/rfc9777/catalog.md#rfc9777-rst-36) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-FWD-1](../../standard/rfc9777/catalog.md#rfc9777-fwd-1) | covered | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-FWD-2](../../standard/rfc9777/catalog.md#rfc9777-fwd-2) | no check | — | — | — | a permission that no observation can fail |
| [RFC9777-FWD-3](../../standard/rfc9777/catalog.md#rfc9777-fwd-3) | selected | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-FWD-4](../../standard/rfc9777/catalog.md#rfc9777-fwd-4) | selected | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | `Rfc9777SourceBlockedOnlyListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-FWD-5](../../standard/rfc9777/catalog.md#rfc9777-fwd-5) | selected | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificBlocking` | PASS | repaired, [gap 15](results.md#gap-15-defect--the-forwarding-asks-for-a-listener-of-the-address-not-of-the-source) |
| [RFC9777-FWD-6](../../standard/rfc9777/catalog.md#rfc9777-fwd-6) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-FWD-7](../../standard/rfc9777/catalog.md#rfc9777-fwd-7) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-FWD-8](../../standard/rfc9777/catalog.md#rfc9777-fwd-8) | selected | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RREP-1](../../standard/rfc9777/catalog.md#rfc9777-rrep-1) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-RREP-2](../../standard/rfc9777/catalog.md#rfc9777-rrep-2) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-RREP-3](../../standard/rfc9777/catalog.md#rfc9777-rrep-3) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-RREP-4](../../standard/rfc9777/catalog.md#rfc9777-rrep-4) | later | — | — | — | level 4: state inside the node |
| [RFC9777-RREP-5](../../standard/rfc9777/catalog.md#rfc9777-rrep-5) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RREP-6](../../standard/rfc9777/catalog.md#rfc9777-rrep-6) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RREP-7](../../standard/rfc9777/catalog.md#rfc9777-rrep-7) | covered | [Stop of listening with another listener](../../protocol/mld/checks/router-state.md#stop-of-listening-with-another-listener) | `Rfc9777StopWithAnotherListener` | PASS | — |
| [RFC9777-RREP-8](../../standard/rfc9777/catalog.md#rfc9777-rrep-8) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RREP-9](../../standard/rfc9777/catalog.md#rfc9777-rrep-9) | selected | [Stop of listening with another listener](../../protocol/mld/checks/router-state.md#stop-of-listening-with-another-listener) | `Rfc9777StopWithAnotherListener` | PASS | — |
| [RFC9777-RREP-10](../../standard/rfc9777/catalog.md#rfc9777-rrep-10) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RREP-11](../../standard/rfc9777/catalog.md#rfc9777-rrep-11) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | reached since the repair of [gap 13](results.md#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9777-RREP-12](../../standard/rfc9777/catalog.md#rfc9777-rrep-12) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RREP-13](../../standard/rfc9777/catalog.md#rfc9777-rrep-13) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RREP-14](../../standard/rfc9777/catalog.md#rfc9777-rrep-14) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RREP-15](../../standard/rfc9777/catalog.md#rfc9777-rrep-15) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RREP-16](../../standard/rfc9777/catalog.md#rfc9777-rrep-16) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RREP-17](../../standard/rfc9777/catalog.md#rfc9777-rrep-17) | selected | [A router that comes up after a start of listening](../../protocol/mld/checks/router-state.md#a-router-that-comes-up-after-a-start-of-listening) | `Rfc9777RouterAfterStart` | PASS | — |
| [RFC9777-RREP-18](../../standard/rfc9777/catalog.md#rfc9777-rrep-18) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RREP-19](../../standard/rfc9777/catalog.md#rfc9777-rrep-19) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | repaired, [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-RREP-20](../../standard/rfc9777/catalog.md#rfc9777-rrep-20) | selected | [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | `Rfc9777SourceSpecificForwarding` | PASS | — |
| [RFC9777-RREP-21](../../standard/rfc9777/catalog.md#rfc9777-rrep-21) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-RREP-22](../../standard/rfc9777/catalog.md#rfc9777-rrep-22) | selected | [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | `Rfc9777ForwardingAfterStart` | PASS | — |
| [RFC9777-RREP-23](../../standard/rfc9777/catalog.md#rfc9777-rrep-23) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-RREP-24](../../standard/rfc9777/catalog.md#rfc9777-rrep-24) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RREP-25](../../standard/rfc9777/catalog.md#rfc9777-rrep-25) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RREP-26](../../standard/rfc9777/catalog.md#rfc9777-rrep-26) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RREP-27](../../standard/rfc9777/catalog.md#rfc9777-rrep-27) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RSW-1](../../standard/rfc9777/catalog.md#rfc9777-rsw-1) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RSW-2](../../standard/rfc9777/catalog.md#rfc9777-rsw-2) | owed | — | — | — | a later level 2 pass: a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router |
| [RFC9777-RSW-3](../../standard/rfc9777/catalog.md#rfc9777-rsw-3) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | — |
| [RFC9777-RQRY-1](../../standard/rfc9777/catalog.md#rfc9777-rqry-1) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-RQRY-2](../../standard/rfc9777/catalog.md#rfc9777-rqry-2) | selected | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | `Rfc9777SourceBlockedOnlyListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RQRY-3](../../standard/rfc9777/catalog.md#rfc9777-rqry-3) | selected | [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | `Rfc9777SourceBlockedOnlyListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RQRY-4](../../standard/rfc9777/catalog.md#rfc9777-rqry-4) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQRY-5](../../standard/rfc9777/catalog.md#rfc9777-rqry-5) | owed | — | — | — | a later level 2 pass: a second router that hears a Query with the S flag set |
| [RFC9777-RQRY-6](../../standard/rfc9777/catalog.md#rfc9777-rqry-6) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC9777-RQRY-7](../../standard/rfc9777/catalog.md#rfc9777-rqry-7) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-RQRY-8](../../standard/rfc9777/catalog.md#rfc9777-rqry-8) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC9777-RQRY-9](../../standard/rfc9777/catalog.md#rfc9777-rqry-9) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC9777-RQRY-10](../../standard/rfc9777/catalog.md#rfc9777-rqry-10) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QuerySource` | PASS | repaired, [gap 2](results.md#gap-2-defect--an-mld-message-leaves-from-a-global-address) |
| [RFC9777-RQRY-11](../../standard/rfc9777/catalog.md#rfc9777-rqry-11) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC9777-RQRY-12](../../standard/rfc9777/catalog.md#rfc9777-rqry-12) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQRY-13](../../standard/rfc9777/catalog.md#rfc9777-rqry-13) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-RQRY-14](../../standard/rfc9777/catalog.md#rfc9777-rqry-14) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuerySFlag` | PASS | repaired, [gap 12](results.md#gap-12-defect--the-s-flag-of-a-multicast-address-specific-query-comes-from-the-timer-before-it-is-lowered) |
| [RFC9777-RQRY-15](../../standard/rfc9777/catalog.md#rfc9777-rqry-15) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | repaired, [gap 14](results.md#gap-14-defect--a-multicast-address-and-source-specific-query-does-not-lower-the-source-timers) |
| [RFC9777-RQRY-16](../../standard/rfc9777/catalog.md#rfc9777-rqry-16) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | repaired, [gap 13](results.md#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9777-RQRY-17](../../standard/rfc9777/catalog.md#rfc9777-rqry-17) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceQuerySFlag` | PASS | the S flag of the second Query holds too since the repair of [gap 13](results.md#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9777-RQRY-18](../../standard/rfc9777/catalog.md#rfc9777-rqry-18) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceQuerySFlag` | PASS | the S flag of the second Query holds too since the repair of [gap 13](results.md#gap-13-defect--a-report-cancels-the-retransmissions-of-the-queries) |
| [RFC9777-RQRY-19](../../standard/rfc9777/catalog.md#rfc9777-rqry-19) | selected | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceQuerySFlag` | PASS | — |
| [RFC9777-RQRY-20](../../standard/rfc9777/catalog.md#rfc9777-rqry-20) | covered | [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | `Rfc9777SourceBlockedOtherListener` | PASS | — |
| [RFC9777-VER-1](../../standard/rfc9777/catalog.md#rfc9777-ver-1) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-VER-2](../../standard/rfc9777/catalog.md#rfc9777-ver-2) | later | — | — | — | level 3: needs a crafted message |
| [RFC9777-COMPL-1](../../standard/rfc9777/catalog.md#rfc9777-compl-1) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-2](../../standard/rfc9777/catalog.md#rfc9777-compl-2) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-3](../../standard/rfc9777/catalog.md#rfc9777-compl-3) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-4](../../standard/rfc9777/catalog.md#rfc9777-compl-4) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-5](../../standard/rfc9777/catalog.md#rfc9777-compl-5) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-6](../../standard/rfc9777/catalog.md#rfc9777-compl-6) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-7](../../standard/rfc9777/catalog.md#rfc9777-compl-7) | selected | [Listener back to MLDv2](../../protocol/mld/checks/compatibility.md#listener-back-to-mldv2) | `Rfc9777ListenerBackToV2` | PASS | — |
| [RFC9777-COMPL-8](../../standard/rfc9777/catalog.md#rfc9777-compl-8) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-9](../../standard/rfc9777/catalog.md#rfc9777-compl-9) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-10](../../standard/rfc9777/catalog.md#rfc9777-compl-10) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-COMPL-11](../../standard/rfc9777/catalog.md#rfc9777-compl-11) | selected | [A mode change cancels the pending reports](../../protocol/mld/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | `Rfc9777ModeChangeCancels` | PASS | repaired, [gap 10](results.md#gap-10-defect--the-mldv1-mode-keeps-the-pending-mldv2-retransmissions) |
| [RFC9777-COMPL-12](../../standard/rfc9777/catalog.md#rfc9777-compl-12) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-COMPL-13](../../standard/rfc9777/catalog.md#rfc9777-compl-13) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-COMPL-14](../../standard/rfc9777/catalog.md#rfc9777-compl-14) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-COMPL-15](../../standard/rfc9777/catalog.md#rfc9777-compl-15) | no check | — | — | — | a permission that no observation can fail |
| [RFC9777-COMPR-1](../../standard/rfc9777/catalog.md#rfc9777-compr-1) | selected | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | repaired, [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-2](../../standard/rfc9777/catalog.md#rfc9777-compr-2) | covered | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | repaired, [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-3](../../standard/rfc9777/catalog.md#rfc9777-compr-3) | selected | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | repaired, [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-4](../../standard/rfc9777/catalog.md#rfc9777-compr-4) | selected | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | repaired, [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-5](../../standard/rfc9777/catalog.md#rfc9777-compr-5) | selected | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | repaired, [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-6](../../standard/rfc9777/catalog.md#rfc9777-compr-6) | later | — | — | — | level 4: state inside the node |
| [RFC9777-COMPR-7](../../standard/rfc9777/catalog.md#rfc9777-compr-7) | selected | [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | `Rfc9777QuerierConfiguredV1` | PASS | reached since the repair of [gap 17](results.md#gap-17-missing-feature--no-mldv1-mode-in-an-mldv2-router) |
| [RFC9777-COMPR-8](../../standard/rfc9777/catalog.md#rfc9777-compr-8) | later | — | — | — | level 4: state inside the node |
| [RFC9777-COMPR-9](../../standard/rfc9777/catalog.md#rfc9777-compr-9) | later | — | — | — | level 4: state inside the node |
| [RFC9777-COMPR-10](../../standard/rfc9777/catalog.md#rfc9777-compr-10) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-COMPR-11](../../standard/rfc9777/catalog.md#rfc9777-compr-11) | later | — | — | — | level 5: an SSM-aware system, RFC 4604 |
| [RFC9777-COMPR-12](../../standard/rfc9777/catalog.md#rfc9777-compr-12) | selected | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-13](../../standard/rfc9777/catalog.md#rfc9777-compr-13) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-14](../../standard/rfc9777/catalog.md#rfc9777-compr-14) | selected | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-15](../../standard/rfc9777/catalog.md#rfc9777-compr-15) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-16](../../standard/rfc9777/catalog.md#rfc9777-compr-16) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-17](../../standard/rfc9777/catalog.md#rfc9777-compr-17) | owed | — | — | — | a later level 2 pass: the end of the MLDv1 mode of an address after the Older Version Host Present Interval |
| [RFC9777-COMPR-18](../../standard/rfc9777/catalog.md#rfc9777-compr-18) | owed | — | — | — | a later level 2 pass: the end of the MLDv1 mode of an address after the Older Version Host Present Interval |
| [RFC9777-COMPR-19](../../standard/rfc9777/catalog.md#rfc9777-compr-19) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-20](../../standard/rfc9777/catalog.md#rfc9777-compr-20) | selected | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-21](../../standard/rfc9777/catalog.md#rfc9777-compr-21) | selected | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-22](../../standard/rfc9777/catalog.md#rfc9777-compr-22) | selected | [A BLOCK record for an address in MLDv1 mode](../../protocol/mld/checks/compatibility.md#a-block-record-for-an-address-in-mldv1-mode) | `Rfc9777BlockInV1Mode` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-COMPR-23](../../standard/rfc9777/catalog.md#rfc9777-compr-23) | selected | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-TIMER-1](../../standard/rfc9777/catalog.md#rfc9777-timer-1) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC9777-TIMER-2](../../standard/rfc9777/catalog.md#rfc9777-timer-2) | covered | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeatedCount` | PASS | — |
| [RFC9777-TIMER-3](../../standard/rfc9777/catalog.md#rfc9777-timer-3) | selected | [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | `Rfc9777QueryRobustness` | PASS | repaired, [gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic) |
| [RFC9777-TIMER-4](../../standard/rfc9777/catalog.md#rfc9777-timer-4) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-TIMER-5](../../standard/rfc9777/catalog.md#rfc9777-timer-5) | selected | [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | `Rfc9777QueryEncapsulation` | PASS | — |
| [RFC9777-TIMER-6](../../standard/rfc9777/catalog.md#rfc9777-timer-6) | selected | [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | `Rfc9777QueryTimerRelations` | PASS | — |
| [RFC9777-TIMER-7](../../standard/rfc9777/catalog.md#rfc9777-timer-7) | selected | [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | `Rfc9777ListeningTimeout` | PASS | repaired, [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-TIMER-8](../../standard/rfc9777/catalog.md#rfc9777-timer-8) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC9777-TIMER-9](../../standard/rfc9777/catalog.md#rfc9777-timer-9) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-TIMER-10](../../standard/rfc9777/catalog.md#rfc9777-timer-10) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC9777-TIMER-11](../../standard/rfc9777/catalog.md#rfc9777-timer-11) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-TIMER-12](../../standard/rfc9777/catalog.md#rfc9777-timer-12) | covered | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-TIMER-13](../../standard/rfc9777/catalog.md#rfc9777-timer-13) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-TIMER-14](../../standard/rfc9777/catalog.md#rfc9777-timer-14) | selected | [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `Rfc9777LastListenerQuery` | PASS | — |
| [RFC9777-TIMER-15](../../standard/rfc9777/catalog.md#rfc9777-timer-15) | selected | [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | `Rfc9777StartRepeated` | PASS | repaired, [gap 6](results.md#gap-6-defect--two-default-intervals-have-the-values-of-older-documents) |
| [RFC9777-TIMER-16](../../standard/rfc9777/catalog.md#rfc9777-timer-16) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC9777-TIMER-17](../../standard/rfc9777/catalog.md#rfc9777-timer-17) | selected | [Listener back to MLDv2](../../protocol/mld/checks/compatibility.md#listener-back-to-mldv2) | `Rfc9777OlderQuerierInterval` | PASS | repaired, [gap 7](results.md#gap-7-defect--the-mldv1-mode-of-a-host-ends-after-the-other-querier-present-interval) |
| [RFC9777-TIMER-18](../../standard/rfc9777/catalog.md#rfc9777-timer-18) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | reached since the repair of [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC9777-TIMER-19](../../standard/rfc9777/catalog.md#rfc9777-timer-19) | owed | — | — | — | a later level 2 pass: the end of the MLDv1 mode of an address after the Older Version Host Present Interval |
| [RFC9777-TIMER-20](../../standard/rfc9777/catalog.md#rfc9777-timer-20) | selected | [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | `Rfc9777QueryTimerRelations` | PASS | — |

### RFC 2710

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC2710-NODE-1](../../standard/rfc2710/catalog.md#rfc2710-node-1) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-2](../../standard/rfc2710/catalog.md#rfc2710-node-2) | covered | [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | `Rfc9777RouterV1Listener` | PASS | repaired, [gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query) |
| [RFC2710-NODE-3](../../standard/rfc2710/catalog.md#rfc2710-node-3) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-4](../../standard/rfc2710/catalog.md#rfc2710-node-4) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-5](../../standard/rfc2710/catalog.md#rfc2710-node-5) | owed | — | — | — | a later level 2 pass: a node that hears an MLDv1 Query for an address it does not listen to, and the scope rules of MLDv1 for reserved, node-local and link-local addresses |
| [RFC2710-NODE-6](../../standard/rfc2710/catalog.md#rfc2710-node-6) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-NODE-7](../../standard/rfc2710/catalog.md#rfc2710-node-7) | covered | [Report suppression in MLDv1 mode](../../protocol/mld/checks/compatibility.md#report-suppression-in-mldv1-mode) | `Rfc2710ReportSuppression` | PASS | reached since the repair of [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-8](../../standard/rfc2710/catalog.md#rfc2710-node-8) | covered | [Report suppression in MLDv1 mode](../../protocol/mld/checks/compatibility.md#report-suppression-in-mldv1-mode) | `Rfc2710ReportSuppression` | PASS | reached since the repair of [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-9](../../standard/rfc2710/catalog.md#rfc2710-node-9) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-NODE-10](../../standard/rfc2710/catalog.md#rfc2710-node-10) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-11](../../standard/rfc2710/catalog.md#rfc2710-node-11) | covered | [Done in MLDv1 mode](../../protocol/mld/checks/compatibility.md#done-in-mldv1-mode) | `Rfc2710DoneInV1Mode` | PASS | — |
| [RFC2710-NODE-12](../../standard/rfc2710/catalog.md#rfc2710-node-12) | selected | [Done in MLDv1 mode](../../protocol/mld/checks/compatibility.md#done-in-mldv1-mode) | `Rfc2710DoneInV1Mode` | PASS | — |
| [RFC2710-NODE-13](../../standard/rfc2710/catalog.md#rfc2710-node-13) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | repaired, [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-14](../../standard/rfc2710/catalog.md#rfc2710-node-14) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1ModeRepeat` | PASS | repaired, [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-15](../../standard/rfc2710/catalog.md#rfc2710-node-15) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-16](../../standard/rfc2710/catalog.md#rfc2710-node-16) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-17](../../standard/rfc2710/catalog.md#rfc2710-node-17) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-18](../../standard/rfc2710/catalog.md#rfc2710-node-18) | owed | — | — | — | a later level 2 pass: two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state |
| [RFC2710-NODE-19](../../standard/rfc2710/catalog.md#rfc2710-node-19) | selected | [Done in MLDv1 mode](../../protocol/mld/checks/compatibility.md#done-in-mldv1-mode) | `Rfc2710DoneInV1Mode` | PASS | — |
| [RFC2710-NODE-20](../../standard/rfc2710/catalog.md#rfc2710-node-20) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | repaired, [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-21](../../standard/rfc2710/catalog.md#rfc2710-node-21) | selected | [Report suppression in MLDv1 mode](../../protocol/mld/checks/compatibility.md#report-suppression-in-mldv1-mode) | `Rfc2710ReportSuppression` | PASS | repaired, [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-22](../../standard/rfc2710/catalog.md#rfc2710-node-22) | selected | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | repaired, [gap 9](results.md#gap-9-defect--the-mldv1-mode-of-a-host-answers-at-once-and-sends-one-report) |
| [RFC2710-NODE-23](../../standard/rfc2710/catalog.md#rfc2710-node-23) | owed | — | — | — | a later level 2 pass: two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state |
| [RFC2710-NODE-24](../../standard/rfc2710/catalog.md#rfc2710-node-24) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-NODE-25](../../standard/rfc2710/catalog.md#rfc2710-node-25) | owed | — | — | — | a later level 2 pass: a node that hears an MLDv1 Query for an address it does not listen to, and the scope rules of MLDv1 for reserved, node-local and link-local addresses |
| [RFC2710-NODE-26](../../standard/rfc2710/catalog.md#rfc2710-node-26) | owed | — | — | — | a later level 2 pass: a node that hears an MLDv1 Query for an address it does not listen to, and the scope rules of MLDv1 for reserved, node-local and link-local addresses |
| [RFC2710-ROUTER-1](../../standard/rfc2710/catalog.md#rfc2710-router-1) | covered | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-2](../../standard/rfc2710/catalog.md#rfc2710-router-2) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-ROUTER-3](../../standard/rfc2710/catalog.md#rfc2710-router-3) | covered | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC2710-ROUTER-4](../../standard/rfc2710/catalog.md#rfc2710-router-4) | covered | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-5](../../standard/rfc2710/catalog.md#rfc2710-router-5) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC2710-ROUTER-6](../../standard/rfc2710/catalog.md#rfc2710-router-6) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC2710-ROUTER-7](../../standard/rfc2710/catalog.md#rfc2710-router-7) | selected | [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | `Rfc9777StartupQueries` | PASS | — |
| [RFC2710-ROUTER-8](../../standard/rfc2710/catalog.md#rfc2710-router-8) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-9](../../standard/rfc2710/catalog.md#rfc2710-router-9) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-10](../../standard/rfc2710/catalog.md#rfc2710-router-10) | selected | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-11](../../standard/rfc2710/catalog.md#rfc2710-router-11) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-12](../../standard/rfc2710/catalog.md#rfc2710-router-12) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-13](../../standard/rfc2710/catalog.md#rfc2710-router-13) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-ROUTER-14](../../standard/rfc2710/catalog.md#rfc2710-router-14) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-ROUTER-15](../../standard/rfc2710/catalog.md#rfc2710-router-15) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-16](../../standard/rfc2710/catalog.md#rfc2710-router-16) | later | — | — | — | level 3: needs a crafted message |
| [RFC2710-ROUTER-17](../../standard/rfc2710/catalog.md#rfc2710-router-17) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-18](../../standard/rfc2710/catalog.md#rfc2710-router-18) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-19](../../standard/rfc2710/catalog.md#rfc2710-router-19) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-20](../../standard/rfc2710/catalog.md#rfc2710-router-20) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-21](../../standard/rfc2710/catalog.md#rfc2710-router-21) | selected | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-22](../../standard/rfc2710/catalog.md#rfc2710-router-22) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-23](../../standard/rfc2710/catalog.md#rfc2710-router-23) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-24](../../standard/rfc2710/catalog.md#rfc2710-router-24) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-25](../../standard/rfc2710/catalog.md#rfc2710-router-25) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-26](../../standard/rfc2710/catalog.md#rfc2710-router-26) | selected | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-27](../../standard/rfc2710/catalog.md#rfc2710-router-27) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-28](../../standard/rfc2710/catalog.md#rfc2710-router-28) | selected | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-29](../../standard/rfc2710/catalog.md#rfc2710-router-29) | selected | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-30](../../standard/rfc2710/catalog.md#rfc2710-router-30) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-31](../../standard/rfc2710/catalog.md#rfc2710-router-31) | selected | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-ROUTER-32](../../standard/rfc2710/catalog.md#rfc2710-router-32) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-33](../../standard/rfc2710/catalog.md#rfc2710-router-33) | covered | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-ROUTER-34](../../standard/rfc2710/catalog.md#rfc2710-router-34) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-35](../../standard/rfc2710/catalog.md#rfc2710-router-35) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-36](../../standard/rfc2710/catalog.md#rfc2710-router-36) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-37](../../standard/rfc2710/catalog.md#rfc2710-router-37) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-38](../../standard/rfc2710/catalog.md#rfc2710-router-38) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-ROUTER-39](../../standard/rfc2710/catalog.md#rfc2710-router-39) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-TIMER-1](../../standard/rfc2710/catalog.md#rfc2710-timer-1) | owed | — | — | — | a later level 2 pass: systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier |
| [RFC2710-TIMER-2](../../standard/rfc2710/catalog.md#rfc2710-timer-2) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-TIMER-3](../../standard/rfc2710/catalog.md#rfc2710-timer-3) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-TIMER-4](../../standard/rfc2710/catalog.md#rfc2710-timer-4) | later | — | — | — | level 4: state inside the node |
| [RFC2710-TIMER-5](../../standard/rfc2710/catalog.md#rfc2710-timer-5) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-TIMER-6](../../standard/rfc2710/catalog.md#rfc2710-timer-6) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-TIMER-7](../../standard/rfc2710/catalog.md#rfc2710-timer-7) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-TIMER-8](../../standard/rfc2710/catalog.md#rfc2710-timer-8) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-TIMER-9](../../standard/rfc2710/catalog.md#rfc2710-timer-9) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |
| [RFC2710-TIMER-10](../../standard/rfc2710/catalog.md#rfc2710-timer-10) | covered | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-TIMER-11](../../standard/rfc2710/catalog.md#rfc2710-timer-11) | covered | [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | `Rfc9777QuerierElection` | PASS | — |
| [RFC2710-TIMER-12](../../standard/rfc2710/catalog.md#rfc2710-timer-12) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-TIMER-13](../../standard/rfc2710/catalog.md#rfc2710-timer-13) | covered | [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | `Rfc9777ListenerV1Mode` | PASS | — |
| [RFC2710-TIMER-14](../../standard/rfc2710/catalog.md#rfc2710-timer-14) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-TIMER-15](../../standard/rfc2710/catalog.md#rfc2710-timer-15) | covered | [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | `Rfc2710DoneAtV1Router` | PASS | — |
| [RFC2710-TIMER-16](../../standard/rfc2710/catalog.md#rfc2710-timer-16) | owed | — | — | — | a later level 2 pass: an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node |

## The coverage debt: the checks this pass owes

Fifty-eight statements that the model claims have no check yet. Each needs a normal exchange
only, in a larger mockup or with a second path of messages, so each is level 2 work for a later
pass. The closing list of
[`checks.md`](../../protocol/mld/checks.md#statements-this-pass-wrote-no-check-for) holds the
same needs in the words of the standard.

| What the check needs | Statements |
| --- | --- |
| a second router that hears a Query with the S flag set | RFC9777-QRY-12, RQRY-5 |
| systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the querier | RFC9777-QRY-14, QRY-16, TIMER-1, RFC2710-TIMER-1 |
| many sources or many addresses, beyond the size of one message | RFC9777-QRY-18, REP-43, REP-44, REP-45 |
| a change that allows and blocks sources at once, and a filter-mode change followed by a source change within the repetitions | RFC9777-REP-32, LSN-20, LSN-21 |
| a node that reports before it has a link-local address | RFC9777-REP-37, REP-38, REP-39 |
| two specific Queries for one address within one Maximum Response Delay, or a Query that meets a pending response, and a stop of listening in the Delaying Listener state | RFC9777-LQRY-5, LQRY-8, LQRY-9, RFC2710-NODE-18, NODE-23 |
| a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state of the router | RFC9777-LTIM-9, RST-12, RST-30, RST-31, RST-32, RST-33, RST-34, RST-35, RST-36, FWD-6, FWD-7, RREP-18, RREP-24, RREP-25, RREP-26, RSW-2 |
| a run longer than the Multicast Address Listening Interval with an INCLUDE-mode listener that answers the Queries | RFC9777-RST-24 |
| the end of the MLDv1 mode of an address after the Older Version Host Present Interval | RFC9777-COMPR-17, COMPR-18, TIMER-19 |
| a node that hears an MLDv1 Query for an address it does not listen to, and the scope rules of MLDv1 for reserved, node-local and link-local addresses | RFC2710-NODE-5, NODE-25, NODE-26 |
| an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine, and the Unsolicited Report Interval of an MLDv1-only node | RFC2710-ROUTER-17, ROUTER-25, ROUTER-27, ROUTER-30, ROUTER-32, ROUTER-34, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39, TIMER-8, TIMER-9, TIMER-16 |

In the level 2 run, three groups of them would have failed from what the code showed: the
forwarding in EXCLUDE mode with blocked sources, RFC9777-FWD-6 and FWD-7, because the forwarding
asked for the address only ([gap 15](results.md#gap-15-defect--the-forwarding-asks-for-a-listener-of-the-address-not-of-the-source));
a non-querier that adopts the QRV and the QQIC, RFC9777-QRY-14 and QRY-16, because the Queries
carried zero in both fields ([gap 1](results.md#gap-1-defect--the-queries-carry-no-qrv-and-no-qqic));
and the MLDv1 checks with an MLDv2 querier on the link, because the MLDv1 node stopped the
simulation on its Queries ([gap 16](results.md#gap-16-defect--the-mldv1-node-stops-the-simulation-on-an-mldv2-query)).
The repairs of the three gaps remove that expectation; the checks are still owed.

## Feature support

The rule of the guide, per feature: `supported` when every core check ran and passed,
`partial` when at least one passed and at least one failed or has no check, `not supported`
when every core check that ran failed, `untested` when no core check exists.

| Feature | Level | Support | Core statements that fail or have no check |
| --- | --- | --- | --- |
| [MLD-F-MESSAGE-FORMAT](../../protocol/mld/features.md#mld-f-message-format) | mandatory | supported | — |
| [MLD-F-QUERY-FORMAT](../../protocol/mld/features.md#mld-f-query-format) | mandatory | supported | — |
| [MLD-F-REPORT-FORMAT](../../protocol/mld/features.md#mld-f-report-format) | mandatory | supported | — |
| [MLD-F-MESSAGE-VALIDATION](../../protocol/mld/features.md#mld-f-message-validation) | mandatory | partial | RFC9777-GEN-8, QRY-6, QRY-20, QRY-21, QRY-27, QRY-30, REP-6, REP-16, REP-17, REP-18, REP-33, REP-35, REP-42, VER-2, LQRY-1, RREP-1, RQRY-1, RFC2710-NODE-6, ROUTER-2, ROUTER-13, ROUTER-14, ROUTER-16 are `later` |
| [MLD-F-STATE-CHANGE-REPORT](../../protocol/mld/features.md#mld-f-state-change-report) | mandatory | supported | — |
| [MLD-F-REPORT-RETRANSMISSION](../../protocol/mld/features.md#mld-f-report-retransmission) | mandatory | partial | RFC9777-LSN-20, LSN-21 are `owed` |
| [MLD-F-QUERY-RESPONSE](../../protocol/mld/features.md#mld-f-query-response) | mandatory | partial | RFC9777-LQRY-5, LTIM-9 are `owed` |
| [MLD-F-GENERAL-QUERY](../../protocol/mld/features.md#mld-f-general-query) | mandatory | supported | — |
| [MLD-F-QUERIER-ELECTION](../../protocol/mld/features.md#mld-f-querier-election) | mandatory | supported | — |
| [MLD-F-LISTENING-STATE](../../protocol/mld/features.md#mld-f-listening-state) | mandatory | partial | RFC9777-RREP-18, RSW-2 are `owed` |
| [MLD-F-FORWARDING](../../protocol/mld/features.md#mld-f-forwarding) | mandatory | partial | RFC9777-FWD-6, FWD-7 are `owed` |
| [MLD-F-STATE-CHANGE-PROCESSING](../../protocol/mld/features.md#mld-f-state-change-processing) | mandatory | partial | RFC9777-RREP-24, RREP-25, RREP-26 are `owed`; RFC9777-RREP-23 is `later` |
| [MLD-F-SPECIFIC-QUERIES](../../protocol/mld/features.md#mld-f-specific-queries) | mandatory | supported | — |
| [MLD-F-QUERY-TIMER-UPDATES](../../protocol/mld/features.md#mld-f-query-timer-updates) | mandatory | partial | RFC9777-RQRY-5, QRY-12 are `owed` |
| [MLD-F-LISTENER-COMPATIBILITY](../../protocol/mld/features.md#mld-f-listener-compatibility) | mandatory | supported | — |
| [MLD-F-VERSION-1-LISTENER](../../protocol/mld/features.md#mld-f-version-1-listener) | mandatory | partial | RFC2710-NODE-18, NODE-23 are `owed` |
| [MLD-F-ROUTER-COMPATIBILITY](../../protocol/mld/features.md#mld-f-router-compatibility) | mandatory | partial | RFC9777-COMPR-17, TIMER-19 are `owed` |
| [MLD-F-VERSION-1-ROUTER](../../protocol/mld/features.md#mld-f-version-1-router) | mandatory | partial | RFC2710-ROUTER-27, ROUTER-30, ROUTER-32, ROUTER-34, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39 are `owed` |
| [MLD-F-SSM-AWARE](../../protocol/mld/features.md#mld-f-ssm-aware) | optional | untested | RFC9777-REP-24, REP-28, RREP-2, RREP-3 are `later` |
| [MLD-F-TIMER-CONFIGURATION](../../protocol/mld/features.md#mld-f-timer-configuration) | mandatory | partial | RFC9777-TIMER-1, RFC2710-TIMER-1 are `owed`; RFC2710-TIMER-4 is `later` |

Eight features are supported, eleven partial, none not supported, and one untested. No core
check fails since the repairs; each of the eleven partial features has a core statement that has
no check yet. In the level 2 run, two features were supported, fifteen partial and two not
supported: MLD-F-MESSAGE-VALIDATION, whose one core statement with a verdict, RFC2710-NODE-2,
failed on gap 16, and MLD-F-ROUTER-COMPATIBILITY, whose core statements failed on the missing
MLDv1 mode (gap 17) or were not reached because gap 16 stopped the run. The untested feature is
the optional MLD-F-SSM-AWARE, which is level 5.

## Achieved level

**Level 2, reached.** Target: level 2, from
[`standards.md`](../../protocol/mld/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold:

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of RFC 9777 §5 to §9 and of the node and router state diagrams of RFC 2710 §5 to §7, 388 entries; the feature map has 20 features and places every entry |
| Every mandatory feature has a core check that ran and has a verdict | holds | the 19 mandatory features have core checks that ran: 47 tests, 47 PASS |

The verdict of MLD-F-MESSAGE-VALIDATION comes from its one core statement on the normal path,
RFC2710-NODE-2; its other statements need a crafted message, level 3.

A level is a measure of how deeply the pass looked, not of how well the model did. In the level
2 run twenty-six tests failed, with seventeen gaps of the model behind them, and the level held
because each of those checks ran; three of them stopped at the start on gap 16, so the router
half of the MLDv1 interoperation had statements without a verdict. The repairs closed all
seventeen gaps, and those statements have their verdicts now.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 58 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes); and a repair of gap 16 for the statements that the older-host checks do not reach |
| 3, Edge | not started | the catalog half holds; the checks need crafted messages: the validation of every message, the unknown types |
| 4, Dynamics | partial | the delays and intervals with a bound have a check with a stated window, but the random halves and the state inside a node (RFC9777-RQ-1, RQ-5, RQ-6, RST-10, RST-16, RST-19) have no test |
| 5, Complete | not started | RFC 4604, the SSM-aware behavior, and the other documents of the register |

Per feature, the level reached is level 2 for the nineteen features with a core check that ran,
and level 1 for MLD-F-SSM-AWARE, whose checks are level 5.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 9777 and RFC 2710 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 3810, claimed at `Mldv2.ned:12`); one RFC 9777 formula change found relative to the model's default (Multicast Address Listening Interval, shared with the same gap in IGMP); the `Mldv2.ned` "Parity gaps" comment found stale on all three of its own points, not the one already known; `Mldv1` still crashes on an unrecognized message type where `Mldv2` already discards it silently |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 9777 (307) and RFC 2710 (81); 20 features; 32 checks; 47 tests; the conformance matrix | 21 PASS, 26 FAIL, one of them declared expected; seventeen gaps of the model, sixteen defects and one missing feature; 58 statements owed |
| 2, repairs | 2026-09-25 to 2026-09-28 | **2, reached** | the seventeen gaps repaired on `topic/standards-tests-igmp-mld-level2-fixes`; five tests with a wrong premise repaired | 47 PASS; no gap left; 58 statements owed; 8 features supported, 11 partial |

The read record of pass 1 named commit `e360ca980e` of the wave 0 branch; its trees, src
`16dc528e10` and tests/protocol `6f0a6bdb05`, identify the code it read.

## Out of scope

The areas that the catalogs leave out, and why, are at the end of each catalog:
[RFC 9777](../../standard/rfc9777/catalog.md#out-of-scope-in-this-catalog),
[RFC 2710](../../standard/rfc2710/catalog.md#out-of-scope-in-this-catalog). The documents
outside the in-scope set are in [`standards.md`](../../protocol/mld/standards.md#in-scope-set).
