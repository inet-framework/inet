Queueing Classifiers
~~~~~~~~~~~~~~~~~~~~

``PacketClassifierBase::classifyPacket()`` is a query. The base class calls it from
``canPushPacket()``, ``canPullPacket()`` and ``handleCanPullPacketChanged()``, and again from the
operation that moves the packet. So it must not change the state of the module. Custom classifiers
must use this signature:

.. code-block:: c++

   int classifyPacket(Packet *packet) const override;

Move each state change of ``classifyPacket()`` into the new hook. The base class calls it one time
for each packet that goes through the module, and one time for each packet stream, before the
packet leaves the module:

.. code-block:: c++

   void handlePacketClassified(Packet *packet, int index) override;

For example, a classifier with a token bucket compares the token count in ``classifyPacket()``, and
it removes the tokens in ``handlePacketClassified()``. A classifier that overrides ``pushPacket()``
or ``pullPacket()`` must call ``handlePacketClassified()`` itself.
