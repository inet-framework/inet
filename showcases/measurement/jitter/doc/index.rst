Measuring Packet Delay Variation
================================

Goals
-----

In this example, we explore the various packet delay variation (also known as packet jitter) statistics of application modules.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/jitter <https://github.com/inet-framework/inet/tree/master/showcases/measurement/jitter>`__

The Model
---------

The packet delay variation is measured in several different forms:

- *Instantaneous packet delay variation*: the difference between the packet delay of successive packets (``packetJitter`` statistic)
- *Variance of packet delay* (``packetDelayVariation`` statistic)
- *Packet delay difference compared to the mean value* (``packetDelayDifferenceToMean`` statistic)

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/PacketJitterHistogram.png
   :align: center

.. figure:: media/PacketJitterVector.png
   :align: center

.. figure:: media/PacketDelayDifferenceToMeanHistogram.png
   :align: center

.. figure:: media/PacketDelayDifferenceToMeanVector.png
   :align: center

.. figure:: media/PacketDelayVariationHistogram.png
   :align: center

.. figure:: media/PacketDelayVariationVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`JitterMeasurementShowcase.ned <../JitterMeasurementShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/measurement/jitter`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir
       -c 'cd inet-4.4/showcases/measurement/jitter && inet'

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

