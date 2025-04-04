Understanding Measurement Relationships
=======================================

Goals
-----

In this example, we explore the relationships between various measurements that
are presented in the measurement showcases.

| Since INET version: ``4.4``
| Source files location: `inet/showcases/measurement/relationships <https://github.com/inet-framework/inet/tree/master/showcases/measurement/relationships>`__

The Model
---------

The end-to-end delay measured between two applications can be thought of as a sum
of different time categories such as queuing time, processing time, transmission
time, propagation time, and so on. Moreover, each one of these specific times can
be further split up between different network nodes, network interfaces, or even
smaller submodules.

Here is the network:

.. .. figure:: media/Network.png
..    :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. .. figure:: media/Results.png
..    :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MeasurementRelationshipsShowcase.ned <../MeasurementRelationshipsShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/measurement/relationships`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.5.*/showcases/measurement/relationships && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

