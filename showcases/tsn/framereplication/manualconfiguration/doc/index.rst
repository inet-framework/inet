Manual Stream Configuration
===========================

Goals
-----

In this example we demonstrate manual configuration of stream identification,
stream splitting, stream merging, stream encoding and stream decoding to achieve
the desired stream redundancy.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/manualconfiguration <https://github.com/inet-framework/inet-showcases/tree/master/tsn/framereplication/manualconfiguration>`__

The Model
---------

In this configuration we replicate a network topology that is presented in the
IEEE 802.1 CB amendment. The network contains one source and on destination nodes,
where the source sends a redundant data stream through five switches. The stream
is duplicated in three of the switches and merged in two of them.

Here is the network:

.. .. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :align: center
   :language: ini

Results
-------

The expected number of successfully received packets relative to the number of
sent packets is verified by the python scripts. The expected result is around 0.657.

The following video shows the behavior in Qtenv:

.. video:: media/behavior.mp4
   :align: center
   :width: 90%

Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ManualConfigurationShowcase.ned <../ManualConfigurationShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

