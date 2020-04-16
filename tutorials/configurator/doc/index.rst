IPv4 Network Configurator Tutorial
==================================

This tutorial shows how to configure IP addresses and routing tables
(that is, achieve static autoconfiguration) in wired and wireless
networks in the INET Framework using the :ned:`Ipv4NetworkConfigurator`
module.

In INET simulations, configurator modules are commonly used for
assigning IP addresses to network nodes and for setting up their routing
tables. There are various configurators modules in INET; this tutorial
covers the most generic and most featureful one,
:ned:`Ipv4NetworkConfigurator`. :ned:`Ipv4NetworkConfigurator` supports
automatic and manual network configuration, and their combinations. By
default, the configuration is fully automatic. The user can also specify
parts (or all) of the configuration manually, and the rest will be
configured automatically by the configurator. The configurator's various
features can be turned on and off with parameters. The details of the
configuration, such as IP addresses and routes, can be specified in an
XML file.



.. The tutorial itself is organized into several steps, each one demonstrating
   a different feature or use case for the network configurator.
   Essentially, the configurator module simulates a real life network administrator.
   The configurator assigns IP addresses to interfaces, and sets up static routing in IPv4 networks.
   It doesn't configure IP addresses and routes directly, but stores the configuration in its internal data structures.
   Network nodes contain an instance of :ned:`Ipv4NodeConfigurator`, which configures the corresponding node's interface table and routing table based on information contained in the global :ned:`Ipv4NetworkConfigurator` module.
   The purpose of this design is that when router reboots after a simulated failure or shutdown,
   this way it can pull its IPv4 configuration from the configurator module, and restore
   its IPv4 addresses and routing table.

IPv4 Network Configurator Tutorial:

.. toctree::
   :maxdepth: 1
   :glob:

   step?
   step1?
   conclusion

This is an advanced tutorial, and it assumes that you are familiar with
creating and running simulations in OMNeT++ and INET. If you aren't, you
can check out the TicToc Tutorial to get started with using OMNeT++.

If you need more information at any time, feel free to refer to the
OMNeT++ and INET documentation:

-  `OMNeT++ User
   Manual <https://omnetpp.org/doc/omnetpp/manual/usman.html>`__
-  `OMNeT++ API
   Reference <https://omnetpp.org/doc/omnetpp/api/index.html>`__
-  `INET Manual
   draft <https://omnetpp.org/doc/inet/api-current/inet-manual-draft.pdf>`__
-  `INET
   Reference <https://omnetpp.org/doc/inet/api-current/neddoc/index.html>`__
