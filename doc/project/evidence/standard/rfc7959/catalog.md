# RFC 7959 (CoAP Block-Wise Transfers) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC7959-*` · **Stands on:** [standards.md](../../protocol/coap/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 7959, Block-Wise Transfers in the Constrained Application Protocol (CoAP), August 2016. The catalog comes from the specification text only and contains no simulation model names and no code references.

Source: `standards/RFC/rfc7959.txt` (<https://www.rfc-editor.org/rfc/rfc7959.txt>, August 2016).

The family of CoAP documents is in [`standards.md`](../../protocol/coap/standards.md), the feature map is in [`features.md`](../../protocol/coap/features.md), and the workflow state lives in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC7959-OPT-1](#rfc7959-opt-1) | `Block1` (`27`) and `Block2` (`23`) encode `(NUM << 4) \| (M << 3) \| SZX` in 1..3 unsigned octets (`SZX` `0..6`); `SZX == 7` returns `4.00 Bad Request` and option length `> 3` returns `4.02 Bad Option`. |
| [RFC7959-B1-1](#rfc7959-b1-1) | Multi-block `PUT`/`POST` uploads transfer sequential `Block1` blocks (`2.31 Continue` on `M=1`, `2.01`/`2.04` on `M=0`) and apply the assembled body atomically on the final block. |
| [RFC7959-B2-1](#rfc7959-b2-1) | Multi-block `GET` downloads transfer sequential `Block2` blocks (`M=1` until the final block `M=0`), carrying `Size2` (`28`) when requested or appropriate. |
| [RFC7959-NEG-1](#rfc7959-neg-1) | Either endpoint may negotiate a smaller `SZX` (`blockSize`) in `Block1` or `Block2`, and subsequent requests recompute `NUM = byteOffset / blockSize` at the smaller size. |
| [RFC7959-ERR-1](#rfc7959-err-1) | Non-sequential or incomplete `Block1` transfers return `4.08 Request Entity Incomplete`, oversized bodies return `4.13 Request Entity Too Large`, and an `ETag` change during `Block2` download restarts assembly from `NUM=0`. |
| [RFC7959-COMB-1](#rfc7959-comb-1) | A combined `Block1` upload + `Block2` response completes all `Block1` `2.31 Continue` exchanges first, returns the first `Block2` response block (`NUM=0, M=1`) on the final `Block1` (`M=0`), and serves remaining blocks via follow-up `Block2` requests. |
| [RFC7959-OBS-1](#rfc7959-obs-1) | An `Observe` notification for a large resource carries the first `Block2` block (`NUM=0, M=1`), and the client retrieves remaining blocks using normal `GET` requests with `Block2` and a distinct Token without `Observe`, superseding or re-registering on version changes. |

## Block option encoding, transfers, negotiation, and Observe integration

### RFC7959-OPT-1

**`Block1` (`27`) and `Block2` (`23`) encode `(NUM << 4) | (M << 3) | SZX` in 1..3 unsigned octets (`SZX` `0..6`); `SZX == 7` returns `4.00 Bad Request` and option length `> 3` returns `4.02 Bad Option`.**

> "The value of a Block Option is a variable-size (0 to 3 byte) unsigned integer (uint). The least significant 3 bits (SZX) indicate the block size as 2**(SZX + 4) for SZX = 0 to 6; the value 7 (111) is reserved (BERT) and MUST NOT be used over UDP. The fourth least significant bit (M) indicates whether more blocks are following, and the remaining upper bits (NUM) indicate the block number." — §2.2, `rfc7959.txt:255-320`

- Strength: must. Class: encoding plus wire.
- Check idea: verify 0-, 1-, 2-, and 3-octet `Block1`/`Block2` encodings, verify `SZX == 7` triggers `4.00 Bad Request`, and verify a 4-octet `Block1` or `Block2` option triggers `4.02 Bad Option`.

### RFC7959-B1-1

**Multi-block `PUT`/`POST` uploads transfer sequential `Block1` blocks (`2.31 Continue` on `M=1`, `2.01`/`2.04` on `M=0`) and apply the assembled body atomically on the final block.**

> "In a block-wise transfer of a request payload (e.g., PUT or POST), the Block1 Option indicates the block number and size, and the server answers intermediate blocks (M=1) with 2.31 (Continue) before acting on the assembled payload when the final block (M=0) arrives." — §2.3 and §2.5, `rfc7959.txt:390-540`

- Strength: must. Class: wire plus end-to-end.
- Check idea: upload a multi-block payload via `PUT`/`POST` with `Block1` and `Size1` (`60`), verify `2.31 Continue` on every `M=1` block and `2.04`/`2.01` on `M=0`, and verify the resource representation stays unchanged until `M=0` completes.

### RFC7959-B2-1

**Multi-block `GET` downloads transfer sequential `Block2` blocks (`M=1` until the final block `M=0`), carrying `Size2` (`28`) when requested or appropriate.**

> "In a block-wise transfer of a response payload (e.g., GET), the server includes a Block2 Option in each block response (with M=1 on all blocks except the last, where M=0), and the client requests subsequent blocks by incrementing NUM." — §2.4 and §4, `rfc7959.txt:450-510`

- Strength: must. Class: wire plus end-to-end.
- Check idea: download a multi-block resource representation with `GET` (both server-initiated and client early-negotiated with `Size2=0`), verifying `Block2` `NUM`, `M`, `SZX`, `Size2` (`28`), `ETag`, and the reassembled payload.

### RFC7959-NEG-1

**Either endpoint may negotiate a smaller `SZX` (`blockSize`) in `Block1` or `Block2`, and subsequent requests recompute `NUM = byteOffset / blockSize` at the smaller size.**

> "Both in Block1 and Block2 transfers, the peer may indicate a smaller block size (SZX) than requested; subsequent block requests MUST use the smaller SZX and adjust the block number NUM to match the byte offset already transferred." — §2.2 and §2.5, `rfc7959.txt:345-385`

- Strength: must. Class: wire.
- Check idea: configure the server with a smaller maximum block size than the client requests in `Block1` and `Block2`, and verify the client switches to the server's smaller `SZX` and recomputes `NUM = offset / blockSize` on the next block.

### RFC7959-ERR-1

**Non-sequential or incomplete `Block1` transfers return `4.08 Request Entity Incomplete`, oversized bodies return `4.13 Request Entity Too Large`, and an `ETag` change during `Block2` download restarts assembly from `NUM=0`.**

> "If a server receives a Block1 request that does not match the expected next byte offset or has not received all blocks needed to assemble the body, it responds with 4.08 (Request Entity Incomplete). If the payload exceeds the server's buffer size, it responds with 4.13 (Request Entity Too Large) (and Size1). In a Block2 transfer, the client checks the ETag Option across blocks to ensure all blocks come from the same resource version." — §2.4, §2.5, and §2.9, `rfc7959.txt:490-690`

- Strength: must. Class: wire.
- Check idea: inject out-of-order `Block1` blocks (`NUM > 0` without `NUM=0`, gap in `NUM`, or `M=0` shorter than `Size1`) and verify `4.08`; exceed `maxPayloadSize` and verify `4.13` with `Size1`; mutate the resource mid-`Block2` download so `ETag` changes and verify the client restarts once from `NUM=0`.

### RFC7959-COMB-1

**A combined `Block1` upload + `Block2` response completes all `Block1` `2.31 Continue` exchanges first, returns the first `Block2` response block (`NUM=0, M=1`) on the final `Block1` (`M=0`), and serves remaining blocks via follow-up `Block2` requests.**

> "Both Block1 and Block2 Options can be present in a single exchange when a large request payload produces a large response payload: the Block1 phase completes first, the response to the final Block1 request carries the first Block2 block (NUM=0), and follow-up requests retrieve the remaining Block2 blocks." — §2.7 and §3.3, `rfc7959.txt:610-655`

- Strength: must. Class: wire plus end-to-end.
- Check idea: send a multi-block `POST` upload that produces a multi-block response and verify the `Block1` (`2.31 Continue`) phase, the transition response carrying both `Block1(M=0)` and `Block2(NUM=0, M=1)`, and the subsequent `Block2` retrieval requests.

### RFC7959-OBS-1

**An `Observe` notification for a large resource carries the first `Block2` block (`NUM=0, M=1`), and the client retrieves remaining blocks using normal `GET` requests with `Block2` and a distinct Token without `Observe`, superseding or re-registering on version changes.**

> "When a notification is larger than can be sent in a single datagram, the server sends the first block (Block2 NUM=0, M=1) in the notification, and the client retrieves the remaining blocks using GET requests with Block2 Options and without an Observe Option (using a fresh token so the observation token remains bound to the registration)." — §2.6 and §3.4, `rfc7959.txt:570-608`

- Strength: must. Class: wire plus end-to-end.
- Check idea: observe a resource whose payload spans multiple `Block2` blocks, verify the notification carries `Observe` and `Block2(NUM=0, M=1)` with the registration Token while follow-up `GET` requests carry `Block2(NUM>=1)` without `Observe` and with a different Token, and verify supersession by a newer notification and recovery from a silent version (`ETag`) change.
