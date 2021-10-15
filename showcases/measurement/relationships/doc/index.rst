Understanding Measurement Relationships
=======================================

Goals
-----

In this example we explore the relationships between various measurements that
are presented in the measurement showcases.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/relationships <https://github.com/inet-framework/inet/tree/master/showcases/measurement/relationships>`__

The Model
---------

The end-to-end delay measured between two applications can be thought of as a sum
of different time categories such as queueing time, processing time, transmission
time, propagation time, and so on. Moreover, each one of these specific times can
be further split up between different network nodes, network interface or even
smaller submodules.

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

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MeasurementRelationshipsShowcase.ned <../MeasurementRelationshipsShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

