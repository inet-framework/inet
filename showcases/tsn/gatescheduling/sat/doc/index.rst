SAT-Solver-based Gate Schedule Configuration
============================================

Goals
-----

This showcase demonstrates a gate schedule configurator that solves the autoconfiguration problem using a multivariate linear inequality system, directly producing the gate control lists from the variables.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/sat <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/sat>`__

The Model
---------

The SAT-solver-based gate schedule configurator requires the `Z3 Gate Scheduling Configurator` INET feature to be enabled, and the ``libz3-dev`` or ``z3-devel`` packages to be installed.

The simulation uses the following network:

.. figure:: media/Network.png
    :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini

Results
-------

A gate cycle of 1ms is displayed on the following sequence chart. Note that the time efficiency of the gate schedules is even better than in the `Eager` case:

.. figure:: media/seqchart.png
    :align: center

The application end-to-end delay for the different traffic classes is displayed on the following chart:

.. figure:: media/delay.png
    :align: center

The delay is constant for every packet, and within the specified constraint of 500us. Note that the traffic delay is symmetric among the different source and sink combinations (in contrast with the `Eager` case).

The next sequence chart excerpt displays one packet as it travels from the packet source to the packet sink, with the delay indicated:

.. figure:: media/seqchart_delay.png
    :align: center

All packets have the exact same delay, which can be calculated analytically: ``(propagation time + transmission time) * 3`` (queueing time is zero).
Inserting the values of 84.64 us transmission time and 0.05 us propagation time per link, the delay is 254.07 us for the best effort traffic category.

The following charts compare the SAT-based and Eager gate schedule configurators in terms of application end-to-end delay:

.. figure:: media/delay_comp.png
    :align: center

The difference is that in case of the SAT-based gate schedule configurator, all flows in a given traffic class have the same constant delay; in case of the eager configurator's delay, some streams have more delay than others.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/792>`__ page in the GitHub issue tracker for commenting on this showcase.

