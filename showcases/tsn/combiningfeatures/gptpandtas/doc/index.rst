Effects of Time Synchronization on Time-Aware Shaping
=====================================================

Goals
-----

In this showcase we demonstrate how time synchronization affects end-to-end delay
in a network that is using time-aware traffic shaping.

.. note:: This showcase builds upon the :doc:`/showcases/tsn/timesynchronization/clockdrift/doc/index`, 
   :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` and :doc:`/showcases/tsn/trafficshaping/timeawareshaper/doc/index` showcases.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/gptpandtas <https://github.com/inet-framework/tree/master/showcases/tsn/combiningfeatures/gptpandtas>`__

The Model
---------

Overview
~~~~~~~~

The showcase contains a network that uses time-aware shaping and inaccurate local clocks in each network node.
Time-aware shaping requires synchronized time among all network nodes, so we also use gPTP for time synchronization.
This results in a constant end-to-end delay for the traffic. We examine how failure and recovery in time synchronization affect the delay.

.. [in this network that uses time-aware shaping]. 

If time synchronization fails for some reason, e.g. due to the primary master clock going offline, the end-to-end delay guarantees of time-aware shaping cannot be met anymore.
However, time synchronization can continue and delay guarantees can be met if all network nodes switch over to a secondary master clock.

.. - time aware shaping requires time synchronization/synchronized time in all network nodes
   - if time synchronization fails for some reason, e.g. due the primary master clock going offline, the end-to-end delay guarantees of TAS can't be met anymore
   - unless, time synchronization continues by failover to another master clock

To demonstrate this, we present three cases, with three configurations:

- **Normal operation**: time synchronization works, and the delay is constant (this is the same case as in last configuration in the :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` showcase).
- **Failure of master clock**: the master clock disconnects from the network, and time is not synchronized anymore
- **Failover to a secondary master clock**: the master clock disconnects from the network, but time synchronization can continue because network nodes switch to the secondary master clock

.. Somewhere: this showcase basically picks up where the GPTP showcase left off, using the same network as in the TODO config in the GPTP showcase.

.. This showcase uses the concepts in the TODO showcases, and progresses from the GPTP showcase.

The Configuration
~~~~~~~~~~~~~~~~~

.. Network
   +++++++

All simulations in the showcase use the same network as in the :ref:`sh:tsn:timesync:gptp:redundancy` section of the `Using gPTP` showcase.
It constains :ned:`TsnDevice` and :ned:`TsnClock` modules connected to a ring of switches (:ned:`TsnSwitch`):

.. figure:: media/Network.png
   :align: center

.. **TODO** traffic config

.. Most settings, such as traffic generation, clock drift, gPTP domains, and visualization, are specified in the ``General`` configuration. The other configurations
   extend ``Normal Operation``. 

Most settings, such as traffic generation, time-aware shaping, clock drift, gPTP domains, and visualization, are shared between the three cases, and are specified in the ``General`` configuration. The following sections present these settings.

Traffic
+++++++

.. TODO traffic in this network consist of UDP application traffic and gPTP traffic (described in TODO)
   -> eloszor ezt

Traffic in the network consists of UDP packets sent between ``tsnDevice1`` and ``tsnDevice4``, and gPTP messages.
sent by all nodes. To generate the UDP application traffic, we configure ``tsnDevice1`` to send 10B UDP packets to ``tsnDevice2``:

.. **TODO** -> not sure this is needed.

.. literalinclude:: ../omnetpp.ini
   :start-at: tsnDevice1.numApps
   :end-at: tsnDevice1.app[0].source.clockModule
   :language: ini

.. The packets are sent to a :ned:`UdpSink` in ``tsnDevice4``.

.. This is the only traffic in the network, besides the gPTP messages./

Clocks and Clock Drift
++++++++++++++++++++++

.. **TODO** clock config

.. The clocks in :ned:`TsnDevice`, :ned:`TsnSwitch`, and :ned:`TsnClock` have a :ned:`SettableClock` by default (because the :par:`hasTimeSynchronization` parameter is set to ``true``).
   The settable clock has a :ned:`ConstantDriftOscillator` by default. To configure clock drift, we only need to set the drift rate of the oscillator:

.. The ``TsnClock`` module has an :ned:`OscillatorBasedClock` by default.

   - tsnClock1 has one clock and its oscillatorbased by default
   - the other nodes have multiclock set, which in turn contains settableclock sub-clocks by default
   - both of the settable clock and the oscillatorbased clock have a constant drift oscillator by default #, which is what we need for simulating clock drift
   - to configure clock drift, we only need to set the drift rate of the oscillator

.. **V1** The master clock node, ``tsnClock1``, contains an :ned:`OscillatorBasedClock` by default. We configure the clock type of all other nodes to be :ned:`MultiClock`; this contains
   :ned:`SettableClock` sub-clocks by default. Both :ned:`OscillatorBasedClock` and :ned:`SettableClock` contains a :ned:`ConstantDriftOscillator` by default.
   To configure clock drift, we only need to set the drift rate of the oscillator:

.. The master clock node, ``tsnClock1``, contains an :ned:`OscillatorBasedClock` by default. We configure the clock type of all other nodes to be :ned:`MultiClock`, which contains
   :ned:`SettableClock` sub-clocks by default. Thus all clocks have 

   **V2** By setting the clock type to :ned:`MultiClock` in all nodes (except ``tsnClock1``, which has just one clock), all clocks in the network have :ned:`ConstantDriftOscillator` submodules
   due to default clock types in :ned:`MultiClock` and :ned:`TsnClock`. To configure clock drift, we only need to set the drift rate of the oscillator:

   TODO clocks are configured similarly to the other one (using multiclock and settable clocks with constant drift oscillators)
   the oscillators are configured a bit differently

Clocks are configured similarly to the TODO showcase, using :ned:`MultiClock` and :ned:`SettableClock` modules with constant drift oscillators.
However, we configure the TODO oscillator a bit differently to make the failover effect in the third example simulation more visible:

.. minden nodeban minden time domain timeja megvan

.. Both :ned:`OscillatorBasedClock` and :ned:`SettableClock` contains a :ned:`ConstantDriftOscillator` by default.
   To configure clock drift, we only need to set the drift rate of the oscillator:


.. literalinclude:: ../omnetpp.ini
   :start-at: oscillator.driftRate
   :end-at: **.oscillator.driftRate
   :language: ini

We specify a random drift rate for all oscillators in the second line, but override this setting for the oscillators of ``TsnClock2`` in the first line with a specific constant drift rate, in order to make the failover effect more visible.

**TODO** minden time domainnek a timeja minden nodeban nyilvan van tartva -> ez a multiclock lenyege

.. **TODO** time aware shaping config

  - we have time aware shaping in all switches
  - two traffic classes; the higher priority is the gptp messages; the lower priority is the UDP packets
  - the gates for the gptp messages are always open; the gates for the UDP packets are open 10% of the time -> so the traffic is limited, on average, to 10Mbps -> but its actually just 10kbps

Time-Aware Shaping
++++++++++++++++++

Here is the time-aware shaping part of the configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: hasEgressTrafficShaping
   :end-at: transmissionGate[1].clockModule
   :language: ini

We enable time-aware shaping in all switches. We configure two traffic classes: the higher priority class for gPTP messages, and the lower priority class for UDP packets.
The gPTP messages are critical to maintain time synchronization in the network, so we assign them higher priority, and configure the time-aware shapers to not schedule gPTP messages, but forward them immediatelly (the corresponding gates are always open).
Gates for the UDP packets are configured to be open 10% of the time, so the UDP traffic is limited to 10Mbps. The UDP traffic data rate is 10kbps, so there is a lot of room in the send
window; this makes the setup more tolerant of clock drift between network nodes.

.. and configure the gates in the time-aware shapers corresponding
   to them to be always open./

gPTP
++++

.. **TODO** summarize the gptp config -> same as the last config in GPTP -> its actually the same -> include in dropdown?

   **TODO** just summarize the config

   - 100Mbps Ethernet links -> not here
   - enable time sync
   - clocks and domains (how many clocks what syncs to what) -> kind of like the last config of gPTP -> summary
   - clock drift rates
   - sync message offsets
   - visualize the gates, the messages

For the time synchronization, we use the same clock and gPTP setup as in the Two Master Clocks Exploiting Network Redundancy configuration in the :doc:`/showcases/tsn/timesynchronization/gptp/doc/index` showcase.
To summarize, we have a primary and a hot-standby master clock node (``tsnClock1`` and ``tsnClock2``, respectively). Each master clock has two gPTP time domains, and
each gPTP domain disseminates timing information in the ring of switches in opposing directions. All bridge and slave nodes are part of all four time domains. With this setup, time synchronization is protected against the failure of one
master clock, and the failure of any link between the switches. The two gPTP domains of the hot-standby master clock synchronize to the two domains of the primary master.
The switches and the devices have four gPTP domains, and four up-to-date clocks during normal operation.

.. .. note:: Here is the gPTP configuration for reference:

          **TODO** dropdown thing?

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

.. Here is the ``General`` configuration:

   .. literalinclude:: ../omnetpp.ini
      :language: ini
      :end-before: NormalOperation

The Example Simulations and Results
-----------------------------------

Normal Operation
~~~~~~~~~~~~~~~~

In this configuration, time synchronization is operating properly. The configuration for this case is empty (only the ``General`` configuration applies):

.. The end-to-end
   delay is a constant low value. 

.. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NormalOperation
   :end-before: LinkFailure

Let us examine the clock drift (i.e. clock time difference to simulation time) and the end-to-end delay. The following chart displays the clock drift
for clocks in `gPTP time domain 0` (one of the gPTP domains of the primary master). This is the active gPTP time domain in all network nodes (i.e. all nodes keep time according
to this gPTP time domain):

.. of the primary master clock. This is the active time domain in all network nodes (everybody keeps time according to this)
   but there are still other time domains being maintained simulatenously, but not used.

   The domain's master clock is the clock of the primary master node, **TODO** rewrite 
   and all other gPTP nodes use this domain for keeping time (i.e. the active clock in bridge and slave nodes
   is the one corresponding to this domain):

   domain 0 of the primary master clock

   three other time domains being maintained but not used

   all nodes use

.. Here are the results:

.. .. figure:: media/primaryclocktimedomain.svg
      :align: center
      :width: 100%

.. figure:: media/NormalOperation_domain0.png
   :align: center

Due to everything in the network operating properly, bridge and slave nodes periodically synchronize their clocks to the primary master node.

.. note:: The other three gPTP time domains are maintained simultaneously, but not used.

The next chart displays the clock drift for `gPTP time domain 2`, where timing information originates from the hot-standby master clock. All bridge and
slave nodes synchronize to the hot-standby master (thick orange line), which in turn synchronizes to the primary master (dotted blue line) in another gPTP domain:

.. note::  In this configuration, this time domain is not used by applications or the time-aware shaping. Later on, this will be important because clocks will fail over to this gPTP time domain.

.. **TODO** this might not be needed

.. **TODO** this time is not used by applications and for time aware shaping;
   later on, this will be important because the clock will fail over to this time domain

.. .. figure:: media/hotstandbytimedomain.svg
      :align: center
      :width: 100%

.. figure:: media/NormalOperation_domain2.png
   :align: center

Note that bridge and slave nodes update their time when the hot-standby master node's clock has already drifted from the primary master somewhat.

.. .. figure:: media/bridgesendstations.svg
      :align: center
      :width: 100%

.. The next chart displays the clock drift of the active clocks in bridges and end-stations (slave nodes):

.. .. figure:: media/NormalOperation_endstations.png
      :align: center

   In this case, the active clock in all nodes is always the default one (``clock[0]``), which uses gPTP domain 0. Thus this chart is exactly the same as the first one displaying clock drift in domain 0.

   TODO

   lehet hogy nem kene ez a chart
   eleg hogy ugyanaz mint a time domain 0

   this is exactly the same as the first one displaying clock drift 0, because domain 0 is the
   active clock

The next chart displays the end-to-end delay of application traffic, which is a constant low value:

.. figure:: media/delay_normaloperation.png
   :align: center

In the next section, we examine what happens if we take the master clock node offline during the simulation.

Link Failure of Master Clock
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, the time synchronization of the two time domains of the primary master clock stop due to a link failure of the
primary master clock (the other two time domains are still synchronized). After the link break, the active clocks in the network begin to diverge from each other.

.. **TODO** time sync doesnt stop, just in one time domain

.. clocks in these time domains

We schedule the link break at 2s (halfway through the simulation) with a scenario manager script. Here is the configuration:

.. Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: LinkFailure
   :end-before: Failover

.. Here are the results:

As in the previous section, we take a look at clock drift and end-to-end delay. The next chart shows the clock drift of gPTP time domain 0:

.. .. figure:: media/linkfailure_primarymaster.svg
      :align: center
      :width: 100%

.. figure:: media/LinkFailure_domain0.png
   :align: center

The clocks begin to diverge after time synchronization in this gPTP time domain stops.

.. TODO time sync in this time domain stops

TODO domain -> time domain! too generic

The next chart shows clock drift in domain 2. This is the domain of the hot-standby master clock, which stays online.
Thus the clocks in this domain keep being synchronized. (the hot-standby master is denoted with the thick orange line, the primary master with the dotted blue line.)

**TODO** keep being synchronized

.. .. figure:: media/linkfailure_hotstandby.svg
      :align: center
      :width: 100%

.. figure:: media/LinkFailure_domain2.png
   :align: center

Note that when the primary master node goes offline, the hot-standby master node cannot synchronize to it any more.
Thus its clock drifts from the primary master's, denoted by the orange and blue lines diverging. The bridge and slave nodes continue to synchronize to the hot-standby master node.
(denoted by the other lines following it)

.. **TODO** a tobbi node gorbeje ragaszkodik a sarga gorbehez

.. TODO denoted by the orange and blue lines diverging

.. TODO sudden change -> ezert nyultunk bele hogy ez latvanyos legyen

.. TODO azert v a nagy change, hirtelen iranyvaltas, azert v mert a domain 0 es 2
   -> az egyik driftje + a masik - -> emiatt van a nagy change
   -> ha a difference kicsi lenne akkor you could barely see any change!!!!
   -> szandekosan csinaltuk igy nem biztos hogy mindig igy van

.. .. figure:: media/linkfailure_bridgesendstations.svg
      :align: center
      :width: 100%

The next chart shows the active clocks in bridge and slave nodes:

.. figure:: media/LinkFailure_endstations.png
   :align: center

The active clocks are still the ones using domain 0 as the source of time, thus their times diverge after the link break TODO due to not being synced. Again, this chart is the same as the one displaying domain 0.

TODO nem kell a chart csak annyi hogy ugynaaz

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

.. **V1** In this configuration, the hot-standby master clock failover happens and the time
   synchronization continues to keep the difference of clocks in the network below
   the required limit.

In this configuration, we take the primary master clock offline just as in the previous one, but we also switch the active clock in each node over to the
one that synchronizes to the hot-standby master, so time synchronization can continue to keep the difference of clocks in the network below
the required limit.

.. TODO there is no difference in time sync at all; abban tortenik valtozas h melyik az aktiv clock

.. Here is the configuration:

We schedule the link break with a scenario manager script. We also schedule changing the active clock parameter in the :ned:`MultiClock` modules in all nodes.
Both changes are scheduled at 2s, halfway through the simulation:

.. note:: We schedule the two changes at the same time, 2s. This is unrealistic.

.. TODO switching from one domain and another is controlled manually -> BS
   exactly the same time -> BS

   -> insert that -> no mechanism here that actually detects the breakage of the time
   sync domain -> this is just one way of it breaking -> also completely unrealistic

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Failover

Here are the results:

Let's examine the results. The clock drifts in domain 0 (clock time of the primary master node) are displayed on the following chart:

.. .. figure:: media/failover_primarymaster.svg
      :align: center
      :width: 100%

.. figure:: media/Failover_domain0.png
   :align: center

.. **TODO** this is the same chart -> is the same egy link

The clocks begin to diverge from each other after the link break.

The next chart displays the clock drifts in domain 2 (clock time of the hot-standby master node):

.. figure:: media/Failover_domain2.png
   :align: center

After the link break, the clocks are synchronized to the hot-standby master's time.

The next chart displays the clock drift of the active clock in all nodes:

.. .. figure:: media/failover_hotstandby.svg
      :align: center
      :width: 100%

.. .. figure:: media/failover_bridgesendstations.svg
      :align: center
      :width: 100%

.. figure:: media/Failover_endstations.png
   :align: center

.. **TODO** the nodes use the time of the active clock

.. **TODO** Basically, the nodes had a correctly synchronized time domain in the previous one as well. We just switch the active clock over to the one that has the correct time.

The following chart displays the delay:

.. figure:: media/delay_normaloperation.png
   :align: center

The delay is constant and the same as during normal operation, due to the seamless failover to the hot-standby master node.
For comparison, the next chart displays the delay for the three configurations on individual plots, with the same axis scale:

.. figure:: media/Delay_all.png
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

