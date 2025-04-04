Multiple Wireless Interfaces
============================

Goals
-----

Wireless devices often have multiple network interfaces so they can
communicate on multiple wireless networks simultaneously. This can be useful for
devices such as mobile phones, which often have cellular, WiFi, and Bluetooth
interfaces, or dual-band wireless routers.

This showcase demonstrates how to simulate such devices and how they can be used
to communicate on multiple wireless networks simultaneously.

| Since INET version: ``4.0``
| Source files location: `inet/showcases/wireless/multiradio <https://github.com/inet-framework/inet/tree/master/showcases/wireless/multiradio>`__

The model
---------

In this showcase, we will simulate a dual-band wireless router. Our dual-band
wireless router will have two 802.11 interfaces, one operating on the 2.4 GHz
band, and the other on 5 GHz. Both interfaces operate in infrastructure mode
and implement two different wireless LANs. The router will provide L2
connectivity (bridging) between the two LANs.

We will use INET's :ned:`AccessPoint` type for the wireless router,
and configure it to have two 802.11 interfaces. This is as simple
as setting the :par:`numWlanInterfaces` parameter to 2. The same
would work to configure a :ned:`StandardHost`, its derivatives
like :ned:`WirelessHost` and :ned:`AdhocHost`, or a :ned:`Router`
to have multiple wireless interfaces.

To test the router, we'll use two hosts, one on each wireless LAN.
The network will look like the following:

.. figure:: media/network.png
   :width: 60%
   :align: center

The network contains the wireless router named ``accessPoint``, and
two :ned:`WirelessHost`'s named ``host1`` and ``host2``. The model
also contains the usual support components, a medium model, a
configurator, and a visualizer.

The important part of the configuration is shown below:

.. literalinclude:: ../omnetpp.ini
   :start-at: access point
   :end-before: application level
   :language: ini

The wireless networks advertised by the two interfaces of ``accessPoint``
are configured to have the names ``wlan2.4`` and ``wlan5``, and the second
interface is configured to operate on 5 GHz using the :par:`bandName` parameter.
Of the hosts, ``host1`` is configured to connect to the ``wlan2.4`` network
and  ``host2`` to ``wlan5``. ``host2``'s wireless interface is also
configured to use the 5 GHz band so that it finds the access point.

The rest of the configuration (omitted) configures traffic
(``host1`` pings ``host2``) and configures visualization.


Results
-------

The following video has been captured from the simulation. Note how
``host1`` is pinging ``host2`` through ``accessPoint``. The radio signals are visualized
as disks, and successful transmissions between nodes' data link layers are visualized by arrows.
The transmissions for the two different networks (both disks and arrows) are colored
differently (red for wlan2.4 and blue for wlan5).

.. video:: media/ping3.mp4
   :width: 80%

.. run until sendPing, zoom 1.3, no animation speed, playback speed 0.4

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MultiRadioShowcase.ned <../MultiRadioShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/wireless/multiradio`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/wireless/multiradio && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/34>`__ page in the GitHub issue tracker for commenting on this
showcase.
