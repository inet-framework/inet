# DTLS extension execution record

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [extension obligations](../../standard/rfc9147/extensions.md), [implementation plan](../../../../../plan/pending/dtls-extensions.md)

The user authorized execution of plan revision 5 on 2026-11-16. Base: `cb1691184444afa64b1819348d446d4cad610474`, branch `feat/dtls-extensions`. The unrelated local `AGENTS.md` edit is excluded. This record distinguishes selected future behavior from implemented and executed evidence. Existing baseline results remain valid only for their recorded revisions.

## Finite model decisions

| Profile | Suite code | Hash bytes | Tag bytes | Explicit nonce bytes |
| --- | --- | --- | --- | --- |
| 1.3 AES-128-GCM/SHA-256 | 0x1301 | 32 | 16 | 0 |
| 1.3 AES-256-GCM/SHA-384 | 0x1302 | 48 | 16 | 0 |
| 1.3 ChaCha20-Poly1305/SHA-256 | 0x1303 | 32 | 16 | 0 |
| 1.2 PSK-AES-128-GCM/SHA-256 | 0x00a8 | 32 | 16 | 8 |
| 1.2 ECDHE-ECDSA-AES-128-GCM/SHA-256 | 0xc02b | 32 | 16 | 8 |

Group encodings are X25519 (0x001d, 32 bytes) and uncompressed secp256r1 (0x0017, 65 bytes). Signature schemes are ECDSA-P256/SHA-256 (0x0403, configured modeled size) and RSA-PSS-RSAE/SHA-256 (0x0804, 256 bytes for the selected RSA-2048 label). Finished is the selected hash length in 1.3 and 12 bytes in 1.2. Certificate bodies remain configured modeled identity bytes, not DER.

The current conservative model caps protected records at 2^24 and initiates an update no later than 2^23 records. Keep those limits explicitly for all selected suites: they are below the GCM 2^24.5 bound and conservative for ChaCha20. Keep the existing send-epoch cap 2^48−1 and never enforce that cap on receive. These model limits are selected policy, not computed security validation. Version 1.2 closes rather than sending KeyUpdate. Cryptographic failed-authentication counters cannot be validated by this abstraction and are not claimed.

Default-off capabilities preserve the existing wire profile. Session capacity is 64, lifetime 1 hour, one ticket per enabled association; early allowance is 0 unless enabled, then 4096 application bytes. Opaque ticket identities combine issuer component ID and lifetime serial. A consumed flag is retained in the session row; eviction makes the identity unknown. CIDs use eight bytes and at most four local receive IDs. PHA admits one locally initiated request, only on certificate or ticket authentication, and reports authentication success at the server.

Discovery starts with a provisional 512-byte UDP budget; this deliberate small-MTU model choice differs from RFC 8899's recommended IPv4 base. Search steps are 128 bytes, maximum consecutive failures three, probe timeout 16 seconds, confirmation interval 60 seconds and raise interval 600 seconds. Initial handshakes cannot discover a path below the base. The final-flight retention for 1.2 is 240 seconds, independent of the default 120-second handshake deadline.

## Requirement-to-commit and test inventory

All named new tests below are **NOT_IMPLEMENTED / NOT_RUN** until their owning stage records a result. Protocol files are relative to `tests/protocol/dtls/` unless stated otherwise. Every behavior stage owns the reviewer document specified by the plan.

| Stage | Obligations | Direct evidence | State |
| --- | --- | --- | --- |
| C1 | All selected obligations and exclusions | Primary-source/errata reconciliation, profile table, links | DONE, documentation only, commit 4756b0cfeb |
| C2 | EXCHANGE | DtlsPostHandshakeExchange_1 and three module controls; five production KeyUpdate controls | DONE: fresh debug build, 4 module PASS, 5 protocol PASS |
| C3 | TICKET | Eight production ticket cases, session lifecycle module, five serializer/store/selection/reassembly units | DONE: fresh debug build; 8 ticket protocol, 5 unit, 5 module PASS; 7 existing protocol controls PASS before the final timeout-only adjustment |
| C4 | EARLY, ADMISSION | Eleven explicit early-data protocol cases; store/offer units; engine lifecycle outcomes | DONE: fresh debug build; 24 combined protocol, 4 unit, 6 module PASS |
| C5 | PROFILE | Eight profile/retry/hash/cost cases; extended negotiation/serializer/store units | DONE: fresh debug build; 37 combined protocol, 6 unit, 6 module PASS |
| C6 | CID | Eight negotiation/budget/isolation/lifecycle cases; record/dissector unit; allocator module | DONE: fresh debug build; 45 combined protocol, 6 unit, 7 module PASS |
| C7 | CID-REQUEST, EXCHANGE | Fourteen CID update/request protocol cases; serializer unit; existing controls | DONE: fresh debug build; 59 combined protocol, 7 unit, 7 module PASS |
| C8 | HEARTBEAT | Thirteen heartbeat protocol cases; serializer unit; selected existing controls | DONE: fresh debug build; 22 combined protocol, 8 unit, 7 module PASS |
| C9 | PATH | Seventeen migration cases; NAT translation, switched/routed IPv4 and routed IPv6, rebind/error checks | DONE: 60 protocol, eight unit and seven module cases pass in debug |
| C10 | PHA | Rfc9147PostHandshakeAuth | NOT_STARTED |
| C11a | Existing fragmentation-request API | tests/protocol/ipv6/Ipv6DontFragmentRequest; source-fragmentation control | NOT_STARTED |
| C11 | PMTU | Rfc9147PathMtu; flight/path units | NOT_STARTED |
| C12 | 12-RECORD | Dtls12RecordSerializer_1, Dtls12HandshakeSerializer_1; unsupported-version control | NOT_STARTED |
| C13 | VERSION, 12-RECOVERY | Rfc6347Handshake, Recovery, Datagrams; Rfc9147VersionNegotiation | NOT_STARTED |
| C14 | 12-SESSION | Rfc6347Resumption | NOT_STARTED |
| C15 | 12-PATH | Rfc9146ConnectionId, Rfc6347PathMtu | NOT_STARTED |
| C16 | Integration/documentation | Owning cases repeated, deterministic examples, both build modes | NOT_STARTED |

## Validation

### C2 focused evidence

Working directory: `/home/miguel/workspace/inet-dtls-extensions`. Debug library freshly rebuilt after adding the helper and refreshing Makefiles. All commands exited zero; the new module pins seed 0, and existing controls retain their own deterministic fixtures. Code and tests match the C2 implementation boundary. Documentation-only additions to this record do not change the tested library.

```sh
make -j8 MODE=debug
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(PostHandshakeExchange|RecoveryLifecycle|RestartLifecycle|SenderState)_1\.test$' --log-file audit/dtls-extensions/c2/module-runner.log
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(KeyUpdate|SimultaneousUpdates|UpdatePeerIsolation|UpdateRequestGuard|PeerRequestActiveProtection)\.test$' --log-file audit/dtls-extensions/c2/protocol-runner.log
```

Four module cases and five protocol cases passed. Logs, including `build-debug.log`, `module-console.log` and `protocol-console.log`, are under `audit/dtls-extensions/c2/`; generated cases are in the standard test work directories. Scoped DTLS architecture and naming checks pass. Seal-index and C1 commit checks pass. Existing overloaded-virtual compiler warnings occur in test support; they are not test failures.

No exact-parent runtime comparison, release build or baseline comparison has completed. Controls are source-grounded preservation checks rather than measured parent failures. Raw standards/errata retrieval artifacts are under `audit/dtls-extensions/standards/`; ignored audit directories are local supporting evidence rather than repository-relative documentation links.

### C3 focused evidence

Working directory and debug mode match C2. A fresh debug build followed the final ticket timeout/admission correction. The seven ticket cases, five units and five modules below all pass with nonzero selection and exit zero. Ticket production fixtures pin seed 0 and declared checksums/FCS; module controls retain their own fixtures. The reviewer document is [resumption.md](../../../../../tests/protocol/dtls/resumption.md).

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(PskResumption|Resumption.*)\.test$' --log-file audit/dtls-extensions/c3-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(DtlsSessionStore|DtlsTicketSelection|TlsNewSessionTicket|TlsHandshakeParameters|DtlsHandshakeReassembly)_1\.test$' --log-file audit/dtls-extensions/c3-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(SessionLifecycle|PostHandshakeExchange|RecoveryLifecycle|RestartLifecycle|SenderState)_1\.test$' --log-file audit/dtls-extensions/c3-final-module-runner.log
```

Before the final correction, seven existing protocol controls also passed: `Rfc9147(Handshake|PskHandshake|ClientAuthentication|KeyUpdate|SimultaneousUpdates|UpdatePeerIsolation|SharedPortClose)\.test$`. Their run additionally included the new issuer-isolation case, giving eight executed cases. The final adjustment changes the newly enabled ticket's failure path; the seven ticket cases and module controls above were rerun afterward. Raw logs are `c3-controls-{runner,console}.log`, `c3-final-{protocol,unit,module}-{runner,console}.log` and `c3-build-debug.log` under `audit/dtls-extensions/`.

One additional ticket case, `Rfc9147ResumptionReceiveDeadline.test`, passes using the same protocol command with its exact filename selector. Logs are `c3-receive-deadline-{runner,console}.log`. It starts one second of logical ticket input service against a 0.5 s receive deadline and verifies that the later continuation cannot create a resumable row. Together the two runs execute eight new ticket cases.

Scoped DTLS/TLS/application architecture checks and DTLS/TLS naming checks pass. The global interface gate fails on 16 classes in unchanged files; `IDtls` is not reported. This remains a repository-wide gate failure, not a passing final gate. No source seal is modified. Release-mode behavior, exact-parent runtime comparison, fingerprints, general queued-admission fault injection and alternate-hash production negotiation remain unrun; the latter belongs to C5. No skill package scripts exist in this checkout, so the supplied skill-source packaging commands cannot run here.

### C4 focused evidence

A fresh debug library includes explicit early-open options, separate send/delivery/status operations, epoch-1 replay state, bounded single-use ticket admission and callback cleanup. All capabilities remain off by default. The final combined protocol run executes 24 cases and passes: eleven new early-data cases, eight ticket cases and five existing controls (full certificate/PSK, initial client authentication, KeyUpdate and shared-port close). The callback-driven failed attempt, queued Finished boundary and unrelated-peer limit checks are production-path evidence. The [reviewer document](../../../../../tests/protocol/dtls/early-data.md) provides each fixture and observation.

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(EarlyData.*|Resumption.*|PskResumption|Handshake|PskHandshake|ClientAuthentication|KeyUpdate|SharedPortClose)\.test$' --log-file audit/dtls-extensions/c4-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(DtlsSessionStore|TlsHandshakeParameters|TlsNewSessionTicket|DtlsTicketSelection)_1\.test$' --log-file audit/dtls-extensions/c4-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c4-final-module-runner.log
```

Artifacts are `c4-build-debug.log` and `c4-final-{protocol,unit,module}-{runner,console}.log` under `audit/dtls-extensions/`. The protocol run uses declared checksums/FCS, seed 0 and finite limits; helper/module cases retain their specified fixtures. Scoped architecture/naming, source protection, document seals and release-note format checks pass. The existing global interface-gate failures remain unresolved. Release-mode execution, exact-parent runtime comparison, fingerprint comparison and alternate-suite early compatibility remain unrun. Stored expectations have not changed.

The final helper run passes four units, and the final module run passes six cases against the same fresh library. The engine lifecycle case observes exactly one negative early outcome on each stopped/crashed pending attempt. Module fixture compiler warnings about pre-existing overloaded virtual methods remain warnings, with no executed-case failures.

### C5 focused evidence

A fresh debug library includes the finite catalog, ordered preferences, negotiated selection and sizes, independent initial-client signature selection, hash-filtered ticket cache, same-hash resumption, original-suite early admission and per-suite processing costs. Pre-negotiation ClientHello/HRR and detached-alert work use base costs; early protection uses its saved ticket suite. The [profile reviewer document](../../../../../tests/protocol/dtls/algorithm-profiles.md) states the finite encodings, observations and limits.

The final combined protocol run executes **37 cases, all PASS** with exit zero: eight new profile/retry/hash/cost cases plus 29 previous early-data/ticket/handshake/size/retry/cost controls. Every production fixture pins seed 0, declared checksums/FCS and a finite simulation limit. Exact selectors:

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(Algorithm.*|Sha384.*|EarlyData.*|Resumption.*|PskResumption|Handshake|PskHandshake|ClientAuthentication|KeyUpdate|SharedPortClose|CookieKeyShareRetry|UnsupportedSuite|UnofferedSelection|FullDuplexCosts|KeyUpdateInputCost)\.test$' --log-file audit/dtls-extensions/c5-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(TlsHandshakeParameters|TlsHandshakeSerializer|DtlsRecordSerializer|DtlsTicketSelection|DtlsSessionStore|TlsNewSessionTicket)_1\.test$' --log-file audit/dtls-extensions/c5-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c5-final-module-runner.log
```

Raw artifacts are `c5-build-debug.log` and `c5-final-{protocol,unit,module}-{runner,console}.log` under `audit/dtls-extensions/`. Scoped DTLS/TLS architecture/naming, source protection, document seals, release-note formatting and DTLS documentation links pass. The global link checker reports missing IEEE standards files in unchanged Wi-Fi evidence; the existing global interface gate also remains failing. These are not passing publication gates. Release execution, exact-parent comparison and baseline comparisons remain unrun; no stored expectation changed.

The final helper run passes all six selected units and the final module run passes all six controls against the final debug library, after the retry-selection check was added. Both runners exit zero with nonzero selection.

### C6 focused evidence

The fresh debug library includes directional CID negotiation, actual header serialization/accounting, local lookup and cleanup, monotonic allocation and explicit changed-tuple refusal. Native outgoing dissection accepts a different peer CID length; raw receiving uses the local eight-byte profile. A queued early record whose header reservation changes finishes its sampled service and is rejected before handoff. The [CID reviewer document](../../../../../tests/protocol/dtls/connection-ids.md) describes every fixture and the limits.

The final combined protocol run passes **45 cases** with exit zero: eight new CID cases plus the 37 C5 cases. Five directional profiles include a SHA-384/RSA association with initial client authentication. Negative records reuse the rejected numbers to verify replay windows directly; restart and delayed old-CID cases check non-reuse. The final build also preserves the previous default printer text for records without a CID and keeps ticket-fragment budget subtraction signed.

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(ConnectionId.*|Algorithm.*|Sha384.*|EarlyData.*|Resumption.*|PskResumption|Handshake|PskHandshake|ClientAuthentication|KeyUpdate|SharedPortClose|CookieKeyShareRetry|UnsupportedSuite|UnofferedSelection|FullDuplexCosts|KeyUpdateInputCost)\.test$' --log-file audit/dtls-extensions/c6-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(TlsHandshakeParameters|TlsHandshakeSerializer|DtlsRecordSerializer|DtlsTicketSelection|DtlsSessionStore|TlsNewSessionTicket)_1\.test$' --log-file audit/dtls-extensions/c6-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(ConnectionIds|EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c6-final-module-runner.log
```

Artifacts are `c6-build-debug.log` and `c6-final-{protocol,unit,module}-{runner,console}.log` under `audit/dtls-extensions/`. Production cases pin seed 0, declared checksums/FCS and finite limits. Scoped architecture/naming, source protection, document seals, release formatting and DTLS documentation links pass. Existing global interface/link gate failures remain outside this change. Release, exact-parent and baseline comparisons remain unrun; no stored expectation changed. Migration remains unavailable until C9.

The final six-unit and seven-module runs also pass with exit zero and nonzero selection against the final matching library. The allocator module is helper evidence; the real restart/close cases provide lifecycle and wire observations.

## C7 execution

Implemented protected NewConnectionId/RequestConnectionId bodies, directional admission, bounded spare/current ID choices, immediate replacement, independent recovery and separate request ACK/fulfillment state. Ticket output now uses the shared post-handshake fragment pump while preserving its default timer names and error reasons. Completed CID input remains reserved through logical input service; fresh-numbered duplicates share one logical cost. Retained old-epoch counters are pruned against all live recovery owners. Header-width changes reject affected application output before handoff. The [reviewer document](../../../../../tests/protocol/dtls/connection-id-updates.md) owns finite capacity, timeout, sequence-gap and epoch-retention limits.

Final combined validation passes 59 protocol cases, seven serializer/selection/store units and seven module controls against a fresh matching debug library, with exit zero and nonzero selection. Fourteen new protocol cases cover loss of both message types and ACKs, fragmentation at 48 bytes, duplicate costs/conflicts, old-ID retirement, ticket/KeyUpdate overlap, excessive requests, capacity-one empty replies, unnegotiated messages, malformed bodies, both request timeout states, late fulfillment and update expiry.

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(ConnectionId.*|Algorithm.*|Sha384.*|EarlyData.*|Resumption.*|PskResumption|Handshake|PskHandshake|ClientAuthentication|KeyUpdate|SharedPortClose|CookieKeyShareRetry|UnsupportedSuite|UnofferedSelection|FullDuplexCosts|KeyUpdateInputCost)\.test$' --log-file audit/dtls-extensions/c7-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(TlsHandshakeParameters|TlsHandshakeSerializer|DtlsRecordSerializer|DtlsTicketSelection|DtlsSessionStore|TlsNewSessionTicket|DtlsConnectionIdUpdateSerializer)_1\.test$' --log-file audit/dtls-extensions/c7-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(ConnectionIds|EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c7-final-module-runner.log
```

Artifacts are `c7-build-debug.log` and `c7-final-{protocol,unit,module}-{runner,console}.log` under `audit/dtls-extensions/`. Scoped engine/contract architecture and naming, source protection, document seals, release formatting and DTLS documentation links pass. The C++ gate cannot run because `clang-tidy` is absent from PATH (`c7-cpp.log`); this is not a pass. The previously recorded global interface/link failures remain outside this change. Release-mode execution, exact-parent trajectory checks and baseline comparisons remain unrun; stored expectations are unchanged. The new pure virtual CID operations have an incompatible release note and migration instructions for custom engines.

## C8 execution

Implemented opt-in ClientHello/EncryptedExtensions heartbeat negotiation, directional modes, typed body serialization and protected request/reply processing. The association owns one lazy probe slot and timers. Requests correlate an eight-byte lifetime token, retry with fresh record numbers, and stop at a finite deadline. Responses echo actual payload, discard request padding and fit the configured UDP budget. Local output refusal and expiry preserve ordinary data. The [reviewer document](../../../../../tests/protocol/dtls/heartbeat-probes.md) owns profile limits and the explicit departure from the standard's connection-termination recommendation. Its test-only subclass exposes the actual internal primitive without providing substitute protocol behavior.

A fresh debug build passes **22 protocol cases**: thirteen heartbeat cases and nine existing disabled-feature/interaction controls. Eight units and seven module controls pass with exit zero and nonzero selection against the same library. Direct checks include exact queue/link costs, a 1,200-byte request with a 57-byte response from a 512-byte-budget peer, malformed lengths, negotiation modes, wrong/fresh-numbered unsolicited responses, retries/expiry, queue bounds, expired active work, shared-server progress, close/reconnect and SHA-384 certificate-origin resumption with CID and early data. The resumed fixture explicitly uses a 32-byte modeled certificate to fit the no-cookie amplification bound. PHA suspension and active-probe stop/crash execution remain later work.

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(Heartbeat.*|Handshake|PskHandshake|ClientAuthentication|EarlyData|Resumption|KeyUpdate|ConnectionId|ConnectionIdEarlyData|ConnectionIdUpdateOverlap)\.test$' --log-file audit/dtls-extensions/c8-final-protocol-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(TlsHandshakeParameters|TlsHandshakeSerializer|DtlsRecordSerializer|DtlsTicketSelection|DtlsSessionStore|TlsNewSessionTicket|DtlsConnectionIdUpdateSerializer|DtlsHeartbeatSerializer)_1\.test$' --log-file audit/dtls-extensions/c8-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(ConnectionIds|EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c8-final-module-runner.log
```

Artifacts are `c8-build-debug.log` and `c8-final-{protocol,unit,module}-{runner,console}.log` under `audit/dtls-extensions/`. Scoped DTLS/TLS architecture and naming, source protection, document seals, release formatting and selected DTLS/standard documentation links pass. The missing C++ checker and prior global interface/link findings remain unresolved. Release, exact-parent and fingerprint comparisons remain unrun. No stored expectation changed.

## C9 execution

Implemented one candidate and one confirmed path per association, replay-window freshness checks before acceptance, correlated return validation, bounded candidate replies and confirmed-path ordinary output. Local source/interface selection reserves a peer spare CID. Explicit single-association socket rebind uses a fresh UDP socket ID, holds application output without charging completed service twice, and rejects stale transport indications. Rebind validation failure retires the association; ordinary candidate timeout preserves the confirmed path. Retained-key validation allows lost KeyUpdate ACK recovery. The [reviewer document](../../../../../tests/protocol/dtls/migration.md) explains configuration, observations, API outcomes and the finite IPv6 limitation.

A fresh debug library passes **60 distinct protocol cases** across the combined 58-case selection and two additional routed cases. The 17 new migration cases cover NAT, switched and routed IPv4, routed IPv6, ordering/freshness, competing candidates, permission and malformed-record refusal, timeout, real port replacement, occupied-port error, multiple-peer refusal, cancelled diagnostic probes and KeyUpdate recovery. Eight units and seven module controls also pass with nonzero selection and exit zero. The occupied-port case expects simulation exit 1 inside the test harness and passes as an expected configuration-error check.

```sh
make -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147(Migration.*|Heartbeat.*|ConnectionId.*|Handshake|PskHandshake|ClientAuthentication|EarlyData|Resumption|KeyUpdate|Multihomed|PlainAlertLocalIdentity)\.test$' --log-file audit/dtls-extensions/c9-final-protocol-runner.log
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147MigrationIpv6\.test$' --log-file audit/dtls-extensions/c9-ipv6-runner.log
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent -w '^tests/protocol/dtls$' -f 'Rfc9147MigrationRouted\.test$' --log-file audit/dtls-extensions/c9-routed-runner.log
opp_run_unit_tests --load . -p inet -m debug --no-build -f '(TlsHandshakeParameters|TlsHandshakeSerializer|DtlsRecordSerializer|DtlsTicketSelection|DtlsSessionStore|TlsNewSessionTicket|DtlsConnectionIdUpdateSerializer|DtlsHeartbeatSerializer)_1\.test$' --log-file audit/dtls-extensions/c9-final-unit-runner.log
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(ConnectionIds|EngineLifecycle|SessionLifecycle|RecoveryLifecycle|RestartLifecycle|SenderState|PostHandshakeExchange)_1\.test$' --log-file audit/dtls-extensions/c9-final-module-runner.log
```

The combined selector executed before the two routed cases were added; rerunning it now selects 60 cases. Raw build, runner and console logs have the above prefixes under `audit/dtls-extensions/`. Scoped engine/contract/application architecture, DTLS naming, source protection, document seals, release-note formatting and DTLS documentation links pass. The C++ checker remains unavailable because `clang-tidy` is absent; prior global interface/link findings remain unresolved. Release, exact-parent, fingerprint and active-validation stop/crash checks remain unrun. PHA and PMTU interactions belong to their later stages. No stored expectation changed, and the unrelated `AGENTS.md` edit remains excluded.
