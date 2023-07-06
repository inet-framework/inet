Credit-Based Shaping
====================

Goals
-----

Credit-based shaping (CBS), as defined in the IEEE 802.1Qav standard, is a
traffic shaping mechanism that regulates the transmission rate of Ethernet
frames to smooth out traffic and reduce bursts.

In this showcase, we demonstrate the configuration and operation of credit-based shaping in INET
with an example simulation.

.. **TODO** some interesting stuff to show? -> shaping in general increases delay even for high priority frames. but can overall decrease delay (as it decreases delay for lower priority frames)

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/creditbasedshaper <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/creditbasedshaper>`__

Credit-Based Shaping Overview
-----------------------------

The Credit Based Shaping (CBS) is an algorithm designed for
network traffic management. Its core function is to limit the bandwidth a
Traffic Class queue can transmit, ensuring optimal bandwidth distribution.
It helps smoothing out bursts by delaying the transmission of successive
frames. CBS helps in mitigating network
congestion in bridges and enhancing overall network performance.

In CBS, each outgoing queue is associated with a credit counter. The credit
counter accumulates credits when the queue is idle, and consumes credits when
frames are transmitted. The rate at which credits are accumulated and consumed
is configured using parameters such as the `idle slope` and `send slope`.

When the queue contains a packet to be transmitted, the credit counter is checked. If the
credit counter is non-negative, the frame is transmitted immediately. If the
credit counter is negative, the frame is held back until the credit counter
becomes non-negative.

CBS Implementation in INET
--------------------------

In INET, the credit-based shaper is implemented by the
:ned:`Ieee8021qCreditBasedShaper` simple module. This module acts as a packet gate,
allowing packets to pass through only when it is open. It can be combined with a
packet queue to implement the credit-based shaper algorithm. 

The :ned:`Ieee8021qCreditBasedShaper` module has the following parameters:

- :par:`idleSlope`: Determines the outgoing data rate of the shaper, measured in bits per second
- :par:`sendSlope`: The consumption rate of credits during transmission, measured in bits per second (default: idleSlope - channel bitrate)
- :par:`transmitCreditLimit`: credit limit above which the gate is open, measured in bits (default: 0)
- :par:`minCredit` and :par:`maxCredit`: a minimum and maximum limit for credits, measured in bits (default: no limit)

The :par:`idleSlope` parameter determines the data rate at which the traffic will be limited, `as measured
in the Ethernet channel` (thus including protocol overhead). Typically, this is the only parameter that needs to be set,
as the others have reasonable defaults.

The shaper allows packets to pass through when the number of credits is zero or
more. When the number of credits is positive, the shaper accumulates a burst
reserve. As defined in the standard, if there are no packets in the queue, the
credits are set to zero.

To conveniently incorporate a credit-based shaper into a network interface, it
can be added as a submodule to an :ned:`Ieee8021qTimeAwareShaper`. The
:ned:`Ieee8021qTimeAwareShaper` module supports a configurable number of traffic
classes, pre-existing queues for each class, and can be enabled for Ethernet
interfaces by setting the :par:`enableEgressTrafficShaping` parameter in the
network node to ``true``. To utilize a credit-based shaper, the
``transmissionSelectionAlgorithm`` submodule of the time-aware shaper can be
overridden accordingly. As an example, here is a time-aware shaper module that
incorporates credit-based shaper submodules and supports two traffic classes:

.. figure:: media/timeawareshaper_.png
   :align: center

Packets entering the time-aware shaper module are classified into different
traffic categories based on their PCP number using the
:ned:`PcpTrafficClassClassifier`. The priority assignment is determined by the
`transmissionSelection` submodule, which utilizes a :ned:`PriorityScheduler` configured
to operate in reverse order, i.e. priority increases with the
traffic class index. For example, in the provided image, video traffic takes
precedence over best effort traffic.

The Model
---------

The Network
+++++++++++

We demonstrate the operation of CBS using a network containing a client, a server and a switch.
The client and the server (:ned:`TsnDevice`)
are connected through the switch (:ned:`TsnSwitch`), with 100Mbps Ethernet
links:

.. figure:: media/Network.png
   :align: center

Traffic
+++++++

In this simulation, we configure the client to generate two streams of
fluctuating traffic, which are assigned to two traffic categories. We insert
credit-based shapers for each category into the switch's outgoing interface
(``eth1``) to smooth traffic.

Analogous to the Time-Aware Shaping showcase, our objective is to isolate and
observe the impact of the credit-based shaper on network traffic. To this end,
we aim for the traffic to be only modified significantly by the credit-based
shaper, avoiding any unintended traffic shaping effects elsewhere in the
network. To achieve this, we set up two traffic source applications in the
client, creating two separate data streams whose throughput varies sinusoidally
with maximum values of ~47 Mbps and ~34 Mbps, respectively. Given these
values, the network links are not operating at their full capacity,
thereby eliminating any significant traffic shaping effects resulting from link
saturation. Subsequently, we configure the traffic shaper to cap the data rates
of these streams at ~42 Mbps and ~21 Mbps, respectively. As a result, the
average incoming data rate is lower than the outgoing limit. Below are the
details of the traffic configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

Traffic Shaping
+++++++++++++++

Within the client, our goal is to classify packets originating from the two
packet sources into two traffic classes: `best effort` and
`video`. To achieve this, we activate IEEE 802.1 stream
identification and stream encoding functionalities by setting the
:par:`hasOutgoingStreams` parameter in the switch to ``true``. We proceed by configuring the stream
identifier module within the bridging layer; this module is responsible for
associating outgoing packets with named streams based on their UDP destination
ports. Following this, the stream encoder sets the Priority Code Point (PCP) number on the packets according to
the assigned stream name (using the IEEE 802.1Q header's PCP field):

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: egress traffic shaping
   :language: ini

We enable egress traffic shaping in the switch, which
adds the time-aware shaper module to interfaces. In the time-aware shaper, we define two
traffic classes, and configure the transmission selection algorithm
submodules by setting their type to :ned:`Ieee8021qCreditBasedShaper`. This action
adds two credit-based shaper modules, one for each traffic class. We
then set the idle slope parameters of the two credit-based shapers to
~42 Mbps and ~21 Mbps, respectively:

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :language: ini

Results
-------

Let's take a look at how the traffic changes in and between the various network nodes. First, we
compare the data rate of the client application traffic with the shaper incoming
traffic for the two traffic categories:

.. figure:: media/client_shaper.png
   :align: center

The client application and shaper incoming traffic is quite similar, but not identical. The shaper's incoming traffic
has a slightly higher data rate because of additional protocol overhead that
wasn't present in the application. Also, the two streams of packets are
combined in the client's network interface, which can cause some packets to be
delayed. Therefore, even if we adjusted for the extra protocol overhead, the traffic
wouldn't match exactly.

Now let's examine how the traffic changes in the shaper by comparing the data
rate of the incoming and outgoing traffic:

.. figure:: media/shaper_both.png
   :align: center

On average, the data rate of the incoming traffic is below the shaper's limit, but there are intermittent periods when it exceeds the limit.
When this happens, the shaper
caps the data rate at the set limit, by storing packets temporarily
and then sending them out eventually, which smooths out the
outgoing traffic. When the data rate of the incoming traffic is below the
shaper's limit, traffic shaping isn't needed, and the outgoing traffic mirrors
the incoming traffic.

.. note:: The data rate specified in the ini file as the idle slope parameter
          corresponds to the channel data rate. However, the outgoing data rate inside the
          shaper differs slightly due to protocol overhead, including factors like the PHY
          (Physical Layer) and IFG (Interframe Gap). In this chart, we measure the data
          rate within the shaper, so we have displayed the data rate limits as calculated
          specifically for the shaper.

The next chart compares the shaper outgoing and server application traffic:

.. figure:: media/shaper_server.png
   :align: center

Much like the first chart, the shaper outgoing and server application traffic profiles are similar, but the shaper's
traffic is slightly higher due to protocol overhead. 

Having examined the traffic across the entire network, we can conclude that the
traffic undergoes significant changes mainly within the shaper, while other
parts of the network remain unaffected by traffic shaping, consistent with our
expectations.

The sequence chart below illustrates the transmission of frames within the
network. In this chart, the `best effort` traffic category is represented in
blue, while the `video category` is depicted in red.

.. figure:: media/seqchart2.png
   :align: center

The traffic is bursty when it arrives in the switch.
The traffic shaper within the switch evenly distributes the packets and
interleaves video packets with those of the best effort category.

The next diagram illustrates the queue lengths of the traffic classes in
the switch's outgoing network interface. The queue lengths don't increase over
time, as the average data rate of the incoming traffic to the shaper is lower
than the permitted data rate for outgoing traffic.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The following chart displays the rapid fluctuations in the number of credits.
The number can exceed zero when the corresponding queue is not empty.

.. figure:: media/TrafficShaperNumberOfCredits.png
   :align: center

The next chart provides a closer view of the chart shown above, with a zoomed-in perspective:

.. figure:: media/TrafficShaperNumberOfCredits_zoomed.png
   :align: center

The following chart illustrates the gate states for the two credit-based
shapers, as well as the transmitting state of the outgoing interface in the
switch. It is worth noting that the gates can remain open for longer durations
than a single packet transmission if the number of credits is zero or higher.
Additionally, the transmitter is not in a constant transmitting state since the
maximum outgoing data rate of the switch (~63Mbps) is lower than the channel
capacity (100Mbps).

.. figure:: media/TransmittingStateAndGateStates.png
   :align: center

The following diagram depicts the relationships among the number of credits, the
gate state of the credit-based transmission selection algorithm, and the
transmitting state of the outgoing network interface for both traffic classes.
To ensure visibility of the details, the diagram focuses on the first 2ms of the simulation:

.. figure:: media/TrafficShaping.png
   :align: center

Note that the queue length remains at zero for most of the time since an
incoming packet can be transmitted immediately without causing the queue length
to increase to 1. Additionally, in the transmitter, there are instances where
two packets (each from a different traffic class) are transmitted consecutively,
with only an Interframe Gap period between them. For example, this can be
observed in the first two transmissions. It's important to highlight that such
back-to-back transmission of packets from different traffic classes does not
result in bursting within individual traffic classes.

In this diagram, we can observe the operation of the credit-based shaper.
Let's consider the number of credits for the video traffic category as an
example. Initially, the number of credits is at 0. Then, when a packet arrives
at the queue and begins transmitting, the number of credits decreases. After the
transmission completes, the number of credits begins to increase again. As the
number of credits reaches 0 once more, another transmission starts, causing the
number of credits to decrease once again.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/800>`__ page in the GitHub issue tracker for commenting on this showcase.

