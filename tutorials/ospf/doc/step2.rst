Step 2. Change link cost
========================

Goals
-----

The goal of this step is to demonstrate how manually configuring OSPF interface costs
affects route selection.

OSPF uses link costs to determine the best path to each destination. By default, the cost
is calculated based on the link bandwidth, but it can also be manually configured. When
multiple paths exist to a destination, OSPF selects the path with the lowest total cost.
Changing the cost of a link can influence which path OSPF chooses.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 1. The OSPF configuration manually overrides the output
cost of R1's ppp1 interface using ``ASConfig_cost.xml``.

.. figure:: media/OspfNetwork.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_cost.xml
   :language: xml

Results
~~~~~~~

The modified cost on R1's ppp1 interface affects routing decisions:

1.  R1's ppp1 interface is assigned a higher cost than the default.

2.  This influences the path selection for traffic originating from hosts behind R1.

3.  In Step 1, traffic from host0 to host6 used the path through R5->R4.
    With the increased cost on R1's ppp1, OSPF now prefers the alternative path
    through R2.

4.  Note that OSPF costs are directional. The modified cost affects routes computed by R1
    (outbound direction) but not routes computed by other routers for reaching R1.

The routing table changes show how adjusting link costs allows network administrators to
influence traffic engineering and load distribution in OSPF networks.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OspfNetwork.ned <../OspfNetwork.ned>`,
:download:`ASConfig_cost.xml <../ASConfig_cost.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
