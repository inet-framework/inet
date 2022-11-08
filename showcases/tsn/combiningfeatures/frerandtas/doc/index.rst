Frame Replication with Time-Aware Shaping
=========================================

Goals
-----

In this example we demonstrate how to automatically configure time-aware shaping
in the presence of frame replication.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/frerandtas <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/frerandtas>`__

The Model
---------

The network contains a source and a destination node and five switches.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

Here is the number of packets received and sent:

.. figure:: media/packetsreceivedsent.svg
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`FrerAndTasShowcase.ned <../FrerAndTasShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/783>`__ page in the GitHub issue tracker for commenting on this showcase.

