DTLS post-handshake authentication
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Implement ``IDtls::requestClientAuthentication()`` and ``getPeerAuthentication()`` in custom engines, and ``ICallback::authenticationResult()`` in callback implementations. A synchronously refused request produces no result. An admitted request reports its result only at the requesting server; failure accompanies invalidation or closure. Keep callback objects alive through the existing error/closed notifications.

Enable ``postHandshakeAuthEnabled`` on both endpoints before requesting authentication. Full external-PSK handshakes refuse requests; certificate and ticket-selected associations support them. ``getPeerAuthentication()`` distinguishes saved ticket context from a fresh PHA result. Older tickets retain their issuance-time snapshot when a current association authenticates later. All certificates and verification remain modeled rather than cryptographic.
