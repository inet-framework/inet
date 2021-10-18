.. _ug:cha:tsn:

Time-Sensitive Networking
=========================

This chapter describes the INET Framework modules that implement a subset of the
IEEE standards related to Time-Sensitive Networking (TSN). Some of these modules
are specific to the IEEE standards, but several of them are more generic and they
are simply reused to achieve the required functionality.

.. _ug:sec:tsn:devices:

Devices
-------

:ned:`TsnClock`
:ned:`TsnDevice`
:ned:`TsnSwitch`

.. _ug:sec:tsn:timesynchronization:

Time Synchronization
--------------------

This section describes the modules that implement a subset of the IEEE 802.1AS
standard titled Timing and Synchronization for Time-Sensitive Applications (IEEE
802.1AS-2020).

:ned:`SettableClock`

:ned:`Gptp`
:ned:`GptpBridge`
:ned:`GptpEndstation`
:ned:`GptpMaster`
:ned:`GptpSlave`

:ned:`MultiClock`
:ned:`MultiGptp`

:par:`hasGptp`
:par:`hasTimeSynchronization`

.. _ug:sec:tsn:streamfiltering:

Per-stream Filtering and Policing
---------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Per-Stream
Filtering and Policing (IEEE 802.1Qci-2017) amendment.

The simplest module for IEEE 802.1Q per-stream filtering and policing is the
:ned:`SimpleIeee8021qciFilter` compound module. This module combines several submodules:
a packet classifier at the input, a packet multiplexer at the output, and
one packet meter, one packet filter, and one packet gate per stream. Each
one of the latter per-stream 3 modules are optional.

When a packet arrives at the input of the :ned:`SimpleIeee8021qciFilter`, it first gets
classified into one of the filtering and policing submodule paths. Then the
packet meter measures the packet as part of the packet stream that was seen
so far, and attaches the result of the measurement. The result may be as
simple as a label on the packet. After the metering, the packet filter checks
if the packet matches the required conditions and either lets the packet go
through or drops it. Finally, the packet gate allows the automatic time based
or programmatic control of the packet passing through the selected path of the
policing module. Packets are never enqueued in the :ned:`SimpleIeee8021qciFilter`, they
either pass through or get dropped immediately.

Note that any of the :ned:`SimpleIeee8021qciFilter` default submodules can be replaced
with other variants. Moreover, other more complicated internal structures
are also possible, this is especially the case when the packet meters are
replaced with token bucket classifiers as described below.

As the first step, the default policing process starts with a packet classifier,
module, the :ned:`StreamClassifier` by default, that classifies packets based on the
attached stream information. This classifier simply maps stream names to output
gate indices. Please note that the stream decoding and identification process
is not part of the :ned:`SimpleIeee8021qciFilter`.

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
packet meter submodules of :ned:`SimpleIeee8021qciFilter`. See the inet.queueing.meter
NED package for alternatives.

In the third step, the default per-stream filtering and policing process
continues with a packet filter module, the :ned:`LabelFilter` by default, that drops
the red packets and lets through the green and yellow ones by default. Of
course, different packet filter modules can also be used by replacing the
default filter submodules of :ned:`SimpleIeee8021qciFilter`. See the inet.queueing.filter
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
the :ned:`Ieee8021qciFilter`. This module is more similar to the architecture that
is present in the IEEE 802.1Q standard. The :ned:`Ieee8021qciFilter` also combines
several submodules but in a slightly different way than the :ned:`SimpleIeee8021qciFilter`.
The most important difference is that this module can be mostly configured
through a single streamFilterTable parameter.

:par:`hasIngressTrafficFiltering`

.. _ug:sec:tsn:trafficshaping:

Scheduling and Traffic Shaping
------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Enhancements
for Scheduled Traffic (IEEE 802.1Qbv-2015) amendment.

The traffic shaping architecture is part of the queue submodule of the MAC layer
of each network interface. Currently three different packet shaper algorithms
are supported, the credit-based shaper, the time-aware shaper, and the asynchronous
shaper. In order to configure the network interface to use traffic shaping the
queue submodule must be replaced with either the :ned:`GatingPriorityQueue` or
the :ned:`PriorityShaper' compound modules. Both contain a packet classifier to
differentiate between the traffic categories and a priority packet scheduler
that prefers higher priority traffic categories over lower priority ones. The
difference is in the structure of the other submodules that form the shapers.

The credit-based shaper is implemented in the :ned:`CreditBasedShaper` module
using a standard :ned:`PacketQueue` and a special purpose :ned:`Ieee8021CreditBasedGate`
submodule. The latter module keeps track of the available credits for the given
traffic category and allows or forbids the transmission of packets.

The time-aware shaper is implemented in the :ned:`TimeAwareShaper` compound module
that uses a standard :ned:`PacketQueue` and a special purpose :ned:`PeriodicGate`.
The latter module has parameters to control the gate schedule that determines
the periodic open and gate.

The asynchronous shaper is in part implemented in the :ned:`AsynchronousShaper`
compound module. This shaper is somewhat more complicated than the previous two
because it also contains submodules that are part of the ingress traffic filtering
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
also be put in place in the usual ways.

.. _ug:sec:tsn:framereplication:

Frame Replication and Elimination
---------------------------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1CB standard titled: Frame Replication and Elimination for
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
be put in place in the usual ways.

.. _ug:sec:tsn:framepreemption:

Frame Preemption
----------------

This section describes the modules that implement a subset of the functionality
of the IEEE 802.1Q standard that was originally introduced by the Frame Preemption
(IEEE 802.1Qbu) amendment.

Frame preemption requires the network interface to be able to interrupt an
ongoing transmission and switch to the transmission of a higher priority frame.
This behavior is implemented in special :ned:`EthernetPreemptingMacLayer` and
:ned:`EthernetPreemptingPhyLayer` modules in the network interface. These modules
use packet streaming inside the network interface in contrast with the default
behavior where modules are passing packets around as a whole.

:ned:`PreemptableStreamer`
:ned:`PreemptingServer`

The TSN specific network nodes, :ned:`TsnDevice` and :ned:`TsnSwitch`, have a
special parameter called the :par:`hasFramePreemption` which can be used to
enable frame preemption in the network interfaces. Of course, these modules can
also be put in place in the usual ways.

.. _ug:sec:tsn:cutthroughswitching:

Cut-through Switching
---------------------

:ned:`EthernetCutthroughLayer`
:ned:`EthernetCutthroughSource`
:ned:`EthernetCutthroughSink`

:par:`hasCutthroughSwitching`

.. _ug:sec:tsn:automaticnetworkconfiguration:

Automatic Network Configuration
-------------------------------

Configuring the features of Time-Sensitive Networking in a complex network topology
with many application traffics with different requirements is a very difficult
and error prone task.

There are several different automatic gate scheduling configurators having
different capabilities. The :ned:`SimpleGateSchedulingConfigurator` is the
most basic one.

:ned:`Z3GateSchedulingConfigurator`
:ned:`TSNSchedGateSchedulingConfigurator`

:ned:`StreamRedundancyConfigurator`

:ned:`TsnConfigurator`
