Step 4. Timeout Timer and Garbage Collection Timer
==================================================

Goals
-----

In this step, we will demonstrate the operation of timeout and
garbage-collection timers.

Routers talk to their neighbours regularly to update them about the changes in
their routing tables. Every time a router receives one of these messages from
one of its neighbours, it resets a timeout timer associated with this route. If
these messages stop coming, the timer expires (its default value is 180
seconds), and the router marks this route as unreachable. However, it keeps the
route in its routing table, and starts a garbage-collection timer. It
deletes the route when the garbage-collection timer expires (its default value
is 120 seconds).

Network Configuration
---------------------

Our network configuration is the same as the one we used in the earlier steps.

The ``omnetpp.ini`` configuration is listed here:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step4
   :end-before: ------

Experiments
-----------

To observe how RIP behaves, we introduce a breakdown by disconnecting the link
between ``router2`` and ``switch1`` at t = 50s, and simulate three
possible scenarios in which the link becomes operational again:

* A. Before the timeout timer associated with its routing table entry expires,
* B. After the timeout timer expires but before the garbage-collection timer expires, and
* C. After both timers expire, and the link is purged from the routing table.

We run these cases one by one. Their ``omnetpp.ini`` configuration sections are
``Step4A``, ``Step4B`` and ``Step4C``, respectively. These configurations are
identical apart from the scenario script they refer to.


A: Link becomes operational before the timeout timer expires
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We break the link connecting ``router2`` to ``switch1`` at t = 50s, and
reconnect at t = 150s. This is written in the ``scenario5.xml`` file:

.. literalinclude:: ../scenario5.xml
   :language: xml

After the ``switch1`` connection is lost, the timeout timer associated with the
routing table entry related to this link is not reset any more, and after 180
seconds (default value) the router marks the associated entry as "unreachable".
But here we re-activate the link before it expires.

B: Link becomes operational after the timeout timer expires
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here, we use ``scenario6.xml``, in which we make the link reactivation to occur
at t = 300s. This time:

.. literalinclude:: ../scenario6.xml
   :language: xml

C: Link becomes operational after the garbage-collection timer expires
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this scenario, we wait until after the garbage-collection timer expires to
observe the changes to the routing tables. The corresponding ``scenario7.xml`` file:

.. literalinclude:: ../scenario7.xml
   :language: xml

The link breaks at t = 50s, and ``router1`` stops receiving any updates on the
route to the 10.0.0.24/29 network from ``router2``. As shown in the highlighted
line of the image below, the last update to the route was approximately at t =
30.5929s, the moment the timeout timer was last reset.

.. figure:: media/step4_3.png
   :width: 80%
   :align: center

The timeout timer expires at t = 210s. At that time, the route's metric is set
to 16, and the garbage-collection timer is started.

.. figure:: media/step4_4.png
   :width: 80%
   :align: center

At t = 330s, the garbage-collection timer expires, and the route is removed from
the routing table:

TODO image 4C3

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`scenario5.xml <../scenario5.xml>`,
:download:`scenario6.xml <../scenario6.xml>`
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
