Automatic Multipath Stream Configuration
========================================

Goals
-----

In this example we demonstrate the automatic stream redundancy configuration based
on multiple paths from source to destination.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/automaticmultipathconfiguration <https://github.com/inet-framework/inet-showcases/tree/master/tsn/framereplication/automaticmultipathconfiguration>`__

The Model
---------

In this case we use an automatic stream redundancy configurator that takes the
different paths for each redundant stream as an argument. The automatic configurator
sets the parameters of all stream identification, stream merging, stream splitting,
string encoding and stream decoding components of all network nodes.

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


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AutomaticMultipathConfigurationShowcase.ned <../AutomaticMultipathConfigurationShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

