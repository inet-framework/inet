Step 5. BGP Scenario with I-BGP over not directly-connected BGP speakers
========================================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step5.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_3.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Multi.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`
:download:`BGPConfig_Multi.xml <../BGPConfig_Multi.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
