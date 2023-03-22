Example: Input Queue Switching
==============================

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
queueing components. The queues in the switch are in the input "interfaces". Here is the network, with the conceptual network nodes highlighted:

.. figure:: media/InputQueueSwitching.png
   :width: 100%
   :align: center

The sources generate packets, which randomly contain either 0 or 1 as data. The classifier in the switch sends
packets based on this data to either sink0 or sink1. Input queue switching is susceptible to head-of-line blocking,
i.e. if the first packet in the queue cannot be forwarded for some reason (e.g. the target sink is busy receiving another
packet), it can delay subsequent packets in the queue that would otherwise be transmittable at the current time.

.. .. note:: Output queue switching isn't susceptible for head-of-line blocking, see the next example.

.. note:: The next example demonstrates output queue switching, which is not susceptible to head-of-line blocking.

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: InputQueueSwitching
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config InputQueueSwitching
   :end-at: packetFilters
   :language: ini
