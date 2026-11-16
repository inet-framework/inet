Datagram Transport Layer Security
================================

The DTLS model supplies a datagram security protocol simulation over the existing UDP stack. It models DTLS 1.3 handshake messages, cookie validation, fragmentation, loss recovery, record numbering, application datagrams and key updates. It uses the host's application slot, so existing hosts require no additional transport submodule.

Model fidelity
--------------

Cryptography follows the INET IPsec abstraction. An ``EncryptedChunk`` wraps retained logical content, and the model accounts for record expansion and configurable processing time. It does not perform encryption, authentication-tag verification, key derivation, signature generation or certificate-chain validation. Compatible transmitted identity labels and configured profiles represent successful authentication. Measurements describe traffic and timing, not resistance to attacks.

The selected profile uses ``TLS_AES_128_GCM_SHA256`` with modeled X25519 shares and either external PSK or certificate authentication. Certificate contents and signatures are configured-sized placeholders. DTLS 1.2, connection IDs and address migration, session tickets/resumption, early data and post-handshake certificate authentication are outside this model.

Use declared UDP checksums and link FCS. The protected content has no real ciphertext byte representation, so computed checksums over it, protected-packet raw export and packet-data fingerprints are unsupported. Replay filtering is tested within the truncated-number reconstruction range; old records whose partial numbers alias later records cannot be cryptographically disambiguated. The same limitation applies to overlapping old associations on one address/port quartet.

Configuration
-------------

Select ``DtlsApp`` in a host application slot. Configure its ``dtls`` submodule for the local UDP endpoint and profile, and its ``io`` submodule for the client destination. Set the compound application's ``server`` parameter to listen without a traffic source. For example:

.. code-block:: ini

   *.client.numApps = 1
   *.client.app[0].typename = "DtlsApp"
   *.client.app[0].dtls.localPort = 44000
   *.client.app[0].io.destAddress = "server"
   *.client.app[0].io.destPort = 4433
   *.client.app[0].source.initialProductionOffset = 100ms
   *.client.app[0].source.productionInterval = 500ms
   *.client.app[0].source.packetLength = 128B
   *.server.numApps = 1
   *.server.app[0].typename = "DtlsApp"
   *.server.app[0].dtls.localPort = 4433
   *.server.app[0].server = true
   **.checksumMode = "declared"
   **.fcsMode = "declared"

The generic server adapter delivers incoming datagrams to its sink. A custom application can use ``DtlsSocket`` and association handles for bidirectional conversations with several peers. Closing one handle leaves the engine's shared listening socket and other associations usable.

``open`` returns zero if the attempt cannot be admitted synchronously. ``send`` transfers packet ownership even when rejected; ``sendError`` reports an individual packet failure. An association ``error`` is terminal and invalidates that handle. ``requestKeyUpdate`` returns whether the request was admitted, so a busy rejection leaves the existing association and update deadline intact. An admitted update can still report an immediate terminal error if required control work cannot be queued.

Set both endpoints' ``dtls.profile`` to ``"certificate"`` for modeled certificate authentication. The listener's ``dtls.clientAuthentication`` additionally requests a certificate during the initial client handshake. ``certificateBytes`` and ``signatureBytes`` control the placeholder sizes. Setting the client's ``offerInitialKeyShare`` to false exercises HelloRetryRequest for the selected key exchange group.

The NED declaration is the reference for parameter defaults and signals. ``udpPayloadLimit`` is a configured known-safe UDP payload budget, not automatic path MTU discovery. Application datagrams exceeding the resulting record limit or the TLS 16 KiB content limit are rejected rather than split into several deliveries. Larger handshake messages are fragmented within the record and UDP budgets.

Timing and recovery
-------------------

Cookies are enabled by default and model return reachability. The server limits output before validation. Its handshake deadline starts at the first structurally valid ClientHello arrival, including queued processing and reassembly, and a cookie retry preserves that deadline. A flight is retained until its peer response or ACK establishes receipt; lost handshake content is retransmitted with backoff. Retransmitted fragments preserve message bytes and epoch but use fresh record numbers. Application data retains UDP semantics: packet loss does not initiate application retransmission or ordered-stream recovery.

Record protection, unprotection and handshake work use independent incoming and outgoing service queues. Delay parameters default to zero and can model finite processing cost. A zero-cost operation with an idle queue completes synchronously; positive processing costs schedule completion. An attempt waiting for sending credit or a key-update ACK does not occupy the shared processor.

Key updates change modeled protection epochs. The sender waits for its update's ACK before using the next epoch. Both manual requests and the configured record-count threshold use this path. A duplicate update is acknowledged without advancing receiving state twice.

Lifecycle and observation
-------------------------

An association close attempts one ``close_notify`` and completes locally without depending on reliable peer notification. A deadline bounds delayed processing; forced closure reports a terminal ``error`` followed by one ``closed`` completion after cleanup. The callback object must remain alive through that notification sequence. A crash cancels work without sending an alert. Module stop closes the shared UDP socket after association shutdown, and restart rebinds it for fresh associations while retaining cumulative counters.

Use declared DTLS signals for establishment, handshake retransmission, record drops and processing delay, together with source/sink statistics. Observers do not drive protocol decisions. The ``examples/dtls`` configurations demonstrate loss-free setup, certificate overhead, fragmentation, seeded link errors, processing costs and IPv6. Protocol tests supply deterministic assertions separately from those demonstrations.
