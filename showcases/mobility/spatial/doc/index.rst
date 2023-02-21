3D Mobility
===========

Goals
-----

Modeling the movement of objects in three dimensions is essential when
simulating scenarios involving aircraft, drones, and similar entities. This
showcase demonstrates how INET can be used to generate and visualize
three-dimensional mobility. This can be useful for understanding and evaluating
the behavior of communication systems in such environments.

| INET version: ``4.1``
| Source files location: `inet/showcases/mobility/spatial <https://github.com/inet-framework/inet/tree/master/showcases/mobility/spatial>`__

Overview
--------

In INET, mobility is added to nodes in the form of modules that represent
mobility models. A large number of mobility models have been provided with
INET, and they can also be combined. This showcase demonstrates producing
movement in three dimensions.

One way to generate spatial movement is to use mobility models
that support it out of the box, for example, :ned:`LinearMobility`,
:ned:`RandomWaypointMobility`, :ned:`MassMobility`, :ned:`TurtleMobility`
or :ned:`BonnMotionMobility`. Spatial movement can also be produced using
superposition of several mobility models (where at least one of them
must support movement in the Z axis). We show an example for both approaches.

In these example simulations, we'll make use of 3D visualization based on
OpenSceneGraph (OSG). To try these examples yourself, make sure that your
OMNeT++ installation has been compiled with OSG support. If it is not,
you won't be able to switch to 3D view using the globe icon on the Qtenv toolbar.

.. figure:: media/QtenvToolbar.png
   :scale: 100%
   :align: center

The model
---------

The simulations use the :ned:`MobilityShowcase` network. It contains a
configurable number of mobile nodes (hosts) and an :ned:`IntegratedVisualizer` module.

.. figure:: media/Playground2D.png
   :scale: 80%
   :align: center

Here are the key parts of the configuration regarding the visualization of the
scene in 3D:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # scene visualization
   :end-at: *.visualizer.osgVisualizer.sceneVisualizer.axisLength = 1000m

By default, :ned:`IntegratedVisualizer` only contains an :ned:`IntegratedCanvasVisualizer`
as submodule, but no OSG visualizer. To add it, we need to set the ``osgVisualizer`` submodule
type to :ned:`IntegratedOsgVisualizer`. We use the ``desert`` image as ground,
and set the background color (:par:`clearColor`) set to ``skyblue``.
The coordinate axes can be displayed by setting the :par:`axisLength` parameter.
Additional settings (not shown above) stretch the rendered scene a little larger
than the constraint area of the mobility models to enhance visual appearance.

Further settings enable various effects in the mobility visualization. Note, however, that
at the time of writing, not all features are implemented in :ned:`MobilityOsgVisualizer`
(practically, only trail visualization is).

When simulations are run, the scene looks like the following in 3D view:

.. figure:: media/3DPlayground.png
   :scale: 100%
   :align: center


Examples and Results
--------------------

Spiral
~~~~~~

The first example simulation is run using only one host. The 3D model of the host
can be set with the :par:`osgModel` parameter. In this example, we use ``glider.osgb``.
For better visibility, the glider is scaled up to 100 times of its original size. It
is also rotated by 180 degrees so that it faces forward as it moves.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].osgModel = "3d/glider.osgb
   :end-at: *.host[*].osgModel = "3d/glider.osgb

We configure the glider to move along a spiral path. This is achieved using the
:ned:`SuperpositioningMobility` mobility model with :ned:`LinearMobility` and
:ned:`CircleMobility` as its components. The superposition of a circular motion
in the XY plane and a linear motion in the positive Z direction will yield
a spiral path. This looks like the following in the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[0].mobility.typename = "SuperpositioningMobility"
   :end-at: *.host[0].mobility.element[1].speed = 20mps

The following video shows the resulting spiral motion:

.. video:: media/Spiral.mp4
   :width: 50%
   :align: center

Drones
~~~~~~

In this example we simulate the movement of 10 drones. The :par:`constraintAreaMinZ`
is set to 200 meters only because we do not want the drones to reach the ground.
In this example we use the ``drone.ive`` as the 3D OSG model of the hosts:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].osgModel = "3d/drone.ive
   :end-at: *.host[*].osgModel = "3d/drone.ive

The :ned:`MassMobility` model is used in the drones in order to achieve a lifelike
motion.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host[*].mobility.typename = "MassMobility"
   :end-at: *.host[*].mobility.faceForward = false

The speed of drones is updated at every change interval, but more importantly,
they also turn by a random but small angle. The change angle is nearly parallel
to the XY plane, but not quite: there is a random inclination
denoted by the :par:`rotationAxisAngle` parameter, which will cause the drones
to ramble in three dimensions.

The following video shows the movement of the drones:

.. video:: media/Drone.mp4
   :width: 50%
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MobilityShowcase.ned <../MobilityShowcase.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/28>`__ in
the GitHub issue tracker for commenting on this showcase.
