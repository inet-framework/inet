Step 16. Displaying 802.11 channel access method
================================================

Goals
-----

Several mechanisms are used as a part of 802.11 channel access to
minimize the likelihood of frame collisions. This function is necessary
because the wireless medium is half-duplex. For this reason, it is often
useful to display information about the access state of nodes. INET
provides a visualizer that display IEEE MAC 802.11 contention states
during the channel access method. This information is contained in
submodules. By visualizing this information, we get a clear picture
about the contention states of network nodes at a glance, without going
deep into submodules.

The model
---------

.. todo::

   Firstly we hide some visualizers, because they are distracting.
   The communication is the same as in the previous step, we have to configure only the visualizer.
   To display the channel access states, we use infoVisualizer. <br>
   Here is the configuration:
   @dontinclude omnetpp.ini
   @skipline [Config Visualization14]
   @until ####

   The module parameter specifies the submodules of network nodes, and the content
   determines what is displayed on network nodes. In addition we can adjust the
   background color, the font color, and the opacity. These are optional settings.


Results
-------

.. todo::

   ![](step16_channel_access_2d.gif)
   Here's what happens, when the simulation is running:
   [gif simulation is running]

   We see, the nodes wait until the channel is sensed to be idle. If the medium is clear,
   instead of immediately transmitting network nodes are waiting a predefined amount of time.
   This waiting period is called the interframe spacing (IFS).
   It depends on the priority of the packet.
   In addition to having a different IFS, a station will add a "random backoff"
   to its waiting period, to reduce the collision probability.
   After that the the network node starts transmitting the data, and it's owning the channel.


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationE.ned <../VisualizationE.ned>`
