DTLS Connection Identifier Update Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IDtls`` implementations must implement ``requestConnectionIds(handle, count)``, ``provideConnectionIds(handle, count, immediate)`` and ``activateSpareConnectionId(handle)``. ``DtlsSocket`` forwards these operations. They return synchronous admission; callbacks are unchanged. Request admission remains busy until both ACK and fulfillment, including after request timeout. Spare availability and selection are reported by ``connectionIdsReceived`` and ``connectionIdChanged``; ``connectionIdRequestTimeout`` reports expiry. Enable ``connectionIdEnabled`` to use these operations with the production engine. ``maxReceiveConnectionIds`` defaults to four and accepts values one through four.
