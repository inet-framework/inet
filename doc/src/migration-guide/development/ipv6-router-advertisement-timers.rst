IPv6 Router Advertisement Timers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The router variables of :cpp:`Ipv6InterfaceData` that
``getAdvReachableTime()``, ``setAdvReachableTime()``, ``getAdvRetransTimer()``
and ``setAdvRetransTimer()`` access hold milliseconds now, the value of the
Router Advertisement field as RFC 4861 Sections 4.2 and 6.2.1 define it.
Their defaults, the macros ``IPv6_DEFAULT_ADV_REACHABLE_TIME`` and
``IPv6_DEFAULT_ADV_RETRANS_TIMER``, change from 3600 and 1 to 3600000 and
1000. Code that set the variables or used the macros in seconds must
multiply by 1000.
