Token-Bucket-Based Policing
===========================

Goals
-----

.. In this example we demonstrate per-stream policing using chained token buckets
.. which allows specifying committed/excess information rates and burst sizes.

.. In this showcase, we demonstrate a token-bucket-based policing mechanism that
.. allows specifying committed information rates and burst sizes.

.. In this showcase, we demonstrate a token-bucket-based policing. We generate traffic in a simple network,
.. and use a token bucket mechanism that allows specifiying committed information rates and committed burst sizes to filter the packet stream.

In this showcase, we demonstrate per-stream policing using a token bucket mechanism that allows specifiying committed information rates and committed burst sizes. 
We generate traffic in a straightforward network, and employ this method to limit the data rate of the packet stream. Finally, we examine the traffic before and after policing.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

Overview of Token-Bucket-Based Policing
---------------------------------------

.. This document assumes that you're familiar with per-stream filtering and policing in INET.
.. For a description, see the TODO section.

This document assumes familiarity with per-stream filtering and policing in INET.
For a description, see the TODO section.

This document assumes familiarity with per-stream filtering and policing in INET, :ref:`detailed here <sh:tsn:filtering:statistical:filtering_in_inet>`.

.. See the TODO section for a description.

.. **description of token-bucket policing**

.. the modules and how it works

.. - the SingleRateTwoColorMeter contains one token bucket. it measures packets passing through it, and attaches a LabelTag to each, containing either 'green' or 'red',
..   depending on if enough tokens are available for transmitting that packet. also, the committed information rate and burst size can be configured
.. - by default, the :ned:`SimpleIeee8021qFilter` module has :ned:`LabelFilter` submodules, and is configured to pass green packets and drop the rest

To implement the token-bucket-based policing, we use :ned:`SingleRateTwoColorMeter` and :ned:`LabelFilter` modules.

.. SingleRateTwoColorMeter contains one token bucket. The module measures packets passing through it, and attaches a LabelTag to each. The label tag contains 'green'
.. if enough tokens are available for transmitting that packet, and 'red' otherwise. Also, the committed information rate and burst size can be configured
.. - by default, the :ned:`SimpleIeee8021qFilter` module has :ned:`LabelFilter` submodules, and is configured to pass green packets and drop the rest

SingleRateTwoColorMeter contains one token bucket, and can be configured with/its main parameters are the committedInformationRate and committedBurstSize parameters. 
Based on these parameters, the module attaches a LabelTag to each packet passing through it. 
The label tag contains 'green' if enough tokens are available for transmitting that packet, and 'red' otherwise.

The :ned:`LabelFilter` module has a label parameter. Packets whose labels match this parameter are allowed through, the rest are dropped.
By default, the :ned:`SimpleIeee8021qFilter` module has :ned:`LabelFilter` submodules, and is configured to pass green packets and drop the rest
:ned:`LabelFilter` is the default filter submodule in :ned:`SimpleIeee8021qFilter`, configured to only allow 'green' packets to pass.

The following internal view of the :ned:`SimpleIeee8021qFilter` module in the switch illustrates this:

.. figure:: media/filter2.png
   :align: center

The Model
---------

.. Let's see the model...token bucket

.. For an overview of filtering in INET, check out the TODO section of the TODO showcase.

.. As described in the TODO showcase in more detail, filtering done in the bridging layer TODO

.. In this showcase, we'll use the TODO module for token bucket filtering.

..    so

..    - tsnswitch has ingresstrafficfiltering
..    - tsndevice has egress and ingress traffic filtering
..    - this adds a streamfilteringlayer to the network node
..    - the streamfilteringlayer has optional ingress and egress filtering submodules
..    - the proper ones are enabled according to the parameters
..    - by default the ingress/egress filtering submodule is a :ned:`SimpleIeee8021qFilter`,
..    that can do per-stream filtering. it has a configurable number of traffic streams,
..    and a meter, a filter and a gate for each. additionally, there is the default route
..    which unfiltered packets take by default.
..    - the meter can do metering
..    - the filter drops the packets (might be based on the metering)
..    - the gate is an interactive gate by default, so its always open
..    - example

..    - the classifier is a streamclassifier by default -> classify according to named streams
..    - token bucket filtering can be done with the SingleRateTwoColorMeter module
..    - this is a queueing element that has one token bucket
..    - can specify a committed information rate and a committed burst rate
..    - the rate of token regeneration defines the information rate
..    - the max number of tokens the burst size (if lots of tokens accumulate, the node can transmit faster/until they are depleted)
..    - the meter actually labels packets green or red, according to the number of tokens available (if there are no tokens, then red)
..    - the filter is a :ned:`LabelFilter` and it drops red packets
..    - note check out other meter modules (DualRateThreeColorMeter)
..    - the submodules in the :ned:`SimpleIeee8021qFilter` doesn't have to be the same type for the different traffic categories (e.g. check out the mixing shapers)

.. Token Bucket Policing Overview
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

..    - how we do token bucket policing/filtering
..    - SingleRateTwoColorMeter + the default :ned:`LabelFilter` (passes green by default)
..    - SingleRateTwoColorMeter has parameters for committedinformationrate and committedburstsize
..    - under the hood it uses tokens, and these two parameters actually configure token generation rate and max number of tokens
..    - so the meter puts a green or red tag based on if there is enough tokens and the filter drops excess packets

..    - token bucket filtering can be done with the SingleRateTwoColorMeter module
..    - this is a queueing element that has one token bucket
..    - can specify a committed information rate and a committed burst rate
..    - the rate of token regeneration defines the information rate
..    - the max number of tokens the burst size (if lots of tokens accumulate, the node can transmit faster/until they are depleted)
..    - the meter actually labels packets green or red, according to the number of tokens available (if there are no tokens, then red)
..    - the filter is a :ned:`LabelFilter` and it drops red packets
..    - note check out other meter modules (DualRateThreeColorMeter)

"under the hood it uses tokens, and these two parameters actually configure token generation rate and max number of tokens"

We can do token bucket filtering by using a :ned:`SingleRateTwoColorMeter` as the meter module, and :ned:`LabelFilter` that is the default filter in :ned:`SimpleIeee8021qFilter`.
The :ned:`SingleRateTwoColorMeter` is a packet meter queueing element that has one token bucket. It has :par:`committedInformationRate` and :par:`committedBurstSize`
parameters. Under the hood, the information rate parameter sets the token generation rate, the burst size the maximum number of tokens.
Packets in the traffic category can be trasmitted if there are tokens available; if there are lots of tokens, packets can be transmitted in a burst until
the tokens are depleted.

The meter measures the data rate of the traffic stream, and labels packets either `green` or `red`, depending on the
number of available tokens. By default, the filter module is a :ned:`LabelFilter`, and drops packets that aren't green.

.. note:: Other, more complex token-based meter modules are available (using multiple token buckets, for example), such as :ned:`SingleRateThreeColorMeter` or :ned:`DualRateThreeColorMeter`.
          For more information and the list of other meter modules, check the NED documentation.

.. note:: Other, more complex token-based meter modules are available (using multiple token buckets, for example), such as :ned:`SingleRateThreeColorMeter` or :ned:`DualRateThreeColorMeter`.

The Network
~~~~~~~~~~~

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels.

.. figure:: media/Network.png
   :align: center

The Configuration
~~~~~~~~~~~~~~~~~

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

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies over time for both traffic classes using a
sinusoid packet interval.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the operation of the per-stream filter for the best effort
traffic class. The outgoing data rate equals with the sum of the incoming data rate
and the dropped data rate.

.. .. figure:: media/BestEffortTrafficClass.png
..    :align: center

.. figure:: media/datarate_be.png
   :align: center

The next diagram shows the operation of the per-stream filter for the video traffic
class. The outgoing data rate equals with the sum of the incoming data rate and
the dropped data rate.

.. .. figure:: media/VideoTrafficClass.png
..    :align: center

.. figure:: media/datarate_vi.png
   :align: center

.. figure:: media/datarate_tokens_be.png
   :align: center

.. figure:: media/datarate_tokens_vi.png
   :align: center

.. The next diagram shows the number of tokens in the token bucket for both streams.
.. The filled areas mean that the number of tokens changes quickly as packets pass
.. through. The data rate is at maximum when the line is near the minimum.

.. .. figure:: media/TokenBuckets.png
..    :align: center

"The data rate is at maximum when the line is near the minimum."

.. The last diagram shows the data rate of the application level incoming traffic
.. in the server. The data rate is somewhat lower than the data rate of the
.. outgoing traffic of the corresponding per-stream filter. The reason is that they
.. are measured at different protocol layers.

.. .. figure:: media/ServerApplicationTraffic.png
..    :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.

