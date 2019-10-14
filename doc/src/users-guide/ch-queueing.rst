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
areas. These modules can be used to build application traffic generators, queueing
models for MAC protocols, traffic shaping, and traffic policing models for quality
of service implementations. Most queueing model elements are simple modules but
they are designed to be composable to form more complex behaviors.

The queueing model can be found in the :ned:`inet.queueing` NED package. All
elements implement the :cpp:`IPacketQueueingElement` interface.

different external structure:
 - queues as submodules without being connected to the outside world via gates
   the queue is not in the packet path, it's rather part of the processing module
 - queues which are connected to generators and consumers
   the queue is in the packet path

different modes of operation:
 - queues must be able to operate synchronously without utilizing handleMessage
   a packet getting into a queue may immediately cause another packet to get out from the queue
 - queues must be able to operate asynchronously with utilizing handleMessage
   this is the old INET 4.0 behaviour

abstraction via composition:
 - queues can be simple modules
 - queues can be composed from simple modules into compound queue modules

easy access through C++ interfaces:
 - protocols should be able to use queues without knowing what is the queue's
   module structure, mode of operation, simple or compound, etc.

TODO

The available modules can be used on their own or they can also be combined in
many ways to create the desired complex behavior for the above purposes. In order
to avoid unexpected runtime errors, the queueing modules validate the assembled
structure during module initialization.

 - component categorization
 - composition rules
   - validation of the assembled structure during module initialization with respect to push/pop gates
   - the following is true for all queueing elements (statistics)
     - #pushed - #dropped - #removed - #popped + #generated = #available + #delayed
 - some selected components

send packets along connections with synchronous method calls but towards non queueing elements asynchronous message sends
queueing elements directly call each other using C++ functions
terminology related push and pop and passive vs. active

Sources
-------

These modules produce packets either actively when they see fit or passively when
they are requested to do so.

-  :ned:`PassivePacketSource`: provides packets with configurable length and content upon request
-  :ned:`ActivePacketSource`: produces packets periodically
-  :ned:`BurstyPacketProducer`: compound module with two different sources to generate bursty traffic
-  :ned:`ResponseProducer`: produces complex response traffic based on the incoming request type
-  :ned:`PcapFilePacketProducer`: reads packets from a PCAP file and sends them based on timestamps

Sinks
-----

These modules consume packets either passively when they arrive or actively when
they see fit.

-  :ned:`PassivePacketSink`: drops packets
-  :ned:`ActivePacketSink`: pops packets from source periodically
-  :ned:`RequestConsumer`: enqueues incoming requests, processes them in order, and initiates response traffic
-  :ned:`PcapFilePacketConsumer`: writes packets to a PCAP file

Queues
------

These modules enqueue packets.
PacketQueue cannot delay packets, if a packetqueue is not empty then it can be popped!
for any queue the following holds true:
 - pushed - dropped - removed - popped + generated = queue length = available + delayed
 - generated = 0
 - delayed = 0
 - #pushed - #dropped - #removed - #popped = #queueLength = #available

-  :ned:`PacketQueue`: a generic packet queue parameterizable with an :cpp:`IPacketComparatorFunction` and an :cpp:`IPacketDropperFunction` 
-  :ned:`DropHeadQueue`: a packet queue which drops packets at the head
-  :ned:`DropTailQueue`: the most commonly used packet queue
-  :ned:`PriorityQueue`: several inner queues and a shared buffer
-  :ned:`CompoundPacketQueue`: allows building complex queues by pure NED composition

Buffers
-------

These modules maintain memory allocation for packets.

-  :ned:`PacketBuffer`: provides sharing storage space between several packet queues with an :cpp:`IPacketDropperFunction`
-  :ned:`PriorityBuffer`: drops packets based on the queue priority

Filters
-------

These modules filter for packets.
push -> push or drop
pop -> pop but may drop several packets and may fail

-  :ned:`PacketFilter`: generic packet filter parameterizable with an :cpp:`IPacketFilterFunction`
-  :ned:`RateLimiter`:
-  :ned:`OrdinalBasedDropper`:

Classifiers
-----------

These modules classify packets.

-  :ned:`PacketClassifier`: generic packet classifier parameterizable with an :cpp:`IPacketClassifierFunction`
-  :ned:`MarkerClassifier`: classifier based on label
-  :ned:`PriorityClassifier`: first non-full sink
-  :ned:`MarkovClassifier`: classifies packets based on the state of a Markov process
TODO: UserPriorityClassifier

Schedulers
----------

These modules schedule packets.

-  :ned:`PacketScheduler`: generic packet scheduler parameterizable with an :cpp:`IPacketSchedulerFunction`
-  :ned:`PriorityScheduler`: 
-  :ned:`WrrScheduler`: schedules packets in a weighted Round-robin manner
-  :ned:`MarkovScheduler`: schedules packets based on the state of a Markov process

Servers
-------

These modules serve packets.

-  :ned:`PacketServer`:
-  :ned:`TokenBasedServer`:

Markers
-------

-  :ned:`PacketMarker`: 

Meters
------

-  :ned:`RateMeter`: 

Token generators
----------------

These modules generate tokens for other modules.

-  :ned:`QueueBasedTokenGenerator`:
-  :ned:`PacketBasedTokenGenerator`:
-  :ned:`TimeBasedTokenGenerator`:

RED modules
-----------

Random early detection modules.

-  :ned:`RedDropper`:
-  :ned:`RedMarker`:
-  :ned:`RedMarkerQueue`:

Other generic modules
---------------------

There are some other generic modules.

-  :ned:`PacketMultiplexer`: passively connects multiple inputs to a single output, packets are pushed into the input
-  :ned:`PacketDemultiplexer`: passively connects a single input to multiple outputs, packets are popped from the output 
-  :ned:`PacketDelayer`: sends received packets to the output with delay independently of each other
-  :ned:`PacketDuplicator`: sends copies of received packets to the same output
-  :ned:`PacketCloner`: sends copies of received packets to all outputs
-  :ned:`PacketHistory`: keeps track of the last N packets which can be inspected in Qtenv

Traffic generators
------------------

All sources and sinks but more specifically.

-  :ned:`RequestConsumer`
-  :ned:`ResponseProducer`

Traffic shapers
---------------

These modules bucket...

-  :ned:`LeakyBucket`:
-  :ned:`TokenBucket`:
