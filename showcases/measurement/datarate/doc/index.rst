Measuring Data Rate
===================

Goals
-----

In this example, we explore the data rate statistics of application, queue, and
filter modules inside network nodes.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/measurement/datarate <https://github.com/inet-framework/inet/tree/master/showcases/measurement/datarate>`__

The Model
---------

The data rate is measured by observing the packets as they pass through
over time at a certain point in the node architecture. For example, an application
source module produces packets over time, and this process has its own data rate.
Similarly, a queue module enqueues and dequeues packets over time, and both of
these processes have their own data rate. These data rates are different, which
in turn causes the queue length to increase or decrease over time.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/source_outgoing.png
   :align: center
   :width: 90%

.. figure:: media/destination_incoming.png
   :align: center
   :width: 90%

.. figure:: media/switch_incoming.png
   :align: center
   :width: 90%

.. figure:: media/switch_outgoing.png
   :align: center
   :width: 90%

.. **TODO** why does the switch has more datarate ? protocol headers?

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`DataRateMeasurementShowcase.ned <../DataRateMeasurementShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/measurement/datarate`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/measurement/datarate && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

