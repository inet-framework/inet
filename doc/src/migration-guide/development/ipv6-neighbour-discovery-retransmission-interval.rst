IPv6 Neighbour Discovery Retransmission Interval
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`Ipv6InterfaceData` holds the RFC 4861 RetransTimer once now. The node
constant accessors ``_getRetransTimer()`` and ``_setRetransTimer()`` are
removed; call ``getRetransTimer()`` and ``setRetransTimer()`` instead. Their
type changes from ``uint`` seconds to ``simtime_t``, so code that passes a
number of seconds keeps compiling, and code that stored the result in an
integer must use :cpp:`simtime_t`.
