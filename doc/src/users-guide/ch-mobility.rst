.. role:: raw-latex(raw)
   :format: latex
..

.. _ug:cha:mobility:

Node Mobility
=============

.. _ug:sec:mobility:overview:

Overview
--------

In order to simulate ad-hoc wireless networks, it is important to model
the motion of mobile network nodes. Received signal strength, signal
interference, and channel occupancy depend on the distances between
nodes. The selected mobility models can significantly influence the
results of the simulation (e.g. via packet loss rates).

A mobility model describes position and orientation over time in a 3D
Euclidean coordinate system. Its main purpose is to provide position,
velocity and acceleration, and also angular position, angular velocity,
and angular acceleration data as three-dimensional quantities at the
current simulation time.

In INET, a mobility model is most often an OMNeT++ simple module
implementing the motion as a C++ algorithm. Although most models have a
few common parameters (e.g. for initial positioning), they always come
with their own set of parameters. Some models support geographic
positioning to ease the configuration of map based scenarios.

Mobility models be *single* or *group* mobility models. Single mobility
models describe the motion of entities independent of each other. Group
mobility models provide such a motion where group members are dependent
on each other.

Mobility models can also be categorized as *trace-based*,
*deterministic*, *stochastic*, and *combining* models.

TODO:

initial positioning vs movement (positioning over time) – example!

how to create initial layout + independent movements after

model controls what: position, orientation, or both (example: combining
facing with movement)

scope: single or group (making group mobility using superpositioning and
“attached”)

simple module or compound mobility, i.e. explain how use combining
mobility

method of configuration (ini+ned, vs. xml) “some models use XML or other
files”

Using Mobility Models
~~~~~~~~~~~~~~~~~~~~~

In order for a mobility model to actually have an effect on the motion
of a network node, the mobility model needs to be included as a
submodule in the compound module of the network node. By default, a
transceiver antenna within a network node uses the same mobility model
as the node itself, but that is completely optional. For example, it is
possible to model a vehicle facing forward while moving on a road that
contains multiple transceiver antennas at different relative locations
with different orientations.

The Scene
~~~~~~~~~

Many mobility models allow the user to define a cubic volume that the
node can not leave. The volume is configured by setting the
:par:`constraintAreaX`, :par:`constraintAreaY`, :par:`constraintAreaZ`,
:par:`constraintAreaWidth`, :par:`constraintAreaHeight` and
:par:`constraintAreaDepth` parameters.

If the :par:`initFromDisplayString` parameter, the initial position is
taken from the display string. Otherwise, the position can be given in
the :par:`initialX`, :par:`initialY` and :par:`initialZ` parameters. If
neither of these parameters are given, a random initial position is
choosen within the contraint area.

When the node reaches the boundary of the constraint area, the mobility
component has to prevent the node to exit. Many mobility models offer
the following policies:

-  reflect of the wall

-  reappear at the opposite edge (torus area)

-  placed at a randomly chosen position of the area

-  stop the simulation with an error

.. _ug:sec:mobility:built-in-mobility-models:


NEW STUFF
~~~~~~~~~

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
of packet reception. In order to simulate this, the :ned:`WirelessHost`
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
icons. For example, a :ned:`StandardHost` with p tag set to the coordinates
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
example, the :ned:`CircleMobility` module (describing circular motion) has
parameters to define the center and the radius of the circle, and also
the starting angle. The starting angle parameter is what does the
initial positioning, along the circle defined by the other parameters.

Generic Parameters
~~~~~~~~~~~~~~~~~~

All mobility modules in INET extend :ned:`MobilityBase`. It has some
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

The :ned:`MobilityVisualizer` module can display certain states of mobility
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

END NEW STUFF
~~~~~~~~~~~~~


Built-In Mobility Models
------------------------

.. _ug:sec:mobility:list-of-mobility-models:

List of Mobility Models
~~~~~~~~~~~~~~~~~~~~~~~

The following, potentially list contains the mobility models available
in INET. Nearly all of these models als single mobility models; group
mobility can be implemented e.g. with combining other mobility models.

Stationary
^^^^^^^^^^

Stationary models only define position (and orientation), but no motion.

-  :ned:`StationaryMobility` provides deterministic and random
   positioning.

-  :ned:`StaticGridMobility` places several mobility models in a
   rectangular grid.

-  :ned:`StaticConcentricMobility` places several models in a set of
   concentric circles.

Deterministic
^^^^^^^^^^^^^

Deterministic mobility models use non-random mathematical models for
describing motion.

-  :ned:`LinearMobility` moves linearly with a constant speed or
   constant acceleration.

-  :ned:`CircleMobility` moves around a circle parallel to the XY plane
   with constant speed.

-  :ned:`RectangleMobility` moves around a rectangular area parallel to
   the XY plane with constant speed.

-  :ned:`TractorMobility` moves similarly to a tractor on a field with a
   number of rows.

-  :ned:`VehicleMobility` moves similarly to a vehicle along a path
   especially turning around corners.

-  :ned:`TurtleMobility` moves according to an XML script written in a
   simple yet expressive LOGO-like programming language.

-  :ned:`FacingMobility` orients towards the position of another
   mobility model.

Trace-Based
^^^^^^^^^^^

Trace-based mobility models replay recorded motion as observed in real
life.

-  :ned:`BonnMotionMobility` replays trace files of the BonnMotion
   scenario generator.

-  :ned:`Ns2MotionMobility` replays files of the CMU’s scenario
   generator used in ns2.

-  :ned:`AnsimMobility` replays XML trace files of the ANSim (Ad-Hoc
   Network Simulation) tool.

Stochastic
^^^^^^^^^^

Stochastic or random mobility models use mathematical models involving
random numbers.

-  :ned:`RandomWaypointMobility` moves to random destination with random
   speed.

-  :ned:`GaussMarkovMobility` uses one parameter to vary the degree of
   randomness from linear to Brown motion.

-  :ned:`MassMobility` moves similarly to a mass with inertia and
   momentum.

-  :ned:`ChiangMobility` uses a probabilistic transition matrix to
   change the motion state.

Combining
^^^^^^^^^

Combining mobility models are not mobility models per se, but instead,
they allow more complex motions to be formed from simpler ones via
superposition and other ways.

-  :ned:`SuperpositioningMobility` model combines several other mobility
   models by summing them up. It allows creating group mobility by
   sharing a mobility model in each group member, separating initial
   positioning from positioning during the simulation, and separating
   positioning from orientation.

-  :ned:`AttachedMobility` models a mobility that is attached to another
   one at a given offset. Position, velocity and acceleration are all
   affected by the respective quantites and also the orientation of the
   referenced mobility.

.. _ug:sec:mobility:more-information-on-some-mobility-models:

More Information on Some Mobility Models
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TractorMobility
^^^^^^^^^^^^^^^

Moves a tractor through a field with a certain amount of rows. The
following figure illustrates the movement of the tractor when the
:par:`rowCount` parameter is 2. The trajectory follows the segments in
``1,2,3,4,5,6,7,8,1,2,3...`` order. The area is configured by the
:par:`x1`, :par:`y1`, :par:`x2`, :par:`y2` parameters.

.. PDF version f image:
   \setlength{\unitlength}{0.5mm}
   (80,80) (40,72):math:`1` (10,70)(1,0)30 (10,70)(1,0)60 (72,55):math:`2`
   (70,70)(0,-1)15 (70,70)(0,-1)30 (40,42):math:`3` (70,40)(-1,0)30
   (70,40)(-1,0)60 (5,25):math:`4` (10,40)(0,-1)15 (10,40)(0,-1)30
   (40,12):math:`5` (10,10)(1,0)30 (10,10)(1,0)60 (72,25):math:`6`
   (70,10)(0,1)15 (70,10)(0,1)30 (40, 33)\ :math:`7` (5,55):math:`8`
   (10,40)(0,1)15 (10,40)(0,1)30 (0,72):math:`(x_1,y_1)`
   (65,2):math:`(x_2,y_2)`

.. figure:: figures/tractormobility.png
   :align: center
   :width: 240

RandomWaypointMobility
^^^^^^^^^^^^^^^^^^^^^^

In the Random Waypoint mobility model the nodes move in line segments.
For each line segment, a random destination position (distributed
uniformly over the scene) and a random speed is chosen. You can
define a speed as a variate from which a new value will be drawn for
each line segment; it is customary to specify it as
``uniform(minSpeed, maxSpeed)``. When the node reaches the target
position, it waits for the time :par:`waitTime` which can also be
defined as a variate. After this time the the algorithm calculates a new
random position, etc.

GaussMarkovMobility
^^^^^^^^^^^^^^^^^^^

The Gauss-Markov model contains a tuning parameter that control the
randomness in the movement of the node. Let the magnitude and direction
of speed of the node at the :math:`n`\ th time step be :math:`s_n` and
:math:`d_n`. The next speed and direction are computed as

.. math:: s_{n+1} = \alpha s_n + (1 - \alpha) \bar{s} + \sqrt{(1-\alpha^2)} s_{x_n}

.. math:: d_{n+1} = \alpha s_n + (1 - \alpha) \bar{d} + \sqrt{(1-\alpha^2)} d_{x_n}

where :math:`\bar{s}` and :math:`\bar{d}` are constants representing the
mean value of speed and direction as :math:`n \to \infty`; and
:math:`s_{x_n}` and :math:`d_{x_n}` are random variables with Gaussian
distribution.

Totally random walk (Brownian motion) is obtained by setting
:math:`\alpha=0`, while :math:`\alpha=1` results a linear motion.

To ensure that the node does not remain at the boundary of the
constraint area for a long time, the mean value of the direction
(:math:`\bar{d}`) modified as the node enters the margin area. For
example at the right edge of the area it is set to 180 degrees, so the
new direction is away from the edge.

MassMobility
^^^^^^^^^^^^

This is a random mobility model for a mobile host with a mass. It is the
one used in :raw-latex:`\cite{Perkins99optimizedsmooth}`.

   "An MH moves within the room according to the following pattern. It
   moves along a straight line for a certain period of time before it
   makes a turn. This moving period is a random number, normally
   distributed with average of 5 seconds and standard deviation of 0.1
   second. When it makes a turn, the new direction (angle) in which it
   will move is a normally distributed random number with average equal
   to the previous direction and standard deviation of 30 degrees. Its
   speed is also a normally distributed random number, with a controlled
   average, ranging from 0.1 to 0.45 (unit/sec), and standard deviation
   of 0.01 (unit/sec). A new such random number is picked as its speed
   when it makes a turn. This pattern of mobility is intended to model
   node movement during which the nodes have momentum, and thus do not
   start, stop, or turn abruptly. When it hits a wall, it reflects off
   the wall at the same angle; in our simulated world, there is little
   other choice."

This implementation can be parameterized a bit more, via the
:par:`changeInterval`, :par:`changeAngleBy` and :par:`changeSpeedBy`
parameters. The parameters described above correspond to the following
settings:

-  changeInterval = normal(5, 0.1)

-  changeAngleBy = normal(0, 30)

-  speed = normal(avgSpeed, 0.01)

ChiangMobility
^^^^^^^^^^^^^^

Implements Chiang’s random walk movement model
(:raw-latex:`\cite{Chiang98wirelessnetwork}`). In this model, the state
of the mobile node in each direction (x and y) can be:

-  0: the node stays in its current position

-  1: the node moves forward

-  2: the node moves backward

The :math:`(i,j)` element of the state transition matrix determines the
probability that the state changes from :math:`i` to :math:`j`:

.. math::

   \left(
   \begin{array}{ccc}
     0 & 0.5 & 0.5 \\
     0.3 & 0.7 & 0 \\
     0.3 & 0 & 0.7
   \end{array}
   \right)

.. _ug:sec:mobility:replaying-trace-files:

Replaying trace files
~~~~~~~~~~~~~~~~~~~~~

BonnMotionMobility
^^^^^^^^^^^^^^^^^^

Uses the native file format of `BonnMotion <http://bonnmotion.net>`__.

The file is a plain text file, where every line describes the motion of
one host. A line consists of one or more (t, x, y) triplets of real
numbers, like:



::

   t1 x1 y1 t2 x2 y2 t3 x3 y3 t4 x4 y4 ...

The meaning is that the given node gets to :math:`(xk,yk)` at
:math:`tk`. There’s no separate notation for wait, so x and y
coordinates will be repeated there.

Ns2MotionMobility
^^^^^^^^^^^^^^^^^

Nodes are moving according to the trace files used in NS2. The trace
file has this format:



::

   # '#' starts a comment, ends at the end of line
   $node_(<id>) set X_ <x> # sets x coordinate of the node identified by <id>
   $node_(<id>) set Y_ <y> # sets y coordinate of the node identified by <id>
   $node_(<id>) set Z_ <z> # sets z coordinate (ignored)
   $ns at $time "$node_(<id>) setdest <x> <y> <speed>" # at $time start moving
   towards <x>,<y> with <speed>

The :ned:`Ns2MotionMobility` module has the following parameters:

-  :par:`traceFile` the Ns2 trace file

-  :par:`nodeId` node identifier in the trace file; -1 gets substituted
   by parent module’s index

-  :par:`scrollX`, :par:`scrollY` user specified translation of the
   coordinates

ANSimMobility
^^^^^^^^^^^^^

It reads trace files of the `ANSim <http://www.ansim.info>`__ Tool. The
nodes are moving along linear segments described by an XML trace file
conforming to this DTD:



.. code-block:: xml

   <!ELEMENT mobility (position_change*)>
   <!ELEMENT position_change (node_id, start_time, end_time, destination)>
   <!ELEMENT node_id (#PCDATA)>
   <!ELEMENT start_time (#PCDATA)>
   <!ELEMENT end_time (#PCDATA)>
   <!ELEMENT destination (xpos, ypos)>
   <!ELEMENT xpos (#PCDATA)>
   <!ELEMENT ypos (#PCDATA)>

Parameters of the module:

-  :par:`ansimTrace` the trace file

-  :par:`nodeId` the ``node_id`` of this node, -1 gets substituted to
   parent module’s index



.. note::

   The :ned:`AnsimMobility` module processes only the ``position_change``
   elements and it ignores the ``start_time`` attribute. It starts the move
   on the next segment immediately.

.. _ug:sec:mobility:turtlemobility:

TurtleMobility
~~~~~~~~~~~~~~

The :ned:`TurtleMobility` module can be parametrized by a script file
containing LOGO-style movement commands in XML format. The content of
the XML file should conform to the DTD in the
:file:`TurtleMobility.dtd` file in the source tree.

The file contains ``movement`` elements, each describing a
trajectory. The ``id`` attribute of the ``movement`` element can
be used to refer the movement from the ini file using the syntax:



.. code-block:: ini

   **.mobility.turtleScript = xmldoc("turtle.xml", "movements//movement[@id='1']")

The motion of the node is composed of uniform linear segments. The
``movement`` elements may contain the the following commands as
elements (names in parens are recognized attribute names):

-  ``repeat(n)`` repeats its content n times, or indefinitely if the
   ``n`` attribute is omitted.

-  ``set(x,y,speed,angle,borderPolicy)`` modifies the state of the
   node. ``borderPolicy`` can be ``reflect``, ``wrap``,
   ``placerandomly`` or ``error``.

-  ``forward(d,t)`` moves the node for ``t`` time or to the
   ``d`` distance with the current speed. If both ``d`` and
   ``t`` is given, then the current speed is ignored.

-  ``turn(angle)`` increase the angle of the node by ``angle``
   degrees.

-  ``moveto(x,y,t)`` moves to point ``(x,y)`` in the given time.
   If :math:`t` is not specified, it is computed from the current speed.

-  ``moveby(x,y,t)`` moves by offset ``(x,y)`` in the given time.
   If :math:`t` is not specified, it is computed from the current speed.

-  ``wait(t)`` waits for the specified amount of time.

Attribute values must be given without physical units, distances are
assumed to be given as meters, time intervals in seconds and speeds in
meter per seconds. Attibutes can contain expressions that are evaluated
each time the command is executed. The limits of the constraint area can
be referenced as ``$MINX``, ``$MAXX``, ``$MINY``, and ``$MAXY``. Random
number distibutions generate a new random number when evaluated, so the
script can describe random as well as deterministic scenarios.

To illustrate the usage of the module, we show how some mobility models
can be implemented as scripts.

RectangleMobility:



.. code-block:: xml

   <movement>
       <set x="$MINX" y="$MINY" angle="0" speed="10"/>
       <repeat>
           <repeat n="2">
               <forward d="$MAXX-$MINX"/>
               <turn angle="90"/>
               <forward d="$MAXY-$MINY"/>
               <turn angle="90"/>
           </repeat>
       </repeat>
   </movement>

Random Waypoint:



.. code-block:: xml

   <movement>
       <repeat>
           <set speed="uniform(20,60)"/>
           <moveto x="uniform($MINX,$MAXX)" y="uniform($MINY,$MAXY)"/>
           <wait t="uniform(5,10)">
       </repeat>
   </movement>

MassMobility:



.. code-block:: xml

   <movement>
       <repeat>
           <set speed="uniform(10,20)"/>
           <turn angle="uniform(-30,30)"/>
           <forward t="uniform(0.1,1)"/>
       </repeat>
   </movement>
