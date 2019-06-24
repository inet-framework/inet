Step 17. Showing network path activity
======================================

Goals
-----

In the Open Systems Interconnection (OSI) communications model, the
network layer has many tasks, for example selects routes and quality of
service, and recognizes and forwards to the transport layer incoming
messages for local host domains. INET offers a visualizer for displaying
traffic at network layer level. In this step, we enable network path
activity visualization for video stream. The visualization helps
verifying whether a videoStream packet passed the network layer of the
client node.

The model
---------

.. todo::

   Firstly we have to edit the configurator. We make an xml file (in this case configurationD.xml),
   to set the static ip addresses. Static addresses are the routers' interfaces and
   the videoStreamServer's IP address.

   @dontinclude configurationD.xml
   @skip config
   @until /config

   The routers assign addresses to wireless nodes via DHCP.
   To that we have to turn on the hasDHCP parameter. Then we adjust the
   other settings of that service. We have to set which interface assign the addresses.
   In our simulation it's the "eth0" on both router. MaxNumClients parameter adjusts
   maximum number of clients (IPs) allowed to be leased (in our simulation we set to 10)
   and numReservedAddresses define number of addresses to skip
   at the start of the network's address range. To gateway we add that interface's IP address,
   that run the DHCP service. Finally we have to add the lease time.
   We can adjust the start time, but usually we want that, DHCP service run the
   beginning, so we leave it on 0s.

   If we want RIP routing protocol work, we have to set true the routers' hasRIP parameter,
   and set to false the configurator.optimizeRoutes parameter.

   The configuration:
   @dontinclude omnetpp.ini
   @skipline [Config Visualization15]
   @until ####

Results
-------

.. todo::

   ![](step17_networkroute_3d.gif)

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationF.ned <../VisualizationF.ned>`
