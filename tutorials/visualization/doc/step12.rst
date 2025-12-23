Step 12. Visualizing physical link activity
===========================================

Goals
-----

In INET simulations, it is often useful to be able to show traffic
between network nodes. Traffic can be visualized at various levels of
the network stack. In this step we visualize physical link activity in
the network. By visualizing physical link activity, we can examine which
nodes have received correctly the sent frame.

The model
---------

The simulation is configured as follows.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization12]
   :end-before: # turning off transmissions and receptions

Physical link activity is displayed by :ned:`PhysicalLinkVisualizer`. By
setting ``displayLinks`` to *true*, we enable displaying physical link
activity. By default, all packets and all nodes are considered for the
visualization. In this step, we want to display physical link activity
only for VoIP packets. The packets are considered for the visualizer can
be narrowed by the ``packetFilter`` parameter. We set it to *"\*VoIP\*"*
to display only VoIP packets. The ``fadeOutTime`` parameter is changed
so that the activity arrows fade out slowly. We set ``lineColor`` and
``labelColor`` to *purple* so that the activity arrows are easy to
recognize just by looking at the simulation.

Results
-------

.. todo::

   figure:: step12_phys_link_3d.gif

   figure:: step12_phys_link_2d.gif

   The VoIP application starts at 1s. Then the pedestrian0 sends the first VoIP message. Because only
   the accessPoint0 is in its communication range, only between them appears an arrow. But when the sender
   is the accessPoint0, and the destination is the pedestrian1, an array turns up towards
   the pedestrian0 too. This happens, because the pedestrian0 is in the accessPoint0's communication
   range too, so its wlan NIC also can receive the VoIP packet.The array always points
   to the receiver.


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationD.ned <../VisualizationD.ned>`
