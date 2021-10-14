Measuring Packet Jitter
=======================

Goals
-----

In this example we explore the various packet jitter statistics of application modules.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/jitter <https://github.com/inet-framework/inet-showcases/tree/master/measurement/jitter>`__

The Model
---------

The packet jitter is measured in several different forms. The instantaneous
packet delay variation is the difference between the packet delay of successive
packets. Packet jitter is also often used for the variance of packet delay or
simply for the packet delay difference compared to the mean value.

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

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`JitterMeasurementShowcase.ned <../JitterMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

