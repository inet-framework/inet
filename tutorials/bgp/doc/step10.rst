Step 10. LOCAL_PREF Attribute
=============================

Goals
-----

TODO elaborate

Configuration
~~~~~~~~~~~~~

This step uses the following network (``BGP_LOCAL_PREF.ned``):

.. figure:: media/BGP_LOCAL_PREF.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_LOCAL_PREF.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` defines the setup:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10
   :end-before: ------

.. literalinclude:: ../BGPConfig_LOCAL_PREF.xml
   :language: xml
   :start-at: <Neighbor address='20.0.0.2'
   :end-at: localPreference='600' />

Results
~~~~~~~

TODO elaborate

Sources: :download:`BGP_LOCAL_PREF.ned <../BGP_LOCAL_PREF.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_LOCAL_PREF.xml <../BGPConfig_LOCAL_PREF.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
