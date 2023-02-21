Displaying IP Addresses and Other Interface Information
=======================================================

Goals
-----

In complex network simulations, it is often useful to display per-interface information,
such as IP addresses, interface names, etc., either above node icons or on the
links. This helps in verifying that the network configuration matches
expectations, especially when automatic address assignment is used (e.g. using
the :ned:`Ipv4NetworkConfigurator`). Although this information can be accessed
through the object inspector panel in the GUI, it can be tedious and difficult
to get an overview.

This showcase presents two example simulations that demonstrate the
visualization of IP addresses and other interface information. The first
simulation shows the display of the information with the default settings of the
visualizer, while the second one focuses on the advanced features.

| INET version: ``4.0``
| Source files location: `inet/showcases/visualizer/interfacetable <https://github.com/inet-framework/inet/tree/master/showcases/visualizer/interfacetable>`__

About the visualizer
--------------------

The :ned:`InterfaceTableVisualizer` module (included in the network as part
of :ned:`IntegratedVisualizer`) displays data about network nodes'
interfaces. (Interfaces are contained in interface tables, hence the
name.) By default, the visualization is turned off. When it is enabled
using the :par:`displayInterfaceTables` parameter, the default is that
interface names, IP addresses, and netmask length are displayed, above
the nodes (for wireless interfaces) and on the links (for wired
interfaces). By clicking on an interface label, details are displayed in
the inspector panel.

The visualizer has several configuration parameters. The :par:`format`
parameter specifies what information is displayed about interfaces. It
takes a format string, which can contain the following directives:

-  ``%N``: interface name
-  ``%4``: IPv4 address
-  ``%6``: IPv6 address
-  ``%n``: network address. This is either the IPv4 or the IPv6 address
-  ``%l``: netmask length
-  ``%M``: MAC address
-  ``%\\``: conditional newline for wired interfaces. (Note that the backslash
   needs to be doubled, due to escaping.)
-  ``%s``: the ``str()`` functions for the interface entry class

The default format string is ``"%N %\\%n/%l"``, i.e. interface name, IP
address, and netmask length.

The set of visualized interfaces can be selected with the configurator's
:par:`nodeFilter` and :par:`interfaceFilter` parameters. By default, all
interfaces of all nodes are visualized, except for loopback addresses
(the default for the :par:`interfaceFilter` parameter is ``"not lo\*"``.)

It is possible to display the labels for wired interfaces above the node
icons, instead of on the links. This selection can be done by setting the
:par:`displayWiredInterfacesAtConnections` parameter to ``false``.

There are also several parameters for styling, such as color and font
selection.

Enabling the visualization
--------------------------

The first example demonstrates the default operation of the visualizer.
The simulation uses the following network:

.. figure:: media/simplenetwork.png
   :width: 60%
   :align: center

The network contains two connected :ned:`StandardHost`'s. IP addresses are
auto-assigned by an :ned:`Ipv4NetworkConfigurator` module.

We enable visualization by the following configuration line:

.. literalinclude:: ../omnetpp.ini
   :start-at: displayInterfaceTables
   :end-at: displayInterfaceTables
   :language: ini

The interface names and the assigned IP addresses are displayed at the
gates where the interfaces are connected. When the simulation is run,
the network looks like the following:

.. figure:: media/simple.png
   :width: 60%
   :align: center

More examples
-------------

In the following example, we'd like to show the usefulness of this
visualizer in a dynamic scenario, as well as demonstrate filtering. The
simulation can be run by choosing the ``AdvancedFeatures`` configuration
from the ini file. It uses the following network:

.. figure:: media/advancednetwork.png
   :width: 80%
   :align: center

It contains two :ned:`StandardHost`'s connected to an :ned:`EthernetSwitch`. The
switch is connected to a :ned:`Router`, which is connected to an
:ned:`AccessPoint`. There is a :ned:`WirelessHost` and an :ned:`AdhocHost` near
the access point. They will obtain their addresses from the router via
DHCP. We would like to see IP addresses appear above the hosts when they
get their addresses.

We would like to hide the display of loopback addresses and the
unspecified address, so we set the following filter for the visualizer:

.. literalinclude:: ../omnetpp.ini
   :start-at: unspec
   :end-at: unspec
   :language: ini

Initially, the addresses of the wired interfaces of ``host1``, ``host2``,
and the router are visualized. The wireless hosts have unspecified
addresses, thus no interface indicator is displayed. The network looks
like this:

.. figure:: media/advancedbeginning.png
   :width: 80%
   :align: center


When the wireless hosts have been associated with the access point and
received their addresses from the DHCP server, the new addresses will be
displayed. The network will look like this:

.. figure:: media/advanced.png
   :width: 80%
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`InterfaceTableVisualizationShowcase.ned <../InterfaceTableVisualizationShowcase.ned>`

Further information
-------------------

For more information, refer to the :ned:`InterfaceTableVisualizer` NED
documentation.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/3>`__ in
the GitHub issue tracker for commenting on this showcase.
