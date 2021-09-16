Automatic Stream Configuration with Failure Protection
======================================================

Goals
-----

In this example we demonstrate the automatic stream redundancy configuration based
on the link and node failure protection requirements.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/automaticfailureprotection <https://github.com/inet-framework/inet-showcases/tree/master/tsn/framereplication/automaticfailureprotection>`__

The Model
---------

In this case we use a different automatic stream redundancy configurator that
takes the link and node failure protection requirements for each redundany stream
as an argument. The automatic configurator computes the different paths that each
stream must take in order to be protected against any of the listed failures so
that at least one working path remains.

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


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AutomaticFailureProtectionShowcase.ned <../AutomaticFailureProtectionShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

