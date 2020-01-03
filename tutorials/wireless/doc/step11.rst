Step 11. Adding obstacles to the environment
============================================

Goals
-----

In an attempt to make our simulation both more realistic and more
interesting, we add some obstacles to the playground.

In the real world, objects like walls, trees, buildings, and hills act as
obstacles to radio signal propagation. They absorb and reflect radio
waves, reducing signal quality and decreasing the chance of successful
reception.

In this step, we add a concrete wall to the model that sits between
hosts ``A`` and ``R1`` and see what happens. Since our model still uses
the unit disk radio and unit disk wireless medium models that do not model
physical phenomena, obstacle modeling will be very simple: all obstacles
completely absorb radio signals, making reception behind them
impossible.

The model
---------

First, we need to represent obstacles. In INET, obstacles are managed as
part of the :ned:`PhysicalEnvironment` module, so we need to add an
instance to the :ned:`WirelessB` network:



.. literalinclude:: ../WirelessC.ned
   :language: ned
   :start-at: network WirelessC

Obstacles are described in an XML file. An obstacle is defined by its
shape, location, orientation, and material. It may also have a name, and
one can define how it should be rendered (color, line width, opacity,
etc.) The XML format allows one to use predefined shapes like cuboid,
prism, polyhedron or sphere, and also to define new shapes that may be
reused for any number of obstacles. It is similar for materials: there
are predefined materials like concrete, brick, wood, glass, forest, and
one can also define new materials. A material is defined with its
physical properties like resistivity, relative permittivity, and relative
permeability. These properties are used in the computations of
dielectric loss tangent, refractive index, and signal propagation speed,
and ultimately in the computation of signal loss.

Our wall is defined in walls.xml, and the file name is given to
:ned:`PhysicalEnvironment` in its ``config`` parameter. The file contents:

.. literalinclude:: ../walls.xml
   :language: xml


Having obstacles is not enough in itself; we also need to teach the
model of the wireless medium to take them into account. This is done by
specifying an obstacle loss model. Since our model contains
``UnitDiskRadioMedium``, we specify :ned:`IdealObstacleLoss`. With
:ned:`IdealObstacleLoss`, obstacles completely block radio signals, making
reception behind them impossible.

The :ned:`IntegratedCanvasVisualizer` we use as the ``visualizer``
submodule in the network contains two submodules related to obstacles:
``physicalEnvironmentVisualizer`` displays the obstacles themselves, and
``obstacleLossVisualizer`` is responsible for visualizing the obstacle
loss of individual signals.

The wall in our simulation is at an elevation of 0m and is 4m high. So
far the hosts (more precisely, their antennas) were at 0m elevation, the
default setting; we change this to 1.7m so that the wall definitely
blocks their signals.

The configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Wireless11]
   :end-before: #---

Results
-------

At the beginning of the simulation, the initial route that was
established in previous steps (A-R1-B) cannot be established, because
the wall is between host A and R1. The wall is completely blocking
transmissions, therefore AODV establishes the A-R2-R1-B route. Host R2's
transmission is cut when R2 moves behind the wall. This time, however,
host R1 is available to relay host A's transmissions to host B. A new
route is formed, and traffic continues to use this route until host R1
moves out of communication range. After that, the A-R2-R3-B route is
used, as seen in the previous steps.

.. video:: media/wireless-step11-1.mp4
   :width: 655
   :height: 575

.. internal video recording, animation and playback speed 1, fadeOut animation time 1.5s

**Number of packets received by host B: 784**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WirelessC.ned <../WirelessC.ned>`,
:download:`walls.xml <../walls.xml>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.
