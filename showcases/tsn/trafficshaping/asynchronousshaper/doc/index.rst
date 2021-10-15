Asynhronous Shaper
==================

Goals
-----

In this example we demonstrate how to use a asynchronous shaper for shaping the
traffic differently of two independent data streams.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/asynchronousshaper <https://github.com/inet-framework/tree/master/showcases/tsn/trafficshaping/asynchronousshaper>`__

The Model
---------

In this configuration there are two independent data streams between two network
nodes that both pass through a switch where the asynchronous traffic shaping takes
place.

Here is the network:

.. image:: media/Network.png
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

.. figure:: media/receivedpackets.png
   :align: center

.. figure:: media/gatestate.svg
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AsynchronousShaperShowcase.ned <../AsynchronousShaperShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

