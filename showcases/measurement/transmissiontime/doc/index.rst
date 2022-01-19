Measuring Transmission Time
===========================

Goals
-----

In this example we explore the packet transmission time statistics of network
interfaces for wired and wireless transmission mediums.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/transmissiontime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/transmissiontime>`__

The Model
---------

The packet transmission time is measured from the moment the beginning of the
physical signal encoding the packet leaves the network interface up to the moment
the end of the same physical signal leaves the same network interface. This time
usually equals with the packet reception time that is measured at the receiver
network interface from the beginning to the end of the physical signal. The
exception would be when the receiver is moving relative to the transmitter with
a relatively high speed compared to the propagation speed of the physical signal,
but it is rarely the case in communication network simulation.

Packet transmission time is measured from the beginning of the
physical signal encoding the packet leaves the network interface up to the moment
the end of the same physical signal leaves the same network interface. 

Packet transmission time is the time difference between the start and th end of the physical
signal transmission on the outgoing interface.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

.. **TODO** should the packet length be variable ? so the transmission time is not the same

Here are the results:

.. figure:: media/TransmissionTimeHistogram.png
   :align: center

.. figure:: media/TransmissionTimeVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`TransmissionTimeMeasurementShowcase.ned <../TransmissionTimeMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

