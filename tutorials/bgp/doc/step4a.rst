Step 4a. Enable nextHopSelf on RB1 and RB2
==========================================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

This step uses the same network as Step 4 (``BGP_Topology_2.ned``).

.. figure:: media/BGP_Topology_2.png
   :width: 100%
   :align: center

The BGP configuration uses a specific XML file:

.. literalinclude:: ../BGPConfig_IBGP_NextHopSelf.xml
   :language: xml

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_IBGP_NextHopSelf.xml <../BGPConfig_IBGP_NextHopSelf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
