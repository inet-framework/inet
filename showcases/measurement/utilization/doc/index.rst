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

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ChannelUtilizationMeasurementShowcase.ned <../ChannelUtilizationMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

