Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the scheduling and traffic shaping modules can
work outside the context of a network node. Doing so may facilitate assembling
and validating specific complex scheduling and traffic shaping behaviors which
can be difficult to replicate in a complete network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/underthehood <https://github.com/inet-framework/inet-showcases/tree/master/tsn/trafficshaping/underthehood>`__

The Model
---------

In this configuration we directly connect a traffic shaping module to multiple
packet sources.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

Here are the simulation results:

.. figure:: media/queueingtime.png
   :align: center

.. figure:: media/queuelength.png
   :align: center


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

