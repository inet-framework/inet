Testing a Simulated VoIP Application Over a Real Network
========================================================

Goals
-----

In this showcase, we show how a simulated application can be used
over a real network.

Let's see a bit of background first.
INET contains several components to allow a variety of hardware-in-the-loop
(HIL) simulation scenarios. Specifically, it contains one or two additional
variants for several protocols and interfaces: an "ext lower" and/or an "ext
upper" variant. The naming indicates that the given "half" of the protocol or
interface connects to something external to the simulation, namely to an
interface or socket on the host OS. This showcase demonstrates how such modules
make it possible to build HIL scenarios like real apps/devices in a simulated network,
or simulated apps/devices in a real network.

In this showcase, we'll use :ned:`ExtLowerUdp`. This is a module that looks like
a normal UDP module to its higher layers (i.e. applications), but in fact it
sends and receives real UDP packets via the network stack of the host OS. In our
simulation, we'll let a pair of VoIP applications that normaly use the
:ned:`Udp` module communicate over :ned:`ExtLowerUdp`, and we'll make the actual
UDP packets travel across a real network so that we can see how that affects
voice quality. (Well, almost: to make it easier to run the simulation without
any special setup, we'll use tricks like routing between virtual interfaces
created in the Linux kernel instead of a real network. Luckily, Linux has
facilities to make those virtual network links somewhat more interesting by adding
delay, jitter, packet loss and packet corruption. The simulation setup can be
trivially modified so that it runs on two distinct hosts connected by a real
network.)

Note that this showcase requires the ``VoIPStream`` and ``ExtInterface``
features of the INET Framework to be turned on (they are off by default), and it
only runs on Linux.

| INET version: ``4.0``
| Source files location: `inet/showcases/emulation/voip <https://github.com/inet-framework/inet-showcases/tree/master/emulation/voip>`__

The Simulation Setup
--------------------

In this showcase, we generate generate realistic VoIP traffic using a simulated
VoIP application. We'll simulate the VoIP sender and receiver applications; however, the
traffic will be sent in a real network over UDP.

Also, the sender and receiver applications are run in separate simulations, so they
could be running on different machines.

We'll use the :ned:`ExtLowerUdp` module to connect the simulation with the real network.
The upper part of the module connects to the rest of the simulation; the lower part
connects to the host computer protocol stack via UDP sockets.

We'll send an audio file as VoIP traffic; the received re-encoded audio file and the
original can be compared to examine how the audio quality is affected by the packets
passing through the network.


There are only two submodules per node. There is a
:ned:`VoipStreamSender` in the sender node and a
:ned:`VoipStreamReceiver` in the receiver node, both called ``app``.
Both nodes contain an :ned:`ExtLowerUdp` module, called ``udp``.

Thus, both the sender and the receiver nodes look like this:

.. image:: media/VoipStreamSenderApplication.png
   :width: 25%
   :align: center

In this showcase, the real world and the simulation connects at two points. Firstly,
we generate VoIP traffic by re-encoding an mp3 file with the simulated VoIP protocol
(as opposed to sending dummy data packets). Secondly, we send UDP traffic (creating
application packets in the simulation and sending them over a real network via a UDP
socket). The following figure illustrates this scenario:

.. figure:: media/setup.png
   :align: center
   :width: 40%

Note that the division of the simulated and real parts of the network is arbitrary;
INET has support for dividing the network at other levels of the protocol stack;
for example, at the link layer.


The :ned:`VoipStreamSender` generates application packets. The simulated packets
enter the real UDP socket in the :ned:`ExtLowerUdp` module, where they are
encapsulated in real UDP packets and injected into the host OS protocol stack.

The packets travel through the host OS network stack to the real UDP socket at the receiver
side. The receiver :ned:`ExtLowerUdp` module injects the packets into the receiver
simulation, the :ned:`VoipStreamReceiver` module receives and decodes them, and creates
an audio file at the end of the simulation.

The simulations for the two cases are controlled by two shell scripts in the showcase's
folder. The scripts set up the network namespaces and ``veth`` interfaces, and run the
simulations. They also apply a small amount of delay, packet loss and bit corruption
to the interfaces in both cases to simulate the effects of the packets going through
a real network.

In the simulation, only a sender node and a receiver node are needed in order to
send the packets into the real network on one side and receive them on
the other side. The two simulated nodes are in separate parts of the whole
network, so they are defined in separate networks in the NED file, and run in
separate simulations.

the :ned:`ExtLowerUdp` module behaves just like the :ned:`Udp` module from the point of view
of the modules above them. In this showcase (aside from assigning the :ned:`ExtLowerUdp`
modules to network namespaces) only the VoIP modules need to be configured.
The simulations are defined in separate ini files, :download:`sender.ini <../sender.ini>`
and :download:`receiver.ini <../receiver.ini>`.


In this scenario, for simplicity, traffic
will only go through the host machine's protocol stack via either the loopback interface or a
pair of virtual Ethernet interfaces (but the traffic could be sent over any real network).

In this showcase, there are two cases depending on how the packets traverse the network:

The VoIP Configuration
~~~~~~~~~~~~~~~~~~~~~~

To generate the realistic VoIP traffic, we'll use the :ned:`VoipStreamSender` and
:ned:`VoipStreamReceiver` modules.

- The :ned:`VoipStreamSender` module transmits the contents of an audio file
  as VoIP traffic. The module resamples the audio, encodes it with a codec,
  chops it into packets and sends the packets via UDP. By default, it creates
  packets containing 20 ms of audio, and sends a packet every 20 ms.
  The codec, the sample rate, the time length of the audio packets, and other
  settings can be specified by parameters.

- The :ned:`VoipStreamReceiver` module can receive this data stream, decode it
  and save it as an audio file. The module numbers the incoming packets, and discards
  out-of-order ones. There is a playout delay (specified by parameter; by default 20 ms)
  simulating a de-jitter buffer.

The :ned:`VoipStreamSender` and :ned:`VoipStreamReceiver` modules are configured
just like they would be in a fully simulated scenario, no special configuration
is needed. Here is the sender-side configuration:

.. literalinclude:: ../sender.ini
   :language: ini
   :end-before: [Config

And the receiver side:

.. literalinclude:: ../receiver.ini
   :language: ini
   :end-before: [Config

Using the Loopback Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In our first configuration, packets will go via the loopback interface. Each
sent packet goes down the protocol stack, through the loopback interface,
and back up the protocol stack to the other simulation process.

The specific setup for the loopback interface case is illustrated by the following
figure:

.. figure:: media/setup2.png
   :align: center
   :width: 60%

Here is the :ned:`VoipStreamSender`'s configuration in :download:`sender.ini <../sender.ini>`.
It sets the ``destAddress`` parameter set to ``127.0.0.1``, the loopback address.

.. literalinclude:: ../sender.ini
   :language: ini
   :start-at: [Config LoopbackSender]
   :end-before: [Config VethSender]

Here is :ned:`VoipStreamReceiver`'s configuration in :download:`receiver.ini <../receiver.ini>`:

.. literalinclude:: ../receiver.ini
   :language: ini
   :start-at: [Config LoopbackReceiver]
   :end-before: [Config VethReceiver]

Here is the ``run_loopback`` script:

.. literalinclude:: ../run_loopback
   :language: bash

The script applies some delay (10ms with 1ms random variation), packet loss and bit
corruption to the traffic passing through the loopback interface.
It runs both simulations in Cmdenv. The simulations are run until the configured
simulation time limit, which is enough for the transfer of the whole audio file.
When the simulations are finished, the delay, packets loss and corruption are removed
from the loopback interface.

Using Virtual Ethernet Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Virtual Ethernet interfaces:** The sender and receiver :ned:`ExtLowerUdp` modules
belong to different network namespaces. Each namespace has a virtual Ethernet interface
(``veth``), which are connected to each other; the packets go through the ``veth``
interfaces.

Basically, a network namespace provides a complete virtualized network stack independent
from the main network stack of the host OS.

The specific setup for the virtual Ethernet interface (``veth``) case is illustrated below:

.. figure:: media/setup3.png
   :align: center
   :width: 60%

Here is the :ned:`VoipStreamSender`'s configuration in :download:`sender.ini <../sender.ini>`:

.. literalinclude:: ../sender.ini
   :language: ini
   :start-at: VethSender

In the ``VethSender`` configuration, the ``destAddress`` parameter is set to ``192.168.2.2``
(the address of the ``veth1`` interface). Also, the :ned:`ExtLowerUdp` module is set to use
the ``net1`` network namespace.

Here is :ned:`VoipStreamReceiver`'s configuration in :download:`receiver.ini <../receiver.ini>`:

.. literalinclude:: ../receiver.ini
   :language: ini
   :start-at: [Config VethReceiver]

The ``VethReceiver`` configuration sets the :ned:`ExtLowerUdp` module to use the ``net0``
network namespace.

Here is the ``run_veth`` script:

.. literalinclude:: ../run_veth
   :language: bash

The script creates two namespaces and a ``veth`` interface in each, and adds routes
(a route for each direction is required because there is an ARP exchange at the beginning).
Note that the veth interfaces are created in pairs, and are automatically connected
to each other. The scripts also adds the same delay, loss and corruption to the ``veth0``
interface as the loopback script. Then it runs the simulations. When they are finished,
it destroys the namespaces.

The received audio file is saved to the ``results`` folder.

Results
-------

As a reference, you can listen to the original audio file by clicking
the play button below:


.. raw:: html

   <p><audio controls> <source src="media/Beatify_Dabei_cut.mp3" type="audio/mpeg">Your browser does not support the audio tag.</audio></p>

Here is the received audio file:

.. raw:: html

   <p><audio controls> <source src="media/received.wav" type="audio/wav">Your browser does not support the audio tag.</audio></p>

The quality of the received sound is degraded compared to the original. The sender creates
and sends audio packets every 20ms, but when they go through the host OS
protocol stack, the packets are sent in bursts, as illustrated by the following image.
The image displays the time for packets sent by the sender (left) and packets received
by the receiver (right):

.. figure:: media/log.png
   :width: 100%
   :align: center

The traffic becomes bursty before the delay and jitter are applied.
The time difference between the packets in a burst is smaller than the applied jitter,
so that some of the packets are reordered. The receiver drops out-of-order packets,
thus the audio quality suffers.

Without the jitter, the packets would still arrive in bursts, but not reordered.
The quality would be nearly as good as the original file.

.. note:: To reduce burstiness and keep the packet order, a data rate can be specified
          in the ``netem`` command used in the scripts (along with the delay, loss and corruption):

          ``netem rate 1000kbps loss 1% corrupt 5% delay 10ms 1ms``

          This eliminates reordering in this scenario.

Sources: :download:`sender.ini <../sender.ini>`, :download:`receiver.ini <../receiver.ini>`, :download:`VoipStreamNode.ned <../VoipStreamNode.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
