Manual Stream Configuration
===========================

Goals
-----

In this example we demonstrate manual configuration of stream identification,
stream splitting, stream merging, stream encoding and stream decoding to achieve
the desired stream redundancy.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/manualconfiguration <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/manualconfiguration>`__

The Model
---------

In this configuration we replicate a network topology that is presented in the
IEEE 802.1 CB amendment. The network contains one source and on destination nodes,
where the source sends a redundant data stream through five switches. The stream
is duplicated in three of the switches and merged in two of them.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the number of received and sent packets:

.. figure:: media/packetsreceivedsent.svg
   :align: center
   :width: 100%

Here is the ratio of received and sent packets:

.. figure:: media/packetratio.png
   :align: center

The expected number of successfully received packets relative to the number of
sent packets is verified by the python scripts. The expected result is around 0.657.

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ManualConfigurationShowcase.ned <../ManualConfigurationShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/789>`__ page in the GitHub issue tracker for commenting on this showcase.

