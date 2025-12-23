Step 5. Using 3D models for network nodes
=========================================

Goals
-----

Although the environment is similar to the real world, customizing
network nodes may further ease the understanding of the simulation. In
this step, we replace default icon of the nodes with an external 3D osg
model.

The model
---------

So far, we only deal with the environment of the simulation. In this
step, we add network nodes to the simulation model. We add two
:ned:`WirelessHost`, ``pedestrian0`` and ``pedestrian1`` and one
:ned:`AccessPoint`, ``accessPoint0``. The pedestrians will communicate with
each other via ``accessPoint0`` using a VoIP application. Communication
will be configured in later steps.

The following picture shows what the network looks like and what the
default appearance of the :ned:`WirelessHost` nodes looks like.

.. figure:: media/step5_model_default_appearance_2d.png

To use a 3D model instead of a 2D icon, set the ``osgModel`` parameter
of the network node. Examine the following code snippet.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization05]
   :end-before: #---

The above ``osgModel`` parameter value will be explained below.

- *boxman.osgb* is the name of the external 3D model,
- *(0.05).scale* means that size of the external 3D model is decreased to 5%.

By default, appearance of the nodes is the same in 2D and 3D
visualization. The default icon of :ned:`WirelessHost` is a laptop. In this
tutorial, the :ned:`WirelessHost`\ s represent pedestrians. In 2D
visualization, we have cellphone icon that represents pedestrians
better, than a laptop icon. Icon can be replaced in the display string
of the network node.

The following code snippet shows :ned:`VisualizationD` network which is
used for this step.

.. literalinclude:: ../VisualizationD.ned
   :language: ned
   :start-at: network VisualizationD

An :ned:`Ipv4NetworkConfigurator` module instance and an
:ned:`Ieee80211ScalarRadioMedium` module instance are also added to the
model. We will use them as ``configurator`` and ``radioMedium``. The
``configurator`` module assigns IP addresses and sets up static routing
for an IPv4 network, ``radioMedium`` is a radio medium model, uses
scalar transmission power in the analog representation.

Results
-------
.. 3D visualization

.. video:: media/step5_result_3d_enc.mp4

.. video:: media/step5_result_3d_v2_enc.mp4
   :width: 698

.. 2D visualization

.. figure:: media/step5_result_2d.png
   :width: 100%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VisualizationD.ned <../VisualizationD.ned>`

