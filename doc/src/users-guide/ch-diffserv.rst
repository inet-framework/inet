.. role:: raw-latex(raw)
   :format: latex
..

.. _ug:cha:diffserv:

Differentiated Services
=======================

.. _ug:sec:diffserv:overview:

Overview
--------

In the early days of the Internet, only best effort service was defined.
The Internet delivers each packet individually, and delivery time is not
guaranteed. Moreover, packets may even be dropped due to congestion at
the routers of the network. It was assumed that transport protocols and
applications can overcome these deficiencies. This worked until FTP and
email were the main applications of the Internet, but the newer
applications such as Internet telephony and video conferencing cannot
tolerate delay jitter and loss of data.

The first attempt to add QoS capabilities to IP routing was
Integrated Services. Integrated services provide resource assurance
through resource reservation for individual application flows. An
application flow is identified by the source and destination addresses
and ports and the protocol ID. Before data packets are sent, the
necessary resources must be allocated along the path from the source to
the destination. Each router along the path must examine the packets and
decide if they belong to a reserved application flow. This could cause a
memory and processing demand on the routers. Another drawback is that
the reservation must be periodically refreshed, so there is overhead
during the data transmission as well.

Differentiated Services is a more scalable approach that offers a better
than best-effort service. Differentiated Services do not require
resource reservation setup. Instead of making per-flow reservations,
Differentiated Services divides the traffic into a small number of
*forwarding classes*. The forwarding class is directly encoded into the
packet header. After packets are marked with their forwarding classes at
the edge of the network, the interior nodes of the network can use this
information to differentiate the treatment of packets. The forwarding
classes may indicate drop priority and resource priority. For example,
when a link is congested, the network will drop packets with the highest
drop priority first.

In the Differentiated Service architecture, the network is partitioned
into DiffServ domains. Within each domain, the resources are allocated
to forwarding classes, taking into account the available resources and
the traffic flows. There are *service level agreements*
(SLAs) between the users and service providers, and between the domains
that describe the mapping of packets to forwarding classes and the
allowed traffic profile for each class. The routers at the edge of the
network are responsible for marking the packets and protecting the domain
from misbehaving traffic sources. Nonconforming traffic may be dropped,
delayed, or marked with a different forwarding class.

.. _ug:sec:diffserv:implemented-standards:

Implemented Standards
~~~~~~~~~~~~~~~~~~~~~

The implementation follows these RFCs:

-  RFC 2474: Definition of the Differentiated Services Field (DS Field)
   in the IPv4 and IPv6 Headers

-  RFC 2475: An Architecture for Differentiated Services

-  RFC 2597: Assured Forwarding PHB Group

-  RFC 2697: A Single Rate Three Color Marker

-  RFC 2698: A Two Rate Three Color Marker

-  RFC 3246: An Expedited Forwarding PHB (Per-Hop Behavior)

-  RFC 3290: An Informal Management Model for Diffserv Routers

.. _ug:sec:diffserv:architecture-of-nics:

Architecture of NICs
--------------------

Network interface modules, such as :ned:`PppInterface` and
:ned:`EthernetInterface`, may contain traffic conditioners in their
input and output data path.

Network interfaces may also contain an optional external queue
component. In the absence of an external queue module, :ned:`Ppp` and
:ned:`EthernetCsmaMacPhy` use an internal drop-tail queue to buffer the packets
while the line is busy.

.. _ug:sec:diffserv:traffic-conditioners:

Traffic Conditioners
~~~~~~~~~~~~~~~~~~~~

Traffic conditioners have one input and one output gate as defined in
the :ned:`ITrafficConditioner` interface. They can transform the
incoming traffic by dropping or delaying packets. They can also set the
DSCP field of the packet or mark it in another way for differentiated
handling in the queues.

Traffic conditioners perform the following actions:

-  Classify the incoming packets.

-  Meter the traffic in each class.

-  Mark or drop packets depending on the result of metering.

-  Shape the traffic by delaying packets to conform to the desired
   traffic profile.

INET provides classifier, meter, and marker modules that can be
composed to build a traffic conditioner as a compound module.

.. _ug:sec:diffserv:output-queues:

Output Queues
~~~~~~~~~~~~~

Queue components must implement the :ned:`IPacketQueue` module
interface. In addition to having one input and one output gate, these
components must implement a passive queue behavior: they only deliver a
packet when the module connected to their output explicitly requests it.
(In C++ terms, the module must implement the :cpp:`IPacketQueue`
interface. The next module requests a packet by calling the
:fun:`requestPacket()` method of that interface.)

.. _ug:sec:diffserv:simple-modules:

Simple modules
--------------

This section describes the primitive elements from which traffic
conditioners and output queues can be built. The following sections show
some examples of how these queues, schedulers, droppers, classifiers,
meters, and markers can be combined.

The types of the components are:

-  ``queue``: container of packets, accessed as FIFO.

-  ``dropper``: attached to one or more queues, it can limit the queue
   length below some threshold by selectively dropping packets.

-  ``scheduler``: decides which packet is transmitted first when more
   packets are available on their inputs.

-  ``classifier``: classifies the received packets according to their
   content (e.g., source/destination address and port, protocol, DSCP
   field of IP datagrams) and forwards them to the corresponding output
   gate.

-  ``meter``: classifies the received packets according to the temporal
   characteristic of their traffic stream.

-  ``marker``: marks packets by setting their fields to control their
   further processing.

.. _ug:sec:diffserv:queues:

Queues
~~~~~~

When packets arrive at a higher rate than the interface can transmit,
they are queued.

Queue elements store packets until they can be transmitted. They have
one input and one output gate. Queues may have one or more thresholds
associated with them.

Received packets are enqueued and stored until the module connected to
their output requests a packet by calling the :fun:`requestPacket()`
method.

They should be able to notify the module connected to their output about
the arrival of new packets.

.. _ug:sec:diffserv:fifo-queue:

FIFO Queue
^^^^^^^^^^

The :ned:`PacketQueue` module implements a passive FIFO queue with
unlimited buffer space. It can be combined with algorithmic droppers and
schedulers to form an IPacketQueue compound module.

The C++ class implements the :cpp:`IQueueAccess` and
:cpp:`IPacketQueue` interfaces.

.. _ug:sec:diffserv:droptailqueue:

DropTailQueue
^^^^^^^^^^^^^

The other primitive queue module is :ned:`DropTailQueue`. Its capacity
can be specified by the :par:`packetCapacity` parameter. When the number
of stored packets reaches the capacity of the queue, further packets are
dropped. Because this module contains a built-in dropping strategy, it
cannot be combined with algorithmic droppers as :ned:`PacketQueue` can be.
However, its output can be connected to schedulers.

This module implements the :ned:`IPacketQueue` interface, so it can be
used as the queue component of an interface card.

.. _ug:sec:diffserv:droppers:

Droppers
~~~~~~~~

Algorithmic droppers selectively drop received packets based on some
condition. The condition can be either deterministic (e.g., to bound the
queue length) or probabilistic (e.g., RED queues).

Another kind of dropper is an absolute dropper that drops each received
packet. It can be used to discard excess traffic, i.e. packets whose
arrival rate exceeds the allowed maximum. In INET, the :ned:`Sink` module
can be used as an absolute dropper.

The algorithmic droppers in INET are :ned:`ThresholdDropper` and
:ned:`RedDropper`. These modules have multiple input and multiple output
gates. Packets that arrive on gate :gate:`in[i]` are forwarded to gate
:gate:`out[i]` (unless they are dropped). However, the queues attached to
the output gates are viewed as a whole, i.e. the queue length parameter
of the dropping algorithm is the sum of the individual queue lengths.
This way, we can emulate shared buffers of the queues. Note that it is
also possible to connect each output to the same queue module.

.. _ug:sec:diffserv:threshold-dropper:

Threshold Dropper
^^^^^^^^^^^^^^^^^

The :ned:`ThresholdDropper` module selectively drops packets based on
the available buffer space of the queues attached to its output. The
buffer space can be specified as the count of packets or as the size in
bytes.

The module sums the buffer lengths of its outputs, and if enqueuing a
packet would exceed the configured capacities, then the packet will be
dropped instead.

By attaching a :ned:`ThresholdDropper` to the input of a FIFO queue, you
can compose a drop tail queue. Shared buffer space can be modeled by
attaching more FIFO queues to the output.

RED Dropper
^^^^^^^^^^^

The :ned:`RedDropper` module implements Random Early Detection
(:raw-latex:`\cite{Floyd93randomearly}`).

It has :math:`n` input and :math:`n` output gates (specified by the
:par:`numGates` parameter). Packets that arrive at the :math:`i^{th}`
input gate are forwarded to the :math:`i^{th}` output gate, or dropped.
The output gates must be connected to simple modules implementing the
:cpp:`IPacketQueue` C++ interface (e.g. :ned:`PacketQueue`).

The module sums the used buffer space of the queues attached to the
output gates. If it is below a minimum threshold, the packet won’t be
dropped, if above a maximum threshold, it will be dropped, if it is
between the minimum and maximum threshold, it will be dropped by a given
probability. This probability determined by a linear function which is 0
at the minth and maxp at maxth.

.. PDF version of image
   \setlength{\unitlength}{1cm}
   (7,4)(-1,-1) (-0.5,0)(1,0)6.5 (0,-0.5)(0,1)3.5 (5.8,-0.3):math:`qlen`
   (-0.5,3):math:`p` (1,0)(3,1)3 (4,1)(0,1)1 (4,2)(1,0)1.5
   (-0.5,1.9):math:`1` (0,2)(0.4,0)10(1,0)0.2 (0,1)(0.4,0)10(1,0)0.2
   (-1,0.9):math:`p_{max}` (4,0)(0,0.4)3(0,1)0.2 (0.9,-0.3):math:`th_{min}`
   (3.9,-0.3):math:`th_{max}`

.. figure:: figures/red-dropper.*
   :align: center
   :width: 340

The queue length can be smoothed by specifying the :par:`wq` parameter.
The average queue length used in the tests are computed by the formula:

.. math:: avg = (1-wq)*avg + wq*qlen

The :par:`minth`, :par:`maxth`, and :par:`maxp` parameters can be
specified separately for each input gate, so this module can be used to
implement different packet drop priorities.

.. _ug:sec:diffserv:schedulers:

Schedulers
~~~~~~~~~~

Scheduler modules decide which queue can send a packet when the
interface is ready to transmit one. They have several input gates and
one output gate.

Modules that are connected to the inputs of a scheduler must implement
the :cpp:`IPacketQueue` C++ interface. Schedulers also implement the
:cpp:`IPacketQueue` interface, so they can be cascaded to other
schedulers and used as the output module of :ned:`IPacketQueue`'s.

There are several possible scheduling disciplines (first come/first
served, priority, weighted fair, weighted round-robin, deadline-based,
rate-based). INET contains an implementation of priority and weighted
round-robin schedulers.

.. _ug:sec:diffserv:priority-scheduler:

Priority Scheduler
^^^^^^^^^^^^^^^^^^

The :ned:`PriorityScheduler` module implements a strict priority
scheduler. Packets that arrive on :gate:`in[0]` have the highest
priority, then packets that arrive on :gate:`in[1]`, and so on. If
multiple packets are available when one is requested, then the one with
the highest priority is chosen. Packets with lower priority are
transmitted only when there are no packets on the inputs with higher priorities.

:ned:`PriorityScheduler` must be used with care because a large volume
of higher priority packets can starve lower priority packets. Therefore,
it is necessary to limit the rate of higher priority packets to a fraction of
the output data rate.

:ned:`PriorityScheduler` can be used to implement the ``EF`` PHB.

Weighted Round Robin Scheduler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`WrrScheduler` module implements a weighted round-robin
scheduler. The scheduler visits the input gates in turn and selects the
number of packets for transmission based on their weight.

For example, if the module has three input gates and the weights are 3,
2, and 1, then packets are transmitted in this order:

::

   A, A, A, B, B, C, A, A, A, B, B, C, ...

where A denotes packets that arrived on :gate:`in[0]`, B denotes packets
that arrived on :gate:`in[1]`, and C denotes packets that arrived on
:gate:`in[2]`. If there are no packets in the current one when a packet
is requested, then the next one is chosen if it has enough tokens.

If the sizes of the packets are equal, then :ned:`WrrScheduler` divides
the available bandwidth according to the weights. In each case, it
allocates the bandwidth fairly. Each flow receives a guaranteed minimum
bandwidth, which is ensured even if other flows exceed their share (flow
isolation). It also efficiently uses the channel because if some
traffic is smaller than its share of bandwidth, then the rest is
allocated to the other flows.

:ned:`WrrScheduler` can be used to implement the ``AFxy`` PHBs.

.. _ug:sec:diffserv:classifiers:

Classifiers
~~~~~~~~~~~

Classifier modules have one input gate and multiple output gates. They examine
received packets and forward them to the appropriate output gate
based on the contents of specific packet headers. More information
about classifiers can be found in RFC 2475 and RFC 3290.

The ``inet.networklayer.diffserv`` package contains two classifiers:
:ned:`MultiFieldClassifier` to classify the packets at the edge routers
of the DiffServ domain, and :ned:`BehaviorAggregateClassifier` to
classify the packets at the core routers.

Multi-field Classifier
^^^^^^^^^^^^^^^^^^^^^^

The :ned:`MultiFieldClassifier` module can be used to identify
micro-flows in the incoming traffic. The flow is identified by the
source and destination addresses, the protocol ID, and the source and
destination ports of the IP packet.

The classifier can be configured by specifying a list of filters. Each
filter can specify a source/destination address mask, a protocol,
a source/destination port range, and bits of the TypeOfService/TrafficClass
field to be matched. They also specify the index of the output gate
to which matching packets should be forwarded. The first matching filter
determines the output gate; if there are no matching filters, then
:gate:`defaultOut` is chosen.

The configuration of the module is given as an XML document. The
document element must contain a list of ``<filter>`` elements. Each
filter must have a mandatory ``@gate`` attribute specifying the index of
the output gate for packets matching the filter. Other attributes are
optional and specify the conditions for a match:

-  ``@srcAddress``, ``@srcPrefixLength``: to match the source
   address of the IP.

-  ``@destAddress``, ``@destPrefixLength``:

-  ``@protocol``: matches the protocol field of the IP packet. Its
   value can be a name (e.g., “udp”, “tcp”) or the numeric code of the
   protocol.

-  ``@tos``, @tosMask: matches bits of the TypeOfService/TrafficClass
   field of the IP packet.

-  ``@srcPort``: matches the source port of the TCP or UDP packet.

-  ``@srcPortMin``, ``@srcPortMax``: matches a range of source ports.

-  ``@destPort``: matches the destination port of the TCP or UDP
   packet.

-  ``@destPortMin``, ``@destPortMax``: matches a range of destination ports.

The following example configuration specifies

-  to transmit packets received from the 192.168.1.x subnet on gate 0,

-  to transmit packets addressed to port 5060 on gate 1,

-  to transmit packets having CS7 in their DSCP field on gate 2,

-  and to transmit other packets on :gate:`defaultGate`.



.. code-block:: xml

   <filters>
     <filter srcAddress="192.168.1.0" srcPrefixLength="24" gate="0"/>
     <filter protocol="udp" destPort="5060" gate="1"/>
     <filter tos="0b00111000" tosMask="0x3f" gate="2"/>
   </filters>

Behavior Aggregate Classifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`BehaviorAggregateClassifier` module can be used to read the
DSCP field from the IP datagram and direct the packet to the
corresponding output gate. The DSCP value is the lower six bits of the
TypeOfService/TrafficClass field. Core routers usually use this
classifier to guide the packet to the appropriate queue.

DSCP values are enumerated in the :par:`dscps` parameter. The first
value is for gate :gate:`out[0]`, the second for :gate:`out[1]`, and so on.
If the received packet has a DSCP value that is not enumerated in the
:par:`dscps` parameter, it will be forwarded to the :gate:`defaultOut` gate.

.. _ug:sec:diffserv:meters:

Meters
~~~~~~

Meters classify the packets based on the temporal characteristics of
their arrival. The arrival rate of packets is compared to an allowed
traffic profile, and packets are either marked as green (in-profile) or
red (out-of-profile). Some meters apply more than two conformance levels.
For example, in three-color meters, packets that partially conform are
classified as yellow.

The allowed traffic profile is usually specified by a token bucket. In
this model, a bucket is filled with tokens at a specified rate until it
reaches its maximum capacity. When a packet arrives, the bucket is
examined. If it contains at least as many tokens as the length of the
packet, then those tokens are removed, and the packet is marked as
conforming to the traffic profile. If the bucket contains fewer tokens
than needed, it is left unchanged, but the packet is marked as
non-conforming.

Meters have two modes: color-blind and color-aware. In color-blind mode,
the color assigned by a previous meter does not affect the classification
of the packet in subsequent meters. In color-aware mode, the color of the
packet cannot be changed to a less conforming color. If a packet is
classified as non-conforming by any meter, it is also handled as
non-conforming in subsequent meters in the data path.

.. important::

   Meters take into account the length of the IP packet only; L2 headers are omitted
   from the length calculation. If they receive a packet that is not
   an IP datagram and does not encapsulate an IP datagram, an error occurs.

TokenBucketMeter
^^^^^^^^^^^^^^^^

The :ned:`TokenBucketMeter` module implements a simple token bucket
meter. The module has two outputs: one for green packets and one for red
packets. When a packet arrives, the gained tokens are added to the
bucket, and the number of tokens equal to the size of the packet are
subtracted.

Packets are classified according to two parameters: Committed
Information Rate (:math:`cir`) and Committed Burst Size (:math:`cbs`),
as either green or red.

Green traffic is guaranteed to be under :math:`cir \cdot (t_1 - t_0) + 8 \cdot cbs` in
every :math:`[t_0,t_1]` interval.

SingleRateThreeColorMeter
^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`SingleRateThreeColorMeter` module implements a Single Rate
Three Color Meter (RFC 2697). The module has three outputs: green,
yellow, and red packets.

Packets are classified according to three parameters: Committed
Information Rate (:math:`cir`), Committed Burst Size (:math:`cbs`), and
Excess Burst Size (:math:`ebs`), as either green, yellow, or red. The
green traffic is guaranteed to be under :math:`cir \cdot (t_1 - t_0) + 8 \cdot cbs`
while the green+yellow traffic is guaranteed to be under
:math:`cir \cdot (t_1 - t_0) + 8 \cdot (cbs + ebs)` in every :math:`[t_0,t_1]` interval.

TwoRateThreeColorMeter
^^^^^^^^^^^^^^^^^^^^^^

The :ned:`TwoRateThreeColorMeter` module implements a Two Rate Three
Color Meter (RFC 2698). The module has three output gates for green,
yellow, and red packets.

It classifies packets based on two rates: Peak Information Rate
(:math:`pir`) and Committed Information Rate (:math:`cir`) and their
associated burst sizes (:math:`pbs` and :math:`cbs`), as either green,
yellow, or red. The green traffic is guaranteed to be under
:math:`pir \cdot (t_1 - t_0) + 8 \cdot pbs` and :math:`cir \cdot (t_1 - t_0) + 8 \cdot cbs`,
the yellow traffic is guaranteed to be under
:math:`pir \cdot (t_1 - t_0) + 8 \cdot pbs` in every :math:`[t_0,t_1]` interval.

.. _ug:sec:diffserv:markers:

Markers
~~~~~~~

DSCP markers set the codepoint (DSCP field) of packets. The codepoint
determines the further processing of the packet in the router or in the
core of the DiffServ domain.

The :ned:`DscpMarker` module sets the DSCP field (lower six bits of
the TypeOfService/TrafficClass field) of IP datagrams to the values specified by
the :par:`dscps` parameter. The :par:`dscps` parameter is a space-separated
list of codepoints. A different value can be specified for each input gate,
where the :math:`i^{th}` value is used for packets arriving at the :math:`i^{th}` input gate.
If there are fewer values than gates, the last value is used for any extra gates.

The DSCP values are enumerated in the :file:`DSCP.msg` file. You can
specify DSCP values using either their names or integer values in the :par:`dscps` parameter.

For example, the following lines are equivalent:



.. code-block:: ini

   **.dscps = "EF 0x0a 0b00001000"
   **.dscps = "46 AF11 8"

.. _ug:sec:diffserv:compound-modules:

Compound Modules
----------------

.. _ug:sec:diffserv:afxyqueue:

AFxyQueue
~~~~~~~~~

The :ned:`AFxyQueue` module is an example queue that implements one
class of the Assured Forwarding PHB group (RFC 2597).

Packets with the same AFx class but different drop priorities arrive at
the :gate:`afx1In`, :gate:`afx2In`, and :gate:`afx3In` gates. The
received packets are stored in the same queue. Before the packet is
enqueued, a RED dropping algorithm may decide to selectively drop them,
based on the average length of the queue and the RED parameters of the
drop priority of the packet.

The afxyMinth, afxyMaxth, and afxyMaxp parameters must have values that
ensure that packets with lower drop priorities are dropped with lower or
equal probability than packets with higher drop priorities.

.. _ug:sec:diffserv:diffservqeueue:

DiffservQueue
~~~~~~~~~~~~~

The :ned:`DiffservQueue` is an example queue that can be used in
interfaces of DiffServ core and edge nodes to support the AFxy (RFC 2597)
and EF (RFC 3246) PHBs.

.. figure:: figures/DiffservQueue.*
   :align: center
   :scale: 70 %

The incoming packets are first classified according to their DSCP field.
DSCP values other than AFxy and EF are handled as best effort (BE).

EF packets are stored in a dedicated queue and served first when a
packet is requested. Because they can preempt the other queues, the rate
of the EF packets should be limited to a fraction of the bandwidth of the
link. This is achieved by metering the EF traffic with a token bucket
meter and dropping packets that do not conform to the traffic profile.

There are other queues for AFx classes and BE. The AFx queues use RED to
implement three different drop priorities within the class. BE packets are
stored in a drop tail queue. Packets from AFxy and BE queues are
scheduled by a WRR scheduler, which ensures that the remaining bandwidth
is allocated among the classes according to the specified weights.

