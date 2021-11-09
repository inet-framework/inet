Measuring End-to-end Delay
==========================

Goals
-----

In this example we explore the end-to-end delay statistics of applications.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/endtoenddelay <https://github.com/inet-framework/inet/tree/master/showcases/measurement/endtoenddelay>`__

The Model
---------

The end-to-end delay is measured from the moment the packet leaves the source
application to the moment the same packet arrives at the destination application.

The end-to-end delay is measured by the ``meanBitLifeTimePerPacket`` statistic.
The statistic measures the lifetime of the packet, i.e. time from creation in the source application
to deletion in the destination application.

.. note:: The `meanBit` part refers to the statistic being defined per bit, and the result is the mean of the per-bit values of all bits in the packet.
   When there is no packet streaming or fragmentation in the network, the bits of a packet travel together, so they have the same lifetime value.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/EndToEndDelayHistogram.png
   :align: center

.. figure:: media/EndToEndDelayVector.png
   :align: center

**TODO** why the uptick ?

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`EndToEndDelayMeasurementShowcase.ned <../EndToEndDelayMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

