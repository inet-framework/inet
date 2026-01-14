Step 8. BGP with OSPF and RIP redistribution
============================================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

The topology is the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` defines the protocol mix:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step8
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Redist.xml
   :language: xml

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
