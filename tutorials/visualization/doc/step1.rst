Step 1. Enabling visualization
==============================

Goals
-----

The default visualization of OMNeT++ already displays message sends,
methods calls, etc. These are rather low level details compared to the
domain specific visualization of INET. The complex state and behavior of
communication protocols provide much more opportunity for
visualizations. In the first step, we create a model with INET
visualizations enabled.

The model
---------

About visualization
~~~~~~~~~~~~~~~~~~~

In INET, visualization is implemented as separate modules. There are
several type of visualizers, displaying various aspects of the
simulation. Visualization can be more specific by using several
visualizer parameters.

This simulation model contains one :ned:`IntegratedVisualizer` which will
be called ``visualizer`` during this tutorial. This type of visualizer
integrates all single visualizers into a single module. In addition,
each visualizer integrates a canvas and an osg visualizer. Canvas and
osg visualizers usually can be set simultaneously by using the same
parameters, however it is also possible to set them separately.

The configuration
~~~~~~~~~~~~~~~~~

The configuration for this step uses the :ned:`VisualizationA` network,
defined in
:download:`VisualizationA.ned <../VisualizationA.ned>`.

.. literalinclude:: ../VisualizationA.ned
   :language: ned
   :start-at: network VisualizationA

For the first step, configuration is empty in the ini file.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization01]
   :end-before: #---

Results
-------

By adding ``visualizer`` to the simulation model, the visualization of
network phenomenons is enabled.

As you can see in the following image, in 2D visualization you can see
only the ``visualizer`` module on the playground.

.. figure:: media/step1_result_2d.png
   :width: 100%

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VisualizationA.ned <../VisualizationA.ned>`
