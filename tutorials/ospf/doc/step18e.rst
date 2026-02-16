Step 18e. Address Forwarding
============================

Goals
-----

The goal of this step is to demonstrate the use of the Forwarding Address field in AS-External
LSAs.

By default, when an ASBR advertises an external route, the Forwarding Address is set to 0.0.0.0,
meaning traffic should be sent to the ASBR itself. However, if the external network is reachable
through another router on a shared network, the ASBR can set the Forwarding Address to that
router's IP, allowing packets to be forwarded directly to the actual next-hop router, saving
a hop.

Configuration
~~~~~~~~~~~~~

This step demonstrates a scenario where R6 (not running OSPF) is the actual gateway to an
external network, and R5 (an ASBR) sets the Forwarding Address to R6's IP.

.. figure:: media/OSPF_Area_External_Forwarding.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18e
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Forwarding.xml
   :language: xml

Results
~~~~~~~

With the Forwarding Address configured:

1.  R5 (ASBR) advertises an external route with Forwarding Address set to R6's IP (192.168.22.3).

2.  Other OSPF routers learn this external route.

3.  When sending packets to the external destination, routers forward them directly to R6
    (via OSPF routing to 192.168.22.3), not to R5.

4.  This saves one hop compared to routing through R5 first.

5.  The ping from host1 to host3 demonstrates the direct forwarding path.

The Forwarding Address field allows OSPF to optimize forwarding when the ASBR is not the actual
next-hop to the external destination.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External_Forwarding.ned <../OSPF_Area_External_Forwarding.ned>`,
:download:`ASConfig_Area_ExternalRoute_Forwarding.xml <../ASConfig_Area_ExternalRoute_Forwarding.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
