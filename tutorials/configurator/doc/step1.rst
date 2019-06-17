Step 1. Fully automatic IP address assignment
=============================================

Goals
-----

In the first step, we want to automatically assign addresses in a wired
network. To make the task a little bit more complicated, the network
won't be a single LAN, but several LANs connected via routers. We want to
place nodes on each subnet into a different subnet, but otherwise, we
don't care how addresses are assigned. We also ignore the question of
filling routing tables for now -- it will be covered in later steps.

The network
-----------

The network we want to configure is :ned:`ConfiguratorA`, defined in
``ConfiguratorA.ned``. It looks like
this:

.. figure:: media/step1network.png
   :width: 100%

The network contains three routers, each connected to the other two.
There are three subnetworks with ``StandardHosts``, connected to the
routers by Ethernet switches. It also contains an instance of
:ned:`Ipv4NetworkConfigurator`.

The configuration
-----------------

In many scenarios, including this one, the :ned:`Ipv4NetworkConfigurator`
module can properly configure the network using just the default
settings. Thus, the configuration in omnetpp.ini for this step is
basically empty:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step1
   :end-before: ####

The configurator has several parameters that affect its operation, but
for now, all of them are left at their default settings. Let's briefly
review the ones pertaining to IP address assignment:

-  ``assignAddresses = default(true)`` controls whether the configurator
   should perform auto address assignment, i.e. assign IP addresses to
   interfaces. Details can be specified in the XML configuration.

-  ``assignDisjunctSubnetAddresses = default(true)`` controls whether
   the configurator should assign different address prefixes and
   netmasks to nodes on different links. (Nodes are considered to be on
   the same link if they can reach each other directly, or through L2
   devices only.)

An XML configuration can be supplied with the ``config`` parameter. When
the user doesn't specify an XML configuration (such as in this step),
the configurator uses the following default:

.. code::

   config = default(xml("<config><interface hosts='**' address='10.x.x.x' netmask='255.x.x.x'/></config>"))

It tells the configurator to assign IP addresses to all interfaces of
all hosts, from the address range 10.0.0.0 - 10.255.255.255 and netmask
range 255.0.0.0 - 255.255.255.255.

Setting up logging and visualization
------------------------------------

The configurator has several boolean parameters named ``dump...``, which
cause it to dump certain parts of the configuration information into the
module log. The one that corresponds to assigned IP addresses is called
``dumpAddresses``. These parameters are false by default, but we set
them to true in the ``General`` section of omnetpp.ini to facilitate
experimenting with the configurator.

Network interface information, such as interface names and IP addresses,
can be visualized using the :ned:`InterfaceTableCanvasVisualizer` module,
which is already included in the network as a submodule of
:ned:`IntegratedCanvasVisualizer`. We also turn on interface visualization
but tell the visualizer to ignore switches and access points, as they
don't have IP addresses.

Other settings in the ``General`` section, such as the WiFi bit rate,
are not relevant for the topic of the tutorial.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: General
   :end-before: ####

Results
-------

The IP addresses assigned to interfaces by the configurator are shown on
the image below. The switches and hosts connected to the individual
routers are considered to be on the same link. Note that the
configurator assigned addresses sequentially starting from 10.0.0.1
while making sure that different subnets got different address prefixes
and netmasks, as instructed by the ``assignDisjunctSubnetAddresses``
parameter.

.. figure:: media/step1addresses.png
   :width: 100%

Note that the configurator assigned a 29-bit netmask to the hosts and
the router interfaces connecting to the switches, and a 30-bit netmask
to the other router interfaces. Three hosts and the router's interface
towards a switch as a group has four interfaces, thus, a 30-bit netmask
with a 2-bit host identifier would have sufficed. However, the
configurator doesn't assign addresses where the host identifier is
all-zeros or all-ones (as they commonly refer to the subnet itself
and the subnet broadcast address.)

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`ConfiguratorA.ned <../ConfiguratorA.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/2>`__ in
the GitHub issue tracker for commenting on this tutorial.
