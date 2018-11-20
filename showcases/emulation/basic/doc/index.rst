Connecting the Real and the Simulated World
===========================================

Goals
-----

Emulation is a fast and cheap method for running experiments in the
real world. With the help of INET's emulation feature, it is possible
divide the simulated network into different parts, leaving some of them
as part of the simulation while extracting others into the real operating
environment.

This showcase presents the emulation feature of the INET framework by
performing ping communication in different emulated models.

INET version: ``4.0``

Source files location: `inet/showcases/emulation/pingpong <https://github.com/inet-framework/inet-showcases/tree/master/emulation/pingpong>`__

The model
---------

This showcase presents many different scenarios in order to explain how
the real world and the simulated environment can be connected at
different parts of a network.

All of these examples contain two hosts, one of which - may it be real or simulated -
pings the other one.

Using :ned:`ExtEthernetUpperInterface`
--------------------------------------

The :ned:`ExtUpperEthernetInterface` looks like the following when the simulation
is run in the GUI:

.. figure:: ExtUpperEthernetInterface.png
   :scale: 100%
   :align: center

The ``Upper`` part in the name of the module means that the upper part of the
functionality of the interface is external.
The :ned:`ExtEthernetTapDeviceFileIo` (``tap``) module provides connection to a real TAP
interface of the host computer which is running the simulation.

In order for these emulation examples to work, some configuration of the real operating environment
is needed as well.
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
the terminal and the textual results are saved in the ``inte.out`` file:

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
