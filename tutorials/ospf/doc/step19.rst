Step 19. Default-route distribution in OSPF
===========================================

Goals
-----

The goal of this step is to demonstrate how OSPF can distribute a default route (0.0.0.0/0)
throughout the AS.

An ASBR can advertise a default route using an AS-External LSA. This allows routers throughout
the OSPF domain to learn a default route for reaching destinations outside the AS, without
needing specific routes to every external network.

Default routes are particularly useful for providing Internet connectivity in enterprise networks.

Configuration
~~~~~~~~~~~~~

This step configures an ASBR to advertise a default route into the OSPF domain.

.. figure:: media/OSPF_Default_Route_Distribution.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step19
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Default_Route.xml
   :language: xml

Results
~~~~~~~

When an ASBR advertises a default route:

1.  The ASBR generates an AS-External LSA with destination 0.0.0.0/0.

2.  This LSA is flooded throughout the OSPF AS.

3.  All routers install a default route pointing to the ASBR (or its Forwarding Address).

4.  Traffic to unknown destinations is forwarded to the ASBR for external routing.

5.  The routing tables show 0.0.0.0/0 routes pointing toward the ASBR.

Default route distribution simplifies configuration and reduces the number of external routes
that must be advertised into OSPF, particularly useful when the AS has limited external
connectivity points.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Default_Route_Distribution.ned <../OSPF_Default_Route_Distribution.ned>`,
:download:`ASConfig_Area_Default_Route.xml <../ASConfig_Area_Default_Route.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
