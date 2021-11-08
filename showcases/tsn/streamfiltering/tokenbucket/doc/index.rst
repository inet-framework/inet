Token Bucket based Policing
===========================

Goals
-----

In this example we demonstrate per-stream policing using chained token buckets
which allows specifying committed/excess information rates and burst sizes.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

The Model
---------

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels.

.. figure:: media/Network.png
   :align: center

There are four applications in the network creating two independent data streams
between the client and the server. The average data rates are 40 Mbps and 20 Mbps
but both varies over time using a sinusoid packet interval.

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

The per-stream ingress filtering dispatches the different traffic classes to
separate metering and filter paths.

.. literalinclude:: ../omnetpp.ini
   :start-at: ingress per-stream filtering
   :end-before: SingleRateTwoColorMeter
   :language: ini

Single Rate Two Color Meter
---------------------------

In this configuration we use a single rate two color meter for both streams. This
meter contains a single token bucket and has two parameters: committed information
rate and committed burst size. Packets are labeled green or red by the meter, and
red packets are dropped by the filter.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: "SingleRateTwoColorMeter"
   :end-before: DualRateThreeColorMeter

Results
-------

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies over time for both traffic classes using a
sinusoid packet interval.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the operation of the per-stream filter for the best effort
traffic class. The outgoing data rate equals with the sum of the incoming data rate
and the dropped data rate.

.. figure:: media/SingleRateBestEffortTrafficClass.png
   :align: center

The next diagram shows the operation of the per-stream filter for the video traffic
class. The outgoing data rate equals with the sum of the incoming data rate and
the dropped data rate.

.. figure:: media/SingleRateVideoTrafficClass.png
   :align: center

The next diagram shows the number of tokens in the token bucket for both streams.
The filled areas mean that the number of tokens changes quickly as packets pass
through. The data rate is at maximum when the line is near the minimum.

.. figure:: media/SingleRateTokenBuckets.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding per-stream filter. The reason is that they
are measured at different protocol layers.

.. figure:: media/SingleRateServerApplicationTraffic.png
   :align: center

Dual Rate Three Color Meter
---------------------------

In this configuration we use a dual rate three color meter for both streams. This
meter contains two chained token buckets and has four parameters: committed
information rate, committed burst size, excess information rate, and excess burst
size. Tokens overflow from the committed bucket into the excess bucket. Packets
are labeled green, yellow or red by the meter. In this example only red packets
are dropped by the filter. Normally, yellow packets would be treated differently.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: "DualRateThreeColorMeter"

Results
-------

The first diagram shows the operation of the per-stream filter for the best effort
traffic class. The maximum of the outgoing data rate equals with the sum of the
committed information rate and the excess information rate, which is somewhat
higher than in the single rate case.

.. figure:: media/DualRateBestEffortTrafficClass.png
   :align: center

The next diagram shows the operation of the per-stream filter for the video traffic
class. The maximum of the outgoing data rate equals with the sum of the committed
information rate and the excess information rate, which is somewhat higher than
in the single rate case.

.. figure:: media/DualRateVideoTrafficClass.png
   :align: center

The next diagram shows the number of tokens in the token bucket for both streams.
The filled areas mean that the number of tokens changes quickly as packets pass
through. The data rate is at maximum when the line is near the minimum.

.. figure:: media/DualRateTokenBuckets.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding per-stream filter. The reason is that they
are measured at different protocol layers.

.. figure:: media/DualRateServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

