Separate Error Decisions of the Header and the Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Error Rates of Signal Regions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:cpp:`IErrorModel` has two new pure virtual methods:

.. code-block:: c++

   virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const = 0;
   virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const = 0;

The methods give the error rates of a region of the header part or the data
part. The region has ``length`` content bits, and the transmitter sends it from
``startTime`` to ``endTime``. The first method returns the probability of at
least one error in the region, and the second one the error rate of one bit of
the region. An :cpp:`IErrorModel` implementation outside INET no longer
compiles until it overrides both methods.

A model that derives from :cpp:`ErrorModelBase` overrides only
:cpp:`computeBitErrorRate()`. The base class computes the region rate as
``1 - (1 - bit error rate)^length``. Use the SNIR of the interval, for example
with :cpp:`ErrorModelBase::getScalarSnir(snir, startTime, endTime)`. The regions
of a part must share all bits that the rate of the part counts, also the bits
that the packet does not hold as content. Then the rates of the regions of the
header and the data give the rate of the whole signal at a constant SNIR. A
model that declares its own :cpp:`computePacketErrorRate()` needs
``using ErrorModelBase::computePacketErrorRate;`` to keep the region overload
visible.

A model that derives from :cpp:`Ieee80211ErrorModelBase` and overrides
:cpp:`getDataSuccessRate()` gets the exact number of bits to count. For an OFDM
mode, :cpp:`computeDataLength()` now adds the SERVICE bits before the call,
and :cpp:`getDataSuccessRate()` no longer adds them. The success rate must be a
power of the number of bits, because the region rates call the method for one
bit.

Bits of the Signal Parts
^^^^^^^^^^^^^^^^^^^^^^^^

A receiver that derives from :cpp:`FlatReceiverBase` names the bits of its
header part and its data part:

.. code-block:: c++

   virtual std::optional<PartRange> computePartRange(const ITransmission *transmission, IRadioSignal::SignalPart part) const override;

:cpp:`PartRange` gives the number of bits of the part, whether the transmitted
packet holds them as content, and their offset in the packet. A header that the
radio does not represent as a chunk has ``isInPacket = false``. The method
returns no value if the packet does not hold the complete part. The
implementation of :cpp:`FlatReceiverBase` throws an error.
:cpp:`computeRegionInterval()` gives the time in which the transmitter sends a
range of the bits of a part; a receiver can override it for a time map that is
not in proportion to the bits.
