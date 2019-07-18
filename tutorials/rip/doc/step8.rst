Step 8. Hold-down timer
=======================

Goals
-----

Demonstrate that routing loops can be prevented by using holddown (and
the holddown timer) in cases when other methods, such as split horizon /
splithorizon with poisoned reverse are not effective.

The model
---------

Results
-------

The 10.0.0.24/29 network becomes unreachable, the route in router1 is
set to metric 16. This is propagated to router3 (it also sets the route
to 10.0.0.24/29 to metric 16). The update from router2 contains the
route with metric 3, but its not updated in router1 because the route is
in holddown. (TODO: this is essentially the same as the paragraph below)

Thus routing loops are prevented.



   .. video:: media/step8.mp4

On the following image, ``router2`` sends a RIP Response message to
``router3``. The message contains the route to the (currently
unreachable) 10.0.0.24/29 network, with a metric of 3. Also, ``router3``
has a route to the network with a metric of 16. The route will not be
updated in ``router3``, as the route is in holddown. The details of the
RIP Response message and ``router3``'s RIP table are highlighted (click
on the image to enlarge):

|image0|

.. |image0| image:: media/step8_1.png

