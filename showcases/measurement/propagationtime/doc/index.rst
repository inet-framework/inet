Measuring Propagation Time
==========================

Goals
-----

In this example, we explore the channel propagation time statistics for wired and
wireless transmission media.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/propagationtime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/propagationtime>`__

The Model
---------

The packet propagation time is measured from the moment the beginning of a physical
signal encoding the packet leaves the transmitter network interface up to the moment
the beginning of the same physical signal arrives at the receiver network interface.
This time usually equals the same difference measured for the end of the physical
signal. The exception would be when the receiver is moving relative to the transmitter
with a relatively high speed compared to the propagation speed of the physical signal,
but it is rarely the case in communication network simulation.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/PropagationTime.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PropagationTimeMeasurementShowcase.ned <../PropagationTimeMeasurementShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/measurement/propagationtime`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/measurement/propagationtime && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

