Step 4. Adding Error Detection
==========================

Goals
-----

In the fourth step, we enhance our network by adding error detection
capabilities. In real networks, transmission errors can occur due to various
factors such as noise, interference, and signal attenuation. We want to model
how these errors are detected and handled in network protocols.

The Model
---------

In this step, we'll use the model depicted in Network4.ned:

.. figure:: media/step4.png
   :width: 80%
   :align: center

The network still contains a client and a server, but now we've added error
detection functionality using Ethernet Frame Check Sequence (FCS).

The client host now contains an ``EthernetFcsHeaderInserter`` in addition to the
previous modules:

.. literalinclude:: ../Network4.ned
   :language: ned
   :start-at: module ClientHost4
   :end-at: }
   :emphasize-lines: 12-14

The ``EthernetFcsHeaderInserter`` computes a checksum for each packet and
inserts it as a header. This checksum is used by the receiver to verify that
the packet was not corrupted during transmission.

The server host now contains an ``EthernetFcsHeaderChecker`` in addition to the
previous modules:

.. literalinclude:: ../Network4.ned
   :language: ned
   :start-at: module ServerHost4
   :end-at: }
   :emphasize-lines: 9-11

The ``EthernetFcsHeaderChecker`` verifies the checksum of each received packet.
If the checksum is correct, the packet is passed to the next module. If the
checksum is incorrect, indicating that the packet was corrupted during
transmission, the packet is discarded.

The connection between the client and server now includes a bit error rate
(BER) of 1E-5, which means that on average, 1 out of every 100,000 bits will be
flipped during transmission:

.. literalinclude:: ../Network4.ned
   :language: ned
   :start-at: network Network4
   :end-at: }
   :emphasize-lines: 12

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates
packets as before, which are queued, processed, and then passed to the
``EthernetFcsHeaderInserter``. The inserter computes a checksum for each packet
and adds it as a header before passing the packet to the ``PacketTransmitter``
for transmission.

During transmission, some bits may be flipped due to the configured bit error
rate. When a packet arrives at the server, the ``PacketReceiver`` passes it to
the ``EthernetFcsHeaderChecker``, which verifies the checksum. If the checksum
is correct, the packet is passed to the ``PassivePacketSink`` for consumption.
If the checksum is incorrect, the packet is discarded.

The key difference from Step 3 is that packets can now be corrupted during
transmission, and corrupted packets are detected and discarded. This introduces
the concept of packet loss due to transmission errors, which is a common
occurrence in real networks.

For example, with a bit error rate of 1E-5 and a packet size of 1000 bytes
(8000 bits), the probability of a packet being corrupted is approximately
1 - (1 - 1E-5)^8000 ≈ 0.077, or about 7.7%. This means that on average, about
7.7% of packets will be corrupted and discarded.

The addition of error detection in this step brings our model closer to real
network protocols, which must handle transmission errors. In subsequent steps,
we'll add more protocol elements to handle other aspects of real-world network
protocols, such as reliability mechanisms to recover from packet loss and
packet ordering.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network4.ned <../Network4.ned>`
