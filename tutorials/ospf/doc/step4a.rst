Step 4a. Advertising loopback interface
=======================================

Goals
-----

The goal of this step is to demonstrate how OSPF advertises loopback interfaces.

Loopback interfaces are virtual interfaces that are always up (unless administratively disabled).
In OSPF, loopback interfaces are treated as stub networks and are advertised as host routes
with a /32 netmask, regardless of the actual configured netmask. This behavior ensures that
the loopback address is reachable as a specific host route, which is useful for management
purposes and as a stable Router ID.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 4. R1 is configured with a second loopback interface (lo1),
and the OSPF configuration includes this interface.

.. figure:: media/RouterLSA.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step4a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Loopback.xml
   :language: xml

Results
~~~~~~~

R1's Router LSA now includes an additional stub network link for the loopback interface lo1:

*   The link is advertised as a /32 host route (255.255.255.255 netmask).
*   The metric is typically 0 or a small value for loopback interfaces.

Other routers in the area receive R1's Router LSA and install a /32 route to R1's loopback
address. This demonstrates that:

1.  Loopback interfaces are always advertised as /32 regardless of configuration.
2.  Loopback addresses provide stable, reachable IP addresses for router management.
3.  Loopback interfaces are commonly used as the Router ID source in OSPF.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RouterLSA.ned <../RouterLSA.ned>`,
:download:`ASConfig_Loopback.xml <../ASConfig_Loopback.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
