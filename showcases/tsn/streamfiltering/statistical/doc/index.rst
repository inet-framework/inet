Statistical Policing
====================

Goals
-----

In this example we combine a sliding window rate meter with a probabilistic packet
dropper to achieve a simple statistical policing.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet-showcases/tree/master/tsn/streamfiltering/statistical>`__

The Model
---------

In this configuration we use a sliding window rate meter in combination with a
statistical rate limiter. The former measures the thruput by summing up the
packet bytes over the time window, the latter drops packets in a probabilistic
way by comparing the measured datarate to the maximum allowed datarate.

Here is the network:

.. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/BGswitch1.png
   :align: center

.. figure:: media/BGswitch2.png
   :align: center

.. figure:: media/VIDswitch1.png
   :align: center

.. figure:: media/VIDswitch2.png
   :align: center

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. image:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`StatisticalPolicingShowcase.ned <../StatisticalPolicingShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

