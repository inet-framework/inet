# DTLS over the existing UDP stack

This example uses two hosts and one Ethernet link. The client creates application datagrams through `DtlsApp`; the server's generic adapter delivers them to its sink. Both hosts use ordinary UDP/IP. The cryptographic model follows IPsec: `EncryptedChunk` carries retained logical content and overhead, without real encryption, authentication or certificate validation.

```mermaid
flowchart LR
    C[Client source and DTLS] <-->|Ethernet and UDP/IP| S[Server DTLS and sink]
```

From this directory, with the INET environment and a current debug library:

```sh
inet --debug -u Cmdenv -c Basic -r 0
inet --debug -u Cmdenv -c Certificate -r 0
inet --debug -u Cmdenv -c Fragmentation -r 0
inet --debug -u Cmdenv -c LossyHandshake -r 0
inet --debug -u Cmdenv -c ProcessingCost -r 0
inet --debug -u Cmdenv -c Ipv6 -r 0
```

`Basic` demonstrates PSK-profile setup and application traffic. `Certificate` changes modeled authentication messages. `Fragmentation` makes the certificate exceed the configured UDP payload budget. `LossyHandshake` additionally introduces seeded link errors; application losses remain losses rather than becoming reliable delivery. `ProcessingCost` sends a burst faster than record processing and exposes queueing delay. `Ipv6` changes the IP stack while retaining the DTLS application.

Inspect the DTLS engine's establishment, retransmission and processing signals and the sink's received-packet statistics. These demonstrations do not establish protocol conformance; the deterministic cases in `tests/protocol/dtls/` provide direct assertions. Use declared UDP checksums and Ethernet FCS. Exporting an entire protected packet as real ciphertext bytes is unsupported.

## Opt-in extension demonstrations

All configurations retain seed zero. `DtlsScenarioNetwork` adds ScenarioManager to the two-host topology; its script invokes the adapter's public socket operations. The original six configurations retain their existing topology and defaults.

| Configuration | Setup and required outcome |
| --- | --- |
| `Reconnect` | Enable tickets. Close at 2 s and reopen at 2.5 s. Both endpoints establish twice and report the second association resumed. |
| `EarlyData` | The same reconnect explicitly offers early data and sends one 32-byte datagram before establishment. The server explicitly accepts early delivery and disables cookies; exactly one accepted status must occur. Rejection would not trigger automatic replay. |
| `Migration` | Negotiate CID/heartbeat/migration, request a spare at 1 s and rebind the actual client UDP socket to port 44001 at 2 s. Both endpoints report a validated path and retain one establishment. |
| `DeferredAuthentication` | Certificate profile with PHA enabled. The server requests client authentication at 2 s; require one successful result and continuing data without reconnecting. |
| `AlgorithmCost` | Certificate profile with AES-256/SHA-384 and P-256. Explicit 2 ms output/1 ms input protection costs produce nonzero measured service. These costs are supplied model inputs. The external PSK profile deliberately requires SHA-256. |
| `PathMtu` | A client/router/server line has an 828-byte router egress MTU. GlobalArp and automatic routes select the only path. Real IPv4 feedback quotes 96 bytes; discovery confirms 800 UDP bytes and 770 application bytes with CID. Reverse discovery is independent. |
| `Legacy` | Select only DTLS 1.2; require the fixed PSK/GCM profile's 37-byte record expansion and delivery. |
| `MixedVersions` | A dual-version client meets a 1.2-only server. Actual legacy record expansion and application delivery demonstrate selection without timeout fallback. |
| `LegacyReconnect` | The reconnect schedule selects an abbreviated 1.2 ticket exchange at both endpoints. |
| `LegacyMigration` | The rebind schedule selects static RFC 9146 IDs; no spare request is made. Require both path callbacks and 46-byte CID record expansion. |

Run an individual configuration using the commands above with its name. For example:

```sh
inet --debug -u Cmdenv -c Migration -r 0
```

From the checkout root, `verify.py` runs all sixteen configurations, exports selected engine scalars through `opp_scavetool -F CSV-R`, and fails on missing/ambiguous results, a failed association, absent delivery or a missing configuration-specific outcome. Output directories must be new. Logs include the commands, raw simulation output, scalar files and exports; `report.json` contains the selected endpoint measurements.

```sh
python3 examples/dtls/verify.py --mode debug --output /tmp/dtls-examples-debug
python3 examples/dtls/verify.py --mode release --output /tmp/dtls-examples-release
```

Build the matching INET library first. `--configs Migration LegacyMigration` narrows the verifier to those demonstrations. Compare the two JSON reports for mode consistency; a single deterministic seed is not a statistical performance study. `ProcessingCost` deliberately ends with queued work, and the reconnect examples may discard a source packet during their explicitly closed interval. Neither behavior is an association failure.

## A changing routed MTU

The temporal MTU-change demonstration reuses the production fixture that already owns router/interface control and explicit assertions. It requires the protocol-test runner and a matching debug library. From the checkout root:

```sh
opp_run_protocol_tests --load . -p inet -m debug --no-build --concurrent \
  -w '^tests/protocol/dtls$' \
  -f '(Rfc9147PathMtuMigration|Rfc6347PathMtuMigration|Rfc6347PathMtuMigrationIpv6)\.test$'
```

Each case confirms the old budget, lowers the router egress MTU, requests a real interface move and checks stable handles, base reset, explicit refusal of oversized queued data and delivery of fitting data. IPv4 updates both gateway and interface. The IPv6 case uses suppressed-feedback adaptation because explicit interface selection in this checkout emits Ethernet broadcasts. It confirms a conservative budget rather than the exact link ceiling. The [legacy path reviewer document](../../tests/protocol/dtls/dtls12-paths.md) gives topology, addresses, routing, ICMP policy and expected byte budgets. This reuse adds no separate router controller or protocol behavior.
