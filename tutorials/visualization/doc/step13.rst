Step 13. Showing data link activity
===================================

Goals
-----

Even though we can see physical link activity, sometimes we want to see
information about communication at data link layer level. INET offers a
visualizer that can visualize network traffic at data link layer level.
In this step, we display data link activity.

The model
---------

The configuration in the ini file is the following.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Visualization13]
   :end-before: # turning off physical link activity

The configuration is similar to the configuration of physical link
activity visualization, because :ned:`PhysicalLinkVisualizer` and
:ned:`DataLinkVisualizer` have the same base (:ned:`LinkVisualizerBase`) and
their most parameters are the same. :ned:`DataLinkVisualizer` is enabled by
setting ``displayLinks`` to *true*. The ``lineColor`` and ``labelColor``
parameters are set to *orange* so that the activity arrows are easy to
recognize just by looking at the simulation. We adjust the
``fadeOutMode`` and the ``fadeOutTime`` parameters so that the activity
arrows do not fade out completely before the next ping messages are
sent.

Results
-------

.. todo::

   - visualizing data link activity video
   - Visuailzing data link activity and physical link activity video

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`VisualizationD.ned <../VisualizationD.ned>`
