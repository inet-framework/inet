Cut-Through Switching
=====================

Goals
-----

Cut-through switching is a method used in packet switching systems, such as Ethernet switches, to forward frames or 
packets through the network. It involves starting the forwarding process before the entire frame has been received, 
typically as soon as the destination address and outgoing interface are determined. This is in contrast to store-and-forward 
switching, which waits until the entire frame has been received before forwarding it. One advantage of cut-through switching 
is that it may reduce the switching delay of Ethernet frames, since the switch can begin forwarding the frame as soon as it 
has enough information to do so. However, cut-through switching also has some potential disadvantages, such as a higher error 
rate compared to store-and-forward switching, since it does not check the entire frame for errors before forwarding it. In this 
showcase, we will demonstrate cut-through switching and compare it to store-and-forward switching in terms of delay.

| Since INET version: ``4.3``
| Source files location: `inet/showcases/tsn/cutthroughswitching <https://github.com/inet-framework/inet/tree/master/showcases/tsn/cutthroughswitching>`__

The Model
---------

Cut-through switching reduces the switching delay but skips the FCS check in the switch. The FCS
is at the end of the Ethernet frame; the FCS check is performed in the destination host.
(This is because by the time the FCS check could happen, the frame is almost completely transmitted,
so it makes no sense).
The delay reduction is more substantial if the packet goes through multiple switches
(as one packet transmission duration can be saved at each switch).

Cut-through switching makes use of intranode packet streaming in INET's modular
Ethernet model. Packet streaming is required because the frame needs to be processed
as a stream (as opposed to as a whole packet) in order for the switch to be able to
start forwarding it before the whole packet is received.

.. note:: The default is store-and-forward behavior in hosts such as :ned:`StandardHost`.

The example simulation contains two :ned:`TsnDevice` nodes connected by two
:ned:`TsnSwitch` nodes (all connections are 1 Gbps):

.. figure:: media/Network.png
   :align: center

In the simulation, ``device1`` sends 1000-Byte UDP packets to ``device2``, with a mean arrival time of 200ms,
and 50ms jitter. There are two configurations in omnetpp.ini, ``StoreAndForward`` and ``CutthroughSwitching``,
which only differ in the use of cut-through switching.

Here are the configurations:

.. literalinclude:: ../omnetpp.ini
   :language: ini


The default :ned:`LayeredEthernetInterface` in :ned:`TsnDevice` and :ned:`TsnSwitch` has cut-through disabled by default. In order to use 
Cut-through is enabled by setting the :par:`hasCutthroughSwitching` parameter to ``true``.

Results
-------

The following video shows the store-and-forward behavior in Qtenv:

.. video:: media/storeandforward.mp4
   :width: 80%
   :align: center

The next video shows the cut-through behavior:

.. video:: media/cutthrough1.mp4
   :width: 80%
   :align: center

The following sequence chart excerpt shows a packet sent from ``device1`` to ``device2`` via the switches,
for store-and-forward and cut-through, respectively (the timeline is linear):

.. figure:: media/storeandforwardseq2.png
   :align: center
   :width: 100%

.. figure:: media/seqchart2.png
   :align: center
   :width: 100%

We compared the end-to-end delay of the UDP packets in the case of store-and-forward switching
vs cut-through switching:

.. figure:: media/delay.png
   :align: center
   :width: 90%

We can verify that result analytically. In the case of store-and-forward, the end-to-end duration
is ``3 * (transmission time + propagation time)``, around 30.246us. In the case of cut-through,
the duration is ``1 * transmission time + 3 propagation time + 2 * cut-through delay``, around 10.534us.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`CutthroughSwitchingShowcase.ned <../CutthroughSwitchingShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/cutthroughswitching`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/cutthroughswitching && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/685>`__ page in the GitHub issue tracker for commenting on this showcase.

.. 1054B; 8.432us; 25.296+propagation time

  (1000 + 8 + 20 + 18 + 8) * 8 / 1E+9 * 3 / 1E-6
  (1000 + 8 + 20 + 18 + 8) * 8 / 1E+9 / 1E-6 + 22 / 1E+9 / 1E-6 * 2
