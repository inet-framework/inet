Mixing Different Shapers
========================

Goals
-----

In this example we demonstrate how to use different traffic shapers in the same
network interface.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/mixingshapers <https://github.com/inet-framework/tree/master/showcases/tsn/trafficshaping/mixingshapers>`__

The Model
---------

There are three network nodes in the network. The source and the destination are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module.

.. figure:: media/Network.png
   :align: center

There are two independent data streams between the source and the destination.
The streams are called high priority and best effort. The data rate of both
streams is ~48 Mbps at the application level in the source. Both streams pass
through the switch and the traffic shaping takes place.

The traffic shaper limits the data rate of the high priority stream to 40 Mbps
and the data rate of the best effort stream to 20 Mbps. The high priority stream
uses the credit-based shaper algorithm, and the best effort stream uses the
asynchronous shaper algorithm. For the latter the ingress stream filtering is
also required in order to calculate the packet transmission eligibility time. 

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

The first diagram shows the data rate of the application level outgoing traffic
in the source. The data rate varies randomly over time but the averages are the
same.

.. figure:: media/SourceApplicationTraffic.png
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

The next diagram shows the queue lengths of the traffic shapers in the outgoing
network interface of the switch. The queue lengths increase over time because
the data rate of the incoming traffic of the shapers is greater than the data
rate of the outgoing traffic.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The next diagram shows the relationships (for the high priority traffic class)
between the number of credits, the gate state of the credit based transmission
selection algorithm, and the transmitting state of the outgoing network interface.

.. figure:: media/TrafficClass0.png
   :align: center

The next diagram shows the relationships (for the best effort traffic class)
between the gate state of the asynchronous transmission selection algorithm
and the transmitting state of the outgoing network interface.

.. figure:: media/TrafficClass1.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the destination. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding traffic shaper. The reason is that they
are measured at different protocol layers.

.. figure:: media/DestinationApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MixingShapersShowcase.ned <../MixingShapersShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

