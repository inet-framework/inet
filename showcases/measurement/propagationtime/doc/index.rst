Measuring Propagation Time
==========================

Goals
-----

In this example we explore the channel propagation time statistics for wired and
wireless transmission mediums.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/propagationtime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/propagationtime>`__

The Model
---------

The packet propagation time is measured from the moment the beginning of a physical
signal encoding the packet leaves the transmitter network interface up to the moment
the beginning of the same physical signal arrives at the receiver network interface.
This time usually equals with the same difference measured for the end of the physical
signal. The exception would be when the receiver is moving relative to the transmitter
with a relatively high speed compared to the propagation speed of the physical signal,
but it is rarely the case in communication network simulation.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/PropagationTime.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PropagationTimeMeasurementShowcase.ned <../PropagationTimeMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

