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

.. In this showcase, we demonstrate a simple statistical policing scenario. We
.. generate traffic in an example network, and measure its data rate with a sliding
.. window. Based on the measured data rate, we drop packets in a way that
.. the packet stream's data rate on average comforms to a specified limit.

This showcase demonstrates a simple statistical policing
scenario. We generate traffic within an example network and
employ a sliding time window to measure its data rate. By probabilistically dropping
packets based on the measured data rate, this mechanism ensures that the average data rate of
the packet stream doesn't exceed a specified limit.

.. **review** what window sliding over what?

.. We dynamically drop
.. packets based on the measured data rate to ensure that the average data rate of
.. the packet stream aligns with a specified limit.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/statistical>`__

.. The Model
.. ---------

**review** does this description belong here? and not one level up?

.. _sh:tsn:filtering:statistical:filtering_in_inet:

Per-Stream Filtering and Policing in INET
-----------------------------------------

.. Filtering and policing - we'll use the two terms interchangeably here - enforces
.. requirements for data rate and burst size in a packet stream, by dropping
.. excess packets. In INET, by default, filtering is done in the bridging layer
.. of TSN network nodes (although filtering components can also be inserted elsewhere
.. in network nodes).

.. Filtering and policing is defined in the TODO standard. In general, filtering
.. prevents certain types of packets from entering a network node. Policing refers
.. to enforcing data rate and bursting limits to protect upstream components and
.. devices in a network. In INET, filtering is used with somewhat different
.. meanings sometimes, as it generally refers to the filtering layer where
.. filtering and policing takes places. Also, policing involves dropping (or
.. filtering out) packets, also done in the filtering layer. So in this document,
.. filtering is used in a broader sense than in the standard.

.. **review** filtering AND policing -> interchangable? then why 'and'?

.. -> actually, in the standard its called filtering and policing, which means basically the same; policing by filtering?

.. - Filtering and policing in the standard: filters out packets. prevents certain
..   types of packets from entering the network for security. policing is limiting
..   the data rate and bursting in order not to overload other network components
.. - In INET, filtering and policing is done in the filtering layer. The filtering layer performs both. It can drop certain types of packets,
..   and can use queueing components for rate limiting traffic streams

.. Per-Stream Filtering and Policing is defined in the IEEE 802.1Qci standard. Filtering
.. prevents certain types of packets from entering the network. Policing refers
.. to enforcing limits for data rate and bursting to protect upstream components and
.. devices in the network. In INET, both filtering and policing is performed in the filtering layer, 
.. located in the bridging layer of TSN network nodes.

The IEEE 802.1Qci standard defines Per-Stream Filtering and
Policing. Filtering can blocking specific packet types from
entering the network, while Policing focuses on enforcing limits for data rate
and bursting to safeguard upstream components and network devices. Both mechanisms involve dropping invalid or excess/selected packets.
In INET, both filtering and policing operations are conducted in the
filtering layer, which is located in the bridging layer of TSN network nodes.

The IEEE 802.1Qci standard defines Per-Stream Filtering and
Policing. In order to safeguard upstream components and network devices, Filtering can blocking specific packet types from
entering the network, while Policing focuses on enforcing limits for data rate
and bursting. Both mechanisms involve dropping selected packets.
In INET, both filtering and policing operations are conducted in the
filtering layer, which is located in the bridging layer of TSN network nodes.

   Thus, in this document, we refer to filtering in general to mean that some selected packets are filtered out and dropped in the filtering layer,
   whether thats for filtering and policing.

   We use filtering in this documents as an umbrella term for filtering and policing operations, as we concentrate
   on how packets are selected and filtered out and dropped in the filtering layer.

   We use filtering in this documents as an umbrella term for filtering and policing operations, as both happen in the
   filtering layer of network nodes and involve filtering out and dropping certain packets.

   .. Since both filtering and policing is performed in the filtering layer in INET, we generally use filtering
   .. in this document to refer to both.

.. In INET, the filtering layer performs both filtering and policing. 
   It is located in the bridging layer of TSN network nodes.

.. In INET, both filtering and policing is done in the filtering
   layer, located in the bridging layer of network nodes.

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

.. To employ filtering and policing in TSN network nodes, this functionality first needs to be enabled.
.. Then, by selecting and configuring the appropriate filtering components, different filtering methods can be realized.

To employ filtering and policing, this functionality first needs to be enabled in TSN network nodes.
Then, by selecting and configuring the appropriate components, different filtering methods can be realized.

.. **review** enabled in them? in the nodes? no 'filtering' in appropriate filtering components

Filtering and policing can be enabled in TSN network nodes by setting their
:par:`ingressTrafficFiltering` and :par:`egressTrafficFiltering` parameters to ``true``. In
:ned:`TsnSwitch`, only ingress filtering is available (as typically only this direction is needed in switches), while :ned:`TsnDevice`
supports both ingress and egress filtering. When enabled in any direction, a
:ned:`StreamFilteringLayer` submodule is added to the node's bridging layer,
containing an ``ingressFilter`` and/or an ``egressFilter`` submodule. By
default, the ingress and egress filter type is :ned:`SimpleIeee8021qFilter`.

For context, here is a stream filtering layer module containing filter submodules for both directions:

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

:ned:`SimpleIeee8021qFilter` is a compound module that can handle a configurable number of
traffic streams, as specified by its :par:`numStreams` parameter. Each traffic
stream has a path that filters the stream independently of the other
streams. For example, here is an internal view of a :ned:`SimpleIeee8021qFilter` module with two
traffic streams:

.. figure:: media/SimpleIeee8021qFilter.png
   :align: center

.. Some notes:

.. these are some general notes on how the filtering works in general.

Let's examine the filtering process in this module:

- The `classifier` decides which traffic path incoming packets should take. By default, it's a :ned:`StreamClassifier` module, which classifies packets by stream name, contained in a packet tag.
- The `gate` module is an :ned:`InteractiveGate` that is always open by default, but can be opened/closed by user interaction.
- The `meter` module measures some property of the packet stream, and attaches this information to each packet using a packet tag.
- The `filter` module can use this information to decide which packets to drop.
- Depending on configuration, the classifier can send some packets through an `unfiltered path` between the classifier and the multiplexer. This unfiltered path is available by default,
  but can be turned off with the :par:`hasDefaultPath` parameter.

**review** how does the classifier use the default path? "can send packets that don't match any traffic class through the unfiltered direct path"

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
It's important to note that each traffic path can be configured independently, for different filtering behaviors.

.. can contain different modules./

.. .. note:: Traffic shapers, such as the Asynchronous Traffic Shaper, can have elements in the filtering layer.

.. note:: Although traffic shaping primarily happens in network interfaces, 
   some elements of traffic shapers might be located in the filtering layer. 
   For example, the Asynchronous Traffic Shaper's meter and filter components are located there.

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

.. The Model
.. ---------

Statistical Policing Overview
-----------------------------

.. "drops packets in a probabilistic way by comparing the measured datarate to the maximum allowed datarate."

In this showcase, we employ a basic statistical policing mechanism,
i.e., measure data rate of a packet stream, and drop packets in a probabilistic manner
to keep the filtered data rate below a specified maximum.
To implement this mechanism, we'll use the :ned:`SlidingWindowRateMeter` and the
:ned:`StatisticalRateLimiter` modules. 

.. In this showcase, we'll use the :ned:`SlidingWindowRateMeter` and the
.. :ned:`StatisticalRateLimiter` modules for basic statistical policing, i.e.,
.. the filtered data rate 

:ned:`SlidingWindowRateMeter` measures data rate with a sliding time window,
whose size is configured with the :par:`timeWindow` parameter.
The module sums up the packet bytes over the specified time window, and attaches a ``rateTag`` to each packet.

.. The :ned:`StatisticalRateLimiter` is a filter module that uses the rate tag to
.. drop packets in a probabilistic way, so that the average data rate doesn't
.. exceed the maximum data rate specified with the :par:`maxDataRate` parameter.

.. The :ned:`StatisticalRateLimiter` is a filter module that uses its
.. :par:`maxDataRate` parameter, and the rate tag on packets to calculate a packet
.. drop probability. It drops certain packets based on this probability,
.. guaranteeing that on average, filtered data rate stays below the specified maximum.

.. The :ned:`StatisticalRateLimiter` is a filter module that drops packets in a probabilistic way by comparing the measured datarate to the maximum allowed datarate.

.. The :ned:`StatisticalRateLimiter` is a filter module that calculates a packet drop probability by comparing the measured datarate to the maximum allowed datarate.
.. Drop probability depends on how excessive the data rate gets.

:ned:`StatisticalRateLimiter` is a filter module that calculates a packet drop probability by comparing the measured datarate (contained in rate tags) to the maximum allowed datarate (specified by the module's :par:`maxDataRate` parameter).
Packet drop probability increases depending how much how much the measured data rate exceeds the maximum.

**review** is it clear that these are the meter and filter submodules in the SimpleIeee8021qFilter?

.. It drops packets with a probability increasing with the measured datarate. It drops packets based on how much the measured data rate exceeds the maximum.

The precision of the policing process relies on the size of the time window, which determines how quickly the filter responds 
to fluctuations in the incoming data rate and how closely the filtered outgoing data rate matches the configured maximum.

.. It calculates packet drop probability based on the maximum data rate specified with the :par:`maxDataRate` parameter.

.. so

.. - specifiy a max data rate
.. - the filter calculates a packet drop probability based on the max data rate and the rate tag
.. - and drops certain packets (By applying this probability, certain packets are selectively dropped.)
.. - the result is that the filtered outgoing data rate stays below the specified limit on average

.. **review** statistically, the components limit the data rate on average. how does that work? it calculates a probability
.. that a packet should be dropped? so that the outgoing rate matches the configured limit.
.. record a statistic (data rate), calculate a packet drop probability, and drop certain packets.

.. .. note:: This setup performs straightforward statistical filtering by utilizing a sliding window to measure the data rate. 
..    The precision of the filtering process relies on the size of the time window, which determines how quickly the filter responds 
..    to fluctuations in the incoming data rate and how closely the filtered outgoing data rate matches the configured maximum.

The Configuration
-----------------

.. In the next sections, we describe the network, and examine the configuration of traffic, stream identification and encoding, and filtering.

.. -> how this, but understandable.

.. We use a simple network in which a client is connected to a server via a switch, using 100Mbps Ethernet links. The client generates traffic, and sends it to the server.
.. The traffic undergoes filtering in the switch. We examine the traffic before and after filtering/how filtering changes the traffic.
.. Here is the network:

.. We use the following simple network, in which a client is connected to a server via a switch, using 100Mbps Ethernet links:

The network contains three hosts: the ``client`` and the ``server`` (both :ned:`TsnDevice` modules), connected via the ``switch`` (:ned:`TsnSwitch`) with
100 Mbps Ethernet links. The client generates traffic, and the filtering layer in the switch limits this traffic to a nominal rate by dropping excessive packets.
We plot the data rate in the client and the server to observe the effects of policing. Here is the network:

.. The network contains three hosts. A client is connected to a server via a switch, using 100 Mbps Ethernet links.
.. Both the client and the server are :ned:`TsnDevice` modules, the switch's type is :ned:`TsnSwitch`. 
.. The client generates traffic, and the filtering layer in the switch limits this traffic to a nominal rate by dropping excessive packets.
.. We plot the data rate in the client and the server to observe the effects of policing. Here is the network:

.. The :ned:`StatisticalRateLimiter` is a filter module. We can specify a maximum data rate with the :par:`maxDataRate` parameter. The module
   drops packets that would exceed this limit.

.. - how we do statistical filtering
   - the slidingwindowratemeter can do that. and the StatisticalRateDropper
   - the slidingwindowratemeter measures the datarate and packetrate of packets passing through in configurable time window (10ms by default).
   it attaches datarate and packetrate tags to packets.
   - the StatisticalRateDropper have a maxDataRate parameter and it drops excessive packets based on the attached tag

.. This is a simple statistical filtering, because the data rate is measured with a sliding window. the precision depends on the timewindow size
.. -> like how fast the filter reacts to changes in incoming data rate? and how precisely the outgoing datarate sticks to the configured maximum. TODO

.. .. note:: This setup does simple statistical filtering, because the data rate is measured with a sliding window. The precision depends on the size of the time window, i.e. how fast the filter reacts to changes in incoming data rate, and how precisely the outgoing data rate aligns with the configured maximum.

.. The Network
.. ~~~~~~~~~~~

.. Here is the network:

.. The simulation uses the :ned:`TsnLinearNetwork`, with two :ned:`TsnDevice` modules connected by a :ned:`TsnSwitch`, using 100Mbps Ethernet links:

.. figure:: media/Network.png
   :align: center
   :width: 100%

.. The Configuration
.. ~~~~~~~~~~~~~~~~~

   .. In this configuration we use a sliding window rate meter in combination with a
   .. statistical rate limiter. The former measures the thruput by summing up the
   .. packet bytes over the time window, the latter drops packets in a probabilistic **(?)**
   .. way by comparing the measured datarate to the maximum allowed datarate.

.. Here is the configuration:

   .. literalinclude:: ../omnetpp.ini
      :language: ini

.. The client generates two traffic streams with sinusoidally changing data rates.
.. Then, the filtering layer in the switch limits this traffic to a nominal rate by dropping excessive packets.

.. The client generates traffic, and the filtering layer in the switch limits this traffic to a nominal rate by dropping excessive packets.

We take a look at the configuration in the following sections.

.. The client is configured to generate two traffic streams

Traffic
+++++++

The client is configured to generate two traffic streams with sinusoidally changing data rates, with a mean of 40 and 20 Mbps:

.. The best effort and video data streams have a sinusoidally changing data rate with a mean of 40 and 20 Mbps, respectively:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: client
   :end-before: enable outgoing streams

Stream Identification, Encoding and Decoding
++++++++++++++++++++++++++++++++++++++++++++

.. We enable outgoing streams in the client (this adds a :ned:`StreamIdentifierLayer`), and assign packets to the `best effort` and `video` named streams based on
.. destination port. The stream coder encodes the streams with PCP numbers. We configure the switch similarly to decode the named stream based on PCP. Here is the relevant
.. configuration:

.. The two streams have two different traffic classes: best effort and video. The
.. bridging layer in the client identifies the outgoing packets by their UDP destination port.
.. The client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
.. field:

We assign the two streams to two different traffic classes: best effort and video. To this end, we configure the
bridging layer in the client to identify outgoing packets by their UDP destination port.
Then, the client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
field:

..    Within the client, our goal is to classify packets originating from the two
..    packet sources into two traffic classes: best effort and video. To achieve this,
..    we activate IEEE 802.1 stream identification and stream encoding functionalities
..    by setting the hasOutgoingStreams parameter in the switch to true. We proceed by
..    configuring the stream identifier module within the bridging layer; this module
..    is responsible for associating outgoing packets with named streams based on
..    their UDP destination ports. Following this, the stream encoder sets the
..    Priority Code Point (PCP) number on the packets according to the assigned stream
..    name (using the IEEE 802.1Q header’s PCP field):

.. **review** more understandable?

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable outgoing streams
   :end-before: enable per-stream filtering

Per-Stream Filtering
++++++++++++++++++++

.. We enable ingress per-stream filtering in the switch. As mentioned before, this setting adds a :ned:`StreamFilteringLayer` to the bridging layer of the switch,
.. with a :ned:`SimpleIeee8021qFilter` submodule as the ingress filter. We set the number of traffic streams to 2; the default path is enabled by default, though
.. it won't be used in this scenario, as all packets are part of either the best effort or video stream. We configure the priority of the streams in the classifier.
.. We set the type of both meter submodules to :ned:`SlidingWindowRateMeter`, and configure the time window to be 10ms. We specify the maximum
.. data rates. The data rates of the incoming traffic are sinusoids with a mean of 40 and 20 Mbps, thus they fluctuate around the mean. The filter limits the data rate to the mean value:

.. .. literalinclude:: ../omnetpp.ini
..    :language: ini
..    :start-at: enable per-stream filtering

To configure the statistical policing, we first enable ingress per-stream filtering in the switch. This setting adds a :ned:`SimpleIeee8021qFilter` module to the switch's bridging layer:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable per-stream filtering
   :end-before: per-stream filtering

Then, we set the number of streams to 2, for our best effort and video classes. The classifier is set to prioritize the video class **review** is it?:

**review** also, shouldn't index 0 be the default path, making the best effort stream unfiltered? or if there is a default path, its index is -1?

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # per-stream filtering
   :end-before: meter[0].display-name

In the meter module, we configure the type to be :ned:`SlidingWindowRateMeter`, and set the sliding time window size:

**review** (and also the display name, so we can see which traffic path belongs to which traffic class)

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: meter[0].display-name
   :end-at: timeWindow

Finally, the filter module type is set to :ned:`StatisticalRateLimiter`. We configure the rate limiter to maximize data rate at 40 and 20 Mbps for the two traffic classes, respectively:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: StatisticalRateLimiter

Results
-------

Let's examine the results. The following chart shows the data rates for the two traffic streams in the client:

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The following two charts show the incoming, outgoing (filtered), and dropped data rates in the switch, for the two traffic categories:

.. figure:: media/datarate_be.png
   :align: center

.. figure:: media/datarate_vi.png
   :align: center

.. figure:: media/datarate_be2.png
   :align: center

.. figure:: media/datarate_vi2.png
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

