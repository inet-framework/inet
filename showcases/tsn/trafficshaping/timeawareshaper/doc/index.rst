Time-Aware Shaping
==================

Goals
-----

The IEEE 802.1qbv Time-aware shaping schedules the transmission of time-critical and lower priority
frames in time-triggered windows, and as such guarantees bounded latency for time-critical frames.

In this showcase, we demonstrate how to use time-aware traffic shaping.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/timeawareshaper <https://github.com/inet-framework/tree/master/showcases/tsn/trafficshaping/timeawareshaper>`__

The Model
---------

Overview
~~~~~~~~

.. **V1** The time-aware shaper (TAS) utilizes Time-division multiple access (TDMA) to schedule different traffic
   streams in their own send windows. Each priority stream has its own allotted time for transmission. The TAS
   makes use of 802.1qbu Frame Preemption, so frames that otherwise wouldn't fit in the available time window can be 
   sent in fragments, and this also makes sure that no frame interferes with the time window of another.

Time-aware shaping (TAS), and its implementation, the Time-aware shaper, utilizes the Time-division multiple access (TDMA) scheme to send different priority traffic streams
in their own allotted transmission time windows. This makes the delay bounded, as lower-priority frames cannot hold up higher priority ones.

.. note:: Time-aware shaping can be combined with 802.1qbu Frame Preemption. This can increase bandwidth utilization because frames that otherwise 
   wouldn't fit in the available time window can be sent in fragments.

The time-aware shaper transmits different traffic streams by opening and closing gates belonging to different queues, according
to a time schedule. To guarantee low delay, this schedule needs to be coordinated among network bridges. This also means that
time-aware shaping requires synchronized clocks throughout the network.

.. note:: Gate scheduling can be a complex problem, especially in a larger network. In INET, various gate scheduling configurators can automate
          this task, see the TSN :doc:`/showcases/tsn/gatescheduling/index` showcases. In this example simulation, we use a simple schedule that we can configure by hand.

In INET, the Time-aware shaping is implemented by the :ned:`Ieee8021qTimeAwareShaper` module. This is a queue module that can be configured to replace the default simple queue 
in the MAC submodule of modular Ethernet interfaces (such as :ned:`LayeredEthernetInterface`). The shaper has multiple sub-queues and corresponding gate submodules, one for each
traffic priority class. This number can be configured with the :par:`numTrufficClasses` parameter of the shaper. By default, it has eight traffic classes, as per the IEEE 802.1Q
standard.

Here is a :ned:`Ieee8021qTimeAwareShaper` module with two traffic classes (instead of the default eight), as used in the example simulation:

.. **TODO** number of traffic classes

.. figure:: media/tas.png
   :align: center

Some notes on the traffic flow in the shaper:

- Frames are classified according to some mechanism (PCP by default) by the classifier and are sent to one of the queues
- The gates open and close according to the configured schedule
- When a packet is pulled from the Time-aware shaper, the priority scheduler at the end pulls the frame from the first available queue through the open gate.
- The gates indicate their state with color (green for open, red for closed)

.. The schedules can be configured at the gates, with the gates' 

.. The gates are :ned:`PeriodicGate` modules, with the following parameters for configuring the gate schedules:

The gates are :ned:`PeriodicGate` modules. Gate scheduling is configured by setting the following parameters of the gates:

- :par:`durations`: A list of intervals when the gate is in one state (empty by default)
- :par:`initiallyOpen`: Sets whether the gate is open in the first specified interval (``true`` by default)
- :par:`offset`: Offsets the intervals specified by the :par:`durations` parameter (0s by default)

The complete period of the gate is the sum of the intervals specified with the :par:`durations` parameter.

.. The default queue can be replaced with the shaper by setting the :par:`hasEgressTrafficShaping` parameter in TsnSwitch to ``true``. This configures a Ieee8021qTimeAwareShaper to
   all Ethernet interfaces. 

Time-aware shaping functionality can be added to a :ned:`TsnSwitch` by setting the :par:`hasEgressTrafficShaping` parameter to ``true``. This setting replaces the default queue with
a :ned:`Ieee8021qTimeAwareShaper` in all Ethernet interfaces in the switch. 

.. TODO refine this.

.. This adds a TAS to all Ethernet interfaces
   in the switch. TODO not a tas a possibility of tas; TAS is time aware shaping;

.. note:: Setting this parameter only adds the possibility of Time-aware shaping to the switch. To use it, gate schedules need to be configured. 
          This is because, by default, the gates are always open; thus, without any configuration, the shaper works as a priority queue, where frames are prioritized by PCP.

..           - The Ieee8021qTimeAwareShaper module can be used to implement other traffic shapers, such as the Credit-based and the asynchronous shapers.
            more on this in the TODO showcases. In this case, the Ieee8021qTimeAwareShaper module can optionally provide "time-awareness" if the gates
            are configured. 
            
.. **TODO** ezt masik noteba valahova mashova;
            a timeaweareshaper lehetove teszi hogy tassal mint featurrel
            kombinaljunk mas shapereket credit based async; KOMBINALNI LEHET
            a feature-oket. errol majd kesobb lesz szo. 
            TODO rename to GateScheduleVisualizer module
            
..            **TODO** too many points

Visualizing Gate Schedules
~~~~~~~~~~~~~~~~~~~~~~~~~~

The configured gate schedules can be visualized with the :ned:`GateScheduleVisualizer` module. It displays a gate schedule in time, as a colored bar near the network node containing the gate, on the top-level canvas (by default, to the right). The horizontal axis of the bar is time, and the current time is indicated by a dashed vertical line in the center. The gate schedule is displayed as color-coded blocks on the bar. Green blocks signify the open, and red blocks the closed gate state. The blocks move to the right with simulation time, so that the current time is in the center, the past is to the left, and the future is to the right. Thus the visualization shows if the gate is currently open or closed, and when in the future it will change state.

The visualization can be enabled by setting the visualizer's :par:`displayGates` parameter to ``true``. By default, it displays all gates in the network, but this can be narrowed with the
:par:`gateFilter` parameter.

For example, two gates in the same interface are visualized on the image below:

.. TODO hol jelenik meg egyszerre tobbre etc)

.. Here is how it looks:

.. figure:: media/gatevisualizer.png
   :align: center

This visualization can be useful for an overview of gate schedules in the network, troubleshooting configurations and understanding what is happening
in the network pertaining to gate schedules.

.. so

  - the TAS uses TDMA
  - so each priority/frame etc have its own transmission window

  The TAS uses a schedule for the opening and closing of gates in all interfaces in the network

  All interfaces in the network has a shedule of gate opening/closing, and they need to be synchronized to each other.

  this, but better.

Relationship with Other Traffic Shapers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
by the bandwidth of a link). Thus our goal is for the traffic to be only altered in the shaper, but be relatively unchanged everywhere else from the source to the destination application.

.. We create two different priority data streams in the client with a data rate that fluctuates randomly around a nominal value.
   The traffic is then limited to the nominal data rate in the time-aware shaper in a switch before getting sent to the server.

   Two applications in the client create two independent data streams
   between the client and the server. The data rates of the applications fluctuate around 40Mbps and 20Mbps:

Two applications in the client create two different priority data streams, video and best effort. The data rates of the two categories fluctuate around an average nominal value, 40Mbps and 20Mbps, respectively. Note that this traffic doesn't saturate the link. Also, sometimes the data rate of the streams can be higher during a given time period, because it is specified by a distribution with the average values mentioned above. This excess traffic is held up in the shaper to limit traffic to the nominal value:

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

.. note:: We set different destination ports for the two UDP applications, so that packets can be assigned to streams based on destination port later.

.. Our goal is to limit the two streams to the nominal 40 and 20 Mbps with the TAS in the switch. **TODO** picit redundans; read it again and again

In the client, we want packets from the two applications to belong to two different traffic classes: best effort and video.
To this end, we use `stream identification` in the client to assign packets to named streams (``"best effort"`` and ``"video"``), based on destination port. Based on the stream name,
we use `stream encoding` to assign the streams to traffic classes using PCP numbers.

The stream identification and stream encoding features can be enabled in :ned:`TsnDevice` by setting its :par:`hasOutgoingStreams` parameter to ``true``. We do this in the client:

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: stream identification
   :language: ini


.. For this, we enable outgoing stream identification and stream encoding in the client:

   a ket sztream 2 kulonbozo traffic classhoz tartozzon
   ehheez turn on stream identificaion and stream encoding.
   TODO ezt nem értem.

   eloszor streamhez rendeljuk aztán a streamet pcpvel traffic classhoz rendeli!

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

.. The two streams have two different traffic classes: best effort and video. The
   bridging layer identifies the outgoing packets by their UDP destination port.
   The client encodes the streams using the PCP field of the IEEE 802.1Q header.

   .. literalinclude:: ../omnetpp.ini
      :start-at: outgoing streams
      :end-before: egress traffic shaping
      :language: ini

The traffic shaping takes place in the outgoing network interface of the switch
where both streams pass through. We enable egress traffic shaping in the switch:

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :end-at: hasEgressTrafficShaping
   :language: ini

.. This adds a :ned:`Ieee8021qTimeAwareShaper` module to the interface's MAC layer:
   TODO nem kell screenshot. kontrollalja a queueol kihuzhato csomagok idoziteset

This setting replaces the default queue with a :ned:`Ieee8021qTimeAwareShaper` module in the MAC layer of all interfaces in the switch.

..   **TODO** ne legyen 2 stilus; nem adds hanem replaces the default queue

.. .. figure:: media/tas2.png
      :align: center

Let's configure the schedules. By default, the :ned:`Ieee8021qTimeAwareShaper` has
eight traffic classes, but we only use two. To limit the data rates in the shaper
to 40Mbps and 20Mbps, we configure the schedules so that the best effort traffic class
can transmit 40% of the time, and video traffic class 20% of the time:

.. but we need only two, so we set the number of traffic classes to two.

   We configure the traffic shaper to limit the data rate of the
   best effort stream to 40 Mbps and the data rate of the video stream to 20 Mbps by
   limiting the time during which they can be transmitted. **TODO** rewrite
   The speed on the interface is 100Mbps, so for 40Mbps data rate, the best effort traffic class
   can transmit during 4/10th of the time, the video for 2/10th of the time:

.. literalinclude:: ../omnetpp.ini
   :start-at: time-aware traffic shaping
   :language: ini

.. .. note:: The specified data rate is interpreted(nope) as on average/averaged; the streams(packets of the stream) still transmitted with 100Mbps when the corresponding gate is open. TODO rewrite

.. .. note:: We specify the average data rate; the packets are still transmitted with 100Mbps when the corresponding gate is open.

.. note:: We specify average data rates in the applications (by setting the packet size and send interval) and in the shaper (by setting the schedules). 
          The packets are still transmitted with 100Mbps from the client's interface, and from the shaper when the corresponding gate is open.

..    the data rate of the stream will be the specified value on average.
      ugy valosul meg h atlagosan lesz annyi

.. TODO redo video with animation time -> nem latszik a time awareseg
   
The shaper limits traffic to the nominal values. The excess traffic is temporarily stored in the MAC layer sub-queues of the
corresponding traffic class.

.. The traffic shaper limits the data rate of the
   best effort stream to 40 Mbps and the data rate of the video stream to 20 Mbps.
   The excess traffic is temporarily stored in the MAC layer subqueues of the
   corresponding traffic class.

Results
-------

.. Here is a video of the simulation:

The following video shows the time when one of the gates closes and the other one opens. The schedules are visualized above the switch. Notice how they move just by a few pixels:

.. so:

  - the gates switch
  - they switch which one is open

  TODO explain this! 1 pixellel arrebb mozdul

.. video_noloop:: media/tas3.mp4
   :align: center

At the beginning of the video, the gate for the best effort traffic class is open, and the switch sends two best effort packets to the server. Then the gate for the video traffic class opens, 
and the switch sends two video packets. Note that during this time, both best effort and video packets are sent from the client to the switch. 

.. TODO ne induljon ujra; eleg par frame; redo mert maradtak frameek egy masikbol

.. TODO a switchen ugyanaz a traffic van egeszen a shaperig
   a meres azon a ponton van ahol megtortenik a change
   a shaperbol kijovo traffic a destinationig ugyanaz

   TODO egy chartra az elso kettot nezzuk meg milyen mennyire ertelmezheto

   TODO az excess trafficet tarolja el a queueban

   TODO a narancssarga azert sarga mert valtakozik

   TODO last figure: be van zoomolva; az elejen van; nem mindig van fullon a queue;
   a best effort elso kinyitasnal meg nincs tele ezert nem transmittal annyira surun
   picit szoros

   TODO a traffic over space is valtozik nem csak over time (hol nezed) a helye is szamit
   bonyolultabb esetben ez bonyolultabb lehet

   TODO az elso a traffic eleje; nem feltétlenül ugyanaz



.. **V1** The following diagram shows the data rate of the application level outgoing traffic TODO applications in the client
   in the client, and the data rate of the incoming traffic of the traffic
   shapers. The data rate varies randomly over time for both traffic classes.
   The client application data rate is measured at the application level, 
   and the traffic shaper incoming traffic is measured inside the outgoing network interface of
   the switch, before traffic shaping happens. The two data rates for a traffic category are only slightly different
   from the each other. The data rate in the interface is slightly lower because it is measured at a different protocol level.
   Also, at this point in time, the traffic has already been transmitted via a link,
   which alters it a somewhat. TODO redo; a tobbi traffic bekavarhat nem lenne igyen
   szep; a video bekavar a masikba ezert nem koveti szepen. 
   ahogy kijon az applicationbol fuggetlen de a switchben mar nem
   ha saturated lenne a network a client es switch kozott akkor nem lenne ugyanaz
   konstans tavolsagban kene lenniuk

The following diagram shows the data rate of the video and best effort streams, measured at client application and as the incoming traffic in the traffic shaper:

.. figure:: media/ClientTrafficShaperIncoming.png
   :align: center

The data rate of both traffic classes varies randomly, but they are very similar at the two measurement points. They aren't exactly the same for a few reasons, for example:

- The data rate of the same stream is higher in the traffic shaper because the measurement includes protocol headers as well as application data.
- The two traffic classes are sent from the client to the switch mixed together, so they affect each other (e.g. a best effort frame might have to wait for a video frame to finish transmitting).

..    azert kell ez hogy a shapert nezzuk csak!
   ne az interface shapeljen csak a shaper shapeljen lehetoleg.
   nagyon nem shapelnek. -> ez demonstracios celu
   velosagban bonyolultabb lehet; a shaper csak garantál

   TODO record with OBS studio

Thus the traffic is mostly unchanged when it arrives at the shaper, so we can observe the shaper's effect on the traffic.

.. **V2** The first diagram shows the data rate of the application level outgoing traffic
   in the client. The data rate varies randomly over time for both traffic classes.

   .. figure:: media/ClientApplicationTraffic.png
      :align: center

   The next diagram shows the data rate of the incoming traffic of the traffic
   shapers. This data rate is measured inside the outgoing network interface of
   the switch, before traffic shaping happens. This diagram is only a bit different
   from the previous one. The data rate here is slightly higher because it is measured at a different protocol level.
   Also, at this point in time, the traffic has already been transmitted via an interface,
   which alters it a bit. TODO too generic; nem a lenyeg h alters hanem mixed!
   in the application level they independent; in the tas incdming they are mixed.
   thats the main reason.

   .. This diagram is somewhat different from the previous one because
      the traffic is already in the switch, and also because it is measured at a
      different protocol level.

   .. figure:: media/TrafficShaperIncomingTraffic.png
      :align: center

The next diagram shows the queue lengths of the traffic classes in the outgoing
network interface of the switch. The queue lengths increase over time, because
packets are not dropped, and the incoming data rate of the shaper is on average
more than 40Mbps and 20Mbps. This is due to protocol overhead.

.. the data rate of the applications are sometimes higher than the average nominal rate,
   and the overhead of the protocol headers increase the data rate as well.
   The shaper limits the data rate to the nominal average rate of 40Mbps and 20Mbps,
   the same rate the applications generate traffic.

.. The queue lengths increase over time because
  the data rate of the incoming traffic of the traffic shapers is greater than
   the data rate of the outgoing traffic, and packets are not dropped.

.. TODO nem egyertelmu; distribuion avg datarate + overhead miatt tobb
   szoval average tobb

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The next diagram shows the transmitting state and the gate states of the time-aware
shaper. The open periods of the two gates are back-to-back, and the transmitter is active
for 60% of the time, for a total traffic of 60Mbps. Note that the transmitting state
diagram appears solid orange, but the state actually fluctuates rapidly between transmitting
and not transmitting.

.. TODO

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

.. **V1** The next diagram shows the data rate of the already shaped outgoing traffic of
   the traffic shapers. This data rate is still measured inside the outgoing network
   interface of the switch but at a different location. The randomly varying data
   rate of the incoming traffic is already transformed here into a somewhat stable
   data rate. This data rate is not as stable as with the credit-based shaper, for
   example. The reason is that the data rate measurement interval is comparable
   with the gate open and close durations. The shaper smooths out the traffic by
   sending excess traffic later, but the data rate dips when the unsmoothed traffic
   is lower.

.. **TODO** itt is egy chart kell -> signifant change in traffic only happens
   in the shaper. ezert kell ez hogy itt se, not contant in time but in place to place
   ezt akarjuk demonstralni

The next diagram shows the already shaped traffic, measured as the outgoing data rate of the traffic shaper,
and the incoming data rate of the applications in the server, for both traffic classes. The data rate is more stable
after the shaping, as the shaper limits the rate, and smooths the traffic by sending excess traffic later. The data rate
dips when the unshaped traffic is lower. Also, the data rate is very similar in the two measurement points.

.. figure:: media/TrafficShaperOutgoingServer.png
   :align: center

.. The data rate only changes significantly in the shaper.

.. .. figure:: media/TrafficShaperOutgoingTraffic.png
   :align: center

.. The last diagram shows the data rate of the application level incoming traffic
   in the server. The data rate is very similar but somewhat lower than the data rate of the
   outgoing traffic of the corresponding traffic shaper because they
   are measured at different protocol layers.

.. .. figure:: media/ServerApplicationTraffic.png
   :align: center

To sum it up, in this scenario, the traffic originating from the sender application
stays more or less identical up until the shaper. After shaping, it stays identical up to the receiver application.
It only changes significantly in the shaper.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

