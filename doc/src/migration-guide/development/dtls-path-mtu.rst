DTLS path payload discovery callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IDtls::ICallback`` implementations must implement
``maximumDatagramSizeChanged(AssociationId, size_t)``. The value is the current
maximum application datagram size; zero reports unavailable sending. Query the
same limit with ``getMaximumDatagramSize()``. Existing associations keep the
configured fixed UDP limit unless ``pathMtuDiscoveryEnabled`` is enabled.

Discovery requires negotiated heartbeat permission. It starts at a provisional
512-byte UDP base, positively confirms larger budgets and suspends application
output after base failure. Small control traffic remains usable. The base is a
simulation profile departure from RFC 8899's IPv4 recommendation and IPv6
minimum. Cryptography remains abstract. Applications must handle explicit send
errors when a queued datagram no longer fits a reduced budget.
