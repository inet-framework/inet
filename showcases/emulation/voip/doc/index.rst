Using Simulated Applications in a Real Network
==============================================

Goals
-----

In this showcase, we show how a simulated application can be used as a real
application that communicates over the (real) network. Being able to do so opens
a lot of possibilities. For example, you can deploy an application that only
exists as a simulation model on real nodes, and test its behavior over a real
network. Or, by letting the model talk to real-world implementations of the same
application, you can test its interoperability and validate its behavior.

The goal will be achieved by running the INET application-layer module inside a
"wrapper" simulation, which (1) ensures that INET sockets map to real
sockets of the host OS (so that application traffic goes via the network stack
of the host OS and not stay inside the simulation), and (2) uses a real-time event
scheduler so that timings correspond to real time.

We'll use a VoIP application as an example. There'll be a sender and a receiver
application, running in two separate simulations, and we'll send realistic VoIP
traffic (contents of an audio file) between them. The received audio will be
saved to a file, which can be compared to the original to examine how the audio
quality is affected by the packets passing through the network.

Note that this showcase requires the ``VoIPStream`` and ``ExtInterface``
features of the INET Framework to be turned on (they are off by default), and it
only runs on Linux.

| INET version: ``4.0``
| Source files location: `inet/showcases/emulation/voip <https://github.com/inet-framework/inet-showcases/tree/master/emulation/voip>`__


Introduction
------------

Let's see a bit of background first. INET contains several components to allow a
variety of hardware-in-the-loop (HIL) simulation scenarios. Specifically, it
contains one or two additional variants for several protocols and interfaces: an
"ext lower" and/or an "ext upper" variant. The naming indicates that the given
"half" of the protocol or interface connects to something external to the
simulation, namely to an interface or socket on the host OS. This showcase
demonstrates how such modules make it possible to build HIL scenarios like real
apps/devices in a simulated network, or simulated apps/devices in a real
network.

In this showcase, we'll use :ned:`ExtLowerUdp`. This is a module that looks like
a normal UDP module to its higher layers (i.e. applications), but in fact it
sends and receives real UDP packets via the network stack of the host OS. In our
simulation, we'll let a pair of VoIP applications that normally use the
:ned:`Udp` module communicate over :ned:`ExtLowerUdp`, and we'll make the actual
UDP packets travel across a real network so that we can see how that affects
voice quality. The simulations will be run under a real-time event scheduler
(``inet::RealTimeScheduler``), so that timings correspond to real time.

Note that the division of the simulated and real parts of the network is
arbitrary; INET has support for dividing the network at other levels of the
protocol stack; for example, at the link layer.

The Simulation Setup
--------------------

The following figure illustrates the simulation setup:

.. figure:: media/setup.png
   :align: center
   :width: 40%

We'll use the :ned:`VoipStreamSender` and :ned:`VoipStreamReceiver` modules to
generate the realistic VoIP traffic, and :ned:`ExtLowerUdp` modules to connect
the applications to the real network. These modules are the only ones we need,
so the following "network" will suffice:

.. literalinclude:: ../AppContainer.ned
   :language: ned
   :start-at: network AppContainer

We'll configure the ``app`` submodule to have type :ned:`VoipStreamSender` and
:ned:`VoipStreamReceiver` on the sender and the receiver side, respectively.
Both networks look like the following when run under Qtenv:

.. image:: media/VoipStreamSenderApplication.png
   :width: 25%
   :align: center

.. note:: The ``AppContainer`` module used in this showcase is quite generic. It can
   turn any (UDP-only) simulated application into a real-life application that
   communicates over the network stack of the host OS.

Operation
~~~~~~~~~

When we run the simulations, :ned:`VoipStreamSender` in the sender-side
simulation process will generate VoIP packets and send them to its
:ned:`ExtLowerUdp` module. :ned:`ExtLowerUdp` maintains a real UDP socket in the
host OS, so the packets it receives from the application will be sent in real
UDP packets over the protocol stack of the host OS.

The UDP packets will then travel through the real network, and find their way to
the node which runs the receiver-side simulation process. The packets will be
received by the real UDP socket that :ned:`ExtLowerUdp` maintains there, converted to
simulation packets, then sent up to the :ned:`VoipStreamReceiver` module. The
:ned:`VoipStreamReceiver` module receives and decodes them, and creates an audio
file at the end of the simulation.

Configuration
~~~~~~~~~~~~~

In the simulation, only a sender node and a receiver node are needed in order to
send the packets into the real network on one side and receive them on
the other side. The two simulated nodes are in separate parts of the whole
network, so they are defined in separate networks in the NED file, and run in
separate simulations.

The :ned:`ExtLowerUdp` module behaves just like the :ned:`Udp` module from the point of view
of the modules above them. In this showcase (aside from assigning the :ned:`ExtLowerUdp`
modules to network namespaces) only the VoIP modules need to be configured.
The simulations are defined in separate configurations of :download:`omnetpp.ini <../omnetpp.ini>`.

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
is needed. Here is the relevant configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :end-before: VoipSenderLoopback

Running
-------

Several setups are provided here. In addition to running the setup over a real
network, two other setup are also provided that only use the local computer, so
they are supposedly easier to try out.

Over a Real Network
~~~~~~~~~~~~~~~~~~~

To try this showcase over a real network, you need to install the simulation on
two different machines. When it is done:

First, run the following command on the receiver side:

.. code-block:: bash

   inet -u Cmdenv -c VoipReceiver

Once the receiver is running, enter the following command on the sender side
(replace ``10.0.0.8`` with the IP address of the host the receiver side is
running on):

.. code-block:: bash

   inet -u Cmdenv -c VoipSender '--*.app.destAddress="10.0.0.8"'

That's all. When the receiver-side simulation exits, you'll find the received
audio file in ``results/received.wav``. You may need to raise the CPU time limit
in the receiver-side simulation so that it does not exit prematurely.

Using the Loopback Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, both simulations run on the same host, and packets will
go via the loopback interface. Each sent packet goes down the protocol stack,
through the loopback interface, and back up the protocol stack into the other
simulation process. The Linux kernel even allows us to apply a small amount of
delay, jitter, packet loss and data corruption to the packets traveling through
the interface, to simulate the effects of the packets going through a real
network.

The setup for the loopback interface case is illustrated by the following
figure:

.. figure:: media/setup2.png
   :align: center
   :width: 40%

The simulation can be run with a shell script, listed below, that performs the
appropriate configuration and starts the simulations. The script applies some
delay (10ms with 1ms random variation), packet loss and data corruption to the
traffic passing through the loopback interface. It runs both simulations in
Cmdenv. The simulations are run until the configured simulation time limit,
which is enough for the transfer of the whole audio file. When the simulations
are finished, the delay, packets loss and corruption are removed from the
loopback interface.

The ``run_loopback`` script:

.. literalinclude:: ../run_loopback
   :language: bash

The only extra configuration needed in the simulations is to set the
``destAddress`` parameter of the sender to ``127.0.0.1``, the loopback address.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: VoipSenderLoopback
   :end-before: VoipSenderVirtualEth

Using Virtual Ethernet Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This configuration uses network namespaces and virtual Ethernet interfaces,
which are features provided by the Linux kernel. A network namespace provides a
complete virtualized network stack independent from the main network stack of
the host OS.

We'll create two namespaces, ``net0`` and ``net1``. Then we'll create a virtual
Ethernet interface in both, and name them ``veth0`` and ``veth1``. The two
interfaces will be connected to each other. This setup is illustrated below:

.. figure:: media/setup3.png
   :align: center
   :width: 40%

The run script in this case (see below) creates the two namespaces and a *veth*
interface in each, and adds routes (a route for each direction is required
because there is an ARP exchange at the beginning). Note that the *veth*
interfaces are created in pairs, and are automatically connected to each other.
The scripts also adds the same delay, loss and corruption to the ``veth0``
interface as the loopback script. Then it runs the simulations. When they are
finished, it destroys the namespaces.

The ``run_veth`` script:

.. literalinclude:: ../run_veth
   :language: bash

In the simulation configurations, we need to set the network namespace of the
:ned:`ExtLowerUdp` modules to ``net0`` and ``net1``, and the sender's
``destAddress`` parameter to ``192.168.2.2``.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: VoipSenderVirtualEth

The received audio file is saved to the ``results`` folder.

Results
-------

Let's look at results obtained from running the loopback-based configuration! As
a reference, you can listen to the original audio file by clicking the play
button below:

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
The quality would be nearly as good as the original file. (To reduce burstiness
and keep the packet order, a data rate can be specified in the ``netem`` command,
e.g. ``rate 1000kbps``. This eliminates reordering in this scenario.)

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AppContainer.ned <../AppContainer.ned>`,
:download:`run_loopback <../run_loopback>`, :download:`run_veth <../run_veth>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/47>`__ in
the GitHub issue tracker for commenting on this showcase.
