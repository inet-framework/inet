Measuring Residence Time
========================

Goals
-----

In this example we explore the packet residence time statistics of network nodes.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/residencetime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/residencetime>`__

The Model
---------

The packet residence time is measured from the moment a packet enters a network
node up to the moment the same packet leaves the network node. This statistic
is also collected for packets which are created and/or destroyed in network
nodes.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/SourceHistogram.png
   :align: center

.. figure:: media/SourceVector.png
   :align: center

.. figure:: media/SwitchHistogram.png
   :align: center

.. figure:: media/SwitchVector.png
   :align: center

.. figure:: media/DestinationHistogram.png
   :align: center

.. figure:: media/DestinationVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ResidenceTimeMeasurementShowcase.ned <../ResidenceTimeMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

