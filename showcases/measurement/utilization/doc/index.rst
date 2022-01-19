Measuring Channel Utilization
=============================

Goals
-----

In this example we explore the channel utilization statistics of wired and wireless
transmission mediums.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/utilization <https://github.com/inet-framework/inet/tree/master/showcases/measurement/utilization>`__

The Model
---------

The channel utilization statistic is measured by observing the packets which
are transmitted through the transmission medium over time. For both wired and
wireless channels, the utilization is measured for any pair of communicating
network interfaces, separately for both directions. This statistic expresses
the relative usage of the channel with a value between 0 and 1, where 0 means
the channel is not used at all and 1 means the channel is fully utilized.

Channel utilization is a statistic of transmitter modules, such as the :ned:`PacketTransmitter` in :ned:`EthernetPhyLayer`.
The channel utilization is related to channel throughput in the sense that 
utilization is the ratio of throughput to channel datarate. By default, channel utilization
is calculated for the past 0.1s or the last 100 packets, whichever comes first.

.. The parameters of the window, such as the window interval, are configurable from the ini file, as ``module.statistic.parameter``. For example:

These values are configurable from the ini file with the :par:`interval` ([s]) and :par:`numValueLimit` parameters, as ``module.statistic.parameter``.
For example:

.. code-block:: ini

   *.host.eth[0].phyLayer.transmitter.utilization.interval = 0.2s

Here is the network:

.. figure:: media/Network.png
   :align: center

The hosts are connected by 100 Mbps Ethernet.

We configure the hosts to use the layered Ethernet model, and the source host to generate UDP packets with around 48 Mbps. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

We measure the channel utilization in the source host (the ``source.eth[0].phy.transmitter.utilization`` statistic). Here are the results:

.. figure:: media/ChannelUtilizationHistogram.png
   :align: center

.. figure:: media/ChannelUtilizationVector.png
   :align: center

.. note:: This is the channel utilization in the ``source -> destination`` direction. Utilization in the other direction on this link could be measured
   with the utilization statistic in ``destination``, but in this case there is no traffic in that direction.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ChannelUtilizationMeasurementShowcase.ned <../ChannelUtilizationMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

