Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the filtering and policing modules can work
outside the context of a network node. Doing so may facilitate assembling and
validating specific complex filtering and policing behaviors which can be difficult
to replicate in a complete network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/underthehood <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/underthehood>`__

The Model
---------

In this configuration we directly connect a per-stream filtering module to multiple
packet sources.

Here is the network:

.. figure:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/TrafficClass1.png
   :align: center

.. figure:: media/TrafficClass2.png
   :align: center

.. figure:: media/TrafficClass3.png
   :align: center

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/796>`__ page in the GitHub issue tracker for commenting on this showcase.

