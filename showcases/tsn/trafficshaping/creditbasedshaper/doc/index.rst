Credit-Based Shaper
===================

Goals
-----

In this example we demonstrate how to use a credit-based shaping.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/creditbasedshaper <https://github.com/inet-framework/inet-showcases/tree/master/tsn/trafficshaping/creditbasedshaper>`__

The Model
---------

In this configuration there are two independent data streams between two network
nodes that both pass through a switch where credit-based traffic shaping takes place.

Here is the network:

.. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/receivedpackets.png
   :align: center

.. figure:: media/transmission2.svg
   :align: center
   :width: 100%

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. image:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`CreditBasedShaperShowcase.ned <../CreditBasedShaperShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

