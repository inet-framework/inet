IEEE 802.11 Transmit Opportunity
================================

Goals
-----

Transmit opportunity (TXOP) is a MAC feature in 802.11, which increases throughput
for high priority data by providing contention-free channel access for a period of time.
This showcase demonstrates frame exchanges during a TXOP.

| INET version: ``4.2``
| Source files location: `inet/showcases/wireless/txop <https://github.com/inet-framework/inet/tree/master/showcases/wireless/txop>`__

About TXOP
----------

TXOP is available in QoS mode as part of EDCA (Enhanced Distributed Channel Access),
and it is a limited time period of contention-free channel access available to the
channel-owning station. During such a period the station can send multiple frames
that belong to a particular access category.

The benefit of TXOP is that it increases throughput and reduces delay of QoS data
frames via eliminating contention periods between transmissions. TXOP can be used
in combination with aggregation and block acknowledgement to further increase throughput.

More precisely, access categories have different channel access parameters,
such as AIFS (Arbitration Interframe Spacing), duration, contention window size,
and TXOP limit. In the default EDCA OFDM parameter
set in the 802.11 standard, these values are set so that higher priority packets are
favored (the MAC waits less before sending them, the contention window is smaller,
and they can be sent in a TXOP). The default parameter set specifies a TXOP limit
of approximately 3 ms for the video category, and 1.5 ms for the voice category.
The background and best effort categories have a TXOP limit of 0, that is, they
do not use TXOP (they can send only one MSDU before having to contend for
channel access again).

A station can send frames to multiple recipients during a TXOP. In addition to
QoS data frames, other frames can be exchanged in the course of the TXOP, such
as ACK and BlockAckReq/BlockAck frames, and other control and management frames.

TXOP in INET
~~~~~~~~~~~~

In INET, the TXOP is enabled automatically when the station is in QoS mode, i.e.
uses HCF (``qosStation = true`` in the MAC). The TXOP limit for an access
category can be set by the :par:`txopLimit` parameter in the :ned:`TxopProcedure`
module of that access category. The module is located at ``hcf.edca.edcaf[*].txopProcedure``
in the module hierarchy; there are four ``edcaf`` (EDCA Function) modules for the
four access categories:

.. figure:: media/edca.png
   :width: 80%
   :align: center

The default value for the :par:`txopLimit` parameter is ``-1`` for all ACs,
meaning the 802.11 standard default values are used:

+--------------+----------------+
| **AC**       | **TXOP limit** |
+--------------+----------------+
| background   | 0              |
+--------------+----------------+
| best effort  | 0              |
+--------------+----------------+
| video        | 3.008 ms       |
+--------------+----------------+
| voice        | 1.504 ms       |
+--------------+----------------+

The Model
---------

Our goal is to demonstrate complex frame exchange sequences during the TXOP,
such as the transmission of QoS data frames, aggregate frames, RTS and CTS frames,
acks, and block acks.

In the example simulation, one host is configured to send video priority UDP packets
to the other. The host sends 1200B, 3400B and 3500B packets. The RTS, aggregation
and block ack thresholds are configured appropriately so that we get the following frame exchanges
for demonstration:

- The 1200B packets are block acked
- Two of the 1200B packets can be aggregated; the aggregate frames are block acked
- The 3500B packets are sent after an RTS/CTS exchange, the 3400B ones are not, both are normal acked

Thus, the list of all alternative frame sequences that can repeat during the TXOP is the following:

- 1200B
- (1200B + 1200B) aggregated
- 3400B + ACK
- RTS + CTS + 3500B + ACK
- BlockAckReq + BlockAck
- ADDBA Request + ACK
- ADDBA Response + ACK

.. note::

   In INET, frame exchange sequences are described by `frame sequence` (``*Fs``) classes.
   These classes describe the order of frames allowed in a frame exchange.
   For example, frame combinations possible in a TXOP are described by the
   :cpp:`TxOpFs` class. Other classes include :cpp:`RtsCtsFs`, :cpp:`DataFs`,
   :cpp:`AckFs`, etc.

The simulation uses the following network:

.. figure:: media/network.png
   :width: 80%
   :align: center

It contains two :ned:`AdhocHost` modules, an :ned:`Ipv4NetworkConfigurator`,
an :ned:`Ieee80211ScalarRadioMedium` and an :ned:`IntegratedVisualizer` module.

The simulation is defined in the ``General`` configuration in :download:`omnetpp.ini <../omnetpp.ini>`.
There are two UDP applications in ``host1``, sending small (1200B) and large
(randomly 3400B or 3500B) packets to ``host2``:

.. literalinclude:: ../omnetpp.ini
   :start-at: host1.numApps
   :end-at: localPort = 2000
   :language: ini

QoS is enabled in both hosts, and a classifier is included. The port numbers in the
classifier are configured to put all UDP packets in the voice access category:

.. literalinclude:: ../omnetpp.ini
   :start-at: qosStation
   :end-at: udpPortUpMap
   :language: ini

IP and 802.11 fragmentation is turned off:

.. literalinclude:: ../omnetpp.ini
   :start-at: mtu
   :end-at: fragmentationThreshold
   :language: ini

Block acks are enabled:

.. literalinclude:: ../omnetpp.ini
   :start-at: isBlockAckSupported
   :end-at: isBlockAckSupported
   :language: ini

The aggregation, RTS, and block ack thresholds are also set appropriately to produce the desired frame sequences:

.. literalinclude:: ../omnetpp.ini
   :start-at: maxAMsduSize
   :end-at: maxBlockAckPolicyFrameLength
   :language: ini

Results
-------

The following image shows frame exchanges during a TXOP, displayed on a linear-scale
sequence chart:

.. figure:: media/elog2.png
   :align: center
   :width: 100%

The gap between the frames is a SIFS;
the TXOP frame exchange is preceded and followed by a much longer contention period.
This frame exchange sequence was recorded with 24 Mbps PHY rate. When using higher rates,
more frames could fit in a TXOP, as the TXOP duration is independent of the PHY rate.

This frame sequence is just an example. Various combinations of frames can be sent during
a TXOP. Observe the simulation for other combinations.

.. note:: The TXOP in the image is about 1.8 ms, which is longer than the 1.5 ms TXOP limit.
          This is due to a limitation in INET's implementation: it is not checked whether
          the last frame fits in the TXOP limit, only that its transmission can be started
          within the limit.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`TxopShowcase.ned <../TxopShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/45>`__ page in the GitHub issue tracker for commenting on this showcase.
