Effects of Time Synchronization on Time-Aware Shaping
=====================================================

Goals
-----

In this showcase we demonstrate how time synchronization affects end-to-end delay
in a network that is using time-aware traffic shaping.

.. note:: This showcase builds upon the :doc:`/showcases/tsn/timesynchronization/clockdrift/doc/index`, 
   :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` and :doc:`/showcases/tsn/trafficshaping/timeawareshaper/doc/index` showcases.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/gptpandtas <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/gptpandtas>`__

The Model
---------

Overview
~~~~~~~~

The showcase contains a network that uses time-aware shaping and inaccurate local clocks in each network node.
Time-aware shaping requires synchronized time among all network nodes, so we also use gPTP for time synchronization.
This results in a constant end-to-end delay for the traffic. We examine how failure and recovery in time synchronization affect the delay.

.. [in this network that uses time-aware shaping]. 

.. **V1** If time synchronization fails for some reason, e.g., due to the primary master clock going offline, the end-to-end delay guarantees of time-aware shaping cannot be met anymore.
   However, time synchronization can continue and delay guarantees can be met if all network nodes switch over to a secondary master clock.

If time synchronization fails for some reason, such as the primary clock going offline, time-aware shaping cannot guarantee bounded delays any longer. 
Time synchronization can continue and delay guarantees can be met however, if all network nodes switch over to a secondary master clock.

To demonstrate this, we present three cases, with three configurations:

- **Normal operation**: time synchronization works, and the delay is constant (this is the same case as in the last configuration in the :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` showcase).
- **Failure of master clock**: the master clock disconnects from the network, and time is not synchronized anymore.
- **Failover to a secondary master clock**: the master clock disconnects from the network, but time synchronization can continue because network nodes switch to the secondary master clock.

The Configuration
~~~~~~~~~~~~~~~~~

All simulations in the showcase use the same network as in the :ref:`sh:tsn:timesync:gptp:redundancy` section of the `Using gPTP` showcase.
The network constains :ned:`TsnDevice` and :ned:`TsnClock` modules connected to a ring of switches (:ned:`TsnSwitch`):

.. figure:: media/Network.png
   :align: center

Most settings, such as traffic generation, time-aware shaping, clock drift, gPTP domains, and visualization, are shared between the three cases, 
and are specified in the ``General`` configuration. The following sections present these settings.

Traffic
+++++++

Traffic in the network consists of UDP packets sent between ``tsnDevice1`` and ``tsnDevice4``, and gPTP messages.
sent by all nodes. To generate the UDP application traffic, we configure ``tsnDevice1`` to send 10B UDP packets to ``tsnDevice2``:

.. literalinclude:: ../omnetpp.ini
   :start-at: tsnDevice1.numApps
   :end-at: tsnDevice1.app[0].source.clockModule
   :language: ini

Clocks and Clock Drift
++++++++++++++++++++++

Clocks are configured similarly to the :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` showcase, using :ned:`MultiClock` and 
:ned:`SettableClock` modules with constant drift oscillators. However, we configure the drift rate of the oscillator in ``tsnClock2`` 
a bit differently to make the failover effect in the third example simulation more visible:

.. literalinclude:: ../omnetpp.ini
   :start-at: oscillator.driftRate
   :end-at: **.oscillator.driftRate
   :language: ini

We specify a random drift rate for all oscillators in the second line, but override this setting for the oscillators of ``TsnClock2`` in 
the first line with a specific constant drift rate, in order to make the failover effect more visible.

Time-Aware Shaping
++++++++++++++++++

Here is the time-aware shaping part of the configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: hasEgressTrafficShaping
   :end-at: transmissionGate[1].clockModule
   :language: ini

We enable time-aware shaping in all switches. We configure two traffic classes: the higher priority class for gPTP messages, and 
the lower priority class for UDP packets. The gPTP messages are critical to maintain time synchronization in the network, so we 
assign them higher priority, and configure the time-aware shapers to not schedule gPTP messages, but forward them immediately 
(the corresponding gates are always open). Gates for the UDP packets are configured to be open 10% of the time, so the UDP traffic 
is limited to 10Mbps. The UDP traffic data rate is 10kbps, so there is a lot of room in the send window; this makes the setup more 
tolerant of clock drift between network nodes.

gPTP
++++

For time synchronization, we use the same clock and gPTP setup as in the Two Master Clocks Exploiting Network Redundancy configuration 
in the :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` showcase. To summarize, we have a primary and a hot-standby master clock
node (``tsnClock1`` and ``tsnClock2``, respectively). Each master clock has two gPTP time domains, and each gPTP domain disseminates 
timing information in the ring of switches in opposing directions. All bridge and slave nodes are part of all four time domains. With 
this setup, time synchronization is protected against the failure of one master clock, and the failure of any link between the switches. 
The two gPTP domains of the hot-standby master clock synchronize to the two domains of the primary master. The switches and the devices 
have four gPTP domains, and four up-to-date clocks during normal operation.

.. note:: Here is the gPTP configuration for reference:

   .. raw:: html

      <details>
      <summary><a>click arrow to open/close</a></summary>

   .. literalinclude:: ../omnetpp.ini
      :start-at: MultiClock
      :end-at: gptp.domain[3].syncInitialOffset
      :language: ini

   .. raw:: html

      </details>

In the next section, we examine the simulation results for the three cases.

Example Simulations and Results
-------------------------------

Normal Operation
~~~~~~~~~~~~~~~~

In this configuration, time synchronization is operating properly. The configuration for this case is empty (only the ``General`` configuration applies):

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NormalOperation
   :end-before: LinkFailure

Let us examine the clock drift (i.e., clock time difference to simulation time) and the end-to-end delay. The following chart displays the clock drift
for clocks in `gPTP time domain 0` (one of the gPTP domains of the primary master). This is the active gPTP time domain in all network nodes 
(i.e., all nodes keep time according to this gPTP time domain):

.. figure:: media/NormalOperation_domain0.png
   :align: center

Due to everything in the network operating properly, bridge and slave nodes periodically synchronize their clocks to the primary master node.

.. note:: The other three gPTP time domains are maintained simultaneously, but not used.

The next chart displays the clock drift for `gPTP time domain 2`, where timing information originates from the hot-standby master clock. All bridge and
slave nodes synchronize to the hot-standby master (thick orange line), which in turn synchronizes to the primary master (dotted blue line) in another gPTP domain:

.. note::  In this configuration, this time domain is not used by applications or the time-aware shaping. Later on, this time domain will be important because clocks will fail over to it.

.. figure:: media/NormalOperation_domain2.png
   :align: center

Note that bridge and slave nodes update their time when the hot-standby master node's clock has already drifted from the primary master somewhat.

The next chart displays the end-to-end delay of application traffic, which is a constant low value:

.. figure:: media/delay_normaloperation.png
   :align: center

In the next section, we examine what happens if we take the master clock node offline during the simulation.

Link Failure of Master Clock
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, the time synchronization of the two time domains of the primary master clock stop due to a link failure of the
primary master clock (the other two time domains are still synchronized). After the link break, the active clocks in the network begin to diverge from each other.

We schedule the link break at 2s (halfway through the simulation) with a scenario manager script. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: LinkFailure
   :end-before: Failover

As in the previous section, we take a look at clock drift and end-to-end delay. The next chart shows the clock drift of gPTP time domain 0:

.. figure:: media/LinkFailure_domain0.png
   :align: center

The clocks begin to diverge after time synchronization in this gPTP time domain stops.

.. TODO time sync in this time domain stops

.. TODO domain -> time domain! too generic

The next chart shows clock drift in time domain 2. This is the time domain of the hot-standby master clock, which stays online, 
so the clocks in this time domain keep being synchronized. (the hot-standby master is denoted with a thick orange line, the primary master with a dotted blue line.)

.. figure:: media/LinkFailure_domain2.png
   :align: center

When the primary master node goes offline, the hot-standby master node cannot synchronize to it any more,
its clock drifts from the primary master's, shown by the orange and blue lines diverging. The bridge and slave nodes continue to synchronize to the hot-standby master node
(shown by the other lines following the hot-standby master node).

.. note:: The clock drift rate of the primary master is positive, the hot-standby master's is negative, thus the large change in direction (we set the constant drift rate for ``tsnClock2`` in the configuration to make sure the change in direction is apparent).

The active clocks in bridge and slave nodes are the ones that use time domain 0 as the source of time, thus the clock drift chart 
for the active clocks is the same as the one displaying clock drift for time domain 0.

The next chart shows the delay:

.. figure:: media/delay_linkfailure.png
   :align: center

After the clock divergence grows above a certain value the
end-to-end delay suddenly increases dramatically. The reason is that frames often
wait for the next gate scheduling cycle because they miss the allocated time slot
due to improperly synchronized clocks. The delay increases from the nominal microsecond-range
to millisecond range. Needless to say, in this scenario, the constant low delay cannot be guaranteed.

In the next section, we examine what happens if time synchronization can continue after the link break due to failover to the hot-standby master clock.

Failover to Hot-Standby Clock
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, we take the primary master clock offline just as in the previous one, but we also switch the active clock in each node over to the
one that synchronizes to the hot-standby master (gPTP time domain 2 as mentioned previously), so time synchronization can continue to keep the difference of clocks in the network below
the required limit.

.. note:: There is no difference in time synchronization at all in the three configurations. The difference is in which clocks/domains are active.

We schedule the link break with a scenario manager script. We also schedule changing the active clock parameter in the :ned:`MultiClock` modules in all nodes.
Both changes are scheduled at 2s, halfway through the simulation:

.. note:: We schedule the two changes at the same time, 2s. This is unrealistic. There is no mechanism here that detects the breakage of the time synchronization domains, so we switch the active clock manually with the scenario manager. Even if there was such a mechanism, switching to another time domain at exactly the same moment as the link break happens is unrealistic as well.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Failover

Let's examine the results. The clock drifts in domain 0 (clock time of the primary master node) are displayed on the following chart:

.. figure:: media/Failover_domain0.png
   :align: center

The clocks begin to diverge from each other after the link break at 2s.

The next chart displays the clock drifts in domain 2 (clock time of the hot-standby master node):

.. figure:: media/Failover_domain2.png
   :align: center

After the link break, the clocks are synchronized to the hot-standby master's time.

.. note:: The two charts above are exactly the same as the charts for Time Domain 0 and 2 in the Link Failure of Master Clock section, because there is no difference between the two cases in time synchronization and the scheduled link break. The difference is in which one is the active time domain.

The next chart displays the clock drift of the active clock in all nodes:

.. figure:: media/Failover_endstations.png
   :align: center

The following chart displays the delay:

.. figure:: media/delay_normaloperation.png
   :align: center

The delay is constant and the same as during normal operation, due to the seamless failover to the hot-standby master node (not even one frame suffers increased delay).
For comparison, the next chart displays the delay for the three configurations on individual plots, with the same axis scale:

.. figure:: media/Delay_all.png
   :align: center

.. note:: There is a configuration in :download:`extras.ini <../extras.ini>` that simulates what happens if the primary master clock comes back online after some time, and all nodes switch back to it. The relevant charts are in Extras.anf.


| Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`GptpAndTasShowcase.ned <../GptpAndTasShowcase.ned>`
| Extra configurations: :download:`extras.ini <../extras.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/784>`__ page in the GitHub issue tracker for commenting on this showcase.

