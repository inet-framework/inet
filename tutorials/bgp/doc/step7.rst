Step 7. BGP with RIP redistribution
===================================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

The topology remains the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables RIP and disables OSPF:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7
   :end-before: ------

The BGP configuration uses ``redistributeRip = true``:

.. code-block:: ini

   *.R*.bgp.redistributeRip = true

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
