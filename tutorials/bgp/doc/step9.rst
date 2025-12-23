Step 9. Using Network attribute to advertise specific networks
==============================================================

Goals
-----

This example shows how to advertise selective networks in BGP using the 'Network' attribute.

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the same network as the previous one.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step9
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig.xml <../BGPConfig.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
