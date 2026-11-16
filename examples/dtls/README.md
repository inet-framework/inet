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
