:orphan:

.. _dg:cha:mobility:

Node Mobility
=============

Mobility in INET
----------------

MobilityBase class
~~~~~~~~~~~~~~~~~~

The abstract :cpp:`MobilityBase` class is the base of the mobility
modules defined in the INET framework. This class implements things like
constraint area (or cubic volume), initial position, and border policy.

When the module is initialized, it sets the initial position of the node
by calling the :fun:`initializePosition()` method. The default
implementation handles the :par:`initFromDisplayString`,
:par:`initialX`, :par:`initialY`, and :par:`initialZ` parameters.

The module is responsible for periodically updating the position. For
this purpose, it should send timer messages to itself. These messages are
processed in the :fun:`handleSelfMessage` method. In derived classes,
:fun:`handleSelfMessage` should compute the new position, update the
display string, and publish the new position by calling the
:fun:`positionUpdated` method.

When the node reaches the boundary of the constraint area, the mobility
component has to prevent the node from exiting. It can call the
:fun:`handleIfOutside` method, which offers policies like reflect,
torus, random placement, and error.

MovingMobilityBase class
~~~~~~~~~~~~~~~~~~

The abstract :cpp:`MovingMobilityBase` class can be used to model mobilities
when the node moves on a continuous trajectory and updates its position
periodically. Subclasses only need to implement the :fun:`move` method, which is
responsible for updating the current position and speed of the node.

The abstract :fun:`move` method is called automatically every
:par:`updateInterval` steps. The method is also called when a client
requests the current position or speed or when the :fun:`move` method
requests an update at a future moment by setting the :var:`nextChange`
field. This can be used when the state of the motion changes at a
specific time that is not a multiple of :par:`updateInterval`. The
method can set the :par:`stationary` field to ``true`` to indicate
that the node has reached its final position and no more position updates are
needed.

.. graphviz:: figures/mobility_classes.dot
   :align: center

LineSegmentsMobilityBase
~~~~~~~~~~~~~~~~~~~~~~~~

The path of a mobile node often consists of linear movements of constant
speed. The node moves with some speed for some time, then with another
speed for another duration, and so on. If a mobility model fits this
description, it might be suitable to derive the implementing C++ class
from :cpp:`LineSegmentsMobilityBase`.

The module first chooses a target position and a target time by calling
the :fun:`setTargetPosition` method. If the target position differs
from the current position, it starts to move toward the target and
updates the position at the configured :par:`updateInterval` intervals.
When the target position is reached, it chooses a new target.