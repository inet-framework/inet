Visualizing Packet Drops
========================

Goals
-----

This showcase presents various simulation models that demonstrate the use of the
packet drop visualizer in identifying network problems in INET simulations. The
visualizer helps to pinpoint the causes of excessive packet drops, such as poor
connectivity, congestion, or misconfiguration, allowing for more efficient
debugging and analysis. The simulation models showcase typical scenarios of
packet drops resulting from issues such as retransmission in wireless networks
and overflowing queues in bottleneck routers.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/packetdrop <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/packetdrop>`__

About the visualizer
--------------------

Packet drops can be visualized by including a :ned:`PacketDropVisualizer`
module in the simulation. The :ned:`PacketDropVisualizer` module indicates
packet drops by displaying an animation effect at the node where the
packet drop occurs. In the animation, a packet icon gets thrown out from
the node icon and fades away.

The visualization of packet drops can be enabled with the visualizer's
:par:`displayPacketDrops` parameter. By default, all packet drops at all nodes
are visualized. This selection can be narrowed with the :par:`nodeFilter`,
:par:`interfaceFilter` and :par:`packetFilter` parameters, which match for node, interface and packet.
(The :par:`packetFilter` can filter for packet properties, such as name, fields, chunks, protocol headers, etc.)
Additionally, and the :par:`detailsFilter` parameter to filter for packet drop reason.

The :par:`packetFormat` parameter is a format string and specifies the text displayed with the packet drop animation.
By default, the dropped packet's name is displayed.
The format string can contain the following directives:

- `%n`: packet name
- `%c`: packet class
- `%r`: drop reason

Packets can be dropped for the following reasons, for example:

.. For example, some of the reasons packets are dropped for are the following:

-  queue overflow
-  retry limit exceeded
-  unroutable packet
-  network address resolution failed
-  interface down

One can click on the packet drop icon to display information about the
packet drop in the inspector panel, such as the packet drop reason,
or the module where the packet was dropped:

.. figure:: media/inspector2.png
   :width: 70%
   :align: center

.. todo::

   The color of the icon indicates the reason for the packet drop
   There is a list of reasons, identified by a number

The following sections demonstrate some reasons for dropped packets, with example simulations.
In the simulations, the :ned:`PacketDropVisualizer` is configured to indicate the packet name and the drop reason.

Queue overflow
--------------

In this section, we present an example for demonstrating packet drops due
to queue overflow. This simulation can be run by choosing the
``QueueFull`` configuration from the ini file. The network contains a
bottleneck link where packets will be dropped due to an overflowing
queue:

.. figure:: media/queuefullnetwork.png
   :width: 80%
   :align: center

It contains a :ned:`StandardHost` named ``source``, an :ned:`EthernetSwitch`, a
:ned:`Router`, an :ned:`AccessPoint`, and a :ned:`WirelessHost` named
``destination``. The ``source`` is configured to send a stream of UDP
packets to ``destination``. The packet stream starts at two seconds
after ``destination`` got associated with the access point. The
``source`` is connected to the ``etherSwitch`` via a high speed, 100
Gbit/s ethernet cable, while the ``etherSwitch`` and the ``router`` are
connected with a low speed, 10 MBit/s cable. This low speed cable creates a bottleneck
in the network, between the switch and the router. The source host is
configured to generate more UDP traffic than the 10Mbit/s channel can
carry. The cause of packet drops, in this case, is that the queue in
``etherSwitch`` fills up.

The queue types in the switch's Ethernet interfaces are set to
:ned:`DropTailQueue`, with a default length of 100 packets (by default, the
queues have infinite lengths). The packets are dropped at the ethernet
queue of the switch.

The visualization is activated with the ``displayPacketDrops``
parameter. The visualizer is configured to display the packet name
and the drop reason, by setting the :par:`labelFormat` parameter.
Also, the fade-out time is set to three seconds, so that the packet
drop animation is more visible.

.. literalinclude:: ../omnetpp.ini
   :start-at: displayPacketDrops
   :end-at: fadeOutTime
   :language: ini

When the simulation is run, the UDP stream starts at around two seconds,
and packets start accumulating in the queue of the switch. When the
queue fills up, the switch starts dropping packets. This scenario is illustrated
in this animation:

.. video:: media/queueoverflow1.mp4
   :width: 628

Here is the queue in the switch's eth1 interface, showing the number of
packet drops:

.. figure:: media/ethqueue.png
   :align: center

This log excerpt shows the packet drop:

.. figure:: media/log_queuefull_3.png


ARP resolution failed
---------------------

In this example, a host tries to ping a non-existent destination. The
configuration for this example is ``ArpResolutionFailed`` in the ini
file. Packets will be dropped because the MAC address of the destination
cannot be resolved. The network for this configuration is the following:

.. figure:: media/arpfailednetwork.png
   :align: center

It contains only one host, an :ned:`AdhocHost`.

The host is configured to ping the IP address 10.0.0.2. It will try to
resolve the destination's MAC address with ARP. Since there are no other
hosts, the ARP resolution will fail, and the ping packets will be
dropped.

The following animation illustrates this:

.. video:: media/arp1.mp4
   :width: 366

This excerpt shows this in the log:

.. figure:: media/log_arpfailed_2.png
   :width: 100%

MAC retry limit reached
-----------------------

In this example, packet drops occur due to two wireless hosts trying to
communicate while out of communication range. The simulation can be run
by selecting the ``MACRetryLimitReached`` configuration from the ini
file. The configuration uses the following network:

.. figure:: media/maclimitnetwork.png
   :align: center

It contains two :ned:`AdhocHost`'s, named ``source`` and ``destination``.
The hosts' communication ranges are set up so they are out of range of
each other. The source host is configured to ping the destination host.
The reason for packet drops, in this case, is that the hosts are not in
range, thus they can't reach each other. The ``source`` transmits the
ping packets, but it doesn't receive any ACK in reply. The ``source's``
MAC module drops the packets after the retry limit has been reached.
This scenario is illustrated in the following animation:

.. video:: media/retry.mp4
   :width: 512

These events looks like the following in the logs:

.. figure:: media/log_macretrylimit_2.png
   :width: 100%

No route to destination
-----------------------

In this example, packets will be dropped due to the lack of static
routes. The configuration is ``NoRouteToDestination`` in the ini file.
The network is the following:

.. figure:: media/noroutenetwork.png
   :width: 60%
   :align: center

It contains two connected :ned:`StandardHost`'s. The
:ned:`Ipv4NetworkConfigurator` is instructed not to add any static routes,
and ``host1`` is configured to ping ``host2``.

The ping packets can't be routed, thus the IP module drops them. This scenario is
illustrated in the following video:

.. video:: media/noroute.mp4
   :width: 416

Here is also a log excerpt illustrating this:

.. figure:: media/log_noroute_2.png

Interface not connected
-----------------------

In this example (``InterfaceNotConnected`` configuration in the ini
file), packet drops occur due to a disabled wired connection between the
hosts:

.. figure:: media/unroutablenetwork.png
   :align: center

It contains two :ned:`StandardHost`'s, connected with an ethernet cable. The
ethernet cable is configured in the NED file to be disabled.
Additionally, ``host1`` is configured to ping ``host2``.

Since the cable between the hosts is configured to be disabled, the MAC
module is unable to send the packets and drops them. This is
illustrated in the following animation:

.. video:: media/notconnected.mp4
   :width: 414

Note the packet drop animation at ``host1``. The packet drops are also
evidenced in the log:

.. figure:: media/log_notconnected_2.png
   :width: 100%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PacketDropVisualizationShowcase.ned <../PacketDropVisualizationShowcase.ned>`

Further information
-------------------

For more information, refer to the :ned:`PacketDropVisualizer` NED
documentation.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/1>`__ in
the GitHub issue tracker for commenting on this showcase.
