# Selected DTLS extension mechanisms

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [extension obligations](../../standard/rfc9147/extensions.md)

The selected extension profile adds ticket resumption, explicit early datagrams, CID-based peer movement, post-handshake certificate authentication, alternative algorithm representations, path-size discovery and finite DTLS 1.2 compatibility. These are behavioral mechanisms with abstract protection; no real keys, signatures, certificate chains, binders or ciphertext authentication are computed.

Resumption identifies saved local session state. A resumed certificate session omits certificate messages, while an external-PSK origin already has a PSK flight. Early data has separate admission and replay handling; acceptance never promises delivery or automatic retry. CID selects association context but does not authenticate a changed address. Heartbeat return reachability and nonfragmenting size probes supply the selected path observations.

The finite 1.3 catalog uses AES-128-GCM/SHA-256, AES-256-GCM/SHA-384 or ChaCha20-Poly1305/SHA-256, with X25519 or secp256r1 and modeled ECDSA-P256/SHA-256 or RSA-PSS/SHA-256 signatures. An external PSK retains SHA-256; SHA-384 PSK resumption requires a matching saved session. The 1.2 catalog uses PSK-AES-128-GCM/SHA-256 or ECDHE-ECDSA-AES-128-GCM/SHA-256 with secp256r1.

Version 1.2 uses CCS and whole-flight recovery, including bounded final-flight response retention. It has no 0-RTT, KeyUpdate, PHA or dynamic CID updates. Static 1.2 CID follows its distinct record format. Renegotiation, TLS over TCP, actual cryptography, live endpoint interoperability and RFC 9853 RRC are excluded.

Implementation selection and test status belong to the [model record](../../model/dtls/extensions.md).
