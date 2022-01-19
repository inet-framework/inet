Measuring Data Rate
===================

Goals
-----

In this example we explore the data rate statistics of application, queue, and
filter modules inside network nodes.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/datarate <https://github.com/inet-framework/inet/tree/master/showcases/measurement/datarate>`__

The Model
---------

The data rate is measured by observing the packets as they are passing through
over time at a certain point in the node architecture. For example, an application
source module produces packets over time and this process has its own data rate.
Similarly, a queue module enqueues and dequeues packets over time and both of
these processes have their own data rate. These data rates are different, which
in turn causes the queue length to increase or decrease over time.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/source_outgoing.png
   :align: center

.. figure:: media/destination_incoming.png
   :align: center

.. figure:: media/switch_incoming.png
   :align: center

.. figure:: media/switch_outgoing.png
   :align: center

.. **TODO** why does the switch has more datarate ? protocol headers?

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`DataRateMeasurementShowcase.ned <../DataRateMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

