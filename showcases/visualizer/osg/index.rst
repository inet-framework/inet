3D Visualization
================

The INET Framework is able to visualize a wide range of events and conditions
in the network: packet drops, data link connectivity, wireless signal path loss,
transport connections, routing table routes, and many more.
3D Visualization is implemented as a collection of configurable INET modules that
can be added to simulations at will. Actual visualization is provided by the
OpenSceneGraph and osgEarth libraries that are automatically linked to INET
if the ``VisualizationOsg`` feature is enabled.

Note that the ``VisualizationOsg`` feature is disabled by default.
The ``VisualizationOsgShowcases`` feature also needs to be enabled to run the showcases.


Infrastructure:

.. toctree::
   :maxdepth: 1

   networknode/doc/index
   environment/doc/index
   earth/doc/index

