Step 11. Multi-hop E-BGP
========================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/Multihop_EBGP.png
   :width: 100%
   :align: center

.. literalinclude:: ../Multihop_EBGP.ned
   :start-at: network Multihop_EBGP
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step11
   :end-before: ------

.. literalinclude:: ../BGPConfig_MultiHopEBGP.xml
   :language: xml
   :start-at: <AS id="64520">
   :end-before: <!--bi-directional

Results
~~~~~~~

TODO elaborate

Sources: :download:`Multihop_EBGP.ned <../Multihop_EBGP.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_MultiHopEBGP.xml <../BGPConfig_MultiHopEBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
