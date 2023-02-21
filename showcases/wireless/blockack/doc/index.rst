IEEE 802.11 Block Acknowledgment
================================

Goals
-----

IEEE 802.11 block acknowledgment is a feature of the Media Access Control (MAC)
layer of the 802.11 protocol that is designed to increase throughput by reducing
protocol overhead. It works by allowing a single block acknowledgment frame to
acknowledge the reception of multiple packets, instead of individually
acknowledging each packet with a separate acknowledgement (ACK) frame. This
reduces the number of ACK frames and corresponding interframe spaces, resulting
in increased throughput.

In this showcase, we will demonstrate the block acknowledgment feature in INET's
802.11 model. By the end of this showcase, you will understand how to enable and configure block
acknowledgment in INET to improve the performance of
wireless networks that use the 802.11 protocol.

| INET version: ``4.1``
| Source files location: `inet/showcases/wireless/blockack <https://github.com/inet-framework/inet/tree/master/showcases/wireless/blockack>`__

About Block Acknowledgment
--------------------------

In 802.11, block acknowledgment frames (block acks or BAs) are used within block ack sessions.
To establish a block ack session, one of the participants, the originator, sends an
`ADDBA request` frame. The other participant, the recipient, replies with an `ADDBA
response` frame. This exchange establishes the block ack session. The frames contain
information about the capabilities of each participant, such as buffer size,
or whether aggregation is supported. Each ADDBA frame is acked. The block ack session
is for one direction only, i.e. a separate block ack session must be established for
the reverse direction. Block ack sessions are terminated by the originator sending
a `DELBA` frame.

The originator sends multiple data frames, then sends a `block ack request` frame.
The recipient replies with a `block ack` frame, acknowledging the correctly received
frames from the previous block. The block ack request frame and block ack frame are
acked if *delayed block ack* is used, and not acked if *immediate block ack* is used.
The frames or fragments of frames from the previous block which are not acked are
retransmitted in the next block by the originator.

There are several kinds of block acks, such as `normal`, `compressed`, and `multi-tid`.
INET currently only supports `normal block ack`. Normal block acks contain a bitmap,
which contains up to 64 entries, each entry being a 16-bit array. The bitmap can acknowledge
up to 64 MSDUs. Each MSDU can be fragmented to at most 16 fragments. The 16 bits in a bitmap
entry are used to acknowledge the individual fragments of the MSDU.

Block Acknowledgment in INET
----------------------------

In INET, the block ack mechanism can be enabled by setting the :par:`isBlockAckSupported`
parameter of the MAC's :ned:`Hcf` submodule to ``true`` (it is disabled by default).

Several submodules of :ned:`Hcf` are involved in the block ack mechanism.
These modules have parameters controlling the block ack behavior. The following paragraphs
list some of the main parameters.

The details of the block ack agreement, which is negotiated at the beginning of
the block ack session, are specified by `block ack agreement policy` modules.
There are two module types, one for the originator and one for the recipient direction.
They have the same set of parameters. The parameters of :ned:`OriginatorBlockAckAgreementPolicy`
(at ``mac.hcf.originatorBlockAckAgreementPolicy``) and :ned:`RecipientBlockAckAgreementPolicy`
(at ``mac.hcf.recipientBlockAckAgreementPolicy``) are the following:

- :par:`aMsduSupported`: If ``true``, aMSDUs are block acked; otherwise they are normal acked (default).
- :par:`maximumAllowedBufferSize`: Sets the buffer size, i.e. how many MSDUs can be acked with a block ack, 64 by default.

`Ack policy modules` control how packets are acked. There are two module types,
one for each direction (originator and recipient), but settings are configured in
the originator module. Parameters of :ned:`OriginatorQosAckPolicy` (at ``mac.hcf.originatorAckPolicy``) are the following:

- :par:`blockAckReqThreshold`: The originator sends a block ack request after sending this many packets
- :par:`maxBlockAckPolicyFrameLength`: Only packets below this length are block acked (others are normal acked)

Currently, only immediate block ack is implemented. Also, block ack sessions cannot be enabled
in one direction and disabled in the other.

Experiments
-----------

The Model
~~~~~~~~~

The example simulation uses a network that contains two :ned:`AdhocHost` modules,
an :ned:`Ipv4NetworkConfigurator`, an :ned:`Ieee80211ScalarRadioMedium` and
an :ned:`IntegratedVisualizer` module. During simulation, ``host1`` will send
UDP packets to ``host2``.

.. figure:: media/network.png
   :width: 70%
   :align: center

We'll use three configurations to demonstrate various aspects of the block ack mechanism.
The configurations in :download:`omnetpp.ini <../omnetpp.ini>` are the following:

- ``NoFragmentation``: Packets are sent without fragmentation or aggregation. Block ack requests are sent after five packets (default).
- ``Fragmentation``: Packets are fragmented to 16 pieces, block ack requests are sent after 16 fragments.
- ``MixedTraffic``: Two traffic sources in ``host1`` generate packets below and above the :par:`maxBlockAckPolicyFrameLength` threshold. Packets with size below the threshold are block acked; those above aren't.

.. TODO when implemented - ``OneWayBlockAck``: Both ``host1`` and ``host2`` send packets to the other. A one-way block ack session is established, i.e. only packets going in one direction are block acked.

Parameter settings common to the three configurations are defined in the ``General`` configuration
in :download:`omnetpp.ini <../omnetpp.ini>`. The rest of this section explains the most important
settings.

``host1`` is configured to send 700B UDP packets to ``host2`` with about 10Mbps:

.. literalinclude:: ../omnetpp.ini
   :start-at: host1.numApps
   :end-at: localPort
   :language: ini

802.11 QoS is enabled in both hosts, and a classifier is included:

.. literalinclude:: ../omnetpp.ini
   :start-at: qosStation
   :end-at: classifier
   :language: ini

Block ack support is enabled in both hosts:

.. literalinclude:: ../omnetpp.ini
   :start-at: isBlockAckSupported
   :end-at: isBlockAckSupported
   :language: ini

The maximum block ack frame length is left on default, 1000B.

Fragmentation and aggregation is disabled by raising the thresholds:

.. literalinclude:: ../omnetpp.ini
   :start-at: fragmentationThreshold
   :end-at: aggregationLengthThreshold
   :language: ini

The transmitter power is lowered to make the communication more noisy, in order to
get some lost packets:

.. literalinclude:: ../omnetpp.ini
   :start-at: transmitter.power
   :end-at: transmitter.power
   :language: ini

In the next sections, we'll look at the three configurations and their results.

No Fragmentation
~~~~~~~~~~~~~~~~

In this simulation, packets are sent unfragmented, and ``host1`` sends a block ack request
after sending five frames (the default). The configuration in :download:`omnetpp.ini <../omnetpp.ini>` (``NoFragmentation``)
is empty:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config NoFragmentation
   :end-before: Config Fragmentation
   :language: ini

Here is the starting frame sequence at the beginning of the simulation displayed in
Qtenv's packet traffic view:

.. figure:: media/startingframesequence.png
   :width: 100%
   :align: center

First, ``host1`` sends an UDP packet, then an ADDBA request frame. ``host2`` replies
with an ADDBA response frame (both frames are acked). This frame exchange establishes
the block ack session, and ``host2`` doesn't normal ack the frames from this point forward
but waits for a block ack request.

Here are the ADDBA request and ADDBA response frames displayed in the packet traffic view:

.. figure:: media/addba.png
   :width: 100%
   :align: center

It indicates that aggregate MSDUs (aMSDUs) are supported, and the buffer size in both hosts is 64.

After sending five UDP packets, ``host1`` sends a block ack request, ``host2`` replies
with a block ack. The block ack frame acks the five previous frames. Here is the block ack
request frame displayed in Qtenv's packet inspector:

.. figure:: media/blockackreq.png
   :width: 90%
   :align: center

The block ack request contains the starting sequence number, which indicates the
first packet to be acked. Here is the block ack response frame:

.. figure:: media/blockackresp.png
   :width: 80%
   :align: center

It contains the starting sequence number as well, and the bitmap which specifies
which packets were received correctly. Here is the bitmap:

.. figure:: media/bitmap.png
   :width: 80%
   :align: center

The first five entries are used. It acks the five packets, starting from sequence number 1.

.. note:: When there is no fragmentation, only the first bit of the 16-bit entry is used to ack the frame. Here, the first three entries are all ones because the MAC already passed those packets to the higher layers, and has no information about the number of fragments, but still it indicates that the packets were successfully received. (This is an implementation detail.)

It is indicated in the block ack when some of the frames in a block were not received correctly.
The MAC retransmits those in the next block. Here is a retransmission in the packet traffic view:

.. figure:: media/retransmission.png
   :width: 100%
   :align: center

First, ``host1`` sends five packets, ``Data-16`` to ``Data-20``. Two of the frames, ``Data-17``
and ``Data-18`` are not received correctly, indicated in the block ack's bitmap (starting
sequence number is 16, corresponding to ``Data-16``):

.. figure:: media/retxblockack2.png
   :width: 80%
   :align: center

The next block starts with ``host1`` retransmitting these two frames, then transmitting
three new ones.

Fragmentation
~~~~~~~~~~~~~

In this simulation, 1080B packets are fragmented to 16 pieces, each 100B. Block ack requests
are sent after 16 frames. It is defined in the ``Fragmentation`` configuration in :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Fragmentation
   :end-before: Config MixedTraffic
   :language: ini

Here is how the simulation starts displayed in the packet traffic view:

.. figure:: media/frag_packetview_.png
   :width: 100%
   :align: center

After sending a fragment, the block ack session is negotiated. After that, ``host1`` transmits
the remaining fragments of ``Data-0``, which are normal acked. However, the fragments of the
next packet, ``Data-1``, are block acked:

.. figure:: media/frag_blockack_messageview_.png
   :width: 100%
   :align: center

In the next block, two fragments of ``Data-1`` are retransmitted, then several fragments of ``Data-2``
are sent:

.. figure:: media/frag_retx_messageview.png
   :width: 100%
   :align: center

Here is the block ack bitmap for this block:

.. figure:: media/frag_retx_ba.png
   :width: 90%
   :align: center

The first entry of the bitmap is all ones, indicating that ``Data-1`` was completely received
(all fragments), even though only two fragments of it were sent in this block. The second
entry has three zeros. The first zero indicates a lost fragment of ``Data-2``; the last two correspond to fragments that haven't been sent yet.

Mixed Traffic
~~~~~~~~~~~~~

In this configuration, there are two UDP apps in ``host1``, sending 1700B and 700B packets.
The :par:`maxBlockAckPolicyFrameLength` parameter is set to 1000B (default), so the large
packets are individually acked, while the smaller ones are block acked. The simulation is
defined in the ``MixedTraffic`` configuration in :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config MixedTraffic
   :end-at: localPort
   :language: ini

The smaller and larger packets are created at the same rate, so ``host1`` sends them alternately.
However, only the smaller ones are considered when sending block ack requests (sent after five
of the 700B packets). The larger packets are normal acked immediately:

.. figure:: media/mixed_messageview.png
   :width: 100%
   :align: center

Here, the block starts with ``Data-6``. ``Data-6`` to ``Data-10``, and also some large packets, are sent.
``Large-8`` and ``Large-9``, for example, are sent twice because they weren't correctly received
for the first time (there was no ack). The 802.11 MAC sequence numbers are contiguous between
all the packets sent by the MAC, so the block ack bitmap contains (the already acked) large
packets as well:

.. figure:: media/mixed_blockack.png
   :width: 100%
   :align: center

Also, the block ack is lost, so the not-yet-acked packets in the previous block (``Data-6``
to ``Data-10``) are retransmitted.

.. note:: Instead of retransmitting the data frames, the originator could send the block ack request again if it didn't receive a block ack. This is currently a limitation of the implementation.

.. Block ack in only one direction
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   by default, if you want to use block ack between two nodes, the block ack needs to be enabled in both nodes. so they are gonna use block acks both ways...how to do asymmetrically

   So far, we enabled block acks in both hosts. Host2 didnt send any data, but if it did, there would be a block ack agreement in the host2->host1 direction as well.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`BlockAckShowcase.ned <../BlockAckShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/43>`__ page in the GitHub issue tracker for commenting on this showcase.
