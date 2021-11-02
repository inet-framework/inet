Asynhronous Shaper
==================

Goals
-----

In this example we demonstrate how to use the asynchronous traffic shaper.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/asynchronousshaper <https://github.com/inet-framework/tree/master/showcases/tsn/trafficshaping/asynchronousshaper>`__

The Model
---------

There are three network nodes in the network. The source and destination network
nodes are :ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module.

.. figure:: media/Network.png
   :align: center

There are four applications in the network forming two independent data streams
between the source and the destination. The two traffic classes are called high
priority and best effort. The data rate of both streams is ~48 Mbps at the
application level in the source. Both data streams pass through the switch and
the traffic shaping takes place in the outgoing network interface.

The traffic shaper limits the data rate of the high priority stream to 40 Mbps
and the data rate of the best effort stream to 20 Mbps. The excess traffic is
stored in the MAC layer subqueue of the corresponding traffic class.

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

The first diagram shows the data rate of the application level outgoing traffic
in the source. The data rate varies randomly over time for both traffic classes
but the averages are the same.

.. figure:: media/SourceApplicationTraffic.png
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
interface of the switch but at a different location. As it is quite apparent,
the randomly varying data rate of the incoming traffic is already transformed
here into a quite stable data rate.

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
in the destination. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding traffic shaper. The reason is that they
are measured at different protocol layers.

.. figure:: media/DestinationApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AsynchronousShaperShowcase.ned <../AsynchronousShaperShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

