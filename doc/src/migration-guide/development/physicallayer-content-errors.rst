Content Errors in the Physical Layer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Failed Regions of a Reception Decision
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:cpp:`IReceptionDecision` has a new pure virtual method:

.. code-block:: c++

   virtual const std::vector<ErrorRegion>& getErrorRegions() const = 0;

It returns the regions of the transmitted packet that the decision found
erroneous. An :cpp:`ErrorRegion` has the offset and the length of the region in
the transmitted packet, and the error rate of one bit of the region. An
:cpp:`IReceptionDecision` implementation outside INET no longer compiles until
it overrides the method; it can return an empty list. :cpp:`ReceptionDecision`
takes the regions as an optional last constructor argument.

:cpp:`ReceiverBase::computeReceptionResult()` now calls the new protected
overload :cpp:`computeReceivedPacket(snir, decisions)`, which calls the overload
with the success flag as before. A receiver that needs the decisions for the
received packet overrides the new overload.

Corruption of Failed Regions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The enum :cpp:`CorruptionMode` moved from :cpp:`ErrorModelBase` to
:cpp:`IErrorModel`; the name :cpp:`ErrorModelBase::CorruptionMode` still refers
to it. :cpp:`IErrorModel` has two new pure virtual methods:

.. code-block:: c++

   virtual CorruptionMode getCorruptionMode() const = 0;
   virtual Packet *computeCorruptedPacket(const Packet *packet, const std::vector<ErrorRegion>& regions) const = 0;

The second method gives the received packet with the failed regions that the
error decisions of the receiver found. A model that derives from
:cpp:`ErrorModelBase` inherits both methods. It can change the corrupted form of
a region with the protected :cpp:`computeCorruptedChunk()`, and
:cpp:`corruptUnits()` gives the units with errors of a failed region for a unit
error rate. An :cpp:`IErrorModel` implementation outside INET no longer compiles
until it overrides the two methods.

Error Decisions of Signal Regions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the ``chunk``, ``byte``, and ``bit`` corruption modes,
:cpp:`FlatReceiverBase::computeIsPartSuccessful()` now decides one region for
each chunk of a part, and it has a new last parameter for the failed regions:

.. code-block:: c++

   virtual bool computeIsPartSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir, std::vector<ErrorRegion>& errorRegions) const;

The decision of a part keeps its failed regions, and the decision of the whole
signal keeps the failed regions of the data. :cpp:`FlatReceiverBase` overrides
:cpp:`computeReceivedPacket(snir, decisions)` and gives these regions to
:cpp:`IErrorModel::computeCorruptedPacket(packet, regions)`. A failed header
gives a packet-level bit error and no regions.

:cpp:`IErrorModel::computeCorruptedPacket(const ISnir *snir)` no longer exists,
because no mode corrupts the whole packet after a decision of the whole
reception. The functions :cpp:`corruptPacket()`, :cpp:`corruptChunks()`,
:cpp:`corruptBytes()`, :cpp:`corruptBits()`, :cpp:`hasProbabilisticError()`,
and :cpp:`computeCorruptedPacket(packet, ber)` of :cpp:`ErrorModelBase` no
longer exist either, and :cpp:`FlatReceiverBase` no longer overrides
:cpp:`computeReceivedPacket(snir, isReceptionSuccessful)`. A MAC that reads a
received packet in these modes must check the incorrect flags of the chunks
before it reads their content, for example with :cpp:`Packet::hasBitError()`.
