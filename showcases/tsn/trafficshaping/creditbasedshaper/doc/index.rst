Credit-Based Shaping
====================

Goals
-----

In this example we demonstrate how to use the credit-based traffic shaper.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/creditbasedshaper <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/creditbasedshaper>`__

The Model
---------

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels.

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

The first diagram shows the data rate of the application level outgoing traffic
in the client. The data rate varies randomly over time for both traffic classes
but the averages are the same.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the data rate of the incoming traffic of the traffic
shapers. This data rate is measured inside the outgoing network interface of
the switch. This diagram is somewhat different from the previous one because
the traffic is already in the switch, and also because it is measured at a
different protocol level.

.. figure:: media/TrafficShaperIncomingTraffic.png
   :align: center

The next diagram shows the data rate of the already shaped outgoing traffic of
the traffic shapers. This data rate is still measured inside the outgoing network
interface of the switch but at a different location. As it is quite apparent,
the randomly varying data rate of the incoming traffic is already transformed
here into a quite stable data rate.

.. figure:: media/TrafficShaperOutgoingTraffic.png
   :align: center

The next diagram shows the queue lengths of the traffic classes in the outgoing
network interface of the switch. The queue lengths increase over time because
the data rate of the incoming traffic of the traffic shapers is greater than
the data rate of the outgoing traffic, and packets are not dropped.

.. figure:: media/TrafficShaperQueueLengths.png
   :align: center

TODO

.. figure:: media/TransmittingStateAndGateStates.png
   :align: center

TODO

.. figure:: media/TrafficShaperNumberOfCredits.png
   :align: center

The next diagram shows the relationships between the number of credits, the gate
state of the credit based transmission selection algorithm, and the transmitting
state of the outgoing network interface for the both traffic classes.

.. figure:: media/TrafficShaping.png
   :align: center

The last diagram shows the data rate of the application level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding traffic shaper. The reason is that they
are measured at different protocol layers.

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/trafficshaping/creditbasedshaper`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir
       -c 'cd inet-4.4/showcases/tsn/trafficshaping/creditbasedshaper && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/800>`__ page in the GitHub issue tracker for commenting on this showcase.

