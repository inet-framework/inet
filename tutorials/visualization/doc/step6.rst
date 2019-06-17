Step 6. Displaying movement of nodes
====================================

Goals
-----

In wireless networks with mobile nodes, mobility often affects network
operation. For this reason, visualizing mobility of nodes is essential.
When mobile nodes are roaming in the network, it is often difficult to
visually follow their movement. Visualizing movement of mobile nodes
makes visually tracking easier and also indicates other properties like
speed and direction. In this step, we visualize movement of mobile
nodes.

The model
---------

Movement of mobile nodes is displayed by default but often we need more
details about the recent and the upcoming movements.

Mobility settings
~~~~~~~~~~~~~~~~~

At first, we have to set mobility of network nodes.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # mobility settings
   :end-before: # mobility settings end

The above code snippet shows how the mobility of the nodes is
configured. The code is explained below. Initial position can be set to
all nodes by using - ``initialX``, ``initialY`` and ``initialZ`` - or
``initialLatitude``, ``initialLongitude`` and ``initialAltitude``
parameters.

In the first case, playground coordinates are used to place nodes on the
playground. In the second case, geographical coordinates are used and
these coordinates are converted before nodes will be placed on the
playground. For pedestrians, *Massmobility* is used as mobility type.
This is a random mobility model for a mobile host with a mass.
Pedestrians movement area is restricted by ``constraintAreaMinX``,
``constraintAreaMaxX``, ``constraintAreaMinY`` and
``constraintAreaMaxY``\ to prevent pedestrians roaming out from
communication range of ``accessPoint0``.

The following video displays nodes' mobility using default
visualization.

.. video:: media/step6_model_2d.mp4
   :width: 698

Visualization settings
~~~~~~~~~~~~~~~~~~~~~~
.. a velocity-re meg az orientation-re szukseg van?

Movement of nodes is visualized by default, but other useful
informations can also be visualized such as velocity, orientation and
movement trail. We enable these features by setting
``displayVelocities``, ``displayOrientations`` and
``displayMovementTrails`` to true. The pedestrians are moving slowly so
the length of velocity vector is too small. For this reason, we multiply
the length of velocity vector by the value of ``velocityArrowScale``.

The following code snippet shows how these features are configured.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # displaying movements
   :end-before: #---

Results
-------

.. video:: media/step6_result_2d.mp4
   :width: 698

.. todo::

   ![](step07_moving_2d.gif)
   ![](step5_result3.gif)
   It is advisable to run the simulation in Fast mode, because the nodes move very slowly if viewed in Normal mode.
   It can be seen in the animation below <i>pedestrian0</i> and <i>pedestrian1</i> roam in the park between invisible borders that we adjust to them.
   Here's that in Module view mode:
   And here's that in 3D Scene view mode:


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationD.ned <../VisualizationD.ned>`
