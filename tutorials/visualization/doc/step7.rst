Step 7. Displaying communication & interference range
=====================================================

Goals
-----

For successful communication, wireless nodes must be within each other's
communication range. By default, communication range is not displayed,
but INET offers a visualizer to show it. Visualizing communication range
also can help to place network nodes to appropriate position on the
playground. In this step, we display the communication and interference
range of the nodes.

The model
---------

Here is the configuration of this step.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization07]
   :end-before: #---

The size of the communication and interference range is mainly depends
on the transmitter power of the device. We set the transmitter power of
each device to *1mW*, because the default transmitter power is too big
for this simulation. Communication and interference range are visualized
by :ned:`MediumVisualizer`. We enable the ``visualizer`` by setting
``displayCommunicationRanges`` and ``displayInterferenceRanges`` to
true.

.. TODO: Visualization of velocity and orientation is disabled in this step...

Results
-------

.. figure:: media/step7_result_2d.png
   :width: 100%

.. todo::

   ![](step3_result1.png)
   ![](step3_result2.png)
   If we run the simulation in the 3D Scene view mode, we can see the three nodes and circles around them.
   Each node is in the center of a circle, that circle is the node's communication range.
   We configured the visualization of interference ranges too.
   These are also on the map, but they're very big, so we have to zoom out or move to any direction to see these ranges.
   The communication and interference ranges seen in the Module view mode too.
   When we run the simulation, the pedestrians associate with the access point.
   In Module view mode there's a bubble message when its happens.


Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VisualizationD.ned <../VisualizationD.ned>`
