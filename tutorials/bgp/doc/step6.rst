Step 6. BGP Scenario and using loopbacks
========================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step6789.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_4.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Redist.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
