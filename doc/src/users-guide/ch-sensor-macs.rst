.. _ug:cha:sensor-macs:

MAC Protocols for Wireless Sensor Networks
==========================================

.. _ug:sec:sensor-macs:overview:

Overview
--------

The INET Framework contains the implementation of several MAC protocols
for wireless sensor networks (WSNs), including B-MAC, L-MAC and X-MAC.

To create a wireless node with a specific MAC protocol, use a node type
that has a wireless interface, and set the interface type to the
appropriate type. For example, :ned:`WirelessHost` is a node type which
is preconfigured to have one wireless interface, ``wlan[0]``.
``wlan[0]`` is of parametric type, so if you build the network from
:ned:`WirelessHost` nodes, you can configure all of them to use e.g.
B-MAC with the following line in the ini file:



.. code-block:: ini

   **.wlan[0].typename = "BMacInterface"

.. _ug:sec:sensor-macs:b-mac:

B-MAC
-----

B-MAC (Berkeley MAC) is a carrier sense media access protocol for
wireless sensor networks that provides a flexible interface to obtain
ultra low power operation, effective collision avoidance, and high
channel utilization. To achieve low power operation, B-MAC employs an
adaptive preamble sampling scheme to reduce duty cycle and minimize idle
listening. B-MAC is designed for low traffic, low power communication,
and is one of the most widely used protocols (e.g. it is part of
TinyOS).

The :ned:`BMac` module type implements the B-MAC protocol.

:ned:`BMacInterface` is a :ned:`WirelessInterface` with the MAC type set
to :ned:`BMac`.

.. _ug:sec:sensor-macs:l-mac:

L-MAC
-----

L-MAC (Lightweight MAC) is an energy-efficient medium acces protocol
designed for wireless sensor networks. Although the protocol uses TDMA
to give nodes in the WSN the opportunity to communicate collision-free,
the network is self-organizing in terms of time slot assignment and
synchronization. The protocol reduces the number of transceiver state
switches and hence the energy wasted in preamble transmissions.

The :ned:`LMac` module type implements the L-MAC protocol, based on the
paper “A lightweight medium access protocol (LMAC) for wireless sensor
networks” by van Hoesel and P. Havinga.

:ned:`LMacInterface` is a :ned:`WirelessInterface` with the MAC type set
to :ned:`LMac`.

.. _ug:sec:sensor-macs:x-mac:

X-MAC
-----

X-MAC is a low-power MAC protocol for wireless sensor networks (WSNs).
In contrast to B-MAC which employs an extended preamble and preamble
sampling, X-MAC uses a shortened preamble that reduces latency at each
hop and improves energy consumption while retaining the advantages of
low power listening, namely low power communication, simplicity and a
decoupling of transmitter and receiver sleep schedules.

The :ned:`XMac` module type implements the X-MAC protocol, based on the
paper “X-MAC: A Short Preamble MAC Protocol for Duty-Cycled Wireless
Sensor Networks” by Michael Buettner, Gary V. Yee, Eric Anderson and
Richard Han.

:ned:`XMacInterface` is a :ned:`WirelessInterface` with the MAC type set
to :ned:`XMac`.
