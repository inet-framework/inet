Step 4. RIP Timeout timer and garbage-collection timer
======================================================

Goals
-----

TODO: Demonstrating the timeout-timer and the garbage collection timer

The model
---------

TODO: about the timout-timer and garbage collection timer... mark routes
as expired after 180s inactivity by default, delete them after 120s (but
still advertise until deleted)

TODO: the network and the config

Results
-------

TODO: video



   .. video:: media/step4.mp4

Here are two images of the RIP table of ``router1``.

The link breaks at 50s, so ``router1`` doesn't receive any updates on
the route to the 10.0.0.24/29 network from ``router2``. As indicated on
the following image, the last update to the route was at 30s, thats when
the timeout timer was started.

|image0|

The timeout timer expires at 210s, the route is set to metric 16, and
the flush timer is started.

|image1|

The flush timer expires at 330s, and the route is removed from the RIP
table.

TODO: what happens

TODO: screenshots or routing tables / RIP packets

.. |image0| image:: media/step4_3.png
.. |image1| image:: media/step4_4.png

