Automatic Multipath Stream Configuration
========================================

Goals
-----

In this example, we demonstrate the automatic stream redundancy configuration based
on multiple paths from the source to the destination.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/automaticmultipathconfiguration <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/automaticmultipathconfiguration>`__

The Model
---------

In this case, we use an automatic stream redundancy configurator that takes the
different paths for each redundant stream as an argument. The automatic configurator
sets the parameters of all stream identification, stream merging, stream splitting,
string encoding, and stream decoding components of all network nodes.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the number of received and sent packets:

.. figure:: media/packetsreceivedsent.png
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center

The expected number of successfully received packets relative to the number of
sent packets is verified by the python script (``compute_frame_replication_success_rate_analytically2()`` function in ``inet/python/inet/tests/validation.py``). The expected result is around 0.657.

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AutomaticMultipathConfigurationShowcase.ned <../AutomaticMultipathConfigurationShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/framereplication/automaticmultipathconfiguration`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/framereplication/automaticmultipathconfiguration && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/788>`__ page in the GitHub issue tracker for commenting on this showcase.

