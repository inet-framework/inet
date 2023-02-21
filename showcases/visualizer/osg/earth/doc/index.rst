Visualizing Terrain and Urban Environment
=========================================

Goals
-----

This showcase demonstrates adding a map in the simulation. Displaying a map in a
network simulation provides a real-world context and can help to improve the
visual appeal of the simulation. It also helps to place network nodes in a
geographic context and allows the addition of objects, such as buildings. The
map doesn't have any effect on the simulation, it only alters the visuals of the
network.

It contains three example configurations of increasing complexity, each
demonstrating various features of the visualization.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/earth <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/earth>`__

About the Visualizer
--------------------

The map can be displayed by including a :ned:`SceneOsgEarthVisualizer`
module in the network. It can display the map in the 3D view by using
osgEarth, thus any part of the world can be inserted (provided there is
a source for the map data). The use of the map requires an Internet
connection, and it is only available in the 3D view.

Visualization with the Default Settings
---------------------------------------

This example configuration demonstrates inserting the map of downtown
Boston into the simulation. It can be run by choosing the
``DefaultSettings`` configuration from the ini file. It uses the
following network:

.. figure:: media/defaultnetwork.png
   :width: 20%
   :align: center

The network contains an :ned:`IntegratedVisualizer` and an
:ned:`OsgGeographicCoordinateSystem` module. The configuration from
:download:`omnetpp.ini <../omnetpp.ini>` is the following:

.. literalinclude:: ../omnetpp.ini
   :start-at: SceneOsgEarthVisualizer
   :end-at: sceneLatitude
   :language: ini

By default, the type of the scene visualizer module in
:ned:`IntegratedVisualizer` is :ned:`SceneOsgVisualizer`. Inserting the map
requires the :ned:`SceneOsgEarthVisualizer` module, thus, the default OSG
scene visualizer is replaced. The :ned:`SceneOsgEarthVisualizer`
provides the same functionality as :ned:`SceneOsgVisualizer`, and adds
support for the osgEarth map.

To display the map, the visualizer requires a ``.earth`` file. This is an
XML file that specifies how the source data is turned into a map, and
how to fetch the necessary data from the Internet. In this
configuration, we use the ``boston.earth`` file that contains
`OpenStreetMap <https://www.openstreetmap.org>`__ configured
as map data source. More ``.earth`` files can be found at
`osgearth.org <http://osgearth.org>`__, and there are also instructions
there on how to create ``.earth`` files.

Locations on the map are identified with geographical coordinates, i.e.
longitude and latitude. In INET, locations of nodes and objects are
represented internally by Cartesian coordinates relative
to the simulation scene's origin, and the
:ned:`OsgGeographicCoordinateSystem` module is responsible for
converting between geographical and Cartesian coordinates.
To define the mapping between the two, the geographical
coordinates of the simulation scene's origin must be specified in
the coordinate system module's :par:`sceneLongitude` and :par:`sceneLatitude`
parameters. In our configuration, the scene's origin is set to somewhere
near a large park in Boston. Additionally, the origin's altitude
could also be configured (:par:`sceneAltitude`); however, specifying
the latitude and longitude is sufficient for the map visualization to work.
The scene's orientation can also be specified in the coordinate system module's
parameters using Euler angles (:par:`sceneHeading`,
:par:`sceneElevation`, :par:`sceneBank`). In the default setting,
the X-axis points East, the Y-axis points North, and the Z-axis
points up.

The size of the simulation scene is determined automatically, taking into
account the position of objects, network nodes, and movement
constraints of network nodes. Thus, everything in the simulation should
happen within the boundaries of the simulation scene.
The scene floor is visualized on the map using a
semi-transparent rectangle overlay. (Displaying the scene floor
is optional but turned on by default.) When the
simulation starts, the view will be centered on the Cartesian
coordinate system's origin if there are no nodes in the network. If
there are nodes, the initial viewpoint will be set so that all nodes
are visible.

The scene's 3D view should look like the following when the simulation is run.
The map is displayed on the 3D scene. Since there are no nodes or
objects in the network, the size of the scene is zero and the screen
floor is not visible.

.. figure:: media/defaultmap.png
   :width: 100%


Adding Physical Objects
-----------------------

The map doesn't affect simulations in any way, it just gives a real-world
context to them. For network nodes to interact with their environment,
physical objects have to be added. The example configuration for this
section can be run by selecting the ``PhysicalObjects`` configuration
from the ini file. It extends the previous configuration by adding
physical objects, meant to represent blocks of buildings, to the simulation.
The objects could affect radio transmissions if an obstacle loss model were set.

The network for this configuration extends the network from the previous
section with a :ned:`PhysicalEnvironment` module:

.. figure:: media/objectsnetwork.png
   :width: 20%
   :align: center

The configuration for this example simulation extends the previous
configuration with the following:

.. literalinclude:: ../omnetpp.ini
   :start-at: coordinateSystemModule
   :end-at: config
   :language: ini

The :ned:`PhysicalEnvironment` module is responsible for placing the
physical objects on the scene. The physical environment module
doesn't use a coordinate system module by default, but it is configured to use
the one present in the network. This module makes it possible to define the
objects using geographical coordinates. The objects are defined in the
``obstacle.xml`` config file.

The scene looks like the following when the simulation is run.
The objects that represent blocks of buildings are displayed as semi-transparent
red blocks, and the group of trees at the park's edge is displayed as a
semi-transparent green block.

.. figure:: media/objectsmap.png
   :width: 100%


Placing Network Nodes on the Map
--------------------------------

This example configuration demonstrates the placement of network nodes
on the map. The simulation can be run by choosing the ``NetworkNodes``
configuration from the ini file. The network for this configuration
extends the previous network with a radio medium and a network
configurator module. It also adds four :ned:`AdhocHost`'s:

.. figure:: media/nodesnetwork.png
   :width: 50%
   :align: center

The configuration extends the previous configuration with the following:

.. literalinclude:: ../omnetpp.ini
   :start-at: sceneShading
   :end-at: host4.mobility.initialLongitude
   :language: ini

The first block of lines configures the scene to be semi-transparent
black so that the underlying map is visible. The next block sets the
altitude of the scene. By default, the altitude is zero; we set it
to one meter so that the nodes are not on the level of the ground.
The heading is also specified so that the edges of the scene roughly
align with the streets on the map.

The rest of the configuration deals with placing the network nodes
on the map. The mobility type of nodes is set to :ned:`StationaryMobility`,
which is a mobility module that has parameters for positioning using
geographical coordinates.
The required latitude and longitude values can be obtained from
`www.openstreetmap.org <http://www.openstreetmap.org>`__. Once
on that site, choose *Share* from the menu at the right,
and tick *Include marker*. The marker can be
dragged on the map, and the coordinates of the marker's location can
be read (and copy-pasted into the ini file) from the *Share* panel.

.. figure:: media/openstreetmap.png
   :width: 100%

The scene with the nodes looks like the following when the simulation is run.
The scene floor is visible against the map, and you can verify that the nodes
have been placed at the specified coordinates.

.. figure:: media/nodesmap2.png
   :width: 100%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`EarthVisualizationShowcase.ned <../EarthVisualizationShowcase.ned>`

Further Information
-------------------

For further information about the visualizer, refer to the NED documentation of
:ned:`SceneOsgEarthVisualizer` and :ned:`OsgGeographicCoordinateSystem`.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/33>`__
in the GitHub issue tracker for commenting on this showcase.
