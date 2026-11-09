AODV selected interfaces
~~~~~~~~~~~~~~~~~~~~~~~~

An empty ``interfaces`` value keeps the primary ``interface`` alone. A nonempty
value is a whitespace-separated list of exact names, includes the primary and
supports IPv4. Remove repeated names and wildcard patterns. Every selection must
be up, nonloopback and have a usable unicast address at actual start. Explicit
selections require distinct local IPv4 addresses and one address family.

Both parameters are mutable and apply on the next start. Stop the model before
removing interfaces or changing addresses, then reconfigure and start again.
For an AckingWirelessInterface profile that must receive immediately after a
restart, configure the radio's ``initialRadioMode`` to ``receiver`` or
``transceiver``. Its default ``off`` mode can otherwise await a local transmission.

Custom AODV subclasses calling ``createRREP`` must now pass the original RREQ
ingress interface as its final argument, so cached precursors retain that link.
