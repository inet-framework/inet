IEEE 802.15.4 Smart Home
========================

Goals
-----

IEEE 802.15.4 is a widely used standard for wireless communication in sensor networks,
which supports multiple options for the physical and MAC layers.
This showcase demonstrates the narrowband IEEE 802.15.4 model
available in INET. It contains an example simulation that features a
wireless sensor network.

| INET version: ``4.0``
| Source files location: `inet/showcases/wireless/ieee802154 <https://github.com/inet-framework/inet/tree/master/showcases/wireless/ieee802154>`__

The model
---------

IEEE 802.15.4 is a standard that defines the physical layer and media
access control (MAC) layer of low-rate wireless personal area networks
(LR-WPANs). LR-WPANs are low power, low throughput communication
networks, which can be used for creating wireless sensor networks
(WSNs), Internet-of-things applications, etc. It defines multiple
physical layer specifications (PHYs), based on different modulations,
such as Direct Sequence Spread Spectrum (DSSS), Chirp Spread Spectrum
(CSS), Ultra-wideband (UWB). It defines CSMA-CA and ALOHA MAC-layer
protocols as well.

The INET implementation
~~~~~~~~~~~~~~~~~~~~~~~

INET features a narrowband and an ultra-wideband IEEE 802.15.4 PHY
model:

- :ned:`Ieee802154NarrowbandScalarRadio`
- :ned:`Ieee802154UwbIrRadio`

This showcase demonstrates the narrowband model,
:ned:`Ieee802154NarrowbandScalarRadio`. It simulates a PHY that uses
DSSS-OQPSK modulation and operates at 2.45 GHz. By default, signals
are transmitted with a bandwidth of 2.8 MHz using 2.24 mW transmission
power. The model uses the scalar analog model.

The :ned:`Ieee802154NarrowbandInterface` module contains a
:ned:`Ieee802154NarrowbandScalarRadio` and the corresponding
:ned:`Ieee802154NarrowbandMac`. The radio medium module required by
the radio is :ned:`Ieee802154NarrowbandScalarRadioMedium`. As per the name, the
radio uses the scalar analog model for signal representation. The radio
has default values for its parameters, based on the 802.15.4 standard.
For example, by default, it uses the carrier frequency of 2450 MHz, 2.8
MHz bandwidth, 250 kbps bitrate, and 2.24 mW transmission power. As
such, it works "out-of-the-box", without setting any of its parameters.
The radio medium uses :ned:`BreakpointPathLoss` by default as its path loss
model. (Refer to the documentation to see all parameters and default values.)

The configuration
~~~~~~~~~~~~~~~~~

The showcase contains an example simulation, which demonstrates the
operation of INET's narrowband IEEE 802.15.4 model. The scenario is
that wireless nodes are used to control lighting in an apartment. There
are sensor nodes in the rooms working as presence sensors, detecting
when people are in a room. They periodically send sensor data to a
controller node, which decides how to adjust the lighting conditions in
different rooms. The controller sends control packets to the lamps in
the rooms to set their brightness or turn them on and off. All nodes use
the IEEE 802.15.4 narrowband model to communicate. Note that this is not
a working simulation of the light control and presence detection, just a
mockup based on that scenario - only the periodic communication of the
nodes is simulated.

The simulation can be run by choosing the ``Ieee802154`` configuration
from :download:`omnetpp.ini <../omnetpp.ini>`. It uses the following network:

.. figure:: media/network.png
   :width: 60%
   :align: center

The network contains 14 hosts of the :ned:`SensorNode` type, which has an
:ned:`Ieee802154NarrowbandInterface` by default. The network also contains
an :ned:`Ipv4NetworkConfigurator`, an :ned:`Ieee802154NarrowbandScalarRadioMedium`,
and an :ned:`IntegratedVisualizer` module.

Routes are set up according to a star topology, with the controller at
the center. This setup is achieved with the following configuration of
:ned:`Ipv4NetworkConfigurator` defined in the ``startopology.xml`` file:

.. literalinclude:: ../startopology.xml
   :language: xml

The following image shows the routes:

.. figure:: media/routes.png
   :width: 60%
   :align: center

All sensors will send packets to the controller, and the controller will
send packets to the lamps.

Here is the app configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: numApps
   :end-before: routing table visualization
   :language: ini

All sensors will send one 10-byte UDP packet to the controller each
second, with randomized start times. The controller will send one
10-byte UDP packet per second as well. The controller's app is an
:ned:`UdpBasicApp`, and all lamp nodes are specified in its ``destination``
parameter. If multiple destinations are specified in :ned:`UdpBasicApp`, a
random destination is chosen for each packet. Thus each packet will be
addressed to a different lamp.

All sensors will send one 10-byte UDP packet to the controller each
second, with randomized start times. The controller will send one
10-byte UDP packet per second as well. The controller's app is an
:ned:`UdpBasicApp`, and all lamp nodes are specified in its ``destination``
parameter (a random destination is chosen for each packet.)

The radio's parameters are not set; the default values will be used. We
arbitrarily set the background noise power to -110 dBm.

Results
-------

The following video shows the running simulation:

.. video:: media/Ieee802154_2.mp4
   :width: 500
   :align: center

Sensors send packets to the controller, each sensor sending one packet
per second. The controller sends 8 packets per second, to one of the
lamps selected randomly. Note that each packet transmission is followed
by an IEEE 802.15.4 ACK, but the ACKs are not visualized.

The :ned:`SensorNode` host type has energy storage and consumption modules
by default, so power consumption can be measured without adding any
modules to the hosts. :ned:`SensorNode` contains an
:ned:`IdealEpEnergyStorage` by default, and the radio in :ned:`SensorNode`
contains a :ned:`SensorStateBasedEpEnergyConsumer`. The
``residualEnergyCapacity`` statistic is available.

We want to measure the energy consumption of the different nodes in the
network. For this, we use the ``Ieee802154Power`` configuration in
:download:`omnetpp.ini <../omnetpp.ini>`. This configuration just extends the
``Ieee802154`` configuration with a simulation time limit of 100s:

.. literalinclude:: ../omnetpp.ini
   :start-at: Ieee802154Power
   :end-at: sim-time-limit
   :language: ini

We plotted the energy consumption (-1 \* ``residualEnergyCapacity``) of
all nodes on the following bar chart (values in Joules):

.. figure:: media/powerconsumption.png
   :width: 100%

The sensors consumed a bit more power than the lamps, and the controller
consumed the most energy. Nodes in the same role (i.e. lamps, sensors)
consumed roughly the same amount of energy. Although the controller
transmitted eight times as much as the sensors, it consumed just about
25% more energy. This is because energy consumption was dominated by the
idle radio state. The controller transmitted in about 8% of the time,
the sensors about 1%. Also, all transmissions were received by all nodes
in the network at the PHY level, thus they shared the energy consumption
due to reception. The small difference between the energy consumption of
the lamps and the sensors is because the sensors transmitted data, and
the lamps just transmitted ACKs (the data transmissions were longer than
ACK transmissions, 1.7 ms vs 0.3 ms.)

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`Ieee802154Showcase.ned <../Ieee802154Showcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/36>`__ page in the GitHub issue tracker for commenting on this showcase.
