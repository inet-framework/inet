Implementing ``IArp``
~~~~~~~~~~~~~~~~~~~~~

:cpp:`IArp` has a new pure virtual method, which :cpp:`DhcpClient` calls
before it takes a granted address:

.. code-block:: c++

   virtual void sendArpProbe(const NetworkInterface *ie, MacAddress srcAddr, Ipv4Address probedAddr) = 0;

An :cpp:`IArp` implementation outside INET no longer compiles until it
overrides this method. An implementation that exchanges ARP packets sends an
RFC 5227 ARP Probe and emits ``arpAddressConflictDetected`` if somebody
answers; :cpp:`Arp` shows how. An implementation that resolves addresses
without packets has nobody to ask. It overrides the method with an empty body,
as :cpp:`GlobalArp` does, and the client then takes the address.
