Step 4a. Enable nextHopSelf on RB1 and RB2
==========================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

Same network as the previous step.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step4a
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_IBGP_NextHopSelf.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_IBGP_NextHopSelf.xml <../BGPConfig_IBGP_NextHopSelf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
