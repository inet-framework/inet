Step 11. Showing propagating signals
====================================

Goals
-----

OMNeT++ has default packet transmission animation but often we need a
more specific visualization. For example, radio signal propagates in all
direction of the space and the default animation can not display that.
INET offers a visualizer that can display propagating radio signal. It
can make it easy to see which nodes are transmitting at any given time
and what transmissions are present at various nodes. In this step, we
visualize propagating signals.

The model
---------

The simulation is configured as follows.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization11]
   :end-before: #---

Propagating signals are visualized by :ned:`MediumVisualizer` as animated
disks. ``Visualizer`` is enabled by setting ``displaySignals`` to true.
In 3D visualization, we have three options to customize shape for
displaying signals:

- *ring*: signal propagates on the ground as a ring
- *sphere*: signal propagates in the air
- *both*: it combines *ring* and *sphere* options.

.. todo::

   - propagationAnimationTime/speed
   - transmissionAnimationSpeed/time???

Results
-------

The following video shows what happens when we start the simulation.

.. video:: media/step11_result_2d_propsignal.mp4
   :width: 698

The signal propagation process consists of three phases:

1. Expansion
2. Presence
3. Recession

When the transmission starts, the signal expands from the transmitting
node, visualized by a growing disk. When the outer edge of the disk
reaches the farthest node from the source, the signal is displayed on
the playground as a stationary disk. At the end of the transmission, the
signal moves apart from the transmitting node, visualized by a receding
disk. It takes until the inner edge of the disk moves past the farthest
node from the source.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationD.ned <../VisualizationD.ned>`
