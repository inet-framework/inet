Step 2a. Reroute after link breakage
=====================================

Goals
-----

The goal of this step is to demonstrate OSPF's ability to automatically reroute traffic
when a link fails.

OSPF continuously monitors link states. When a link fails, the affected router generates
a new LSA reflecting the topology change and floods it throughout the area. Other routers
update their LSDBs, rerun the SPF algorithm, and update their routing tables to use
alternative paths (if available).

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 1. The scenario manager script disconnects the link
between **R5** and **R4** at t=60s.

.. figure:: media/OspfNetwork.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2a
   :end-before: ------

Results
~~~~~~~

When the link between R5 and R4 breaks at t=70s:

1.  Both **R5** and **R4** detect the link down event on their respective interfaces.

2.  R5 and R4 each generate new Router LSAs that no longer include the link between them.

3.  These LSAs are flooded throughout Area 0.0.0.0.

4.  All routers receive the updated LSAs, update their LSDBs, and rerun the SPF algorithm.

5.  Routers that were using the R5-R4 link in their shortest paths now compute alternative
    paths. For example, traffic from R1 to networks behind R4 now routes through R2 instead
    of through R5.

6.  Applications may experience brief packet loss during the reconvergence
    period, but connectivity is restored once OSPF converges on the new
    topology. In this example, no ping packet is lost.

The routing table changes demonstrate OSPF's resilience: when a link fails, the protocol
automatically discovers and uses alternative paths, ensuring continued net work connectivity.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OspfNetwork.ned <../OspfNetwork.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
