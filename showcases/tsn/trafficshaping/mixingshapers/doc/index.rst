Mixing Different Shapers
========================

Goals
-----

In this example we demonstrate how to use different traffic shapers in the same
network interface.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/mixingshapers <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/mixingshapers>`__

The Model
---------

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels.

.. figure:: media/Network.png
   :align: center

There are four applications in the network creating two independent data streams
between the client and the server. The data rate of both streams are ~48 Mbps at
the application level in the client.

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

The two streams have two different traffic classes: best effort and video. The
bridging layer identifies the outgoing packets by their UDP destination port.
The client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
field.

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: ingress per-stream filtering
   :language: ini

The asynchronous traffic shaper requires the transmission eligibility time for
each packet to be already calculated by the ingress per-stream filtering.

.. literalinclude:: ../omnetpp.ini
   :start-at: ingress per-stream filtering
   :end-before: egress traffic shaping
   :language: ini

The traffic shaping takes place in the outgoing network interface of the switch
where both streams pass through. The traffic shaper limits the data rate of the
best effort stream to 40 Mbps and the data rate of the video stream to 20 Mbps.
The excess traffic is stored in the MAC layer subqueues of the corresponding
traffic class.

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :language: ini

Results
-------

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies randomly over time but the averages are the
same.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the data rate of the incoming traffic of the traffic
shapers of the outgoing network interface in the switch. This is different
from the previous because the traffic is already in the switch and it is also
measured at different protocol level.

.. figure:: media/TrafficShaperIncomingTraffic.png
   :align: center

The next diagram shows the data rate of the already shaped outgoing traffic of
the outgoing network interface in the switch. The randomly varying data rate of
the incoming traffic is transformed into a quite stable data rate for the outgoing
traffic.

.. figure:: media/TrafficShaperOutgoingTraffic.png
   :align: center

TODO

.. figure:: media/TransmittingStateAndGateStates.png
   :align: center

The next diagram shows the queue lengths of the traffic shapers in the outgoing
network interface of the switch. The queue lengths increase over time because
the data rate of the incoming traffic of the shapers is greater than the data
rate of the outgoing traffic.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The next diagram shows the relationships between the number of credits, the gate
state of the credit based transmission selection algorithm, and the transmitting
state of the outgoing network interface for the best effort traffic class.

.. figure:: media/BestEffortTrafficClass.png
   :align: center

The next diagram shows the relationships between the number of credits, the gate
state of the credit based transmission selection algorithm, and the transmitting
state of the outgoing network interface for the video traffic class.

.. figure:: media/VideoTrafficClass.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding traffic shaper. The reason is that they
are measured at different protocol layers.

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/801>`__ page in the GitHub issue tracker for commenting on this showcase.

