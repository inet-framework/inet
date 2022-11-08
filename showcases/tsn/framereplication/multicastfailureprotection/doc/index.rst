Multicast Streams with Failure Protection
=========================================

Goals
-----

In this example we replicate the multicast stream example from the IEEE 802.1 CB standard.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/multicastfailureprotection <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/multicastfailureprotection>`__

The Model
---------

In this configuration we a use a network of TSN switches. A multicast stream is
sent through the network from one of the switches to all other switches.

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

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MulticastFailureProtectionShowcase.ned <../MulticastFailureProtectionShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/790>`__ page in the GitHub issue tracker for commenting on this showcase.

