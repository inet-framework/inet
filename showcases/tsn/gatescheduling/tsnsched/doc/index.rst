TSNsched-based Gate Scheduling
==============================

Goals
-----

This showcase demonstrates the TSNsched gate schedule configurator that solves the autoconfiguration problem with an external SAT solver tool.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/tsnsched <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/tsnsched>`__

The Model
---------

The :ned:`TsnSchedGateScheduleConfigurator` module provides a gate scheduling configurator that uses the `TSNsched` tool that is available at https://github.com/ACassimiro/TSNsched (check the NED documentation of the configurator for installation
instructions). The module has the same parameters as the other gate schedule configurators.

The simulation uses the same network as the showcases for the Eager and SAT configurators:

.. figure:: media/Network.png
    :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini

.. note:: There are two changes in this configuration compared to the Eager and SAT configurator showcases, due to limitations in the SAT solver tool: 
    
          - The traffic is half as dense as in the Eager and SAT configurator showcases. Currently, the tool can only find a solution
            to the autoconfiguration problem if all transmissions are completed within the gate cycle (though it should be possible for the individual frame transmissions to overlap). 
          - 12 Bytes need to be added to the packet size in the XML configuration to account for the inter-frame gap.

Results
-------

The following sequence chart shows a gate cycle period (1ms):

.. figure:: media/seqchart3.png
    :align: center

Note that the frames are forwarded immediatelly by the switches.

The following sequence chart shows frame transmissions, with the best effort gate state displayed on the axes for the two switches.

.. figure:: media/gates.png
    :align: center

Note that the gates are open so that two frames can be transmitted, and the frame transmissions and the send window are closely aligned in time.

The following figure shows application end-to-end delay for the four streams:

.. figure:: media/delay.png
    :align: center

All streams have the minimal possible delay, as they are forwarded immediatelly.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/793>`__ page in the GitHub issue tracker for commenting on this showcase.

