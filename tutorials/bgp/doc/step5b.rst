Step 5b. Enabling BGP on RB3
============================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

This step extends Step 5.

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5b
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Multi_RB3.xml
   :language: xml

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_Multi_RB3.xml <../BGPConfig_Multi_RB3.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
