Step 2. BGP Scenario with E-BGP session only
============================================

Goals
-----

This step demonstrates BGP operating in a more realistic scenario where an
Interior Gateway Protocol (IGP) is used within each Autonomous System.
Specifically, OSPF is configured within AS 64500 (RA routers) and AS 64600 (RB
routers).

The key objectives are:

- Establishing an E-BGP session between the border routers RA4 and RB1.
- Redistributing OSPF routes into BGP so that internal networks of one AS can be reached from the other.

In ``omnetpp.ini``, redistribution is enabled using the ``redistributeOspf`` parameter:

.. code-block:: ini

   *.RA4.bgp.redistributeOspf = "O IA"
   *.RB1.bgp.redistributeOspf = "O IA"

This tells the BGP module to advertise routes learned via OSPF (both intra-area
'O' and inter-area 'IA' routes) to its BGP peers.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_1.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_1.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_EBGP.xml
   :language: xml

Results
~~~~~~~

The simulation proceeds in two phases:

1. **IGP Convergence**: Initially, OSPF converges within each AS. You can see
   OSPF HELLO and LSU packets being exchanged, and the routing tables filling
   with internal routes.
2. **BGP Session Establishment**: After a configured delay (``startDelay = 60s``
   in ``BGPConfig_EBGP.xml``), RA4 and RB1 initiate their BGP session.

Once the E-BGP session is established at approximately 61.5s:

- RA4 receives BGP UPDATE messages from RB1 containing all the internal networks of AS 64600 (the ``20.0.0.0/30`` subnets).
- RB1 receives BGP UPDATE messages from RA4 containing all the internal networks of AS 64500 (the ``10.0.0.0/30`` subnets).

Checking ``step2.rt`` reveals that both border routers now have complete
reachability to the other AS's internal networks via the BGP peer. However, note
that internal routers (like RA1 or RB2) still cannot reach the other AS because
the BGP-learned routes have not been redistributed back into OSPF.

Sources: :download:`BGP_Topology_1.ned <../BGP_Topology_1.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_EBGP.xml <../OSPFConfig_EBGP.xml>`,
:download:`BGPConfig_EBGP.xml <../BGPConfig_EBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
