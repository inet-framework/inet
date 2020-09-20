Step 2. BGP Scenario with E-BGP session only
============================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step2.png
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

[explanation]

Sources: :download:`BGP_Topology_1.ned <../BGP_Topology_1.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_EBGP.xml <../OSPFConfig_EBGP.xml>`,
:download:`BGPConfig_EBGP.xml <../BGPConfig_EBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
