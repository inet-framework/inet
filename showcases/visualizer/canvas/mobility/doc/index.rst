Visualizing Node Mobility
=========================

Goals
-----

In wireless simulations, the movement of mobile nodes can be a crucial aspect that
greatly impacts communication among them. However, visually tracking the movement
can be challenging. INET provides a mobility visualizer that makes it easier to
follow mobile nodes and displays properties such as speed and direction.

This showcase demonstrates the capabilities of the mobility visualizer.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/mobility <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/mobility>`__

About the Visualizer
--------------------

In INET, the mobility of nodes can be visualized by :ned:`MobilityVisualizer`
module (included in the network as part of :ned:`IntegratedVisualizer`). By
default, mobility visualization is enabled; it can be disabled by
setting :par:`displayMovements` parameter to false.

By default, all mobilities are considered for the visualization. This
selection can be narrowed with the visualizer's :par:`moduleFilter`
parameter.

The visualizer has several important features:

-  **Movement Trail**: It displays a line along the recent path of
   movements. The trail gradually fades out as time passes. Color, trail
   length and other graphical properties can be changed with parameters
   of the visualizer.
-  **Velocity Vector**: Velocity is represented visually by an arrow.
   Its starting point is the node, and its direction coincides with the
   movement's direction. The arrow's length is proportional to the
   node's speed.
-  **Orientation Arc**: Node orientation is represented by an arc whose
   size is specified by the :par:`orientationArcSize` parameter. This value
   is the relative size of the arc compared to a full circle. The arc's
   default value is 0.25, i.e. a quarter of a circle.

These features are disabled by default; they can be enabled by setting
the visualizer's :par:`displayMovementTrails`, :par:`displayVelocities` and
:par:`displayOrientations` parameters to true.

Visualizing Mobility Features
-----------------------------

The following example shows how to enable mobility visualization
features. The simulation can be run by choosing the
``VisualizingFeatures`` configuration from the ini file.

Three nodes of the type :ned:`AdhocHost`, ``host1``, ``host2``, and
``host3``, are placed in the scene. They are roaming within
predefined borders.

The following video has been captured from the simulation. The default
settings of mobility visualization are used.

.. video:: media/NoFeatures_v0620.m4v
   :width: 80%

It is difficult to track the nodes because they are moving randomly and
quite fast. In our next experiment, we enable movement trails, velocity
vectors, and orientation arcs. We expect that nodes can be tracked
easier.

.. literalinclude:: ../omnetpp.ini
   :start-at: Movement trail settings
   :end-at: displayOrientations
   :language: ini

The following video shows what happens when we run the simulation.

.. video:: media/VisualizingFeatures_v0627.m4v
   :width: 80%

Compare this video to the previous one! The first thing you may notice
is that the hosts' movement is the same as in the previous video.
However, it is now possible to see that the movement of ``host3`` is not
actually random, but rather, it moves along a circle. The ``host1`` and
``host2`` nodes can also be easily tracked because of the visualization.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MobilityVisualizerShowcase.ned <../MobilityVisualizerShowcase.ned>`

More Information
----------------

This example only demonstrates the key features of mobility
visualization. For more information, refer to the :ned:`MobilityVisualizer`
NED documentation.


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/visualizer/canvas/mobility`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.0 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.0.*/showcases/visualizer/canvas/mobility && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.0
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/14>`__ in the GitHub issue tracker for commenting on this
showcase.
