Emulation basics
================

Goals
-----

This showcase demonstrates the basic features of the emulation support in INET using
a simple network which contains two network nodes. Each configuration represents the
same conceptual network but they divide the network (or network nodes) into simulated
and real-world parts in a different way. The focus is on the different possibilities
of this division of the conceptual network, the required simulation and real-world
configuration, and the operation.

INET version: ``4.1``

Source files location: `inet/showcases/emulation/basic <https://github.com/inet-framework/inet-showcases/tree/master/emulation/basic>`__

The network
-----------

As mentioned above, all of the following examples present a simple conceptual network and a simple scenario.
The network consists of two hosts, one of which - may it be real or simulated
- pings the other one. In each configuration a different part of the conceptual network
is extracted into the real world, presenting the basics of the INET emulation feature.
The simulated part of the network is configured form the ini file as usual, though
not only the simulated, but the real part needs to be configured as well.
For each example, all commands for the configuration of the host computer
are contained by the ``Test<ConfigurationName>.sh`` shell scripts.
These scripts are self-contained and ready to be run under Linux.

In the configurations of the simulation, the sender host is always referred
as ``host1`` and the receiver host as ``host2``.

ExtUpperEthernetInterfaceInHost1
--------------------------------

This example simulation presents a real ping application sending ping requests
to a simulated node. The separation between the simulation and the real world
takes place in the Ethernet interface of the sender host. The part of the network below
the Ethernet interface is run in the simulation, while the real host, on which the
simulation is running, is used above the Ethernet interface. This kind of separation
can be achieved using the :ned:`ExtUpperEthernetInterface` in the sender host's
Ethernet interface.

The :ned:`ExtUpperEthernetInterface` represents an Ethernet network interface which
has its upper part in the real world and its lower part in the simulation. Packets
received by the simulated network interface from the network will be received on the
real interface. Packets received by the real network interface from above will be sent
out on the underlying simulated network interface. This device requires a TAP device
in the host OS. The simulation sends and receives packets through the TAP device using
the OS file API.

The :ned:`ExtUpperEthernetInterface` is used in the sender host's Ethernet interface:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.eth[0].typename = "ExtUpperEthernetInterface"
   :end-at: *.host1.eth[0].copyConfiguration = "copyFromExt"

The only configuration needed for the receiver host is to set the IP address of its
Ethernet interface. This is essential for the sender host to know how to route the
ping request messages.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'
   :end-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'

In the host computer, the above mentioned TAP device needs to be created.
An IP address with the same network number as the receiver host's needs to be assigned to it.
This step ensures that the messages are routed correctly between the two nodes. After that,
the simulation can be run. While it is running, we ping the IP address of the simulated
receiver host from the real host computer.
The real ping application of the host computer generates the ECHO REQUEST messages.
The messages are routed through the virtual TAP interface. The sender node's Ethernet
interface reads TAP device - this point is the bridge between the real operating environment
and the simulation. The ECHO REQUEST messages are then forwarded towards the receiver host
inside the simulation. It replies to the requests by sending ECHO REPLY
messages to the sender host. The :ned:`ExtUpperEthernetInterface` of the sender host writes
the messages into the TAP interface. The real ping application
of the OS receives the ECHO REPLY messages and prints the results into the ``ping.out``
file. The following schematic image shows the route of the messages through the
layers of the Internet protocol suite and between the hosts:

.. figure:: ExtUpperEthernetInterfaceInHost1.png
   :width: 100%
   :align: center

ExtUpperEthernetInterfaceInHost2
--------------------------------

This time a simulated ping application sends ping requests to the host computer.
The real world and the simulation are separated in the Ethernet interface of the
receiver host. The simulation is run below the Ethernet interface, while the
host computer is above it. This kind of separation is again achieved using the
:ned:`ExtUpperEthernetInterface`, but now it is used in the receiver host's
Ethernet interface:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host2.eth[0].typename = "ExtUpperEthernetInterface"
   :end-at: *.host2.eth[0].copyConfiguration = "copyFromExt"

The IP address of the simulated sender host's Ethernet interface is set as
the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.configurator.config = xml("<config><interface hosts='host1' names='eth0' address='192.168.2.1' netmask='255.255.255.0'
   :end-at: *.configurator.config = xml("<config><interface hosts='host1' names='eth0' address='192.168.2.1' netmask='255.255.255.0'

Since the ping application sending the ping request messages is in the simulated
sender host, it is configured from the ``omnetpp.ini`` file. The destination address
is set to the IP address we assign to the TAP device that we create in the host computer.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.numApps = 1
   :end-at: *.host1.app[0].printPing = true

After the TAP device is created and brought up, the simulation can be run.
The simulated ping application of the sender host generates the
ECHO REQUEST messages. The messages are routed towards the receiver host inside the
simulation. The receiver host's Ethernet interface writes the messages into the
TAP device. The host computer gets the requests and replies by sending
ECHO REPLY messages addressed to the sender host. The :ned:`ExtUpperEthernetInterface` of
the receiver host reads the TAP device and forwards the messages towards the sender host inside
the simulation. Since the ping application sending the ECHO REQUEST messages
is fully simulated, the results of the pinging can be found in the ``inet.out`` file.

ExtLowerEthernetInterfaceInHost2
--------------------------------

ExtLowerEthernetInterfaceInHost1
--------------------------------

ExtUpperIeee80211InterfaceInHost1
---------------------------------

ExtUpperIeee80211InterfaceInHost2
---------------------------------

Results
-------

Conclusion
----------

Further Information
-------------------

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
