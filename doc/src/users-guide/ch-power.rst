.. _ug:cha:power:

Modeling Power Consumption
==========================

.. _ug:sec:power:overview:

Overview
--------

Modeling power consumption becomes more and more important with the
increasing number of embedded devices and the upcoming Internet of
Things. Mobile personal medical devices, large scale wireless
environment monitoring devices, electric vehicles, solar panels,
low-power wireless sensors, etc. require paying special attention to
power consumption. High-fidelity simulation of power consumption
allows designing power-sensitive routing protocols, MAC protocols with 
power management features, etc., which in turn results in more energy efficient
devices.

In order to help the modeling process, the INET power model is separated
from other simulation models. This separation makes the power model
extensible, and it also allows easy experimentation with alternative
implementations. In a nutshell, the power model consists of the following components:

-  energy consumption models

-  energy generation models

-  temporary energy storage models

The power model elements fall into two categories, abbreviated with
``Ep`` and ``Cc`` as part of their names:

-  ``Ep`` models are simpler, and deal with energy and power
   quantities.

-  ``Cc`` models are more realistic, and deal with charge, current,
   and voltage quantities.

The following sections provide a brief overview of the power model.

.. _ug:sec:power:energy-consumer-models:

Energy Consumer Models
----------------------

Energy consumer models describe the energy consumption of devices over
time. For example, a transceiver consumes energy when it transmits or
receives a signal, a CPU consumes energy when the network protocol forwards
a packet, and a display consumes energy when it is turned on.

In INET, an energy consumer model is an OMNeT++ simple module that
implements the energy consumption of software processes or hardware
devices over time. Its main purpose is to provide the power or current
consumption for the current simulation time. Most often energy consumers
are included as submodules in the compound module of the hardware
devices or software components.

INET provides only a few built-in energy consumer models:

-  :ned:`AlternatingEpEnergyGenerator` is a trivial energy/power based
   statistical energy consumer model example.

-  :ned:`StateBasedEpEnergyConsumer` is a transceiver energy consumer
   model based on the radio mode and transmission/reception states.

In order to simulate power consumption in a wireless network, the energy
consumer model type must be configured for the transceivers. The
following example demonstrates how to configure the power consumption
parameters for a transceiver energy consumer model:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !EnergyConsumerConfigurationExample
   :end-before: !End
   :name: Energy consumer configuration example

.. _ug:sec:power:energy-generator-models:

Energy Generator Models
-----------------------

Energy generator models describe the energy generation of devices over
time. A solar panel, for example, produces energy based on time, the
panel’s location on the globe, its orientation towards the sun and the
actual weather conditions. Energy generators connect to an energy
storage that absorbs the generated energy.

In INET, an energy generator model is an OMNeT++ simple module
implementing the energy generation of a hardware device using a physical
phenomena over time. Its main purpose is to provide the power or current
generation for the current simulation time. Most often energy generation
models are included as submodules in network nodes.

INET provides only one trivial energy/power based statistical energy
generator model called :ned:`AlternatingEpEnergyGenerator`. The
following example shows how to configure its power generation
parameters:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !EnergyGeneratorConfigurationExample
   :end-before: !End
   :name: Energy generator configuration example

.. _ug:sec:power:energy-storage-models:

Energy Storage Models
---------------------

Electronic devices which are not connected to external power source must
contain some component to store energy. For example, an electrochemical
battery in a mobile phone provides energy for its display, its CPU, and
its communication devices. It might also absorb energy produced by a
solar installed on its display, or by a portable charger plugged into
the wall socket.

In INET, an energy storage model is an OMNeT++ simple module which
models the physical phenomena that is used to store energy produced by
generators and provide energy for consumers. Its main purpose is to
compute the amount of available energy or charge at the current
simulation time. It maintains a set of connected energy consumers and
energy generators, their respective total power consumption and total
power generation.

INET contains a few built-in energy storage models:

-  :ned:`IdealEpEnergyStorage` is an idealistic model with infinite
   energy capacity and infinite power flow.

-  :ned:`SimpleEpEnergyStorage` is a non-trivial model integrating the
   difference between the total consumed power and the total generated
   power over time.

-  :ned:`SimpleCcBattery` is a more realistic charge/current based
   battery model using a charge independent ideal voltage source and
   internal resistance.

The following example shows how to configure a simple energy storage
model:



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !EnergyStorageConfigurationExample
   :end-before: !End
   :name: Energy storage configuration example

.. _ug:sec:power:energy-management-models:

Energy Management Models
------------------------

:ned:`SimpleEpEnergyManagement`



.. literalinclude:: lib/Snippets.ini
   :language: ini
   :start-after: !EnergyManagementConfigurationExample
   :end-before: !End
   :name: Energy management configuration example
