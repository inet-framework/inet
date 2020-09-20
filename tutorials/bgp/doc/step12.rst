Step 12. Multi-hop E-BGP
========================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step12.png
   :width: 100%
   :align: center

.. literalinclude:: ../Multihop_EBGP.ned
   :start-at: network Multihop_EBGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step11
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_MultiHopEBGP.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources: :download:`Multihop_EBGP.ned <../Multihop_EBGP.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_MultiHopEBGP.xml <../BGPConfig_MultiHopEBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
