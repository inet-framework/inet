Step 12. Configure an interface as Passive
==========================================

Goals
-----

The goal of this step is to demonstrate the Passive interface mode in OSPF.

A passive interface in OSPF is one that:

*   **Does NOT** send or receive Hello packets (no adjacencies are formed)
*   **IS** advertised in Router LSAs (the network is visible to OSPF)

Passive mode is useful for stub networks (networks with only hosts, no other OSPF routers).
It reduces OSPF overhead by not running the protocol where it's unnecessary, while still
advertising the network so it's reachable from other parts of the OSPF domain.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 3. One of the router's interfaces is configured with
``interfaceMode="Passive"``.

.. figure:: media/Network.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step12
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Passive.xml
   :language: xml

Results
~~~~~~~

With the interface configured as Passive:

1.  No OSPF Hello packets are sent on the interface.
2.  No OSPF adjacencies can form on this interface.
3.  However, the network is still advertised in the router's Router LSA as a stub network.
4.  Other OSPF routers learn about this network and can route to it.
5.  Hosts on the passive network can communicate with the rest of the OSPF domain.

Passive mode is commonly used on interfaces connected to end-user networks or networks
where no other OSPF routers are expected, reducing unnecessary protocol overhead.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network.ned <../Network.ned>`,
:download:`ASConfig_Passive.xml <../ASConfig_Passive.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
