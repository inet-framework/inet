Queueing Filters
~~~~~~~~~~~~~~~~

``IPacketFilter::matchesPacket()`` is a query. ``PacketFilterBase`` calls it from
``canPushPacket()`` and ``canPullPacket()``, and again from the operation that moves the packet. So
it must not change the state of the module, and it must not draw a random number. Move each state
change into the new hook. The base class calls it one time for each packet that goes through the
filter or that the filter drops, and one time for each packet stream, when the module holds the
packet and before the packet leaves the module:

.. code-block:: c++

   void handlePacketFiltered(Packet *packet, bool matches) override;

``processPacket()`` changes each packet instance that passes, for example it removes a header, and
``dropPacket()`` disposes of each instance that the filter drops. In a packet stream the base class
calls them for each instance, so neither may change the state of the module. A filter that decides
with a random number draws the number in advance, and draws the next one in
``handlePacketFiltered()`` after a decision used it.
