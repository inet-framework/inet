Emulation basics
================

Goals
-----

This showcase demonstrates the basic features of the emulation support in INET.
Each configuration represents the same conceptual network but each of them divide the
network (or network nodes) into simulated and real-world parts in a different
way. The focus is on the different possibilities of this division of the conceptual
network, the required simulation and real-world configuration, and the operation.

INET version: ``4.1``

Source files location: `inet/showcases/emulation/basic <https://github.com/inet-framework/inet-showcases/tree/master/emulation/basic>`__

The network
-----------

All of the following examples present a simple conceptual network and a simple scenario.
The network consists of two hosts, one of which - may it be real or simulated
- pings the other one. In each configuration, a different part of the conceptual network
is extracted into the real world. The simulated part of the network is configured
form the ini file as usual, though not only the simulated, but the real part needs
to be configured as well. For each example, all commands for the configuration of
the host computer are contained by the ``Test<ConfigurationName>.sh`` shell scripts.
These scripts are self-contained and ready to be run under Linux.

In the configurations of the simulation, the sender host is always referred
as ``host1`` and the receiver host as ``host2``.

.. todo::

    <!-- TODO: rename the hosts to "sender" and "receiver" or something like that,
    because "host1" and "host2" could be a bit confusing, host1 is always the sender
    and host2 is always the receiver-->

.. todo::

    <!-- TODO: rename the titles of the single examples to something more sophisticated-->


ExtUpperEthernetInterfaceInHost1
--------------------------------

This example simulation presents a real ping application sending ping requests
to a simulated node. The separation between the simulation and the real world
takes place in the Ethernet interface of the sender host. The part of the network below
the Ethernet interface is run in the simulation, while the real host's protocol stack
is used above the Ethernet interface. This kind of separation can be achieved using
the :ned:`ExtUpperEthernetInterface` as the sender host's Ethernet interface.

The :ned:`ExtUpperEthernetInterface` represents an Ethernet network interface which
has its upper part in the real world and its lower part in the simulation. Packets
received by the simulated network interface from the network will be received on the
real interface. Packets received by the real network interface from above will be sent
out on the underlying simulated network interface. This device requires a TAP device
in the host OS. The simulation sends and receives packets through the TAP device using
the OS file API.

The sender host's :ned:`ExtUpperEthernetInterface` is configured as follows in the ini file:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.eth[0].typename = "ExtUpperEthernetInterface"
   :end-at: *.host1.eth[0].copyConfiguration = "copyFromExt"

The IP address and other configuratons of the Ethernet interface are not explicitly configured,
but instead they are set to be copied form the TAP device.

The only configuration needed for the receiver host is to set the IP address of its
Ethernet interface. This IP address is later also set as the destination address
of the ping request messages.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'
   :end-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'

With that done, the simulated part of the network is properly configured.
The further configurations affect the real OS and are contained in the
``TestExtUpperEthernetInterfaceInHost1.sh`` shell script.

In the host computer, the above mentioned TAP device needs to be created.
An IP address from within the same subnet as the receiver host needs to be assigned to it:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: sh
   :start-at: # create TAP interface
   :end-at: sudo ip link set dev tap0 up

After that, the simulation can be run. While it is running, the ping command with
the IP address of the simulated receiver host is run from the terminal.

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: sh
   :start-at: # run simulation
   :end-at: ping -c 2 -W 2 192.168.2.2 > ping.out

The real ping application of the host computer generates the ECHO REQUEST messages.
The message is routed through the virtual TAP interface. The sender node's Ethernet
interface reads the TAP device - this point is the bridge between the real operating environment
and the simulation. The ECHO REQUEST message is then sent to the receiver host
inside the simulation. This replies to the request by sending back an ECHO REPLY
message. The :ned:`ExtUpperEthernetInterface` of the sender host writes
the message into the TAP interface. The real ping application
of the OS receives the ECHO REPLY message and prints the result into the ``ping.out``
file.

The following schematic image illustrates the structure of the network and
the route of the messages:

.. figure:: ExtUpperEthernetInterfaceInHost1.png
   :width: 100%
   :align: center

After the simulation stops, the output of the pinging contained in the ``ping.out`` file are checked
and the TAP interface is destroyed:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: sh
   :start-at: # check output
   :end-at: sudo ip tuntap del mode tap dev tap0

ExtUpperEthernetInterfaceInHost2
--------------------------------

This time a simulated ping application sends ping requests to the host computer.
The real world and the simulation are separated in the Ethernet interface of the
receiver host. The simulation is run below the Ethernet interface, while the
host computer is above it. This kind of separation is again achieved using the
:ned:`ExtUpperEthernetInterface`:

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
sender host, it is configured in the ``omnetpp.ini`` file. The destination address
is set to the IP address that is going to be assigned to the TAP device.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.numApps = 1
   :end-at: *.host1.app[0].printPing = true

After the TAP device is created and brought up like in the previous example, the simulation
can be run. The simulated ping application of the sender host generates the
ECHO REQUEST messages. The message is routed towards the receiver host inside the
simulation. The receiver host's Ethernet interface writes the messages into the
TAP device. The host computer gets the requests and replies by sending an
ECHO REPLY message addressed to the sender host. The :ned:`ExtUpperEthernetInterface` of
the receiver host reads the TAP device and forwards the messages towards the sender host inside
the simulation. The results of the pinging can be found in the ``inet.out`` file.

ExtLowerEthernetInterfaceInHost2
--------------------------------

In this example, a real ping application in the sender host sends the ping request messages
to the simulated receiver host.
In the previous two examples, not only one of the hosts were simulated, but the link between them
as well. In this example however, the link will be also set up by the real OS.
For this kind of separation we use the :ned:`ExtLowerEthernetInterface` as the receiver host's
Ethernet interface. This represents an Ethernet network interface which has its lower part in the
real operating environment and its upper part in the simulation. The working mechanism is
similar to that of the :ned:`ExtUpperEthernetInterface`. Packets sent to the interface will be
sent out on the host OS interface, and packets received by the host OS interface will appear
in the simulation as if received on the interface. The lower part of the network interface is
realized in the real world using a real ethernet socket of the host computer which is running the
simulation. This device requires a pair of connected virtual ethernet interface in the host OS.

For the sender host is fully real, only the receiver host needs some configurations
in the ini file:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: description = "real ping application in host1 sends ping requests to host2 using ExtLowerEthernetInterface in host2"
   :end-at: *.host2.eth[0].copyConfiguration = "copyFromExt"

In the host OS, the above mentioned veth devices need to be created and brought up. A route
for the virtual link also needs to be added. The steps for the configuration are
contained in the ``TestExtLowerEthernetInterfaceInHost2.sh`` shell script:

.. literalinclude:: ../TestExtLowerEthernetInterfaceInHost2.sh
   :language: sh
   :start-at: # create virtual ethernet link: veth0 <--> veth1
   :end-at: sudo route add -net 192.168.2.0 netmask 255.255.255.0 dev veth0

After the veth pair is successfully created, the simulation can be run and the ping
application can be started:

.. literalinclude:: ../TestExtLowerEthernetInterfaceInHost2.sh
   :language: sh
   :start-at: # run simulation
   :end-at: ping -c 2 -W 2 192.168.2.2 > ping.out

The real ping application of the host computer generates the ECHO REQUEST messages.
The message is routed through the connected virtual ethernet interfaces ``veth0`` and ``veth1``.
The receiver node's Ethernet interface reads ``veth1`` and the ECHO REQUEST message is passed forward
inside the simulation. The receiver host gets the message and replies to it.
The :ned:`ExtLowerEthernetInterface` if the receiver host writes the message into
the veth device, which is then sent to the sender host in the real OS. The results
are saved into the ``ping.out`` file.

ExtLowerEthernetInterfaceInHost1
--------------------------------

This time a simulated ping application in the sender host sends ping request messages
to the real receiver host. The :ned:`ExtLowerEthernetInterface` is now used as the sender host's
ethernet interface.

Now that the sender host is the simulated one, besides its ethernet interface the ping
application needs to be configured as well:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: description = "simulated ping application in host1 sends ping requests to host2 using ExtLowerEthernetInterface in host1"
   :end-at: *.host1.eth[0].device = "veth0

The veth devices are created with the help of the script file. The destination address
for the ping application in the sender host needs to be the same as the IP address of ``veth1``.
When the veth devices are properly configured and brought up, the simulation can be started.

The simulated ping application in the sender host sends the PING REQUEST messages towards the receiver host.
The :ned:`ExtLowerEthernetInterface` of the sender host writes the message into the ``veth0``
virtuel ethernet device. The message the arrives at ``veth1`` and is processed be the host OS.
A PING REPLY message is then routed through the veth devices, where the :ned:`ExtLowerEthernetInterface`
of the sender host read ``veth0`` and processes the reply message. The output is written into the
``inet.out`` file.

ExtUpperIeee80211InterfaceInHost1
---------------------------------

Not only the Ethernet interface can be used as a separation point between the real and the simulated
parts of the network. The following two examples present the usage of the :ned:`ExtUpperIeee80211Interface` for
emulation.

In this example, the real ping application in the sender host sends ping requests to the simulated
receiver host using the :ned:`ExtUpperIeee80211Interface` as the sender host's wlan interface.
The configuration is very similar to the one in the ``ExtUpperEthernetInterfaceInHost1`` example, the
only difference being the type of the interface.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-after: description = "real ping application in host1 sends ping requests to host2 using ExtUpperIeee80211Interface in host1"
   :end-at: *.host1.wlan[0].copyConfiguration = "copyFromExt"

This device requires a TAP device in the host OS. The configurations needed for the host OS are also
identical to the previous ones.

The real ping application of the host computer generates the ECHO REQUEST messages.
The message is routed through the virtual TAP interface. The sender node's wlan
interface reads the TAP device - this point is the bridge between the real operating environment
and the simulation. The ECHO REQUEST message is then sent to the receiver host
inside the simulation. This replies to the request by sending back an ECHO REPLY
message. The :ned:`ExtUpperIeee80211Interface` of the sender host writes the message
into the TAP interface. The real ping application of the OS receives the ECHO REPLY
message and prints the result into the ``ping.out`` file.

ExtUpperIeee80211InterfaceInHost2
---------------------------------

.. todo::

    <!-- TODO: these examples do exactly the same as the Ethernet ones, feels a bit redundant
    to explain again-->

In this current example, a simulated ping application in the sender host sends ping request messages to
the receiver host using the :ned:`ExtUpperIeee80211Interface` as the receiver host's wlan interface.

This example is very similar to the ``ExtUpperEthernetInterfaceInHost2`` example. The only difference is,
that here we use the the host's wlan interface instead of its Ethernet interface.

After the TAP device is created and brought up like in the previous example, the simulation
can be run. The simulated ping application of the sender host generates the
ECHO REQUEST messages. The message is routed towards the receiver host inside the
simulation. The receiver host's wlan interface writes the messages into the
TAP device. The host computer gets the requests and replies by sending an
ECHO REPLY message addressed to the sender host. The :ned:`ExtUpperIeee80211Interface` of
the receiver host reads the TAP device and forwards the messages towards the sender host inside
the simulation. The results of the pinging can be found in the ``inet.out`` file.

Further Information
-------------------

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
