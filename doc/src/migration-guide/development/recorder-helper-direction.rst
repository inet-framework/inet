PCAP Recorder Helpers
~~~~~~~~~~~~~~~~~~~~~

``PcapRecorder::IHelper::tryConvertToLinkType()`` has a new last parameter.
A helper outside INET must change its override to this signature:

.. code-block:: c++

   Packet *tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset,
           PcapLinkType pcapLinkType, const Protocol *protocol, Direction direction) const override;

``direction`` is ``DIRECTION_OUTBOUND`` for a packet that the network interface
sent, and ``DIRECTION_INBOUND`` for a packet that it received. A helper that
does not need the direction ignores the parameter.

``PcapRecorder`` now asks the helpers in ``protocolToLinkType()`` and
``matchesLinkType()`` before it uses its built-in table. A helper that gives a
link type for a protocol of the built-in table, for example ``ieee80211Mac``,
now changes the records of that protocol. A helper that gives
``LINKTYPE_INVALID`` and ``false`` for such a protocol changes nothing.
