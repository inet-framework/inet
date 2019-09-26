:orphan:

Visualizing Obstacle Loss
=========================

Goals
-----

Physical objects interacting with transmissions can lead to packet loss.
When radio signals pass through objects in the physical environment, the
objects reflect and absorb electromagnetic energy. The loss of energy
often results in incorrectly received packets. It's not immediately
obvious where the loss of energy happens, so visualizing it can
significantly facilitate understanding what happens in the simulation.

INET can visualize the intersections of obstacles and signals, with
several details.

This showcase contains two simulations, which demonstrate the
visualization of obstacle loss.

INET version: ``3.6``

Source files location: `inet/showcases/wireless/obstacleloss <https://github.com/inet-framework/inet-showcases/tree/master/visualizer/obstacleloss>`__

About the visualizer
--------------------

The ``ObstacleLossVisualizer`` module (part of the
:ned:`IntegratedVisualizer` included in the network) can display the
intersection between physical objects and propagating radio signals. It
can display the magnitude of power loss in dB. Additionally, the face
normal vectors can be displayed at both ends of the intersection.

The visualization is enabled with the ``displayIntersections``
parameter.

Enabling the visualization
--------------------------

The configuration uses the following network:

.. figure:: media/simplenetwork.png
   :width: 100%

It contains two ``AdhocHosts``, and a wall between them. The hosts are
stationary, and one of them is configured to ping the other. The wall
obstructs the signals.

The visualization is enabled with the ``displayIntersections``
parameter. The ``displayFaceNormalVectors`` parameter enables the face
normal vectors. The default color of the intersection lines is red,
which is hardly visible against the brick wall. Thus the color is
changed to yellow with the ``intersectionLineColor`` parameter:

.. code-block:: none

   *.visualizer.*.obstacleLossVisualizer.displayIntersections = true
   *.visualizer.*.obstacleLossVisualizer.displayFaceNormalVectors = true
   *.visualizer.*.obstacleLossVisualizer.intersectionLineColor = "yellow"

When the simulation is run, and ``source`` starts pinging
``destination``, the signals go through the wall:

.. figure:: media/simple2.png
   :width: 100%

The intersection of the wall and the transmission is indicated with a
yellow line. The magnitude of power loss is indicated in decibels.

Multiple Obstacles
------------------

The following configuration uses a more complex network:

.. figure:: media/advancednetwork.png
   :width: 100%

It contains two ``AdhocHosts``. In the middle of the playground, there
is a rectangular brick structure, with a hollow interior. The hosts are
configured to go around the structure on a rectangular path. The
``destinationHost`` is configured to move with a higher speed than the
``sourceHost``. The ``sourceHost`` is configured to ping
``destinationHost``. The following animation illustrates what happens
when the simulation is run:

.. video:: media/obstacleloss2_5.mp4
   :width: 698

The hosts send ping packets through the structure. Even when the
obstacles are not contiguous, the yellow lines are drawn wherever
transmissions and the obstacles intersect. The power loss values are
indicated in decibels, at each section of the wall the signals go
through. The face normal vectors are displayed at both ends of the
intersections, their relative length indicate the length of the
intersection.

By default, the intersection lines do not disappear immediatelly at the
end of the transmission, but gradually fade away. This allows you to see
all recent transmissions at once. This is demonstrated in the following
video, that shows the simulation running in fast mode:



.. video:: media/obstacleloss3.mp4"
   :width: 698

The fading time, as well several other parameters can be configured.

Further information
-------------------

For more information, refer to the ``ObstacleLossVisualizer`` NED
documentation.

Discussion
----------

Use `this page <TODO>`__ in the GitHub issue tracker for commenting on
this showcase.
