Step 3. BGP Path Attributes
===========================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step3.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_1a.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_FullAS.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`BGP_Topology_1a.ned <../BGP_Topology_1a.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_FullAS.xml <../OSPFConfig_FullAS.xml>`,
:download:`BGPConfig_FullAS.xml <../BGPConfig_FullAS.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
