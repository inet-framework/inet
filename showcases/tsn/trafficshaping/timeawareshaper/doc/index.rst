Time-Aware Shaper
=================

Goals
-----

In this example we demonstrate how to use the time-aware traffic shaper.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/timeawareshaper <https://github.com/inet-framework/tree/master/showcases/tsn/trafficshaping/timeawareshaper>`__

The Model
---------

There are three network nodes in the network. The client and the server network
nodes are :ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module.

.. figure:: media/Network.png
   :align: center

There are four applications in the network forming two independent data streams
between the client and the server. The two traffic classes are called high
priority and best effort. The data rate of both streams is ~48 Mbps at the
application level in the client. Both data streams pass through the switch and
the traffic shaping takes place in the outgoing network interface.

The traffic shaper limits the data rate of the high priority stream to 40 Mbps
and the data rate of the best effort stream to 20 Mbps. The excess traffic is
stored in the MAC layer subqueue of the corresponding traffic class.

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies randomly over time for both traffic classes
but the averages are the same.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the data rate of the incoming traffic of the traffic
shapers. This data rate is measured inside the outgoing network interface of
the switch. This diagram is somewhat different from the previous one because
the traffic is already in the switch, and also because it is measured at a
different protocol level.

.. figure:: media/TrafficShaperIncomingTraffic.png
   :align: center

The next diagram shows the data rate of the already shaped outgoing traffic of
the traffic shapers. This data rate is still measured inside the outgoing network
interface of the switch but at a different location. The randomly varying data
rate of the incoming traffic is already transformed here into a somewhat stable
data rate. This data rate is not as stable as with the credit-based shaper, for
example. The reason is that the data rate measurement interval is comparable
with the gate open and close durations.

.. figure:: media/TrafficShaperOutgoingTraffic.png
   :align: center

The next diagram shows the queue lengths of the traffic classes in the outgoing
network interface of the switch. The queue lengths increase over time because
the data rate of the incoming traffic of the traffic shapers is greater than
the data rate of the outgoing traffic, and packets are not dropped.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

The next diagram shows the relationships (for both traffic classes) between
the gate state of the transmission gates and the transmitting state of the
outgoing network interface.

.. figure:: media/TrafficClasses.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding traffic shaper. The reason is that they
are measured at different protocol layers.

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`TimeAwareShaperShowcase.ned <../TimeAwareShaperShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

