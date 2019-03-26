:orphan:

IEEE 802.11 Infrastructure and Ad Hoc Mode
==========================================

Goals
-----

802.11 devices can commonly operate in two basic modes. In infrastructure mode,
nodes connect to wireless networks created by access points, which
provide connectivity to other networks. In ad hoc mode, nodes form
an ad hoc wireless network, without using additional network
infrastructure.

INET has support for simulating both operating modes. This showcase
demonstrates how to configure 802.11 networks in infrastructure and
ad hoc mode, and how to check if they are configured correctly. The
showcase contains two example simulations defined in :download:`omnetpp.ini <../omnetpp.ini>`.

| INET version: ``4.0``
| Source files location: `inet/showcases/wireless/infrastructure <https://github.com/inet-framework/inet-showcases/tree/master/wireless/infrastructure>`__

The Model
---------

If you're not yet familiar with management and agent modules in the 802.11 model,
read the :doc:`corresponding section </users-guide/ch-80211>` in the INET User's Guide.

The showcase contains two example simulations, where two nodes communicate either through
an access point, in infrastructure mode, or directly, in ad hoc mode.
The simulations use the following networks:

.. figure:: network.png
   :width: 80%
   :align: center

The networks contain two :ned:`WirelessHost`'s named ``host1`` and
``host2``. They also contain an :ned:`Ipv4NetworkConfigurator`, an
:ned:`Ieee80211ScalarRadioMedium` and an :ned:`IntegratedVisualizer` module.
The network for the infrastructure mode configuration also contains an
:ned:`AccessPoint`.

The configurations in :download:`omnetpp.ini <../omnetpp.ini>` are named ``Infrastructure`` and
``Adhoc``. In both simulations, ``host1`` is configured to send UDP packets to
``host2``. :ned:`WirelessHost` has :ned:`Ieee80211MgmtSta` by default, thus no
configuration of the management module is needed in the infrastructure
mode simulation. In the ad hoc mode simulation, the default management
module in hosts is replaced with :ned:`Ieee80211MgmtAdhoc`. The ad hoc management module
doesn't require an agent module, so the agent module type is set to empty string.
(The same effect could have been achieved by using the :ned:`AdhocHost` host type
instead of :ned:`WirelessHost`, as the former has the ad hoc management
module and no agent by default.) The configuration keys for the management module type
in :download:`omnetpp.ini <../omnetpp.ini>` is the following:

.. literalinclude:: ../omnetpp.ini
   :start-at: mgmt
   :end-at: agent
   :language: ini

Results
-------

Infrastructure Mode
~~~~~~~~~~~~~~~~~~~

When the infrastructure mode simulation is run, the hosts associate
with the access point, and ``host1`` starts sending UDP packets to
``host2``. The packets are relayed by the access point. The following
video depicts the UDP traffic:

.. video:: Infrastructure4.mp4
   :width: 698

   <!--internal video recording, animation speed none, zoom 1.3x-->

The ``mib`` module (management information base) is a submodule of
:ned:`Ieee80211Interface`, and displays information about the node's
status in the network, e.g. MAC address, association state, whether
or not it's using QoS, etc. It also displays information about the mode,
i.e. infrastructure or ad hoc, station or access point.

To verify that the correct management type is configured, see the ``mib`` module's
display string by going into the host's or access point's wireless interface module;
it should read ``infrastructure``. Here is the wireless interface module of ``host1``
and ``accessPoint``:

.. figure:: mib_infrastructure.png
   :width: 100%

.. todo::

   <!--
   TODO: this might not be needed because it should be mentioned earlier
   or the earlier image should be cropped to show only the topology
   there should be a screenshot showing the mib in both cases
   and even for the AP and after association for host1
   -->

Ad Hoc Mode
~~~~~~~~~~~

In the ad hoc mode simulation, ``host1`` is sending UDP packets to ``host2`` in the following video:

.. video:: Adhoc3.mp4
   :width: 698

   <!--internal video recording, animation speed none, zoom 1.3x-->

The wireless interface module of ``host1`` is displayed on the following image,
showing the ``mib``, indicating that the interface operates in ad hoc mode and
non-QoS mode, and its MAC address.

.. figure:: adhocmib.png
   :width: 60%
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`InfrastructureShowcase.ned <../InfrastructureShowcase.ned>`

Discussion
----------

Use `this <TODO>`__ page in the GitHub issue tracker for commenting on this showcase.
