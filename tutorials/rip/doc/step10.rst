Step 10. Configure an interface as NoRIP
========================================

Goals
-----

TODO

The model
---------

This step uses the same network as the previous one.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10

Results
-------

The RIP messages are only sent among the routers, but not the hosts.

.. video:: media/step10.mp4
   :width: 100%

..   <!--internal video recording, animation speed none, playback speed 2.138, zoom 0.77-->

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkC.ned <../RipNetworkC.ned>`
