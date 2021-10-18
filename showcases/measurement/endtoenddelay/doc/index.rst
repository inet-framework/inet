Measuring End-to-end Delay
==========================

Goals
-----

In this example we explore the end-to-end delay statistics of applications.

| INET version: ``4.4``
| Source files location: `inet/showcases/measurement/endtoenddelay <https://github.com/inet-framework/inet/tree/master/showcases/measurement/endtoenddelay>`__

The Model
---------

The end-to-end delay is measured from the moment the packet leaves the source
application to the moment the same packet arrives at the destination application.

Here is the network:

.. figure:: media/Network.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/EndToEndDelayHistogram.png
   :align: center

.. figure:: media/EndToEndDelayVector.png
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`EndToEndDelayMeasurementShowcase.ned <../EndToEndDelayMeasurementShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

