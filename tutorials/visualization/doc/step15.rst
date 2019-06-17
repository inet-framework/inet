Step 15. Showing routing table entries
======================================

Goals
-----

Routing information is scattered among nodes and can be accessed in
individual routing tables of the nodes. Visualizing routing table
entries graphically clearly shows how a packet would be routed, without
looking into individual routing tables. In this step, we enable
visualization of routing table entries.

The model
---------

Network topology
~~~~~~~~~~~~~~~~

The configuration for this step uses the :ned:`VisualizationE` network,
defined in :download:`VisualizationE.ned <../VisualizationE.ned>`.

We add the following nodes to the network:

- one :ned:`Router` (``router0``),
- one :ned:`EtherSwitch` (``switch0``),
- one :ned:`WirelessHost` (``pedestrianVideo``)
- and two ``Standardhost`` s (``videoStreamServer`` and ``server1``).

Wireless hosts connect to ``router0`` via ``accessPoint0``,
``videoStreamServer`` and ``server1`` connect to ``router0`` via
``switch0``. The nodes are placed on the playground as follows.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # initializing pedestrianVideo position
   :end-before: # videoStreamServer application settings

The following image shows how the network looks like.

.. figure:: media/step15_model_network.png

Video stream application
~~~~~~~~~~~~~~~~~~~~~~~~

We add a video stream application to the configuration. The client is
``pedestrianVideo``, the server is ``videoStreamServer``. They
communicate at UDP port 4000. In addition, sending interval, packet
length and the video size are also defined in the ``omnetpp.ini`` file.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # videoStreamServer application settings
   :end-before: # showing routing table entries towards videoStreamServer

Visualizer
~~~~~~~~~~

Routing entries are visualized by :ned:`RoutingTableVisualizer` (included
in the network as part of :ned:`IntegratedVisualizer`). The visualizer can
be enabled by setting ``displayRoutingTables`` to *true*. The
``videoStreamServer`` node is selected as destination by setting
``destinationFilter`` to *\*videoStreamServer\**. We want to display the
route which is between *\*pedestrianVideo\** and
*\*videoStreamServer\**. To this end, we set the visualizer's
``nodeFilter`` parameter to *pedestrianVideo or videoStreamServer or
switch\* or router\**. (The route can lead through any switch or router
so they need to be added to ``nodeFilter``.)

Results
-------

.. todo::

   When we start the simulation we can see that, the routingTableVisualizer draw arrows
   to represent the routes. This is because by default netmask routes, default routes
   and static routes added to routing table. Later we change that.<br>
   [img: routes]

   After 1 second the VoIP application starts and VoIP data links appear,
   because dataLinkVisualizer is on. After 5 seconds the videoPedestrian sends
   request to the videoStreamServer and the application starts.
   In the Module view mode we can follow the progress. The client send the request,
   and in response the server starts the video stream.
   [gif: video stream start]


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationE.ned <../VisualizationE.ned>`

