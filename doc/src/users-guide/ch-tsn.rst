.. _ug:cha:tsn:

Time-Sensitive Networking
=========================

This chapter describes the part of INET Framework that implements a subset of
the IEEE standards related to Time-Sensitive Networking (TSN). The supported
TSN features include among others: time synchronization, per-stream filtering
and policing, scheduling and traffic shaping, frame replication and elimination,
frame preemption, cut-through switching, automatic network configuration for
failure protection, stream redundancy, and gate scheduling.

The above TSN features are implemented in several modules separate from other
standard Ethernet functionality. Some of the modules presented in this chapter
are specific to the IEEE standards, but several of them are more generic and
they are simply reused to achieve the required functionality.

However, it must be noted that several TSN features are only partially supported.
The following sections describe the individual TSN features in detail.

.. _ug:sec:tsn:devices:

Devices
-------

TSN networks require additional functionality compared to standard Ethernet.
To facilitate the configuration of the TSN features, INET provides several TSN
specific network devices. These are derived from the basic INET network nodes and
they provide several additional TSN specific parameters. The additional parameters
mostly determine the internal module structure of the TSN specific network nodes,
and also further parameterize the basic network node submodules. The usage of
these network nodes is completely optional, it is possible to combine the TSN
modules in other ways. However, it is advisable to start making TSN simulations
using them.

-  :ned:`TsnClock` models a hardware device that exclusively acts as a gPTP
   master node for time synchronization
-  :ned:`TsnDevice` models a TSN specific end device capable of running
   multiple applications and all other supported TSN features
-  :ned:`TsnSwitch` models an Ethernet switch capable of all supported TSN
   features

Using the TSN specific network nodes and having the TSN specific features enabled
is just the first step for using Time-Sensitve Networking. Most TSN features
require additional configuration in the corresponding modules.

.. _ug:sec:tsn:timesynchronization:

Time Synchronization
--------------------

This section describes the modules that implement a subset of the IEEE 802.1AS
standard titled as Timing and Synchronization for Time-Sensitive Applications (IEEE
802.1AS-2020). There are two main components for this TSN feature, clock modules
that keep track of time in the individual network nodes, and time synchronization
protocol modules that synchronize these clocks. All required modules are already
included in the TSN specific network nodes.

If the default TSN specific network nodes are not used, then the following modules
can still be used to keep track of time in the network nodes.

-  :ned:`OscillatorBasedClock` models a clock having potentially drifting
   oscillator
-  :ned:`SettableClock` extends the previous model with the capability of setting
   the clock time

Similarly to the above the following gPTP time synchronization related protocol
modules and network nodes can also be used to build time synchronization in a
network:

-  :ned:`Gptp` the gPTP time synchronization protocol
-  :ned:`GptpBridge` models a gPTP time synchronization bridge network node
-  :ned:`GptpEndstation` models a gPTP time synchronization end station network node
-  :ned:`GptpMaster` models a gPTP time synchronization master network node
-  :ned:`GptpSlave` models a gPTP time synchronization slave network node

In order to implement node failure (e.g. master clock) and link failure (e.g.
between gPTP bridges) protection, multiple time synchronization domains are
required. These time domains operate independently of each other and it's up to
the clock user modules of each network node to decide which clock they are using.
Typically they use the active clock of the :ned:`MultiClock` and there has to
be some means of changing the active clocks when failover happens. The following
modules can be used to implement multiple time domains:

-  :ned:`MultiClock` contains several subclocks for the different time domains
-  :ned:`MultiDomainGptp` contains several gPTP submodules for the different
   time domains

The following parameters can be used to enable the gPTP time synchronization
in various predefined network nodes:

-  :par:`hasTimeSynchronization` parameter enables time synchronization in TSN
   specific network nodes
-  :par:`hasGptp` parameter enables the gPTP time synchronization protocol in
   gPTP specific network nodes

.. _ug:sec:tsn:streamfiltering:

Per-stream Filtering and Policing
---------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Per-Stream
Filtering and Policing (IEEE 802.1Qci-2017) amendment.

The simplest module for IEEE 802.1Q per-stream filtering and policing is the
:ned:`SimpleIeee8021qFilter` compound module. This module combines several submodules:
a packet classifier at the input, a packet multiplexer at the output, and
one packet meter, one packet filter, and one packet gate per stream. Each
one of the latter per-stream 3 modules are optional.

When a packet arrives at the input of the :ned:`SimpleIeee8021qFilter`, it first gets
classified into one of the filtering and policing submodule paths. Then the
packet meter measures the packet as part of the packet stream that was seen
so far, and attaches the result of the measurement. The result may be as
simple as a label on the packet. After the metering, the packet filter checks
if the packet matches the required conditions and either lets the packet go
through or drops it. Finally, the packet gate allows the automatic time based
or programmatic control of the packet passing through the selected path of the
policing module. Packets are never enqueued in the :ned:`SimpleIeee8021qFilter`, they
either pass through or get dropped immediately.

Note that any of the :ned:`SimpleIeee8021qFilter` default submodules can be replaced
with other variants. Moreover, other more complicated internal structures
are also possible, this is especially the case when the packet meters are
replaced with token bucket classifiers as described below.

As the first step, the default policing process starts with a packet classifier,
module, the :ned:`StreamClassifier` by default, that classifies packets based on the
attached stream information. This classifier simply maps stream names to output
gate indices. Please note that the stream decoding and identification process
is not part of the :ned:`SimpleIeee8021qFilter`.

In the second step, the default policing process continues with a packet meter
module, the :ned:`DualRateThreeColorMeter` by default, that labels the packets either
as green, yellow or red based on the committed and excess information rate,
and the committed and excess burst size parameters.

The most commonly used packet meters for per-stream filtering and policing
are:

-  :ned:`SingleRateTwoColorMeter` labels packets as green or red based on CIR
   and CBS parameters
-  :ned:`SingleRateThreeColorMeter` labels packets as green, yellow or red based
   on CIR, CBS and EBS parameters
-  :ned:`DualRateThreeColorMeter` labels packets as green, yellow or red based
   on CIR, CBS, EIR and EBS parameters

The above modules are based on the following generic token bucket meter
modules:

-  :ned:`TokenBucketMeter` contains a single token bucket and labels packets one
   of 2 labels
-  :ned:`MultiTokenBucketMeter` contains an overflowing chain of N token buckets
   and labels packets with one of N+1 labels

Different packet meter modules can also be used by replacing the default
packet meter submodules of :ned:`SimpleIeee8021qFilter`. See the inet.queueing.meter
NED package for alternatives.

In the third step, the default per-stream filtering and policing process
continues with a packet filter module, the :ned:`LabelFilter` by default, that drops
the red packets and lets through the green and yellow ones by default. Of
course, different packet filter modules can also be used by replacing the
default filter submodules of :ned:`SimpleIeee8021qFilter`. See the inet.queueing.filter
NED package for alternatives.

Finally, the default policing process finishes by merging the per-stream
filtering and policing paths into a single output gate by using the generic
:ned:`PacketMultiplexer` module. There's no need to prioritize between the per-stream
paths here, because the packets pass through in zero simulation time.

Different per-stream filtering and policing compound modules can also be
created by combining the existing queueing and protocol element modules
of the INET Framework. For example, instead of the packet meter modules,
the token bucket based packet classifier modules give more freedom in terms
of the module structure. See the inet.queueing NED package for more modules.

The most commonly used packet classifiers for per-stream filtering and
policing are:

-  :ned:`SingleRateTwoColorClassifier` classifies packets to 2 output gates based
   on CIR and CBS parameters
-  :ned:`SingleRateThreeColorClassifier` classifies packets to 3 output gates based
   on CIR, CBS and EBS parameters
-  :ned:`DualRateThreeColorClassifier` classifies packets to 3 output gates based
   on CIR, CBS, EIR and EBS parameters

The above modules are derived from the generic token bucket classifier modules.
These modules can also be used on their own and combined in many different
ways with all the other queueing modules to achieve the desired per-stream
filtering and policing.

-  :ned:`TokenBucketClassifier` contains a single token bucket and classifies
   packets to 2 output gates
-  :ned:`MultiTokenBucketClassifier` contains an overflowing chain of N token buckets
   and classifies packets to the N+1 output gates

There is also a more complex per-stream filtering and policing module, called
the :ned:`Ieee8021qFilter`. This module is more similar to the architecture that
is present in the IEEE 802.1Q standard. The :ned:`Ieee8021qFilter` also combines
several submodules but in a slightly different way than the :ned:`SimpleIeee8021qFilter`.
The most important difference is that this module can be mostly configured
through a single streamFilterTable parameter.

The TSN specific network node :ned:`TsnDevice` and :ned:`TsnSwitch` have a special
parameter called :par:`hasIngressTrafficFiltering` which can be used to enable the
traffic filtering and policing in the network node architecture. Of course, these
modules can also be used in other ways.


.. _ug:sec:tsn:trafficshaping:

Scheduling and Traffic Shaping
------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Enhancements
for Scheduled Traffic (IEEE 802.1Qbv-2015) amendment.

The traffic shaping architecture is part of the queue submodule of the MAC layer
in the network interface. Currently three different packet shaper algorithms
are supported, the credit-based shaper, the time-aware shaper, and the asynchronous
shaper. In order to configure the network interface to use traffic shaping the
queue submodule must be replaced with either the :ned:`GatingPriorityQueue` or
the :ned:`PriorityShaper` compound modules. Both contain a packet classifier to
differentiate between the traffic categories and a priority packet scheduler
that prefers higher priority traffic categories over lower priority ones. The
difference is in the structure of the other submodules that form the shapers.

The credit-based shaper is implemented in the :ned:`CreditBasedShaper` module
using a standard :ned:`PacketQueue` and a special purpose :ned:`Ieee8021qCreditBasedGate`
submodule. The latter module keeps track of the available credits for the given
traffic category and allows or forbids the transmission of packets.

The time-aware shaper is implemented in the :ned:`TimeAwareShaper` compound module
that uses a standard :ned:`PacketQueue` and a special purpose :ned:`PeriodicGate`.
The latter module has parameters to control the gate schedule that determines
the periodic open and gate.

The asynchronous shaper is in part implemented in the :ned:`AsynchronousShaper`
compound module. This shaper is somewhat more complicated than the previous two
because it also contains submodules that are part of the ingress per-stream filtering
module in the bridging layer. These are the :ned:`EligibilityTimeMeter` and the
corresponding :ned:`EligibilityTimeFilter` submodules. The first is responsible
for calculating the transmission eligibility time for incoming packets, the
latter is responsible for dropping packets which are considered to be too old
for transmission. The shaper in the network interface queue contains two additional
submodules called :ned:`EligibilityTimeQueue` and :ned:`EligibilityTimeGate`. The
former is responsible for sorting the frames according to the transmission
eligibility time, the latter is a gate that is open only if the transmission
eligibility time of the first frame of the queue is greater than the current
time.

The TSN specific network node :ned:`TsnDevice` and :ned:`TsnSwitch` have a special
parameter called :par:`hasEgressTrafficShaping` which can be used to enable the
traffic shaping in the network node architecture. Of course, these modules can
also be used in other ways.

.. _ug:sec:tsn:framereplication:

Frame Replication and Elimination
---------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1CB standard titled as Frame Replication and Elimination for
Reliability (IEEE 802.1CB-2017).

The relevant modules are all part of the :ned:`BridgingLayer` compound module
that resides between the network layer and link layer protocols. This compound
module also contains other functionality such as frame forwarding. There are
four relevant submodules, each one implements a very specific part of frame
replication.

The first part deals with stream identification, and is implemented in the
:ned:`StreamIdentifierLayer` module and its :ned:`StreamIdentifier` submodule.
This module is only useful in network nodes which produce application traffic
themselves. The stream identifier module is responsible for assigning a stream
name for outgoing packets by looking at their contents and meta data. For example,
packets can be identified by the destination MAC address and PCP request tags.
Since at this point the packets don't yet contain any layer 2 header the decision
can be based on the attached request tags that will be later turned into packet
headers.

The second layer handles incoming stream merging and outgoing stream splitting.
This layer is called the :ned:`StreamRelayLayer` and contains two submodules
called :ned:`StreamMerger` and :ned:`StreamSplitter`. The former is responsible
for merging incoming member streams into a single stream and removing duplicate
frames. The latter is responsible for splitting outgoing streams into potentially
several member streams.

The third part deals with ingress and egress stream filtering, and it's implemented
in the :ned:`StreamFilterLayer` module that contains one submodule for both
directions. This part is not strictly necessary for frame replication. Most
often only the ingress filtering submodule is used as described in the previous
section.

The last layer handles incoming packet decoding and outgoing packet encoding.
This module is called the :ned:`StreamCoderLayer` and it contains two submodules
the :ned:`StreamDecoder` and :ned:`StreamEncoder`. The former handles the stream
decoding of incoming packets by checking the attached indication tags. The latter
deals with the encoding of outgoing packets by attaching the necessary request
tags.

The TSN specific network node :ned:`TsnDevice` and :ned:`TsnSwitch` have a special
parameter called :par:`hasStreamRedundancy` which can be used to enable frame
replication in the network node architecture. Of course, these modules can also
be used in other ways.

.. _ug:sec:tsn:framepreemption:

Frame Preemption
----------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Frame Preemption
(IEEE 802.1Qbu) amendment.

Frame preemption requires the network interface to be able to interrupt an
ongoing transmission and switch to the transmission of a higher priority frame.
This behavior is implemented in special MAC and PHY layer modules that use packet
streaming in the network interface. This is in contrast with the default behavior
where modules pass packets around as a whole.

-  :ned:`EthernetPreemptingMacLayer` models an Ethernet MAC layer that contains
   multiple MAC sublayers to allow the preemption of background traffic
-  :ned:`EthernetPreemptingPhyLayer` models a PHY layer that allows the preemption
   of an ongoing transmission

The TSN specific network nodes, :ned:`TsnDevice` and :ned:`TsnSwitch`, have a
special parameter called the :par:`hasFramePreemption` which can be used to
enable frame preemption in the network interfaces. Of course, these modules can
also be used in other ways.

.. _ug:sec:tsn:cutthroughswitching:

Cut-through Switching
---------------------

The default store and forward mechanism in Ethernet switches greatly influences
the end-to-end latency of application traffic. This effect can be overcome and
drastically reduced by using cut-through switching. This methods starts forwarding
the incoming frame before the whole frame has been received, usually right after
the reception of the MAC header. However, cut-through switching is not a standard
mechanism and there are all kinds of variants in operation.

INET provides the following modules related to cut-through switching:

-  :ned:`EthernetCutthroughInterface` models an Ethernet interface that contains
   a special cut-through layer between the MAC and PHY layers that in certain
   circumstances allows the direct forwarding of frames from the incoming network
   interface to the outgoing
-  :ned:`EthernetCutthroughLayer` models the cut-through layer with direct
   connections to other cut-through interfaces inside the same network node
-  :ned:`EthernetCutthroughSource` models the source of the cut-through forwarding
   inside the network interface
-  :ned:`EthernetCutthroughSink` models the sink of the cut-through forwarding
   inside the network interface

Surprisingly cut-through switch also has to be enabled in the end devices because
the receiving switch has to be notified both at the start and at the end of the frame.

The TSN specific network nodes, :ned:`TsnDevice` and :ned:`TsnSwitch`, have a
special parameter called the :par:`hasCutthroughSwitching` which can be used to
enable cut-through switching in the network interfaces. Of course, these modules can
also be used in other ways.

.. _ug:sec:tsn:automaticnetworkconfiguration:

Automatic Network Configuration
-------------------------------

Configuring the features of Time-Sensitive Networking in a complex network that
contains many applications with different traffic requirements is a difficult
and error prone task. To facilitate this task, INET provides three types of
network level configurators:

-  gate scheduling configurators are capable of configuring the gate control
   lists (i.e. periodic open/close states) for all traffic classes in all network
   interfaces based on packet length, packet interval, and maximum latency parameters
-  stream redundancy configurators are capable of configuring the stream merging
   and stream splitting modules as well as the stream identification in all network
   nodes to form the desired redundant streams for each application traffic
-  failure protection configurators are capable of using the previous two to
   achieve the desired link and node failure protections for all streams in the
   network based on the set of failure cases

All other network level configurators such as the :ned:`Ipv4NetworkConfigurator`
or the :ned:`MacForwardingTableConfigurator` can also be used.

There are several different automatic gate scheduling configurators having
different capabilities:

-  :ned:`EagerGateScheduleConfigurator` eagerly allocates time slots in the
   order of increasing traffic priority
-  :ned:`Z3GateScheduleConfigurator` uses a SAT solver to fulfill the traffic
   constraints all at once
-  :ned:`TSNschedGateScheduleConfigurator` uses a state-of-the-art external
   tool called TSNsched that is available at https://github.com/ACassimiro/TSNsched

There is only one stream redundancy configurator:

-  :ned:`StreamRedundancyConfigurator` configures stream splitting, stream merging
   and stream filtering in all network nodes

Currently there is only one failure protection configurator:

-  :ned:`FailureProtectionConfigurator` configures the gate scheduling and the stream redundancy
   configurators to provide protection against the specified link and node failures

All of these configurators automatically discover the network topology and then
taking into account their own independent configuration they compute the necessary
parameters for the individual underlying modules and configure them. However,
anything they can do, can also be done from INI files manually, and the result
can also be seen at the configured module parameters in the runtime user interface.
