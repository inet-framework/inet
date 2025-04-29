Step 3. Adding Packet Queue and Server
=================================

Goals
-----

In the third step, we enhance our network by adding packet queueing and
processing capabilities. We want to model how packets are buffered when they
are generated faster than they can be transmitted, and how they are processed
before transmission.

The Model
---------

In this step, we'll use the model depicted in Network3.ned:

.. figure:: media/step3.png
   :width: 80%
   :align: center

The network still contains a client and a server, but now we've added queueing
and processing functionality to the client.

The client host now contains a ``PacketQueue`` and an ``InstantServer`` in
addition to the ``ActivePacketSource`` and ``PacketTransmitter``:

.. literalinclude:: ../Network3.ned
   :language: ned
   :start-at: module ClientHost3
   :end-at: }
   :emphasize-lines: 9-15

The ``PacketQueue`` is responsible for buffering packets when they are generated
faster than they can be processed and transmitted. It stores packets in a FIFO
(First In, First Out) manner until they can be processed by the server.

The ``InstantServer`` processes packets from the queue. In this case, it
processes them instantly without any delay, but in a real network, this could
represent protocol processing that takes time.

The server host remains the same as in Step 2, with a ``PacketReceiver`` and a
``PassivePacketSink``:

.. literalinclude:: ../Network3.ned
   :language: ned
   :start-at: module ServerHost3
   :end-at: }

The connection between the client and server is the same as in Step 2, with a
data rate of 100Mbps and a delay of 1us:

.. literalinclude:: ../Network3.ned
   :language: ned
   :start-at: network Network3
   :end-at: }

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates
packets at random intervals as before. However, now these packets are first
placed in the ``PacketQueue`` before being processed by the ``InstantServer``
and then transmitted by the ``PacketTransmitter``.

The key difference from Step 2 is that packets are now buffered in a queue
before transmission. This introduces the concept of queueing delay, which
occurs when packets are generated faster than they can be transmitted.

For example, if the ``ActivePacketSource`` generates packets at an average rate
of 100 packets per second, and each packet takes 80 microseconds to transmit
(as calculated in Step 2), then the average transmission rate is about 12,500
packets per second. Since the generation rate is lower than the transmission
rate, the queue will usually be empty, and packets will experience minimal
queueing delay.

However, due to the random nature of packet generation (with exponential
distribution), there will be times when packets are generated in quick
succession, causing them to be temporarily buffered in the queue. This models
the bursty nature of real network traffic.

The addition of queueing and processing elements in this step brings our model
closer to real network protocols, which must handle varying traffic loads and
process packets before transmission. In subsequent steps, we'll add more
protocol elements to handle other aspects of real-world network protocols, such
as error detection, reliability mechanisms, and packet ordering.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network3.ned <../Network3.ned>`
