Step 1. BGP Basic Topology
==========================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step1.png
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

[explanation]

Sources: :download:`BGP_Basic_Topology.ned <../BGP_Basic_Topology.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_Basic.xml <../BGPConfig_Basic.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
