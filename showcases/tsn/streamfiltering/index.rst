Per-Stream Filtering and Policing
=================================

Stream filtering involves dropping packets from traffic streams when they exceed specifications for data rate, burstiness
or delay, for example. Per-stream filtering and policing provides protection against bandwidth violation,
malfunctioning devices, network attacks, etc. Filtering and policing decisions are
made on a per-stream, per-priority, per-frame, etc. basis using various metering
methods. In INET, filtering and policing is performed in the bridging layer of network nodes (as opposed to traffic shaping,
which takes place in network interfaces).

.. Filtering and policing is performed by layer 2. In INET, 

.. **TODO** this might not be needed -> high level overview only In INET, filtering and policing is performed in the bridging layer of network nodes.
   By default, :ned:`EthernetSwitch` has a simple briding layer module that only does forwarding of frames.
   Filtering and policing requires the modular briding layer module (:ned:`BridgingLayer`), which
   the TSN-specific extension of :ned:`EthernetSwitch`, the :ned:`TsnSwitch` module, has by default.
   Then, filtering and policing can be enabled in the switch with the :par:`hasIngressTrafficFiltering` parameter.

   .. This adds a :ned:`StreamFilterLayer` module to the briding layer.

   .. In an Ethernet LAN, network devices are connected on L2 by network bridges. In INET, The bridges are basically Ethernet switches,
      and the bridging is done in the switch's bridging layer. The bridging layer forwards packets in the LAN on L2, in constrast
      to L3 (IP) routing, which routes packets between LANs.

      Packets arrive on the switch's incoming interface, and are sent up the protocol stack to the bridging layer.
      The bridging layer makes the decision as to which interface to forward the packet on. Then the packet goes down the
      protocol stack to the outgoing interface.

      **TODO** this, less simplistically

      The bridging layer can include various functionality, such as filtering and policing. **TODO** what else?

   - streamfilterlayer
   - by default simpleieee8021qfilter module
   - can add stuff like filters and meters

   -> should this be here, or in the showcases? should this be the same in the trafficshaping?

   -> also, should the order be underthehood-tokenbucket-statistical?

   -> the category page should contain just a high level overview of filtering vs traffic shaping

The following showcases demonstrate per-stream filtering and policing:

.. toctree::
   :maxdepth: 1

   statistical/doc/index
   tokenbucket/doc/index
   underthehood/doc/index

