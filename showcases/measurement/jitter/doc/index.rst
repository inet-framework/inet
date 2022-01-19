Measuring Packet Delay Variation
================================

Goals
-----

In this example we explore the various packet delay variation (also known as packet jitter) statistics of application modules.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/jitter <https://github.com/inet-framework/inet/tree/master/showcases/measurement/jitter>`__

The Model
---------

The packet delay variation is measured in several different forms:

- *Instantaneous packet delay variation*: the difference between the packet delay of successive packets (``packetJitter`` statistic)
- *Variance of packet delay* (``packetDelayVariation`` statistic)
- *Packet delay difference compared to the mean value* (``packetDelayDifferenceToMean`` statistic)

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/PacketJitterHistogram.png
   :align: center

.. figure:: media/PacketJitterVector.png
   :align: center

.. figure:: media/PacketDelayDifferenceToMeanHistogram.png
   :align: center

.. figure:: media/PacketDelayDifferenceToMeanVector.png
   :align: center

.. figure:: media/PacketDelayVariationHistogram.png
   :align: center

.. figure:: media/PacketDelayVariationVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`JitterMeasurementShowcase.ned <../JitterMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

