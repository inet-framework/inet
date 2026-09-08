ICMPv6 Checksum Computation
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In ``"computed"`` checksum mode, the static ``Icmpv6::insertChecksum()`` now
only zeroes the checksum field and records the mode. The ``Icmpv6`` module
writes the value from a post-routing hook of the ``Ipv6`` module named by its
``networkProtocolModule`` parameter, once the source address of the datagram is
known. Code that builds an ICMPv6 message and sends it through ``Ipv6`` needs
no change. Code that sends such a message without passing it through ``Ipv6``
must compute the checksum itself.
