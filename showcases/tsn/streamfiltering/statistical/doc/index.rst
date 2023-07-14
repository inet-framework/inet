Statistical Policing
====================

Goals
-----

.. In this example, we combine a sliding window rate meter with a probabilistic packet
.. dropper to achieve a simple statistical policing.

.. what does that mean? the sliding window rate meter measures data rate with a sliding window, and puts the data rate on packets in a tag.
.. the probabilistic packet dropper has a parameter to limit the data rate of packets going through it, and, based on the data rate tag,
.. randomly drops packets so that the outgoing data rate complies with the configured limit. how is this statistical? statitically, the
.. outgoing data rate will comply? what does that mean? on average, it will comply.

.. In this showcase, we demonstrate a simple statistical policing scenario. We generate traffic in an example network,
.. measure its data rate with a sliding window, and based on the data rate, we use a statisticalratelimiter
.. to drop packets in a probabilistic way, so that rate limiter's outgoing data rate comforms to a specified limit.

.. In this showcase, we demonstrate a simple statistical policing scenario. We
.. generate traffic in an example network, and measure its data rate with a sliding
.. window. Based on the measured data rate, we drop packets in a probabilistic way, so
.. that the packet stream's data rate comforms to a specified limit.

In this showcase, we demonstrate a simple statistical policing scenario. We
generate traffic in an example network, and measure its data rate with a sliding
window. Based on the measured data rate, we drop packets in a way that
the packet stream's data rate on average comforms to a specified limit.

This showcase demonstrates a simple statistical policing
scenario. We generate traffic within an example network and
employ a sliding window to measure its data rate. By dynamically dropping
packets based on the measured data rate, we ensure that the average data rate of
the packet stream aligns with a specified limit.

.. We dynamically drop
.. packets based on the measured data rate to ensure that the average data rate of
.. the packet stream aligns with a specified limit.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/statistical>`__

.. The Model
.. ---------

Filtering and Policing in INET
------------------------------

Filtering and policing - we'll use the two terms interchangably here - enforces
requirements for data rate and burst size in a packet stream, by dropping
excessive packets. In INET, by default, filtering is done in the bridging layer
of TSN network nodes (although filtering components can also be inserted elsewhere
in the network stack). network stack? wdym?

.. note:: Filtering components don't even require a complete network to function, as demonstrated in the the :doc:`/showcases/tsn/streamfiltering/underthehood/doc/index` showcase.

.. .. note:: Filtering components can even work outside of network nodes, as demonstrated in the the :doc:`/showcases/tsn/streamfiltering/underthehood/doc/index` showcase.

.. .. note:: Filtering modules can work on their own, i.e., they don't require a complete network to function.

   is this redundant?

   also, the underthehood should be a note later

.. In INET, TSN network nodes, such as TsnDevice or TsnSwitch, have boolean parameters that enable ingress and egress filtering (similarly to parameters
.. that enable ingress or egress traffic shaping). For example, TsnDevice has :par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering` parameters,
.. TsnSwitch has :par:`ingressTrafficFiltering` (check the NED documentation for TSN network nodes for the available parameters).
.. Setting any of the boolean traffic filtering parameters adds a StreamFilteringLayer to the network node's bridging layer. It also adds an ingressFilter
.. or egressFilter submodule (or both, depending on the direction) that has the type SimpleIeee8021qFilter. 

.. In INET, TSN network nodes, such as :ned:`TsnDevice` or :ned:`TsnSwitch`, have boolean parameters that enable ingress and egress filtering. 
.. For example, TsnDevice has :par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering` parameters, and
.. :ned:`TsnSwitch` has :par:`ingressTrafficFiltering`.
.. Enabling filtering adds a :ned:`StreamFilteringLayer` to the network node's bridging layer. It also adds an ``ingressFilter``
.. or ``egressFilter`` submodule (or both, depending on the direction) that has the type :ned:`SimpleIeee8021qFilter`. 

.. To employ filtering and policing, the feature first needs to be enabled in TSN network nodes.
.. Then, by selecting and configuring the appropriate filtering components, different filtering methods can be realized.

.. The filtering method can be specified by selecting and configuring the appropriate filtering components.

.. TsnDevice has both directions, TsnSwitch only has ingress filtering

.. so in general, ingress and egress filtering can be enabled independently in Tsn network nodes.
.. Note that not all have both capabilities

.. tsnswithc only has ingress filtering, because typically that direction us used in that module
.. however, tsnDevice has both.

.. To enable filtering and policing in TSN network nodes, the feature can be
.. activated by configuring the ingressTrafficFiltering and egressTrafficFiltering
.. parameters. In TsnSwitch, only ingress filtering is available, while TsnDevice
.. support both ingress and egress filtering. When any enabled in any direction, a StreamFilteringLayer
.. submodule is added to the node's bridging layer, containing an ingress and/or
.. egress filter submodule. By default, the ingress and egress filter type is SimpleIeee8021qFilter.

.. To employ filtering and policing in TSN network nodes, the feature first needs to be enabled.
.. Then, by selecting and configuring the appropriate filtering components, different filtering methods can be realized.

.. Filtering and policing can be enabled in TSN network nodes with their :par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering`
.. parameters. In :ned:`TsnSwitch`, only ingress filtering is available, while :ned:`TsnDevice`
.. support both ingress and egress filtering. When enabled in any direction, a :ned:`StreamFilteringLayer`
.. submodule is added to the node's bridging layer, containing an ingress and/or
.. egress filter submodule. By default, the ingress and egress filter type is :ned:`SimpleIeee8021qFilter`.

To employ filtering and policing in TSN network nodes, the feature first needs to be enabled.
Then, by selecting and configuring the appropriate filtering components, different filtering methods can be realized.

Filtering and policing can be enabled in TSN network nodes by setting their
:par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering` parameters to ``true``. In
:ned:`TsnSwitch`, only ingress filtering is available (as typically this direction is needed in switches), while :ned:`TsnDevice`
supports both ingress and egress filtering. When enabled in any direction, a
:ned:`StreamFilteringLayer` submodule is added to the node's bridging layer,
containing an ``ingressFilter`` and/or an ``egressFilter`` submodule. By
default, the ingress and egress filter type is :ned:`SimpleIeee8021qFilter`.

For context, here is a stream filtering layer module containing :ned:`SimpleIeee8021qFilter` submodules for both directions:

.. figure:: media/both_directions.png
   :align: center

.. In TSN network nodes (such as :ned:`TsnDevice` or :ned:`TsnSwitch`), ingress and
.. egress filtering can be enabled independently with the
.. :par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering` parameters
.. (both directions are off by default). When filtering is enabled in any
.. direction, a :ned:`StreamFilteringLayer` submodule is added into the bridging
.. layer of the node. Then, depending on these parameters, the filtering layer will contain an ingress
.. and/or egress filter submodule, a :ned:`SimpleIeee8021qFilter` by default.
.. This module can employ different filtering and policing methods, depending on its submodules and their settings. REWRITE

   draw a dependency graph

.. This module can handle a configurable
.. number of traffic streams (numTrafficStreams). contains TODO:

.. This module has a configurable number of paths, each of which can handle the metering and filtering of a traffic stream independently of the other streams.

The :ned:`SimpleIeee8021qFilter` is a compound module that can handle a configurable number of
traffic streams (specified by its :par:`numStreams` parameter). Each traffic
stream has a path that filters the stream independently of the other
streams. For example, here is an internal view of a :ned:`SimpleIeee8021qFilter` module with two
traffic streams:

.. figure:: media/SimpleIeee8021qFilter.png
   :align: center

.. Some notes:

.. these are some general notes on how the filtering works in general.

Let's examine the filtering process in general in a :ned:`SimpleIeee8021qFilter` module:

- The `classifier` decides which traffic path incoming packets should take. By default, it's a :ned:`StreamClassifier` module, which classifies packets by stream name contained in a packet tag.
- The `gate` module is an :ned:`InteractiveGate` that is always open by default, but can be opened/closed by user interaction.
- The `meter` module measures some property of the packet stream, and attaches this information to each packet by using a packet tag
- The `filter` module can use this information to decide which packets to drop
- Depending on configuration, the classifier can send some packets through an `unfiltered path` between the classifier and the multiplexer. This unfiltered path is available by default,
  but can be turned off with :ned:`SimpleIeee8021qFilter`'s :par:`hasDefaultPath` parameter.

.. - By default, the packets for the different streams are classified by stream name by a :ned:`StreamClassifier` module.
.. - The module meter submodule adds a tag based on the metering to the packet. The filter module
..   then can use this information to drop excessive packets.
.. - By default, the classifier is a :ned:`StreamClassifier` module, which classifies packets by stream name. WHAT DOES THAT MEAN? they have a stream name tag?
.. and adds a tag to each packet./
.. to drop certain packets./
.. - The direct, unfiltered path between the classifier and the multiplexer is available by default, but can be turned off with the :par:`hasDefaultPath`.
.. - Some packets can pass through the :ned:`SimpleIeee8021qFilter` module without undergoing any filtering. The classifier can send some packets via the unfiltered path
.. between the classifier and the multiplexer.

By overriding the type of the meter and filter submodules in :ned:`SimpleIeee8021qFilter`, specific filtering behaviors can be achieved. 
It's important to note that each traffic path can be configured independently for different filtering behaviors.

.. can contain different modules./

.. .. note:: Traffic shapers, such as the Asynchronous Traffic Shaper, can have elements in the filtering layer.

.. note:: Although traffic shaping primarily happens in network interfaces, some elements of traffic shapers might be located in the filtering layer. For example, the Asynchronous Traffic Shaper has its meter and filter components there.

   .. so

   .. - tsnswitch has ingresstrafficfiltering
   .. - tsndevice has egress and ingress traffic filtering
   .. - this adds a streamfilteringlayer to the network node
   .. - the streamfilteringlayer has optional ingress and egress filtering submodules
   .. - the proper ones are enabled according to the parameters
   .. - by default the ingress/egress filtering submodule is a SimpleIeee8021qFilter,
   .. that can do per-stream filtering. it has a configurable number of traffic streams,
   .. and a meter, a filter and a gate for each. additionally, there is the default route
   .. which unfiltered packets take by default.
   .. - the meter can do metering
   .. - the filter drops the packets (might be based on the metering)
   .. - the gate is an interactive gate by default, so its always open
   .. - example

   .. - the classifier is a streamclassifier by default -> classify according to named streams
   .. - token bucket filtering can be done with the SingleRateTwoColorMeter module
   .. - this is a queueing element that has one token bucket
   .. - can specify a committed information rate and a committed burst rate
   .. - the rate of token regeneration defines the information rate
   .. - the max number of tokens the burst size (if lots of tokens accumulate, the node can transmit faster/until they are depleted)
   .. - the meter actually labels packets green or red, according to the number of tokens available (if there are no tokens, then red)
   .. - the filter is a labelfilter and it drops red packets
   .. - note check out other meter modules (DualRateThreeColorMeter)can contain different modules./
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Model
---------

In this showcase, we'll use the :ned:`SlidingWindowRateMeter` and the :ned:`StatisticalRateLimiter` modules for basic statistical filtering.
The :ned:`SlidingWindowRateMeter` module measures data rate by summing up the
packet bytes over the specified time window, and attaches a ``rateTag`` to each packet.
The :ned:`StatisticalRateLimiter` is a filter module that uses the rate tag to drop packets in a probabilistic way, so that the 
average data rate doesn't exceed the maximum data rate specified with the :par:`maxDataRate` parameter.

.. The :ned:`StatisticalRateLimiter` is a filter module. We can specify a maximum data rate with the :par:`maxDataRate` parameter. The module
   drops packets that would exceed this limit.

.. - how we do statistical filtering
   - the slidingwindowratemeter can do that. and the StatisticalRateDropper
   - the slidingwindowratemeter measures the datarate and packetrate of packets passing through in configurable time window (10ms by default).
   it attaches datarate and packetrate tags to packets.
   - the StatisticalRateDropper have a maxDataRate parameter and it drops excessive packets based on the attached tag

.. This is a simple statistical filtering, because the data rate is measured with a sliding window. the precision depends on the timewindow size
.. -> like how fast the filter reacts to changes in incoming data rate? and how precisely the outgoing datarate sticks to the configured maximum. TODO

.. note:: This setup does simple statistical filtering, because the data rate is measured with a sliding window. The precision depends on the size of the time window, i.e. how fast the filter reacts to changes in incoming data rate, and how precisely the outgoing data rate aligns with the configured maximum.

The Network
~~~~~~~~~~~

.. Here is the network:

The simulation uses the :ned:`TsnLinearNetwork`, with two :ned:`TsnDevice` modules connected by a :ned:`TsnSwitch`, using 100Mbps Ethernet links:

.. figure:: media/Network.png
   :align: center
   :width: 100%

The Configuration
~~~~~~~~~~~~~~~~~

   .. In this configuration we use a sliding window rate meter in combination with a
   .. statistical rate limiter. The former measures the thruput by summing up the
   .. packet bytes over the time window, the latter drops packets in a probabilistic **(?)**
   .. way by comparing the measured datarate to the maximum allowed datarate.

.. Here is the configuration:

   .. literalinclude:: ../omnetpp.ini
      :language: ini

In this simulation, the client generates two traffic streams with sinusoidally changing data rates.
The filtering layer in the switch limits this traffic to a nominal rate by dropping excessive packets.

Traffic
+++++++

The source is configured to generate two traffic streams (we classify them to the best effort and video traffic classes in the next section, based on destination port).
The best effort and video data streams have a sinusoidally changing data rate with a mean of 40 and 20 Mbps, respectively:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: client
   :end-before: enable outgoing streams

Stream Identification, Encoding and Decoding
++++++++++++++++++++++++++++++++++++++++++++

We enable outgoing streams in the client (this adds a :ned:`StreamIdentifierLayer`), and assign packets to the `best effort` and `video` named streams based on
destination port. The stream coder encodes the streams with PCP numbers. We configure the switch similarly to decode the named stream based on PCP. Here is the relevant
configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable outgoing streams
   :end-before: enable per-stream filtering

Per-Stream Filtering
++++++++++++++++++++

We enable ingress per-stream filtering in the switch. As mentioned before, this setting adds a :ned:`StreamFilteringLayer` to the bridging layer of the switch,
with a :ned:`SimpleIeee8021qFilter` submodule as the ingress filter. We set the number of traffic streams to 2; the default path is enabled by default, though
it won't be used in this scenario, as all packets are part of either the best effort or video stream. We configure the priority of the streams in the classifier.
We set the type of both meter submodules to :ned:`SlidingWindowRateMeter`, and configure the time window to be 10ms. We specify the maximum
data rates. The data rates of the incoming traffic are sinusoids with a mean of 40 and 20 Mbps, thus they fluctuate around the mean. The filter limits the data rate to the mean value:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable per-stream filtering

Results
-------

Let's examine the results. The following chart shows the data rates for the two traffic streams in the source:

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The following two charts show the incoming, outgoing (filtered), and dropped data rates, for the two traffic categories:

.. figure:: media/datarate_be.png
   :align: center

.. figure:: media/datarate_vi.png
   :align: center

The filter limits the data rate to the configured value. Note that the filtered data rate is less than the incoming rougly with the amount of the dropped data rate.

The next chart displays the incoming and outgoing data rate for both traffic categories for an overview:

.. **TODO** this might not be needed cos its the same

.. figure:: media/source_sink.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/794>`__ page in the GitHub issue tracker for commenting on this showcase.

