IPv6 Neighbour Discovery Reachable Time
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`Ipv6InterfaceData` holds the RFC 4861 ReachableTime once now. The node
constant accessors ``_getReachableTime()`` and ``_setReachableTime()`` are
removed; call ``getReachableTime()`` and ``setReachableTime()`` instead.
``getBaseReachableTime()`` and ``setBaseReachableTime()`` change from ``uint``
seconds to ``simtime_t``, and ``setBaseReachableTime()`` now draws a new
random ReachableTime when the value changes, as RFC 4861 Section 6.3.2
requires. The overload of ``generateReachableTime()`` with three arguments
takes the base as ``simtime_t``.
