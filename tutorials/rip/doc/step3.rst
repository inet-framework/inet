Step 3. Link Breakage and Routing Table Updates
===============================================

Goals
-----

We demonstrate how RIP updates the routing tables after changes occur in a
network topology.

Network Configuration
---------------------

We keep the network configuration same as the previous step, except that we will
schedule a break with the help of :ned:`ScenarioManager`. RIP should update the routing
information in the routing tables of routers to eliminate the references to the
broken link. Here, for the reliable convergence of routing tables, we need to
enable SplitHorizon though (we will postpone discussion of this issue until
Step 6 of the tutorial).

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

The last key in the above figure is for scheduling a break.

Experiment
----------

In the video below observe that at t=50 seconds, as directed by the scenario manager script,
the link connecting ``router2`` and ``switch2`` breaks (note that the video starts
approximately at t=30, after the routing tables stabilize).


.. video:: media/step3_linkbreak.mp4
   :width: 100%

..   <!--internal video recording, normal run from 31s to 71+s, animation speed none, playback speed 2.138-->

TODO explain why arrow from host6 remains: It is a static route. RIP only controls routes between routers,
and host routes are statically configured by :ned:`Ipv4NetworkConfigurator`.


Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
