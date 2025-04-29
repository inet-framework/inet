Step 5. Adding Interpacket Gap
===========================

Goals
-----

In the fifth step, we enhance our network by adding an interpacket gap (IPG)
mechanism. In real networks, especially in protocols like Ethernet, a minimum
gap is required between consecutive packet transmissions to allow receivers to
process each packet. We want to model how this gap affects the timing and
throughput of packet transmissions.

The Model
---------

In this step, we'll use the model depicted in Network5.ned:

.. figure:: media/step5.png
   :width: 80%
   :align: center

The network still contains a client and a server, but now we've added an
interpacket gap inserter to the client.

The client host now contains an ``InterpacketGapInserter`` in addition to the
previous modules:

.. literalinclude:: ../Network5.ned
   :language: ned
   :start-at: module ClientHost5
   :end-at: }
   :emphasize-lines: 15-17

The ``InterpacketGapInserter`` enforces a minimum time gap between consecutive
packet transmissions. It delays packet transmission to ensure that the
configured gap duration is maintained between packets.

The server host remains the same as in Step 4, with a ``PacketReceiver``,
``EthernetFcsHeaderChecker``, and ``PassivePacketSink``:

.. literalinclude:: ../Network5.ned
   :language: ned
   :start-at: module ServerHost5
   :end-at: }

The connection between the client and server is the same as in Step 4, with a
data rate of 100Mbps, a delay of 1us, and a bit error rate of 1E-5:

.. literalinclude:: ../Network5.ned
   :language: ned
   :start-at: network Network5
   :end-at: }

The interpacket gap duration is configured to be 1ms:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Base3]
   :end-at: duration

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates
packets as before, which are queued, processed, and then passed through the
``EthernetFcsHeaderInserter``. Now, before transmission, the packets also pass
through the ``InterpacketGapInserter``, which ensures that a minimum gap of 1ms
is maintained between consecutive packet transmissions.

The key difference from Step 4 is that packet transmissions are now spaced out
in time, with a minimum gap of 1ms between them. This introduces the concept of
interpacket gap, which is an important aspect of many real-world network
protocols.

For example, in Ethernet, the interpacket gap (also known as the interframe
spacing) is 96 bit times, which at 100Mbps is 0.96 microseconds. Our model uses
a much larger gap of 1ms for demonstration purposes.

The interpacket gap affects the maximum achievable throughput of the network.
With a gap of 1ms and a packet transmission time of about 80 microseconds (for
1000-byte packets at 100Mbps), the maximum number of packets that can be
transmitted per second is approximately 1 / (0.08ms + 1ms) ≈ 926 packets per
second. This corresponds to a maximum throughput of about 926 * 1000 bytes ≈
926 KB/s or 7.4 Mbps, which is much lower than the link capacity of 100Mbps.

The addition of the interpacket gap in this step brings our model closer to
real network protocols, which often require spacing between consecutive packet
transmissions. In subsequent steps, we'll add more protocol elements to handle
other aspects of real-world network protocols, such as reliability mechanisms
to recover from packet loss and packet ordering.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network5.ned <../Network5.ned>`
