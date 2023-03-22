Example: Output Queue Switching
===============================

.. This example network demonstrates how queueing components can be combined to
.. simulate a point-to-point link with a finite bit rate. (This is only interesting
.. as a demonstration of the power of the queueing library, network links are never
.. actually simulated like this in OMNeT++ or INET.)

.. The network features two hosts (``ExampleHost``) communicating. The hosts are
.. connected by a cable module (``ExampleCable``) which adds delay to the connection.
.. Each host contains a packet source and a packet sink application, connected to
.. the network level by an interface module (``ExampleInterface``).

.. This example demonstrates input queue switching,

In this example, we'll build a network containing three packet sources, a switch, and two packet sinks, from
queueing components. The queues in the switch are in the output "interfaces". Here is the network, with the conceptual network nodes highlighted:

.. figure:: media/OutputQueueSwitching.png
   :width: 100%
   :align: center

The sources generate packets, which randomly contain either 0 or 1 as data. The classifier in the switch sends
packets based on this data to either sink0 or sink1. Output queue switching isn't susceptible to head-of-line blocking, as opposed to input queue switching.

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: OutputQueueSwitching
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config OutputQueueSwitching
   :end-at: packetFilters
   :language: ini
