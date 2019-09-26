Connecting the Real and the Simulated World
===========================================

Goals
-----

Ping is a basic Internet program that allows a user to verify that a
particular IP address exists and can accept requests. Ping is used
diagnostically to ensure that a host computer the user is trying to
reach is actually operating. INET features a way to simulate ping
between hosts.

This showcase presents the emulation feature of the INET framework by
performing ping communication in simulated and emulated models.

INET version: ``4.0``

Source files location: `inet/showcases/emulation/pingpong <https://github.com/inet-framework/inet-showcases/tree/master/emulation/pingpong>`__

About ping
----------

Ping is a computer network administration software utility used to test
the reachability of a host on an Internet Protocol (IP) network. It
measures the round-trip time for messages sent from the originating host
to a destination computer that are echoed back to the source.

Ping operates by sending Internet Control Message Protocol (ICMP) echo
request packets to the target host and waiting for an ICMP echo reply.

The model
---------

The network
~~~~~~~~~~~

This showcase presents three different scenarios in order to present how
an emulated network can be divided at different parts in contrast with a
fully simulated one.

**Fully Simulated Network**

The first example presented in this showcase is a fully simulated
network. The network consist of two hosts (``host1`` and ``host2``) and
a 100Mbps Ethernet connection between them. The following image shows
the layout of the first network:

.. figure:: pingpong_layout_1.png
   :width: 60%
   :align: center

**First Emulated Network**

The second example presents an emulated network, consisting of one
simulated node called ``host1``. The other parts of the network, from
the ``ExtLowerEthernetInterface`` of ``host1`` are not part of the
simulated environment. The simulated side of the network can be seen in
the following picture:

.. figure:: pingpong_layout_2.png
   :width: 60%
   :align: center

**Second Emulated Network**

In the third example, there is a simulated node, ``host1``, and a a
simulated connection between ``host1`` and ``host2``. The layout of the
third network is the same as it was with the fully simulated scenario,
although ``host2`` is not part of the simulation:

.. figure:: pingpong_layout_1.png
   :width: 60%
   :align: center

Configuration and behaviour
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please make sure to follow the steps on the `previous page <file:///home/marcell/inet/doc/src/_build/html/showcases/emulation/index.html>`__
to set the application permissions!


The showcase involves three different configurations:

-  ``Simulated``: A fully simulated model with two nodes and a 100Mbps Ethernet connection
   between them.
-  ``Emulated1``: One of the nodes is simulated, the other
   node and the connection is in the real operating environment.
-  ``Emulated2``: One of the nodes and the 100Mbps Ethernet connection is
   simulated, the other node is in the real operating environment.

In all the three examples the ``PingApp`` of the simulated ``host1``
sends ping packets with the destination address of ``192.168.2.2``.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: numApps
   :end-before: ####

**Simulated configuration**

There is no need for any other parameter to be set in the ``Simulated``
configuration.

**Emulated1 configuration**

The ``Emulated1`` configuration uses the ``ExtLowerEthernetInterface``
provided by INET. This is a socket based external ethernet interface.
Before running the emulation, there are some preparations that need be
done. The ``Emulation1`` configuration is run using virtual ethernet
devices, called ``vetha`` and ``vethb``. These two devices need to be
created and configured. This is achieved as the following:

.. literalinclude:: ../setup1.sh

.. note::

   veth (virtual ethernet device): they can act as tunnels
   between network namespaces to create a bridge to a physical network
   device in another namespace, but can also be used as standalone network
   devices. The veth devices are always created in interconnected pairs.

We can see that ``vethb`` gets the IP address ``192.168.2.2``, which is
the same as the destination Address of ``host1``'s ``PingApp``. In this
emulation ``host1`` pings ``vethb`` through ``vetha``.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Emulated1
   :end-before: ####

It is important that the TAP device used in the ``Emulated2``
configuration and the veth devices used in the ``Emulated1``
configuration do not exist at the same time during an emulation. The
reasun for that is that both ``vethb`` and ``tapa`` have the same IP
address, which can falsify the results. So it is highly recommended to
destroy the virtual ethernet link after running the ``Emulated1``
configuration:

.. literalinclude:: ../teardown1.sh

**Emulated2 configuration**

The ``Emulated2`` configuration uses the ``ExtUpperEthernetInterface``
provided by INET. This emulation is with real TAP interface connected to
the simulation. For this reason the ``tapa`` TAP interface need to be
created and configured before the ``Emulation2`` configuration is run:

.. literalinclude:: ../setup2.sh

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

In this configuration we do not need the configurator to set the IP
address of ``host2``, because ``host2`` uses the ``tapa`` real TAP
interface, to which the proper IP address was previously assigned.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Emulated2
   :end-before: ####

It is recommended to destroy the TAP interface if the emulation is
finished and it is not needed anymore:

.. literalinclude:: ../teardown2.sh

Results
-------

This time the simulations are run from the terminal in order to show the
above detailed preparations as well.

Simulated configuration
~~~~~~~~~~~~~~~~~~~~~~~

There are no preparations needed for the ``Simulated`` configuration to
run. As the video confirms, this is a fully simulated model, which runs
in a simulated environment. No real network interfaces of the computer
are influenced by the simulation, meaning that no extra traffic appears
on them:

.. video:: Simulated_EDIT.mp4
   :width: 100%
|

Emulated1 configuration
~~~~~~~~~~~~~~~~~~~~~~~

As mentioned above, this configuration uses the
``ExtLowerEthernetInterface`` offered by INET. This external interface
acts as a bridge between the simulated and the real world. From this
module on, the emulation enters the real operation environment of the
OS. The following video shows how the traffic rate of the virtual
ethernet devices changes, while the emulation is running. Right at the
beginning of the video, you can also take a look at the previous
configurations that needed to be done for the emulation to run without
errors:

.. video:: Emulated1_EDIT.mp4
   :width: 100%
|

The change in the traffic rate of ``vetha`` and ``vethb`` is conspicuous
when the emulation is started.

Emulated2 configuration
~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, the ``ExtUpperEthernetInterface`` is used. In a
sense this interface is similar to the ``ExtLowerEthernetInterface``
used in the ``Emulated1`` configuration, meaning that it acts as a
bridge between the simulated and the real world. If we run the
``Emulated2`` configuration and observe the traffic rate of the ``tapa``
TAP interface, we can conclude that ``host1`` is indeed pinging the real
TAP interface:

.. video:: Emulated2_EDIT.mp4
   :width: 100%
|

Conclusion
----------

We can clearly see that using the external interfaces, the emulation can
switch between the real and the simulated environment. This emulation
feature of INET makes it possible for the user to "cut" the network at
arbitrary points into many pieces and leave some of them in the
simulation while extracting the others into the real world.

Further Information
-------------------

The following link provides more information about ping in general:
- `ping (networking utility) <https://en.wikipedia.org/wiki/Ping_(networking_utility)>`__

The network traffic is observed using `bmon <https://github.com/tgraf/bmon>`__,
which is a monitoring and debugging tool to capture networking related statistics
and prepare them visually in a human friendly way.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
