Step 2. Showing Boston downtown
===============================

Goals
-----

In network simulations, it is often desirable to have a real world
context. Displaying a street map is one way to achieve this. Although
displaying a map does not directly affect the outcome of the simulation,
nevertheless it can still facilitate understanding. In this step, we
display a street map of downtown Boston.

The model
---------

The :ned:`SceneVisualizer` module is responsible for displaying the map.
(Note that, :ned:`SceneVisualizer` can be used via ``visualizer``.) The
``sceneVisualizerType`` parameter is set to :ned:`SceneOsgEarthVisualizer`.
It displays a map of some part of the earth as an integrated part of the
3D visualization of the simulation. The map is loaded from an external
file, specified in the ``mapFile`` parameter. The visualizer's name
contains *osg* refers to that map will be displayed only in 3D
visualization.

Adding a map to the simulation model requires adding coordinate system
module to the model, because the map geographical coordinates are
assigned to the coordinate system module. (In the next step we will
explain the reason in detail.) We add an
:ned:`OsgGeographicCoordinateSystem` module to the model and set the origin
of the coordinate system in the ini file.

We add the coordinate system to our model as follows. This snippet is
copied from :download:`VisualizationB.ned <../VisualizationB.ned>`
that is used in this step.

.. literalinclude:: ../VisualizationB.ned
   :language: ned
   :start-at: network VisualizationB

The following lines show how ``visualizer`` and ``coordinateSystem`` are
configured in the ini file.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization02]
   :end-before: #---

Results
-------

.. figure:: media/step2_result_boston3d.png
   :width: 100%

.. 3d, real world context, camera zoom, rotate...,

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VisualizationB.ned <../VisualizationB.ned>`
