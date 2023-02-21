Visualizing Transport Path Activity
===================================

Goals
-----

INET offers a range of network traffic visualizers that operate at different
levels of the network stack. In this showcase, we will
focus on :ned:`TransportRouteVisualizer` that provides graphical representation of
transport layer traffic between two endpoints by displaying polyline arrow along
the path that fades as the traffic ceases.

This showcase contains two simulation models, each highlighting various aspects
of the transport path activity visualizer, allowing for a comprehensive
understanding of its features.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/transportpathactivity <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/transportpathactivity>`__

About the Visualizer
--------------------

In INET, transport path activity can be visualized by including a
:ned:`TransportRouteVisualizer` module in the simulation. Adding an
:ned:`IntegratedVisualizer` is also an option because it also contains a
:ned:`TransportRouteVisualizer`. Transport path activity visualization is
disabled by default; it can be enabled by setting the visualizer's
:par:`displayRoutes` parameter to true.

:ned:`TransportRouteVisualizer` observes packets that pass through the
transport layer, i.e. carry data from/to higher layers.

The activity between two nodes is represented visually by a polyline
arrow which points from the source node to the destination node.
:ned:`TransportRouteVisualizer` follows packets throughout their path so
that the polyline goes through all nodes which are the part of the path
of packets. The arrow appears after the first packet has been received,
then gradually fades out unless it is reinforced by further packets.
Color, fading time and other graphical properties can be changed with
parameters of the visualizer.

By default, all packets and nodes are considered for the visualization.
This selection can be narrowed with the visualizer's :par:`packetFilter`
and :par:`nodeFilter` parameters.

Enabling Visualization of Transport Path Activity
-------------------------------------------------

The following example shows how to enable transport path activity
visualization with its default settings. In the first example, we
configure a simulation for a wired network. This simulation can be run
by choosing the ``EnablingPathVisualizationWired`` configuration from
the ini file.

The wired network contains two connected :ned:`StandardHost` type nodes:
``source`` and ``destination``.

.. figure:: media/TransportPathVisualizerSimpleWired_v0615.png
   :width: 60%
   :align: center

The ``source`` node will be continuously sending UDP packets to the
``destination`` node by using a :ned:`UdpBasicApp` application.

In this simulation, ``pathVisualizer's`` type is
:ned:`TransportRouteVisualizer`. It is enabled by setting the
:par:`displayRoutes` parameter to true.

.. literalinclude:: ../omnetpp.ini
   :start-at: pathVisualizer.*.displayRoutes
   :end-at: pathVisualizer.*.displayRoutes
   :language: ini

The following video shows what happens when the simulation is run.

.. video:: media/EnablingPathVisualizationWired_v0615.m4v
   :width: 420

At the beginning of the video, a red strip appears and moves from
``source`` to ``destination``. This strip is the standard OMNeT++
animation for packet transmissions and has nothing to do with
:ned:`TransportRouteVisualizer`. When the packet is received in whole by
``destination`` (the red strip disappears), an arrow is added by
:ned:`TransportRouteVisualizer` between the two hosts, indicating transport
path activity. The packet's name is also displayed above the arrow.

Note, however, that the ARP packets do not activate the visualization,
because ARP packets do not pass through the transport layer. The
transport path activity arrow fades out quickly because the
:par:`fadeOutTime` parameter of the visualizer is set to a small value.

Our next simulation model is the wireless variant of the above example.
In this network, we use two :ned:`AdhocHost`'s. The traffic and the
visualization settings are the same as the configuration of the wired
example. The simulation can be run by choosing the
``EnablingPathVisualizationWireless`` configuration from the ini file.

Here is the network for the wireless configuration.

.. figure:: media/TransportPathVisualizerSimpleWireless_v0615.png
   :width: 60%
   :align: center

The following video shows what happens when the simulation is run.

.. video:: media/EnablingPathVisualizationWireless_v0615.m4v
   :width: 420

This animation is similar to the video of the wired example (apart from
an extra blue dotted line which is also part of the standard OMNeT++
packet animation). Note, however, that the ACK and ARP frames do not
activate the visualization because these frames do not pass through the
transport layer.

Filtering Transport Path Activity
---------------------------------

In complex networks where many nodes and several protocols are used, it
is often useful to be able to filter network traffic and visualize only
the part of the network traffic we are interested in.

In this simulation, we show how to use :par:`packetFilter` and
:par:`nodeFilter`. The simulation can be run by choosing the ``Filtering``
configuration from the ini file.

We set up a complex network with five ``routers``
(``router0..router4``), four ``etherSwitches`` (``switch0..switch4``)
and eight endpoints. The source nodes (``source1`` and ``source2``) are
continuously generating traffic by a :ned:`UdpBasicApp` application, which
is handled by a :ned:`UdpSink` application in the destination nodes
(``destination1`` and ``destination2``). ``VideoStreamServer`` streams
video (sends ``VideoStrmPK-frag`` packets) to ``videoStreamClient``. The
remaining two endpoints (``host1`` and ``host2``) are inactive in this
simulation.

.. figure:: media/TransportPathVisualizerFiltering_v0615.png
   :width: 100%

In our first experiment, we want to observe the traffic generated by
:ned:`UdpBasicApp`. For this reason, we configure the visualizer's
:par:`packetFilter` parameter to display only the ``UDPBasicAppData``
packets. Video stream traffic will not be visualized by transport path
activity visualizer. We adjust the visualizer's ``fadeOutMode`` and the
:par:`fadeOutTime` parameters so that the transport path activity arrow
does not fade out completely before the next ``UDPBasicAppData`` packet
arrives.

.. literalinclude:: ../omnetpp.ini
   :start-at: transportRouteVisualizer.displayRoutes
   :end-at: transportRouteVisualizer.packetFilter
   :language: ini

The following video has been captured from the simulation and shows
what happens if :par:`packetFilter` is set.

.. video:: media/Filtering_PacketFilter_v0615.m4v
   :width: 100%

You can see that although there are both video stream and
``UDPBasicAppData`` traffic in the network, :ned:`TransportRouteVisualizer`
displays only the latter, due to the presence of the :par:`packetFilter`
parameter.

In the first experiment, we filtered network traffic based on packets.
In INET, it is also possible to filter traffic based on network nodes.
In our second experiment, we want to display traffic only between
``source1`` and ``destination1``. For this reason, we set the
visualizer's :par:`nodeFilter` parameter to display only the part of the
traffic between ``source1`` and ``destination1``. :par:`PacketFilter` is
still enabled in this simulation so that the video stream will not be
visualized.

We add the following line to the configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: transportRouteVisualizer.nodeFilter
   :end-at: transportRouteVisualizer.nodeFilter
   :language: ini

The following video has been captured from the simulation and shows
what happens if :par:`nodeFilter` is set.

.. video:: media/Filtering_NodeFilter_v0615.m4v
   :width: 100%

If you observe the default OMNeT++ packet transmission animation (red
stripes), you can see that although there is UDP data traffic between
both source-destination pairs, the traffic is visualized only
between ``source1`` and ``destination1`` because of the :par:`nodeFilter`
parameter setting.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`TransportPathVisualizerShowcase.ned <../TransportPathVisualizerShowcase.ned>`

More Information
----------------

This example only demonstrates the key features of transport path
visualization. For more information, refer to the
:ned:`TransportRouteVisualizer` NED documentation.

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/10>`__ in the GitHub issue tracker for commenting on this
showcase.
