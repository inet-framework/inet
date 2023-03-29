:orphan:

.. _dg:cha:naming-conventions:

Naming Conventions
==================

Overview
--------

Folders
-------

no plurals (package names)

-  base
-  contract
-  common
-  model: a collection of models for a similar purpose?

-  *cache
-  bitlevel/packetlevel
-  physicallayer/linklayer/networklayer/transportlayer


Modules
-------

-  Omitted*: optional modules using OmittedModule
-  *Layer: compound protocol layers with dual upper/lower gates
-  *Inserter: header insertion with in/out gates
-  *Checker: header checking and removal with in/out gates
-  *Processor: generic command and packet processing with in/out gates
-  I*: interfaces
-  *Base: base modules
-  Compound*: compound variants
-  *Table: storing information used by several other modules
-  *Configurator: configuring various aspects across a larger scope
-  Ext*: modules communicating to the external world (OS)
-  Ieee*: standard specific models
-  Ep*: energy/power related
-  Cc*: charge/current related
-  *Source, *Sink, *Queue, *Server, *Buffer, *Filter, *Classifier, *Scheduler, *Gate: queueing model elements
-  *CanvasVisualizer, *OsgVisualizer: visualizer modules
-  *Host, *Router: network nodes
-  *6: IPv6 variants
-  *Dual: IPv4 & IPv6 supporting

C++ Classes
-----------

-  *Chunk: packet data representation
-  *Serializer: protocol specific header and message serialiers
-  *ProtocolDissector: protocol specific header and message dissectors
-  *ProtocolPrinter: protocol specific header and message printers
-  *Signal: physical layer signals (in contrast with binary data)
-  *Impl: hidden implementation
-  *Function: classes wrapping a function

Packets
-------

-  *Header
-  *Trailer
-  *Fcs
-  *Packet
-  *Frame protocol messages
-  *Command protocol specific packet independent requests

Tags
----
-  *Tag packet specific (meta information)
-  *Req
-  *Ind packet specific protocol requests and indications (meta information)

Signals
-------

-  *CreatedSignal
-  *DeletedSignal
-  *AddedSignal
-  *ChangedSignal
-  *SentSignal
-  *ReceivedSignal
-  *StartedSignal
-  *EndedSignal
