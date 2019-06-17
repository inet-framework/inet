Step 8. Modeling energy consumption
===================================

Goals
-----

Wireless ad-hoc networks often operate in an energy-constrained
environment, and thus it is useful to be able to model the energy
consumption of the devices. Consider, for example, wireless motes that
operate on battery. The mote's activity has to be planned so that the
battery lasts until it can be recharged or replaced.

In this step, we augment the nodes with components so that we can model
(and measure) their energy consumption. For simplicity, we ignore energy
constraints and just install infinite energy sources into the nodes.

The model
---------

Energy consumption model
~~~~~~~~~~~~~~~~~~~~~~~~

In a real system, there are energy consumers like the radio, and energy
sources like a battery or the mains. In the INET model of the radio,
energy consumption is represented by a separate component. This energy
consumption model maps the activity of the core parts of the radio (the
transmitter and the receiver) to power (energy) consumption. The core
parts of the radio themselves do not contain anything about power
consumption; they only expose their state variables. This allows one to
switch to arbitrarily complex (or simple) power consumption models,
without affecting the operation of the radio. The energy consumption
model can be specified in the ``energyConsumerType`` parameter of the
:ned:`Radio` module.

Here, we set the energy consumption model in the node radios to
:ned:`StateBasedEpEnergyConsumer`. :ned:`StateBasedEpEnergyConsumer` models
radio power consumption based on states like radio mode, transmitter and
receiver state. Each state has a constant power consumption that can be
set by a parameter. Energy use depends on how much time the radio spends
in a particular state.

To go into a little bit more detail: the radio maintains two state
variables, *receive state* and *transmit state*. At any given time, the
radio mode (one of *off*, *sleep*, *switching*, *receiver*,
*transmitter*, and *transceiver*) decides which of the two state
variables are valid. The receive state may be *idle*, *busy*, or
*receiving*, the former two referring to the channel state. When it is
*receiving*, a sub-state stores which part of the signal it is
receiving: the *preamble*, the (physical layer) *header*, the *data*, or
*any* (we don't know/care). Similarly, the transmit state may be *idle*
or *transmitting*, and a sub-state stores which part of the signal is
being transmitted (if any).

:ned:`StateBasedEpEnergyConsumer` expects the consumption in various states
to be specified in watts in parameters like ``sleepPowerConsumption``,
``receiverBusyPowerConsumption``,
``transmitterTransmittingPreamblePowerConsumption``, and so on.

Measuring energy consumption
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Hosts contain an energy storage component that basically models an
energy source like a battery or the mains. INET contains several energy
storage models, and the desired one can be selected in the
``energyStorageType`` parameter of the host. Also, the radio's energy
consumption model is preconfigured to draw energy from the host's energy
storage. (Hosts with more than one energy storage component are also
possible.)

In this model, we use :ned:`IdealEpEnergyStorage` in hosts.
:ned:`IdealEpEnergyStorage` provides an infinite amount of energy, can't be
fully charged or depleted. We use this because we want to concentrate on
the power consumption, not the storage.

The energy storage module contains an ``energyBalance`` watched variable
that can be used to track energy consumption. Also, energy consumption
over time can be obtained by plotting the ``residualEnergyCapacity``
statistic.

Visualization
~~~~~~~~~~~~~

The visualization of radio signals as expanding bubbles is no longer
needed, so we turn it off.

Configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Wireless08]
   :end-before: #---

Results
-------

The image below shows host A's ``energyBalance`` variable at the end of
the simulation. The negative energy value signifies the consumption of
energy.

.. figure:: media/wireless-step8-energy-2.png

The ``residualCapacity`` statistic of hosts A, R1 and B are plotted on the
following diagram. The diagram shows that host A has consumed the most
power because it transmitted more than the other nodes.

.. figure:: media/wireless-step8.png

**Number of packets received by host B: 1393**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WirelessB.ned <../WirelessB.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.
