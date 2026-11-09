AODV Local Repair
~~~~~~~~~~~~~~~~~

``useLocalRepair=true`` now enables bounded transit-only repair; its default is
still false. ``maxRepairTTL`` defaults to ``floor(0.3*netDiameter)`` and accepts
zero through ``netDiameter``. Zero disables eligibility. ``localAddTTL`` remains
nonnegative, with default 2. A missing reverse distance or a computed TTL above
``netDiameter``/255 declines repair rather than clamping it.

One request attempt uses the normal jitter and shared rate budget. Its reply wait
starts at actual send; creation time plus ``pathDiscoveryTime`` also bounds delay
and rate starvation. Member arrivals do not extend the deadline. An existing
originating search keeps its purpose. The failed MAC-owned packet is never queued;
only subsequent eligible traffic can join repair.

Aggregate repair uses U and D with numeric zero, retaining its counter locally.
An independent host route may release one member without finishing the aggregate
owner. A matching-prefix update finishes the owner, including when all members
have already left. Failure disposes all remaining attached queues without an
automatic originating retry. A later local packet can start ordinary discovery.
