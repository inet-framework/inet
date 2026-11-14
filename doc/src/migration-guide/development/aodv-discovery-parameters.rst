AODV discovery parameters
~~~~~~~~~~~~~~~~~~~~~~~~~

AODV now validates the parameters needed to make discovery progress. Keep
``1 <= ttlStart <= ttlThreshold <= netDiameter <= 255`` and ``ttlIncrement``
in ``1..255``. Request/error rate limits and allowed Hello loss must be positive.
Retries and timeout buffer may be zero, but cannot be negative.

All traversal, route, Hello, deletion, discovery, ACK and blacklist durations
must be positive and fit simulation time. The full exponential retry schedule
must also fit. Broadcast jitter samples must lie between zero and ``maxJitter``.
Periodic jitter and its samples must lie between zero and their declared maximum,
which cannot exceed half ``helloInterval``.

With Hello enabled, ``activeRouteTimeout`` must exceed its loss interval.
``deletePeriod`` must cover active timeout and any enabled Hello-loss interval.
``myRouteTimeout`` must be at least twice ``pathDiscoveryTime``. Its new default
is the maximum of that lower bound and twice active timeout.

The blacklist default now covers the configured unthrottled search, including
retained-hop starts and exponential retries. Explicit positive blacklist
overrides remain supported. Rate-limit queueing can extend discovery beyond
this bound. No existing valid single-interface configuration needs a new
parameter, but overrides violating these bounds must be corrected.
