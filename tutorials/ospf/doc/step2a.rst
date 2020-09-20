Step 2a. Reroute after link breakage
====================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This configuration is based on step 1.

This step uses the following network:

.. figure:: media/step1-2-11-12.png
   :width: 100%
   :align: center

.. literalinclude:: ../OspfNetwork.ned
   :start-at: OspfNetwork
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2a
   :end-before: ------

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OspfNetwork.ned <../OspfNetwork.ned>`,
:download:`ASConfig_cost.xml <../ASConfig_cost.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
