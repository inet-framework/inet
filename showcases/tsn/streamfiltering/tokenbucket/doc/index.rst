Token Bucket based Policing
===========================

Goals
-----

In this example, we demonstrate per-stream policing using chained token buckets
which allows specifying committed/excess information rates and burst sizes.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

The Model
---------

There are three network nodes in the network. The client and the server are
:ned:`TsnDevice` modules, and the switch is a :ned:`TsnSwitch` module. The
links between them use 100 Mbps :ned:`EthernetLink` channels.

.. figure:: media/Network.png
   :align: center

There are four applications in the network creating two independent data streams
between the client and the server. The average data rates are 40 Mbps and 20 Mbps
but both vary over time using a sinusoid packet interval.

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

The per-stream ingress filtering dispatches the different traffic classes to
separate metering and filter paths.

.. literalinclude:: ../omnetpp.ini
   :start-at: ingress per-stream filtering
   :end-before: SingleRateTwoColorMeter
   :language: ini

We use a single rate two-color meter for both streams. This meter contains a
single token bucket and has two parameters: committed information rate and
committed burst size. Packets are labeled green or red by the meter, and red
packets are dropped by the filter.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SingleRateTwoColorMeter

Results
-------

The first diagram shows the data rate of the application-level outgoing traffic
in the client. The data rate varies over time for both traffic classes using a
sinusoid packet interval.

.. figure:: media/ClientApplicationTraffic.png
   :align: center

The next diagram shows the operation of the per-stream filter for the best-effort
traffic class. The outgoing data rate equals the sum of the incoming data rate
and the dropped data rate.

.. figure:: media/BestEffortTrafficClass.png
   :align: center

The next diagram shows the operation of the per-stream filter for the video traffic
class. The outgoing data rate equals the sum of the incoming data rate and
the dropped data rate.

.. figure:: media/VideoTrafficClass.png
   :align: center

The next diagram shows the number of tokens in the token bucket for both streams.
The filled areas mean that the number of tokens changes quickly as packets pass
through. The data rate is at its maximum when the line is near the minimum.

.. figure:: media/TokenBuckets.png
   :align: center

The last diagram shows the data rate of the application-level incoming traffic
in the server. The data rate is somewhat lower than the data rate of the
outgoing traffic of the corresponding per-stream filter. The reason is that they
are measured at different protocol layers.

.. figure:: media/ServerApplicationTraffic.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/streamfiltering/tokenbucket`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/streamfiltering/tokenbucket && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.

