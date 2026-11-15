# CoAP — English check procedures: Block-wise transfers and Request-Tag

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7959/catalog.md](../../../standard/rfc7959/catalog.md), [rfc9175/catalog.md](../../../standard/rfc9175/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## Block1 atomic request upload

- **Checks** — [RFC7959-B1-1](../../../standard/rfc7959/catalog.md#rfc7959-b1-1).
- **Requirement** — a `PUT` or `POST` payload exceeding the block size is uploaded in sequential `Block1` (`27`) blocks with `Size1` (`60`) on `NUM=0`; intermediate blocks (`M=1`) draw `2.31 Continue` echoing `Block1` without modifying the target resource, and the final block (`M=0`) applies the assembled payload atomically and returns `2.04 Changed` or `2.01 Created`.

### Procedure

1. Build the plain pair mockup with a writable resource `/firmware` and a block size of `64` octets (`SZX=2`).
2. Start a 3-block (`160`-octet) `PUT /firmware` upload from `C`, issue a concurrent `GET /firmware` after block `0` (`2.31 Continue`) before block `2` (`M=0`) arrives, and issue a second `GET /firmware` after block `2` completes; also verify an 8192-block (`128 KiB` at `16`-byte blocks) upload completes without message-ID history self-eviction.
3. Observe all `Block1` options, response codes, and `GET` payloads.

### Expected observations

1. Blocks `0` and `1` carry `Block1(NUM=0, M=1, SZX=2)` (with `Size1=160`) and `Block1(NUM=1, M=1, SZX=2)` and each receives `2.31 Continue` echoing `Block1`.
2. The `GET /firmware` before the final block returns the original pre-upload representation, proving atomic buffering.
3. Block `2` carries `Block1(NUM=2, M=0, SZX=2)` and receives `2.04 Changed`, after which `GET /firmware` returns the complete `160`-octet uploaded body; the `8192`-block upload likewise completes with `2.04 Changed`.

## Block2 response download and ETag consistency

- **Checks** — [RFC7959-B2-1](../../../standard/rfc7959/catalog.md#rfc7959-b2-1), [RFC7959-ERR-1](../../../standard/rfc7959/catalog.md#rfc7959-err-1).
- **Requirement** — a resource representation larger than the block size is transferred across sequential `GET` requests carrying `Block2` (`23`), with `M=1` on intermediate blocks, `M=0` on the final block, `Size2` (`28`) indicating total length, and `ETag` (`4`) identifying the version; if `ETag` changes mid-download, the client discards partial data and restarts once from `NUM=0`.

### Procedure

1. Build the plain pair mockup with a `160`-octet resource `/large` (`blockSize = 64B`, `SZX=2`).
2. Perform a normal multi-block `GET /large` download (and an early-negotiated download with `Block2(NUM=0, M=0, SZX=1)` + `Size2=0`); in a second download, mutate `/large` on the server after block `0` so block `1` carries a new `ETag`.
3. Observe the `Block2`, `Size2`, and `ETag` options on every block and the final delivered representation.

### Expected observations

1. In the normal download, `S` sends `Block2(NUM=0, M=1, SZX=2)`, `Block2(NUM=1, M=1, SZX=2)`, and `Block2(NUM=2, M=0, SZX=2)` with consistent `ETag` and `Size2=160`, and `C` delivers the exact `160`-octet body once on `M=0`.
2. When `/large` changes after block `0`, `C` detects the new `ETag` on block `1`, discards the stale partial buffer, restarts from `Block2(NUM=0)`, and delivers the new consistent representation.

## Block size negotiation for Block1 and Block2

- **Checks** — [RFC7959-NEG-1](../../../standard/rfc7959/catalog.md#rfc7959-neg-1).
- **Requirement** — when a server answers `Block1(NUM=0, SZX_c)` or `Block2(NUM=0, SZX_c)` with a smaller `SZX_s < SZX_c`, the client switches to `SZX_s` (`blockSize = 1 << (SZX_s + 4)`) and recomputes `NUM = byteOffset / blockSize` for all subsequent blocks.

### Procedure

1. Build the plain pair mockup where `C` proposes `64`-byte blocks (`SZX=2`) and `S` enforces a maximum block size of `32` bytes (`SZX=1`) (or `16` bytes, `SZX=0`).
2. Run a `Block1` upload and a `Block2` download across the size negotiation boundary.
3. Observe `SZX` and `NUM` on each block request and response.

### Expected observations

1. In `Block1`, `C` sends `NUM=0, M=1, SZX=2` (`64` bytes), `S` answers `2.31 Continue` with `Block1(NUM=0, M=1, SZX=1)` (or accepts the `64` bytes and negotiates `SZX=1`), and `C` sends the next block at byte offset `64` as `Block1(NUM=2, M=1, SZX=1)` (`32`-byte blocks).
2. In `Block2`, `C` requests `NUM=0, M=0, SZX=2`, `S` returns `32` bytes with `Block2(NUM=0, M=1, SZX=1)`, and `C` requests `Block2(NUM=1, M=0, SZX=1)` for the next `32` bytes.

## Blockwise transfer error responses 4.00, 4.08, 4.13, and 5.03

- **Checks** — [RFC7959-OPT-1](../../../standard/rfc7959/catalog.md#rfc7959-opt-1), [RFC7959-ERR-1](../../../standard/rfc7959/catalog.md#rfc7959-err-1).
- **Requirement** — `SZX == 7` returns `4.00 Bad Request`; `Block1` starting at `NUM > 0`, skipping a block, falling short of `Size1` on `M=0`, or arriving after upload state expiry returns `4.08 Request Entity Incomplete`; payloads or `Size1` exceeding `maxPayloadSize` return `4.13 Request Entity Too Large` with `Size1`; and exhausting retained `POST` result slots returns `5.03 Service Unavailable` with `Max-Age`.

### Procedure

1. Build the plain pair mockup.
2. Inject or send requests with `SZX=7`, `Block1(NUM=1)` without prior `NUM=0`, `Block1` with `M=0` shorter than `Size1`, `Block1` after `blockTransferTimeout` expiry, `Size1 > maxPayloadSize`, and concurrent multi-block `POST` results exceeding `maxRetainedBlock2Results`.
3. Observe the server's error response codes and options.

### Expected observations

1. `SZX == 7` returns `4.00 Bad Request`.
2. Non-sequential `Block1(NUM=1)`, `M=0` length mismatch against `Size1`, and expired upload state each return `4.08 Request Entity Incomplete`.
3. Oversized `Size1` or assembled payload returns `4.13 Request Entity Too Large` with `Size1 = maxPayloadSize`, and exhausting retained `Block2` result contexts returns `5.03 Service Unavailable` with `Max-Age`.

## Combined Block1 upload and Block2 response transfer

- **Checks** — [RFC7959-COMB-1](../../../standard/rfc7959/catalog.md#rfc7959-comb-1).
- **Requirement** — when a multi-block `POST` or `PUT` upload (`Block1`) produces a response body larger than the block size (`Block2`), all `Block1(M=1)` blocks receive `2.31 Continue`, the response to the final `Block1(M=0)` carries both `Block1(M=0)` and `Block2(NUM=0, M=1)` with the first response block, and follow-up requests retrieve `Block2(NUM >= 1)` without re-uploading the request body.

### Procedure

1. Build the plain pair mockup with a `POST /compute` resource whose response body spans multiple `Block2` blocks (`120` bytes at `64`-byte block size).
2. Send a 2-block (`100`-byte) `POST /compute` upload from `C` and let `C` retrieve the complete multi-block response.
3. Observe all `Block1` and `Block2` options on the wire.

### Expected observations

1. `C` sends `POST` with `Block1(NUM=0, M=1, SZX=2)` and receives `2.31 Continue` echoing `Block1(NUM=0, M=1, SZX=2)`.
2. `C` sends `POST` with `Block1(NUM=1, M=0, SZX=2)` and receives `2.04 Changed` carrying both `Block1(NUM=1, M=0, SZX=2)` and `Block2(NUM=0, M=1, SZX=2)` plus the first `64` response bytes.
3. `C` sends a follow-up request with `Block2(NUM=1, M=0, SZX=2)` (and no `Block1` option), receives `2.04`/`2.05` with `Block2(NUM=1, M=0, SZX=2)` and the remaining `56` bytes, and delivers the complete `120`-byte response once.

## Request-Tag concurrent Block1 uploads

- **Checks** — [RFC9175-RTAG-1](../../../standard/rfc9175/catalog.md#rfc9175-rtag-1).
- **Requirement** — the repeatable `Request-Tag` option (`38`, `0..8` octets) keys concurrent `Block1` assemblies from the same client endpoint so interleaved uploads do not corrupt each other; a new `Request-Tag` starting at `NUM > 0` returns `4.08`, and a `Request-Tag` longer than `8` octets returns `4.02 Bad Option`.

### Procedure

1. Build the plain pair mockup.
2. Interleave two 2-block `PUT` uploads from the same client endpoint to `/data` using distinct `Request-Tag` values (`"tagA"` and `"tagB"`), inject a `Block1(NUM=1)` with an unseen `Request-Tag` `"tagC"`, and inject a 9-octet `Request-Tag`.
3. Observe the server responses and final resource representations.

### Expected observations

1. Both interleaved uploads (`"tagA"` and `"tagB"`) complete with `2.31 Continue` on `NUM=0` and `2.04 Changed` on `NUM=1`, each assembling its own uncorrupted payload.
2. `Block1(NUM=1)` with unseen `"tagC"` is rejected with `4.08 Request Entity Incomplete`, and the 9-octet `Request-Tag` is rejected with `4.02 Bad Option`.

## Observe notification with Block2 follow-up retrieval

- **Checks** — [RFC7959-OBS-1](../../../standard/rfc7959/catalog.md#rfc7959-obs-1).
- **Requirement** — when an observed resource's representation exceeds the block size, the server sends the first block (`Block2(NUM=0, M=1)`) in the `Observe` notification using the registration Token, and the client retrieves the remaining blocks (`NUM >= 1`) with normal `GET` requests carrying `Block2`, no `Observe` option, and a fresh exchange Token distinct from the registration Token.

### Procedure

1. Build the plain pair mockup with an observable resource `/sensors/History` whose payload (`150` octets) spans three `64`-octet blocks (`SZX=2`).
2. Register an observation with `GET /sensors/History` (`Observe=0`), complete the initial 3-block retrieval, and then trigger a server state update to `150` new octets.
3. Observe the tokens and options on the notification and follow-up block requests.

### Expected observations

1. Both the initial response and the subsequent update notification carry `Observe`, `Block2(NUM=0, M=1, SZX=2)`, `ETag`, and the registration Token `T_obs`.
2. For blocks `NUM=1` and `NUM=2`, `C` sends `GET /sensors/History` with `Block2(NUM, M=0, SZX=2)`, no `Observe` option, and a fresh exchange Token `T_blk != T_obs`.
3. When `Block2(NUM=2, M=0, SZX=2)` arrives with matching `ETag`, `C` delivers the complete `150`-octet representation for the observation while keeping `T_obs` registered for future notifications.

## Observe and Block2 version change supersession and recovery

- **Checks** — [RFC7959-OBS-1](../../../standard/rfc7959/catalog.md#rfc7959-obs-1).
- **Requirement** — if a newer `Observe` notification arrives while a `Block2` retrieval for an earlier notification is in flight, the client aborts the old assembly and switches to the newer notification; if a follow-up `Block2` block returns a different `ETag` without a superseding notification having arrived, the client aborts the inconsistent assembly and schedules a re-registration `GET` (`Observe=0`) after the `[5s, 15s]` jitter window.

### Procedure

1. Build the plain pair mockup with a multi-block observable resource.
2. In phase 1, delay the follow-up `Block2(NUM=1)` response for notification `V1` while the server emits a newer notification `V2`; in phase 2, mutate the resource silently on the server before `Block2(NUM=1)` is served (dropping the notification for the mutation) so `Block2(NUM=1)` returns a mismatched `ETag`.
3. Observe the client's follow-up requests, re-registration timing, and delivered representations.

### Expected observations

1. In phase 1, upon receiving notification `V2` (`Block2(NUM=0, M=1)` with newer `Observe` serial), `C` discards the partial `V1` buffer, retrieves `V2`'s remaining blocks, and delivers only `V2`'s complete body.
2. In phase 2, upon receiving `Block2(NUM=1)` with a mismatched `ETag` and no superseding notification, `C` discards the partial buffer and re-registers with `GET` (`Observe=0`, registration Token) after the `[5s, 15s]` jitter delay, recovering the complete updated representation.
