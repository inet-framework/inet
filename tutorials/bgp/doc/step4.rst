Step 4. BGP Scenario with I-BGP over directly-connected BGP speakers
======================================================================

Goals
-----

The goal of this step is to introduce Internal BGP (I-BGP) for distributing external
routes within an autonomous system.

So far, we have seen E-BGP sessions between routers in different autonomous systems.
However, when an AS has multiple border routers, they need a way to share the external
routes they learn from their E-BGP peers. This is where I-BGP comes in.

**I-BGP** (Internal BGP) is used to distribute BGP routes within an autonomous system.
Key differences between I-BGP and E-BGP:

*   **AS_PATH**: I-BGP does not modify the AS_PATH attribute (no AS prepending)
*   **NEXT_HOP**: By default, I-BGP preserves the NEXT_HOP from E-BGP advertisements
*   **Route propagation**: I-BGP routers do not re-advertise routes learned via I-BGP
    to other I-BGP peers (to prevent loops)
*   **Full mesh requirement**: Because of the no-propagation rule, all I-BGP speakers
    within an AS typically need to be fully meshed

This step demonstrates a common issue with I-BGP: the NEXT_HOP attribute points to
an address that may not be reachable within the AS, causing connectivity problems.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_2.png
   :width: 100%
   :align: center

The network has three autonomous systems:

*   **AS 64500** (router RA4 with OSPF): Border router to AS 64600
*   **AS 64600** (routers RB1 and RB2): Two border routers with I-BGP between them
*   **AS 64700** (router RC1 with OSPF): Border router to AS 64600

AS 64600 has two border routers:

*   **RB1**: E-BGP session with RA4, I-BGP session with RB2
*   **RB2**: E-BGP session with RC1, I-BGP session with RB1

The I-BGP session allows RB1 and RB2 to exchange routes learned from their respective
E-BGP peers. In this step, RB1 and RB2 are directly connected.

.. literalinclude:: ../BGP_Topology_2.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step4]
   :end-before: ------

Note that only RA4 and RC1 run OSPF (within their respective ASes), while RB1 and
RB2 do not run an IGP between them (``*.RB{1,2}.hasOspf = false``).

The BGP configuration:

.. literalinclude:: ../BGPConfig_IBGP.xml
   :language: xml

The configuration shows that RB1 and RB2 are in the same AS (64600) but no explicit
I-BGP session is defined. The I-BGP session is automatically established because both
routers advertise the same network (20.0.0.0/30) indicating they are I-BGP peers.

Results
~~~~~~~

The simulation reveals a problem with the default I-BGP behavior:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF runs in AS 64500 and AS 64700
    *   Default routes are distributed within each AS

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB2-RC1
    *   I-BGP session: RB1-RB2

3.  **Route Exchange** (t≈60-62s):

    *   RA4 advertises AS 64500's networks to RB1 via E-BGP
    *   RC1 advertises AS 64700's networks to RB2 via E-BGP

4.  **I-BGP Route Distribution**:

    *   RB1 learns routes to AS 64500 with NEXT_HOP = 192.168.0.2 (RA4's interface)
    *   RB1 advertises these routes to RB2 via I-BGP
    *   **Problem**: RB2 receives the routes with NEXT_HOP = 192.168.0.2
    *   RB2 cannot reach 192.168.0.2 because there is no IGP running between RB1 and RB2
    *   Similarly, RB1 cannot reach the NEXT_HOP for routes learned from RC1

5.  **Connectivity Failure**:

    *   Although BGP routes are exchanged, packets cannot be forwarded correctly
    *   The routers have BGP routes but cannot resolve the NEXT_HOP addresses
    *   This breaks end-to-end connectivity between AS 64500 and AS 64700

This demonstrates a fundamental issue: I-BGP preserves the NEXT_HOP attribute from
E-BGP, but if the I-BGP routers cannot reach that NEXT_HOP (due to lack of IGP or
reachability), the routes are unusable.

**Solution**: Use the **next-hop-self** feature, demonstrated in Step 4a.

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_IBGP.xml <../OSPFConfig_IBGP.xml>`,
:download:`BGPConfig_IBGP.xml <../BGPConfig_IBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
