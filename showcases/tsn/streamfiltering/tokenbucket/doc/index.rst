Token Bucket based Policing
===========================

Goals
-----

In this example we demonstrate per-stream policing using chained token buckets
which allows specifying committed/excess information rates and burst sizes.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

The Model
---------

so

- tsnswitch has ingresstrafficfiltering
- tsndevice has egress and ingress traffic filtering
- this adds a streamfilteringlayer to the network node
- the streamfilteringlayer has optional ingress and egress filtering submodules
- the proper ones are enabled according to the parameters
- by default the ingress/egress filtering submodule is a SimpleIeee8021qFilter,
  that can do per-stream filtering. it has a configurable number of traffic streams,
  and a meter, a filter and a gate for each. additionally, there is the default route
  which unfiltered packets take by default.
- the meter can do metering
- the filter drops the packets (might be based on the metering)
- the gate is an interactive gate by default, so its always open
- example

- the classifier is a streamclassifier by default -> classify according to named streams
- token bucket filtering can be done with the SingleRateTwoColorMeter module
- this is a queueing element that has chained token buckets (?)
- can specify a committed information rate and a committed burst rate
- the rate of token regeneration defines the information rate
- the max number of tokens the burst size (if lots of tokens accumulate, the node can transmit faster/until they are depleted)
- the meter actually labels packets green or red, according to the number of tokens available (if there are no tokens, then red)
- the filter is a labelfilter and it drops red packets
- note check out other meter modules (DualRateThreeColorMeter)
- the submodules in the simpleieee8021qfilter doesn't have to be the same type for the different traffic categories (e.g. check out the mixing shapers)

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

We use a single rate two color meter for both streams. This meter contains a
single token bucket and has two parameters: committed information rate and
committed burst size. Packets are labeled green or red by the meter, and red
packets are dropped by the filter.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SingleRateTwoColorMeter

Results
-------

.. figure:: media/datarate_tokens_be.png
   :align: center

.. figure:: media/datarate_tokens_vi.png
   :align: center

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies over time for both traffic classes using a
sinusoid packet interval.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the operation of the per-stream filter for the best effort
traffic class. The outgoing data rate equals with the sum of the incoming data rate
and the dropped data rate.

.. figure:: media/BestEffortTrafficClass.png
   :align: center

.. figure:: media/datarate_be.png
   :align: center

The next diagram shows the operation of the per-stream filter for the video traffic
class. The outgoing data rate equals with the sum of the incoming data rate and
the dropped data rate.

.. figure:: media/VideoTrafficClass.png
   :align: center

.. figure:: media/datarate_vi.png
   :align: center

The next diagram shows the number of tokens in the token bucket for both streams.
The filled areas mean that the number of tokens changes quickly as packets pass
through. The data rate is at maximum when the line is near the minimum.

.. figure:: media/TokenBuckets.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding per-stream filter. The reason is that they
are measured at different protocol layers.

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.

