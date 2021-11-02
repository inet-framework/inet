Effects of Time Synchronization on Time-Aware Shaping
=====================================================

Goals
-----

In this example we demonstrate how time synchronization affect end-to-end delay
in network that is using time-aware traffic shaping.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/gptpandtas <https://github.com/inet-framework/tree/master/showcases/tsn/combiningfeatures/gptpandtas>`__

The Model
---------

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the ``General`` configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :end-before: NormalOperation

Normal Operation
----------------

In this configuration time synchronization is operating properly. The end-to-end
delay is a constant low value.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NormalOperation
   :end-before: LinkFailure

Here are the results:

.. figure:: media/primaryclocktimedomain.svg
   :align: center
   :width: 100%

.. figure:: media/hotstandbytimedomain.svg
   :align: center
   :width: 100%

.. figure:: media/bridgesendstations.svg
   :align: center
   :width: 100%

Link Failure of Master Clock
----------------------------

In this configuration time synchronization stops due to a link failure at the
primary master clock. After the clock divergence grows above a certain value the
end-to-end delay suddenly increases dramatically. The reason is that frames often
wait for the next gate scheduling cycle because they miss the allocated time slot
due to improperly synchronized clocks.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: LinkFailure
   :end-before: Failover

Here are the results:

.. figure:: media/linkfailure_primarymaster.svg
   :align: center
   :width: 100%

.. figure:: media/linkfailure_hotstandby.svg
   :align: center
   :width: 100%

.. figure:: media/linkfailure_bridgesendstations.svg
   :align: center
   :width: 100%

.. figure:: media/linkfailure_endtoenddelay.png
   :align: center

Failover to Hot-Standby Clock
-----------------------------

In this configuration the hot-standby master clock failover happens and the time
synchronization continues to keep the difference of clocks in the network below
the required limit.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Failover

Here are the results:

.. figure:: media/failover_primarymaster.svg
   :align: center
   :width: 100%

.. figure:: media/failover_hotstandby.svg
   :align: center
   :width: 100%

.. figure:: media/failover_bridgesendstations.svg
   :align: center
   :width: 100%

.. figure:: media/failover_endtoenddelay.png
   :align: center

.. Results
   -------

   The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`GptpAndTasShowcase.ned <../GptpAndTasShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

