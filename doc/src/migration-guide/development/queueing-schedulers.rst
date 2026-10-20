Queueing Schedulers
~~~~~~~~~~~~~~~~~~~

``PacketSchedulerBase::schedulePacket()`` is a query. The base class calls it from
``canPushSomePacket()`` and ``handleCanPushPacketChanged()``, and again from the operation that
moves the packet. So it must not change the state of the module. Custom schedulers must use this
signature:

.. code-block:: c++

   int schedulePacket() const override;

Move each state change of ``schedulePacket()`` into the new hook. The base class calls it one time
for each packet that goes through the module, and one time for each packet stream, when the module
holds the packet and before the packet leaves the module:

.. code-block:: c++

   void handlePacketScheduled(Packet *packet, int index) override;

For example, a weighted round-robin scheduler selects the input in ``schedulePacket()``, and it
decrements the bucket of that input in ``handlePacketScheduled()``. The hook gets the packet, so a
scheduler that charges by length, such as deficit round-robin, can use its length. A scheduler that
overrides ``pushPacket()`` or ``pullPacket()`` must call ``handlePacketScheduled()`` itself.
