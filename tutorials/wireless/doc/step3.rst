Step 3. Adding more nodes and decreasing the communication range
================================================================

Goals
-----

Later in this tutorial, we'll want to turn our model into an ad-hoc
network and experiment with routing. To this end, we add three more
wireless nodes and reduce the communication range so that our two
original hosts cannot reach one another directly. In later steps, we'll
set up routing and use the extra nodes as relays.

The model
---------

We need to add three more hosts. This could be done by copying and
editing the network used in the previous steps, but instead, we extend
:ned:`WirelessA` into :ned:`WirelessB` using the inheritance feature of NED:



.. literalinclude:: ../WirelessB.ned
   :language: ned
   :start-at: network WirelessB

We decrease the communication range of the radios of all hosts to 250
meters. This will make direct communication between hosts A and B
impossible because their distance is 400 meters. The recently added
hosts are in the correct positions to relay data between hosts A and B,
but routing is not yet configured. The result is that hosts A and B will
not be able to communicate at all.

The configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Config Wireless03
   :end-before: #---

Results
-------

When we run the simulation, blue circles confirm that hosts R1 and R2
are the only hosts in the communication range of host A. Therefore, they
are the only ones that receive host A's transmissions. This is indicated
by the dotted arrows connecting host A to R1 and R2, respectively,
representing recent successful receptions in the physical layer.

Host B is in the transmission range of host R1, and R1 could potentially
relay A's packets, but it drops them because routing is not configured
yet (it will be configured in a later step). Therefore no packets are
received by host B.

.. figure:: media/wireless-step3-2.png
   :width: 100%

Host R1's MAC submodule logs indicate that it is discarding the received
packets, as they are not addressed to it:

.. figure:: media/wireless-step3-log.png
   :width: 100%

**Number of packets received by host B: 0**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WirelessB.ned <../WirelessB.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.
