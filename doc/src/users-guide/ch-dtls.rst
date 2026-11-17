Datagram Transport Layer Security
================================

The DTLS model runs over ordinary UDP/IP in a host application slot. It models
handshake messages, cookie validation, fragmentation, recovery, record numbering,
application datagrams and closure. DTLS 1.3 is the default. Optional capabilities
are disabled unless explicitly configured.

Model fidelity
--------------

Cryptography follows INET's IPsec abstraction: an ``EncryptedChunk`` retains
logical content while accounting for expansion and directional processing time.
There is no actual AEAD, key derivation, signature or certificate-chain validation.
Compatible identity labels represent modeled authentication success. These models
measure simulated traffic and timing; they do not establish security or hardware
performance. Use declared UDP checksums and Ethernet FCS. Exporting arbitrary
protected packets as real ciphertext bytes is unsupported.

The default profile uses AES-128-GCM/SHA-256 with X25519 and external PSK, or
modeled ECDSA certificate authentication when ``profile = "certificate"``.
DTLS 1.3 additionally supports the finite AES-256-GCM/SHA-384 and
ChaCha20-Poly1305/SHA-256 profiles, P-256 key shares and modeled RSA-PSS signatures.
Select preferences through ``cipherSuites``, ``keyExchangeGroups`` and
``signatureSchemes``. Certificate/signature bodies remain sized placeholders.
Algorithm-specific delay parameters inherit the generic directional costs unless
explicitly set. Their values are simulation inputs, not measured algorithm speeds.
The configured external PSK uses SHA-256; select the certificate profile for the
AES-256/SHA-384 demonstration rather than combining it with that PSK.

Composition and application API
-------------------------------

``DtlsApp`` composes a generic source/sink, ``DtlsSocketIo`` and a replaceable
``IDtls`` engine. Existing host UDP/IP modules carry its packets; hosts need no
additional transport submodule. The default adapter sends client source packets
to the current association and delivers received packets to its sink. Applications
that need multiple handles or finer admission policy should use ``DtlsSocket``
with their own callback implementation.

``open`` returns a stable association handle or zero when admission fails.
Packets passed to ``send`` or ``sendEarly`` transfer ownership to the engine,
including refusal paths. Normal ``send`` waits for establishment as necessary;
application datagrams retain UDP loss semantics. A later ``sendError`` identifies
the refused packet. It does not imply association failure. Keep callbacks alive
through the terminal ``error``/``closed`` sequence, and handle synchronous callbacks
when configured service cost is zero. Application callbacks may close a handle.

Custom callbacks must implement ``earlyDataArrived``, ``earlyDataStatus``,
``pathChanged``, ``authenticationResult`` and ``maximumDatagramSizeChanged`` in
addition to the original association callbacks. Early delivery requires an
explicit application policy; ordinary data must not silently stand in for it.
For dynamic budgets, query ``getMaximumDatagramSize`` after establishment and
consume budget-change callbacks. Zero means that application output is currently
unavailable. A queued datagram may become too large after a path change and receive
``sendError``; the engine never splits an application datagram into DTLS messages.

Session and early-data policy
-----------------------------

Enable ``resumptionEnabled`` to issue and cache modeled opaque tickets. The store
is bounded by ``maxSessions`` and ``ticketLifetime`` and is cleared by engine
stop/crash. Normal association close preserves eligible sessions. ``open`` with
resume intent falls back to a full handshake when no compatible live entry exists;
``isResumed`` reports the actual outcome. Tickets are stateful simulated identities,
not stateless encrypted credentials.

DTLS 1.3 early data additionally requires ``earlyDataEnabled``, a compatible unused
saved allowance, explicit ``DtlsOpenOptions::offerEarlyData`` and ``sendEarly``.
The application receives a separate early status and delivery callback. Cookie
retry rejects early data. The acceptance example therefore deliberately disables
the server cookie policy. Rejected early data is never replayed automatically as
ordinary data. The bounded ticket store models one-use admission; it cannot prove
cryptographic anti-replay or security across independent servers.

Connection IDs, migration and authentication
--------------------------------------------

``connectionIdEnabled`` negotiates eight-byte local receive IDs. Setting
``receiveConnectionId = false`` offers an empty receive ID; directional overhead
then differs. DTLS 1.3 can request/provide spare IDs and activate one through the
socket API. DTLS 1.2 uses a static negotiated CID and refuses those update APIs.

Migration requires ``migrationEnabled``, negotiated nonempty CIDs and heartbeat
permission. ``migrateLocal`` selects an owned local address/interface;
``rebindLocalPort`` replaces the actual client UDP socket and is restricted to one
association without a listener. DTLS 1.3 local moves require a spare peer CID.
The destination changes only after a fresh correlated heartbeat response. A
successful move preserves the association handle. A failed rebind validation
terminates that handle because the old socket has been replaced. This is modeled
heartbeat validation, not RFC 9853 or cryptographic path authentication.

``postHandshakeAuthEnabled`` lets a DTLS 1.3 server request a client certificate
later using ``requestClientAuthentication``. One request may be active. Query
``getPeerAuthentication`` for its current modeled snapshot. Tickets preserve the
snapshot at issuance; later authentication does not retroactively rewrite them.
DTLS 1.2 refuses this API and refuses renegotiation with no_renegotiation.

Path budgets and record service
--------------------------------

``pathMtuDiscoveryEnabled`` uses heartbeat to confirm a directional UDP budget.
The configured ``pathMtuBase`` defaults to 512 bytes: this is an explicit small-MTU
simulation departure from RFC 8899's IPv4/IPv6 bases. Search increases in 128-byte
steps with three actual attempts per size, exponentially spaced from
``pathMtuProbeTimeout``. Confirmation and upward search have separate intervals.
Typed ICMP quotations are correlated with the current probe; truncated, raw,
stale or ordinary-application quotes do not change the budget. ICMP is not
authenticated. Migration resets confirmation to the base. Unconfirmed/error paths
may suspend application output while retaining small control exchanges.

Startup rejects a base that cannot carry a heartbeat for every allowed version
and the configured CID profile. A mixed-version offer therefore needs room for
legacy expansion even if a particular peer later selects 1.3. Negotiated peer
CID lengths are checked again by the association.

DTLS 1.3 record overhead is 22 bytes plus an outgoing CID. DTLS 1.2 uses 37 bytes
without CID, or 38 plus the CID length, including its explicit nonce and inner
content type. No-padding application limits are 16384 bytes normally and 16383
for legacy CID records. The query also applies the current UDP budget. Discovery
sets the existing don't-fragment request. In this checkout, explicit interface
selection for IPv6 emits Ethernet broadcasts, which can suppress ICMPv6 feedback;
loss-based confirmation remains conservative in that case.

Input and output have independent service queues. Zero-cost work can finish
synchronously; positive costs schedule completion. Retransmissions use fresh
record numbers and retain logical handshake content without charging its service
twice. A stalled peer does not occupy the shared processor while waiting for credit
or a control response. Application data is never made reliable by handshake retry.

Legacy version selection
------------------------

Set ``versions = "1.2"`` or ``"1.3 1.2"`` to permit legacy operation. A dual offer
prefers 1.3; a timeout never triggers fallback. Legacy profiles are fixed
PSK-AES-128-GCM/SHA-256 or ECDHE-ECDSA-AES-128-GCM/SHA-256 with P-256/ECDSA.
Legacy records use full numbers, HelloVerifyRequest cookies, CCS and whole-flight
retry. The final sender retains its response for 240 seconds independently of the
120-second default handshake deadline. With resumption enabled,
``dtls12SessionTicketsEnabled = false`` selects session-ID-only operation;
otherwise opaque tickets are permitted. Each resumed association negotiates fresh
CIDs. Early data, KeyUpdate, deferred authentication and dynamic CID updates are
unavailable in 1.2. Reaching its configured record-use bound fails the association
rather than invoking KeyUpdate.

Lifecycle, scenarios and observation
-----------------------------------

Graceful close attempts one close_notify and completes without reliable peer
notification. A deadline bounds delayed processing. Crash cancels work without
sending an alert; restart binds a fresh socket. Observe establishment, failure,
record overhead, retransmission and processing signals together with source/sink
statistics. Observers publish completed facts and do not drive protocol decisions.

``DtlsSocketIo`` implements ScenarioManager's ``IScriptable`` contract for opt-in
application actions: ``close-dtls``, ``open-dtls`` (optional ``early="true"``),
``send-early-dtls`` (a 32-byte demonstration datagram), ``request-dtls-cid``,
``rebind-dtls`` (required integer ``port``) and ``authenticate-dtls``. Reopen uses
its ``resumeSession`` preference. Commands act on the current client handle or
latest accepted server handle and require an operating adapter. Use a custom
application for multiple independently controlled server associations.

``examples/dtls`` includes standalone configurations and a fixed-seed verifier
for all sixteen demonstrations. Its README also gives the asserting routed
MTU-change fixture command. Those deterministic checks demonstrate selected
outcomes; protocol tests and their reviewer documents state the wider behavior
coverage and limits. No stored statistical or fingerprint baseline is refreshed
by the example verifier.
