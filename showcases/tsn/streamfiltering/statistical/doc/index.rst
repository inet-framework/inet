Statistical Policing
====================

Goals
-----

In this example we combine a sliding window rate meter with a probabilistic packet
dropper to achieve a simple statistical policing.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/statistical>`__

The Model
---------

In this configuration we use a sliding window rate meter in combination with a
statistical rate limiter. The former measures the thruput by summing up the
packet bytes over the time window, the latter drops packets in a probabilistic
way by comparing the measured datarate to the maximum allowed datarate.

Here is the network:

.. figure:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/ClientApplicationTraffic.png
   :align: center

.. figure:: media/BestEffortTrafficClass.png
   :align: center

.. figure:: media/VideoTrafficClass.png
   :align: center

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/794>`__ page in the GitHub issue tracker for commenting on this showcase.

