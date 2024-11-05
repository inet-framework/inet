Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the scheduling and traffic shaping modules can
work outside the context of a network node. Doing so may facilitate assembling
and validating specific complex scheduling and traffic shaping behaviors which
can be difficult to replicate in a complete network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/trafficshaping/underthehood <https://github.com/inet-framework/inet/tree/master/showcases/tsn/trafficshaping/underthehood>`__

The Model
---------

TODO incomplete

The network contains three independent packet sources which are connected to a
single asynchronous traffic shaper using a packet multiplexer.

.. figure:: media/Network.png
   :align: center

The three sources generate the same stochastic traffic. The traffic shaper is
driven by an active packet server module with a constant processing time.

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

TODO

.. figure:: media/QueueingTime.png
   :align: center

TODO

.. figure:: media/QueueLength.png
   :align: center


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/trafficshaping/underthehood`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir
       -c 'cd inet-4.4/showcases/tsn/trafficshaping/underthehood && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/803>`__ page in the GitHub issue tracker for commenting on this showcase.

