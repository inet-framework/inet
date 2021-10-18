Measuring Queueing Time
=======================

Goals
-----

In this example we explore the queueing time statistics of queue modules of
network interfaces.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/queueingtime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/queueingtime>`__

The Model
---------

The queueing time is measured from the moment a packet is enqueued up to the
moment the same packet is dequeued from the queue. Simple packet queue modules
are also often used to build more complicated queues such as a priority queue
or even traffic shapers. The queueing time statistics are automatically collected
for each of one these cases too.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/QueueingTimeHistogram.png
   :align: center

.. figure:: media/QueueingTimeVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`QueueingTimeMeasurementShowcase.ned <../QueueingTimeMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

