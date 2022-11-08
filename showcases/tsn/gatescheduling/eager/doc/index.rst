Eager Gate Schedule Configuration
=================================

Goals
-----

This showcase demonstrates how the eager gate schedule configurator can set up schedules in a simple network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/eager <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/eager>`__

The Model
---------

.. Q what are gate schedules? set gate opening closing of time aware shapers in the network and also app start time offsets to guarantee some delay or jitter
  can be set by hand (in the periodic gates in the time aware shapers) but in complex networks this is hard but can be automated by configurators
  (dont repeat everything on the intro page)

.. Q what is eager gate scheduling? this is done by the eager gate schedule configurator which is a simple configurator that sets schedules eagerly

The simulation uses the following network:

.. figure:: media/Network.png
    :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini

Results
-------

A gate cycle duration of 1ms is displayed on the following sequence chart. Note how time efficient the flow of packets from the sources to the sinks are:

.. figure:: media/seqchart.png
    :align: center

Here is the delay for the second packet of ``client2`` in the best effort traffic class, from the packet source to the packet sink. Note that this stream is the outlier on the above chart. The delay is within the 500us requirement, but it's quite close to it:

.. figure:: media/timediff.png
    :align: center

The following chart displays the delay for individual packets of the different traffic categories:

.. figure:: media/delay.png
    :align: center

All delay is within the specified constraints.

.. note:: Both video streams and the ``client2 best effort`` stream have two cluster points. This is due to these traffic classes having multiple packets per gate cycle. As the different flows interact, some packets have increased delay.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/791>`__ page in the GitHub issue tracker for commenting on this showcase.

