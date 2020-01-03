Step 2. Manually overriding individual IP addresses
===================================================

Goals
-----

Automatic address assignment is great, but it is sometimes also useful
to be able to manually specify IP addresses for certain nodes or
interfaces. This step shows how to do that while retaining automatic
assignment for the rest of the network.

The model
---------

This step uses the :ned:`ConfiguratorA` network from the previous step. We
will assign the 10.0.0.50 address to ``host1``, and 10.0.0.100 to
``host3``. The configurator will automatically assign addresses to the
rest of the nodes.

Configuration
~~~~~~~~~~~~~

The configuration in omnetpp.ini for this step is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ####

The value for the ``config`` parameter can be supplied either inline
using the ``xml()`` function, or in an external XML file using the
``xmldoc()`` function (with the file name as the argument). In this
step, the XML configuration is supplied to the configurator as inline
XML.

In the above XML configuration, the first two rules state that
``host3``'s ``eth0`` interface should get the address 10.0.0.100, and
``host1``'s ``eth0`` interface should get the address 10.0.0.50. The
third rule is the exact copy of the default configuration and tells the
configurator to assign the rest of the addresses automatically.

Note that the XML configuration contains ``<config>`` as root element.
Under this root element, there can be multiple configuration elements,
such as the ``<interface>`` elements here.

The <interface> element
~~~~~~~~~~~~~~~~~~~~~~~

An ``<interface>`` element configures a network interface or a set of
interfaces, and it has two groups of attributes: selector attributes
(``hosts``, ``names``, ``towards``, ``among``, etc.) that define which
interface(s) are to be configured, and parameter attributes
(``address``, ``netmask``, ``metric``, etc.) that specify values to be
set on those interfaces. The attributes used in the configuration above
are:

-  The ``hosts`` selector attribute selects hosts. The selection pattern
   can be a full path (e.g. ``"*.host0"``) or a module name anywhere in
   the hierarchy (e.g. ``"host0"``). Only interfaces in the selected
   hosts will be affected by the ``<interface>`` element.

-  The ``names`` selector attribute selects interfaces by name. Only the
   interfaces that match the specified names will be selected (e.g.
   ``"eth0"``).

-  The ``address`` parameter attribute specifies the addresses to be
   assigned. Address templates can be used, where an ``x`` in place of
   an octet means that the value should be selected by the configurator
   automatically. The value ``""`` means that no address will be
   assigned. Unconfigured interfaces will still have allocated addresses
   in their subnets, so they can be easily configured later dynamically.

-  The ``netmask`` parameter attribute specifies the netmasks to be
   assigned. Address templates can be used here as well.

All attributes are optional. There are many other attributes available.
For the complete list, please refer to the configurator's NED
documentation.

The order of configuration elements is important, but the configurator
doesn't assign addresses in the order of XML ``<interface>`` elements.
It iterates interfaces in the network, and for each interface, the first
matching rule in the XML configuration will take effect. Thus,
statements that are positioned earlier in the configuration take
precedence over those that come later.

When an XML configuration is supplied, it must contain ``<interface>``
elements in order to assign addresses at all. To make sure the
configurator automatically assigns addresses to all interfaces, a rule
similar to the one in the default configuration has to be included
(unless the intention is to leave some interfaces unassigned.) The
default rule should be the *last one* among the interface rules so that
more specific rules can override it.

Note that after applying a manually specified address, auto address
assignment will continue from that address. This may or may not be what
you want.

Results
-------

The assigned addresses are shown in the following image.

.. figure:: media/step2address.png
   :width: 100%

As in the previous step, the configurator assigned disjunct subnet
addresses. Note that the configurator still assigned addresses
sequentially, i.e. after setting the 10.0.0.100 address to ``host3``, it
didn't go back to the beginning of the address pool when assigning the
remaining addresses.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`ConfiguratorA.ned <../ConfiguratorA.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/2>`__ in
the GitHub issue tracker for commenting on this tutorial.
