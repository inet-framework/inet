Effects of Time Synchronization on Time-Aware Shaping
=====================================================

Goals
-----

In this example we demonstrate how time synchronization affect end-to-end delay
in network that is using time-aware traffic shaping.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/gptpandtas <https://github.com/inet-framework/inet-showcases/tree/master/tsn/combiningfeatures/gptpandtas>`__

The Model
---------

Here is the network:

.. .. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :align: center
   :language: ini

Normal Operation
----------------

In this configuration time synchronization is operating properly. The end-to-end
delay is a constant low value.

Link Failure of Master Clock
----------------------------

In this configuration time synchronization stops due to a link failure at the
primary master clock. After the clock divergence grows above a certain value the
end-to-end delay suddenly increases dramatically. The reason is that frames often
wait for the next gate scheduling cycle because they miss the allocated time slot
due to improperly synchronized clocks.

Failover to Hot-Standby Clock
-----------------------------

In this configuration the hot-standby master clock failover happens and the time
synchronization continues to keep the difference of clocks in the network below
the required limit.

Results
-------

The following video shows the behavior in Qtenv:

.. video:: media/behavior.mp4
   :align: center
   :width: 90%

Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`GptpAndTasShowcase.ned <../GptpAndTasShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

