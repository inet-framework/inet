Step 11. Configure an interface as NoOSPF
=========================================

Goals
-----

The goal of this step is to demonstrate the effect of excluding an interface from OSPF using
the ``NoOSPF`` interface mode.

Setting an interface to ``NoOSPF`` mode effectively removes it from OSPF processing. The
interface will not:

*   Send or receive OSPF packets
*   Form adjacencies
*   Be advertised in OSPF LSAs

This is equivalent to not including the interface in the OSPF configuration at all.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 3. Each router's eth0 interface (the one facing the switch) is configured with
``interfaceMode="NoOSPF"``.

.. figure:: media/Network.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step11
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_NoOspf.xml
   :language: xml

Results
~~~~~~~

With the interface configured as NoOSPF:

1.  The interface does not participate in OSPF at all.
2.  No Hello packets are sent on the interface.
3.  The network connected to this interface is not advertised in Router LSAs.
4.  Other routers are unaware of this network and cannot route to it via OSPF.

.. TODO merge these to 2 points

This mode is useful when you want OSPF running on a router but need to exclude specific
interfaces from OSPF processing, perhaps because they connect to non-OSPF networks or for
security reasons.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network.ned <../Network.ned>`,
:download:`ASConfig_NoOspf.xml <../ASConfig_NoOspf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
