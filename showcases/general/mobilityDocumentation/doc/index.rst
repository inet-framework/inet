:orphan:

Mobility Documentation
======================

In general, mobility models describe motion in a 3D right-handed
Euclidean coordinate system. More precisely, they describe how position
and orientation (and also their derivatives: velocity/acceleration,
angular velocity/acceleration) changes over time. Mobility models are
abstract models in the sense that they are not concerned with how the
motion is actually used (e.g. desribe the motion of a wireless network
node).

Coordinates are in X, Y, Z order measured in metres. Conceptually, the X
axis goes to the right, the Y axis goes forward, the Z axis goes upward.
The 2D canvas in Qtenv displays the scene looking towards positive Z by
default, so the X axis goes to the right and the Y axis goes downward.
Orientations are represented by 3D Tait-Bryan (Euler) angles measured in
radians. The angles are in Z, Y', X" order that is often called
intrinsic rotations.

.. <!-- How are mobility models implemented in INET?-->

In INET, mobility models are usually implemented as OMNeT++ simple
modules. There are several mobility modules, implementing various
mobility models. Some only determine initial position, some describe a
motion over time, some allows combining other mobility models to create
complex motions. The available mobility modules can be found in the
``inet/src/inet/mobility`` directory.

.. <!-- How are moblity models used? -->

But of course, modeling motion in itself is not very useful, so the
motion pretty much always has a subject. For example, mobility models
are used to describe the motion of wireless network nodes. In wireless
networks, the signal strength depends on the position and orientation of
transmitting and receiving nodes, which in turn determines the success
of packet reception. In order to simulate this, the ``WirelessHost``
module contains a mobility submodule which describes its motion.

In contrast, wired network nodes generally don't have mobility
submodules, because they are stationary and the position of wired

.. <!-- How are mobility modules connected to their subject modules? -->

There's an optional bidirectional mapping between the motion described
by the mobility module and the position of its subject module on the 2D
canvas in Qtenv. The bidirectional mapping is realized through reading
from and writing to the **display string p tag** of the subject module.

.. <!-- What is the display string p-tag and how does it work? -->

Submodules, such as network nodes, have optional "p" tags in their
display strings in the NED file, which tell the graphical runtime
environment where to draw the icons representing the modules on the
canvas (and also in the IDE). Display string p-tags are two-dimensional
(X and Y), and their purpose is to contol where to draw the submodule
icons. For example, a ``StandardHost`` with p tag set to the coordinates
(93,167):

.. literalinclude:: ../MobilityShowcase.ned
   :start-at: host1
   :end-at: display

Position is three-dimensional and is a state of mobility modules.
Position is used in the calculations of wireless communications, for
example. For example, the position of a host (the host's mobility
submodule selected in the inspector panel):

.. figure:: position_inspector.png
   :width: 50%
   :align: center

Position is always relevant, even if the simulation is run without a
GUI, whereas the display string is not. (not always)

.. <!-- How is the bidirectional mapping realized? -->

The two directions of the bidirectional mapping can be toggled
independently, with parameters.

In one direction, mobility modules can read the display string p tag of
a subject module, and initialize the position based on the coordinates
in the display string p tag. This mapping helps positioning using the
IDE, where you can drag submodules. The mapping is done only once, at
initialization, and works even if there is no GUI. The mapping is
controlled by the ``initFromDisplayString`` parameter, where applicable.

In the other direction, mobility modules can continuously update the
display string p tag of the subject module, according to the current
position. This mapping helps understanding motion in the runtime GUI,
where submodules move around on the parent compound module's canvas. The
mapping is controlled by the ``updateDisplayString`` paramater. The
mapping is only relevant when the simulation is run with a GUI.

The above effects pertain to the subject module of the mobility.

.. <!-- How can mobility modules get positions? -->

Without mobility, the position of nodes is not interpreted (in fact
doesn't exist), and not relevant. In this case, if the nodes have a
display string p tag, they are drawn in the GUI and IDE according to
that. If they don't have a p tag, the GUI places them with the layouter
algorithm (this can lead to different layouts when re-layout is
selected).

.. <!--how do nodes get positions when they have mobility?-->

When nodes have mobility, the situation is a bit more complex. They can
either get their initial position from the display string p tag, or from
the mobility module.

Some mobility modules have the ``initFromDisplayString`` and
``initialX``, ``initialY``, and ``initialZ`` parameters for initial
positioning. When a module has a p tag set, and the
``initFromDisplayString`` parameter is set to ``true`` in its mobility
module, the position will be initialized from the p tag. Otherwise, when
there is no p tag set or ``initFromDisplayString`` is ``false``, the
position will be set according the ``initialX``, ``initialY``, and
``initialZ`` parameters. The default for these three parameters is a
random value inside the constraint area.

Other mobility modules (e.g. those that describe completely
deterministic trajectory) initialize position with other parameters. For
example, the ``CircleMobility`` module (describing circular motion) has
parameters to define the center and the radius of the circle, and also
the starting angle. The starting angle parameter is what does the
initial positioning, along the circle defined by the other parameters.

Generic Parameters
~~~~~~~~~~~~~~~~~~

All mobility modules in INET extend ``MobilityBase``. It has some
generic parameters, which are thus common to all mobility modules, such
as the following:

-  The **constraint area**, as the name suggests, limits movement to an
   area (more precisely, volume) of definite size. By default, the
   constraint area is infinite (i.e. unconstrained). The constraint area
   is defined by six parameters, setting the two boundaries of the area
   per coordinate axis. The parameters are the following:
-  ``constraintAreaMinX``
-  ``constraintAreaMaxX``
-  ``constraintAreaMinY``
-  ``constraintAreaMaxY``
-  ``constraintAreaMinZ``
-  ``constraintAreaMaxZ``
-  ``displayStringTextFormat``: Contains directives as to what to
   display above mobility submodules in the GUI. For example, displaying
   position and velocity can be achieved with ``"p: %p\nv: %v"`` format
   string:
.. figure:: displaystringformat.png
   :width: 25%
   :align: center
-  ``visualRepresentation``: Selects which module is moved by this
   mobility module (parent module by default), more on this in a later
   section.

Visualizing state of mobility modules
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``MobilityVisualizer`` module can display certain states of mobility
modules:

-  **Position** as a colored dot (useful when the mobility doesn't have
   a subject module, i.e. doesn't move anything; ``displayPositions``
   parameter)
-  **Orientation** as a semi-transparent cone, representing angular
   position (``displayOrientations`` parameter)
-  **Velocity** as an arrow, representing the velocity vector
   (``displayVelocities`` parameter)
-  **Movement trail** as a fading path (``displayMovementTrails``
   parameter)

The ``displayMobility`` parameter is the master switch, toggling the
display of the above features.
