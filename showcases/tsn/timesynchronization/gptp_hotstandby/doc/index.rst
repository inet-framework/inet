MultiDomain gPTP and Hot-Standby
=================================

Overview
--------

In this showcase we present the multi-domain gPTP feature of INET
together with a basic implementation of the IEEE 802.1ASdm :ned:`HotStandby` amendment.
The showcase consists of two networks scenarios:

- **Primary and Hot-Standby Master Clocks**: More complex setup with two time domains for a primary and a hot-standby master clock.
  It implements the :ned:`HotStandby` module, so if the primary master node goes offline,
  the stand-by clock can take over and become the new Master Clock.
- **Two Master Clocks Exploiting Network Redundancy**: A larger network containing four domains and a primary and a hot-standby master node,
  with two time domains each. Time synchronization is protected against the failure of a master node and any link in the network.
  We do not use HotStandby in this scenario.

Primary and Hot-Standby Master Clocks
-------------------------------------

The network of this showcase
contains one primary master clock node and one hot-standby master clock node.
Both master clock nodes have their own time synchronization domain. The switch
and device nodes have two clocks, each synchronizing to one of the master clocks
separately. The only connection between the two time domains is in the
hot-standby master clock that is also synchronized to the primary master clock.
This connection effectively causes the two time domains to be totally
synchronized and allows seamless failover in the case of the master clock
failure.

The network contains two clock nodes (:ned:`TsnClock`) and four TSN device nodes (:ned:`TsnDevice`), connected by two TSN switches (:ned:`TsnSwitch`):

.. figure:: media/PrimaryAndHotStandbyNetwork.png
   :align: center

Our goal is to configure the two gPTP spanning trees for the two time domains.
In this setup, the clock nodes have one clock, and the others have two (one for
each domain).

- ``tsnClock1`` (the primary master) has one clock and one gPTP domain, and disseminates timing information to `domain 0` of `all` other nodes.
- ``tsnClock2`` (the hot-standby master) has one clock and two gPTP domains, and disseminates timing information of its `domain 1` to `domain 1` of all other nodes except ``tsnClock1``.
- The clock in ``tsnClock2`` gets synchronized to the primary master node in `domain 0`.

Let's see the configuration in omnetpp.ini, starting with settings for the clock nodes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnClock2.clock.typename = "PiServoClock"
   :end-at: *.tsnClock2.gptp.domain[1].masterPorts = ["eth0"]

We configure ``tsnClock2`` to have a :ned:`PiServoClock`, so we can set the
time. We configure ``tsnClock1`` to have a :ned:`Gptp` module, and set it as a
master node. Also, we specify that it should use its own clock, and set the only
interface, ``eth0`` to be a master port (the node will send gPTP sync messages
on that port).

In ``tsnClock2``, we need two :ned:`Gptp` modules (one is a leaf, the other a
root in the tree), so we set the type of the ``gptp`` module to
:ned:`MultiDomainGptp` with two domains. Both domains use the only clock in the
node, but one of them acts as a gPTP master, the other one a gPTP slave (using
the same port, ``eth0``).

Here is the configuration for the switches:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnSwitch*.clock.typename = "MultiClock"
   :end-at: *.tsnSwitch2.gptp.domain[1].masterPorts = ["eth1", "eth2", "eth3"]

We configure the switches to have two clocks and two :ned:`Gptp` modules (one
for each domain). We then specify the spanning tree by setting the ports (the
:par:`gptpModuleType` is ``BRIDGE_NODE`` by default in :ned:`TsnSwitch`, so we
don't need to specify that). In both domains, the interface connecting to the
clock node is the slave port, and the others are master ports. The only
exception is that ``tsnSwitch1`` shouldn't send sync messages to ``tsnClock1``
(as we don't want it getting synchronized to anything), so the ``eth1``
interface isn't set as a master port.

Finally, here is the configuration for the devices:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnDevice*.clock.typename = "MultiClock"
   :end-at: *.tsnDevice*.gptp.domain[*].slavePort = "eth0"

Just as in the switches, we need two clocks and two :ned:`Gptp` modules in the
devices as well, so we use :ned:`MultiClock` and :ned:`MultiDomainGptp` with two
submodules. We configure each device's ``gptp`` module to use the
:ned:`MultiClock` module in the device; the appropriate sub-clock for the domain
is automatically selected. We set all ``gptp`` modules to use the only interface
as the slave port (the Gptp module type is ``SLAVE_NODE`` by default in
:ned:`TsnDevice`, so we don't need to configure that).

We also configure some offsets for the pDelay measurement and gPTP sync messages in the different domains so they are not transmitted concurrently and suffer queueing delays.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: pdelayInitialOffset
   :end-at: domain[1].syncInitialOffset

Lastly, we enable the :ned:`HotStandby` module on every device besides the
primary and the hot standby masterclock.
This is achieved by setting the :par:`hasHotStandby` parameter:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnClock2.gptp.hasHotStandby = false
   :end-at: **.hasHotStandby = true

The :ned:`HotStandby` module is a basic implementation of the IEEE 802.1ASdm
amendment and keeps track of the synchronization state of contained gPTP modules.
In case one gPTP module loses synchronization, it will switch the active clock index
of the :ned:`MultiClock` to the standby clock in case it is still in sync.

The following is a video of the time synchronization process at the beginning of
the simulation. The clock time in master nodes, and the time difference to this
clock time in the other nodes are displayed for each clock. Messages for gPTP
are visualized as arrows. The visualization is color-coded according to domain:

.. video_noloop:: media/PrimaryAndHotStandbyMasterClocks.mp4
   :align: center
   :width: 100%

First, the bridge and slave nodes measure link delay by exchanging pDelay
messages. Then, the master clocks send gPTP sync messages. Note that there is a
jump in the time difference when the clocks are set to the new time after the
gPTP follow-up messages are received.

The spanning tree is visualized as the datalink-layer gPTP message
transmissions. This outlines the flow of timing information in the network,
originating from the master clock:

.. figure:: media/PrimaryAndHotStandbyMasterClocks_tree.png
   :align: center

To simulate the behavior of the :ned:`HotStandby` failover system, we add a
scenario script whit the following failure cases:

- From 3.1s to 7.1s the primary master clock is disconnected from the network.
- From 10.1s to 14.1s the link between ``tsnSwitch1`` and ``tsnSwitch2``  is broken,
  leading to the network splitting in half.

.. literalinclude:: ../link_failure.xml
   :language: xml

Let's examine some clock drift charts. Instead of plotting clock drift for all
clocks in one chart, let's use four charts so they are less cluttered. Here is
the clock drift (clock time difference to simulation time) of the two `master
clocks`:

.. figure:: media/PrimaryAndHotStandBy_masterclocks.png
   :align: center

Both master clocks have a random drift rate, but the hot-standby master clock's
time and clock drift rate are periodically synchronized to the primary.
However, in the failure cases, there is no synchronization between the primary
and the hot-standby master clock possible, thus they drift apart.

Here is the clock drift of all clocks in `time domain 0` (primary master) during the first failure case:

.. figure:: media/PrimaryAndHotStandBy_timedomain0_zoomed.png
   :align: center

Each clock has a distinct drift rate, while the master fluctuating randomly.
The slave clocks are periodically synchronized with the master clock. before and after
the first failure case.
During the first failure case, however, it becomes evident that no time synchronization is possible
in domain 1 during that time.

The following graph shows the same timeframe but plots the
activeClock instead of domain 0:

.. figure:: media/PrimaryAndHotStandBy_activeclocks_failure1.png
   :align: center

It is evident that shortly after the primary master is disconnected
a sync timeout occurs (typically ``3xsyncInterval=375ms``) and
the :ned:`HotStandby` module switches the :par:`activeClockIndex`  to 1.

In the second failure case a similar thing happens.
However here, there are two parts of the network diverging from each other.
The left parts still synchronizes to the primary master clock,
while the right part synchronizes to the hot-standby master clock:

.. figure:: media/PrimaryAndHotStandBy_activeclocks_failure2.png
   :align: center

.. _sh:tsn:timesync:gptp:redundancy:

Two Master Clocks Exploiting Network Redundancy
-----------------------------------------------

In this configuration the network topology is a ring. The primary master clock
and the hot-standby master clock each have two separate time domains. One time
domain uses the clockwise and another one uses the counterclockwise direction in
the ring topology to disseminate the clock time in the network. This approach
provides protection against the failure of the primary master node `and` a
single link failure in the ring because all bridges can be reached in both
directions by one of the time synchronization domains of both master clocks.

Here is the network (it uses the same node types as the previous ones,
:ned:`TsnClock`, :ned:`TsnSwitch` and :ned:`TsnDevice`):

.. figure:: media/TwoMasterClocksNetwork.png
   :align: center

The time synchronization redundancy is achieved in the following way:

- The primary master node has one clock and two master gPTP time domains. The domains send timing information in the ring in the clockwise and counterclockwise direction. 
- The hot-standby master node has two slave and two master gPTP domains, and two sub-clocks. Domains 0 and 1 sync the two clocks to the primary master's two domains, and domains 2 and 3 send timing information of the two clocks in both directions in the ring.
- Switch and device nodes have four domains (and four sub-clocks), with domains 0 and 1 syncing to the primary master node, and domains 2 and 3 to the hot-standby master node.
- Consequently, gPTP modules in the switches are gPTP bridges, in the devices, gPTP slaves.

In case of failure of the primary master node `and` a link in the ring, the
switches and devices would have at least one synchronized clock they could
switch over to.

How do we configure this scheme? We add the needed gPTP domains and clocks, and
configure the spanning tree outlined above. Some important aspects when setting
the ports and clocks:

- We don't want to forward any timing information to the primary master node, so we set the master ports in ``tsnSwitch1`` accordingly.
- We take care not to forward timing messages to a switch that originally sent it (so sync messages don't go in circles indefinitely). For example, tsnSwitch6 shouldn't send sync messages to ``tsnSwitch1`` in domain 0.
- The hot-standby master node has just two clocks, used by the four domains. The timing information from domains 0 and 1 is transferred to domains 2 and 3 here. So we set domains 0 and 2 to use ``clock[0]``, and domains 1 and 3 to use ``clock[1]``.

Here is the configuration for the clock nodes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: TwoMasterClocksExploitingNetworkRedundancy
   :end-at: *.tsnClock2.gptp.domain[3].masterPorts = ["eth0"]

We set ``tsnClock1`` to have two :ned:`Gptp` modules, each using the only clock
in the host. The type of the clock network nodes is :ned:`TsnClock`; in these,
the :ned:`Gptp` modules are set to master by default. We set the master ports in
both modules, so they disseminate timing information on their only Ethernet
interface.

``tsnClock2`` is set to have four gPTP domains. Since ``tsnClock2`` has only two
sub-clocks, we need to specify that domains 2 and 3 use ``clock[0]`` and
``clock[1]`` in the :ned:`MultiClock` module (it is sufficient to set the
:par:`clockModule` parameter to the :ned:`MultiClock` module, as it assigns the
sub-clocks to the domains automatically). 

Thus, in ``tsnClock2``, domains 0 and 1 are gPTP slaves, syncing to the two
domains of the primary master. Domains 2 and 3 are gPTP masters, and disseminate
the time of the clocks set by the first two domains.

The configuration for the switches is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: *.tsnClock2.gptp.domain[3].masterPorts = ["eth0"]
   :end-at: *.tsnSwitch6.gptp.domain[3].slavePort = "eth1"

Here is the configuration for the devices:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnDevice*.clock.typename = "MultiClock"
   :end-at: *.tsnDevice*.gptp.domain[*].slavePort = "eth0"

Finally, we configure offsets for the four domains, so that they don't send sync
messages at the same time:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: **.pdelayInitialOffset = 0.1ms
   :end-at: *.*.gptp.domain[3].syncInitialOffset = syncInterval * 4 / 4

Here is the spanning tree visualized by gPTP messages:

.. figure:: media/ExploitingNetworkRedundancy_tree.png
   :align: center

Just as in the previous sections, let's examine the clock drift of the different
clocks in the network. Here is the clock drift of the master clocks:

.. figure:: media/ExploitingNetworkRedundancy_masterclocks_zoomed.png
   :align: center

The clocks of the hot-standby master node are synced to the time of the primary
master periodically. Note that the sync times have the offset we configured.
Let's see the clock drifts in domain 0 (the primary master clock is plotted with
the thicker line):

.. figure:: media/ExploitingNetworkRedundancy_domain0_zoomed.png
   :align: center

In Domain 0, all clocks sync to the primary master clock. They sync at the same
time, because the offsets are between domains. The clock drift in Domain 1 is
similar, so we don't include it here. Let's see Domain 2 (the primary master
clock is displayed with a dashed line for reference, as it's not part of this
domain; the hot-standby master clock in this domain is displayed with the
thicker line):

.. figure:: media/ExploitingNetworkRedundancy_domain2_zoomed.png
   :align: center

All switches and devices sync to the hot-standby master clock (which itself is
synced to the primary master periodically).

.. note:: The charts for all domains are available in the .anf file in the showcase's folder.
