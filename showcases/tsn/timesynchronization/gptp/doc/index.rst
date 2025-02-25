Using gPTP
==========

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
multiple master clocks for redundancy (in case one of the master clocks fails or
goes offline due to link break, for example, nodes can still have synchronized
time). Each time domain contains one master clock, and any number of slave
clocks. The protocol synchronizes the slave clocks to the master clock by
sending `sync messages` from the master clock nodes to the slave clock nodes.

According to the IEEE 802.1 AS standard, the master clock can be automatically
selected by the Best Master Clock algorithm (BCMA). BMCA also determines the
clock spanning tree, i.e., the routes on which sync messages are propagated to
slave clocks in the network. INET currently doesn't support BMCA; the master
clock and the spanning tree needs to be specified manually for each gPTP time
domain. 

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
submodules, one for each domain.

:ned:`Gptp` modules operate in one of three roles, according to their location in the spanning tree: `master`, `bridge` or `slave`. 
Nodes containing master gPTP modules contain the master clock for the time
domain, create gPTP sync messages, and broadcast them down-tree to bridge and
slave nodes. Bridge nodes forward sync messages to bridge and slave nodes as
well. The module type can be selected by the :ned:`Gptp` module's
:par:`gptpNodeType` parameter (either ``MASTER_NODE``, ``BRIDGE_NODE`` or
``SLAVE_NODE``)

.. note:: A network node might have different gPTP roles in different time domains, if there are several.

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

When nodes have multiple gPTP time domains, each time domain has a corresponding
:ned:`Gptp` module. The :ned:`MultiDomainGptp` module makes this convenient, as
it contains multiple :ned:`Gptp` modules. Also, each domain can have a
corresponding clock module. The :ned:`MultiClock` module can be used for this
purpose, as it contains multiple clock submodules. 

One of the sub-clocks in the :ned:`MultiClock` module is designated as the
active clock. Users of the :ned:`MultiClock` module (i.e., other modules that
get the time from the :ned:`MultiClock` module) use the time of the active
clock. The active clock can be changed by a scenario manager script to another
sub-clock in case of failure of time synchronization in one domain, for example.

.. note:: When using :ned:`MultiClock` and :ned:`MultiDomainGptp`, it is enough to specify the :ned:`MultiClock` module to the :ned:`MultiDomainGptp` as the clock module. The corresponding sub-clock for each domain is selected automatically (i.e., :ned:`Gptp` submodules are paired to clock submodules in :ned:`MultiClock` based on index).

For more information on the parameters of :ned:`Gptp`, :ned:`MultiDomainGptp`, and :ned:`MultiClock`, check the NED documentation.

The Model
---------

In this showcase, we demonstrate the setup and operation of gPTP in three simulations:

- **One Master Clock**: Simple setup with one time domain and one master clock.
- **Primary and Hot-Standby Master Clocks**: More complex setup with two time domains for a primary and a hot-standby master clock. If the primary master node goes offline,
  the stand-by clock can take over and become the new Master Clock.
- **Two Master Clocks Exploiting Network Redundancy**: A larger network containing a primary and a hot-standby master node, with two time domains each. Time synchronization is protected against the failure of a master node and any link in the network.

In the ``General`` configuration, we enable :ned:`Gptp` modules in all network nodes, and configure a random clock drift rate for the master clocks, and a constant clock drift rate
for the clocks in slave and bridge nodes (specified with a random distribution for each one):

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

We configure the spanning tree by setting the master ports in ``tsnClock`` and ``tsnSwitch``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: TSN clock gPTP master ports
   :end-at: tsnSwitch

.. note:: The slave ports are set to ``eth0`` by default in :ned:`TsnDevice` and :ned:`TsnSwitch`, so they don't have to be set explicitly.

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
clocks is periodically synchronized to the master's. At the second time
synchronization event at 0.25s, the drift rates of the slave clocks are
compensated to be more aligned with the master clock's drift rate.

All such charts have these two large sawtooth patterns at the beginning, before
the drift rate is compensated. From now on, we will generally omit these to
concentrate on the details of the clock drift after it has been stabilized by
time synchronization.

.. note:: A `clock time difference to simulation time` chart can be easily produced by plotting the ``timeChanged:vector`` statistic, and applying a linear trend operation with -1 as argument.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/798>`__ page in the GitHub issue tracker for commenting on this showcase.

