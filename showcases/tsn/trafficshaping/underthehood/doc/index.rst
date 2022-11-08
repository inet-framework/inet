Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the scheduling and traffic shaping modules can
work outside the context of a network node. Doing so may facilitate assembling
and validating specific complex scheduling and traffic shaping behaviors which
can be difficult to replicate in a complete network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/underthehood <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/underthehood>`__

The Model
---------

TODO incomplete

The network contains three independent packet sources which are connected to a
single asynchronous traffic shaper using a packet multiplexer.

.. figure:: media/Network.png
   :align: center

The three sources generate the same stochastic traffic. The traffic shaper is
driven by an active packet server module with a constant processing time.

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

TODO

.. figure:: media/QueueingTime.png
   :align: center

TODO

.. figure:: media/QueueLength.png
   :align: center


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/803>`__ page in the GitHub issue tracker for commenting on this showcase.

