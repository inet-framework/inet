Modeling Directional Antennas
=============================

Goals
-----

Modeling directional antennas refers to the use of antenna models in simulations
to represent the directional characteristics of real-world antennas. Directional
antennas are designed to transmit and receive signals in a specific direction,
rather than uniformly in all directions. This can be useful in a variety of
wireless scenarios where the directionality of the antenna is important, such as
in long-range communication.

INET contains various models of directional
antennas, which can be used to simulate different types of antenna patterns. In
this showcase, we will highlight the antenna models that are available in INET
and provide an example simulation that demonstrates the directionality of five
different antenna models. Four of these models represent well-known antenna
patterns, while the fifth model is a universal antenna that can be used to model
any rotationally symmetrical antenna pattern. By the end of this showcase, you
will understand the different antenna models that are available in INET and how
they can be used to simulate the directionality of antennas.

| INET version: ``4.1``
| Source files location: `inet/showcases/wireless/directionalantennas <https://github.com/inet-framework/inet/tree/master/showcases/general/directionalantennas>`__

Concepts
--------

In INET, radio modules contain transmitter, receiver, and antenna
submodules. The success of receiving a wireless transmission depends on
the strength of the signal present at the receiver, among other things,
such as interference from other signals. Both the transmitter and the
receiver submodules use the antenna submodule of the radio when sending and
receiving transmissions.

The antenna module affects transmission and reception in multiple ways:

- The relative position of the transmitting and receiving antennas is used when calculating reception signal strength (e.g. attenuation due to distance).
- Antenna gain is applied to the signal power at transmission and reception. The applied gain depends on each antenna module's directional characteristics, and the relative position and the orientation of the two antennas. The gain can increase or decrease the received signal power.
- By default, the antenna uses the containing network node's mobility submodule to describe its position and orientation, i.e. it has the same position and orientation as the network node. However, antenna modules have optional mobility submodules of their own.
- All antenna modules have 3D directional characteristics.

INET contains the following antenna module types:

- Isotropic:

  - :ned:`IsotropicAntenna`: hypothetical antenna that radiates with the same intensity in all directions
  - :ned:`ConstantGainAntenna`: the same as :ned:`IsotropicAntenna`, but has a constant `gain` parameter (for testing purposes)

- Omnidirectional:

  - :ned:`DipoleAntenna`: models a `dipole antenna <https://en.wikipedia.org/wiki/Dipole_antenna>`__

- Directional:

  - :ned:`ParabolicAntenna`: models a `parabolic antenna <https://en.wikipedia.org/wiki/Parabolic_antenna>`__'s main radiation lobe, ignoring sidelobes
  - :ned:`CosineAntenna`: models directional antenna characteristics with a cosine pattern

- Other:

  - :ned:`AxiallySymmetricAntenna`: can model arbitrary antennas with axially symmetric radiation patterns
  - :ned:`InterpolatingAntenna`: can model antennas with arbitrary radiation patterns, using interpolation

The default antenna module in all radios is :ned:`IsotropicAntenna`.

Visualizing Antenna Directionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`RadioVisualizer` module can visualize antenna directional characteristics,
using its antenna lobe visualization feature. For example, the radiation patterns of
an isotropic and a directional antenna looks like the following:

.. figure:: media/antennalobe.png
   :width: 100%
   :align: center

The visualized lobes indicate the antenna gain.
At any given direction, the distance between the node and the boundary of the lobe shape
is a (linear or logarithmic, the default being the latter) function of the gain in that direction.
Details of the mapping can be tuned with the visualizer's parameters.
Dashed circles indicate the 0 dB gain and the maximum gain on the radiation pattern figure.

The visualization is actually a cross-section of the 3D radiation pattern.
By default, the cross-section plane is perpendicular to the current
viewing angle (however, one can specify other
planes in the antenna's local coordinate system).

For a more in-depth overview of antenna lobe visualization, read the :ref:`corresponding section <ug:sec:visualization:radio-state>`
in the INET User's Guide.
For the description of all parameters of the visualizer, check the NED documentation
of :ned:`RadioVisualizerBase`.

The Model and Results
---------------------

The showcase contains five example simulations, which demonstrate the
directional characteristics of five antenna models. The simulation uses
the ``DirectionalAntennasShowcase`` network:

.. figure:: media/network.png
   :width: 60%
   :align: center

The network contains two :ned:`AdhocHost`\ s, named ``source`` and
``destination``. There is also an :ned:`Ipv4NetworkConfigurator`, an
:ned:`IntegratedVisualizer`, and an :ned:`Ieee80211ScalarRadioMedium` module.

The source host is positioned in the center of the playground. The
destination host is configured to circle the source host, while the
source host pings the destination every 0.5 seconds. We'll use the ping
transmissions to probe the directional characteristics of ``source``'s
antenna, by recording the power of the received signal in
``destination``. The destination host will do one full circle around the
source. The distance of the two hosts will be constant to get meaningful
data about the antenna characteristics. We'll run the simulation with
five antenna types in ``source``: :ned:`IsotropicAntenna`,
:ned:`ParabolicAntenna`, :ned:`DipoleAntenna`, :ned:`CosineAntenna`, and
:ned:`AxiallySymmetricAntenna`. The destination has the default
:ned:`IsotropicAntenna` in all simulations.

The configurations for the five simulations differ in the antenna
settings only; all other settings are in the ``General`` configuration
section:

.. literalinclude:: ../omnetpp.ini
   :start-at: General
   :end-at: displayLink
   :language: ini

The source host is configured to send ping requests every 0.5s. This is
effectively the probe interval; the antenna characteristics data can be
made more fine-grained by setting a more frequent ping rate. The
destination is configured to circle the source with a radius of 150m.
The simulation runs for 360s, and the speed of ``destination`` is set so
it does one full circle. This way, when plotting the reception power,
the time can be directly mapped to the angle.

The visualizer is set to display antenna lobes in ``source`` (the
:par:`displayRadios` is the master switch in :ned:`RadioVisualizer`, so it
needs to be set to ``true``), and active data links (indicating successfully received
transmissions).

The antenna specific settings are defined in distinct configurations,
named according to the antenna type used (``IsotropicAntenna``,
``ParabolicAntenna``, ``DipoleAntenna``, ``CosineAntenna``, and
``AxiallySymmetricAntenna``).

Isotropic Antenna
~~~~~~~~~~~~~~~~~

The :ned:`IsotropicAntenna` is the default in all radio modules. This
module models a hypothetical antenna which radiates with the same power
in all directions. The module has no parameters. The
simulation configuration which demonstrates this antenna is :ned:`IsotropicAntenna` in :download:`omnetpp.ini <../omnetpp.ini>`.
The configuration just sets the antenna type in ``source``:

.. literalinclude:: ../omnetpp.ini
   :start-at: IsotropicAntenna
   :end-before: ParabolicAntenna
   :language: ini

When the simulation is run, it looks like the following:

.. video:: media/isotropic.mp4

.. internal video recording, animation speed 1, playback speed 21.88, normal run, zoom 1, crop 25 25 150 50, no dpi scaling

The radiation pattern is shown as a circle, as expected
(the isotropic antenna's radiation pattern is a sphere).

As the destination node circles the source node, we record the reception power of the frames.
Here is the reception power vs. direction plot:

.. figure:: media/isotropicchart.png
   :width: 100%
   :align: center

Parabolic Antenna
~~~~~~~~~~~~~~~~~

The :ned:`ParabolicAntenna` module simulates the radiation
pattern of the main lobe of a `parabolic antenna <https://en.wikipedia.org/wiki/Parabolic_antenna>`__,
such as this one:

.. figure:: media/parabolicantenna.jpg
   :width: 40%
   :align: center

The antenna module has the following parameters:

-  :par:`maxGain`: the maximum gain of the antenna in dB
-  :par:`minGain`: the minimum gain of the antenna in dB
-  :par:`beamWidth`: width of the 3 dB beam in degrees

The configuration for this antenna is :ned:`ParabolicAntenna` in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: ParabolicAntenna
   :end-before: DipoleAntenna
   :language: ini

When the simulation is run, it looks like the following:

.. video:: media/parabolic.mp4

.. <!--internal video recording, animation speed 1, playback speed 21.88, normal run, crop 25 25 150 750-->

The radiation pattern is a narrow lobe. Note that in directions away from
the main direction, the radiation pattern might appear to be zero, but
actually, it is just very small. Note the small protrusion to the left
on the following, zoomed-in image:

.. figure:: media/parabolicsidelobe.png
   :width: 10%
   :align: center

The ping probe messages are successfully received when the destination
node is near the main lobe of ``source``'s antenna.
Here is the reception power vs. direction plot (note that the destination host
starts at 90 degrees away from the main lobe axis, so that the main lobe is
more apparent on the reception power plot):

.. figure:: media/parabolicchart.png
   :width: 100%

Dipole Antenna
~~~~~~~~~~~~~~

The :ned:`DipoleAntenna` module models a dipole antenna. Antenna length can be specified with
a parameter. By default, the antenna is vertical. When viewed from above, its radiation pattern
is a circle, which would not be very interesting for demonstration. To make the simulation
more interesting, we set the antenna to point in the direction of the y axis.
The configuration in :download:`omnetpp.ini <../omnetpp.ini>` is the
following:

.. literalinclude:: ../omnetpp.ini
   :start-at: DipoleAntenna
   :end-before: CosineAntenna
   :language: ini

It looks like this when the simulation is run:

.. video:: media/dipole.mp4

.. <!--internal video recording, animation speed 1, playback speed 21.88, normal run, crop 25 25 150 750-->

The visualization shows the cross-section of the donut shape of the dipole antenna's
radiation pattern. As can be seen from the animation, there is no successful communication
when the destination node is near the antenna's axis due to low antenna gain in that direction.
Here is the reception power vs. direction plot:

.. figure:: media/dipolechart.png
   :width: 100%

Cosine Antenna
~~~~~~~~~~~~~~

The :ned:`CosineAntenna` module models a hypotethical antenna with a cosine-based
radiation pattern. This antenna model is commonly used in the real world to approximate
various directional antennas. The module has two parameters, :par:`maxGain`
and :par:`beamWidth`. The configuration in :download:`omnetpp.ini <../omnetpp.ini>` is the
following:

.. literalinclude:: ../omnetpp.ini
   :start-at: CosineAntenna
   :end-before: AxiallySymmetricAntenna
   :language: ini

It looks like the following:

.. video:: media/cosine.mp4

As can be seen on the video, the radiation pattern is similar to that of the parabolic antenna.
Here is the reception power vs. direction plot:

.. figure:: media/cosinechart.png
   :width: 100%

Axially Symmetric Antenna
~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`AxiallySymmetricAntenna` is a universal antenna model which can describe
any axially symmetrical radiation pattern. It can model an isotropic antenna, a parabolic
antenna, dipole antenna, and many other antenna types.

The radiation pattern is described by specifying
the gain at various angles on a half-plane attached to
the axis of symmetry.
The gain is then interpolated at the intermittent angles, and the pattern
is rotated around the axis to get the radiation pattern in 3D.

The antenna module has three parameters:
the :par:`gains` parameter takes a sequence of gain and angle pairs
(given in decibels and degrees), the first pair must be ``0 0``.
The angle is in the range of (0,180).
The axis of symmetry is given by the :par:`axisOfSymmetry` parameter, ``x`` by default.
There is also a :par:`baseGain` parameter (0 dB by default). The default for the :par:`gains`
parameters is ``"0 0"``, defaulting to an isotropic antenna.

We demonstrate :ned:`AxiallySymmetricAntenna` by modeling a real-world
16-element Yagi antenna, such as this one:

.. figure:: media/yagiantenna.jpg
   :width: 60%
   :align: center

The antenna type in ``source``'s radio is set to
:ned:`AxiallySymmetricAntenna`. We entered the characteristics data for the antenna
in the :par:`gains` parameter with 1 degree resolution.
We also set a :par:`baseGain` of 10 dB, because the gain data is given as relative
(i.e. the maximum gain is at 0 dB).

The simulation configuration is :ned:`AxiallySymmetricAntenna` in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: AxiallySymmetricAntenna]
   :end-at: gains
   :language: ini

As we run the simulation, we can see the radiation pattern displayed:

.. video:: media/axiallysymmetric.mp4

Here is the reception power vs. direction plot:

.. figure:: media/axiallysymmetricchart.png
   :width: 100%

Here is the same plot with a logarithmic scale, where the details
further from the main lobe are more apparent:

.. figure:: media/axiallysymmetricchart_log.png
   :width: 100%

Here are the results for all antennas on one plot, for comparison:

.. figure:: media/allantennaschart.png
      :width: 100%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`DirectionalAntennasShowcase.ned <../DirectionalAntennasShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/37>`__ page in the GitHub issue tracker for commenting on this showcase.
