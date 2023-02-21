Visualizing IEEE 802.11 Network Membership
==========================================

Goals
-----

When simulating wireless networks that overlap in space or where network membership
changes over time, getting an overview of the membership of each node can be cumbersome.
In such simulations, the IEEE 802.11 network membership visualizer can be very helpful.

In this showcase, we demonstrate how to display the SSID of each node, making it
easier to understand the network membership and how it changes over time.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/ieee80211 <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/ieee80211>`__

About the visualizer
--------------------

In INET, IEEE 802.11 network membership can be visualized by including a
:ned:`Ieee80211Visualizer` module in the simulation. Adding an
:ned:`IntegratedVisualizer` is also an option because it also contains a
:ned:`Ieee80211Visualizer`. Displaying network membership is disabled by
default; it can be enabled by setting the visualizer's
:par:`displayAssociations` parameter to ``true``.

The :ned:`Ieee80211Visualizer` displays an icon and the SSID above network
nodes which are part of a wifi network. The icons are color-coded
according to the SSID. The icon, colors, and other visual properties can
be configured via parameters of the visualizer.

The visualizer's :par:`nodeFilter` parameter selects which nodes'
memberships are visualized. The :par:`interfaceFilter` parameter selects
which interfaces are considered in the visualization. By default, all
interfaces of all nodes are considered.

Basic use
---------

The first example simulation demonstrates the visualization with the
default visualizer settings. It can be run by choosing the
``OneNetwork`` configuration from the ini file. The simulation uses the
following network:

.. figure:: media/simplenetwork.png
   :width: 60%
   :align: center

The network contains a :ned:`WirelessHost` and an :ned:`AccessPoint`. The
access point SSID is left at the default setting, ``"SSID"``. At the
beginning of the simulation, the host will initiate association with the
access point. When the association process goes through, the node
becomes part of the wireless network, and this should be indicated by
the icon.

The visualization is activated with the visualizer's
:par:`displayAssociations` parameter:

.. literalinclude:: ../omnetpp.ini
   :start-at: ieee80211Visualizer.displayAssociations
   :end-at: ieee80211Visualizer.displayAssociations
   :language: ini

When the simulation is run for a while, the network will look like the
following. Note the icons above the host and the access point.

.. figure:: media/displayassoc.png
   :width: 60%
   :align: center

Multiple networks
-----------------

The following example simulation demonstrates the visualization when
multiple networks are present. The simulation can be run by choosing the
``MultipleNetworks`` configuration from the ini file.

The network contains two :ned:`AccessPoint`'s with different SSIDs, and
three :ned:`WirelessHost`'s configured to associate with each. We will see
the icons being color-coded. When the association processes take place,
the network will look like the following. Note the different SSIDs
(``alpha``, ``bravo``) and the colors.

.. figure:: media/advanced2.png
   :width: 80%
   :align: center

Visualizing handover
--------------------

The following example simulation shows how visualization can help you
follow handovers in the network. The simulation can be run by choosing
the ``VisualizingHandover`` configuration from the ini file. The network
contains two :ned:`AccessPoint`'s with different SSIDs, ``alpha`` and
``bravo``. There is also a :ned:`WirelessHost` which is configured to move
horizontally back and forth between the two access points. Transmission
powers are configured so that when a host gets near one access point, it
will go out of the range of the other access point. This transition will trigger a
handover.

The communication ranges of the access points are visualized as blue
circles. The following animation shows what happens when the simulation
is run. Note how the indicator above the host changes after each
handover.

.. video:: media/handover10.mp4
   :width: 580

   <!--internal video recording, animation speed node, run in fast mode-->

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`Ieee80211VisualizationShowcase.ned <../Ieee80211VisualizationShowcase.ned>`

Further information
-------------------

For more information on IEEE 802.11 visualization, see the
:ned:`Ieee80211Visualizer` NED documentation.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/4>`__ in
the GitHub issue tracker for commenting on this showcase.
