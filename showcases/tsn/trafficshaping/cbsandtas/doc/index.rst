Combining Time-Aware and Credit-Based Shaping
=============================================

Goals
-----

INET allows multiple traffic shapers to be used in the same traffic stream. This showcase
demonstrates this option by showing a simple network where credit-based and time-aware shaping
are combined.

.. note:: You might be interested in looking at another showcase, in which multiple traffic shapers are used in different traffic streams: :doc:`/showcases/tsn/trafficshaping/cbsandats/doc/index`.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/cbsandtas <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/cbsandtas>`__

The Model
---------

Time-aware shapers (TAS) and credit-based shapers (CBS) can be combined by
inserting an :ned:`Ieee8021qTimeAwareShaper` module into an interface, and
setting the queue type to :ned:`Ieee8021qCreditBasedShaper`. The number of
credits in the CBS only changes when the corresponding gate of the TAS
is open.

The Network
+++++++++++

In this demonstration, similarly to the :doc:`/showcases/tsn/trafficshaping/creditbasedshaper/doc/index` showcase, we employ a
:ned:`Ieee8021qTimeAwareShaper` module with two traffic classes. The two traffic classes
are shaped with both CBS and TAS.

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The links between them
use 100 Mbps Ethernet links. The client generates two traffic streams,
and transmits them to the switch. In the switch, these streams undergo traffic
shaping, and are transmitted to the server. In the results section, we plot the
traffic in the switch before and after shapers, to see the effects of traffic
shaping.

.. figure:: media/Network.png
   :align: center

Traffic
+++++++

.. Similarly to the other traffic shaping showcases, we want to observe how the
.. shapers change the traffic. When generating packets, we make sure that traffic
.. is only changed significantly in the shapers (i.e. other parts of the network
.. have no significant shaping effects).

We create two sinusoidally changing traffic streams (called ``best effort`` and
``video``) in the ``client``, with an average data rate of 40 and 20 Mbps. The
two streams are terminated in two packet sinks in the ``server``:

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: outgoing streams
   :language: ini

Stream Identification and Encoding
++++++++++++++++++++++++++++++++++

The two streams have two different traffic classes: best effort and video. The
bridging layer in the client identifies the outgoing packets by their UDP destination port.
The client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
field.

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: egress traffic shaping
   :language: ini

Traffic Shaping
+++++++++++++++

The traffic shaping takes place in the outgoing network interface of the switch
where both streams pass through. We configure the CBS to limit the data rate of
the best effort stream to ~40 Mbps, and the video stream to ~20 Mbps.
In the time-aware shaper, we configure the gates to be open for 4ms for best effort, and 2ms for video.

Here is the egress traffic shaping configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: egress traffic shaping
   :language: ini

Note that the actual committed information rate for CBS is 1/3 and 2/3 of the idle slope values set
here, because the corresponding gates are open for 1/3 and 2/3 of the time.

Packets that are held up by the shapers are stored in the MAC layer subqueues of
the corresponding traffic class.

Results
-------

The following chart displays the incoming and outgoing data rate in the
credit-based shapers:

.. figure:: media/shaper_both.png
   :align: center
   :width: 100%

Data rate measurement produces a data point after every 100 packet
transmissions, i.e. ~10 ms of continuous transmission. This is the same as the
cycle time of the time-aware shaping (including the periods when the gate is
closed), so ~2.5 open-gate periods for best effort, ~5 for video. Thus, the
fluctuation depends on how many idle periods are counted in a measurement
interval (so the data rate seems to fluctuate between two distinct values).

The following sequence chart displays packet transmissions for both traffic
categories (blue for best effort, orange for video). We can observe the
time-aware shaping gate schedules:

.. figure:: media/seqchart2.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/trafficshaping/cbsandtas`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/trafficshaping/cbsandtas && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/900>`__ page in the GitHub issue tracker for commenting on this showcase.

