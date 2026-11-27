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

| Verified with INET version: ``4.6``
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
The recipient replies with a `block ack` frame, acknowledging the reception of the previous block's
frames. The block ack request frame and block ack frame are
acked if *delayed block ack* is used, and not acked if *immediate block ack* is used.
The frames or fragments of frames from the previous block that were not acked are
retransmitted in the next block by the originator.

This showcase selects the historical Basic Block Ack profile with ``mac.hcf.blockAckVariant = "basic"``. Basic uses 64 bitmap entries, each with 16 bits for individual fragments. The recipient can acknowledge up to 64 MSDUs, each with up to 16 fragments.

`IEEE Std 802.11e-2005 <https://standards.ieee.org/ieee/802.11e/3131/>`__ introduced this format in Clauses 7.2.1.7 and 7.2.1.8. `IEEE Std 802.11-2024 <https://standards.ieee.org/ieee/802.11/10548/>`__ reserves type 0 in Tables 9-36 and 9-37. Thus, IEEE Std 802.11-2024 does not support Basic. INET retains Basic for historical compatibility with existing studies. The showcase's PHY modes and INET response rules do not establish complete conformance to IEEE Std 802.11e-2005.

Block Acknowledgment in INET
----------------------------

In INET, the block ack mechanism can be enabled by setting the :par:`isBlockAckSupported`
parameter of the MAC's :ned:`Hcf` submodule to ``true`` (it is disabled by default).

This showcase selects ``blockAckVariant = "basic"`` on both endpoints to retain the historical profile. HCF defaults to Compressed for other configurations. Compressed agreements require usable local and peer HT state. Without that state, DATA uses Normal Ack. HCF rejects unknown selector values during initialization, also when Block Ack support is disabled.

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
- :par:`maximumAllowedBufferSize`: Sets the buffer size, i.e. how many MSDUs can be acked with a block ack, default is 64.

`Ack policy modules` control how packets are acked. There are two module types,
one for each direction (originator and recipient), but settings are configured in
the originator module. Parameters of :ned:`OriginatorQosAckPolicy` (at ``mac.hcf.originatorAckPolicy``) are the following:

- :par:`blockAckReqThreshold`: The originator sends a block ack request after sending this many packets (default is 5).
- :par:`maxBlockAckPolicyFrameLength`: Only packets below this length are block acked (others are normal acked, default is 1000).

Currently, only immediate block ack is implemented. Also, block ack sessions cannot be enabled
in one direction and disabled in the other.

Experiments
-----------

The Model
~~~~~~~~~

The example simulation uses a network that contains two :ned:`AdhocHost` modules,
an :ned:`Ipv4NetworkConfigurator`, an :ned:`Ieee80211RadioMedium` and
an :ned:`IntegratedCanvasVisualizer` module. During simulation, ``host1`` will send
UDP packets to ``host2``.

.. figure:: media/network.png
   :width: 70%
   :align: center

We'll use three configurations to demonstrate various aspects of the block ack mechanism.
The configurations in :download:`omnetpp.ini <../omnetpp.ini>` contain the following setups:

- ``NoFragmentation``: Packets are sent without fragmentation or aggregation. Block ack requests are sent after five packets (default).
- ``Fragmentation``: Packets are fragmented into 16 pieces, and block ack requests are sent after 16 fragments.
- ``MixedTraffic``: Two traffic sources in ``host1`` generate packets below and above the :par:`maxBlockAckPolicyFrameLength` threshold. Packets with a size below the threshold are block acked; those above are not.

.. TODO when implemented - ``OneWayBlockAck``: Both ``host1`` and ``host2`` send packets to the other. A one-way block ack session is established, i.e. only packets going in one direction are block acked.

The parameters common to the three configurations are defined in the ``General`` configuration
in :download:`omnetpp.ini <../omnetpp.ini>`. The rest of this section explains the most important
settings.

``host1`` is configured to send 700B UDP packets to ``host2`` at about 10Mbps:

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
   :end-at: blockAckVariant
   :language: ini

The maximum block ack frame length is left as default, 1000B.

Fragmentation and aggregation are disabled by raising the thresholds:

.. literalinclude:: ../omnetpp.ini
   :start-at: fragmentationThreshold
   :end-at: aggregationLengthThreshold
   :language: ini

Transmitter power is lowered to make the communication more noisy, in order to
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
has no specific settings:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config NoFragmentation
   :end-before: Config Fragmentation
   :language: ini

Here is the starting frame sequence at the beginning of the simulation displayed in
Qtenv's packet traffic view:

.. figure:: media/startingframesequence.png
   :width: 90%
   :align: center

First, ``host1`` sends a UDP packet, then an ADDBA request frame. ``host2`` replies
with an ADDBA response frame (both frames are acked). This frame exchange establishes
the block ack session, and ``host2`` doesn't normal ack the frames from this point forward
but waits for a block ack request.

Here are the ADDBA request and ADDBA response frames displayed in the packet traffic view:

.. figure:: media/addba.png
   :width: 100%
   :align: center

It indicates that aggregate MSDUs (aMSDUs) are supported, and the buffer size in both hosts is 64.

After sending five UDP packets, ``host1`` sends a block ack request, and ``host2`` replies
with a block ack. The block ack frame tells which of the five previous frames were received correctly. Here is the block ack
request frame displayed in Qtenv's packet inspector:

.. figure:: media/blockackreq.png
   :width: 80%
   :align: center

The block ack request contains the starting sequence number, which indicates the
first packet to be acked. Here is the block ack response frame:

.. figure:: media/blockackresp.png
   :width: 70%
   :align: center

The block ack response frame also contains the starting sequence number and the bitmap, which specifies
which packets were received correctly. Here is the bitmap:

.. figure:: media/bitmap.png
   :width: 70%
   :align: center

..
   FIGURE RECIPE
   config:   NoFragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv object inspector, sorted by name, on the BasicBlockAck WirelessSignal in the radio medium (run with **.radioMedium.mediumLimitCache.maxTransmissionDuration=3ms so only this one is kept), expanded to encapsulatedPacket > content > full content > Ieee80211BasicBlockAck > blockAckBitmap;
             stop at 0.00461 s (first BasicBlockAck on the air)
   anchor:   entries 1111111111111111, 1000..., 0000..., 1000..., 1000... from sequence 1; Data-3 lost. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

The first five entries are used, starting from sequence number 1 (``Data-1``). The third entry is all zeros:
``Data-3`` was not received correctly, so it is retransmitted in the next block. The other four packets are acknowledged.

.. note:: When there is no fragmentation, only the first bit of the 16-bit entry is used to ack the frame. Here, the first entry is all ones because the MAC has already passed that packet to the higher layers and has no information about the number of fragments, but it still indicates that the packet was successfully received. (This is an implementation detail.)

The block ack also indicates when some of the frames in a block were not received correctly.
The MAC retransmits those in the next block. Here is a retransmission in the packet traffic view:

.. figure:: media/retransmission.png
   :width: 100%
   :align: center

..
   FIGURE RECIPE
   config:   NoFragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv packet traffic view (log inspector, messages mode), log font Ubuntu Mono 11,
             no digit grouping, columns ID/Kind/Length + Source/Destination/Protocol/Type/Length
             with widths 54,120,135,144,152,152,135,102,62 px, fast run to the stop time,
             cropped to the header and the rows without the event number column;
             stop at 0.0316 s; rows Data-28 to the BasicBlockAck after Data-35
   anchor:   Data-29 and Data-30 lost and sent again before Data-33 to Data-35. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

First, ``host1`` sends five packets, ``Data-28`` to ``Data-32``. Two of the frames, ``Data-29``
and ``Data-30``, were not received correctly, as indicated in the block ack's bitmap (starting
sequence number is 28, corresponding to ``Data-28``):

.. figure:: media/retxblockack2.png
   :width: 70%
   :align: center

..
   FIGURE RECIPE
   config:   NoFragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv object inspector, sorted by name, on the BasicBlockAck WirelessSignal in the radio medium (run with **.radioMedium.mediumLimitCache.maxTransmissionDuration=3ms so only this one is kept), expanded to encapsulatedPacket > content > full content > Ieee80211BasicBlockAck > blockAckBitmap;
             stop at 0.02818 s
   anchor:   start sequence 28; entries 1000..., 0000..., 0000..., 1000..., 1000.... If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

The next block starts with ``host1`` retransmitting these two frames, and then transmitting
three new ones.

Fragmentation
~~~~~~~~~~~~~

In this simulation, 1080B packets are fragmented into 16 pieces, each 100B. Block ack requests
are sent after 16 frames. It is defined in the ``Fragmentation`` configuration in :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Fragmentation
   :end-before: Config MixedTraffic
   :language: ini

Here is how the simulation starts displayed in the packet traffic view:

.. figure:: media/frag_packetview_.png
   :width: 100%
   :align: center

..
   FIGURE RECIPE
   config:   Fragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv packet traffic view (log inspector, messages mode), log font Ubuntu Mono 11,
             no digit grouping, columns ID/Kind/Length + Source/Destination/Protocol/Type/Length
             with widths 54,120,135,144,152,152,135,102,62 px, fast run to the stop time,
             cropped to the header and the rows without the event number column;
             stop at 0.00275 s; rows from the start to the WlanAck of the AddbaResponse
   anchor:   all 16 fragments of Data-0 normally acked, then AddbaReq and AddbaResponse. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

First, ``host1`` sends all fragments of ``Data-0``, which are normally acked, and then the block ack
session is negotiated. The fragments of the next packet, ``Data-1``, are block acked:

.. figure:: media/frag_blockack_messageview_.png
   :width: 100%
   :align: center

..
   FIGURE RECIPE
   config:   Fragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv packet traffic view (log inspector, messages mode), log font Ubuntu Mono 11,
             no digit grouping, columns ID/Kind/Length + Source/Destination/Protocol/Type/Length
             with widths 54,120,135,144,152,152,135,102,62 px, fast run to the stop time,
             cropped to the header and the rows without the event number column;
             stop at 0.00995 s; rows Data-1-frag0 to the BasicBlockAck
   anchor:   Data-1-frag0 to frag15, BasicBlockAckReq, BasicBlockAck. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

Later, a block starts with retransmitting a lost fragment of ``Data-2``, and then fragments of ``Data-3``
are sent:

.. figure:: media/frag_retx_messageview.png
   :width: 100%
   :align: center

..
   FIGURE RECIPE
   config:   Fragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv packet traffic view (log inspector, messages mode), log font Ubuntu Mono 11,
             no digit grouping, columns ID/Kind/Length + Source/Destination/Protocol/Type/Length
             with widths 54,120,135,144,152,152,135,102,62 px, fast run to the stop time,
             cropped to the header and the rows without the event number column;
             stop at 0.0219 s; rows Data-2-frag12 (resent) to the BasicBlockAck
   anchor:   Data-2-frag12 resent, then Data-3-frag0 to frag14. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

Here is the block ack bitmap for this block:

.. figure:: media/frag_retx_ba.png
   :width: 90%
   :align: center

..
   FIGURE RECIPE
   config:   Fragmentation   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv object inspector, sorted by name, on the BasicBlockAck WirelessSignal in the radio medium (run with **.radioMedium.mediumLimitCache.maxTransmissionDuration=3ms so only this one is kept), expanded to encapsulatedPacket > content > full content > Ieee80211BasicBlockAck > blockAckBitmap;
             stop at 0.02189 s
   anchor:   entries 1111111111111111, 1111110111111110 (Data-3 frag6 lost, frag15 not sent). If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

The first entry of the bitmap is all ones, indicating that ``Data-2`` was completely received
(all fragments), even though only one fragment of it was sent in this block. The second
entry has two zeros. The first zero indicates a lost fragment of ``Data-3``, and the last one corresponds to a fragment that hasn't been sent yet.

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
of the 700B packets). The larger packets are normally acked immediately:

.. figure:: media/mixed_messageview.png
   :width: 100%
   :align: center

..
   FIGURE RECIPE
   config:   MixedTraffic   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv packet traffic view (log inspector, messages mode), log font Ubuntu Mono 11,
             no digit grouping, columns ID/Kind/Length + Source/Destination/Protocol/Type/Length
             with widths 54,120,135,144,152,152,135,102,62 px, fast run to the stop time,
             cropped to the header and the rows without the event number column;
             stop at 0.0265 s; rows Data-6 to the resent Data-10
   anchor:   Large-6 and Large-7 sent twice; Data-10 resent after the BasicBlockAck. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

Here, the block starts with ``Data-6``. ``Data-6`` to ``Data-10``, and also some large packets, are sent.
``Large-6`` and ``Large-7``, for example, are sent twice because they weren't correctly received
the first time (there was no ack). The 802.11 MAC sequence numbers are contiguous between
all the packets sent by the MAC, so the block ack bitmap contains the (already acked) large
packets as well:

.. figure:: media/mixed_blockack.png
   :width: 80%
   :align: center

..
   FIGURE RECIPE
   config:   MixedTraffic   # ../omnetpp.ini, Qtenv with --mcp-server-address
   capture:  Qtenv object inspector, sorted by name, on the BasicBlockAck WirelessSignal in the radio medium (run with **.radioMedium.mediumLimitCache.maxTransmissionDuration=3ms so only this one is kept), expanded to encapsulatedPacket > content > full content > Ieee80211BasicBlockAck > blockAckBitmap;
             stop at 0.02591 s
   anchor:   start sequence 12; entries [0]-[6] all ones, [7] and [8] zeros. If the run shows other frames, pick the example again
             and rewrite the text and this recipe.
   stamp:    captured 2026-11-27, INET master 22df607673

``Data-10`` was not received correctly, so it is retransmitted after the block ack. When the block ack
itself is lost, all the not-yet-acked packets of the block are retransmitted.

.. note:: Instead of retransmitting the data frames, the originator could send the block ack request again if it didn't receive a block ack. This is currently a limitation of the implementation.

.. Block ack in only one direction
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   by default, if you want to use block ack between two nodes, the block ack needs to be enabled in both nodes. so they are gonna use block acks both ways...how to do asymmetrically

   So far, we enabled block acks in both hosts. Host2 didnt send any data, but if it did, there would be a block ack agreement in the host2->host1 direction as well.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`BlockAckShowcase.ned <../BlockAckShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/wireless/blockack`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/wireless/blockack && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/43>`__ page in the GitHub issue tracker for commenting on this showcase.
