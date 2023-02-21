Using Real Applications in a Simulated Network
==============================================

Goals
-----

This showcase demonstrates the use of real applications in a simulated network, allowing for testing
of application behavior without having to set up a physical network. This technique is also known as
software-in-the-loop (SIL).

.. TODO this paragraph is too generic, it barely mean anything
The simulation can be easily configured for various topologies and behaviors to test a variety of cases.
Using INET's emulation feature, the real world (host OS environment) is interfaced with the simulation.
INET offers various modules that make this interfacing possible, and more information about these modules
can be found in the :doc:`Emulation section </users-guide/ch-emulation>` of the INET Framework User's Guide.

In this showcase, a real video application is used to stream a video file to another real video application. Note that this showcase requires the INET Framework's Emulation feature to be turned on (default is off) and is only supported on Linux.

| INET version: ``4.2``
| Source files location: `inet/showcases/emulation/videostreaming <https://github.com/inet-framework/inet/tree/master/showcases/emulation/videostreaming>`__

The Model
---------

The simulation scenario is illustrated with the following diagram:

.. figure:: media/setup.png
   :width: 50%
   :align: center

In this scenario, a VLC instance in a sender host streams a video file to another VLC instance
in a receiver host over the network. The hosts from the link-layer up are real; parts of the
link-layer, as well as the physical layer and the network are simulated.

We'll use the :ned:`ExtUpperEthernetInterface` to connect the real and simulated parts of the scenario.
The lower part of this interface is present in the simulation, and uses TAP interfaces in the host OS
to send and receive packets to and from the upper layers of the host OS.

Note that the real and simulated parts can be separated at other levels of the protocol stack, using
other, suitable EXT interface modules, such as at the transport layer (:ned:`ExtLowerUdp`), and the
network layer (:ned:`ExtUpperIpv4`, :ned:`ExtLowerIpv4`).

In fact, the real parts of the sender and receiver hosts are running on the same machine, as both use
the protocol stack of the host OS (even though in this scenario logically they are different hosts):

.. figure:: media/actualsetup.png
   :width: 50%
   :align: center

We'll use a VLC instance in the sender host to stream a video file. The packets created by VLC go
down the host OS protocol stack and enter the simulation at the Ethernet interface. Then they
traverse the simulated network, enter the receiver host's Ethernet interface, and are injected
into the host OS protocol stack, and go up to another VLC instance which plays the video.

The network for the simulation is the following:

.. figure:: media/Network2.png
   :width: 90%
   :align: center

It contains two :ned:`StandardHost`'s. Each host is connected by an :ned:`EthernetSwitch` to a :ned:`Router`.

The sender VLC application will stream the video to the address of the router's ``eth0`` in the simulation.
The router will perform network address translation to rewrite the destination address to the address of
the receiver host's EXT/TAP interface.

This is required so that the video packets actually enter the simulated network; if they were sent to the
receiver host's EXT/TAP interface, they would go through the loopback interface because the host OS optimizes traffic.

Three shell scripts in the showcase's directory can be used to control the emulation scenario.
The ``setup.sh`` script creates the TAP interfaces, assigns IP addresses to them, and brings them up:

.. literalinclude:: ../setup.sh
   :language: bash

The ``teardown.sh`` script does the opposite; it destroys the TAP interfaces when they're no longer needed.

.. literalinclude:: ../teardown.sh
   :language: bash

The ``run.sh`` script starts the simulation, and both video applications:

.. literalinclude:: ../run.sh
   :language: bash

In the configuration in omnetpp.ini, the scheduler class is set to ``RealTimeScheduler`` so that
the simulation can run in the real time of the host OS:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :end-at: sim-time-limit

The hosts are configured to have an :ned:`ExtUpperEthernetInterface`, and to use the TAP devices
which were created by the setup script. The setup script assigned IP addresses to the TAP interfaces;
the EXT interfaces are configured to copy the addresses from the TAP interfaces:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ExtUpperEthernetInterface
   :end-at: host2.eth[0].copyConfiguration

The addresses in the network are important; the configurator is set to assign the correct addresses so
the simulation and the script can work together (the VLC sends the video packets to the router, so
its address needs to match as the destination address in the script):

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: configurator
   :end-at: /config

Also, the CRC and FCS need to be set to ``computed`` to be able to properly serialize/deserialize packets.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: crcMode
   :end-at: fcsMode

Running/Results
---------------

Before running the emulation scenario, run ``setenv`` in the `omnetpp` and `inet` directories,
and run the ``setup.sh`` script in the showcase's folder:

.. code-block:: bash

  $ cd ~/workspace/omnetpp
  $ . setenv
  $ cd ~/workspace/inet
  $ . setenv
  $ cd showcases/emulation/videostreaming
  $ ./setup.sh

To start the simulation and the VLC instances, run the ``run.sh`` script:

.. code-block:: bash

  $ ./run.sh

The script starts the simulation in Cmdenv; the streaming VLC client is also started in command line mode.
The received video stream is played by the other VLC instance. The received video is lower quality than
the original video file,
because it's downscaled, and the bitrate is reduced, so that the playback is smooth.

.. note:: Emulating the network is CPU-intensive. The downscaling and bitrate settings were chosen to lead to smooth playback on the PC we tested the showcase on. However, it might be able to work in higher quality on a faster machine; the user can experiment with different encoding settings for the VLC streaming instance by editing them in the run script.

Here are some of the packets captured in Wireshark:

.. figure:: media/wireshark.png
   :width: 100%
   :align: center

Note that there are packets sent from the ``tapa`` (192.168.2.20) interface to the router's ``eth0`` (192.168.2.99) interface,
and also packets sent from the router's ``eth1`` (192.168.3.99) interface to ``tapb`` (192.168.3.20).

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VideostreamingShowcaseNetwork.ned <../VideostreamingShowcaseNetwork.ned>`,
:download:`run.sh <../run.sh>`, :download:`setup.sh <../setup.sh>`, :download:`teardown.sh <../teardown.sh>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/52>`__ in
the GitHub issue tracker for commenting on this showcase.
