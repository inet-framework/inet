Step 15. Set advertisement of a network to 'false'
==================================================

Goals
-----

The goal of this step is to demonstrate how to suppress the advertisement of a
network using the ``advertise="false"`` attribute in the OSPF configuration.

By default, OSPF routers advertise all networks configured in their areas. However,
in some scenarios, you may want to prevent a specific network from being advertised
to other areas. This can be achieved by setting the ``advertise`` attribute to
``false`` for a specific ``AddressRange`` in the OSPF area configuration. When this
is set, the ABR will not generate a Summary LSA for that network, effectively hiding
it from routers in other areas.

Configuration
~~~~~~~~~~~~~

This configuration is based on step 14. The only change is in the OSPF configuration
file, where the network connected to **R5**'s eth0 interface (192.168.22.0/30, which
includes **host3**) is configured with ``advertise="false"``.

.. figure:: media/OSPF_AreaTest.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step15
   :end-before: ------

The OSPF configuration shows the ``advertise="false"`` attribute on line 27:

.. literalinclude:: ../ASConfig_Area_NoAdvertisement.xml
   :language: xml

Results
~~~~~~~

When ``advertise="false"`` is set for the R5->switch3 network (192.168.22.0/30):

1.  **R5** still learns about this directly connected network and installs it in its own routing table.

2.  **R4** (the ABR for Area 0.0.0.2) **does not generate a Summary LSA** for the 192.168.22.0/30
    network to advertise it into Area 0.0.0.0 or into other areas.

3.  As a result, routers in Area 0.0.0.1 (**R1**, **R2**, **R3**) **do not learn** about the
    192.168.22.0/30 network and cannot reach **host3**.

4.  Routers in Area 0.0.0.1 can still reach the 192.168.22.4/30 network (the R4-R5 link),
    because that network is still being advertised normally.

Comparing the routing tables from Step 14 and Step 15 confirms that the 192.168.22.0/30
network is absent from routers in Area 0. 0.0.1 in this step, while it was present in Step 14.

This feature is useful when you want to keep certain networks local to an area and prevent
them from being visible to the rest of the OSPF domain.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_AreaTest.ned <../OSPF_AreaTest.ned>`,
:download:`ASConfig_Area_NoAdvertisement.xml <../ASConfig_Area_NoAdvertisement.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
