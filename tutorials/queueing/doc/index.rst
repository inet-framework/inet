Queueing Tutorial
=================

Explore the use of queueing components in INET, including traffic generators,
queues, and traffic conditioners. These components can be utilized to build
diverse functionalities at various layers of the protocol stack.

Introduction

.. toctree::
   :maxdepth: 1

   gettingstarted

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
