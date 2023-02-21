Frame Preemption
================

Goals
-----

Ethernet frame preemption is a feature specified in the 802.1Qbu standard that
allows higher priority frames to interrupt the transmission of lower priority
frames at the Media Access Control (MAC) layer of an Ethernet network. This can
be useful for time-critical applications that require low latency for
high-priority frames. For example, in a Time-Sensitive Networking (TSN)
application, high-priority frames may contain time-critical data that must be
delivered with minimal delay. Frame preemption can help ensure that these
high-priority frames are given priority over lower priority frames, reducing the
latency of their transmission.

In this showcase, we will demonstrate Ethernet frame preemption and examine the
latency reduction that it can provide. By the end of this showcase, you will
understand how frame preemption works and how it can be used to improve the
performance of time-critical applications in an Ethernet network.

| INET version: ``4.3``
| Source files location: `inet/showcases/tsn/framepreemption <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framepreemption>`__

The Model
---------

Overview
~~~~~~~~

In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. 
When a high-priority frame becomes available for transmission during the transmission of a low priority frame, 
the Ethernet MAC can interrupt the transmission of the low priority frame, and start sending the 
high-priority frame immediately. When the high-priority frame finishes, the MAC can continue 
transmission of the low priority frame from where it left off, eventually sending the low priority 
frame in two (or more) fragments. 

Preemption is a feature of INET's composable Ethernet model. It uses INET's packet streaming API, 
so that packet transmission is represented as an interruptable stream. Preemption requires the 
:ned:`LayeredEthernetInterface`, which contains a MAC and a PHY layer, displayed below:

.. figure:: media/LayeredEthernetInterface2.png
   :align: center

To enable preemption, the default submodules :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` 
need to be replaced with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`.

The :ned:`EthernetPreemptingMacLayer` contains two submodules which themselves represent Ethernet 
MAC layers, a preemptable (:ned:`EthernetFragmentingMacLayer`) and an express MAC layer 
(:ned:`EthernetStreamingMacLayer`), each with its own queue for frames:

.. figure:: media/mac.png
   :align: center

The :ned:`EthernetPreemptingMacLayer` uses intra-node packet streaming. Discrete packets 
enter the MAC module from the higher layers, but leave the sub-MAC-layers (express and preemptable) 
as packet streams. Packets exit the MAC layer as a stream, and are represented as such through 
the PHY layer and the link.

In the case of preemption, packets initially stream from the preemptable sub-MAC-layer. 
The ``scheduler`` notifies the ``preemptingServer`` when a packet arrives at the express MAC. 
The ``preemptingServer`` stops the preemptable stream, sends the express stream in full, 
and then eventually it resumes the preemptable stream.

Interframe gaps are inserted by the PHY layer. 

.. **TODO** `somewhere else?` Note that only one frame can be fragmented by preemption at any given moment.

The :ned:`EthernetPreemptingPhyLayer` supports both packet streaming and fragmenting 
(sending packets in multiple fragments).

Configuration
~~~~~~~~~~~~~

The simulation uses the following network:

.. figure:: media/network.png
   :align: center

It contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet, and also a :ned:`PcapRecorder` 
to record PCAP traces; ``host1`` periodically generates packets for ``host2``.

Primarily, we want to compare the end-to-end delay, so we run simulations with the same packet length 
for the low and high-priority traffic in the following three configurations in omnetpp.ini: 

- ``FifoQueueing``: The baseline configuration; doesn't use priority queue or preemption.
- ``PriorityQueueing``: Uses priority queue in the Ethernet MAC to lower the delay of high-priority frames.
- ``FramePreemption``: Uses preemption for high-priority frames for a very low delay with a guaranteed upper bound.

Additionally, we demonstrate the use of priority queue and preemption with more realistic traffic: 
longer and more frequent low priority frames and shorter, less frequent high-priority frames. 
These simulations are the extension of the three configurations mentioned above, and are defined 
in the ini file as the configurations with the ``Realistic`` prefix.

In the ``General`` configuration, the hosts are configured to use the layered Ethernet model 
instead of the default, which must be disabled:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap.typename
   :end-at: LayeredEthernetInterface
   :language: ini

We also want to record a PCAP trace, so we can examine the traffic in Wireshark. We enable PCAP recording, 
and set the PCAP recorder to dump Ethernet PHY frames, because preemption is visible in the PHY header:

.. literalinclude:: ../omnetpp.ini
   :start-at: recordPcap
   :end-at: fcsMode
   :language: ini

Here is the configuration of traffic generation in ``host1``:

.. literalinclude:: ../omnetpp.ini
   :start-at: numApps
   :end-at: app[1].io.destPort
   :language: ini

There are two :ned:`UdpApp`'s in ``host1``, one is generating background traffic (low priority) 
and the other, high-priority traffic. The UDP apps put VLAN tags on the packets, and the Ethernet 
MAC uses the VLAN ID contained in the tags to classify the traffic into high and low priorities.

We set up a high-bitrate background traffic (96 Mbps) and a lower-bitrate high-priority traffic 
(9.6 Mbps); both with 1200B packets. Their sum is intentionally higher than the 100 Mbps link capacity 
(we want non-empty queues); excess packets will be dropped.

.. literalinclude:: ../omnetpp.ini
   :start-at: app[0].source.packetLength
   :end-at: app[1].source.productionInterval
   :language: ini

The ``FifoQueueing`` configuration uses no preemption or priority queue, the configuration just limits 
the :ned:`EthernetMac`'s queue length to 4. 

In all three cases, the queues need to be short to decrease the queueing time's effect on the 
measured delay. However, if they are too short, they might be empty too often, which renders 
the priority queue useless (it cannot prioritize if it contains just one packet, for example). 
The queue length of 4 is an arbitrary choice. The queue type is set to :ned:`DropTailQueue` 
so that it can drop packets if the queue is full:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config FifoQueueing
   :end-before: Config
   :language: ini

In the ``PriorityQueueing`` configuration, we change the queue type in the Mac layer from the 
default :ned:`PacketQueue` to :ned:`PriorityQueue`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueueing
   :end-before: Config
   :language: ini

The priority queue utilizes two internal queues, for the two traffic categories. To limit 
the queueing time's effect on the measured end-to-end delay, we also limit the length of 
internal queues to 4. We also disable the shared buffer, and set the queue type to 
:ned:`DropTailQueue`. We use the priority queue's classifier to put packets into the 
two traffic categories.

In the ``FramePreemption`` configuration, we replace the :ned:`EthernetMacLayer` and 
:ned:`EthernetPhyLayer` modules default in :ned:`LayeredEthernetInterface` with 
:ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, 
which support preemption.

.. literalinclude:: ../omnetpp.ini
   :start-at: Config FramePreemption
   :end-at: DropTailQueue
   :language: ini

There is no priority queue in this configuration, the two MAC submodules both have their own queues.
We also limit the queue length to 4, and configure the queue type to be :ned:`DropTailQueue`.

.. note:: We could also have just one shared priority queue in the EthernetPreemptableMac module, 
but this is not covered here.

We use the following traffic for the ``RealisticFifoQueueing``, ``RealisticPriorityQueueing`` 
and ``RealisticFramePreemption`` configurations:

.. literalinclude:: ../omnetpp.ini
   :start-after: Config RealisticBase
   :end-before: Config RealisticFifoQueueing
   :language: ini

In this traffic configuration, high-priority packets are 100 times less frequent, 
and are 1/10th the size of low-priority packets.

Transmission on the Wire
~~~~~~~~~~~~~~~~~~~~~~~~

In order to make sense of how frame preemptions are represented in the OMNeT++ GUI 
(in Qtenv's animation and packet log, and in the Sequence Chart in the IDE), it is 
necessary to understand how packet transmissions are modeled in OMNeT++.

Traditionally, transmitting a frame on a link is represented in OMNeT++ by sending a "packet".  
The "packet" is a C++ object (i.e. data structure) which is of, or is subclassed from, the 
OMNeT++ class ``cPacket``. The sending time corresponds to the start of the transmission. 
The packet data structure contains the length of the frame in bytes, and also the (more 
or less abstracted) frame content. The end of the transmission is implicit: it is 
computed as *start time* + *duration*, where *duration* is either explicit, or derived 
from the frame size and the link bitrate. This approach in vanilla form is of course 
not suitable for Ethernet frame preemption, because it is not known in advance whether 
or not a frame transmission will be preempted, and at which point.

Instead, in OMNeT++ 6.0 the above approach was modified to accommodate new use cases. 
In the new approach, the original packet sending remains, but its interpretation changes slightly. 
It now represents a *prediction*: "this is a frame whose transmission will go through, unless 
we say otherwise". Namely, while the transmission is ongoing, it is possible to send 
*transmission updates*, which modifies the prediction about the remaining part of the transmission. 
A *transmission update* packet essentially says "ignore what I said previously about the total 
frame size/content and transmission time, here's how much time the remaining transmission 
is going to take according to the current state of affairs, and here's the updated frame length/content". 

A transmission update may truncate, shorten or extend a transmission (and the frame). 
For technical reasons, the transmission update packet carries the full frame size and 
content (not just the remaining part), but it must be crafted by the sender in a way 
that it is consistent with what has already been transmitted (it cannot alter the past). 
For example, truncation is done by indicating zero remaining time, and setting the frame 
content to what has been transmitted up to that point. An updated transmission may be 
further modified by subsequent transmission updates. The end of the transmission is 
still implicit (it finishes according to the last transmission update), but it is 
also possible to make the ending explicit by sending a zero-remaining-time transmission update 
at exactly the time the transmission would otherwise end. After the transmission's end time 
has passed, it is naturally not possible to send any more transmission updates for it 
(we cannot modify the past).

In light of the above, it is easy to see why a preempted Ethernet frame appears in e.g. Qtenv's 
packet log multiple times: the original transmission and the subsequent transmission update(s) 
are all packets.

- The first one is the original packet, which contains the full frame size/content and carries the prediction that the frame transmission will go through uninterrupted.
- The second one is sent at the time the decision is made inside the node that the frame is going to be preempted. At that time, the node computes the truncated frame and the remaining transmission time, taking into account that at least the current octet and FCS need to be transmitted, and there is a minimum frame size requirement as well. The packet represents the size/content of the truncated frame, including FCS.
- In the current implementation, the Ethernet model also sends an explicit end-transmission update, with zero remaining transmission duration and identical frame size/content as the previous one. This would not be strictly necessary, and may change in future INET releases.

The above packets are distinguished using name suffixes: ``:progress`` and ``:end`` are 
appended to the original packet name for transmission updates and for the explicit end-transmission, 
respectively. In addition, the packet itself is also renamed by adding ``-frag0``, ``-frag1``, 
etc. to its name, to make frame fragments distinguishable from each other. For example, 
a frame called ``background3`` may be followed by ``background3-frag0:progress`` and 
``background3-frag0:end``. After the intervening express frame has also completed transmission, 
``background3-frag1`` will follow (see video in the next section).



Results
-------

Frame Preemption Behavior
~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a video of the frame preemption behavior:

.. video:: media/preemption3.mp4
   :width: 100%
	 :align: center

The Ethernet MAC in ``host1`` starts transmitting ``background-3``. During the transmission, 
a high-priority frame (``ts-1``) arrives at the MAC. The MAC interrupts the transmission of  
``background-3``; in the animation, ``background-3`` is first displayed as a whole frame, 
then changes to ``background-3-frag0:progress`` when the high-priority frame is available. 
After transmitting the high-priority frame, the remaining fragment of ``background-3-frag1`` 
is transmitted.

The frame sequence is displayed in the Qtenv packet log:

.. figure:: media/packetlog5.png
   :align: center
   :width: 100%

As mentioned in the previous section, a preempted frame appears multiple times 
in the packet log, as updates to the frame are logged. At first, ``background-3`` 
is logged as an uninterrupted frame. When the high-priority frame becomes available, 
the frame name changes to ``background-3-frag0``, and it is logged separately. 
Actually, only  one frame named ``background-3-frag0`` was sent before ``ts-1``, 
but with three separate packet updates.

The same frame sequence is displayed on a sequence chart on the following images, 
with a different frame selected and highlighted in red on each image. Note that 
the timeline is non-linear:

.. figure:: media/seqchart4.png
   :align: center
   :width: 100%

Just as in the packet log, the sequence chart contains the originally intended, 
uninterrupted ``background-3`` frame as it is logged when its transmission is started. 

.. note:: You can think of it as there are actually two time dimensions present on the sequence chart: the events and messages as they happen at the moment, and what the modules "think" about the future, i.e. how long will a transmission take. In reality, the transmission might be interrupted and so both the original (``background-3``) and the "updated" (``background-3-frag0``) is present on the chart.

Here is the frame sequence on a linear timeline, with the ``background-3-frag0`` frame highlighted:

.. figure:: media/linear.png
   :align: center
   :width: 100%

Note that ``background-3-frag0:progess`` is very short (it basically contains 
just an updated packet with an FCS, as a remaining data part of the first fragment). 
Transmission of ``ts-1`` starts after a short interframe gap.

Here is the same frame sequence displayed in Wireshark:

.. figure:: media/wireshark.png
   :align: center
   :width: 100%

The frames are recorded in the PCAP file at the end of the transmission of 
each frame or fragment, so the originally intended 1243B ``background-3`` 
frame frame is not present there, only the two fragments.

In the Wireshark log, ``frame 5`` and ``frame 7`` are the two fragments of 
``background-3``. Note that FPP refers to `Frame Preemption Protocol`; 
``frame 6`` is ``ts-1``, sent between the two fragments.

Here is ``background-3-frag1`` displayed in Qtenv's packet inspector:

.. figure:: media/packetinspector5.png
   :align: center
   :width: 100%

This fragment does not contain a MAC header, because it is the second part of the original Ethernet frame.

.. **TODO** without highlight

The paths which the high and low priority (express and preemptable) packets take in the 
:ned:`EthernetPreemptingMacLayer` are illustrated below by the red lines:

.. figure:: media/preemptible2.png
   :align: center

.. figure:: media/express2.png
   :align: center

Analyzing End-to-end Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~

Simulation Results
++++++++++++++++++

To analyze the results for the identical packet length configurations, we plot the end-to-end delay 
averaged on [0,t] of the UDP packets for the three cases on the following chart. Note that the 
configuration is distinguished using different line styles and the traffic category by colors:

.. figure:: media/delay.png
   :align: center
   :width: 80%

The chart shows that in the case of the default configuration, the delay for the 
two traffic categories is about the same. The use of the priority queue significantly 
decreases the delay for the high priority frames, and marginally increases the 
delay of the background frames compared to the baseline default configuration. 
Preemption causes an even greater decrease for high priority frames at the cost 
of a slight increase for background frames.

Estimating the End-to-end delay
+++++++++++++++++++++++++++++++

In the next section, we will examine the credibility of these results by doing some 
back-of-the-envelope calculations.

FifoQueueing Configuration
**************************

For the ``FifoQueueing`` configuration, the MAC stores both background and high-priority 
packets in the same FIFO queue. Thus, the delay of the two traffic categories is 
about the same. Due to high traffic, the queue always contains packets. The queue 
is limited to 4 packets, so the queueing time has an upper bound: about the 
transmission time of 4 frames. Looking at the queue length statistics (see anf file), 
we can see that the average queue length is ~2.6, so packets suffer an average queueing 
delay of 2.6 frame transmission durations.

The end-to-end delay is rougly the transmission duration of a frame + queueing delay + interframe gap.
The transmission duration for a 1200B frame on 100Mbps Ethernet is about 0.1ms. 
On average, there are two frames in the queue so frames wait two frame transmission 
durations in the queue. The interframe gap for 100Mbps Ethernet is 0.96us, so we assume it negligable:

``delay ~= txDuration + 2.6 * txDuration + IFG = 3.6 * txDuration = 0.36ms``

PriorityQueueing Configuration
******************************

For the ``PriorityQueueing`` configuration, high-priority frames have their own sub-queue in 
the PriorityQueue module in the MAC. When a high-priority frame arrives at the queue, the 
MAC will finish the ongoing low-priority transmission (if there is any) before beginning 
the transmission of the high-priority frame.  Thus high-priority frames can be delayed, 
as the transmission of the current frame needs to be finished first. Still, using a priority 
queue decreases the delay of the high-priority frames and increases that of the background 
frames, compared to just using one queue for all frames.

Due to high background traffic, a frame is always present in the background queue. 
A high-priority frame needs to wait until the current background frame transmission 
finishes; on average, the remaining duration is half the transmission duration of a 
background frame:

``delay ~= txDuration + 0.5 * txDuration + IFG = 1.5 * txDuration = 0.15ms``

FramePreemption Configuration
*****************************

For the ``FramePreemption`` configuration, the high-priority frames have their own queue in the MAC.
When a high priority frame becomes available, the current background frame transmission 
is almost immediately stopped.

The delay is roughly the duration of an FCS + transmission duration + interframe gap. 
The duration of an FCS is about 1us, so we neglect it in the calculation (as previously, 
the interframe gap is neglected as well):

``delay = txDuration + fcsDuration + IFG ~= txDuration = 0.1ms``

The calculated values above roughly match the results of the simulation.

Realistic Traffic
+++++++++++++++++

The mean end-to-end delay for the realistic traffic case is plotted on the following chart:

.. figure:: media/realisticdelay.png
   :align: center
   :width: 80%

The range indicated by the rectangle on the chart above is shown zoomed in on the chart below, 
so that its more visible:

.. figure:: media/realisticdelay_zoomed.png
   :align: center
   :width: 80%

As described above, the end-to-end delay of high-priority frames when using preemption 
is independent of the length of background frames. The delay is approximately the 
transmission duration of a high-priority frame (apperent in the case of both the 
realistic and the comparable length traffic results). 

In case of realistic traffic, the delay of the background frames is not affected by 
either the use of a priority queue or preemption. The delay of the high-priority 
frames is reduced significantly, because the traffic is different (originally, 
both the background and high-priority packets had the same length, so they could 
be compared for better demonstration).

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`FramePreemptionShowcase.ned <../FramePreemptionShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/676>`__ page in the GitHub issue tracker for commenting on this showcase.