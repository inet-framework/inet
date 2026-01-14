Step 9. Using Network attribute to advertise specific networks
==============================================================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

The topology is the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables OSPF but does not set ``redistributeOspf``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step9
   :end-before: ------

The BGP configuration (``BGPConfig.xml``) uses ``Network`` elements to define the advertised prefixes for each AS:

.. literalinclude:: ../BGPConfig.xml
   :language: xml
   :start-at: <AS id="64500">
   :end-before: <!--bi-directional

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig.xml <../BGPConfig.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
