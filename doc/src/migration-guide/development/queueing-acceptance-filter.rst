Queueing Acceptance Filter
~~~~~~~~~~~~~~~~~~~~~~~~~~

``IPassivePacketSource`` has two new methods. The filter is the acceptance condition of the sink.
The source answers and pulls as if it held only the packets that the filter matches:

.. code-block:: c++

   Packet *canPullPacket(const cGate *gate, const IPacketFilterFunction *filter) const override;
   Packet *pullPacket(const cGate *gate, const IPacketFilterFunction *filter) override;

A class that extends a base class of the queueing model, such as ``PacketFlowBase`` or
``PacketQueueBase``, gets both methods, and they throw an error. A class that implements
``IPassivePacketSource`` directly must declare them. A class that holds one packet at most returns
that packet if the filter matches it, and ``nullptr`` otherwise. A class that cannot choose among
the packets that the filter matches throws an error. If a class supports the filter and adds an
effect in ``pullPacket()``, the filtered ``pullPacket()`` must add the same effect.

A scheduler that extends ``PacketSchedulerBase`` supports the filter when it overrides this
overload of ``schedulePacket()``. It returns the input that ``schedulePacket()`` would return if
each input held only the packets that the filter matches. The base class then pulls from that input
with the filter, and it calls ``handlePacketScheduled()`` as for a normal pull:

.. code-block:: c++

   int schedulePacket(const IPacketFilterFunction *filter) const override;

``IPacketQueue`` has the new method ``dequeuePacket(filter)``. ``PacketQueueBase`` implements it as
``pullPacket(outputGate, filter)`` and ``drop()``, as it implements ``dequeuePacket()``. A class that
implements ``IPacketQueue`` directly must declare it:

.. code-block:: c++

   Packet *dequeuePacket(const IPacketFilterFunction *filter) override;

A class that declares only one of the overloads hides the other one in its own scope. Add
``using IPassivePacketSource::pullPacket;`` (and the same for ``canPullPacket``) if the class calls
the other overload without a qualifier.
