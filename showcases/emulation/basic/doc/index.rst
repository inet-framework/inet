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

The model
---------

This showcase presents many different scenarios in order to explain how
the real world and the simulated environment can be connected at
different parts of a network.

All of these examples contain two hosts, one of which - may it be real or simulated -
pings the other one.

%TODO: instead of first describing what is ExtUpperEthernetInterface and how it works, we should first describe how is the conceptual network divided between the real world and the simulation
%TODO: suggested document structure
% 1 the conceptual network: two nodes, one pings the other
% 2.1 scenario 1: where is the separation between the simulation and the real world: in the ethernet interface of the sender host, simulation is used below the ethernet interface, host is used above the ethernet interface
% 2.2 scenario 1: what component is used (e.g. ExtUpperEthernetInterface) to configure scenario 1 and how does this component operate?
% 2.3 what configuration is used in the simulation, explain why, etc.
% 2.4 what configuration is necessary in the host computer, explain why, etc.
% 2.5 follow the ping requests/replies in scenario 1, explain how it works
% 3.1 scenario 2: where is the separation between the simulation and the real world: in the ethernet interface of the receiver host...
% etc.

Using :ned:`ExtUpperEthernetInterface`
--------------------------------------

%TODO: add a short description how ExtUpperEthernetInterface operates wrt. connecting the simulation and the real world, see the following from the INET section of the OMNeT++ ecosystem book:
%TODO: this should be reworded, shortened, I don't know, but don't take this over verbatim
%
%ExtLowerEthernetInterface represents an Ethernet network interface which has
%its upper part in the simulation and its lower part in the real world. This interface
%allows, for example, using a real network interface in a simulation. Packets
%received by the simulated network interface from above will be sent out on the
%underlying real network interface. Packets received by the real network interface
%(or rather, the appropriate subset of them) from the network will be received on
%the simulated network interface. This module requires a real or a virtual Ethernet
%interface on the host OS. The simulation sends and receives packets through this
%network interface using the OS Socket API.
%
%ExtUpperEthernetInterface represents an Ethernet network interface which has
%its upper part in the real world and its lower part in the simulation. This interface
%allows, for example, using a real routing protocol in a simulation. Basically, this
%interface works the opposite way as ExtLowerEthernetInterface. Packets received
%by the simulated network interface from the network will be received on the real
%interface. Packets received by the real network interface from above will be sent
%out on the underlying simulated network interface. This module requires a TAP
%device in the host OS. The simulation sends and receives packets through the TAP
%device using the OS file API.

The :ned:`ExtUpperEthernetInterface` looks like the following when the simulation
is run in the GUI:

.. figure:: ExtUpperEthernetInterface.png
   :scale: 100%
   :align: center

The ``ExtUpper`` part in the name of the module means that the upper part of the
functionality of the interface is external.
The :ned:`ExtEthernetTapDeviceFileIo` (``tap``) module provides connection to a real TAP
interface of the host computer which is running the simulation.
%TODO: it's not entirely clear what is the purpose of the TAP interface, see above

%TODO: this is a too generic sentence and not too informative, should be replaced with a more specific one or removed
In order for these emulation examples to work, some configuration of the real operating environment
is needed as well.

%TODO: the shell scripts should only be mentioned at the end, that they contain all the necessary commands, and that they are self contained and ready to be run
%TODO: while you are explaining the operation, it's not important to mention them, just explain the individual parts
For each configuration, the ``Test<ConfigurationName>.sh`` file contains the commands,
that we need to execute in the terminal under Linux. Now, we will only look at some of the
common steps.

First of all, the above mentioned virtual TAP interface needs to be created with a specific IP
address:

.. literalinclude:: CreateTap.sh
   :language: xml
   :start-at: # create TAP interface
   :end-at: sudo ip link set dev tap0 up

As soon as the TAP interface (``tap0``) is brought up, a new entry appears
in the routing table of the OS:

.. figure:: new_entry.png
   :scale: 100%
   :align: center

When the virtual TAP interface is up and running, the simulation can be run
as well. This step will be explained later.

After the simulation, the TAP interface can be destroyed with
the following commands:

.. literalinclude:: CreateTap.sh
   :language: xml
   :start-at: # destroy TAP interface
   :end-at: sudo ip tuntap del mode tap dev tap0


ExtUpperEthernetInterfaceInHost1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first example uses the ``NoExtHostsEthernetShowcase`` network:

.. figure:: NoExtHostsEthernetShowcase.png
   :scale: 100%
   :align: center

This example demonstrates how a real ping application can send echo
request messages to a simulated host, which then replies. This communication
between the real and the simulated word can be achieved by using the
:ned:`ExtUpperEthernetInterface` in ``host1``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.eth[0].typename = "ExtUpperEthernetInterface"
   :end-at: *.host1.eth[0].copyConfiguration = "copyFromExt"

In the simulation, the IP address of ``host2``'s Ethernet interface is set to
192.168.2.2/24:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'
   :end-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'

After creating the TAP interface using the script from the
``TestExtUpperEthernetInterfaceInHost1.sh`` file, the simulation is run from
the terminal and the textual results are saved in the ``inet.out`` file:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # run simulation
   :end-at: inet -u Cmdenv -c ExtUpperEthernetInterfaceInHost1 --sim-time-limit=2s &> inet.out &

While the simulation is running, we can ping into it using the computers ping application.
This can be done by pinging the IP address 192.168.2.2, the IP address
of the simulated ``host2`` module. The output is saved as ``ping.out`` file for later examination:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # ping into simulation
   :end-at: ping -c 2 -W 2 192.168.2.2 > ping.out

This way the real ping application of the OS generates the ECHO REQUEST messages.
Because of the above mentioned entry in the routing table, the messages are
routed through the virtual TAP interface. ``host1``'s Ethernet interface reads
``tap0`` - this point is the bridge between the real operating environment
and the simulation. The ECHO REQUEST messages are then forwarded towards ``host2``
inside the simulation. ``host2`` replies to the requests by sending ECHO REPLY
messages to ``host1``. The :ned:`ExtUpperEthernetInterface` of ``host1`` writes
the messages into the ``tap0`` virtual TAP interface. The real ping application
of the OS receives the ECHO REPLY messages and prints the results into the ``ping.out``
file.

We can check whether or not the pinging was successful by checking the content
of the ``ping.out`` file with the following command:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # check output
   :end-at: if grep -q "from 192.168.2.2" "ping.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; firm *.out

Or we can also open the ``ping.out`` file to see the textual results of the
ping.

.. literalinclude:: ExtUpperEthernetInterfaceInHost1_ping.out
   :language: xml

ExtUpperEthernetInterfaceInHost2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using :ned:`ExtEthernetLowerInterface`
--------------------------------------

ExtLowerEthernetInterfaceInHost2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ExtLowerEthernetInterfaceInHost1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using :ned:`ExtUpperIeee80211Interface`
---------------------------------------

ExtUpperIeee80211InterfaceInHost1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ExtUpperIeee80211InterfaceInHost2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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





.. note::

   Linux and most other operating systems have the ability to
   create virtual interfaces which are usually called TUN/TAP devices.
   Typically a network device in a system, for example eth0, has a physical
   device associated with it which is used to put packets on the wire. In
   contrast a TUN or a TAP device is entirely virtual and managed by the
   kernel. User space applications can interact with TUN and TAP devices as
   if they were real and behind the scenes the operating system will push
   or inject the packets into the regular networking stack as required
   making everything appear as if a real device is being used.

.. note::

   veth (virtual ethernet device): they can act as tunnels
   between network namespaces to create a bridge to a physical network
   device in another namespace, but can also be used as standalone network
   devices. The veth devices are always created in interconnected pairs.




**ExtUpperEthernetInterfaceInHost1**

The first example uses the ``NoExtHostsEthernetShowcase`` network:

.. figure:: NoExtHostsEthernetShowcase.png
   :scale: 100%
   :align: center

This example demonstrates how a real ping application can send echo
request messages to a simulated host, which then replies. This communication
between the real and the simulated word can be achieved by using the
:ned:`ExtUpperEthernetInterface` in ``host1``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.host1.eth[0].typename = "ExtUpperEthernetInterface"
   :end-at: *.host1.eth[0].copyConfiguration = "copyFromExt"

The :ned:`ExtUpperEthernetInterface` looks like the following when the simulation
is run in the GUI:

.. figure:: ExtUpperEthernetInterface.png
   :scale: 100%
   :align: center

The ``Upper`` part in the name of the module means that the upper part of the
functionality of the interface is external.
The :ned:`ExtEthernetTapDeviceFileIo` (``tap``) module provides connection to a real TAP
interface of the host computer which is running the simulation.

In order for the emulation to work, some configuration of the real operating environment
is needed as well. The ``TestExtUpperEthernetInterfaceInHost1.sh`` file contains the commands
that we need to execute in the terminal under Linux.

First of all, the above mentioned virtual TAP interface needs to be created with a specific IP
address:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # create TAP interface
   :end-at: sudo ip link set dev tap0 up

.. note::

   Linux and most other operating systems have the ability to
   create virtual interfaces which are usually called TUN/TAP devices.
   Typically a network device in a system, for example eth0, has a physical
   device associated with it which is used to put packets on the wire. In
   contrast a TUN or a TAP device is entirely virtual and managed by the
   kernel. User space applications can interact with TUN and TAP devices as
   if they were real and behind the scenes the operating system will push
   or inject the packets into the regular networking stack as required
   making everything appear as if a real device is being used.

As soon as the TAP interface (``tap0``) is brought up, a new entry appears
in the routing table of the OS:

.. figure:: new_entry.png
   :scale: 100%
   :align: center

In the simulation, the IP address of ``host2``'s Ethernet interface is set to
192.168.2.2/24:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'
   :end-at: *.configurator.config = xml("<config><interface hosts='host2' names='eth0' address='192.168.2.2' netmask='255.255.255.0'

The simulation is run from the terminal and the textual results are saved in the ``inte.out`` file:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # run simulation
   :end-at: inet -u Cmdenv -c ExtUpperEthernetInterfaceInHost1 --sim-time-limit=2s &> inet.out &

While the simulation is running, we can ping into it using the computers ping application.
This can be done from the terminal by pinging the IP address 192.168.2.2, the IP address
of the simulated ``host2`` module. The output is saved as ``ping.out`` file for later examination:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # ping into simulation
   :end-at: ping -c 2 -W 2 192.168.2.2 > ping.out

This way the real ping application of the OS generates the ECHO REQUEST messages.
Because of the above mentioned entry in the routing table, the messages are
routed through the virtual TAP interface. ``host1``'s Ethernet interface reads
``tap0`` - this point is the bridge between the real operating environment
and the simulation. The ECHO REQUEST messages are then forwarded towards ``host2``
inside the simulation. ``host2`` replies to the requests by sending ECHO REPLY
messages to ``host1``. The :ned:`ExtUpperEthernetInterface` of ``host1`` writes
the messages into the ``tap0`` virtual TAP interface. The real ping application
of the OS receives the ECHO REPLY messages and prints the results into the ``ping.out``
file.

We can check whether or not the pinging was successful by checking the content
of the ``ping.out`` file with the following command:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # check output
   :end-at: if grep -q "from 192.168.2.2" "ping.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; firm *.out

After the simulation, the TAP interface can be destroyed with
the following commands:

.. literalinclude:: ../TestExtUpperEthernetInterfaceInHost1.sh
   :language: xml
   :start-at: # destroy TAP interface
   :end-at: sudo ip tuntap del mode tap dev tap0
