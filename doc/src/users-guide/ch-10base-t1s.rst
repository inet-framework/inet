.. _ug:cha:10BASE-T1S:

Ethernet 10BASE-T1S
===================

This chapter outlines the INET Framework's implementation of select features from the IEEE 802.3cg-2019 standard.
Specifically, it covers 10BASE-T1S in both point-to-point and multidrop topologies, utilizing the Physical Layer
Collision Avoidance (PLCA) protocol.

Overview
--------

The Ethernet 10BASE-T1S model is a distinct implementation from the INET Framework's default Ethernet model. Although
the default The 10BASE-T1S model re-implements the default model's CSMA/CD functionality to have separate CSMA/CD MAC
and CSMA/CD PHY layers. This separation is essential for incorporating the Physical Layer Collision Avoidance (PLCA)
protocol between the MAC and PHY layers. The new model is geared towards multidrop link topologies and it offers the
following modules for this purpose:

- :ned:`EthernetPlcaHost` models a network node suitable for use in multidrop links
- :ned:`EthernetPlcaInterface` models a network interface suitable for use in multidrop links
- :ned:`EthernetPlca` implements the Ethernet Physical Layer Collision Avoidance (PLCA) protocol
- :ned:`EthernetCsmaMac` implements the Ethernet CSMA/CD MAC protocol suitable for use with PLCA
- :ned:`EthernetCsmaPhy` implements the Ethernet CSMA/CD PHY protocol suitable for use with PLCA
- :ned:`EthernetMultidropLink` implements the single twisted-pair cable suitable for use in multidrop links

Additionally, the model incorporates modules non-specific to 10BASE-T1S:

- :ned:`WireJunction` represents a universal wiring hub suitable for use in multidrop links

Point-to-point links
--------------------

For the Ethernet 10BASE-T1S point-to-point communication mode the default Ethernet model can be used, more specifically
the :ned:`EthernetLink` channel and the :ned:`EthernetInterface` module. In the network interface, the full-duplex
:ned:`EthernetMacPhy` module can be employed.

Multidrop links
---------------

To construct an Ethernet 10BASE-T1S multidrop link, connect multiple :ned:`EthernetPlcaHost` modules together. Utilize
:ned:`EthernetMultidropLink` channels and :ned:`WireJunction` modules to establish the network topology. Set the cable
length for each connection to define the signal propagation delay.

For mixed networks, you can incorporate other network node modules like :ned:`StandardHost`, :ned:`EthernetSwitch`, or
:ned:`TsnSwitch`. Replace their network interface directed towards the multidrop link with an :ned:`EthernetPlcaInterface`
module. Make sure every interface connected to the multidrop link utilizes the :ned:`EthernetPlcaInterface` module.

Result analysis
---------------

The 10BASE-T1S modules offer various statistics useful for evaluating network node performance on a multidrop link. For
a comprehensive list of statistics, check the :ned:`EthernetCsmaMac`, :ned:`EthernetPlca`, and :ned:`EthernetCsmaPhy`
modules.

Key statistics are:

- queue length
- MAC state machine state
- PLCA transmit opportunity ID
- PLCA control state machine state
- PLCA data state machine state
- PHY state machine state
- PHY transmitted signal type
- PHY received signal type

Understanding the operation of a multidrop network interface can become quite complex. Consider creating a chart with
multiple subplots and a shared X-axis, displaying various statistics simultaneously. To create such a chart:

- select the desired statistics on the 'Browse data' page of the result analysis tool
- choose 'Plot using line chart on separate axis with Matplotlib' from the context menu
- open the chart configuration dialog, go to the 'Line' tab and enable 'Display enums as colored strips'.

.. note::

   For long running simulations, the recorded statistical results may grow large, causing chart performance to degrade,
   especially for the colored strips displaying state machine states. To address this issue, open the chart configuration
   dialog and specify a start/end time on the 'Input' tab to narrow down the data to the relevant portion.
