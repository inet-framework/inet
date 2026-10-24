PCAP Writers
~~~~~~~~~~~~

Custom capture writers must support metadata before the selected bytes of a ``Packet``. The writer interface, ``IPcapWriter``, requires the new pure virtual method ``writePacketWithPrefix()`` for this purpose. A custom implementation without this method no longer compiles. Implement this method in your writer. Alternatively, derive from a built-in writer that implements it. Use this signature:

.. code-block:: c++

   void writePacketWithPrefix(simtime_t time, const std::vector<uint8_t>& prefix,
       const Packet *packet, b frontOffset, b backOffset, Direction direction,
       NetworkInterface *ie, PcapLinkType linkType) override;

Write the prefix followed by the selected range of the ``Packet``. Count both parts in the original record length. Apply the capture length limit to their combined length. If the limit falls inside the prefix, write only that part of the prefix. Do not modify the ``Packet``. Do not take ownership of the ``Packet``. The existing ``writePacket()`` method remains available for records without a prefix.

For a hypothetical record, an eight-byte prefix precedes a selected range of 24 bytes. With a capture length limit of 20 bytes, the writer emits the prefix followed by 12 bytes from the selected range. The captured length is 20 bytes, and the original length is 32 bytes. PCAPng alignment padding does not count toward either length. A zero limit means unlimited for PCAPng; classic PCAP keeps its existing limit semantics.

If your custom writer supports PCAPng, assign a separate interface description to each interface and link type pair. A PCAPng record refers to that description through an interface ID. Two link types on one interface therefore need separate IDs. Reset this mapping when you open a new file. Reject a record without a network interface, because the record needs an interface description.
