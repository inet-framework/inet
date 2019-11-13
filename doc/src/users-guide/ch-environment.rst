.. _ug:cha:environment:

The Physical Environment
========================

.. _ug:sec:environment:overview:

Overview
--------

Wireless networks are heavily affected by the physical environment, and
the requirements for today’s ubiquitous wireless communication devices
are increasingly demanding. Cellular networks serve densely populated
urban areas, wireless LANs need to be able to cover large buildings with
several offices, low-power wireless sensors must tolerate noisy
industrial environments, batteries need to remain operational under
various external conditions, and so on.

The propagation of radio signals, the movement of communicating agents,
battery exhaustion, etc., depend on the surrounding physical
environment. For example, signals can be absorbed by objects, can pass
through objects, can be refracted by surfaces, can be reflected from
surfaces, or battery capacity might depend on external temperature.
These effects cannot be ignored in high-fidelity simulations.

In order to help the modeling process, the model of the physical
environment in the INET Framework is separated from the rest of the
simulation model. The main goal of the physical environment model is to
describe buildings, walls, vegetation, terrain, weather, and other
physical objects and conditions that might have effects on radio signal
propagation, movement, batteries, etc. This separation makes the model
reusable by all other simulation models that depend on these
circumstances.

The following sections provide a brief overview of the physical
environment model.

.. _ug:sec:environment:physicalenvironment:

PhysicalEnvironment
-------------------

In INET, the physical environment is modeled by the
:ned:`PhysicalEnvironment` compound module. This module normally has one
instance in the network, and acts as a database that other parts of the
simulation can query at runtime. It contains the following information:

-  geometry and properties of *physical objects* (usually referrred to
   as “obstacles” in wireless simulations)

-  a *ground model*

-  other physical properties of the environment, like its bounds in
   space

:ned:`PhysicalEnvironment` is an active compound module, that is, it has
an associated C++ class that contains the data structures and implements
an API that allows other modules to query the data.

Part of :ned:`PhysicalEnvironment`’s functionality is implemented in
submodules for easy replacement. They are currently the ground model,
and an object cache (for efficient queries):



.. code-block:: ned

   ground: <default("")> like IGround if typename != "";
   objectCache: <default("")> like IObjectCache if typename != "";

.. _ug:sec:environment:physical-objects:

Physical Objects
----------------

The most important aspect of the physical environment is the objects
which are present in it. For example, simulating an indoor Wifi scenario
may need to model walls, floors, ceilings, doors, windows, furniture,
and similar objects, because they all affect signal propagation
(obstacle modeling).

Objects are located in space, and have shapes and materials. The
physical environment model supports basic shapes and homogeneous
materials, which is a simplified description but still allows for a
reasonable approximation of reality. Physical objects in INET have the
following properties:

-  *shape* describes the object in 3D independent of its position and
   orientation.

-  *position* determines where the object is located in the 3D space.

-  *orientation* determines how the object is rotated relative to its
   default orientation.

-  *material* describes material specific physical properties.

-  *graphical properties* provide parameters for better visualization.

Graphical properties include:

-  *line width*: affects surface outline

-  *line color*: affects surface outline

-  *fill color*: affects surface fill

-  *opacity*: affects surface outline and fill

-  *tags*: allows filtering objects on the graphical user interface

Physical objects in INET are stationary, they cannot change their
position or orientation over time. Since the shape of the physical
objects might be quite diverse, the model is designed to be extensible
with new shapes. INET provides the following shapes:

-  *sphere* shapes are specified by a radius

-  *cuboid* shapes are specified by a length, a width, and a height

-  *prism* shapes are specified by a 2D polygon base and a height

-  *polyhedron* shapes are specified by the convex hull of a set of 3D
   vertices

The following example shows how to define various physical objects using
the XML syntax supported by the physical environment:

.. literalinclude:: lib/Snippets.xml
   :language: xml
   :start-after: !DefiningPhysicalObjectsExample
   :end-before: !End
   :name: Defining physical objects example

In order to load the above XML file, the following configuration could
be used:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !PhysicalObjectsConfigurationExample
   :end-before: !End
   :name: Physical objects configuration example

.. _ug:sec:environment:ground-models:

Ground Models
-------------

In inter-vehicle simulations the terrain has profound effects on signal
propagation. For example, vehicles on the opposite sides of a mountain
cannot directly communicate with each other.

A ground model describes the 3D surface of the terrain. Its main purpose
is to compute a position on the surface underneath an particular
position.

INET contains the following built-in ground models implemented as
OMNeT++ simple modules:

-  :ned:`FlatGround` is a trivial model which provides a flat surface
   parallel to the XY plane at a certain height.

-  :ned:`OsgEarthGround` is a more realistic model (based on ) which
   provides a terrain surface.

.. _ug:sec:environment:geographic-coordinate-system-models:

Geographic Coordinate System Models
-----------------------------------

In order to run high fidelity simulations, it is often required to embed
the communication network into a real world map. With the new OMNeT++ 5
version, INET already provides support for 3D maps using for
visualization and as the map provider.

However, INET carries out all geometric computation internally
(including signal propagation and path loss) in a 3D Euclidean
coordinate system. The discrepancy between the internal scene
coordinate system and the usual geographic coordinate systems must be
resolved.

A geographic coordinate system model maps scene coordinates to
geographic coordinates, and vice versa. Such a model allows positioning
physical objects and describing network node mobility using geographical
coordinates (e.g longitude, latitude, altitude).

In INET, a geographic coordinate system model is implemented as an
OMNeT++ simple module:

-  :ned:`SimpleGeographicCoordinateSystem` provides a trivial linear
   approximation without any external dependency.

-  :ned:`OsgGeographicCoordinateSystem` provides an accurate mapping
   using the external library.

In order to use geographic coordinates in a simulation, a geographic
coordinate system module must be included in the network. The desired
physical environment module and mobility modules must be configured
(using module path parameters) to use the geographic coordinate system
module. The following example also shows how the geographic coordinate
system module can be configured to place the scene at a particular
geographic location and orientation.



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !GeographicCoordinateSystemConfigurationExample
   :end-before: !End
   :name: Geographic coordinate system configuration example

.. _ug:sec:environment:object-cache:

Object Cache
------------

If a simulation contains a large number of physical objects, then signal
propagation may become computationally very expensive. The reason is
that the transmission medium model must check each line of sight path
between all transmitter and receiver pairs against all physical objects.

An object cache organizes physical objects into a data structure which
provides efficient geometric queries. Its main purpose is to iterate all
physical objects penetrated by a 3D line segment.

In INET, an object cache model is implemented as an OMNeT++ simple
module:

-  :ned:`GridObjectCache` organizes objects into a fixed cell size 3D
   spatial grid.

-  :ned:`BvhObjectCache` organizes objects into a tree data structure
   based on recursive 3D volume division.
