Combining Mobility Models
=========================

Goals
-----

Node positioning and mobility are crucial in simulation scenarios that involve
wireless communication. INET provides several mobility models, such as linear
motion or random waypoint, which can be added to nodes as modules. The ability
to combine these models opens up the possibility of creating complex and
realistic simulation scenarios.

This showcase focuses on combining elementary mobility models, which describe
motion, position, and orientation independently, to generate more sophisticated
motion patterns. (The use of elementary mobility models is covered in a separate
showcase, :doc:../../basic/doc/index.)

| INET version: ``4.0``
| Source files location: `inet/showcases/mobility/combining <https://github.com/inet-framework/inet/tree/master/showcases/mobility/combining>`__

Overview
--------

INET has two special mobility models that do not define motion on their own,
but rather, they allow combining existing mobility models. These modules are
:ned:`AttachedMobility` and :ned:`SuperpositioningMobility`. We'll show
example simulations for both.

The model
---------

Example simulations in this showcase, except for the last one, use the following network:

.. image:: media/scene.png
   :width: 50%
   :align: center

The size of the scene is 400x400x0 meters. It contains a configurable
number of hosts and an :ned:`IntegratedMultiVisualizer` module to visualize
some aspects of mobility.

AttachedMobility
----------------

:ned:`AttachedMobility`, as its name shows, can "attach" itself to another mobility module,
with a certain offset. This "other" mobility module may be part of the same node
or may be in another node. The latter allows for implementing simple group mobility
where member nodes move in a formation.

This offset is interpreted in the other mobility module's own coordinate system.
I.e. the :ned:`AttachedMobility` module takes the
mobility state of the mobility module it is attached to and applies an
optional offset to the position and orientation. It keeps its mobility
state up-to-date. Position and orientation, and their derivatives are
all affected by the mobility where the :ned:`AttachedMobility` is used.

Example: Linear Movement
~~~~~~~~~~~~~~~~~~~~~~~~

In the first example, ``host[0]`` is moving using :ned:`LinearMobility`.
Three more hosts are attached to it at different offsets using :ned:`AttachedMobility`.
The :par:`mobilityModule` parameter selects the mobility module to attach
to (the parameter has no default, so it must be set). There are also per
coordinate offset parameters for position and orientation:

-  :par:`offsetX`, :par:`offsetY`, :par:`offsetZ`
-  :par:`offsetHeading`, :par:`offsetElevation`, :par:`offsetBank`

The simulation is run with four hosts. The configuration
is defined in the omnetpp.ini file:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.numHosts = 4
   :end-at: offsetHeading

``host[0]`` starts at the center of the scene, and moves along a straight line using
:ned:`LinearMobility`. The other three hosts have :ned:`AttachedMobility`, and
are attached to ``host[0]``'s mobility module. Their offset is interpreted
in the coordinate system of ``host[0]``'s mobility module. The
coordinate system rotates and translates as ``host[0]`` moves.
``host[1]`` is offset 50m along the X axis, ``host[2]`` 50m along the Y
axis, ``host[3]`` -50m along the Y axis. Also, ``host[3]``'s heading is
offset 45 degrees.

Here is a video of the simulation running:

.. video:: media/Attached2_2.mp4
   :width: 50%

The hosts keep a formation around ``host[0]``. Note that as ``host[0]``
bounces back from the boundary of the constraint area, there is a jump
in the position of the other hosts.

Example: Concentric Movement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the second example simulation, one of the hosts moves along
a circle, with other hosts "attached". The example simulation
is run with five hosts. The simulation is
defined in the ``Attached2`` configuration in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.numHosts = 5
   :end-at: offsetHeading

``host[0]`` moves along a circle, orbiting the center of the
scene. The other hosts use :ned:`AttachedMobility`, and are
attached to ``host[0]`` with various offsets, summarized on the
following image:

.. image:: media/circular.png
   :width: 50%
   :align: center

The attached hosts keep the offset from ``host[0]``, and they
move circularly as the coordinate system of ``host[0]`` moves and rotates:

.. video:: media/Attached3.mp4
   :width: 50%

The relative positions of the hosts are constant. For example, it appears as if
``host[4]`` was using  a :ned:`CircleMobility` similar to ``host[0]``,
while actually, its position in ``host[0]``'s coordinate system stays constant.

SuperpositioningMobility
------------------------

:ned:`SuperpositioningMobility` is a compound module that can contain several
other mobility modules as submodules, and combines their effects.
The mobility state (position, orientation, and their derivatives) exposed by
:ned:`SuperpositioningMobility` is the sum of the states of these contained submodules.

The use of :ned:`SuperpositioningMobility` is that it allows one to create complex motion
patterns by combining other mobility models, and also to combine arbitrary motion
with arbitrary initial positioning.

Example: Perturbed Circle
~~~~~~~~~~~~~~~~~~~~~~~~~

In this example simulation, ``host[0]`` moves in a circle using
:ned:`CircleMobility`. Some random movement is applied to the circular
motion using :ned:`GaussMarkovMobility`, which is a model to control
the randomness in the movement. The simulation is run with only one host.
You can take a look at the configuration in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.numHosts = 1
   :end-at: constraintAreaMaxZ

The :par:`numElements` parameter defines the number of mobility submodules,
which are contained in a submodule vector named :par:`element`.
Therefore, instead of :par:`mobility.typeName = XY`, the mobility
submodules can be referenced with :par:`mobility.element[0].typeName = XY`.
This is also visible if we take a look at the inside of ``host[0]``'s
mobility submodule:

.. image:: media/MobilityElements.png
   :width: 40%
   :align: center

In the following image you can see that the position and velocity
of the :ned:`SuperpositioningMobility` module is indeed the sum of the position
and velocity of the contained submodules (visible in the previous image):

.. image:: media/MobilitySum.png
   :width: 40%
   :align: center

On the following video, you can see the resulting motion:

.. video:: media/Superpositioning1.mp4
   :width: 50%

Example: Orbiting a Node
~~~~~~~~~~~~~~~~~~~~~~~~

This simulation contains a host that moves in a hexagonal
pattern, and another host that orbits the first host as it
moves. The first host, ``host[0]``, uses :ned:`TurtleMobility`, which can
be useful for describing random as well as deterministic scenarios.
The hexagonal pattern is achieved with the following config.xml file:

.. literalinclude:: ../config.xml
   :language: xml

The other host, ``host[1]``, uses :ned:`SuperpositioningMobility`, with the
superposition of an :ned:`AttachedMobility` and a :ned:`CircleMobility`. Here
is the configuration in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.numHosts = 2
   :end-at: -50mps

The mobility of ``host[1]`` is attached to ``host[0]``'s mobility,
without an offset. The :ned:`CircleMobility` in ``host[1]`` is configured
to circle around 0,0 with a radius of 50m. The constraint area of the
:ned:`CircleMobility` module is interpreted in the coordinate system of the
mobility module it is attached to. By default, it is limited between 0
and 400m as defined in the ``General`` configuration. Position 0,0 in
this coordinate system is at the position of ``host[0]``'s mobility. So
when ``host[1]`` starts to orbit point 0,0 (``host[0]``), the
:ned:`CircleMobility`'s coordinates would become negative, and the host
would bounce back. Thus, the constraint area of the :ned:`CircleMobility`
module needs to include negative values for the X and Y coordinates.

The following video shows the resulting movement of the hosts:

.. video:: media/Superpositioning2.mp4
   :width: 50%

Example: Mars Rover
-------------------

The following simulation shows how the :ned:`AttachedMobility` and
:ned:`SuperpositioningMobility` models can be used to orient antennas
independently from the orientation of their containing network node.
The simulation contains the following nodes:

- ``rover:`` A Mars rover moving along a straight line.
- ``drone:`` A drone orbiting around the center of the scene.
- ``base:`` A base station with a fixed position.

The following image shows the initial layout of the scene:

.. image:: media/AntennaOrientation_layout.png
   :width: 75%
   :align: center

By default, radio modules contain antenna submodules, whose positions are
taken into account when simulating wireless signal transmission and
reception. In network nodes (more specifically, in those that extend
:ned:`LinkLayerNodeBase`), the antenna module uses the containing network
node's mobility submodule to describe the antenna's position and
orientation. Thus, by default, the position and orientation of the
antenna (where the signal reception and transmission take place) are the
same as the position and orientation of the containing network node.
However, antenna modules have optional mobility submodules. The
antenna's mobility submodule allows the network node to have antennas
whose position and orientation are different from those of the network
node.

Such antennas can be oriented independently of the network node's
orientation. The antenna's position can also be independent of the
containing network node's position. However, in many cases, it makes more
sense to attach the antenna position to the network node's position,
with some offset. This allows for creating nodes that are extended objects
(as opposed to being point objects).

Note that antenna orientation is only relevant with directional
antennas. This example uses isotropic antennas (and there is no
communication) because the goal is to just demonstrate how antennas can
be oriented arbitrarily.

The scenario for the example simulation is the following: a Mars rover
prototype is being tested in the desert. The rover moves in a straight
line and has two antennas. It uses them to communicate with a base and
a nearby circling drone. Each antenna is oriented independently. One of
the antennas tracks the drone, the other one is directed at the base.

The ``rover`` is configured to use :ned:`LinearMobility` to move on the
scene. The ``drone`` uses :ned:`CircleMobility` to circle around the
center of the scene. The ``rover`` has two wireless interfaces, and
thus, two antennas, which each have a mobility submodule. The configuration in
omnetpp.ini related to antenna mobility is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: antenna.mobility
   :end-at: initFromDisplayString

Each antenna needs to be attached to the network node, and also face
towards its target. Thus each antenna has a :ned:`SuperpositioningMobility`
submodule. :par:`element[0]` of the array is an :ned:`AttachedMobility`, and the
antenna's position is attached to and offset from the position of the
host. ``element[1]`` is a :ned:`FacingMobility`, the antenna tracks its target.

For demonstration purposes, we chose the offsets of the antennas to be
unrealistically large.

The following video shows the results:

.. video:: media/AntennaOrientation.mp4
   :width: 75%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MobilityShowcase.ned <../MobilityShowcase.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/27>`__ in
the GitHub issue tracker for commenting on this showcase.
