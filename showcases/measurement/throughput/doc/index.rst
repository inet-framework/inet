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

The channel throughput is measured by observing the packets that are transmitted
through the transmission medium over time. For both wired and wireless channels,
the throughput is measured for any pair of communicating network interfaces,
separately for both directions.

.. the throughput is measured as a sliding window, the parameters can be set in the ResultRecorder HOW?

Channel throughput is a statistic of transmitter modules, such as the :ned:`PacketTransmitter` in :ned:`EthernetPhyLayer`.
Throughput is measured with a sliding window. By default, the window is 0.1s or 100 packets, whichever comes first.
The parameters of the window, such as the window interval, are configurable from the ini file, as ``module.statistic.parameter``. For example:

.. code-block:: ini

   *.host.eth[0].phyLayer.transmitter.throughput.interval = 0.2s

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/throughput.png
   :align: center

The frequency of data points is denser than 0.1s, so the statistic is emitted more frequently, after 100 packets.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ChannelThroughputMeasurementShowcase.ned <../ChannelThroughputMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

