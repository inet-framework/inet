Step 4. BGP Scenario with I-BGP over directly-connected BGP speakers
====================================================================

Goals
-----

This step introduces Internal BGP (I-BGP). The topology consists of three
Autonomous Systems: AS 64500 (RA), AS 64600 (RB), and AS 64700 (RC).

The key setup is as follows:

- **E-BGP sessions**: RA4 <-> RB1 and RB2 <-> RC1.
- **I-BGP session**: RB1 <-> RB2.
- RB1 and RB2 are directly connected via their ``eth1`` interfaces.

The goal is to demonstrate how I-BGP is used to carry routing information across
an AS. However, this step also highlights a common issue with I-BGP: the
``NEXT_HOP`` attribute behavior.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_2.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_2.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step4
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_IBGP.xml
   :language: xml

Results
~~~~~~~

Running the simulation reveals an important BGP constraint. When an E-BGP router
(like RB1) learns a route and advertises it to an I-BGP peer (like RB2), it does
not change the ``NEXT_HOP`` attribute of the route by default.

Observations in ``step4.rt``:

- RB1 learns RA's network (10.0.0.0/30) and correctly installs it with RA4 as the next hop.
- RB1 passes this advertisement to RB2 via their I-BGP session.
- **RB2 receives the route but does not install it.**

The reason for this is that the next hop for the RA routes remains RA4's
interface address (192.168.0.6). Since RB2 belongs to AS 64600 and has no
interior route to the RA4-RB1 external subnet, the next hop is considered
unreachable. For a BGP route to be installed in the IP routing table, its next
hop must be resolvable via the existing routing table.

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_IBGP.xml <../OSPFConfig_IBGP.xml>`,
:download:`BGPConfig_IBGP.xml <../BGPConfig_IBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
