Token Bucket based Policing
===========================

Goals
-----

In this example we demonstrate per-stream policing using chained token buckets
which allows specifying committed/excess information rates and burst sizes.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet-showcases/tree/master/tsn/streamfiltering/tokenbucket>`__

The Model
---------

Here is the network:

.. .. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :align: center
   :language: ini

Single Rate Two Color Meter
---------------------------

In this configuration we use a single rate two color meter for each of the four
streams. This meter consists of a token bucket and has two parameters: committed
information rate, committed burst size. Packets are labeled green or red, and
red packets are dropped.

Dual Rate Three Color Meter
---------------------------

In this configuration we use a dual rate three color meter for each of the four
streams. This meter consists of two chained token buckets and has four parameters:
committed information rate, committed burst size, excess information rate, and
excess burst size. Tokens overflow from the committed bucket into the excess
bucket. Packets are labeled green, yellow or red. In this example only red packets
are dropped. Normally, yellow packets would also be treated differently.

Results
-------

The following video shows the behavior in Qtenv:

.. video:: media/behavior.mp4
   :align: center
   :width: 90%

Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`TokenBucketBasedPolicingShowcase.ned <../TokenBucketBasedPolicingShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

