Step 1. BGP Basic Topology
==========================

Goals
-----

This step introduces a basic BGP configuration. The network consists of two
Autonomous Systems (AS): AS 64520 and AS 64530.

- AS 64520 contains two routers, RA1 and RA2.
- AS 64530 contains two routers, RB1 and RB2.

An External BGP (E-BGP) session is established between RA1 and RB1. The primary
goal is to demonstrate how BGP peers exchange routing information. In this
configuration, the ``Network`` attribute in the XML configuration is used to
manually specify which networks should be advertised by each router:

- RA1 advertises the network ``10.0.0.8/30``.
- RB1 advertises the network ``10.0.0.0/30``.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Basic_Topology.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Basic_Topology.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step1
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Basic.xml
   :language: xml

Results
~~~~~~~

Upon running the simulation, the BGP speakers RA1 and RB1 perform the following
sequence:

1. They establish a TCP connection between their external interfaces.
2. They exchange BGP OPEN messages to negotiate session parameters.
3. Once the session is in the Established state, they exchange BGP UPDATE messages to advertise their configured networks.

In the routing table log (``step1.rt``), we can observe the impact of these exchanges:

- At approximately 6.5s, RB1 installs a route to ``10.0.0.8/30`` with the next hop set to RA1 (``10.0.0.6``).
- Simultaneously, RA1 installs a route to ``10.0.0.0/30`` with the next hop set to RB1 (``10.0.0.5``).

Note that in this basic step, RA2 and RB2 do not learn these routes because
I-BGP (Internal BGP) is not yet configured, and BGP routes are not redistributed
into any interior gateway protocol.

Sources: :download:`BGP_Basic_Topology.ned <../BGP_Basic_Topology.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_Basic.xml <../BGPConfig_Basic.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
