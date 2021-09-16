Frame Replication with Time-Aware Shaping
=========================================

Goals
-----

In this example we demonstrate how to automatically configure time-aware shaping
in the presence of frame replication.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/frerandtas <https://github.com/inet-framework/inet-showcases/tree/master/tsn/combiningfeatures/frerandtas>`__

The Model
---------

The network contains a source and a destination node and five switches.

Here is the network:

.. .. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :align: center
   :language: ini

Results
-------

The following video shows the behavior in Qtenv:

.. video:: media/behavior.mp4
   :align: center
   :width: 90%

Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`FrerAndTasShowcase.ned <../FrerAndTasShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

