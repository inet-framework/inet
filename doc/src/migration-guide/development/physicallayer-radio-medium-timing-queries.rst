Radio medium timing queries
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom radio media must implement ``getTransmissionTimingSnapshot()`` and ``computePrecedingSignalGap()``. These queries supply copied physical endpoints from actual transmissions and cached arrivals. Unavailable endpoints produce an absent result. The snapshot must not retain transmission, arrival, or radio pointers. Communication caches must implement the const ``findCachedArrival()`` query without insertion or other mutation.

``SignalTimeInd`` now carries the medium's transmission identifier. The receiver supplies this local indication when it accepts a transmission. Consumers can associate later frame delivery with the corresponding physical timing evidence. The identifier is local metadata; it adds no transmitted field.
