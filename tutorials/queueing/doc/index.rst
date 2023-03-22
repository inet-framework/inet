Queueing Tutorial
=================

INET contains a queueing library which provides various components
such as traffic generators, queues, and traffic conditioners.
Elements of the library can be used to assemble queueing functionality
that can be
used at layer 2 and layer 3 of the protocol stack. With extra elements, the queueing library can also be used
to define custom application behavior without C++ programming.

Each step in this tutorial demonstrates one of the available queueing elements,
with a few more complex examples at the end.
Note that most of the available elements are demonstrated here, but not all
(for example, elements specific to DiffServ are omitted from here).
See the INET Reference for the complete list of elements.

Sources and Sinks

.. toctree::
   :maxdepth: 1

   ActiveSourcePassiveSink
   PassiveSourceActiveSink

Queues and Buffers

.. toctree::
   :maxdepth: 1

   Queue
   DropTailQueue
   Comparator
   Buffer

Classifying Packets from One Input to Multiple Outputs

.. toctree::
   :maxdepth: 1

   PriorityClassifier
   WrrClassifier
   ContentBasedClassifier
   MarkovClassifier
   GenericClassifier

Scheduling Packets from Multiple Inputs to One Output

.. toctree::
   :maxdepth: 1

   PriorityScheduler
   WrrScheduler
   ContentBasedScheduler
   MarkovScheduler
   GenericScheduler

Advanced Queues and Buffers

.. toctree::
   :maxdepth: 1

   PriorityBuffer
   PriorityQueue
   CompoundQueue

Filtering and Dropping Packets

.. toctree::
   :maxdepth: 1

   Filter1
   Filter2
   OrdinalBasedDropper
   RedDropper

Actively Serving Packets from a Passive Source

.. toctree::
   :maxdepth: 1

   Server
   TokenBasedServer

Generating Tokens for a Token-based Server

.. toctree::
   :maxdepth: 1

   TimeBasedTokenGenerator
   PacketBasedTokenGenerator
   QueueBasedTokenGenerator
   SignalBasedTokenGenerator

Markers and Meters

.. toctree::
   :maxdepth: 1

   Meter
   Tagger
   ContentBasedTagger
   Labeler

Traffic Conditioning

.. toctree::
   :maxdepth: 1

   LeakyBucket
   TokenBucket

Other Generic Elements

.. toctree::
   :maxdepth: 1

   Delayer
   Multiplexer
   Demultiplexer
   Gate1
   Gate2
   Duplicator
   OrdinalBasedDuplicator
   Cloner

Advanced Sources and Sinks

.. toctree::
   :maxdepth: 1

   QueueFiller
   RequestResponse

Some Complex Examples

.. toctree::
   :maxdepth: 1

   Telnet
   Network
   InputQueueSwitching
   OutputQueueSwitching
