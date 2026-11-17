# Minimum DTLS path budget admission

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [legacy paths](dtls12-paths.md), [plan](../../../plan/pending/dtls-extensions.md)

C17, `dtls: fix: reject path bases below the enabled heartbeat minimum`, repairs configuration admission. A DTLS 1.2 engine previously accepted a 50-byte PMTU base and established real peers, but could not send its confirmation heartbeat. Every subsequent attempt requested the same impossible size. This prevented confirmation and upward search even on a fitting path.

The probe body contains a three-byte header, an eight-byte correlation token and sixteen padding bytes. Its record expansion is 22 bytes for the modeled 1.3 profile, 37 bytes for 1.2 without CID, and 38 plus CID length for 1.2 with CID. The minimum UDP sizes are therefore 49/57 bytes for 1.3 without/with the model's eight-byte CID, and 64/73 bytes for 1.2. These are retained-content model sizes, not cryptographic validation results.

The association's shared footprint calculation now drives startup, actual probe admission, discovery and migration eligibility. Startup requires the base to fit every configured version, so a mixed offer also reserves the legacy footprint. After negotiation, checks use actual directional CID lengths. Defaults and wire encodings stay unchanged. Invalid configurations throw before UDP binding rather than entering repeated local probe refusal.

## Setup and observations

```mermaid
flowchart LR
    C[StandardHost client, six independent DTLS engines] <--> T[Ethernet PacketTap, 1 ms links]
    T <--> S[StandardHost server, six matching DTLS engines]
```

`Rfc6347PathMtuMinimumBase.test` uses the existing real UDP/IP fixture, GlobalArp, seed zero, declared checksums/FCS, zero configured processing cost and a six-second cutoff. Ports distinguish six engine pairs; their associations and discovery state are independent. Both endpoints enable heartbeat and discovery, with the probe ceiling equal to the base.

| Pair | Allowed client / server versions | CID | Base | Required result |
| --- | --- | --- | ---: | --- |
| 0 | 1.2 / 1.2 | off | 64 B | Legacy probe and confirmation |
| 1 | 1.2 / 1.2 | eight bytes | 73 B | Legacy CID probe and confirmation |
| 2 | 1.3 and 1.2 / 1.2 | off | 64 B | Actual legacy selection and confirmation |
| 3 | 1.3 and 1.2 / 1.2 | eight bytes | 73 B | Actual legacy CID selection and confirmation |
| 4 | 1.3 / 1.3 | off | 49 B | Existing 1.3 boundary control |
| 5 | 1.3 / 1.3 | eight bytes | 57 B | Existing 1.3 CID boundary control |

The fixture observes a request with exactly the configured UDP size and its actual version-specific record family. Both endpoints must report a confirmed 27-byte maximum application datagram. Each client must establish once without error and deliver a 27-byte marker through the real server callback. This is a preservation control: the parent already handles valid boundary values correctly.

Four module fixtures drive the production DtlsApp on a StandardHost with the PMTU base one byte below the enabled legacy minimum:

| Fixture in `tests/module/` | Versions | CID | Base |
| --- | --- | --- | ---: |
| `DtlsPathMtuLegacyBaseLow_1.test` | 1.2 | off | 63 B |
| `DtlsPathMtuLegacyBaseCidLow_1.test` | 1.2 | on | 72 B |
| `DtlsPathMtuMixedBaseLow_1.test` | 1.3 and 1.2 | off | 63 B |
| `DtlsPathMtuMixedBaseCidLow_1.test` | 1.3 and 1.2 | on | 72 B |

They require process exit 1 and the exact configuration error, `Invalid DTLS path MTU base, ceiling or timer profile`. These are successful checks of expected configuration errors, not expected protocol failures. On the parent, each engine instead initializes, binds UDP and reaches the cutoff with exit zero, so the same fixture fails its expectation. The correction rejects the impossible base before binding. No scripted peer substitutes for the engine in either category.

`DtlsPathMtuLegacyBaseNegative_1.test` additionally supplies a negative base and requires the same startup error. This preserves signed lower-bound validation when comparing the configured quantity with a computed byte count. It is an additional guard control; no exact-parent execution is claimed for this case.

## Verification and limits

From the INET root, build the matching library with `make -j$(nproc) MODE=debug`, then run:

```sh
opp_run_module_tests --load . -p inet -m debug -f 'DtlsPathMtu.*_1\.test$' --no-build
opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/dtls$' -f '(PathMtu|Heartbeat|Migration|Rfc9147Handshake\.test$|Rfc6347Handshake\.test$)' --no-build
```

For release, build with `MODE=release` and repeat the new module boundaries and `Rfc6347PathMtuMinimumBase.test` with `-m release`. Executed results and exact raw artifacts belong in the [verification results](../../../doc/project/evidence/model/dtls/results.md); these commands alone do not claim a pass. The unchanged parent fixtures were executed at `c95450bb5648`: four rejection checks fail because the expected configuration error is absent, while the six-profile production fixture passes.

The tests cover finite configured minima, real negotiation, confirmation and data. They do not establish arbitrary external CID lengths, real encryption, hardware costs or every possible version/parameter combination. Existing routed PMTU, heartbeat and migration fixtures check the shared consumers. Mermaid rendering is unverified.
