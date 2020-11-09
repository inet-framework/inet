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
The Internet delivers individually each packet, and delivery time is not
guaranteed, moreover packets may even be dropped due to congestion at
the routers of the network. It was assumed that transport protocols, and
applications can overcome these deficiencies. This worked until FTP and
email was the main applications of the Internet, but the newer
applications such as Internet telephony and video conferencing cannot
tolerate delay jitter and loss of data.

The first attempt to add QoS capabilities to the IP routing was
Integrated Services. Integrated services provide resource assurance
through resource reservation for individual application flows. An
application flow is identified by the source and destination addresses
and ports and the protocol id. Before data packets are sent the
necessary resources must be allocated along the path from the source to
the destination. At the hops from the source to the destination each
router must examine the packets, and decide if it belongs to a reserved
application flow. This could cause a memory and processing demand in the
routers. Other drawback is that the reservation must be periodically
refreshed, so there is an overhead during the data transmission too.

Differentiated Services is a more scalable approach to offer a better
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
into DiffServ domains. Within each domain the resources of the domain
are allocated to forwarding classes, taking into account the available
resources and the traffic flows. There are *service level agreements*
(SLA) between the users and service providers, and between the domains
that describe the mapping of packets to forwarding classes and the
allowed traffic profile for each class. The routers at the edge of the
network are responsible for marking the packets and protect the domain
from misbehaving traffic sources. Nonconforming traffic may be dropped,
delayed, or marked with a different forwarding class.

.. _ug:sec:diffserv:implemented-standards:

Implemented Standards
~~~~~~~~~~~~~~~~~~~~~

The implementation follows these RFCs below:

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
:ned:`EthernetCsmaMac` use an internal drop-tail queue to buffer the packets
while the line is busy.

.. _ug:sec:diffserv:traffic-conditioners:

Traffic Conditioners
~~~~~~~~~~~~~~~~~~~~

Traffic conditioners have one input and one output gate as defined in
the :ned:`ITrafficConditioner` interface. They can transform the
incoming traffic by dropping or delaying packets. They can also set the
DSCP field of the packet, or mark them other way, for differentiated
handling in the queues.

Traffic conditioners perform the following actions:

-  classify the incoming packets

-  meter the traffic in each class

-  marks/drops packets depending on the result of metering

-  shape the traffic by delaying packets to conform to the desired
   traffic profile

INET provides classifier, meter, and marker modules, that can be
composed to build a traffic conditioner as a compound module.

.. _ug:sec:diffserv:output-queues:

Output Queues
~~~~~~~~~~~~~

Queue components must implement the :ned:`IPacketQueue` module
interface. In addition to having one input and one output gate, these
components must implement a passive queue behaviour: they only deliver a
packet when the module connected to their output explicitly requests it.
(In C++ terms, the module must implement the :cpp:`IPacketQueue`
interface. The next module requests a packet by calling the
:fun:`requestPacket()` method of that interface.)

.. _ug:sec:diffserv:simple-modules:

Simple modules
--------------

This section describes the primitive elements from which traffic
conditioners and output queues can be built. The next sections shows
some examples, how these queues, schedulers, droppers, classifiers,
meters, markers can be combined.

The type of the components are:

-  ``queue``: container of packets, accessed as FIFO

-  ``dropper``: attached to one or more queue, it can limit the queue
   length below some threshold by selectively dropping packets

-  ``scheduler``: decide which packet is transmitted first, when more
   packets are available on their inputs

-  ``classifier``: classify the received packets according to their
   content (e.g. source/destination, address and port, protocol, dscp
   field of IP datagrams) and forward them to the corresponding output
   gate.

-  ``meter``: classify the received packets according to the temporal
   characteristic of their traffic stream

-  ``marker``: marks packets by setting their fields to control their
   further processing

.. _ug:sec:diffserv:queues:

Queues
~~~~~~

When packets arrive at higher rate, than the interface can trasmit, they
are getting queued.

Queue elements store packets until they can be transmitted. They have
one input and one output gate. Queues may have one or more thresholds
associated with them.

Received packets are enqueued and stored until the module connected to
their output asks a packet by calling the :fun:`requestPacket()`
method.

They should be able to notify the module connected to its output about
the arrival of new packets.

.. _ug:sec:diffserv:fifo-queue:

FIFO Queue
^^^^^^^^^^

The :ned:`FifoQueue` module implements a passive FIFO queue with
unlimited buffer space. It can be combined with algorithmic droppers and
schedulers to form an IPacketQueue compound module.

The C++ class implements the :cpp:`IQueueAccess` and
:cpp:`IPacketQueue` interfaces.

.. _ug:sec:diffserv:droptailqueue:

DropTailQueue
^^^^^^^^^^^^^

The other primitive queue module is :ned:`DropTailQueue`. Its capacity
can be specified by the :par:`packetCapacity` parameter. When the number
of stored packet reached the capacity of the queue, further packets are
dropped. Because this module contains a built-in dropping strategy, it
cannot be combined with algorithmic droppers as :ned:`FifoQueue` can be.
However its output can be connected to schedulers.

This module implements the :ned:`IPacketQueue` interface, so it can be
used as the queue component of interface card per se.

.. _ug:sec:diffserv:droppers:

Droppers
~~~~~~~~

Algorithmic droppers selectively drop received packets based on some
condition. The condition can be either deterministic (e.g. to bound the
queue length), or probabilistic (e.g. RED queues).

Other kind of droppers are absolute droppers; they drop each received
packet. They can be used to discard excess traffic, i.e. packets whose
arrival rate exceeds the allowed maximum. In INET the :ned:`Sink` module
can be used as an absolute dropper.

The algorithmic droppers in INET are :ned:`ThresholdDropper` and
:ned:`RedDropper`. These modules has multiple input and multiple output
gates. Packets that arrive on gate :gate:`in[i]` are forwarded to gate
:gate:`out[i]` (unless they are dropped). However the queues attached to
the output gates are viewed as a whole, i.e. the queue length parameter
of the dropping algorithm is the sum of the individual queue lengths.
This way we can emulate shared buffers of the queues. Note, that it is
also possible to connect each output to the same queue module.

.. _ug:sec:diffserv:threshold-dropper:

Threshold Dropper
^^^^^^^^^^^^^^^^^

The :ned:`ThresholdDropper` module selectively drops packets, based on
the available buffer space of the queues attached to its output. The
buffer space can be specified as the count of packets, or as the size in
bytes.

The module sums the buffer lengths of its outputs and if enqueuing a
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
:cpp:`IQueueAccess` C++ interface (e.g. :ned:`FifoQueue`).

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

Scheduler modules decide which queue can send a packet, when the
interface is ready to transmit one. They have several input gates and
one output gate.

Modules that are connected to the inputs of a scheduler must implement
the :cpp:`IPacketQueue` C++ interface. Schedulers also implement
:cpp:`IPacketQueue`, so they can be cascaded to other schedulers, and
can be used as the output module of :ned:`IPacketQueue`’s.

There are several possible scheduling discipline (first come/first
served, priority, weighted fair, weighted round-robin, deadline-based,
rate-based). INET contains implementation of priority and weighted
round-robin schedulers.

.. _ug:sec:diffserv:priority-scheduler:

Priority Scheduler
^^^^^^^^^^^^^^^^^^

The :ned:`PriorityScheduler` module implements a strict priority
scheduler. Packets that arrived on :gate:`in[0]` has the highest
priority, then packets arrived on :gate:`in[1]`, and so on. If more
packets available when one is requested, then the one with highest
priority is chosen. Packets with lower priority are transmitted only
when there are no packets on the inputs with higher priorities.

:ned:`PriorityScheduler` must be used with care, because a large volume
of higher packets can starve lower priority packets. Therefore it is
necessary to limit the rate of higher priority packets to a fraction of
the output datarate.

:ned:`PriorityScheduler` can be used to implement the ``EF`` PHB.

Weighted Round Robin Scheduler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`WrrScheduler` module implements a weighted round-robin
scheduler. The scheduler visits the input gates in turn and selects the
number of packets for transmission based on their weight.

For example if the module has three input gates, and the weights are 3,
2, and 1, then packets are transmitted in this order:

::

   A, A, A, B, B, C, A, A, A, B, B, C, ...

where A packets arrived on :gate:`in[0]`, B packets on :gate:`in[1]`,
and C packets on :gate:`in[2]`. If there are no packets in the current
one when a packet is requested, then the next one is chosen that has
enough tokens.

If the size of the packets are equal, then :ned:`WrrScheduler` divides
the available bandwith according to the weights. In each case, it
allocates the bandwith fairly. Each flow receives a guaranteed minimum
bandwith, which is ensured even if other flows exceed their share (flow
isolation). It is also efficiently uses the channel, because if some
traffic is smaller than its share of bandwidth, then the rest is
allocated to the other flows.

:ned:`WrrScheduler` can be used to implement the ``AFxy`` PHBs.

.. _ug:sec:diffserv:classifiers:

Classifiers
~~~~~~~~~~~

Classifier modules have one input and many output gates. They examine
the received packets, and forward them to the appropriate output gate
based on the content of some portion of the packet header. You can read
more about classifiers in RFC 2475 and RFC 3290.

The ``inet.networklayer.diffserv`` package contains two classifiers:
:ned:`MultiFieldClassifier` to classify the packets at the edge routers
of the DiffServ domain, and :ned:`BehaviorAggregateClassifier` to
classify the packets at the core routers.

Multi-field Classifier
^^^^^^^^^^^^^^^^^^^^^^

The :ned:`MultiFieldClassifier` module can be used to identify
micro-flows in the incoming traffic. The flow is identified by the
source and destination addresses, the protocol id, and the source and
destination ports of the IP packet.

The classifier can be configured by specifying a list of filters. Each
filter can specify a source/destination address mask, protocol,
source/destination port range, and bits of TypeOfService/TrafficClass
field to be matched. They also specify the index of the output gate
matching packet should be forwarded to. The first matching filter
determines the output gate, if there are no matching filters, then
:gate:`defaultOut` is chosen.

The configuration of the module is given as an XML document. The
document element must contain a list of ``<filter>`` elements. The
filter element has a mandatory ``@gate`` attribute that gives the
index of the gate for packets matching the filter. Other attributes are
optional and specify the condition of matching:

-  ``@srcAddress``, ``@srcPrefixLength``: to match the source
   address of the IP

-  ``@destAddress``, ``@destPrefixLength``:

-  ``@protocol``: matches the protocol field of the IP packet. Its
   value can be a name (e.g. “udp”, “tcp”), or the numeric code of the
   protocol.

-  ``@tos``,@tosMask: matches bits of the TypeOfService/TrafficClass
   field of the IP packet.

-  ``@srcPort``: matches the source port of the TCP or UDP packet.

-  ``@srcPortMin``, ``@srcPortMax``: matches a range of source
   ports.

-  ``@destPort``: matches the destination port of the TCP or UDP
   packet.

-  ``@destPortMin``, ``@destPortMax``: matches a range of
   destination ports.

The following example configuration specifies

-  to transmit packets received from the 192.168.1.x subnet on gate 0,

-  to transmit packets addressed to port 5060 on gate 1,

-  to transmit packets having CS7 in their DSCP field on gate 2,

-  to transmit other packets on :gate:`defaultGate`.



.. code-block:: xml

   <filters>
     <filter srcAddress="192.168.1.0" srcPrefixLength="24" gate="0"/>
     <filter protocol="udp" destPort="5060" gate="1"/>
     <filter tos="0b00111000" tosMask="0x3f" gate="2"/>
   </filters>

Behavior Aggregate Classifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`BehaviorAggregateClassifier` module can be used to read the
DSCP field from the IP datagram, and direct the packet to the
corresponding output gate. The DSCP value is the lower six bits of the
TypeOfService/TrafficClass field. Core routers usually use this
classifier to guide the packet to the appropriate queue.

DSCP values are enumerated in the :par:`dscps` parameter. The first
value is for gate :gate:`out[0]`, the second for :gate:`out[1]`, so on.
If the received packet has a DSCP value not enumerated in the
:par:`dscps` parameter, it will be forwarded to the :gate:`defaultOut`
gate.

.. _ug:sec:diffserv:meters:

Meters
~~~~~~

Meters classify the packets based on the temporal characteristics of
their arrival. The arrival rate of packets is compared to an allowed
traffic profile, and packets are decided to be green (in-profile) or red
(out-of-profile). Some meters apply more than two conformance level,
e.g. in three color meters the partially conforming packets are
classified as yellow.

The allowed traffic profile is usually specified by a token bucket. In
this model, a bucket is filled in with tokens with a specified rate,
until it reaches its maximum capacity. When a packet arrives, the bucket
is examined. If it contains at least as many tokens as the length of the
packet, then that tokens are removed, and the packet marked as
conforming to the traffic profile. If the bucket contains less tokens
than needed, it left unchanged, but the packet marked as non-conforming.

Meters has two modes: color-blind and color-aware. In color-blind mode,
the color assigned by a previous meter does not affect the
classification of the packet in subsequent meters. In color-aware mode,
the color of the packet can not be changed to a less conforming color:
if a packet is classified as non-conforming by a meter, it also handled
as non-conforming in later meters in the data path.



.. important::

   Meters take into account the length of the IP packet only, L2 headers are omitted
   from the length calculation. If they receive a packet which is not
   an IP datagram and does not encapsulate an IP datagram, an error occurs.

TokenBucketMeter
^^^^^^^^^^^^^^^^

The :ned:`TokenBucketMeter` module implements a simple token bucket
meter. The module has two output, one for green packets, and one for red
packets. When a packet arrives, the gained tokens are added to the
bucket, and the number of tokens equal to the size of the packet are
subtracted.

Packets are classified according to two parameters, Committed
Information Rate (:math:`cir`), Committed Burst Size (:math:`cbs`), to
be either green, or red.

Green traffic is guaranteed to be under :math:`cir*(t_1-t_0)+8*cbs` in
every :math:`[t_0,t_1]` interval.

SingleRateThreeColorMeter
^^^^^^^^^^^^^^^^^^^^^^^^^

The :ned:`SingleRateThreeColorMeter` module implements a Single Rate
Three Color Meter (RFC 2697). The module has three output for green,
yellow, and red packets.

Packets are classified according to three parameters, Committed
Information Rate (:math:`cir`), Committed Burst Size (:math:`cbs`), and
Excess Burst Size (:math:`ebs`), to be either green, yellow or red. The
green traffic is guaranteed to be under :math:`cir*(t_1-t_0)+8*cbs`,
while the green+yellow traffic to be under
:math:`cir*(t_1-t_0)+8*(cbs+ebs)` in every :math:`[t_0,t_1]` interval.

TwoRateThreeColorMeter
^^^^^^^^^^^^^^^^^^^^^^

The :ned:`TwoRateThreeColorMeter` module implements a Two Rate Three
Color Meter (RFC 2698). The module has three output gates for the green,
yellow, and red packets.

It classifies the packets based on two rates, Peak Information Rate
(:math:`pir`) and Committed Information Rate (:math:`cir`), and their
associated burst sizes (:math:`pbs` and :math:`cbs`) to be either green,
yellow or red. The green traffic is under :math:`pir*(t_1-t_0)+8*pbs`
and :math:`cir*(t_1-t_0)+8*cbs`, the yellow traffic is under
:math:`pir*(t_1-t_0)+8*pbs` in every :math:`[t_0,t_1]` interval.

.. _ug:sec:diffserv:markers:

Markers
~~~~~~~

DSCP markers sets the codepoint of the crossing packets. The codepoint
determines the further processing of the packet in the router or in the
core of the DiffServ domain.

The :ned:`DscpMarker` module sets the DSCP field (lower six bit of
TypeOfService/TrafficClass) of IP datagrams to the value specified by
the :par:`dscps` parameter. The :par:`dscps` parameter is a space
separated list of codepoints. You can specify a different value for each
input gate; packets arrived at the :math:`i^{th}` input gate are marked
with the :math:`i^{th}` value. If there are fewer values, than gates,
then the last one is used for extra gates.

The DSCP values are enumerated in the :file:`DSCP.msg` file. You can
use both names and integer values in the :par:`dscps` parameter.

For example the following lines are equivalent:



.. code-block:: ini

   **.dscps = "EF 0x0a 0b00001000"
   **.dscps = "46 AF11 8"

.. _ug:sec:diffserv:compound-modules:

Compound modules
----------------

.. _ug:sec:diffserv:afxyqueue:

AFxyQueue
~~~~~~~~~

The :ned:`AFxyQueue` module is an example queue, that implements one
class of the Assured Forwarding PHB group (RFC 2597).

Packets with the same AFx class, but different drop priorities arrive at
the :gate:`afx1In`, :gate:`afx2In`, and :gate:`afx3In` gates. The
received packets are stored in the same queue. Before the packet is
enqueued, a RED dropping algorithm may decide to selectively drop them,
based on the average length of the queue and the RED parameters of the
drop priority of the packet.

The afxyMinth, afxyMaxth, and afxyMaxp parameters must have values that
ensure that packets with lower drop priorities are dropped with lower or
equal probability than packets with higher drop priorities.

.. _ug:sec:diffserv:diffservqeueue:

DiffservQeueue
~~~~~~~~~~~~~~

The :ned:`DiffservQueue` is an example queue, that can be used in
interfaces of DS core and edge nodes to support the AFxy (RFC 2597) and
EF (RFC 3246) PHB’s.

.. figure:: figures/DiffservQueue.*
   :align: center
   :scale: 70 %

The incoming packets are first classified according to their DSCP field.
DSCP’s other than AFxy and EF are handled as BE (best effort).

EF packets are stored in a dedicated queue, and served first when a
packet is requested. Because they can preempt the other queues, the rate
of the EF packets should be limited to a fraction of the bandwith of the
link. This is achieved by metering the EF traffic with a token bucket
meter and dropping packets that does not conform to the traffic profile.

There are other queues for AFx classes and BE. The AFx queues use RED to
implement 3 different drop priorities within the class. BE packets are
stored in a drop tail queue. Packets from AFxy and BE queues are
sheduled by a WRR scheduler, which ensures that the remaining bandwith
is allocated among the classes according to the specified weights.
