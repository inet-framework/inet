Measuring Queueing Time
=======================

Goals
-----

In this example we explore the queueing time statistics of queue modules of network interfaces.
The queueing time is measured from the moment a packet is enqueued up to the moment the same
packet is dequeued from the queue.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/queueingtime <https://github.com/inet-framework/inet-showcases/tree/master/measurement/queueingtime>`__

The Model
---------

TODO

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

The following video shows the behavior in Qtenv:

.. .. video:: media/behavior.mp4
   :align: center
   :width: 90%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`QueueingTimeMeasurementShowcase.ned <../QueueingTimeMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

