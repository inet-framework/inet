Step 4. BGP Scenario with I-BGP over directly-connected BGP speakers
====================================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step4.png
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

[explanation]

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_IBGP.xml <../OSPFConfig_IBGP.xml>`,
:download:`BGPConfig_IBGP.xml <../BGPConfig_IBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
