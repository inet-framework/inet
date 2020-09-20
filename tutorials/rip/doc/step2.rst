Step 2. Pinging after RIP convergence
=====================================

Goals
-----

We show how RIP is used for progressively building up the routing tables of the
routers. In order to keep the discussion easy to digest, in this step, we do not
introduce any changes in the network while the simulation is running, and we do
not use the advanced RIP features.

If you would like to brush up your RIP knowledge, you can find a good discussion
of the distance-vector algorithm and RIP in `this section
<https://book.systemsapproach.org/internetworking/routing.html#distance-vector-rip>`_
of L. Peterson and B. Davie’s superb book *Computer Networks: A Systems
Approach*, now freely available `online  <https://book.systemsapproach.org>`_.

Network Configuration
---------------------

We use the same network configuration as the previous one, except that we start
the simulation with empty routing tables.  The experiment is set up in
``omnetpp.ini``, in section ``Step2``:

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ------


Experiment
----------

Observe in the following video how routers inform their neighbours and
progressively build their routing tables (as indicated by the appearance of blue
and red arrows linking the routers). At the end of a few iterations, we see the
same routing configuration as Step 1.

.. video:: media/step2_3.mp4
   :width: 100%

..   <!--internal video recording, normal run until sendPing, animation speed none, playback speed 2.138, max anim speed kludge in simtimetextfigure-->

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
