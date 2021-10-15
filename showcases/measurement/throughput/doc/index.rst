Measuring Channel Throughput
============================

Goals
-----

In this example we explore the channel throughput statistics of wired and wireless
transmission mediums.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/throughput <https://github.com/inet-framework/inet/tree/master/showcases/measurement/throughput>`__

The Model
---------

The channel throughput is measured by observing the packets which are transmitted
through the transmission medium over time. For both wired and wireless channels,
the throughput is measured for any pair of communicating network interfaces,
separately for both directions.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/Results.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ChannelThroughputMeasurementShowcase.ned <../ChannelThroughputMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

