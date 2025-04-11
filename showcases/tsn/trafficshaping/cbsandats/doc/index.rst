Using Different Traffic Shapers for Different Traffic Classes
=============================================================

Goals
-----

In INET, it is possible to use different traffic shapers for different traffic classes in a
TSN switch. In this showcase, we demonstrate using a Credit-Based Shaper (CBS)
and an Asynchronous Traffic Shaper (ATS), together in the same switch.

.. note:: You might be interested in looking at another showcase, in which multiple traffic shapers are combined in the same traffic stream: :doc:`/showcases/tsn/trafficshaping/cbsandtas/doc/index`.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/cbsandats <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/cbsandats>`__

The Model
---------

In this demonstration, similarly to the :doc:`/showcases/tsn/trafficshaping/creditbasedshaper/doc/index` and :doc:`/showcases/tsn/trafficshaping/asynchronousshaper/doc/index` showcases, we employ a
:ned:`Ieee8021qTimeAwareShaper` module with two traffic classes. However, unlike the previously mentioned two showcases, here one
class is shaped using a CBS, and the other class is shaped
by an ATS. Time-aware shaping is not enabled.

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The links
between them use 100 Mbps :ned:`EthernetLink` channels. The client generates two
traffic streams, and transmits them to the switch. In the switch, these streams
undergo traffic shaping, and are transmitted to the
server. In the results section, we plot the traffic in the switch before and
after shapers, to see the effects of traffic shaping. 

.. figure:: media/Network.png
   :align: center

There are four applications in the network creating two independent data streams
between the client and the server. The data rate of both streams are ~48 Mbps at
the application level in the client.

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

The two streams have two different traffic classes: best effort and video. The
bridging layer identifies the outgoing packets by their UDP destination port.
The client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
field.

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: ingress per-stream filtering
   :language: ini

The asynchronous traffic shaper requires the transmission eligibility time for
each packet to be already calculated by the ingress per-stream filtering.

.. literalinclude:: ../omnetpp.ini
   :start-at: ingress per-stream filtering
   :end-before: egress traffic shaping
   :language: ini

The traffic shaping takes place in the outgoing network interface of the switch
where both streams pass through. The traffic shaper limits the data rate of the
best effort stream to 40 Mbps and the data rate of the video stream to 20 Mbps.
The excess traffic is stored in the MAC layer subqueues of the corresponding
traffic class.

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :language: ini

Results
-------

Let's examine the traffic shaping process within the switch. The chart below
illustrates the incoming and outgoing data rates within the base time-aware
shaper module for the two traffic streams:

.. figure:: media/shaper_both.png
   :align: center

The CBS shapes the best effort traffic by restricting the data rate to a fixed
value of 40 Mbps without any bursts. The ATS shapes the video
stream by not only limiting the data rate to the desired nominal value but also
allowing for bursts when the traffic exceeds the nominal rate.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/trafficshaping/cbsandats`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/trafficshaping/cbsandats && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/801>`__ page in the GitHub issue tracker for commenting on this showcase.
