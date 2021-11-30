.. _ug:cha:transmission-medium:

The Transmission Medium
=======================

.. _ug:sec:medium:overview:

Overview
--------

For wireless communication, an additional module is required to model
the shared physical medium where the communication takes place. This
module keeps track of transceivers, noise sources, ongoing
transmissions, background noise, and other ongoing noises.

It relies on several models:

#. signal propagation model

#. path loss model

#. obstacle loss model

#. background noise model

#. signal analog model

With the help of the above models, the medium module computes when,
where, and how signals arrive at receivers, including the set of
interfering signals and noises. In addition, the medium module also
contains various mechanisms and ways to improve the scalability of
wireless network simulations.

.. _ug:sec:medium:radiomedium:

RadioMedium
-----------

The standard transmission medium model in INET is :ned:`RadioMedium`.
:ned:`RadioMedium` is as an OMNeT++ compound module with several
replaceable submodules. It contains submodules for each of the above
models (signal propagation, path loss, etc.), and various caches for
efficiency.

Note that :ned:`RadioMedium` is an active compound module, that is, it
has an associated C++ class that encapsulates the computations.

:ned:`RadioMedium` contains its components as submodules with parametric
types:



.. code-block:: ned

   propagation: <default("ConstantSpeedPropagation")> like IPropagation;
   analogModel: <default("ScalarAnalogModel")> like IAnalogModel;
   backgroundNoise: <default("IsotropicScalarBackgroundNoise")> like IRadioBackgroundNoise
       if typename != "";
   pathLoss: <default("FreeSpacePathLoss")> like IPathLoss;
   obstacleLoss: <default("")> like IObstacleLoss
       if typename != "";
   mediumLimitCache: <default("MediumLimitCache")> like IMediumLimitCache;
   communicationCache: <default("VectorCommunicationCache")> like ICommunicationCache;
   neighborCache: <default("")> like INeighborCache
       if typename != "";

There are many preconfigured versions of :ned:`RadioMedium`:

-  For use with :ned:`UnitDiskRadio`: :ned:`UnitDiskRadioMedium`

-  For APSK radios: :ned:`ApskScalarRadioMedium`,
   :ned:`ApskDimensionalRadioMedium`,
   :ned:`ApskLayeredScalarRadioMedium`,
   :ned:`ApskLayeredDimensionalRadioMedium`,

-  For IEEE 802.11: :ned:`Ieee80211ScalarRadioMedium`,
   :ned:`Ieee80211DimensionalRadioMedium`,
   :ned:`Ieee80211LayeredScalarRadioMedium`,
   :ned:`Ieee80211LayeredDimensionalRadioMedium`,

-  For IEEE 802.15.4: :ned:`Ieee802154UwbIrRadioMedium`,
   :ned:`Ieee802154NarrowbandScalarRadioMedium`

The following sections describe the parts of the medium model.

.. _ug:sec:medium:propagation-models:

Propagation Models
------------------

When a transmitter starts to transmit a signal, the beginning of the
signal propagates through the transmission medium. When the transmitter
ends the transmission, the signal’s end propagates similarly. The
propagation model describes how a signal moves through space over time.
Its main purpose is to compute the arrival space-time coordinates at
receivers. There are two built-in models in INET, implemented as simple
modules:

-  :ned:`ConstantTimePropagation` is a simplistic model where the
   propagation time is independent of the traveled distance. The
   propagation time is simply determined by a module parameter.

-  :ned:`ConstantSpeedPropagation` is a more realistic model where the
   propagation time is proportional to the traveled distance. The
   propagation time is independent of the transmitter and receiver
   movement during both signal transmission and propagation. The
   propagation speed is determined by a module parameter.

The default propagation model is configured as follows:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !PropagationModelConfigurationExample
   :end-before: !End
   :name: Propagation model configuration example

A more accurate model could take into consideration the transmitter and
receiver movement. This effect becomes especially important for acoustic
communication, because the propagation speed of the signal is much more
comparable to the speed of the transceivers.

.. _ug:sec:medium:path-loss-models:

Path Loss Models
----------------

As a signal propagates through space its power density decreases. This
is called path loss and it is the combination of many effects such as
free-space loss, refraction, diffraction, reflection, and absorption.
There are several different path loss models in the literature, which
differ in their parameterization and application area.

In INET, a path loss model is an OMNeT++ simple module implementing a
specific path loss algorithm. Its main purpose is to compute the power
loss for a given signal, but it is also capable of estimating the range
for a given loss. The latter is useful, for example, to allow
visualizing communication range. INET contains a number of built-in path
loss algorithms, each comes with its own set of parameters:

-  :ned:`FreeSpacePathLoss` models line of sight path loss for air or
   vacuum.

-  :ned:`BreakpointPathLoss` refines it using dual slope model with two
   separate path loss exponents.

-  :ned:`LogNormalShadowing` models path loss for a wide range of
   environments (e.g. urban areas, and buildings)

-  :ned:`TwoRayGroundReflection` models interference between line of
   sight and single ground reflection.

-  :ned:`TwoRayInterference` refines the above for inter-vehicle
   communication.

-  :ned:`RicianFading` is a stochastical model for the anomaly caused by
   partial cancellation of a signal by itself.

-  :ned:`RayleighFading` is a stochastical model for heavily built-up
   urban environments when there is no dominant propagation along the
   line of sight.

-  :ned:`NakagamiFading` further refines the above two models for
   cellular systems.

The following example replaces the default free-space path loss model
with log normal shadowing:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !PathLossConfigurationExample
   :end-before: !End
   :name: Path loss configuration example

.. _ug:sec:medium:obstacle-loss-models:

Obstacle Loss Models
--------------------

When the signal propagates through space it also passes through physical
objects present in that space. As the signal penetrates physical
objects, its power decreases when it reflects from surfaces, and also
when it is absorbed by their material. There are various ways to model
this effect, which differ in the trade-off between accuracy and
performance.

In INET, an obstacle loss model is an OMNeT++ simple module. Its main
purpose is to compute the power loss based on the traveled path and the
signal frequency. The obstacle loss models most often use the physical
environment model to determine the set of penetrated physical objects.
INET contains a few built-in obstacle loss models:

-  :ned:`IdealObstacleLoss` model determines total or no power loss at
   all by checking if there is any obstructing physical object along the
   straight propagation path.

-  :ned:`DielectricObstacleLoss` computes the power loss based on the
   accurate dielectric and reflection loss along the straight path
   considering the shape, position, orientation, and material of
   obstructing physical objects.

By default, the medium module doesn’t contain any obstacle loss model,
but configuring one is very simple:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !ObstacleLossModelConfigurationExample
   :end-before: !End
   :name: Obstacle loss model configuration example

Statistical obstacle loss models are also possible but currently not
provided.

.. _ug:sec:medium:background-noise-models:

Background Noise Models
-----------------------

Thermal noise, cosmic background noise, and other random fluctuations of
the electromagnetic field affect the quality of the communication
channel. This kind of noise doesn’t come from a particular source, so it
doesn’t make sense to model its propagation through space. The
background noise model describes instead how it changes over space and
time.

In INET, a background noise model is an OMNeT++ simple module. Its main
purpose is to compute the analog representation of the background noise
for a given space-time interval. For example,
:ned:`IsotropicScalarBackgroundNoise` computes a background noise that
is independent of space-time coordinates, and its scalar power is
determined by a module parameter.

The simplest background noise model can be configured as follows:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !BackgroundNoiseModelConfigurationExample
   :end-before: !End
   :name: Background noise model configuration example

.. _ug:sec:medium:analog-models:

Analog Models
-------------

The analog signal is a complex physical phenomenon which can be modeled
in many different ways. Choosing the right analog domain signal
representation is the most important factor in the trade-off between
accuracy and performance. The analog model of the transmission medium
determines how signals are represented while being transmitted,
propagated, and received.

In INET, an analog model is an OMNeT++ simple module. Its main purpose
is to compute the received signal from the transmitted signal. The
analog model combines the effect of the antenna, path loss, and obstacle
loss models. Transceivers must be configured transmit and receive
signals according to the representation used by the analog model.

The most commonly used analog model, which uses a scalar signal power
representation over a frequency and time interval, can be configured as
follows:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !AnalogModelConfigurationExample
   :end-before: !End
   :name: Analog model configuration example

.. _ug:sec:medium:neighbor-cache:

Neighbor Cache
--------------

Transceivers are considered neighbors if successful communication is
possible between them. For wired communication it is easy to determine
which transceivers are neighbors, because they are connected by wires.
In contrast, in wireless communication determining which transceivers
are neighbors isn’t obvious at all.

In INET, a neighbor cache is an OMNeT++ simple module which provides an
efficient way of keeping track of the neighbor relationship between
transceivers. Its main purpose is to compute the set of affected
receivers for a given transmission. All built-in models in INET provide
a conservative approximation only, because they update their state
periodically:

-  :ned:`NeighborListNeighborCache` takes a range as parameter, and for
   each transceiver it maintains the list of receivers within range
   (*neighbor list*).

-  :ned:`GridNeighborCache` organizes transceivers in a 3D grid with
   constant cell size.

-  :ned:`QuadTreeNeighborCache` organizes transceivers in a 2D quad tree
   (ignoring the Z axis) with constant node size.

The following example sets :ned:`QuadTreeNeighborCache` as neighbor
cache:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !NeighborCacheModelConfigurationExample
   :end-before: !End
   :name: Neighbor cache model configuration example

How should one decide which neighbor cache to choose for a given
simulation? As the sole purpose of the neighbor cache is to speed up the
simulation, one should choose the one that leads to the best performance
for that particular network. Which one performs best is best determined
by experimentation, as it depends on many factors: number of nodes,
their spatial distribution, their speed and movement pattern, their
communication pattern, and so on. Note that not only the choice of
neighbor cache but also its parameterization can affect performance.

.. _ug:sec:medium:medium-limit-cache:

Medium Limit Cache
------------------

The medium limit cache (and its default implementation
:ned:`MediumLimitCache`) keeps track of certain thresholds and
minimum/maximum values of quantities related to layer 1 modeling. Some
of these limits can be gathered from other modules in the network, but
still, all of them can be explicitly specified by the user. The
quantities include:

-  maximum speed (can be gathered from mobility models)

-  maximum transmission power

-  minimum interference power and reception power

-  maximum antenna gain (can be computed from antenna models)

-  minimum time interval to consider two overlapping signals interfering

-  maximum duration of a transmission

-  maximum communication range and interference range (can be computed
   from transmitter and receiver models)

These limits allow the transmission medium model to make assumptions
about the locations of nodes (i.e. the maximum distance they can move
during some interval), about the possibility of interference, and about
the possibility of a signal being receivable.

.. _ug:sec:medium:communication-cache:

Communication Cache
-------------------

The communication cache is used to cache various intermediate
computation results related to the communication on the medium. The main
motivation to have multiple implementations is that different
implementations may be the most efficient in different simulations.
Also, a conservative (simple but robust) implementation may be used for
validating new (more efficient but also more complex) implementations.

Implementations include:

-  :ned:`ReferenceCommunicationCache`

-  :ned:`MapCommunicationCache`

-  :ned:`VectorCommunicationCache`

.. _ug:sec:medium:improving-scalability:

Improving Scalability
---------------------

The simulation of wireless networks is inherently less scalable than
that of wired networks. In wired networks, a transmission only affects
the host’s neighbors on the link, which is usually 1 in modern networks
that are dominated by point-to-point links. The wireless medium,
however, is a broadcast medium. Any transmission is “heard” by all nodes
within interference range, not only the intended recipients. The signal
may be receivable by them (and must be indeeded received before the
destination address field in it can be examined), or may interfere with
the reception of other transmissions. Whichever the case, the
transmission must be evaluated or processed by a much larger number of
nodes than in the wired case. This makes the computational complexity at
least :math:`O(n^2)` (:math:`n` being the number of nodes.) Other
effects may further increase the exponent.

The medium module provides a set of parameters that can be used to
alleviate the scalability issue. These *filter* parameters that can be
used to reduce the amount of processing at nodes that are not the
indended recipients of the frame, increasing simulation performance.

There are several filters that can be enabled/disabled individually:

-  *Range filter*. When this filter is active, the medium module does
   not send signals to a radio if it is outside interference range (or
   communication range, this option can also be selected.)

-  *Radio mode filter*. When this filter is active, the medium module
   does not send signals to a radio if it is neither in *receiver* nor
   in *transceiver* mode.

-  *Listening filter*. When this filter is active, the medium module
   does not send signals to a radio if it listens on the channel in
   incompatible mode (e.g. different carrier frequency and bandwidth, or
   different modulation)

-  *MAC address filter*. When this filter is active, the radio medium
   does not send signals to a radio if it the destination MAC address
   does not match

The corresponding module parameters are called ``rangeFilter``,
``radioModeFilter``, ``listeningFilter`` and
``macAddressFilter``. By default, all filters are turned off.
