Time-Aware Shaping
==================

Goals
-----

Time-aware shaping is a feature specified in the IEEE 802.1Qbv standard that
allows for the scheduling of the transmission of time-critical and lower
priority frames in time-triggered windows. This helps to guarantee bounded
latency for time-critical frames, which is important for applications that
require low and predictable latency in Time-Sensitive Networking (TSN).
Time-aware shaping works by dividing time into fixed intervals, or windows, and
scheduling the transmission of frames within these windows based on their
priority. Higher priority frames are given priority by transmitting them in a
separate window, while lower priority frames are transmitted in the remaining time.

In this showcase, we will demonstrate how to use time-aware traffic shaping to
improve the performance of time-critical applications in an Ethernet network. By
the end of this showcase, you will understand how time-aware shaping works and
how it can be used to guarantee bounded latency for time-critical frames.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/timeawareshaper <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/timeawareshaper>`__

The Model
---------

Overview
~~~~~~~~

Time-aware shaping (TAS), and its implementation, the Time-aware shaper, utilizes the Time-division multiple access (TDMA) scheme to send different priority traffic streams
in their own allotted transmission time windows. This makes the delay bounded, as lower-priority frames cannot hold up higher priority ones.

.. note:: Time-aware shaping can be combined with 802.1Qbu Frame Preemption. This can increase bandwidth utilization because frames that otherwise 
   wouldn't fit in the available time window can be sent in fragments.

The time-aware shaper transmits different traffic streams by opening and closing gates belonging to different queues, according
to a time schedule. To guarantee low delay, this schedule needs to be coordinated among network bridges. This also means that
time-aware shaping requires synchronized clocks throughout the network.

.. note:: Gate scheduling can be a complex problem, especially in a larger network. In INET, various gate scheduling configurators can automate
          this task, see the TSN :doc:`/showcases/tsn/gatescheduling/index` showcases. In this example simulation, we use a simple schedule that we can configure by hand.

In INET, the Time-aware shaping is implemented by the :ned:`Ieee8021qTimeAwareShaper` module. This is a queue module that can be configured to replace the default simple queue 
in the MAC submodule of modular Ethernet interfaces (such as :ned:`LayeredEthernetInterface`). The shaper has multiple sub-queues and corresponding gate submodules, one for each
traffic priority class. This number can be configured with the :par:`numTrafficClasses` parameter of the shaper. By default, it has eight traffic classes, as per the IEEE 802.1Q
standard.

Here is an :ned:`Ieee8021qTimeAwareShaper` module with two traffic classes (instead of the default eight), as used in the example simulation:

.. figure:: media/tas.png
   :align: center

Some notes on the traffic flow in the shaper:

- Frames are classified according to some mechanism (PCP by default) by the classifier and are sent to one of the queues
- The gates open and close according to the configured schedule
- When a packet is pulled from the Time-aware shaper, the priority scheduler at the end pulls the frame from the first available queue through the open gate.
- The gates indicate their state with color (green for open, red for closed)

The gates are :ned:`PeriodicGate` modules. Gate scheduling is configured by setting the following parameters of the gates:

- :par:`durations`: A list of intervals when the gate is in one state (empty by default)
- :par:`initiallyOpen`: Sets whether the gate is open in the first specified interval (``true`` by default)
- :par:`offset`: Offsets the intervals specified by the :par:`durations` parameter (0s by default)

The complete period of the gate is the sum of the intervals specified with the :par:`durations` parameter.

Time-aware shaping functionality can be added to a :ned:`TsnSwitch` by setting the :par:`hasEgressTrafficShaping` parameter to ``true``. This setting replaces the default queue with
an :ned:`Ieee8021qTimeAwareShaper` in all Ethernet interfaces in the switch. 

.. note:: Setting this parameter only adds the option of Time-aware shaping to the switch. To use it, gate schedules need to be configured. 
          This is because, by default, the gates are always open; thus, without any configuration, the shaper works as a priority queue, where frames are prioritized by PCP.

Visualizing Gate Schedules
~~~~~~~~~~~~~~~~~~~~~~~~~~

The configured gate schedules can be visualized with the :ned:`GateScheduleVisualizer` module. It displays a gate schedule in time, as a colored bar near the network node containing the gate, on the top-level canvas (by default, to the right). The horizontal axis of the bar is time, and the current time is indicated by a dashed vertical line in the center. The gate schedule is displayed as color-coded blocks on the bar. Green blocks signify the open, and red blocks the closed gate state. The blocks move to the right with simulation time, so that the current time is in the center, the past is to the left, and the future is to the right. Thus, the visualization shows if the gate is currently open or closed, and when it will change state in the future.

The visualization can be enabled by setting the visualizer's :par:`displayGates` parameter to ``true``. By default, it displays all gates in the network, but this can be narrowed down with the
:par:`gateFilter` parameter.

For example, two gates in the same interface are visualized on the image below:

.. figure:: media/gatevisualizer.png
   :align: center

This visualization can be useful for an overview of gate schedules in the network, troubleshooting configurations and understanding what is happening
in the network regarding gate schedules.

Relationship to Other Traffic Shapers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`Ieee8021qTimeAwareShaper` makes it possible to add the time-aware shaping feature to other traffic shapers, such as the Credit-based shaper and the Asynchronous shaper.
For more information, check out the :doc:`/showcases/tsn/trafficshaping/creditbasedshaper/doc/index` and the :doc:`/showcases/tsn/trafficshaping/asynchronousshaper/doc/index` showcases.

The Configuration
~~~~~~~~~~~~~~~~~

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels:

.. figure:: media/Network.png
   :align: center

We configure the client to send traffic to the server, and enable time-aware traffic shaping in the switch.

In this simulation, we want to focus on how the Time-aware shaper works, and avoid any unintended traffic shaping effects by other parts of the network (such as traffic being limited
by the bandwidth of a link). Our goal is for the traffic to be only altered in the shaper, but be relatively unchanged everywhere else from the source to the destination application.

Two applications in the client create two different priority data streams: video and best effort. The data rates of the two categories fluctuate around an average nominal value, 40Mbps and 20Mbps, respectively. Note that this traffic doesn't saturate the link. Also, sometimes the data rate of the streams can be higher during a given time period, because it is specified by a distribution with the average values mentioned above. This excess traffic is held up in the shaper to limit traffic to the nominal value:

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

.. note:: We set different destination ports for the two UDP applications, so that packets can be assigned to streams by destination port later.

In the client, we want packets from the two applications to be classified into two different traffic classes: best effort and video.
To this end, we use `stream identification` in the client to assign packets to named streams (``"best effort"`` and ``"video"``), based on destination port. Based on the stream name,
we use `stream encoding` to assign the streams to traffic classes using PCP numbers.

The stream identification and stream encoding features can be enabled in :ned:`TsnDevice` by setting its :par:`hasOutgoingStreams` parameter to ``true``. We do this in the client:

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: stream identification
   :language: ini

This setting adds a :ned:`StreamIdentifierLayer` and a :ned:`StreamCoderLayer` submodule to the bridging layer in the client:

.. figure:: media/client_bridging.png
   :align: center

.. note:: The ``streamCoder`` module contains a stream encoder and a stream decoder submodule, so it works in both directions.

The stream identifier matches packets against a filter expression, and attaches request tags to matching packets.
The request tag contains the name of the assigned stream. We configure the stream identifier to assign streams based on
destination UDP port:

.. literalinclude:: ../omnetpp.ini
   :start-at: stream identification
   :end-before: stream encoding
   :language: ini

The stream encoder attaches 802.1q-tag requests to packets. Here, we can configure how to encode the various streams in the 802.1q header,
such as with VLAN ID, or PCP number. We assign the best effort stream to PCP 0, and the video stream to PCP 4:

.. literalinclude:: ../omnetpp.ini
   :start-at: stream encoding
   :end-at: video
   :language: ini

The Ieee8021qProtocol module in the link layer adds 802.1q headers to packets and sets the PCP field according to the request tags.

The traffic shaping takes place in the outgoing network interface of the switch
where both streams pass through. We enable egress traffic shaping in the switch:

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :end-at: hasEgressTrafficShaping
   :language: ini

This setting replaces the default queue with an :ned:`Ieee8021qTimeAwareShaper` module in the MAC layer of all interfaces in the switch.

Let's configure the schedules. By default, the :ned:`Ieee8021qTimeAwareShaper` has
eight traffic classes, but we only use two. To limit the data rates in the shaper
to 40Mbps and 20Mbps, we configure the schedules so that the best effort traffic class
can transmit 40% of the time, and video traffic class 20% of the time:

.. literalinclude:: ../omnetpp.ini
   :start-at: time-aware traffic shaping
   :language: ini

.. note:: We specify average data rates in the applications (by setting the packet size and send interval) and in the shaper (by setting the schedules). 
          The packets are still transmitted with 100Mbps from the client's interface, and from the shaper when the corresponding gate is open.
   
The shaper limits traffic to the nominal values. The excess traffic is temporarily stored in the MAC layer sub-queues of the
corresponding traffic class.

Results
-------

The following video shows the time when one of the gates closes and the other one opens. The schedules are visualized above the switch. Notice how they move just by a few pixels:

.. so:

  - the gates switch
  - they switch which one is open

  TODO explain this! 1 pixellel arrebb mozdul

.. video_noloop:: media/tas3.mp4
   :align: center

At the beginning of the video, the gate for the best effort traffic class is open, and the switch sends two best effort packets to the server. Then the gate for the video traffic class opens, 
and the switch sends two video packets. Note that during this time, both best effort and video packets are sent from the client to the switch. 

The following diagram shows the data rate of the video and best effort streams, measured at client application and as the incoming traffic in the traffic shaper:

.. figure:: media/ClientTrafficShaperIncoming.png
   :align: center

The data rate of both traffic classes varies randomly, but they are very similar at the two measurement points. They aren't exactly the same for a few reasons, for example:

- The data rate of the same stream is higher in the traffic shaper because the measurement includes protocol headers as well as application data.
- The two traffic classes are sent from the client to the switch mixed together, so they affect each other (e.g. a best effort frame might have to wait for a video frame to finish transmitting).

Thus the traffic is mostly unchanged when it arrives at the shaper, so we can observe the shaper's effect on the traffic.
The next diagram shows the data rate of the incoming and outgoing traffic in the shaper:

.. figure:: media/ShaperBoth.png
   :align: center

The shaper limits the data rate to the specified values.

The following sequence chart displays a duration of two gate cycles (20ms). Blue packets are best effort, red ones are video:

.. figure:: media/elog.png
   :align: center

The packets are sent by the client in random intervals. However, they are forwarded by the switch according to the configured gate schedule (in the first cycle, 0-4ms: best effort, 4-6ms: video, 6-10ms: closed).
Note that there are less packets in the queues in the first cycle than in the second, thus traffic sent by the switch doesn't fill the send windows entirely in the first cycle.

The next diagram shows the queue lengths of the traffic classes in the outgoing
network interface of the switch. The queue lengths increase over time, because
packets are not dropped, and the incoming data rate of the shaper is on average
more than 40Mbps and 20Mbps. This is due to protocol overhead.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The next diagram shows the transmitting state and the gate states of the time-aware
shaper. The open periods of the two gates are back-to-back, and the transmitter is active
for 60% of the time, for a total traffic of 60Mbps. Note that the transmitting state
diagram appears solid orange, but the state actually fluctuates rapidly between transmitting
and not transmitting.

.. figure:: media/TransmittingStateAndGateStates.png
   :align: center

The next diagram shows the relationships (for both traffic classes) between
the gate state of the transmission gates, the transmitting state of the
outgoing network interface, and the queue lengths. This view is zoomed in to the
start of the simulation, and the rapid fluctuation of the transmitter states is visible.
the transmission state fluctuates less frequently at
the beginning because the queues are not yet full.
Also, the transmitter is not active during the entire time the gates are open, because the
queues run out of packets.

.. figure:: media/TrafficShaping.png
   :align: center

The next diagram shows the already shaped traffic, measured as the outgoing data rate of the traffic shaper,
and the incoming data rate of the applications in the server, for both traffic classes. The data rate is more stable
after the shaping, as the shaper limits the rate, and smooths the traffic by sending excess traffic later. The data rate
dips when the unshaped traffic is lower. Also, the data rate is very similar in the two measurement points.

.. figure:: media/TrafficShaperOutgoingServer.png
   :align: center

To sum it up, in this scenario, the traffic originating from the sender application
stays more or less identical up until the shaper. The shaper forwards packets according to the configured gate
schedules, and this limits the average data rate of the two streams to the specified values. After shaping, the traffic stays identical up to the receiver application.
It only changes significantly in the shaper.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/802>`__ page in the GitHub issue tracker for commenting on this showcase.

