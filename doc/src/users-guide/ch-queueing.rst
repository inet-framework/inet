.. role:: raw-latex(raw)
   :format: latex
..

.. _ug:cha:queueing:

Queueing Model
==============

.. _ug:sec:queueing:overview:

Overview
--------

The INET queueing model provides several reusable modules for various application
areas. They can be used to build application traffic generators, queueing models
for MAC protocols, traffic shaping, and traffic policing models for quality of
service implementations, and so on.

Usage
~~~~~

The queueing modules can be used in two very different ways. For one, they can
be connected to other INET modules using their gates. In this case, the modules
send and receive packets asynchronously as many other INET modules do. For example,
application packet source and packet sink modules are used this way. The other
way to use them, is to directly call their C++ methods through one of the C++
interfaces of the contract package. In this case, the queueing modules are
not connected to other INET modules at all. For example, MAC protocol modules
use packet queues as submodules through C++ method calls.

Model
~~~~~

Most queueing model elements provide simple behaviors, so they are implemented
as simple modules. But queueing elements can also be composed to form more
complex behaviors. For example, priority queues, request-response based traffic
generators, traffic shapers are usually implement as compound modules. In fact,
some of the queueing model elements provided by INET are actually realized as
compound modules using composition.

The queueing model can be found in the :ned:`inet.queueing` NED package. All
queueing model elements implement one or more NED module interfaces and also
the corresponding C++ interfaces from the contract folder. As a minimum they
all implement the :cpp:`IPacketQueueingElement` interface.

Operation
~~~~~~~~~

Internally, connected queueing model elements most often communicate with each
other using synchronous C++ method calls without utilizing :cpp:`handleMessage()`.
The most notable exception is when an operation takes a non-zero simulation
time. For example, when the processing of a packet is delayed.

There are two new important operations on queueing model elements. Packets can
be *pushed* into gates and packets can be *popped* from gates. The main difference
between them is the subject of the activity. In the former case, when a packet
is pushed, the activity is initiated by the *source* of the packet. In contrast,
when a packet is popped, the activity is initiated by the *sink* of the packet.

Queueing model elements can be divided into two categories with respect to the
operation on a given gate: *passive* and *active*. Active elements push packets
into output gates and pop packets from input gates as they see fit. Passive
elements are pushed into and popped from by other connected modules.

The active queueing elements take into consideration the state of the connected
passive elements. That is they push or pop packets only when the passive end is
able to consume or provide accordingly. The queueing model elements also validate
the assembled structure during module initialization with respect to the active
and passive behavior of the connected gates.

The following equation about the number of packets holds true for all queueing
elements:

#pushed - #dropped - #removed - #popped + #created = #available + #delayed

Sources
-------

These modules act as a source of packets. An active packet source pushes packets
to its output. A passive packet source returns a packet when it is popped by
other queueing model elements.

-  :ned:`ActivePacketSource`: generic source that produces packets periodically
-  :ned:`PassivePacketSource`: generic source that provides packets as requested
-  :ned:`BurstyPacketProducer`: mixes two different sources to generate bursty traffic
-  :ned:`QueueFiller`: produces packets to continuously fill a queue
-  :ned:`ResponseProducer`: produces complex response traffic based on the incoming request type
-  :ned:`PcapFilePacketProducer`: replays packets from a PCAP file

Sinks
-----

These modules act as a sink of packets. An active packet sink pops packets from
its input. A passive packet sink is pushed with packets by other queueing model
elements.

-  :ned:`ActivePacketSink`: generic sink that collects packets periodically
-  :ned:`PassivePacketSink`: generic sink that consumes packets as they arrive
-  :ned:`RequestConsumer`: processes incoming requests in order and initiates response traffic
-  :ned:`PcapFilePacketConsumer`: writes packets to a PCAP file

Queues
------

These modules store packets and maintain the order among them. Queues cannot
delay packets, so if a queue is not empty, then a packet is always available.
When a packet is pushed into the input of a queue, then the packet either gets
stored or dropped if the queue is overloaded. When a packet is popped from the
output of a queue, then one of the stored packets is returned.

The following equation about the number of packets always holds true for queues:

#pushed - #dropped - #removed - #popped = #queueLength = #available

-  :ned:`PacketQueue`: generic queue that provides ordering and selective dropping

   parameterizable with an :cpp:`IPacketComparatorFunction` and an :cpp:`IPacketDropperFunction`

-  :ned:`DropHeadQueue`: drops packets at the head of the queue
-  :ned:`DropTailQueue`: drops packets at the tail of the queue, the most commonly used queue
-  :ned:`PriorityQueue`: contains several inner queues using a shared buffer prioritizing over them
-  :ned:`RedMarkerQueue`: combines random early detection with a queue
-  :ned:`CompoundPacketQueue`: allows building complex queues by pure NED composition

Buffers
-------

These modules deal with memory allocation of packets without considering the
order among them. A packet buffer generally doesn't have gates and packets are
not pushed into or popped from it.

-  :ned:`PacketBuffer`: generic buffer that provides shared storage between several queues

   parameterizable with an :cpp:`IPacketDropperFunction`

-  :ned:`PriorityBuffer`: drops packets based on the queue priority

Filters
-------

These modules filter for specific packets while dropping the rest. When a packet
is pushed into the input of a packet filter, then the filter either pushes the
packet to its output or it simply drops the packet. In contrast, when a packet
is popped from the output of a packet filter, then it continuously pops and drops
packets from its input until it finds one that matches the filter criteria.

-  :ned:`PacketFilter`: generic packet filter

   parameterizable with an :cpp:`IPacketFilterFunction`

-  :ned:`ContentBasedDropper`: drops packets based on the data they contain
-  :ned:`OrdinalBasedDropper`: drops packets based on their ordinal number
-  :ned:`RateLimiter`: drops packets above the specified packetrate or datarate
-  :ned:`RedDropper`: drops packets based on random early detection

Classifiers
-----------

These modules classify packets to one of their outputs. When a packet is pushed
into the input of a packet classifier, then it immediately pushes the packet
to one of its outputs.

-  :ned:`PacketClassifier`: generic packet classifier

   parameterizable with an :cpp:`IPacketClassifierFunction`

-  :ned:`ContentBasedClassifier`: classifies packets based on the data they contain
-  :ned:`PriorityClassifier`: classifies packets to the first non-full output
-  :ned:`LabelClassifier`: classifies packets based on the attached labels
-  :ned:`MarkovClassifier`: classifies packets based on the state of a Markov process
-  :ned:`UserPriorityClassifier`: classifies packets based on the attached UserPriorityReq.

Schedulers
----------

These modules schedule packets from one of their inputs. When a packet is popped
from the output of a packet scheduler, then it immediately pops a packet from
one of its inputs and returns that packet.

-  :ned:`PacketScheduler`: generic packet scheduler

   parameterizable with an :cpp:`IPacketSchedulerFunction`

-  :ned:`PriorityScheduler`: schedules packets from the first non-empty source
-  :ned:`WrrScheduler`: schedules packets in a weighted Round-robin manner
-  :ned:`LabelScheduler`: schedules packets based on the attached labels
-  :ned:`MarkovScheduler`: schedules packets based on the state of a Markov process

Servers
-------

These modules process packets in order one by one. A packet server actively pops
packets from its input when it sees fit, and it also actively pushes packets into
its output.

-  :ned:`PacketServer`: serves packets according to the processing time based on packet length
-  :ned:`TokenBasedServer`: serves packets when the required number of tokens are available

Markers
-------

These modules attach some information to packets on an individual basis. Packets
can be both pushed into the input and popped from the output of packet markers.

-  :ned:`PacketLabeler`: generic marker which attaches labels to packets

   parameterizable with an :cpp:`IPacketFilterFunction`

-  :ned:`PacketTagger`: attaches tags to packet such as outgoing interface, hopLimit, VLAN, user priority 
-  :ned:`RedMarker`: random early detection marker

Meters
------

These modules measure some property of a stream of packets. Packets can be both
pushed into the input and popped from the output of packet meters.

-  :ned:`RateMeter`: measuring the packetrate and datarate of the received stream of packets 

Token generators
----------------

These modules generate tokens for other modules. A token generator generally
doesn't have gates and packets are not pushed into or popped from it.

-  :ned:`TimeBasedTokenGenerator`: generates tokens based on elapsed simulation time
-  :ned:`PacketBasedTokenGenerator`: generates tokens based on received packets
-  :ned:`SignalBasedTokenGenerator`: generates tokens based on received signals
-  :ned:`QueueBasedTokenGenerator`: generates tokens based on the state of a queue

Shapers
-------

These modules actively shape traffic by changing the order of packets, dropping
packets, delaying packets, etc. If a shaper is not empty, then a packet is not
necessarily available, because it can delay packets. They are generally built
by composition using other queueing model elements.

-  :ned:`LeakyBucket`: generic shaper with overflow and configurable output rate
-  :ned:`TokenBucket`: generic shaper with overflow and configurable burstiness and output rate

Other generic modules
---------------------

There are also some other generic queueing model elements. Each one has its own
specific purpose and behavior.

-  :ned:`PacketGate`: allows or prevents packets to pass through, either pushed or popped
-  :ned:`PacketMultiplexer`: passively connects multiple inputs to a single output, packets are pushed into the inputs
-  :ned:`PacketDemultiplexer`: passively connects a single input to multiple outputs, packets are popped from the outputs 
-  :ned:`PacketDelayer`: sends received packets to the output with some delay independently
-  :ned:`PacketDuplicator`: sends copies of each received packet to the only output
-  :ned:`PacketCloner`: sends one copy of each received packet to all outputs
-  :ned:`PacketHistory`: keeps track of the last N packets which can be inspected in Qtenv
-  :ned:`OrdinalBasedDuplicator`: copies received packets based on their ordinal number
