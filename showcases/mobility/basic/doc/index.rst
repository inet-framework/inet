Mobility Models
===============

Goals
-----

The positioning and mobility of nodes play a crucial role in many simulation
scenarios, especially those that involve wireless communication. INET allows you
to add mobility to nodes by using modules that represent different mobility
models, such as linear motion or random waypoint. There is a wide variety of
mobility models available in INET and you can even combine them to create
complex motion patterns.

This showcase provides a demonstration of some of the elementary mobility models
available in INET. The topic of combining multiple mobility models is covered in
a separate showcase, :doc:`../../combining/doc/index`.

| INET version: ``4.0``
| Source files location: `inet/showcases/mobility/basic <https://github.com/inet-framework/inet/tree/master/showcases/mobility/basic>`__

Overview
--------

Mobility models are mostly used to describe the motion of wireless network nodes.
In wireless networks, the signal strength depends on the position and orientation of
transmitting and receiving nodes, which in turn determines the success
of packet reception. Because of this, position is always relevant,
even if the simulation is run without a GUI.

In INET, mobility is added to nodes in the form of modules that represent
mobility models. Mobility module types implement the :ned:`IMobility` module interface
and are in the `inet.mobility <https://github.com/inet-framework/inet/tree/master/src/inet/mobility>`__
package. This showcase presents an example for most of the frequently used mobility models.

Mobility models can be categorized in numerous different ways (see User's Guide),
but here we present them organized in two groups: ones describing proper
motion and/or dynamic orientation, and ones describing the placement and
orientation of stationary nodes.

For the interested reader, the `INET User’s Guide <https://inet.omnetpp.org/docs/users-guide/ch-mobility.html>`__
contains a more in-depth treatment of mobility models.


The Model
---------

All simulations use the :ned:`MobilityShowcase` network.
The size of the scene is 400x400x0 meters. It contains a configurable
number of hosts and an :ned:`IntegratedVisualizer` module to
visualize some aspects of mobility. The following image shows the layout of the network:

.. figure:: media/scene.png
   :scale: 100%
   :align: center

The ``General`` configuration in the ``omnetpp.ini`` contains some configuration
keys common to all example simulations:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: **.networkConfiguratorModule = ""
   :end-at: constraintAreaMaxZ

All visualization features of :ned:`MobilityVisualizer` are turned on, and
the constraint areas of all mobility modules are set to match the size
of the scene. All nodes will move in the XY plane, so the Z
coordinate is always set to 0.

The model does not need a network configurator module because there is
no communication between the hosts, so we set the configurator module path in
the hosts to the empty string.


Motion
------

This section covers mobility models that define motion. Such models in INET include
:ned:`AnsimMobility`,
:ned:`BonnMotionMobility`,
:ned:`ChiangMobility`,
:ned:`CircleMobility`,
:ned:`FacingMobility`,
:ned:`GaussMarkovMobility`,
:ned:`LinearMobility`,
:ned:`MassMobility`,
:ned:`Ns2MotionMobility`,
:ned:`RandomWaypointMobility`,
:ned:`RectangleMobility`,
:ned:`TractorMobility`,
:ned:`TurtleMobility`, and
:ned:`VehicleMobility`.

Some of these mobility models are introduced by the following example simulations.

LinearMobility
~~~~~~~~~~~~~~

The :ned:`LinearMobility` module describes linear motion with a constant speed or
constant acceleration. As such, it has parameters for speed, acceleration, and starting angle.
The model also has parameters for initial positioning (:par:`initialX`,
:par:`initialY`, :par:`initialZ`), which, by default, are random
values inside the constraint area.

The configuration in omnetpp.ini is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].mobility.typename = "LinearMobility"
   :end-at: speed

We leave both acceleration and angle on their default values, which is
zero for acceleration and a random value for the angle.

The following video shows the motion of the nodes:

.. video:: media/LinearMobility.mp4
   :width: 50%

The video shows that the seven hosts are placed randomly on the scene with a
random starting angle. They all move along a straight line with a constant speed.
They also bounce back from the boundaries of the constraint area when they reach it.


CircleMobility
~~~~~~~~~~~~~~

The :ned:`CircleMobility` module describes circular motion around a center.
This example uses two hosts orbiting the same center (the center of the scene)
with different radii, directions and speeds:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].mobility.typename = "CircleMobility"
   :end-at: *.host[1].mobility.startAngle = 270deg

You can see the result of the configuration on the following video:

.. video:: media/CircleMobility.mp4
   :width: 50%

TurtleMobility
~~~~~~~~~~~~~~

:ned:`TurtleMobility` is a programmable mobility model, where the "program" is provided
in the form of an XML script. The script can contain commands that set the position and speed,
turn by some specified angle, travel a certain distance, etc. These motion elements can be
used as building blocks for describing various motion patterns.

The mobility model's only ini parameter is :par:`turtleScript`, which
specifies the XML file to use:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].mobility.typename = "TurtleMobility"
   :end-at: *.host[1].mobility.turtleScript = xmldoc("config2.xml")

The simulation contains two hosts, of which ``host[0]`` moves along a perfect hexagon. Here you
can see the XML script for ``host[0]``:

.. literalinclude:: ../config.xml
   :language: xml

The flexibility of :ned:`TurtleMobility` allows the implementation of
the functionality of some of the other mobility models.
As such, ``host[1]``'s XML script describes another mobility model, :ned:`MassMobility`.
This is a mobility model describing a random motion. The node is assumed to have a mass, and so it
can not turn abruptly. This kind of motion is achieved by allowing only
small time intervals for forward motion, and small turning angles:

.. literalinclude:: ../config2.xml
   :language: xml

It looks like the following when the simulation is run:

.. video:: media/TurtleMobility.mp4
   :width: 50%

GaussMarkovMobility
~~~~~~~~~~~~~~~~~~~

:ned:`GaussMarkovMobility` model uses the Gauss-Markov mobility model
that involves random elements when describing the motion.
It has an :par:`alpha` parameter which can run from 0
(totally random motion) to 1 (deterministic linear motion), with the
default value of 0.5.
The random variable has a mean of 0, and its variance can be set by
the :par:`variance` parameter.
The :par:`margin` parameter adds a margin to the boundaries of the constraint
area, so that the mobility bounces back before reaching it.

Here is the configuration in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].mobility.typename = "GaussMarkovMobility"
   :end-at: alpha

The mobility module is set to totally random motion, with a variance of 0.5.

The following video shows the resulted random motion:

.. video:: media/GaussMarkovMobility.mp4
   :width: 50%

FacingMobility
~~~~~~~~~~~~~~

:ned:`FacingMobility` only affects orientation: it sets the orientation
of the mobility module to face towards another mobility module.
More precisely, the orientation is set to point from a source mobility module
to a target mobility module. Both can be selected by the :par:`sourceMobility`
and :par:`targetMobility` parameters.
By default, the :par:`sourceMobility` parameter is the mobility module
itself. The :par:`targetMobility` parameter has no default value.

For example, if ``host3``'s  :par:`sourceMobility` parameter is set to ``host1``'s
mobility module, the :par:`targetMobility` parameter to ``host2``'s mobility module,
the orientation of ``host3`` points in the direction of the
``host1``-``host2`` vector:

.. figure:: media/FacingMobility.png
   :width: 50%
   :align: center

Note that this screenshot is not from the example simulation; it is just for illustration.

The example simulation contains seven hosts.
``host[0]`` is configured to move around using :ned:`LinearMobility`, while the rest of the nodes
are configured to face ``host[0]`` using :ned:`FacingMobility`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[1..6].mobility.typename = "FacingMobility"
   :end-at: *.host[1..6].mobility.targetMobility = "^.^.host[0].mobility"

The following video shows how all of the nodes are facing ``host[0]`` as it moves:

.. video:: media/FacingMobility.mp4
   :width: 50%

BonnMotionMobility
~~~~~~~~~~~~~~~~~~

The :ned:`BonnMotionMobility` mobility model is a trace-based mobility model,
which means that node trajectories come from a pre-recorded trace file.
The :ned:`BonnMotionMobility` uses the native file format of the BonnMotion simulation tool.
The file is a plain text file, where every line describes the motion of one host.

A line consists of either (t, x, y) triplets or (t, x, y, z) quadruples. One tuple means
that the given node gets to the point (x,y,[z]) at the time t.
The :par:`is3D` boolean parameter controls whether lines are interpreted as consisting
of triplets (2D) or quadruples (3D). Here you can see the configuration in the ini file:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: **.host[*].mobility.typename = "BonnMotionMobility"
   :end-at: **.host[*].mobility.nodeId = -1

The :par:`nodeId` parameter selects the line in the trace file for the given mobility module.
The value -1 gets substituted to the parent module's index.

The ``bonnmotion.movements`` contains the trace that we want the nodes to follow:

.. literalinclude:: ../bonnmotion.movements
   :language: xml

If we take ``host[0]`` for example, it gets the first line of the file. It stays at the position
(100,100) for 1 second, then moves to the point (175,100) on a straight line and gets there at
t=3s, etc.

The movement of the nodes looks like the following when the simulation is run:

.. video:: media/BonnmotionMobility.mp4
   :width: 50%

Stationary Placement
--------------------

The following mobility models define only the stationary position and initial orientation: :ned:`StaticGridMobility`,
:ned:`StaticConcentricMobility`, :ned:`StaticLinearMobility`, :ned:`StationaryMobility`.

StaticGridMobility
~~~~~~~~~~~~~~~~~~

The example simulation is run with seven hosts.
:ned:`StaticGridMobility` positions all nodes in a rectangular grid. It has
parameters for setting the properties of the grid, such as a margin, the
number of row and columns, and the separation between rows and columns.
By default, the grid has the same aspect ratio as the available space
(constraint area by default). The :par:`numHosts` parameter must be set in
all mobility modules of the group.

The configuration in omnetpp.ini is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.*host[*].mobility.typename = "StaticGridMobility"
   :end-at: mobility.numHosts

We specify only the :par:`numHosts` parameter; the other parameters of the
mobility are left on their defaults. Thus the layout conforms to the
available space:

.. figure:: media/StaticGridMobility.png
   :scale: 100%
   :align: center

StationaryMobility
~~~~~~~~~~~~~~~~~~

The :ned:`StationaryMobility` model only sets position. It has :par:`initialX`, :par:`initialY`, :par:`initialZ`
and :par:`initFromDisplayString` parameters. By default, the :par:`initFromDisplayString` parameter is
true, and the initial coordinate parameters select a random value inside the constraint
area. Additionally, there are parameters to set the initial heading, elevation and bank
(i.e. orientation in 3D), all zero by default. Note that :ned:`StationaryMobility` is the
default mobility model in :ned:`WirelessHost` and derivatives.

The configuration for the example simulation in omnetpp.ini is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.numHosts = 3
   :end-at: *.host[2].mobility.initialHeading = 180deg

The configuration just sets the mobility type. Here is what it looks like when the simulation is run:

.. figure:: media/StationaryMobility.png
   :scale: 100%
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MobilityShowcase.ned <../MobilityShowcase.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/26>`__ in
the GitHub issue tracker for commenting on this showcase.
