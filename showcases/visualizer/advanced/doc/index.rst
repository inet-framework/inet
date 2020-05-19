:orphan:

Using Multiple Visualizer Modules
=================================

.. todo::

   -  TODO: The title could be "Advanced Visualizer module use cases" or
      not NOPE
   -  TODO: emphasize how this visualization needs two visualizer modules
   -  TODO: another configuration where there are two radioMediums -> need
      two visualizers there are 2.4 and 5 GHz wifi hosts...the frequency
      should be color coded

Goals
-----
.. todo::

   another angle: it is possible to create more complex
   visualizations by including multiple visualizer modules, which can be
   configured independently. The focus is on the possibility...not the need

   More complex visualizations can be created by using more than one
   visualizer. Something like that.

V2

It is possible to create more complex visualization by using multiple
visualizer modules modules of the same type instead of just one. The
modules can be configured independently, providing a flexible way to
customize visualization of complex simulations. For example, it can be
useful to configure different path visualizations for different parts of
the network.

V3

The modules can be configured independently, providing a flexible way to
customize visualization of complex simulations. The modules can be
configured independently to suit visualization of complex simulations.

V1

Complex simulations can benefit from complex visualization to better
understand what is happening in the network. Multiple visualizer modules
of the same type can be included in the network if the desired
visualization cannot be accomplished with a single visualizer module.
The various visualizer modules can be configured independently,
providing a flexible way to customize visualizations to one's needs. For
example, one can configure different path visualizations for different
parts of the network.

This showcase describes the various types of compound visualizer
modules, such as ``IntegratedVisualizer``. It demonstrates the use of
multiple visualizers with two example simulations, and shows how the
visualization settings can be modified at runtime. Finally, it
demonstrates how to add your own visualizer types to the existing
compound visualizer modules.

INET version: ``3.6``

Source files location: `inet/showcases/visualizer/advanced <https://github.com/inet-framework/inet-showcases/tree/master/visualizer/advanced>`__

About visualizer module types
-----------------------------

INET contains visualizer simple modules suitable for a single
visualization task, such as ``RoutingTableCanvasVisualizer`` or
``InterfaceTableOsgVisualizer``. However, there are visualizer compound
modules, which contain multiple single-task visualizer simple modules.
One has to include just one compound visualizer module in the network,
and multiple visualizers can be configured from the ini file. The
following section aims to clarify the various simple and compound
visualizer module types available in INET.

**Canvas and Osg simple modules**

There are separate types of visualizer simple modules for 2D and 3D
visualization. 2D visualization is implemented by
``Canvas   visualizer`` modules, while 3D visualization is handled by
``Osg visualizers``. The visualizer simple modules have the word
``Canvas`` or ``Osg`` in their type names, e.g.
``RoutingTableCanvasVisualizer`` or ``StatisticOsgVisualizer``.

**Compound modules of Canvas and Osg visualizers**

There are compound modules containing the Canvas and Osg versions of
single-task visualizers, e.g. the ``RoutingTableVisualizer`` compound
module contains a ``RoutingTableCanvasVisualizer`` and a
``RoutingTableOsgVisualizer`` simple module. These single-task compound
visualizers can provide both 2D and 3D visualizations (i.e. the
RoutingTableVisualizer module can visualize routing tables in both 2D
and 3D.) A rule of thumb is if the type name doesn't contain either
``Canvas`` or ``Osg``, the module contains both kinds of visualizers.

.. figure:: compound.png
   :width: 100%

**Integrated Canvas and Osg visualizers**

The ``IntegratedCanvasVisualizer`` and ``IntegratedOsgVisualizer``
compound modules each contain all available Canvas and Osg visualizer
types, respectively.

.. figure:: integratedcanvasosg.png
   :width: 100%

**Integrated visualizer**

The ``IntegratedVisualizer`` contains an ``IntegratedCanvasVisualizer``
and a ``IntegratedOsgVisualizer``. Thus it contains all available
visualizers.

.. figure:: integrated.png
   :width: 100%

**Multi-visualizers**

Multi-visualizers are compound visualizer modules containing submodule
vectors of visualizer simple modules. Such visualizers are useful if
multiple visualizers of the same type are required for creating complex
visualizations. The available multi visualizer modules are
``IntegratedMultiCanvasVisualizer`` and
``IntegratedMultiOsgVisualizer``, each containing submodule vectors of
canvas and osg visualizer simple modules. The
``IntegratedMultiVisualizer`` contains both an
``IntegratedMultiCanvasVisualizer`` and an
``IntegratedMultiOsgVisualizer``, similarly to ``IntegratedVisualizer``.
By default, the canvas and osg multi visualizers contain one submodule
of each visualizer simple module. The number of submodules can be
specified for each visualizer submodule with parameters, e.g.
``numTransportConnectionVisualizers = 2`` or
``numDataLinkVisualizers = 3``.

.. image:: multicanvas.png
.. image:: multiintegrated2.png

.. todo::

   Consisely

   - the visualizations are handled by visualizer simple modules such as foo
   - there are two kinds of those, canvas and osg, for 2d and 3d, respecively
   - there are compound modules that contain a specific visualizer's canvas and osg version. the name doesnt have
     canvas or osg in it -> it means it is capable of both / it contains both
   - there are integrated visualizers, that contain multiple visualizer simple modules
   - there are 3 types
   - the integrated canvas and osg visualizers contain all canvas and osg visualizers, respectively
   - the IntegratedVisualizer contains an integrated canvas and an integrated osg, thus containing all available visualizer
     simple module types

   <!--
   <p>V2</p>

   The various visualizations are handled by specific visualizer simple modules. Visualizations in 2D are implemented by
   `Canvas` visualizer simple modules, while visualizations in 3D are handled by `Osg` visualizer simple modules. The visualizer simple modules have the word `Canvas` or `Osg` in their type name, e.g. `RoutingTableCanvasVisualizer` or `InterfaceTableOsgVisualizer`.

   There are compound modules which contain both the Canvas and the Osg version of a specific visualizer simple module,
   e.g. `RoutingTableVisualizer` contains a `RoutingTableCanvasVisualizer` and a `RoutingTableOsgVisualizer` simple module. Thus these specific compound visualizers can provide both 2D and 3D visualizations (i.e. the RoutingTableVisualizer can visualize routing tables in both 2D and 3D).
   The rule of thumb is if the type name doesn't contain either
   `Canvas` or `Osg`, the module contains both kinds of visualizers.

   There are integrated visualizer compound modules, which contain multiple specific visualizer simple modules. There are three kinds of integrated visualizers:

   The `IntegratedCanvasVisualizer` contains all Canvas visualizer simple modules, while the `IntegratedOsgVisualizer` contains all Osg visualizer simple modules.

   The `IntegratedVisualizer` contains an `IntegratedCanvasVisualizer` and an `IntegratedOsgVisualizer`. Thus it contains all available visualizer simple modules.

   By including an `IntegratedVisualizer` in the network, the features of all contained visualizers are available in
   the simulation using the network, both in 2D and 3D.
   The parameters of the visualizer simple modules can be configured from the ini file.

   <p>The screenshots illustrating the above HERE</p>

   <p>V3</p>

   The various visualizations are handled by specific visualizer simple modules. Visualizations in 2D are implemented by
   `Canvas` visualizer simple modules, while visualizations in 3D are handled by `Osg` visualizer simple modules. The visualizer simple modules have the word `Canvas` or `Osg` in their type name, e.g. `RoutingTableCanvasVisualizer` or `InterfaceTableOsgVisualizer`.

   There are compound modules which contain both the Canvas and the Osg version of a specific visualizer simple module,
   e.g. `RoutingTableVisualizer` contains a `RoutingTableCanvasVisualizer` and a `RoutingTableOsgVisualizer` simple module. Thus these specific compound visualizers can provide both 2D and 3D visualizations (i.e. the RoutingTableVisualizer can visualize routing tables in both 2D and 3D).
   The rule of thumb is if the type name doesn't contain either
   `Canvas` or `Osg`, the module contains both kinds of visualizers.

   ![](compound.png)

   There are integrated visualizer compound modules, which contain multiple specific visualizer simple modules. There are three kinds of integrated visualizers:

   The `IntegratedCanvasVisualizer` contains all Canvas visualizer simple modules, while the `IntegratedOsgVisualizer` contains all Osg visualizer simple modules.

   ![](integratedcanvasosg.png)

   The `IntegratedVisualizer` contains an `IntegratedCanvasVisualizer` and an `IntegratedOsgVisualizer`. Thus it contains all available visualizer simple modules.

   ![](integrated.png)

   -->

By including an ``IntegratedVisualizer`` or
``IntegratedMultiVisualizer`` in the network, the features of all
contained visualizers are available in the simulation using the network,
both in 2D and 3D. The parameters of the visualizer simple modules can
be configured from the ini file.

.. todo::

   <!--
   <p>
   TODO: are these screenshots needed? seem to cut up the flow of text. without it it was more fluid
   </p>

   <p>TODO: are these screenshots ok?</p>

   <pre>
   TODO
   For consistency, these should be made with the IDE
   The modules names should be capitalized
   Actually, the qtenv looks better
   </pre>
   -->

Including multiple visualizer modules
-------------------------------------

Visualizations from multiple visualizer modules of the same type can be
combined to create more complex visualizations that would not be
possible using a single visualizer module. In this section, we present
two example simulations that demonstrates the use of two visualizer
modules. The configurations for the simulations are defined in the
omnetpp.ini file. TODO: rewrite for two configs The configuration uses
the following network:

.. figure:: network.png
   :width: 100%

The hosts and the server in the network are ``StandardHosts``. The
network contains two ``IntegratedVisualizer`` modules, named
``visualizer1`` and ``visualizer2``.

The ``server`` runs an UDP video stream server
(``UdpVideoStreamServer``), and two ``TCPSessionApps``. These
applications are configured to send UDP and TCP streams to certain
hosts:

-  ``host2`` and ``host4`` are configured to send UDP video stream
   requests to the server, which in turn sends UDP streams to the hosts.
-  The ``server`` is configured to send TCP streams to ``host3`` and
   ``host5``.

The goal is to visualize UDP and TCP streams with different colors, so
it is easier to differentiate between them. We configure the
``NetworkRouteVisualizer`` in ``visualizer1`` to display TCP packet
paths with blue arrows, and the ``NetworkRouteVisualizer`` in
``visualizer2`` to indicate UDP packet paths with red arrows.

.. code-block: none

   *.visualizer1.networkRouteVisualizer.displayRoutes = true
   *.visualizer1.networkRouteVisualizer.packetFilter = "*tcp* or *ACK* or
   *SYN*" \*.visualizer1.networkRouteVisualizer.lineColor = "blue"

   *.visualizer2.networkRouteVisualizer.displayRoutes = true
   *.visualizer2.networkRouteVisualizer.packetFilter = "*Video*"
   *.visualizer2.networkRouteVisualizer.lineColor = "red"

The TCP visualization is configured to display all TCP packets,
including the ones that take part in establishing the connection, i.e.
the ACK, SYN+ACK, and SYN. The other visualizer is configured to
visualize all UDP packets (packets with 'Video' in their names, which
covers all UDP packets in the network).

The ``StatisticVisualizer`` in ``visualizer1`` is configured to display
the number of received UDP packets in the affected nodes. The other
``StatisticVisualizer`` in ``visualizer2`` is configured to indicate the
total size of the received TCP data.

.. code-block:: none

   *.visualizer1.statisticVisualizer.displayStatistics = true
   *.visualizer1.statisticVisualizer.signalName = "passedUpPk"
   *.visualizer1.statisticVisualizer.sourceFilter = "**.udp"
   *.visualizer1.statisticVisualizer.format = "UDP packets received: %v"

   *.visualizer2.statisticVisualizer.displayStatistics = true
   *.visualizer2.statisticVisualizer.signalName = "packetReceived"
   *.visualizer2.statisticVisualizer.statisticName = "rcvdBytes"
   *.visualizer2.statisticVisualizer.unit = "KiB"
   *.visualizer2.statisticVisualizer.sourceFilter = "**.tcpApp[*]"
   *.visualizer2.statisticVisualizer.format = "TCP data received: %v %u"

Additionally, the ``TransportConnectionVisualizer`` in ``visualizer1``
is enabled, and set to visualize transport connections in the network,
i.e. the TCP connections. The set of colors for the icons to use is set
to yellow and green.

.. todo::

   <!--
   V2
   Additionally, the `TransportConnectionVisualizer` in
   `visualizer1` is enabled, and set to visualize transport
   connections in the network, i.e. the TCP connections. The set of colors for the icons to use is set to blue and darkblue to differentiate the two TCP connections.
   -->

  TODO: why cant it be blue?

  TODO: maybe it should be two shades of blue (darkblue, lightblue)

.. code-block:: none

   *.visualizer1.transportConnectionVisualizer.displayTransportConnections = true *.visualizer1.transportConnectionVisualizer.iconColor = "green, yellow"

The ``NetworkNodeVisualizer`` module creates the visualization figures
for all other visualizers. By default, the two visualizer modules in the
network each use their own NetworkNodeVisualizer. The
NetworkNodeVisualizer modules would create overlapping visualizations,
so ``visualizer2`` has to be configured to display its visualizations
using ``visualizer1's networkNodeVisualizer`` module. The following keys
achieve this:

.. code-block:: none

   *.visualizer2.networkNodeVisualizerType = "" *.visualizer2.*.networkNodeVisualizerModule = "visualizer1.networkNodeVisualizer"

When the simulation is run, this happens:

.. video:: advanced3.mp4
   :width: 698

The server starts sending the UDP and TCP streams to the hosts. The
paths of UDP packets are indicated with red arrows, and the paths of TCP
packets with blue arrows. The statistics for the UDP and TCP packets are
displayed above the affected nodes. The number of received UDP packets
at the server is just two, which are the two video stream request
packets from the hosts. The received UDP packets at ``hosts 2 and 4``
keep increasing. The received TCP data is displayed above the TCP hosts.
TCP connection visualization icons are differentiated based on the
letters in the icons, as the ``TransportConnectionVisualizer`` is using
just one color.

Visualizing multiple radio mediums
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

V1

This example simulation features two wireless networks, which operate in
different frequency bands (2.4 Ghz and 5 Ghz.) The two wireless networks
are in independent frequency bands, and thus can operate concurrently
without interfering with each other. The simulation can be optimized by
configuring the two wireless networks to use two different radio medium
modules. This way radio medium modules have to send transmissions only
to those hosts that can receive them. The transmissions on the two radio
mediums can only be visualized using two visualizer modules.

.. todo::

   actually, the same can be achieved by one radio medium and one
   visualizer, but it is not efficient

V2

This example simulation features two wireless networks operating in
non-interfering frequency bands (2.4 and 5 GHz.)

-  there is no interference
-  the radioMedium still sends every transmission to every node,
   regardless of the node's ability to receive it
-  when it is certain that some nodes wont receive a transmission,
   because it is certain they are out of range
-  or they operate in different frequency bands
-  it can be useful to have two radio medium modules, and put the
   separate networks on their own radio medium
-  this way transmission are only sent to nodes that can receive it.
   this can optimize performance.

key things: the radio medium sends transmission to all nodes, even those
that are out of range or on different channels or frequency bands. The
simulation can be optimized by putting those hosts that are known not to
interfere to different radio mediums.

V3

Radio medium modules send all transmissions in the network to all
wireless nodes. The individual nodes calculate wether or not they can
receive the transmissions correctly. When there are multiple wireless
networks, and it is certain that they cannot receive each other's
transmissions (e.g. they are out of range or operate on different
channels or frequency domainsa), the simulation can be optimized by
putting each wireless network on a different radio medium.

The ``MediumVisualizer`` module can visualize transmissions from just
one radio medium, so more visualizer modules are needed if there are
multiple radio medium modules. This example simulation features two
wireless networks, which operate in different frequency bands (2.4 and 5
GHz.) The two networks are put on two different radio mediums, and two
visualizer modules are used for visualizing transmissions.

.. video:: Wireless2.mp4
   :width: 698

Note that even though the two signals overlap, the transmissions are
received correctly at both wireless networks, because the signals are in
different frequency bands and don't interfere with each other.

TODO: configure them to 2.4 and 5 GHz or just say it doesnt matter or
just use different wifi channels

Modifying visualizer parameters at runtime
------------------------------------------

Visualizer parameters are usually set from the ini file, but it is also
possible to modify some of the parameters in the graphical runtime
environment. The modified parameters take effect immediatelly. This is
useful for tuning the visualizer parameters, as one doesn't have to
close and restart qtenv to see the effects of the parameter changes. The
following parameters react to changes in qtenv:

-  ``display flag``: used to turn visualizations on and off
-  ``filters``: various filters like node, packet, and interface filters
-  ``format string``: used to customize how information is displayed

The parameters can be changed by clicking on the appropriate visualizer
module in the network. The parameters are displayed in the inspector
panel to the right (make sure children mode is selected at the top of
the panel.)

.. todo::

   <!--
   ![](modify.png)

   TODO: this image is too big
   -->

.. figure:: modify_4.png
   :width: 100%

.. figure:: modify_3.png
   :width: 100%

Double click a parameter to select it, then double click on the value
field to change it:

.. figure:: modify2.png
   :width: 100%

The changes will take effect immediatelly.

Including your own visualizers in the integrated visualizer modules
-------------------------------------------------------------------

It is possible to replace the specific visualizer types in the compound
visualizer modules with your own for example you have a
CustomRoutingTableCanvasVisualizer module, you can use that in
IntegratedCanvasVisualizer instead of the default
RoutingTableCanvasVisualizer module. How to do it, illustrated with
screenshots

Further information
-------------------

Discussion
----------

Use `this page <TODO>`__ in the GitHub issue tracker for commenting on
this showcase.
