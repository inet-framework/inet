gPTP Basics: Static Configuration
=================================

Goals
-----

The Generic Precision Time Protocol (gPTP, as specified in IEEE 802.1AS) is a
network protocol that can synchronize clocks with high accuracy. This is useful
for applications such as Time-Sensitive Networking (TSN). In this showcase, we
will demonstrate how to configure gPTP master clocks, bridges, and end stations
to establish reliable time synchronization across the entire network.

| INET version: ``4.5``
| Source files location: `inet/showcases/tsn/timesynchronization/gptp <https://github.com/inet-framework/inet/tree/master/showcases/tsn/timesynchronization/gptp>`__

About gPTP
----------

Overview
~~~~~~~~

In reality, the time kept by clocks in different network devices can drift from
each other. This clock drift can be simulated in INET (see the
:doc:`/showcases/tsn/timesynchronization/clockdrift/doc/index` showcase for more
information). In time-sensitive applications, this drift is mitigated by using
time synchronization.

Having time synchronized between any two clocks means that the difference in
time between them has an upper bound. This can be achieved by periodically
synchronizing the time between the clocks (i.e., before the time difference gets
too large). In gPTP, the times kept by a number of slave clocks are synchronized
to a master clock's time within a gPTP `time domain`. A network can have
multiple gPTP time domains, i.e., nodes can synchronize multiple clocks to
multiple master clocks for redundancy (see the
:doc:`/showcases/tsn/timesynchronization/gptp_hotstandby/doc/index` showcase for more
information).
The protocol synchronizes the slave clocks to the master clock by
sending `sync messages` from the master clock nodes to the slave clock nodes.

According to the IEEE 802.1 AS standard, the master clock can be automatically
selected by the Best Master Clock algorithm (BCMA).
In this showcase, however, we focus on the static configuration of gPTP.
The :doc:`/showcases/tsn/timesynchronization/gptp_bmca/doc/index` showcase
demonstrates the automatic selection of the master clock using BCMA.

The operation of gPTP is summarized as follows:

- All nodes compute the residence time (i.e., how much time a packet spends in a node before being forwarded) and up-tree link latency.
- The gPTP sync messages are propagated down-tree.
- Nodes calculate the precise time from the sync messages, the residence time and the latency of the link the sync message was received from.

gPTP in INET
~~~~~~~~~~~~

In INET, the protocol is implemented by the :ned:`Gptp` application module. This
is an optional module in several network nodes, such as TSN hosts and switches.
The optional :ned:`Gptp` modules can be enabled with the
:par:`hasTimeSynchronization` parameter in the ini file, for example:

``*.tsnHost.hasTimeSynchronization = true``

Nodes can be part of multiple gPTP time domains by having multiple :ned:`Gptp`
submodules, one for each domain. The following description of this showcase uses a single domain
with static configuration.
For a dynamic configuration and multiple domains, take a look at the
:doc:`/showcases/tsn/timesynchronization/gptp_bmca/doc/index` and
:doc:`/showcases/tsn/timesynchronization/gptp_hotstandby/doc/index` showcases respectively.

:ned:`Gptp` modules operate in one of three roles, according to their location in the spanning tree: `master`, `bridge` or `slave`. 
Nodes containing master gPTP modules contain the master clock for the time
domain, create gPTP sync messages, and broadcast them down-tree to bridge and
slave nodes. Bridge nodes forward sync messages to bridge and slave nodes as
well. In case of a static configuration, the module type can be selected by the
:ned:`Gptp` module's :par:`gptpNodeType` parameter
(either ``MASTER_NODE``, ``BRIDGE_NODE`` or ``SLAVE_NODE``)

The spanning tree is created by labeling nodes' interfaces (called `ports`) as
either a master or a slave, with the :par:`slavePort` and :par:`masterPorts`
parameters. Sync messages are sent on master ports, and received on slave ports.
Master nodes only have master ports, and slave nodes only slave ports (bridge
nodes have both). Consequently, since these ports define the spanning tree, each
node can have just one slave port.

The :ned:`Gptp` module has the following distinct mechanisms:

- `peer delay measurement`: Slave and bridge nodes periodically measure link delay by sending peer delay request messages (``pDelayReq``) up-tree; 
  they receive peer delay response messages (``pDelayResp``).
- `time synchronization`: Master nodes periodically broadcast gPTP sync messages (``GptpSync``) with their local time that propagate down-tree. The time of slave clocks is set to the time of the master clock, corrected for link and processing delays. Furthermore, the drift rate of slave clocks is aligned to that of the master clock by setting the oscillator compensation factor. The compensation factor is estimated from the time of the master and slave clocks at the current and the previous synchronization events. 

.. note:: Recipients of these messages need to know the timestamp of when those messages were sent to be able to calculate "correct" time. Currently, only two-step synchronization is supported, i.e., ``pDelayResp`` and ``GptpSync`` messages are immediately proceeded by follow-up messages that contain the precise time when the original ``pDelayResp``/``GptpSync`` message was sent. Clocks are set to the new time when the follow-up message is received.

Nodes send sync and peer delay measurement messages periodically. The period and
offset of sync and peer delay measurement messages can be specified by
parameters (:par:`syncInterval`, :par:`pDelayInterval`,
:par:`syncInitialOffset`, :par:`pDelayInitialOffset`).

The Model
---------

In this showcase, we demonstrate the setup and operation of gPTP in multiple simulations:

- **One Master Clock**: Simple setup with one time domain and one master clock.
- **Multi Hop**: Setup with multiple hops, useful for testing the peer delay measurement mechanism and the effect of link latency on time synchronization.
- **Rate Ratio Studies**: Allows to analyze the effect of the neighbor rate ratio (nRR) and grandmaster rate ratio (gmRR) on the time synchronization process.
- **Parameter Studies**: This usecase can be used to study the effect of the kp and ki parameter of the :ned:`PiServoClock`. For example, when adjusting the sync interval, these parameters need to be adjusted as well.
- **Jumping Clock**: This usecase can be used to study the effect of what happens when the master clock peforms time jumps into the future or the past.

In the ``General`` configuration, we enable :ned:`Gptp` modules in all network nodes, and configure a random clock drift rate the clocks (specified with a random distribution for each one):

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable time synchronization
   :end-at: driftRate

We detail each simulation in the following sections.

One Master Clock
----------------

In this configuration the network topology is a simple tree. The network
contains one master clock node (:ned:`TsnClock`), one bridge and two end
stations (:ned:`TsnDevice`), connected via a :ned:`TsnSwitch`:

.. figure:: media/OneMasterClockNetwork.png
   :align: center

We configure the spanning tree by setting the master ports in ``tsnClock`` and ``tsnSwitch``.
The configuration also sets different clock servos on different clocks for comparison.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: TSN clock gPTP master ports
   :end-at: true

.. note:: The slave ports are set to ``eth0`` by default in :ned:`TsnDevice` and :ned:`TsnSwitch`, so they don't have to be set explicitly.

The tsnSwitch uses the :ned:`InstantServoClock` clock, adjusts its time but not its drift rate.
the tsnDevice2 also uses the :ned:`InstantServoClock` clock, but it also adjusts its drift rate.
The tsnDevice1 uses the :ned:`PiServoClock` clock, which only adjusts its drift rate to stay in sync without performing any time jumps.

Here is a video of the synchronization mechanism (the time of the master clock
and the difference from this time in the other nodes are displayed):

.. video_noloop:: media/onemasterclock.mp4
   :align: center

Note that the clocks are set after the follow up messages are received.

Here is the spanning tree indicated by the direction of gPTP sync messages:

.. figure:: media/OneMasterClock_tree.png
   :align: center

We examine clock drift of all clocks by plotting the clock time difference against simulation time:

.. figure:: media/OneMasterClock.png
   :align: center

The master clock drifts according to a random walk process. The times of slave
clocks is periodically synchronized to the master's.
From the figure it becomes clear, that all clocks need the first two sync intervals as a startup phase
to stabilize their drift rate.
The figure also shows, that the tsnSwitch deviates much further from the master, as it is not able
to adjust its drift rate.

In the following, we omit the startup phase and only concentrate on the synchronization process afterwards.

.. figure:: media/OneMasterClock_zoomed.png
   :align: center

The figure above provides a zoomed in view and shows how tsnDevice1 and tsnDevice2 synchronize to the master.
In this figure, the difference between the two clock servos become evident.
While tsnDevice1 (using the :ned:`PiServoClock`) adjusts its clock smoothly to the master's time,
tsnDevice2 (using the :ned:`InstantServoClock`) instead performs jumps to synchronize to the master's time.

.. note:: A `clock time difference to simulation time` chart can be easily produced by plotting the ``timeChanged:vector`` statistic, and applying a linear trend operation with -1 as argument.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/798>`__ page in the GitHub issue tracker for commenting on this showcase.

