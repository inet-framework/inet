Measuring Residence Time
========================

Goals
-----

In this example, we explore the packet residence time statistics of network nodes.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/residencetime <https://github.com/inet-framework/inet/tree/master/showcases/measurement/residencetime>`__

The Model
---------

The packet residence time is measured from the moment a packet enters a network
node up to the moment the same packet leaves the network node. This statistic
is also collected for packets that are created and/or destroyed in network
nodes.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/SourceHistogram.png
   :align: center

.. figure:: media/SourceVector.png
   :align: center

.. figure:: media/SwitchHistogram.png
   :align: center

.. figure:: media/SwitchVector.png
   :align: center

.. figure:: media/DestinationHistogram.png
   :align: center

.. figure:: media/DestinationVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ResidenceTimeMeasurementShowcase.ned <../ResidenceTimeMeasurementShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/measurement/residencetime`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/measurement/residencetime && inet'

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

