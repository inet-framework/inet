Step 14. Displaying statistics
==============================

Goals
-----

Statistics are important factors of simulation. They are collected in
simulation time and usually examined and evaluated after simulation has
ended. From the statistics it can be concluded whether the simulation
works properly. Sometimes we need a quick overview in simulation time
whether the simulation is working as expected. INET provides a
visualizer that can show statistical data in simulation time. In this
step, we display a statistic about VoIP packet's end-to-end delay.

The model
---------

:ned:`StatisticVisualizer` (included in the network as part of
:ned:`IntegratedVisualizer`) keeps track of the last values of statistics,
and displays them above the icon of the network node.

In this tutorial, we show statistic about end-to-end delay of VoIP
traffic between ``pedestrian0`` and ``pedestrian1``. The visualizer
subscribes for the *VoIPPacketDelay* signal selected with the
``signalName`` parameter and displays the *endToEndDelay* statistic
selected with the ``statisticName`` parameter. We set the
``sourceFilter`` parameter to *"\*.pedestrian1.udpApp[0]"*. Visualizer
displays the statistic of that module at the node that contains the
module.

The default unit of *endToEndDelay* statistic is the second, but packet
delay in VoIP communication is measured in milliseconds (*ms*) so we set
``unit`` to *ms*. We set ``textColor`` to *yellow* and
``backgroundColor`` to *grey*, because it looks better in the
playground.

Results
-------


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationD.ned <../VisualizationD.ned>`
