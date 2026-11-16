DTLS Session Resumption Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Existing calls to ``IDtls::open(peer, port, callback)`` retain their full-handshake behavior. To request best-effort ticket resumption, enable ``Dtls.resumptionEnabled`` and call ``open(peer, port, callback, true)``. A missing, expired or declined ticket permits the configured full handshake. ``isResumed(handle)`` reports a selected ticket after establishment. ``DtlsSocket`` forwards both operations; composed applications can set ``DtlsSocketIo.resumeSession``.

Custom implementations of ``IDtls`` must implement the new four-argument ``open`` overload and the const ``isResumed`` query. Callback signatures are unchanged. ``TlsHandshakeParameters::Selection`` now represents authentication with ``authenticationMode`` (certificate, external PSK or ticket PSK); replace reads of ``usePsk`` with ``usesPsk()`` and select the corresponding mode when constructing a value.
