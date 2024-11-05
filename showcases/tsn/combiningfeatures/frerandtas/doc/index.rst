Frame Replication with Time-Aware Shaping
=========================================

Goals
-----

In this example, we demonstrate how to automatically configure time-aware shaping
in the presence of frame replication.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/frerandtas <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/frerandtas>`__

The Model
---------

The network contains a source and a destination node and five switches.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

Here is the number of packets received and sent:

.. figure:: media/packetsreceivedsent.svg
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`FrerAndTasShowcase.ned <../FrerAndTasShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/combiningfeatures/frerandtas`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/combiningfeatures/frerandtas && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/783>`__ page in the GitHub issue tracker for commenting on this showcase.


