Native route expiry query
~~~~~~~~~~~~~~~~~~~~~~~~~

Custom implementations of ``IRoute`` must implement ``getNativeExpiryTime()``.
Return zero when the provider does not expire routes, or the absolute native
expiry time otherwise. A nonzero expiry is valid through that simulation time.
Protocol-specific metadata lifetimes remain independent. IPv6 exposes its
existing expiry; IPv4 and next-hop routes return zero.
