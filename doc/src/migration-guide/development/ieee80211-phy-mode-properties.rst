IEEE 802.11 PHY Mode Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The physical layer (PHY) mode interface ``IIeee80211Mode`` gains three pure
virtual methods. Direct implementations outside INET must implement them:

.. code-block:: c++

   ModulationClass getModulationClass() const override;
   PreambleType getLegacyPreambleType() const override;
   bps getNonHtReferenceRate() const override;

``ModulationClass`` identifies the mode family. Its values are ``UNKNOWN``,
``DSSS_HRDSSS``, ``OFDM``, ``ERP_OFDM``, ``HT``, and ``VHT``.
HT means High Throughput; VHT means Very High Throughput.

``PreambleType`` describes the legacy preamble, which precedes the frame header.
Its values are ``UNKNOWN``, ``LONG``, ``SHORT``, and ``NOT_APPLICABLE``.
``LONG`` and ``SHORT`` apply only to legacy direct-sequence modes.
OFDM, HT, and VHT modes return ``NOT_APPLICABLE`` for this query.
OFDM means orthogonal frequency-division multiplexing.

``getNonHtReferenceRate()`` returns a bound for response-rate selection, in
bits per second. Supported legacy modes return their data rate. HT and VHT
modes derive the bound from the modulation and code rate of stream 1.
The bound does not depend on channel width, guard interval, or stream count.
An unsupported mapping returns ``bps(NaN)``, where NaN means not a number.
Check the value with ``std::isnan(rate.get())`` before rate comparisons.

Subclasses of ``Ieee80211ModeBase`` inherit defaults and need no new override
to compile. Those defaults return ``UNKNOWN`` for both enum queries and
``bps(NaN)`` for the reference rate.
Override each query that the custom mode supports.
Custom rate selectors can use these queries instead of concrete mode casts
or a second copy of the reference-rate formula.
